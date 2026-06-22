// This file is part of UNItopia Mudlib.
// ---------------------------------------------------------------------------
// File:        /apps/notifier.c
// Description: Benachrichtigung aus FDB,tap und sonstige heraus.
// Author:      Myonara(25.12.2016,Neustart 04.04.2017 und 26.10.2017)
//
// UID: Apps

nosave variables inherit "/i/tools/security";
nosave variables inherit "/i/tools/build_table";
nosave variables inherit "/i/tools/debuglog_db";
nosave variables inherit "/i/item/message";

#include <apps.h>
#include <config.h>
#include <database.h>
#include <dynamic_browser.h>
#include <editor.h>
#include <error.h>
#include <error_db.h>
#include <gilden.h>
#include <input_to.h>
#include <level.h>
#include <mail.h>
#include <message.h>
#include <misc.h>
#include <notify_fail.h>
#include <room_types.h>
#include <security.h>
#include <time.h>

#include <notifier.h>

#ifdef UNItopia
#define PDBG_GROUP "myonara:notifier"
#include <p/debugger.h>     // TODO rauswerfen.
#else
#define PDBG(x)
#endif

#define NOTIFIER_DB_SAVE "/var/spool/mail/notifier.db"
#define RETENTION_DAYS 30

#define SOFORTGRUPPE_ARMAGEDDON ({"gnomi","pumuk","myonara"})

#define NOTIFIER_MAX_MAILS_PER_SUMMARY  20
#define NOTIFIER_LIMIT_FDB_EVENTS       20

#define SCHEDULER_MIN_EVAL  750000
#define SCHEDULER_MAX_ABOS  10
#define SCHEDULER_MAX_FDB   3
#define SCHEDULER_MAX_MAIL  3
#define SCHEDULER_START_MIN     5
#define SCHEDULER_START_DELTA   10
#define SCHEDULER_MAX_DELTA     600
#define SCHEDULER_SMALL_LOOP    5

#define SCHEDULER_LAST_ERR      "scheduler:last:error"
#define SCHEDULER_ERROR_COUNT   "scheduler:error:count"
#define SCHEDULER_COMMIT_COUNT  "scheduler:commit:count"
#define SCHEDULER_PHASE         "scheduler:phase"
#define SCHEDULER_LAST_EVAL     "scheduler:last:eval"
#define SCHEDULER_LAST_START    "scheduler:last:start"
#define SCHEDULER_NEXT_START    "scheduler:next:start"
#define SCHEDULER_PH_INIT       0
#define SCHEDULER_PH_ABO_F0     1
#define SCHEDULER_PH_FDB_F0     2
#define SCHEDULER_PH_ABO_F1     3
#define SCHEDULER_PH_FDB_F1     4
#define SCHEDULER_PH_ABO_F2     5
#define SCHEDULER_PH_FDB_F2     6
#define SCHEDULER_PH_ABO_F3     7
#define SCHEDULER_PH_FDB_F3     8
#define SCHEDULER_PH_SEND_MAIL  9

// Optionen fuer den Editor
private nosave mapping m_edit = ([]);
// Variablen fuer den Scheduler
private nosave mapping m_scheduler = ([]);
static void scheduler();

// ---------------------------------------------------------------------------
// Beschreibung fuer den Notifier...
public string query_database_description()
{
    return "Zentrale Konfiguration rund um den Mail-Notifier";
}

// wer schreiben darf, darf auch intern alles... :)
// Funktion wird innerhalb der DB ebenso verwendet wie fuer die Kommandos.
protected int validate_debugger(string rname)
{
    return member(ADMINS,rname)!=-1;
}

public varargs int find_next_start(int tim)
{
    int *ti = timearray(tim||time());
    for (tim = SCHEDULER_START_MIN;
         tim <= ti[TM_MIN] ;
         tim +=SCHEDULER_START_DELTA);
    ti[TM_MIN] = tim;
    ti[TM_SEC] = 0;
    return array_to_time(ti);
}

//----------------------------------------------------------------------------

private void pr_inhalt_mini_ed_end(string * text)
{
    PDBG("minied-end"+mixed2str(caller_stack())+ "\n"+mixed2str(text));
    if (closurep(m_edit[NOTIFIER_EDITOR_CB]))
        funcall(m_edit[NOTIFIER_EDITOR_CB],text);
}

static void pr_inhalt_web_ed_end(string text)
{
    PDBG("webed-end"+mixed2str(caller_stack())+ "\n"+mixed2str(text));
}

private void pr_inhalt_edit_end(string * puffer, int changed)
{
    object *cs = caller_stack();
    PDBG("edit-end"+mixed2str(cs)+ "\n"+mixed2str(puffer));
}

public int start_edit(string * buffer,string title)
{
    if (TP->mini_ed(#'pr_inhalt_mini_ed_end, 0,0,0,
        ([MINI_ED_TITLE: title]), buffer)) 
    {
        return 1;
    } 
    else 
    {
        send_message_to(TP,MT_NOTIFY,MA_LOOK,wrap(
                "Editieren nicht verfügbar."));
        return 0;
    }
}

// ---------------------------------------------------------------------------
private int _notify_online_wiz(string wiz,string summary)
{
    mixed wizdata = db_query_err("SELECT flags,min_frequency "
        "FROM wizard WHERE wizname = ?1",wiz);
    object pl = find_player(wiz);
    if (!sizeof(wizdata))
        return 0; // No Mail, No Online info
    wizdata = wizdata[0];
    if (!wizp(pl))
    {
        if (wizdata[1] == 0)
            return 0; // No frequency set, no mail.
        return 1; // Send Offline Mail 
    }
    if (wizdata[0] & NOTIFIER_WF_ONLINE_N)
    {
        send_message_to(pl,MT_DEBUG,MA_USE,wrap(summary));
    }
    if (wizdata[0] & NOTIFIER_WF_ONLINE_M)
    {
        return 2; // online mail...
    }
    return -1; // Online Notification Only.
}

private varargs int _send_mail_to(string wiz,string eventid)
{
    mixed events,event,r;
    string body = "",tmp;
    int cnt = 0;
    if (!stringp(wiz))
    {
        r = db_query_err("SELECT DISTINCT wizname "
            "FROM wiz_events WHERE sent_on = 0 "
            "ORDER BY notified_on,eventid "
            "LIMIT 1 ");
        if (sizeof(r))
        {
            wiz = get_one_string(r);
        }
        else
        {
            return 1; // no action.
        }
    }
#define MAIL_EVENT_ID           0
#define MAIL_EVENT_SUMMARY      1
#define MAIL_EVENT_BODY         2
    events = db_query_err("SELECT w.eventid,a.summary,a.body "
        "FROM wiz_events w JOIN all_events a ON w.eventid = a.eventid "
        "WHERE w.wizname = ?1 AND w.sent_on = 0 "
        +(eventid?("AND w.eventid = '"+
        db_escape_string(eventid)+"' "):"")+
        "AND (a.obsolete_on = 0 OR a.obsolete_on > ?3) "
        "ORDER BY w.notified_on,w.eventid LIMIT ?2",
        wiz,NOTIFIER_MAX_MAILS_PER_SUMMARY,time());
    if (!sizeof(events))
        return 1; // no actions.
    if (sizeof(events)==1)
    {
        event = events[0];
        if (_notify_online_wiz(wiz,event[MAIL_EVENT_SUMMARY])<=0)
            return 2;
// void send_notification(int date, string subject, string txt)
        MAILD->send_notification(time(),event[MAIL_EVENT_SUMMARY],
            event[MAIL_EVENT_BODY]);
        db_query_err("UPDATE wiz_events SET sent_on = ?1 "
            "WHERE sent_on = 0 AND wizname = ?2 AND eventid = ?3 ",
                time(),wiz,event[MAIL_EVENT_ID]);
        return 2;
    }
    foreach (event : events)
    {
        if (_notify_online_wiz(wiz,event[MAIL_EVENT_SUMMARY])>0)
        {
            tmp = "\n"+event[MAIL_EVENT_SUMMARY]+"\n";
            tmp += event[MAIL_EVENT_BODY];
            // TODO size check against body...
            body += tmp;
        }
        cnt++;
        db_query_err("UPDATE wiz_events SET sent_on = ?1 "
            "WHERE sent_on = 0 AND wizname = ?2 AND eventid = ?3 ",
                time(),wiz,event[MAIL_EVENT_ID]);
    }
    MAILD->send_notification(time(),
            sprintf("Zusammenfassung von %d Ereignissen",cnt),
            body);
    return 1+cnt;
}

private varargs int send_mail_to(string *wizzes, string eventid)
{
    int cnt,tmp;
    string wiz;
    if (!sizeof(wizzes))
    {
        mixed r = db_query_err("SELECT DISTINCT wizname "
            "FROM wiz_events WHERE sent_on = 0 "
            "ORDER BY notified_on,eventid "
            "LIMIT "+SCHEDULER_MAX_MAIL+" ");
        wizzes = map(r||({}),(: $1[0] :) );
    }
    foreach (wiz : wizzes-({0,""}) )
    {
        tmp = _send_mail_to(wiz,eventid);
        cnt += (tmp-1);
    }
    return cnt;
}

private string register_mail(string eventid,string summary, string body, 
    string * wizzes, int obsolete_on) 
{
    if (!sizeof(wizzes))
        return 0;
    
    db_query_err("INSERT INTO all_events "
        "(eventid,summary,body,created_on,obsolete_on) "
        "VALUES (?,?,?,?,?) ",eventid,summary,body,time(),obsolete_on);
    string wiz;
    foreach (wiz : wizzes)
    {
        db_query_err("INSERT INTO wiz_events "
            "(eventid,wizname,notified_on,sent_on) "
            "VALUES (?,?,?,?)",eventid,wiz,time(),0);
    }
    return eventid;
}

// ---------------------------------------------------------------------------
public int register_fdb_errnum(string listid, int errnum)
{
    if (!check_security())
        return 0;
    db_query_err("INSERT OR IGNORE INTO errnum2listid "
        "(listid,errnum,notified_on,cnt_senders) VALUES (?,?,?,?)",
        listid,errnum,time(),0);
    return 1;
}

private void _process_fdb_event(string listid,int errnum,
    string wiz,int mailformat)
{
    string summary = "zfe "+errnum+" ("+listid+")";
    string body = "/obj/zauberstab"->zfe_get_errmsg(errnum);
    string eventid = "FDB#"+listid+"#"+errnum+"#"+time();
    string *split;
    switch (mailformat)
    {
        case 1: // Long format
            break;
        default:
        case 2: // Short format
            split = explode(body,"\n");
            if (sizeof(split)<42)
                break;
            split = split[0..19]
                  + ({center(" usw ",79,".")})
                  + split[<20..];
            body = implode(split,"\n");
            break;
        case 3: // no body.
            body = ""; 
            break;
    }
    register_mail(eventid,summary,body,({wiz}),0);
}

private int process_fdb_events(int frq)
{
    string q,dday;
    mixed eline;
    mixed r;
    int* ti = timearray(time());
    int daytime = ti[TM_HOUR]*3600 + ti[TM_MIN]*60 + ti[TM_SEC];
    
    q = "SELECT e.listid, e.errnum, wl.wiz, wl.mailformat,e.notified_on, ";
    q+= "MAX(w.min_frequency,wl.frequency) AS eff_frequency, ";
    q+= "(w.daily_hour*3660 + w.daily_min*60) AS daily_time, ";
    q+= "w.weekly_day,(w.weekly_hour*3600 + w.weekly_min*60) AS weekly_time ";
    q+= "FROM errnum2listid e JOIN wiz2listid wl ON e.listid = wl.listid ";
    q+= "JOIN wizard w ON w.wizname = wl.wiz ";
    q+= "WHERE eff_frequency = "+frq+" ";
    q+= "AND w.min_frequency <> 0 AND wl.frequency <> 0 ";
    switch (frq)
    {
        case NOTIFIER_FQ_AT_ONCE:
            q+= "AND (w.wizname ||'#FDBLO:'||e.listid||'#'||e.errnum||'#"
                +ti[TM_MDAY]+"#'||e.notified_on) NOT IN ";
            q+= "(SELECT dday FROM time_done WHERE dtype = "+frq+") ";
            break;
        case NOTIFIER_FQ_DAILY:
            q+= "AND daily_time <= "+daytime+" ";
            q+= "AND (w.wizname ||'#FDBL:'||e.listid||'#'||e.errnum||'#"
                +ti[TM_MDAY]+"') NOT IN ";
            q+= "(SELECT dday FROM time_done WHERE dtype = "+frq+") ";
            break;
        case NOTIFIER_FQ_WEEKLY:
            q+= "AND weekly_day = "+ti[TM_WDAY]+" ";
            q+= "AND weekly_time <= "+daytime+" ";
            q+= "AND (w.wizname ||'#FDBL:'||e.listid||'#'||e.errnum||'#"
                +ti[TM_MDAY]+"') NOT IN ";
            q+= "(SELECT dday FROM time_done WHERE dtype = "+frq+") ";
            break;
        case NOTIFIER_FQ_MONTHLY:
            if (ti[TM_MDAY]!=1 || daytime < 11*3600) 
                return 0; // Nur Monatserster ab 11:00
            q+= "AND (w.wizname ||'#FDBL:'||e.listid||'#'||e.errnum||'#"
                +ti[TM_MDAY]+"') NOT IN ";
            q+= "(SELECT dday FROM time_done WHERE dtype = "+frq+") ";
            break;
        default:
            return 0;
    }
    q+= "ORDER BY w.wizname,e.notified_on ";
    q+= "LIMIT "+NOTIFIER_LIMIT_FDB_EVENTS;
    r = db_query_err(q);
    if (!sizeof(r))
        return 0;
    foreach (eline : r)
    {
        db_begin();
        // e.listid, e.errnum, wl.wiz, wl.mailformat, e.notified_on
        _process_fdb_event(eline[0],eline[1],eline[2],eline[3]);
        switch(frq)
        {
            case NOTIFIER_FQ_AT_ONCE:
                dday = eline[2]+"#FDBLO:"+eline[0]
                    +"#"+eline[1]+"#"+to_string(ti[TM_MDAY])
                    +"#"+eline[4];
                db_query_err("INSERT OR REPLACE INTO time_done (dday,dtype) "
                    "VALUES (?,?)",dday,frq);
                db_query_err("DELETE FROM time_done WHERE dday like ?1 "
                    "AND dday NOT LIKE ?2 AND dtype = ?3",
                    eline[2]+"#FDBLO:"+eline[0]+"#"+eline[1]+"#%",
                    eline[2]+"#FDBLO:"+eline[0]+"#"+eline[1]+"#"
                    +to_string(ti[TM_MDAY])+"#%",frq);
                break;
            case NOTIFIER_FQ_DAILY:
            case NOTIFIER_FQ_WEEKLY:
            case NOTIFIER_FQ_MONTHLY:
                dday = eline[2]+"#FDBL:"+eline[0]
                    +"#"+eline[1]+"#"+to_string(ti[TM_MDAY]);
                db_query_err("INSERT OR REPLACE INTO time_done (dday,dtype) "
                    "VALUES (?,?)",dday,frq);
                db_query_err("DELETE FROM time_done WHERE dday like ?1 "
                    "AND dday <> ?2 AND dtype = ?3",
                    eline[2]+"#FDBL:"+eline[0]+"#"+eline[1]+"#%",dday,frq);
                break;
            default:
                break;
        }
        db_commit();
    }
    return sizeof(r);
}
// ---------------------------------------------------------------------------
private void process_one_abo(string wizname,string abo_type)
{
    db_query_err("INSERT OR IGNORE INTO wiz_events "
        "(eventid,wizname,notified_on, sent_on) "
        "SELECT eventid,?1,?3,0 FROM all_events WHERE eventid LIKE ?2||'%' "
        "AND eventid||'#'||?1 NOT IN (SELECT eventid||'#'|wizname "
        "FROM wiz_events WHERE wizname = ?1)",
        wizname,abo_type,time());
}

private int process_abos(int frq)
{
    string q,dday;
    mixed r;
    int* ti = timearray(time());
    int daytime = ti[TM_HOUR]*3600 + ti[TM_MIN]*60 + ti[TM_SEC];
    int ix;
    
    q = "SELECT w.wizname,w.flags,wa.abo_type,wa.mailformat, ";
    q+= "MAX(w.min_frequency,wa.frequency) AS eff_frequency, ";
    q+= "(w.daily_hour*3660 + w.daily_min*60) AS daily_time, ";
    q+= "w.weekly_day,(w.weekly_hour*3600 + w.weekly_min*60) AS weekly_time ";
    q+= "FROM wizard w JOIN wizard_abo wa ON (w.wizname = wa.wizname) ";
    q+= "WHERE eff_frequency = "+frq+" ";
    q+= "AND w.min_frequency <> 0 AND wa.frequency <> 0 ";
    switch (frq)
    {
        case NOTIFIER_FQ_AT_ONCE:
            break;
        case NOTIFIER_FQ_DAILY:
            q+= "AND daily_time <= "+daytime+" ";
            q+= "AND w.wizname ||'#'||abo_type||'#"+ti[TM_MDAY]+"' NOT IN ";
            q+= "(SELECT dday FROM time_done WHERE dtype = "+frq+") ";
            break;
        case NOTIFIER_FQ_WEEKLY:
            q+= "AND weekly_day = "+ti[TM_WDAY]+" ";
            q+= "AND weekly_time <= "+daytime+" ";
            q+= "AND w.wizname ||'#'||abo_type||'#"+ti[TM_MDAY]+"' NOT IN ";
            q+= "(SELECT dday FROM time_done WHERE dtype = "+frq+") ";
            break;
        case NOTIFIER_FQ_MONTHLY:
            if (ti[TM_MDAY]!=1 || daytime < 11*3600) 
                return 0; // Nur Monatserster ab 11:00
            q+= "AND w.wizname ||'#'||abo_type||'#"+ti[TM_MDAY]+"' NOT IN ";
            q+= "(SELECT dday FROM time_done WHERE dtype = "+frq+") ";
            break;
        default:
            return 0;
    }
    q+= "LIMIT "+SCHEDULER_MAX_ABOS+" "; 
    r = db_query_err(q);
    for (ix=0;ix<sizeof(r);ix++)
    {
        db_begin();
        process_one_abo(r[ix][0],r[ix][2]);
        switch(frq)
        {
            case NOTIFIER_FQ_DAILY:
            case NOTIFIER_FQ_WEEKLY:
            case NOTIFIER_FQ_MONTHLY:
                dday = r[ix][0]+"#"+r[ix][2]+"#"+to_string(ti[TM_MDAY]);
                db_query_err("INSERT OR REPLACE INTO time_done (dday,dtype) "
                    "VALUES (?,?)",dday,frq);
                db_query_err("DELETE FROM time_done WHERE dday like ?1 "
                    "AND dday <> ?2 AND dtype = ?3",
                    r[ix][0]+"#"+r[ix][2]+"#%",dday,frq);
            default:
                break;
        }
        db_commit();
    }
    return sizeof(r);
}
// ---------------------------------------------------------------------------
private void submit_abo_event(string abo,int ti,string body)
{
    string eventid = abo+"-"+ti;
    string summary;
    int expiry;
    if (abo == 0 || ti == 0 || body == 0)
    {
        return;
    }
    if (ti <= time())
    {        
        summary = abo+" vom "+shorttimestr(ti);
        expiry = ti+3600*24*40; // bisschen laenger als ein Monat
    }
    else 
    {
        summary = abo+" am "+shorttimestr(ti);
        expiry = ti+3600*24*3; // 3 Tage nach Plan aus.
    }
    db_query_err("INSERT OR IGNORE INTO all_events "
        "(eventid,summary,body,created_on,obsolete_on) "
        "VALUES (?,?,?,?,?)",
        eventid,summary,body,time(),expiry); 
    
}

private void inp_agenda(string* text)
{
    if (text)
        submit_abo_event(m_edit[NOTIFIER_ABO],
            m_edit[NOTIFIER_PLANNED_TIME],
            implode(text,"\n") );
    else
        browse_write_line("Editieren abgebrochen.");
    m_edit = ([]);
}

private void inp_time(string tstr,string abo_type,string agenda)
{
    int ti = dblog_shorttimestr2time(tstr);
    if (!ti)
    {
        input_to(#'inp_time, INPUT_PROMPT, "Planzeit?:", abo_type,agenda);
    } 
    else
    {
        if (!agenda)
        {
            m_edit[NOTIFIER_EDITOR_CB] = #'inp_agenda;
            m_edit[NOTIFIER_ABO] = abo_type;
            m_edit[NOTIFIER_PLANNED_TIME] = ti;
            start_edit( ({}),"Agenda eingeben" );
        }
    }
}

varargs int start_notifier_news(string news)
{
    check_security(CHECK_ERROR);
    if (!news)
    {
        m_edit[NOTIFIER_EDITOR_CB] = #'inp_agenda;
        m_edit[NOTIFIER_ABO] = "Notifier-News";
        m_edit[NOTIFIER_PLANNED_TIME] = time();
        start_edit( ({}),"Notifier-News" );
        return 2;
    }
    submit_abo_event("Notifier-News",time(),news);
    return 3;
}

varargs int start_pantheonstreffen(int planned_time,string agenda)
{
    check_security(CHECK_ERROR);
    if (planned_time == 0)
    {
        input_to(#'inp_time, INPUT_PROMPT, "Planzeit?:",
            "Pantheonstreffen",agenda);
        return 1;
    }
    if (!agenda)
    {
        m_edit[NOTIFIER_EDITOR_CB] = #'inp_agenda;
        m_edit[NOTIFIER_ABO] = "Pantheonstreffen";
        m_edit[NOTIFIER_PLANNED_TIME] = planned_time;
        start_edit( ({}),"Pantheonstreffen-Agenda" );
        return 2;
    }
    submit_abo_event("Pantheonstreffen",planned_time,agenda);
    return 3;
}

//----------------------------------------------------------------------------
private void check_for_armageddon()
{
    string current_id = "Armageddon_"+__BOOT_TIME__;
    string old_id = get_db_info("Armageddon");
    string summary, body,wizname;
    if (old_id != current_id)
    {
        summary = "UNItopia-Neustart";
        body = "Driver: "+__VERSION__
             + "\nBoot-Time: "+shorttimestr(__BOOT_TIME__);
        db_begin();
        db_query_err("INSERT OR IGNORE INTO all_events "
            "(eventid,summary,body,created_on,obsolete_on) "
            "VALUES (?,?,?,?,?)",
            current_id,summary,body,__BOOT_TIME__,time()+3600*24*40);
        foreach (wizname : SOFORTGRUPPE_ARMAGEDDON)
        {
            db_query_err("INSERT OR IGNORE INTO wiz_events "
                "(eventid,wizname,notified_on,sent_on) VALUES (?,?,?,0) ",
                current_id,wizname,time());
        }
        set_db_info("Armageddon",current_id);
        db_commit();
        send_mail_to(SOFORTGRUPPE_ARMAGEDDON,current_id);
    }
}

// ---------------------------------------------------------------------------
public int has_valid_config(string rname)
{
    mixed r;
    r = db_query_err("SELECT wizname FROM wizard WHERE wizname = ?",rname);
    if (!wizplayerp(rname) || sizeof(r)==0) return 0;
    return 1;
}

private int _create_config(string rname)
{
    mixed r;
    int *ti;
    r = db_query_err("SELECT wizname FROM wizard WHERE wizname = ?",rname);
    if (!wizplayerp(rname) || sizeof(r)>0) return 0;
    ti = timearray(time());
    db_query_err("INSERT INTO wizard (wizname,daily_hour,daily_min, "
        "weekly_day,weekly_hour,weekly_min,min_frequency, "
        "flags) "
        "VALUES (?,?,?, ?,?,?,?, ?) ",
        rname,ti[TM_HOUR],ti[TM_MIN],
        ti[TM_WDAY],ti[TM_HOUR],ti[TM_MIN],NOTIFIER_FQ_NEVER, 0);
    return 1;
}

#define L_NOTIFIER_CFG ({\
    NOTIFIER_WIZNAME,NOTIFIER_CFG_D_HOUR,NOTIFIER_CFG_D_MIN,\
    NOTIFIER_CFG_W_DAY,NOTIFIER_CFG_W_HOUR,NOTIFIER_CFG_W_MIN,\
    NOTIFIER_CFG_FREQUENCY,NOTIFIER_CFG_FLAGS })

public mapping get_one_config(string rname)
{
    string q;
    mixed r;
    if (!check_security() || !rname || !wizplayerp(rname))
        return 0;
    _create_config(rname);
    q = "SELECT wizname, daily_hour, daily_min, ";
    q+= "weekly_day, weekly_hour, weekly_min, ";
    q+= "min_frequency,flags ";
    q+= "FROM wizard WHERE wizname = ? ";
    r = db_query_err(q,rname);
    if (!sizeof(r)) return 0;
    return mkmapping(L_NOTIFIER_CFG,r[0]);
}

public int set_one_config(mapping cfg)
{
    string q,*cols = ({});
    mapping oldcfg;
    //int fq_flag = 0;
    if (!check_security() || !mappingp(cfg))
        return 0;
    oldcfg = get_one_config(cfg[NOTIFIER_WIZNAME]);
    if (!mappingp(oldcfg))
        return 0;
    if (oldcfg[NOTIFIER_CFG_D_HOUR] != cfg[NOTIFIER_CFG_D_HOUR])
    {
        cols += ({"daily_hour = "+cfg[NOTIFIER_CFG_D_HOUR]+" " });
    }
    if (oldcfg[NOTIFIER_CFG_D_MIN] != cfg[NOTIFIER_CFG_D_MIN])
    {
        cols += ({"daily_min = "+cfg[NOTIFIER_CFG_D_MIN]+" " });
    }
    if (oldcfg[NOTIFIER_CFG_W_DAY] != cfg[NOTIFIER_CFG_W_DAY])
    {
        cols += ({"week_day = "+cfg[NOTIFIER_CFG_W_DAY]+" " });
    }
    if (oldcfg[NOTIFIER_CFG_W_HOUR] != cfg[NOTIFIER_CFG_W_HOUR])
    {
        cols += ({"week_hour = "+cfg[NOTIFIER_CFG_W_HOUR]+" " });
    }
    if (oldcfg[NOTIFIER_CFG_W_MIN] != cfg[NOTIFIER_CFG_W_MIN])
    {
        cols += ({"week_min = "+cfg[NOTIFIER_CFG_W_MIN]+" " });
    }
    if (oldcfg[NOTIFIER_CFG_FREQUENCY] != cfg[NOTIFIER_CFG_FREQUENCY])
    {
        cols += ({"min_frequency = "+cfg[NOTIFIER_CFG_FREQUENCY]+" " });
        //if (oldcfg[NOTIFIER_CFG_FREQUENCY] == NOTIFIER_FQ_NEVER)
        //    fq_flag = 1;
    }
    if (oldcfg[NOTIFIER_CFG_FLAGS] != cfg[NOTIFIER_CFG_FLAGS])
    {
        cols += ({"flags = "+cfg[NOTIFIER_CFG_FLAGS]+" " });
    }
    if (!sizeof(cols))
        return 1; // no action.
    q = "UPDATE wizard SET "+implode(cols,", ");
    q+= " WHERE wizname = ?1 ";
    db_query_err(q,cfg[NOTIFIER_WIZNAME]);
    // TODO if fq_flag => mark_abo
    return 2; // update done.
}

public string* get_all_wizards()
{
    mixed r;
    if (!check_security())
        return 0;
    r = db_query_err("SELECT wizname FROM wizard ORDER BY wizname");
    if (!sizeof(r))
        return 0;
    return map(r,(: $1[0] :));
}

// ---------------------------------------------------------------------------
#define L_NOTIFIER_ABO ({NOTIFIER_WIZNAME,NOTIFIER_ABO_TYPE,\
    NOTIFIER_CFG_FREQUENCY,NOTIFIER_MAIL_FORMAT})
public <int|mapping*> get_all_wiz_abo(mapping options, int countflag)
{
    string q;
    mixed r;
    if (!check_security() || !mappingp(options))
        return 0;
    if (countflag)
    {
        q = "SELECT COUNT(*) FROM wizard_abo ";
    }
    else
    {
        q = "SELECT wizname,abo_type,frequency,mailformat FROM wizard_abo ";
    }
    if (stringp(options[NOTIFIER_WIZNAME]))
    {
        q+= "WHERE wizname = '"
            +db_escape_string(options[NOTIFIER_WIZNAME])+"' ";
    }
    else
    {
        return 0;
    }
    if (stringp(options[NOTIFIER_ABO_TYPE]))
    {
        q+= "AND abo_type = '"
            +db_escape_string(options[NOTIFIER_ABO_TYPE])+"' ";
    }
    if (!countflag)
    {
        q+= "ORDER BY wizname,abo_type ";
    }
    if (options[DB_DBG_LIMIT]>0)
    {
        q+= "LIMIT "+options[DB_DBG_LIMIT]+" OFFSET "+options[DB_DBG_OFFSET];
    }
    r = db_query_err(q);
    if (countflag)
    {
        return sizeof(r) ? get_one_int(r) : 0;
    }
    if (!sizeof(r))
        return ({});
    return map(r, (: mkmapping(L_NOTIFIER_ABO,$1) :));
    
}

public mapping get_one_wiz_abo(string wiz,string abo)
{
    if (!check_security() || !stringp(wiz) || !stringp(abo))
        return 0;
    mapping* m_abo = get_all_wiz_abo( ([
        NOTIFIER_WIZNAME : wiz,
        NOTIFIER_ABO_TYPE : abo
        ]),0 );
    if (!sizeof(m_abo)) 
        return 0;
    return m_abo[0];
}

public int set_one_wiz_abo(mapping m_abo)
{
    string q,*cols = ({});
    mixed r;
    mapping oldabo;
    if (!check_security() || !mappingp(m_abo))
        return 0; // error security or parameter
    oldabo = get_one_wiz_abo(m_abo[NOTIFIER_WIZNAME],m_abo[NOTIFIER_ABO_TYPE]);
    if (!mappingp(oldabo))
    {
        r = db_query_err("SELECT abo_type FROM abonnements "
            "WHERE abo_type = ?",m_abo[NOTIFIER_ABO_TYPE]);
        if (!wizplayerp(m_abo[NOTIFIER_WIZNAME]) ||!sizeof(r))
            return 0; // Fehler...
        db_query_err("INSERT INTO wizard_abo "
            "(wizname,abo_type,frequency,mailformat) VALUES (?,?,?,?) ",
            m_abo[NOTIFIER_WIZNAME],m_abo[NOTIFIER_ABO_TYPE],
            m_abo[NOTIFIER_CFG_FREQUENCY],m_abo[NOTIFIER_MAIL_FORMAT]);
        return 1; // insert
    }
    if (oldabo[NOTIFIER_CFG_FREQUENCY] != m_abo[NOTIFIER_CFG_FREQUENCY])
    {
        cols += ({"frequency = "+m_abo[NOTIFIER_CFG_FREQUENCY]+" " });
        // TODO mark_one_abo???
    }
    if (oldabo[NOTIFIER_MAIL_FORMAT] != m_abo[NOTIFIER_MAIL_FORMAT])
    {
        cols += ({"mailformat = "+m_abo[NOTIFIER_MAIL_FORMAT]+" " });
    }
    if (!sizeof(cols))
    {
        return 2; // no change
    }
    q = "UPDATE wizard_abo SET "+implode(cols,", ");
    q+= " WHERE wizname = ?1 AND abo_type = ?2";
    db_query_err(q,m_abo[NOTIFIER_WIZNAME],m_abo[NOTIFIER_ABO_TYPE]);
    return 3; // update done.
}

public <int|string *> get_free_abos(string wizard,int countflag)
{
    if (!check_security() || !stringp(wizard))
        return 0; // error security or parameter
    mixed r = db_query_err("SELECT abo_type FROM abonnements "
        "WHERE abo_type NOT IN (SELECT abo_type FROM wizard_abo "
        "WHERE wizname = ?1) ",wizard);
    if (!sizeof(r))
        return 0;
    return countflag ? sizeof(r) : map(r,(: $1[0] :) );
}
// ---------------------------------------------------------------------------
#define L_NOTIFIER_FDB_LIST ({NOTIFIER_WIZNAME,NOTIFIER_FDB_LISTID,\
    NOTIFIER_CFG_FREQUENCY,NOTIFIER_MAIL_FORMAT})
public <int|mapping*> get_all_wiz_fdb_lists(mapping options, int countflag)
{
    string q;
    mixed r;
    if (!check_security() || !mappingp(options))
        return 0;
    if (countflag)
    {
        q= "SELECT COUNT(*) FROM wiz2listid ";
    }
    else
    {
        q= "SELECT wiz,listid,frequency,mailformat FROM wiz2listid ";
    }
    if (stringp(options[NOTIFIER_WIZNAME]))
    {
        q+= "WHERE wiz = '"
            +db_escape_string(options[NOTIFIER_WIZNAME])+"' ";
    }
    else
    {
        return 0;
    }
    if (stringp(options[NOTIFIER_FDB_LISTID]))
    {
        q+= "AND listid = '"
            +db_escape_string(options[NOTIFIER_FDB_LISTID])+"' ";
    }
    if (!countflag)
    {
        q+= "ORDER BY wiz,listid ";
    }
    if (options[DB_DBG_LIMIT]>0)
    {
        q+= "LIMIT "+options[DB_DBG_LIMIT]+" OFFSET "+options[DB_DBG_OFFSET];
    }
    r = db_query_err(q);
    if (countflag)
    {
        return sizeof(r) ? get_one_int(r) : 0;
    }
    if (!sizeof(r))
        return ({});
    return map(r, (: mkmapping(L_NOTIFIER_FDB_LIST,$1) :));
}

public mapping get_one_wiz_fdb_list(string wiz,string listid)
{
    mapping * mlists;
    if (!check_security() || !stringp(wiz) || !stringp(listid))
        return 0;
    mlists = get_all_wiz_fdb_lists( ([
        NOTIFIER_WIZNAME : wiz, NOTIFIER_FDB_LISTID:listid,
        ]),0 );
    if (sizeof(mlists)!=1)
        return 0;
    return mlists[0];
}

public <string|string*> check_fdb_listid(string listid)
{
    return ERROR_ARCHIVE->check_listid(listid);
}

public int set_one_wiz_fdb_list(mapping m_fdblist)
{
    mapping oldlist;
    string *cols = ({});
    if (!check_security())
        return 0;
    m_fdblist ||= ([]);
    if (stringp(check_fdb_listid(m_fdblist[NOTIFIER_FDB_LISTID]))
        || !wizplayerp(m_fdblist[NOTIFIER_WIZNAME]) )
    {
        return 0; // Parameterfehler!!
    }
    oldlist = get_one_wiz_fdb_list(
        m_fdblist[NOTIFIER_WIZNAME],m_fdblist[NOTIFIER_FDB_LISTID]);
    if (!mappingp(oldlist))
    {
        db_query_err("INSERT INTO wiz2listid "
            "(listid,wiz,frequency,mailformat) VALUES (?,?,?,?)",
            m_fdblist[NOTIFIER_FDB_LISTID],
            m_fdblist[NOTIFIER_WIZNAME],
            m_fdblist[NOTIFIER_CFG_FREQUENCY],
            m_fdblist[NOTIFIER_MAIL_FORMAT]);
        return 1; // INSERT.
    }
    if (oldlist[NOTIFIER_CFG_FREQUENCY]!=m_fdblist[NOTIFIER_CFG_FREQUENCY])
    {
        cols += ({"frequency = "+m_fdblist[NOTIFIER_CFG_FREQUENCY] });
    }
    if (oldlist[NOTIFIER_MAIL_FORMAT]!=m_fdblist[NOTIFIER_MAIL_FORMAT])
    {
        cols += ({"mailformat = "+m_fdblist[NOTIFIER_MAIL_FORMAT] });
    }
    if (!sizeof(cols))
        return 2; // no action
    db_query_err("UPDATE wiz2listid SET "+implode(cols,", ")
        +" WHERE listid = ?1 AND wiz = ?2",
        m_fdblist[NOTIFIER_FDB_LISTID],
        m_fdblist[NOTIFIER_WIZNAME]);
    return 3; // Update.
}

// ---------------------------------------------------------------------------
#define L_NOTIFIER_ALL_EVENTS ({NOTIFIER_EVT_EVENTID,NOTIFIER_EVT_SUMMARY, \
    NOTIFIER_EVT_BODY,NOTIFIER_EVT_CREATED_ON,NOTIFIER_EVT_OBSOLETE_ON})
public <int|mapping*> get_all_events(mapping options, int countflag)
{
    string q;
    mixed r;
    int whereflag = 0;
    if (!check_security() || !mappingp(options))
        return 0;
    if (countflag)
    {
        q= "SELECT COUNT(*) FROM all_events ";
    }
    else
    {
        q=  "SELECT eventid,summary,body,created_on,obsolete_on "
            "FROM all_events ";
    }
    if (member(options,NOTIFIER_EVT_EVENTID))
    {
        q+= whereflag++ ? "AND " : "WHERE ";
        q+= "eventid = '"+db_escape_string(options[NOTIFIER_EVT_EVENTID])+"' ";
    }
    if (member(options,NOTIFIER_WIZNAME))
    {
        q+= whereflag++ ? "AND " : "WHERE ";
        q+= "eventid IN (SELECT eventid FROM wiz_events ";
        q+= "WHERE wizname = '"+db_escape_string(options[NOTIFIER_WIZNAME]);
        q+= "') ";
    }
    if (!countflag)
    {
        q+= "ORDER BY created_on DESC,eventid ASC ";
        if (options[DB_DBG_LIMIT]>0)
        {
            q+= "LIMIT "+options[DB_DBG_LIMIT]
                +" OFFSET "+options[DB_DBG_OFFSET];
        }
    }
    r = db_query_err(q);
    if (countflag)
    {
        return sizeof(r) ? get_one_int(r) : 0;
    }
    if (!sizeof(r))
        return ({});
    return map(r, (: mkmapping(L_NOTIFIER_ALL_EVENTS,$1) :));
}

public mapping get_one_event(string eventid)
{
    if (!check_security() || !stringp(eventid))
        return 0;
    mapping *m = get_all_events(([NOTIFIER_EVT_EVENTID:eventid]),0);
    return sizeof(m)==1?m[0]:0;
}

// ---------------------------------------------------------------------------
#define L_NOTIFIER_WIZ_EVENTS ({NOTIFIER_EVT_EVENTID,NOTIFIER_WIZNAME, \
    NOTIFIER_EVT_NOTIFIED_ON,NOTIFIER_EVT_SENT_ON})
public <int|mapping*> get_all_wiz_events(mapping options, int countflag)
{
    string q;
    mixed r;
    int whereflag = 0;
    if (!check_security() || !mappingp(options))
        return 0;
    if (countflag)
    {
        q= "SELECT COUNT(*) FROM wiz_events ";
    }
    else
    {
        q=  "SELECT eventid,wizname,notified_on,sent_on "
            "FROM wiz_events ";
    }
    if (member(options,NOTIFIER_EVT_EVENTID))
    {
        q+= whereflag++ ? "AND " : "WHERE ";
        q+= "eventid = '"+db_escape_string(options[NOTIFIER_EVT_EVENTID])+"' ";
    }
    if (member(options,NOTIFIER_WIZNAME))
    {
        q+= whereflag++ ? "AND " : "WHERE ";
        q+= "wizname = '"+db_escape_string(options[NOTIFIER_WIZNAME]);
        q+= "' ";
    }
    if (!countflag)
    {
        q+= "ORDER BY notified_on DESC,eventid ASC ";
        if (options[DB_DBG_LIMIT]>0)
        {
            q+= "LIMIT "+options[DB_DBG_LIMIT]
                +" OFFSET "+options[DB_DBG_OFFSET];
        }
    }
    r = db_query_err(q);
    if (countflag)
    {
        return sizeof(r) ? get_one_int(r) : 0;
    }
    if (!sizeof(r))
        return ({});
    return map(r, (: mkmapping(L_NOTIFIER_WIZ_EVENTS,$1) :));
}

// ---------------------------------------------------------------------------
private void inner_scheduler()
{
    while ( (m_scheduler[SCHEDULER_LAST_EVAL]=get_eval_cost())
             > SCHEDULER_MIN_EVAL)
    {
        PDBG("START-"+m_scheduler[SCHEDULER_PHASE]);
        switch (m_scheduler[SCHEDULER_PHASE])
        {
            case SCHEDULER_PH_INIT:
                m_scheduler[SCHEDULER_COMMIT_COUNT] = 0;
                m_scheduler[SCHEDULER_PHASE] = SCHEDULER_PH_ABO_F0;
                m_scheduler[SCHEDULER_NEXT_START] = find_next_start();
                m_scheduler[SCHEDULER_LAST_START] = time();
                continue;
            case SCHEDULER_PH_ABO_F0:
                if (process_abos(0)==SCHEDULER_MAX_ABOS)
                    continue;
                m_scheduler[SCHEDULER_PHASE] = SCHEDULER_PH_FDB_F0;
                continue;
            case SCHEDULER_PH_FDB_F0:
                if (process_fdb_events(0)==SCHEDULER_MAX_FDB)
                    continue;
                m_scheduler[SCHEDULER_PHASE] = SCHEDULER_PH_ABO_F1;
                continue;
            case SCHEDULER_PH_ABO_F1:
                if (process_abos(1)==SCHEDULER_MAX_ABOS)
                    continue;
                m_scheduler[SCHEDULER_PHASE] = SCHEDULER_PH_FDB_F1;
                continue;
            case SCHEDULER_PH_FDB_F1:
                if (process_fdb_events(1)==SCHEDULER_MAX_FDB)
                    continue;
                m_scheduler[SCHEDULER_PHASE] = SCHEDULER_PH_ABO_F2;
                continue;
            case SCHEDULER_PH_ABO_F2:
                if (process_abos(2)==SCHEDULER_MAX_ABOS)
                    continue;
                m_scheduler[SCHEDULER_PHASE] = SCHEDULER_PH_FDB_F2;
                continue;
            case SCHEDULER_PH_FDB_F2:
                if (process_fdb_events(2)==SCHEDULER_MAX_FDB)
                    continue;
                m_scheduler[SCHEDULER_PHASE] = SCHEDULER_PH_ABO_F3;
                continue;
            case SCHEDULER_PH_ABO_F3:
                if (process_abos(3)==SCHEDULER_MAX_ABOS)
                    continue;
                m_scheduler[SCHEDULER_PHASE] = SCHEDULER_PH_FDB_F3;
                continue;
            case SCHEDULER_PH_FDB_F3:
                if (process_fdb_events(3)==SCHEDULER_MAX_FDB)
                    continue;
                m_scheduler[SCHEDULER_PHASE] = SCHEDULER_PH_SEND_MAIL;
                continue;
            case SCHEDULER_PH_SEND_MAIL:
                if (send_mail_to(0)==SCHEDULER_MAX_MAIL)
                    continue;
                break;
        }
        m_scheduler[SCHEDULER_PHASE] = SCHEDULER_PH_INIT;
        if (m_scheduler[SCHEDULER_NEXT_START] < time())
        {
            // seit dem letzten Start sind mehr als 10 Minuten vergangen!
            m_scheduler[SCHEDULER_NEXT_START] = find_next_start();
            call_out("scheduler",SCHEDULER_SMALL_LOOP);
            PDBG("CONTINUE-SHORT2");
            return;
        }
        else
        {
            // ok, auf naechste 10 minuten timen...
            call_out("scheduler",m_scheduler[SCHEDULER_NEXT_START]-time());
            PDBG("CONTINUE-LONG");
            return;
        }
    }
    call_out("scheduler",SCHEDULER_SMALL_LOOP);
    PDBG("CONTINUE-SHORT1");
}

static void scheduler()
{
    string err;
    if (m_scheduler[SCHEDULER_ERROR_COUNT]>5)
        return; // Fehlerbehebung notwendig.
    if (err = catch(inner_scheduler();publish))
    {
        m_scheduler[SCHEDULER_LAST_ERR] = err;
        // TODO Analyser err auf TLE?
        m_scheduler[SCHEDULER_ERROR_COUNT]++;
    }
}

public int restart_scheduler()
{
    check_security(CHECK_ERROR);
    m_scheduler[SCHEDULER_ERROR_COUNT] = 0;
    m_scheduler[SCHEDULER_LAST_ERR] = "--";
    while (remove_call_out("scheduler")!=-1);
    scheduler();
}

public string* query_scheduler_info()
{
    string* lines = ({});
    lines += ({"Scheduler-Info vom "+shorttimestr(time()) });
    lines += ({"Nächster Callout: "+find_call_out("scheduler") });
    lines += ({"Letzter Start: " 
        +shorttimestr(m_scheduler[SCHEDULER_LAST_START])});
    lines += ({"Nächster Start: " 
        +shorttimestr(m_scheduler[SCHEDULER_NEXT_START])});
    lines += ({"Letzte Evals: "+m_scheduler[SCHEDULER_LAST_EVAL]});
    lines += ({"Phase: "+m_scheduler[SCHEDULER_PHASE]});
    lines += explode(wrap_say("Letzter Fehler:",
        m_scheduler[SCHEDULER_LAST_ERR])[..<2],"\n");
    lines += ({"Anzahl Fehler: "+m_scheduler[SCHEDULER_ERROR_COUNT]});
    lines += ({"Anzahl Commits: "+m_scheduler[SCHEDULER_COMMIT_COUNT]});
    return lines;
}


// ---------------------------------------------------------------------------

// Internes Kommando fuer Admins, um _ALLE_ sql-Kommandos abzusetzen.
// immer hilfreich um zu schauen und zu reparieren... *hust*
int sql(string sql)
{
    mixed * m;
    if (!check_security()  || !validate_debugger(TP_RN)) {
        FAILWP("Security failed.", FAIL_INTERNAL);
    }
    m = db_query(sql||"");
    
    if(!m) send_message_to(TP,MT_NOTIFY,MA_USE,
       wrap("Es gab keine Rückgabe."));
    else if(!pointerp(m)) send_message_to(TP,MT_NOTIFY,MA_USE,
       wrap(sprintf("Der Rückgabewert war: '%O'",m)));
    else if(sizeof(m)==0) send_message_to(TP,MT_NOTIFY,MA_USE,
       wrap("Die Tabelle ist (noch) leer."));
    else 
       TP->more (map(
  build_table ( transpose_array(map(m,(: map($1,(: to_string($1) :)) :))),
                      ({"|","-"})
                ), (: space($1) :))
       );
    if(query_db_error()) 
       send_message_to(TP,MT_NOTIFY,MA_USE,
          wrap("Es gab folgenden Fehler: "+ query_db_error()));
    return 1;
}

// Interne Kommandofunktion fuer Admins um die Defintionen von
// Tabellen und Views anzuschaun. 
int tablelist(string str)
{
    check_security(CHECK_ERROR);
    string tb = space(str);
    if (tb == "") {
        return sql("SELECT type,name FROM sqlite_master WHERE type IN "
                "('table','view') ORDER BY type,name");
    } else {
        return sql("SELECT sql FROM sqlite_master WHERE NAME = '"+tb+"'");
    }
}

void init_db()
{
    if (!init_debuglog(NOTIFIER_DB_SAVE, RETENTION_DAYS))
        raise_error("Datenbank konnte nicht geöffnet werden!");
    db_debug(0,0,DB_DBG_CREATE,0);
    db_query_err("CREATE TABLE IF NOT EXISTS all_events ("
            "eventid TEXT, "
            "summary TEXT, "
            "body TEXT, "
            "created_on INTEGER, "
            "obsolete_on INTEGER, "
            "PRIMARY KEY (eventid) )");
    db_query_err("CREATE TABLE IF NOT EXISTS wiz_events ("
            "eventid TEXT, "
            "wizname TEXT, "
            "notified_on INTEGER, "
            "sent_on INTEGER, "
            "PRIMARY KEY (eventid,wizname) )");
    db_query_err("CREATE TABLE IF NOT EXISTS errnum2listid ("
            "listid TEXT, "
            "errnum INTEGER, "
            "notified_on INTEGER, "
            "PRIMARY KEY (listid,errnum) )");
    db_query_err("CREATE TABLE IF NOT EXISTS wiz2listid ("
            "listid TEXT, "
            "wiz TEXT, "
            "frequency INTEGER, "
            "mailformat INTEGER, "
            "PRIMARY KEY (listid,wiz) )");
    db_query_err("CREATE TABLE IF NOT EXISTS wizard ("
            "wizname TEXT PRIMARY KEY, " // unique id.
            "daily_hour INTEGER, " // 00..23 fuer die Stunde.
            "daily_min INTEGER, " // 00..59 fuer die Minuten.
            "weekly_day INTEGER, " // 0..6 fuer Sonntag bis Samstag
            "weekly_hour INTEGER, " // 00..23 fuer die Stunde.
            "weekly_min INTEGER, " // 00..59 fuer die Minuten.
            "min_frequency INTEGER, "// 1 zeitnah,2 taeglich, 4woechentlich, 8mon
            "flags INTEGER  "    // Flags: NOTIFIER_WF_*
            ")");
    db_query_err("CREATE TABLE IF NOT EXISTS wizard_abo ("
            "wizname TEXT, " // unique id.
            "abo_type TEXT, " // abonnement-typ Armageddon,Pantheontreffen...
            "frequency INTEGER, " // 1 zeitnah,2 taeglich, 4woechentlich, 8mon
            "mailformat INTEGER, " // welches mailformat...
            "PRIMARY KEY (wizname,abo_type) "
            ")");
    db_query_err("CREATE TABLE IF NOT EXISTS abonnements ("
            "abo_type TEXT PRIMARY KEY " // abonnement-typ Armageddon,Pantheontreffen...
            ")");
    db_query_err("INSERT OR IGNORE INTO abonnements (abo_type) "
        "VALUES "
        "('Armageddon'), "
        "('Pantheontreffen'), "
        "('Notifier-News') ");
    db_query_err("CREATE TABLE IF NOT EXISTS time_done ("
            "dday TEXT, "       // wizname und monatstag als erledigungsflag
            "dtype INTEGER, "   // 1 zeitnah,2 taeglich, 4woechentlich, 8mon
            "PRIMARY KEY (dday,dtype) )");
}

void reset()
{
}

void create()
{
    reset();
    init_security_for_actions();
    // init_security_trust_mudlib();
    add_security_condition("/apps/error_archive");
    add_security_condition("/w/myonara/public/db/obj/zmail#");
    init_db();
    // update_debugger("myonara",27);
    check_for_armageddon();
}

void abort_renewal() {}
void finish_renewal(object neu) 
{
    neu->set_data(m_scheduler);
}
void prepare_renewal()
{
    db_close();
}
