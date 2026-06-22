// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /apps/error_archive.c
// Description: Offline Archiv aller Fehler zur Volltextsuche
// Author:      Myonara (05.Aug.2013)

// Userid fuer  /apps/error_archive setzen:
// UID: Apps

#pragma no_shadow
#pragma no_inherit
#pragma save_types

inherit "/i/tools/debuglog_db";
inherit "/i/tools/security";
inherit "/i/tools/build_table";

#include <apps.h>
#include <config.h>
#include <error_db.h>
#include <database.h>
#include <error.h>
#include <files.h>
#include <level.h>
#include <message.h>
#include <misc.h>
#include <more.h>
#include <notifier.h>
#include <rtlimits.h>
#include <security.h>

//#define DEBUGGER "myonara"
//#include <debug.h>
#undef DEBUG
#define DEBUG(x) ERROR_DB->debug_msg(x)

#define LINE      "-------------------------------------------------------------------------------"
#define MORE_LINE "...--------------------------------------------------------------------- (MORE)"
#define DELAY_EVENT_LOOP 7
#define EVALS_EVENT_LOOP 800000
static void process_event_loop();
public varargs int* query_errnums_for_hashtags(string *hashtags,
    int limit,int offset);

private string last_error = 0;

public string query_database_description()
{
    return "Diese Datenbank dient zur Archivierung der Fehlerdatenbank. "
    "Zum Anschauen stellt der Zaubertab das zfe-Option -A\"<suchmuster>\" "
    "zur Verfügung. Für Admins gibt es speziell die Lfuns sql und "
    "db_get_table.";
}

/*
NOENZY: flatten_array
DEKLARATION: private mixed flatten_array(mixed arr)
BESCHREIBUNG:
Aus /p/Misc/i/array kopiert: verflacht ein Array.
VERWEISE: 
GRUPPEN: protokoll
*/
private mixed flatten_array(mixed arr)
{
    mixed tmp, erg;
    int i, size;

    if (!pointerp(arr)) {
        do_error("flatten_array: Parameter 1 muss ein Array sein.\n");
        return 0;
    }

    tmp = ({}) + arr;
    erg = ({});
    while (sizeof(tmp)) {
        if (pointerp(tmp[0])) {
            tmp[0..0] = tmp[0];
        } else {
            i = 1;
            size = sizeof(tmp);
            while (i<size && !pointerp(tmp[i])) {
                i++;
            }
            erg += tmp[0..(i-1)];
            tmp = tmp[i..<1];
        }
    }
    return erg;
}

// ------------------------------------------------------------------------

/*
NOENZY: add_error
DEKLARATION: public int add_error(int errnum, string errtext, int time_started, int time_changed)
BESCHREIBUNG:
Fuegt einen Fehler inkl seinem Fehlertext und start und change-Zeitstempel
der Datenbank hinzu.
VERWEISE: get_errtext
GRUPPEN: protokoll
*/
public int add_error(int errnum, string errtext, 
                     int time_started, int time_changed, int flagvis)
{
    mixed * result;
    if (object_name(PO) != ERROR_DB) return 0;
    if (errnum <= 0 || !stringp(errtext) || time_started <= 0
            || time_started > time_changed)
        return 0;
    result = db_query("SELECT errnum FROM earchive_1 WHERE errnum = ?",
                      errnum);
    if (query_db_error() || sizeof(result)==0) 
    {
        db_query("INSERT INTO earchive_1 "
                 "(errnum, opened, changed, errtext, flagvis)"
                 " VALUES (?, ?, ?, ?, ?)",
                 errnum,time_started,time_changed,errtext,flagvis);
        return query_db_error() == 0;
    }
    else
    {
        db_query("UPDATE earchive_1 "
                 "SET opened = ?,changed = ?, errtext = ?, flagvis = ? "
                 "WHERE errnum = ? ",
                 time_started,time_changed,errtext, flagvis,errnum);
        return query_db_error() == 0;
    }
}

/*
NOENZY: get_errtext
DEKLARATION: public string get_errtext(int errnum)
BESCHREIBUNG:
Liefert den Fehlertext zur Fehlernummer, sofern vorhanden.
VERWEISE: add_error
GRUPPEN: protokoll
*/
public string get_errtext(int errnum)
{
    mixed * result;
    if (!check_security() || errnum <= 0) return 0;
    if (adminp(TP))
    {
        result = db_query("SELECT errtext FROM earchive_1 WHERE errnum = ?",
                      errnum);
    }
    else
    {
        result = db_query("SELECT errtext FROM earchive_1 "
                      "WHERE flagvis = 0 AND errnum = ?",
                      errnum);
    }
    if (pointerp(result) && sizeof(result)==1)
    {
        return get_one_string(result);
    }
    return 0; // nicht gefunden.
}


/*
NOENZY: get_timestamp
DEKLARATION: public int get_timestamps(int type)
BESCHREIBUNG:
Liefert die Zeitstenpel zur Fehlernummer, sofern vorhanden. 0 sonst.
({ open,last_changed,closed})
VERWEISE: add_error
GRUPPEN: protokoll
*/
public int * get_timestamps(int errnum)
{
    mixed * result;
    if (!check_security() || errnum <= 0) return 0;
    if (adminp(TP))
    {
        result = db_query("SELECT opened, changed, closed "
                      "FROM earchive_1 WHERE errnum = ?",
                      errnum);
    }
    else 
    {
        result = db_query("SELECT opened, changed, closed "
                      "FROM earchive_1 WHERE errnum = ? and flagvis = 0",
                      errnum);
    }
    if (pointerp(result) && sizeof(result)==1)
    {
        return result[0];
    }
    return 0; // nicht gefunden.
}

/*
NOENZY: get_errorlist
DEKLARATION: public mixed * get_errorlist(int start, int lines, mapping options)
BESCHREIBUNG:
Liefert ab Zeile "start" nur Anzahl "lines" Zeilen abhaengig von den 
Optionen. Folgende Optionen haben Einfluss auf das Ergebnis:
EDB_OPT_ARCHIVE_PATTERN: Das Datenbank/Volltext-Suchpattern mit den 
                         Wildcards _(ein beliebiges Zeichen
                         und % (null bis beliebig viele beliebige Zeichen).
EDB_OPT_CREATED_SINCE:   Nur Fehler die an oder nach diesen Zeitstempel
                         erstellt wurden.
EDB_OPT_CREATED_BEFORE:  Nur Fehler die an oder vor diesen Zeitstempel
                         erstellt wurden.
EDB_OPT_NEWER_SINCE:     Nur Fehler die an oder nach diesen Zeitstempel
                         geaendert wurden.
EDB_OPT_NEWER_BEFORE:    Nur Fehler die an oder vor diesen Zeitstempel
                         geaendert wurden.
EDB_OPT_ARCHIVE_INVIS:   Nur unsichtbare Archivfehler anzeigen (Admin-Only)
EDB_OPT_SORT_OPEN_ASC:   Die Ausgabe nach Erstelldatum aufsteigend sortieren
EDB_OPT_SORT_OPEN_DESC:  Die Ausgabe nach Erstelldatum absteigend sortieren
EDB_OPT_SORT_CHANGE_ASC: Die Ausgabe nach Aenderungsdatum aufsteigend 
                         sortieren
EDB_OPT_SORT_CHANGE_DESC:Die Ausgabe nach Aenderungsdatum absteigend 
                         sortieren
Die zurueckgegebenen Zeilen haben im Erfolgsfall das Format
({ ({ errnum0, opened0, changed0 }), ({ errnum1, opened1, changed1 }), ...})
VERWEISE: add_error
GRUPPEN: protokoll
*/
public mixed get_errorlist(int start, int lines, mapping options)
{
    mixed *result, *args = ({});
    string q, pattern;
    int flag_where = 0;

    if (!check_security()) return 0;

    options ||= ([]);
    pattern = options[EDB_OPT_ARCHIVE_PATTERN];

    if (stringp(pattern) && sizeof(pattern - " ")>0 )
    {
        flag_where = 1;

        if (lines <= 0)
            q = "SELECT COUNT(rowid)";
        else
            q = "SELECT rowid AS errnum, opened, changed";
        q += " FROM earchive_fts WHERE earchive_fts MATCH ? ";
        args += ({ "\"" + space(pattern, 0, "\" AND \"") + "\"" });
    }
    else
    {
        if (lines <= 0)
            q = "SELECT COUNT(errnum) FROM earchive_1 ";
        else
            q = "SELECT errnum, opened, changed FROM earchive_1 ";
    }

    if (member(options,EDB_OPT_ERRNUMS))
    {
        int *errnums = options[EDB_OPT_ERRNUMS];

        q+= ( flag_where ? "AND " : "WHERE " );
        flag_where = 1;

        q+= "errnum IN (" + ("?," * sizeof(errnums))[..<2] + ") ";
        args += errnums;
    }
    if (member(options,EDB_OPT_HASHTAGS))
    {
        string *hashtags = options[EDB_OPT_HASHTAGS];

        q+= ( flag_where ? "AND " : "WHERE " );
        flag_where = 1;

        q+= "errnum IN (SELECT DISTINCT errnum FROM hashtags "
            "WHERE hashtag IN (" + ("?," * sizeof(hashtags))[..<2] + ") ";
        args += hashtags;
    }
    if (member(options,EDB_OPT_CREATED_SINCE))
    {
        q+= ( flag_where ? "AND " : "WHERE " );
        flag_where = 1;

        q+= "opened >= ? ";
        args += ({ options[EDB_OPT_CREATED_SINCE] });
    }
    if (member(options,EDB_OPT_CREATED_BEFORE))
    {
        q+= ( flag_where ? "AND " : "WHERE " );
        flag_where = 1;

        q+= "opened <= ? ";
        args += ({ options[EDB_OPT_CREATED_BEFORE] });
    }
    if (member(options,EDB_OPT_NEWER_SINCE))
    {
        q+= ( flag_where ? "AND " : "WHERE " );
        flag_where = 1;

        q+= "changed >= ? ";
        args += ({ options[EDB_OPT_NEWER_SINCE] });
    }
    if (member(options,EDB_OPT_NEWER_BEFORE))
    {
        q+= ( flag_where ? "AND " : "WHERE " );
        flag_where = 1;

        q+= "changed <= ? ";
        args += ({ options[EDB_OPT_NEWER_BEFORE] });
    }
    if (member(options,EDB_OPT_ARCHIVE_INVIS) && adminp(TP))
    {
        q+= ( flag_where ? "AND " : "WHERE " );
        flag_where = 1;
        q+= "flagvis = 1 ";
    }
    else
    {
        q+= ( flag_where ? "AND " : "WHERE " );
        flag_where = 1;
        q+= "flagvis = 0 ";
    }
    if (lines > 0)
    {
        if (member(options,EDB_OPT_SORT_OPEN_ASC))
            q+= "ORDER BY opened ASC, errnum ASC ";
        else if (member(options,EDB_OPT_SORT_OPEN_DESC))
            q+= "ORDER BY opened DESC, errnum DESC ";
        else if (member(options,EDB_OPT_SORT_CHANGE_ASC))
            q+= "ORDER BY changed ASC, errnum ASC ";
        else if (member(options,EDB_OPT_SORT_CHANGE_DESC))
            q+= "ORDER BY changed DESC, errnum DESC ";

        q += sprintf("LIMIT %d OFFSET %d", lines, start);
        result = db_query(q, args...);
    }
    else
    {
        result = db_query(q, args...);
        return get_one_int(result);
    }
    DEBUG(sprintf("%d,%d: %Q\n%Q\n%Q",start,lines,options,q,result));
    return result;
}

// ------------------------------------------------------------------------

public int enter_hashtag(int errnum,string hashtag,string wizname)
{
    mixed result;
    if (!check_security()) 
        return 0;
    if (errnum <=0 || !stringp(hashtag) || strstr(hashtag," ")!=-1
            || lower_case(hashtag) != hashtag
            || !player_exists(wizname) || !wizplayerp(wizname) )
        return 0;
    result = db_query_err("SELECT 1 FROM hashtags "
        "WHERE errnum = ?1 AND hashtag = ?2",errnum,hashtag);
    if (sizeof(result)==1)
        return 1; // done.
    db_begin();
    debuglog(hashtag+"="+errnum,DB_DBGLVL_INFO,wizname,"enter_hashtag");
    db_query_err("INSERT INTO hashtags (errnum,hashtag) VALUES (?,?) ",
        errnum, hashtag);
    db_commit();
    return 2;
}

public int enter_hashtags(int errnum,string *hashtags,string wizname)
{
    if (!check_security()) 
        return 0;
    if (!sizeof(hashtags))
        return 0;
    foreach (string tag : hashtags)
    {
        if (!enter_hashtag(errnum,tag,wizname))
            return 0;
    }
    return 1;
}

// ------------------------------------------------------------------------

public int delete_hashtag(int errnum,string hashtag,string wizname)
{
    string q;
    if (!check_security()) 
        return 0;
    if (!player_exists(wizname) || !wizplayerp(wizname) )
        return 0;
    if (errnum <= 0 && stringp(hashtag))
    {
        q = "DELETE FROM hashtags WHERE hashtag = ?1 ";
    }
    else if (errnum > 0 && !stringp(hashtag))
    {
        q = "DELETE FROM hashtags WHERE errnum = ?2 ";
    }
    else if (errnum > 0 && stringp(hashtag))
    {
        q = "DELETE FROM hashtags WHERE hashtag = ?1 AND errnum = ?2 ";
    }
    else
    {
        return 0;
    }
    db_begin();
    debuglog((hashtag||"(0)")+"="+errnum,DB_DBGLVL_INFO,wizname,
        "delete_hashtags");
    db_query_err(q,hashtag,errnum);
    db_commit();
    return 1;
}

public int delete_hashtags(int errnum,string* hashtags,string wizname)
{
    if (!check_security()) 
        return 0;
    if (!sizeof(hashtags))
        return 0;
    foreach (string tag : hashtags)
    {
        if (!delete_hashtag(errnum,tag,wizname))
            return 0;
    }
    return 1;
}

// ------------------------------------------------------------------------

public int rename_hashtag(string old, string new,string wizname)
{
    if (!check_security()) 
        return 0;
    if (!stringp(old) || strstr(old," ")!=-1 
            || !stringp(new) || strstr(new," ")!=-1
            || lower_case(new) != new
            || !player_exists(wizname) || !wizplayerp(wizname) )
        return 0;
    db_begin();
    debuglog(old+"=>"+new,DB_DBGLVL_INFO,wizname,"rename_hashtag");
    db_query_err("DELETE FROM hashtags WHERE hashtag = ?1 "
        "AND errnum IN (SELECT errnum FROM hashtags WHERE hashtag = ?2) "
        "AND errnum IN (SELECT errnum FROM hashtags WHERE hashtag = ?1) ",
        new,old); // neue dubletten erstmal eliminieren
    db_query_err("UPDATE hashtags SET hashtag = ?1 WHERE hashtag = ?2",
        new,old);// und umtragen.
    db_commit();
    return 1;
}

// ------------------------------------------------------------------------

public int query_hashtag_count(string hashtag)
{
    if (!stringp(hashtag) || space(hashtag)=="")
        return 0;
    mixed result = db_query_err("SELECT COUNT(errnum) FROM hashtags "
        "WHERE hashtag = ?1 GROUP BY hashtag ",hashtag);
    return sizeof(result) ? get_one_int(result) : 0;
}

// ------------------------------------------------------------------------

public int* query_errnums_for_hashtag(string hashtag,int limit,int offset)
{
    if (!stringp(hashtag) || space(hashtag)=="")
        return 0;
    mixed result = db_query_err("SELECT errnum FROM hashtags "
        "WHERE hashtag = ?1 LIMIT ?2 OFFSET ?3 ",hashtag,limit,offset);
    if (!sizeof(result))
        return ({});
    return map(result,(: $1[0] :) );
}

public varargs int* query_errnums_for_hashtags(
        string *hashtags,int limit,int offset)
{
    string q;
    mixed *args=({});
    if (!sizeof(hashtags))
        return ({});
    q = "SELECT DISTINCT errnum FROM hashtags WHERE hashtag IN (";
    q+= implode( ({"?"})*sizeof(hashtags),",")+") ";
    args = hashtags;
    if (limit>0)
    {
        q+= "LIMIT ? OFFSET ? ";
        args+= ({limit,offset});
    }
    mixed result = db_query_err(q,args...);
    if (!sizeof(result))
        return ({});
    return map(result,(: $1[0] :) );
}


// ------------------------------------------------------------------------

public string* query_hashtags_for_errnum(int errnum,int limit, int offset)
{
    if(errnum <= 0)
        return 0;
    mixed result = db_query_err("SELECT hashtag FROM hashtags "
        "WHERE errnum = ?1 LIMIT ?2 OFFSET ?3 ",errnum,limit,offset);
    if (!sizeof(result))
        return ({});
    return map(result,(: $1[0] :) );
}

// ------------------------------------------------------------------------

public varargs <int|string*> query_all_hashtags(
            mapping options,int limit,int offset)
{
    string q;
    options ||= ([]);
    mixed *args=({});
    if (limit <= 0)
    {
        q = "SELECT COUNT(DISTINCT hashtag) FROM hashtags WHERE 1=1 ";
    }
    else
    {
        q = "SELECT hashtag,COUNT(errnum) as cnt FROM hashtags WHERE 1=1 ";
    }
    if (pointerp(options[EDB_OPT_HASHTAGS]) 
       && sizeof(options[EDB_OPT_HASHTAGS]))
    {
        q+= "AND hashtag IN ("
            +implode(({"?"})*sizeof(options[EDB_OPT_HASHTAGS]),",")+") ";
        args += options[EDB_OPT_HASHTAGS];
    }
    if (limit > 0)
    {
        q+= "GROUP BY hashtag ORDER BY hashtag ";
        q+= "LIMIT ? OFFSET ? ";
        args += ({limit,offset});
    }
    mixed result = db_query_err(q,args...);
    if (limit <= 0)
        return get_one_int(result);
    return map(result||({}), (: sprintf("%s (%d)",$1[0],$1[1]) :));
}

// ------------------------------------------------------------------------

public string* debug_list_actions(int action_type)
{
    string* lines = ({});
    if (action_type & EDB_LA_INPUT_ACTION)
        lines += ({"FDB-Automatik"});
    if (action_type & EDB_LA_IN_LIST_ACTION)
        lines += ({"FDB-Manuell"});
    if (action_type & EDB_LA_ON_NEW)
        lines += ({"Trigger Neu"});
    if (action_type & EDB_LA_ON_DBG_CHANGE)
        lines += ({"Trigger Dbg Change"});
    if (action_type & EDB_LA_ON_CHANGE)
        lines += ({"Trigger Change"});
    if (action_type & EDB_LA_ON_DELETE)
        lines += ({"Trigger Delete"});
    if (action_type & EDB_LA_ON_READ)
        lines += ({"Trigger Read"});
    if (action_type & EDB_LA_ON_READ_OWN)
        lines += ({"Trigger Read Own"});
    if (action_type & EDB_LA_ON_READ_ARCHIVE)
        lines += ({"Trigger Read Archive"});
    if (action_type & EDB_LA_REMOVE)
        lines += ({"Remove"});
    if (action_type & EDB_LA_FORWARD)
        lines += ({"Forward"});
    if (action_type & EDB_LA_MAIL)
        lines += ({"Mail"});
    if (action_type & EDB_LA_NEW)
        lines += ({"New"});
    if (action_type & EDB_LA_FORWARD_OTHERS)
        lines += ({"Forward Others"});
    return lines;
}

public <string*|string> check_listid(string listid)
{
    if (!sizeof(regexp(({listid||""}),"^[A-Za-z0-9_]+\@[A-Za-z0-9_:]+$")))
        return "Falsche list-id!";
    string *split = explode(listid,"@");
    if (lower_case(split[1])==split[1]) // persoenlicher Gottbereich
    {
        if (split[1] != TP_RN && !adminp(TP))
            return "Kein Recht für persönliche Liste eines anderen.";
        if (adminp(TP) && !wizplayerp(split[1]))
            return "Kein gültiger Gottname.";
    }
    else // eine acl-Gruppe?
    {
        if (GROUP_MASTER->query_group_info(split[1])
                =="Keine solche Gruppe vorhanden.\n")
            return "Keine Gruppe \""+split[1]+"\" vorhanden.";
        if (!GROUP_MASTER->is_group_member(TP_RN,split[1]) && !adminp(TP))
            return "Kein Gruppenmitglied von \""+split[1]+"\",";
    }
    return split;
}

public <string|string*> check_write_access_to_listid(string listid)
{
    if (!sizeof(regexp(({listid||""}),"^[A-Za-z0-9_]+\@[A-Za-z0-9_:]+$")))
        return "Falsche list-id!";
    string *split = explode(listid,"@");
    mixed r;
    r = db_query_err("SELECT 1 FROM list_filters "
        "WHERE listid = ?1 AND filter_type = ?2 AND filter_group = ?3 ",
        listid,EDB_LFT_LIST_WRITERS,TP_RN);
    if (sizeof(r))
        return split; // Freifahrtsschein.
    if (lower_case(split[1])==split[1]) // persoenlicher Gottbereich
    {
        if (split[1] != TP_RN && !adminp(TP))
            return "Kein Recht für persönliche Liste eines anderen.";
        if (adminp(TP) && !wizplayerp(split[1]))
            return "Kein gültiger Gottname.";
    }
    else // eine acl-Gruppe?
    {
        if (GROUP_MASTER->query_group_info(split[1])
                =="Keine solche Gruppe vorhanden.\n")
            return "Keine Gruppe \""+split[1]+"\" vorhanden.";
        if (!GROUP_MASTER->is_group_member(TP_RN,split[1]) && !adminp(TP))
            return "Kein Gruppenmitglied von \""+split[1]+"\",";
    }
    return split;
}

public string create_extended_list(string listid)
{
    mixed result;
    <string|string*> split;
    if (!check_security()) 
        return "Zugriffsproblem.";
    if (!stringp(listid) || listid == "")
        return "Falsche list-id.";
    result = db_query_err("SELECT 1 FROM list_header WHERE listid = ?",
        listid);
    if (result && sizeof(result))
    {
        return ""; // ok: Liste schon vorhanden.
    }
    split = check_listid(listid);
    if (stringp(split))
        return split;
    db_query_err("INSERT INTO list_header (listid,error_types) "
        "VALUES (?1,?2)",listid,EDB_TMASK_ALL);
    return ""; // ok.
}

private int add_errnum_to_extended_list(int errnum,string listid)
{
    mixed result = db_query_err("SELECT 1 FROM list_header WHERE listid = ?",
        listid);
    if (!result || !sizeof(result))
    {
        return 0;
    }
    db_query_err("INSERT OR IGNORE INTO list2errnum(listid,errnum) VALUES (?,?)",
            listid,errnum);
    return 1;
}

public int insert_errnum_as_manual_event(int* errnums,string* listids,
    string wiz)
{
    int ix,jx,cnt=0;
    mixed r;
    if (!check_security() || !wizplayerp(wiz||""))
        return 0;
    db_begin();
    for (jx=0;jx<sizeof(listids||({}));jx++)
    {
        r = db_query_err("SELECT 1 FROM list_header WHERE listid = ?",
            listids[jx]);
        if (!sizeof(r))
        {
            continue; // ueberspringen.
        }
        if (stringp(check_write_access_to_listid(listids[jx])))
        {
            continue; // nur listids mit Zugriff.
        }
        for (ix = 0;ix < sizeof(errnums||({}));ix++)
        {
            db_query_err("INSERT OR IGNORE INTO list2errnum (listid,errnum) "
                "VALUES (?,?)",listids[jx],errnums[ix]);
            db_query_err("INSERT INTO list_events "
                "(event_time,event_user,errnum,listid,action_type) "
                "VALUES (?,?,?,?,?)",
                time(),wiz,errnums[ix],listids[jx],
                EDB_LA_IN_LIST_ACTION | EDB_LA_ON_NEW);
            cnt++;
        }
    }
    db_commit();
    if (cnt>0 && find_call_out("process_event_loop")==-1)
    {
        call_out("process_event_loop",DELAY_EVENT_LOOP);
    }
    return cnt+1;
}

public int delete_errnum_as_manual_event(int* errnums,string* listids,
    string wiz)
{
    int ix,jx,cnt=0;
    mixed r;
    if (!check_security() || !wizplayerp(wiz||""))
        return 0;
    db_begin();
    for (jx=0;jx<sizeof(listids||({}));jx++)
    {
        r = db_query_err("SELECT 1 FROM list_header WHERE listid = ?",
            listids[jx]);
        if (!sizeof(r))
        {
            continue; // ueberspringen.
        }
        if (stringp(check_write_access_to_listid(listids[jx])))
        {
            continue; // nur listids mit Zugriff.
        }
        for (ix = 0;ix < sizeof(errnums||({}));ix++)
        {
            // db_query_err("DELETE FROM list2errnum "
                // "WHERE listid = ? AND errnum = ?",
                // listids[jx],errnums[ix]);
            db_query_err("INSERT INTO list_events "
                "(event_time,event_user,errnum,listid,action_type) "
                "VALUES (?,?,?,?,?)",
                time(),wiz,errnums[ix],listids[jx],
                EDB_LA_IN_LIST_ACTION | EDB_LA_ON_DELETE);
            cnt++;
        }
    }
    db_commit();
    if (cnt>0 && find_call_out("process_event_loop")==-1)
    {
        call_out("process_event_loop",DELAY_EVENT_LOOP);
    }
    return cnt+1;
}

public string get_listids_for_errnum(int errnum)
{
    mixed r;
    string result;
    if (!check_security())
        return 0;
    r = db_query_err("SELECT listid FROM list2errnum WHERE errnum = ?",
        errnum);
    if (!sizeof(r))
    {
        result = "Fehlernr. "+errnum+" in keiner Liste.\n";
    }
    else
    {
        result = wrap_say("Fehlernr. "+errnum+" in den Listen:",
            implode(map(r,(:$1[0]:)),", "));
    }
    r = db_query_err("SELECT listid FROM list_header "
        "WHERE listid NOT IN (SELECT DISTINCT listid FROM list2errnum "
        "WHERE errnum = ?) ",errnum);
    if (!sizeof(r))
    {
        result += "Keine weiteren Listen verfügbar.\n";
    }
    else
    {
        result += wrap_say("Verfügbare Listen:",
            implode(map(r,(:$1[0]:)),", "));
    }
    return result;
}

private int _insert_event_on_fdb_errnum(int errnum,string* debugger,int acttype,
        string wiz,int error_type)
{
    string dberr;
    if (!pointerp(debugger) || !sizeof(debugger))
        debugger = ({});
    acttype &= EDB_LA_TRIGGERS;
    if (!acttype || errnum < 0) 
        return 0;
    if (acttype & EDB_LA_ON_READ_ARCHIVE 
        && ERROR_DB->query_error_header(errnum)!=0)
    {
        return 1; // Kein event erzeugen!
    }
    // db_query_err("DELETE FROM list_events "
        // "WHERE event_time = ?1 AND errnum = ?2",
        // time(),errnum);
    db_query("INSERT INTO list_events "
        "(event_time,event_user,errnum,debugger,action_type,error_type) "
        "VALUES (?,?,?,?,?,?)",
        time(),wiz||"-",errnum,implode(debugger,","),
        EDB_LA_INPUT_ACTION | acttype,error_type);
    dberr = query_db_error();
    if (dberr)
    {
        sys_log("ERROR_DB",sprintf("%d: INSERT_list_events errnum %d:\n%s\n",
            time(),errnum,dberr));
        return 0;
    }
    if (find_call_out("process_event_loop")==-1)
        call_out("process_event_loop",DELAY_EVENT_LOOP);
    return 1;
}

public int insert_event_on_fdb_errnum(int errnum,string* debugger,int acttype,
        string wiz,int error_type)
{
    if (!check_security())
        return 0;
    // return _insert_event_on_fdb_errnum(errnum,debugger,acttype,wiz,error_type);
    return limited(#'_insert_event_on_fdb_errnum,
        LIMIT_EVAL,100000,
        LIMIT_MAPPING_SIZE,LIMIT_UNLIMITED,
            LIMIT_MAPPING_KEYS,LIMIT_UNLIMITED,
        LIMIT_ARRAY,LIMIT_UNLIMITED);
}

public int insert_listaction(string listid,int action_type,string param)
{
    if (!check_security()) 
        return 0;
    mixed result = db_query_err("SELECT 1 FROM list_header WHERE listid = ?",
        listid);
    if (!result || !sizeof(result))
    {
        return 0;
    }
    db_query_err("INSERT INTO listactions "
        "(listid,action_type,action_parameter) VALUES (?,?,?)",
        listid,action_type,param||"");
    result = db_query_err("SELECT MAX(actionid) FROM listactions");
    return result && sizeof(result)==1 && get_one_int(result);
}

public int check_list_transfer(string id_source,string id_target)
{
    if (!stringp(id_source)||!stringp(id_target))
        return 0;
    //mixed result = db_query_err("SELECT writers FROM list_header "
    //    "WHERE listid = ?",id_target);
    mixed result = db_query_err("SELECT filter_group FROM list_filters "
        "WHERE listid = ?1 AND filter_type = ?2",
        id_target,EDB_LFT_LIST_WRITERS);
    if (!result || !sizeof(result))
        return 0;
    string* writers = explode(result[0][0]||"",",");
    if (member(writers,id_source)==-1) 
    {
        // TODO internal message: no write access!
        return 0;
    }
    return 1;
}

private int forward_errnum_to_targets(string source_id,int errnum,int fwd)
{
    int cnt = 0;
    mixed targets;
    if (fwd == EDB_LA_FORWARD_OTHERS)
    {
        targets = db_query_err("SELECT filter_group "
            "FROM list_filters WHERE listid = ?1 AND filter_type = ?2 "
            "AND filter_group NOT LIKE ?3 ",
            source_id||"",EDB_LFT_TARGET_LISTIDS,"%@"+TP_RN);
    }
    else
    {
        targets = db_query_err("SELECT filter_group "
            "FROM list_filters WHERE listid = ?1 AND filter_type = ?2 ",
            source_id||"",EDB_LFT_TARGET_LISTIDS);
    }
    if (!sizeof(targets))
        return 0; // done.
    db_begin();
    foreach (mixed target: targets)
    {
        if (check_list_transfer(source_id,target[0]))
        {
            cnt++;
            db_query_err("INSERT OR IGNORE INTO list2errnum(listid,errnum) "
                "VALUES (?,?)", target[0],errnum);
        }
    }
    db_commit();
    return cnt;
}
#define MY_EVENTS_SEQ           0
#define MY_EVENTS_TIME          1
#define MY_EVENTS_USER          2
#define MY_EVENTS_ERRNUM        3
#define MY_EVENTS_LISTID        4
#define MY_EVENTS_DEBUGGER      5
#define MY_EVENTS_ACTION_TYPE   6
#define MY_EVENTS_ERROR_TYPE    7
#define MY_ACTIONS_ID       0
#define MY_ACTIONS_LISTID   1
#define MY_ACTIONS_TYPE     2
#define MY_ACTIONS_PARAM    3
private varargs void execute_actions(mixed event,mixed actions,int cnt,int la)
{
    <string|string*> split;
    foreach (mixed action : actions||({}))
    {
        split = check_write_access_to_listid(action[MY_ACTIONS_LISTID]);
        debuglog(sprintf("action %x split %Q",
            event[MY_EVENTS_ACTION_TYPE],split),
            DB_DBGLVL_DEBUG,event[MY_EVENTS_USER],"execute_actions");
        if (stringp(split)) 
            continue;
        if (event[MY_EVENTS_ACTION_TYPE] & EDB_LA_ON_READ_OWN)
        {
            if (split[1]!=event[MY_EVENTS_USER])
                continue;
        }
        cnt++;
        if ((!la || (la & EDB_LA_NEW)>0)
            && action[MY_ACTIONS_TYPE]&EDB_LA_NEW)
        {
            add_errnum_to_extended_list(event[MY_EVENTS_ERRNUM],
                action[MY_ACTIONS_LISTID]);
        }
        if (action[MY_ACTIONS_TYPE]&EDB_LA_MAIL)
        {
            NOTIFIER_MASTER->register_fdb_errnum(
                action[MY_ACTIONS_LISTID],event[MY_EVENTS_ERRNUM]);
        }
        if (action[MY_ACTIONS_TYPE]&EDB_LA_FORWARD_OTHERS)
        {
            forward_errnum_to_targets(action[MY_ACTIONS_LISTID],
                event[MY_EVENTS_ERRNUM],EDB_LA_FORWARD_OTHERS);
        }
        else if (action[MY_ACTIONS_TYPE]&EDB_LA_FORWARD)
        {
            forward_errnum_to_targets(action[MY_ACTIONS_LISTID],
                event[MY_EVENTS_ERRNUM],EDB_LA_FORWARD);
        }
        if ((!la || (la & EDB_LA_REMOVE)>0)
            && action[MY_ACTIONS_TYPE]&EDB_LA_REMOVE)
        {
            db_query_err("DELETE FROM list2errnum "
                "WHERE errnum = ?1 AND listid = ?2",
                event[MY_EVENTS_ERRNUM],
                action[MY_ACTIONS_LISTID]);
        }
    }

}

private int process_one_event()
{
    mixed actions;
    mixed event = db_query_err("SELECT seq,event_time,event_user,errnum, "
        "listid,debugger,action_type, error_type "
        "FROM list_events ORDER BY seq ASC LIMIT 1 ");
    string *debuggers;
    int cnt_actions;
    if (event && sizeof(event)==1)
    {
        event = event[0];
    }
    else
    {
        return 0;
    }
    cnt_actions = 0;
    debuglog(sprintf("action %x",event[MY_EVENTS_ACTION_TYPE]),
        DB_DBGLVL_DEBUG,event[MY_EVENTS_USER],"process_one_event");
    switch (event[MY_EVENTS_ACTION_TYPE] & EDB_LA_SOURCES)
    {
        case EDB_LA_INPUT_ACTION:
            debuggers = explode(event[MY_EVENTS_DEBUGGER]||"",",");
            db_query_err("DROP TABLE IF EXISTS actdbg");
            db_query_err("CREATE TEMPORARY TABLE IF NOT EXISTS actdbg ("
                "debugger TEXT PRIMARY KEY)");
            db_query_err("INSERT INTO actdbg (debugger) VALUES ("
                +implode(({"?"})*sizeof(debuggers),",")+") ",debuggers...);
            if ( (event[MY_EVENTS_ACTION_TYPE] & EDB_LA_ON_NEW) !=0)
            {
                // Checkliste (Korrekte Liste inkl Action suchen.)
                // - Whitelist, Blacklist
                // - Input-Action on New
                // - Error-Types.
                debuglog("EDB_LA_ON_NEW",
                    DB_DBGLVL_DEBUG,event[MY_EVENTS_USER],"process_one_event");
                actions = db_query_err("SELECT actionid,lh.listid,action_type, "
                    "action_parameter FROM listactions la JOIN list_header lh "
                    "ON lh.listid = la.listid WHERE (lh.error_types & ?4) > 0 "
                    "AND (la.action_type & ?3) > 0 AND lh.listid IN "
                    "(SELECT DISTINCT listid "
                    "FROM list_filters lf1 JOIN actdbg ad1 "
                    "ON ad1.debugger LIKE lf1.filter_group "
                    "WHERE lf1.filter_type = ?1 AND lf1.listid NOT IN "
                    "(SELECT DISTINCT lf2.listid "
                    "FROM list_filters lf2 JOIN actdbg ad2 "
                    "ON ad2.debugger LIKE lf2.filter_group "
                    "WHERE lf2.filter_type = ?2)) ",
                    EDB_LFT_WHITELIST,EDB_LFT_BLACKLIST,
                    EDB_LA_INPUT_ACTION|EDB_LA_ON_NEW,
                    event[MY_EVENTS_ERROR_TYPE]);
                execute_actions(event,actions, &cnt_actions);
            }
            if ((event[MY_EVENTS_ACTION_TYPE] & EDB_LA_ON_DBG_CHANGE) !=0)
            {
                // fuer eine errnum change of debugger abgleichen. => austragen
                debuglog("EDB_LA_ON_DBG_CHANGE",
                    DB_DBGLVL_DEBUG,event[MY_EVENTS_USER],"process_one_event");
                actions = db_query_err("SELECT actionid,lh.listid,action_type, "
                    "action_parameter FROM listactions la JOIN list_header lh "
                    "ON lh.listid = la.listid WHERE (lh.error_types & ?4) > 0 "
                    "AND (la.action_type & ?3) > 0 AND lh.listid IN "
                    "(SELECT listid FROM list2errnum WHERE errnum = ?5) "
                    "AND lh.listid NOT IN "
                    "(SELECT DISTINCT listid "
                    "FROM list_filters lf1 JOIN actdbg ad1 "
                    "ON ad1.debugger LIKE lf1.filter_group "
                    "WHERE lf1.filter_type = ?1 AND lf1.listid NOT IN "
                    "(SELECT DISTINCT lf2.listid "
                    "FROM list_filters lf2 JOIN actdbg ad2 "
                    "ON ad2.debugger LIKE lf2.filter_group "
                    "WHERE lf2.filter_type = ?2)) ",
                    EDB_LFT_WHITELIST,EDB_LFT_BLACKLIST,
                    EDB_LA_INPUT_ACTION|EDB_LA_ON_DBG_CHANGE,
                    event[MY_EVENTS_ERROR_TYPE],
                    event[MY_EVENTS_ERRNUM]);
                execute_actions(event,actions, &cnt_actions,EDB_LA_REMOVE);
                // => eintragen wenn neu in gruppe.
                actions = db_query_err("SELECT actionid,lh.listid,action_type, "
                    "action_parameter FROM listactions la JOIN list_header lh "
                    "ON lh.listid = la.listid WHERE (lh.error_types & ?4) > 0 "
                    "AND (la.action_type & ?3) > 0 "
                    "AND ((la.action_type & ?6) > 0 OR lh.listid NOT IN "
                    "(SELECT listid FROM list2errnum WHERE errnum = ?5)) "
                    "AND lh.listid IN "
                    "(SELECT DISTINCT listid "
                    "FROM list_filters lf1 JOIN actdbg ad1 "
                    "ON ad1.debugger LIKE lf1.filter_group "
                    "WHERE lf1.filter_type = ?1 AND lf1.listid NOT IN "
                    "(SELECT DISTINCT lf2.listid "
                    "FROM list_filters lf2 JOIN actdbg ad2 "
                    "ON ad2.debugger LIKE lf2.filter_group "
                    "WHERE lf2.filter_type = ?2)) ",
                    EDB_LFT_WHITELIST,EDB_LFT_BLACKLIST,
                    EDB_LA_INPUT_ACTION|EDB_LA_ON_DBG_CHANGE,
                    event[MY_EVENTS_ERROR_TYPE],
                    event[MY_EVENTS_ERRNUM],EDB_LA_NEW);
                execute_actions(event,actions, &cnt_actions,EDB_LA_NEW);
            }
            if ((event[MY_EVENTS_ACTION_TYPE] & EDB_LA_ON_READ) !=0)
            {
                // fuer eine errnum change of debugger abgleichen. => austragen
                debuglog("EDB_LA_ON_READ",
                    DB_DBGLVL_DEBUG,event[MY_EVENTS_USER],"process_one_event");
                actions = db_query_err("SELECT actionid,listid,action_type, "
                    "action_parameter FROM listactions "
                    "WHERE listid IN "
                    "(SELECT listid FROM list2errnum WHERE errnum = ?1) "
                    "AND (action_type & ?2) > 0 AND (action_type & ?3) > 0",
                    event[MY_EVENTS_ERRNUM],EDB_LA_INPUT_ACTION,EDB_LA_ON_READ);
                execute_actions(event,actions, &cnt_actions);
                actions = db_query_err("SELECT actionid,listid,action_type, "
                    "action_parameter FROM listactions "
                    "WHERE listid LIKE ?4 AND listid IN "
                    "(SELECT listid FROM list2errnum WHERE errnum = ?1) "
                    "AND (action_type & ?2) > 0 AND (action_type & ?3) > 0",
                    event[MY_EVENTS_ERRNUM],EDB_LA_INPUT_ACTION,
                    EDB_LA_ON_READ_OWN,"%@"+lower_case(event[MY_EVENTS_USER]));
                 event[MY_EVENTS_ACTION_TYPE] &= (~EDB_LA_ON_READ); // Flag aus
                 event[MY_EVENTS_ACTION_TYPE] |= EDB_LA_ON_READ_OWN;// Flag an!
                 execute_actions(event,actions, &cnt_actions);
            }
            if ((event[MY_EVENTS_ACTION_TYPE] & EDB_LA_TRIGGERS_SUBSET1) !=0)
            {
                debuglog("EDB_LA_TRIGGERS_SUBSET1",
                    DB_DBGLVL_DEBUG,event[MY_EVENTS_USER],"process_one_event");
                actions = db_query_err("SELECT actionid,listid,action_type, "
                    "action_parameter FROM listactions "
                    "WHERE ((action_type & ?4) > 0 OR listid IN "
                    "(SELECT listid FROM list2errnum WHERE errnum = ?1)) "
                    "AND (action_type & ?2) > 0 AND (action_type & ?3) > 0",
                    event[MY_EVENTS_ERRNUM],EDB_LA_INPUT_ACTION,
                    event[MY_EVENTS_ACTION_TYPE]&EDB_LA_TRIGGERS_SUBSET1,
                    EDB_LA_NEW);
                execute_actions(event,actions, &cnt_actions);
            }
            break;
        case EDB_LA_IN_LIST_ACTION:
                debuglog("EDB_LA_IN_LIST_ACTION",
                    DB_DBGLVL_DEBUG,event[MY_EVENTS_USER],"process_one_event");
                actions = db_query_err("SELECT actionid,listid,action_type, "
                    "action_parameter FROM listactions "
                    "WHERE listid = ?1 AND (action_type & ?2) > 0 "
                    "AND (action_type & ?3) > 0",
                    event[MY_EVENTS_LISTID],EDB_LA_IN_LIST_ACTION,
                    event[MY_EVENTS_ACTION_TYPE]&EDB_LA_TRIGGERS);
            execute_actions(event,actions, &cnt_actions);
            break;
    }
    db_query_err("DELETE FROM list_events WHERE seq = ?",
        event[MY_EVENTS_SEQ]);
    return 1;
}

#define SECURE if (!(adminp(this_interactive()) && \
        this_interactive() == this_player() && \
        geteuid(previous_object()) == geteuid(this_interactive()))) \
        return 0
public int process_event()
{
    SECURE;
    process_one_event();
    return 1;
}

static void process_event_loop()
{
    int cnt1,cnt2,todo=1,*ut1,*ut2,*ut3;
    string uts;
    mixed r;
    ut1 = utime();
    while (find_call_out("process_event_loop")!=-1);
    r = db_query_err("SELECT COUNT(*) FROM list_events");
    cnt1 = sizeof(r)?get_one_int(r):0;
    while (todo && !last_error 
        && get_eval_cost() > EVALS_EVENT_LOOP
        && (!pointerp(ut3) || ut3[0]==0) ) // ab einer vollen Sekunde raus.
    {
        last_error = catch(todo = process_one_event();publish);
        ut2 = utime();
        ut3 = ({ ut2[0]-ut1[0], ut2[1]-ut1[1]}); 
        if (ut3[1]<0) 
        {
            ut3[0]--;
            ut3[1]=1000000+ut3[1];
        } 
        uts = sprintf("%d.%06d", ut3[0],ut3[1]);
    }
    r = db_query_err("SELECT COUNT(*) FROM list_events");
    cnt2 = sizeof(r)?get_one_int(r):0;
    if (cnt1 > 0 || last_error)
    {
        db_query_err("INSERT INTO list_event_stats "
            "(event_time,cnt_before,cnt_after,errmsg) "
            "VALUES( ?,?,?,?) ",time(),cnt1,cnt2,last_error||uts);
    }
    if (!last_error && cnt2 > 0)
    {
        call_out("process_event_loop",DELAY_EVENT_LOOP);
    }
}

public void clean_up_list2errnum()
{
    db_query_err("DELETE FROM list2errnum WHERE errnum NOT IN "
        "(SELECT errnum FROM earchive_1)");
}

public <int|string *> query_list_headers(mapping options)
{
    mixed r,*args=({});
    string q,dbg;
    int whereflag = 0;
    if (member(options,DB_DBG_COUNT))
    {
        q = "SELECT COUNT(*) FROM list_header ";
    }
    else
    {
        q = "SELECT listid FROM list_header ";
    }
    if (pointerp(options[EDB_OPT_DEBUGGERS]))
    {
        dbg = "("+implode(({"?"})*sizeof(options[EDB_OPT_DEBUGGERS]),",")+") ";
        q+= (whereflag++?"AND ":"WHERE ");
        q+= "(SUBSTR(listid,INSTR(listid,'@')+1) IN "+dbg;
        args += options[EDB_OPT_DEBUGGERS];
        if (member(options,EDB_OPT_ZUSTAENDIGER))
        {
            q+= "OR SUBSTR(listid,INSTR(listid,'@')+1) = ? ";
            args += ({ options[EDB_OPT_ZUSTAENDIGER] });
        }
        q+= "OR listid IN (SELECT DISTINCT listid FROM list_filters ";
        q+= "filter_type IN (?,?) ";
        args += ({EDB_LFT_LIST_READERS,EDB_LFT_LIST_WRITERS});
        q+= "AND filter_group IN "+dbg;
        args += options[EDB_OPT_DEBUGGERS];
        q+= ")) ";
    }
    else if (member(options,EDB_OPT_ZUSTAENDIGER))
    {
        q+= (whereflag++?"AND ":"WHERE ");
        q+= "SUBSTR(listid,INSTR(listid,'@')+1) = ? ";
        args += ({ options[EDB_OPT_ZUSTAENDIGER] });
    }
    if (!member(options,DB_DBG_COUNT))
    {
        q+= "ORDER BY SUBSTR(listid,INSTR(listid,'@')+1),listid ";
        if (member(options,DB_DBG_LIMIT))
        {
            q+= "LIMIT ? OFFSET ? ";
            args += ({options[DB_DBG_LIMIT],options[DB_DBG_OFFSET] });
        }
    }
    r = db_query_err(q,args...);
    if (member(options,DB_DBG_COUNT))
    {
        return get_one_int(r);
    }
    return map( r||({}), (: $1[0] :) );
}

public int update_listheader(mapping listheader)
{
    if (!check_security() || !mappingp(listheader) ||
            (listheader[EDB_OPT_ERROR_TYPES] & EDB_TMASK_ALL) == 0)
        return 0;
    // TODO check accessrights form TP_RN
    db_query_err("UPDATE list_header SET error_types = ?2 WHERE listid = ?1",
        listheader[EDB_OPT_LISTID]||"",
        listheader[EDB_OPT_ERROR_TYPES] & EDB_TMASK_ALL);
    return 1;
}

public <int|mapping*> get_list_filters(string listid,int limit,int offset)
{
    string lo = "";
    mixed r;
    mapping *result = ({});
    mixed line;
    if (!check_security()) 
        return 0;
#define MAP_LIST_FILTER ({EDB_OPT_LISTID,\
                          EDB_OPT_FILTER_TYPE, \
                          EDB_OPT_FILTER_GROUP})
    if (limit > 0)
    {
        lo = "LIMIT "+limit+" OFFSET "+offset+" ";
    }
    else if (limit < 0)
    {
        r = db_query_err("SELECT COUNT(*) FROM list_filters "
            "WHERE listid = ?1",listid);
        return sizeof(r) ? get_one_int(r) : 0;
    }
    r = db_query_err("SELECT listid,filter_type,filter_group "
        "FROM list_filters WHERE listid = ?1 "+lo,listid||"");
    if (!sizeof(r))
        return ({});
    foreach (line : r )
    {
        result += ({ mkmapping(MAP_LIST_FILTER,line) });
    }
    return result;
}

public int update_one_list_filter(mapping lf)
{
    if (!check_security() || !mappingp(lf)) 
        return 0;
    // TODO check accessrights form TP_RN
    mixed r = db_query_err("SELECT 1 FROM list_header WHERE listid = ?1",
        lf[EDB_OPT_LISTID]||"");
    if (!sizeof(r))
        return 0;
    db_query_err("INSERT OR REPLACE INTO list_filters "
        "(listid,filter_type,filter_group) VALUES(?,?,?) ",
        lf[EDB_OPT_LISTID],lf[EDB_OPT_FILTER_TYPE],lf[EDB_OPT_FILTER_GROUP]);
    return 1;
}

public int delete_one_list_filter(mapping lf)
{
    if (!check_security()) 
        return 0;
    // TODO check accessrights form TP_RN
    db_query_err("DELETE FROM list_filters WHERE listid = ?1 "
        "AND filter_type = ?2 AND filter_group = ?3",
        lf[EDB_OPT_LISTID],lf[EDB_OPT_FILTER_TYPE],lf[EDB_OPT_FILTER_GROUP]);
    return 1;
}

public <int|mapping*> get_list_actions(string listid,int limit,int offset)
{
    if (!check_security()) 
        return 0;
#define MAP_LIST_ACTIONS ({EDB_OPT_ACTIONID, \
                           EDB_OPT_LISTID,\
                           EDB_OPT_ACTION_TYPE, \
                           EDB_OPT_ACTION_PARAM })
    string lo = "";
    mixed r;
    if (limit > 0)
    {
        lo = "LIMIT "+limit+" OFFSET "+offset+" ";
    }
    else if (limit < 0)
    {
        r = db_query_err("SELECT COUNT(*) FROM listactions "
            "WHERE listid = ?1",listid);
        return sizeof(r) ? get_one_int(r) : 0;
    }
    // TODO check accessrights form TP_RN
    r = db_query_err("SELECT 1 FROM list_header WHERE listid = ?1 "+lo,
        listid||"");
    if (!sizeof(r))
        return 0;
    r = db_query_err("SELECT actionid,listid,action_type,action_parameter "
        "FROM listactions WHERE listid = ?1 ORDER BY actionid",
        listid);
    mapping * result = ({});
    foreach (mixed line : r||({}) )
    {
        result += ({ mkmapping(MAP_LIST_ACTIONS,line) });
    }
    return result;
}

public mapping query_list_action(int actionid)
{
    if (!check_security()) 
        return 0;
    // TODO check accessrights form TP_RN
    mixed r = db_query_err("SELECT actionid,listid,action_type,action_parameter "
        "FROM listactions WHERE actionid = ?1 ",
        actionid);
    if (!sizeof(r))
        return ([]);
    return mkmapping(MAP_LIST_ACTIONS,r[0]);
}

public int set_list_action(mapping act)
{
    if (!check_security() || !mappingp(act)) 
        return 0;
    // TODO check accessrights form TP_RN
    mapping old = query_list_action(act[EDB_OPT_ACTIONID]);
    if (!sizeof(old))
    {
        db_query_err("INSERT INTO listactions "
            "(listid,action_type,action_parameter) VALUES(?,?,?) ",
            act[EDB_OPT_LISTID],
            act[EDB_OPT_ACTION_TYPE],
            act[EDB_OPT_ACTION_PARAM]);
        mixed r = db_query_err("SELECT MAX(actionid) FROM listactions");
        return sizeof(r)?get_one_int(r):0;
    }
    if (!stringp(act[EDB_OPT_LISTID]))
    {
        db_query_err("DELETE FROM listactions WHERE actionid = ?1",
            act[EDB_OPT_ACTIONID]);
        return -1;
    }
    db_query_err("UPDATE listactions SET listid = ?2,action_type = ?3,"
        "action_parameter = ?4 WHERE actionid = ?1",
            act[EDB_OPT_ACTIONID],
            act[EDB_OPT_LISTID],
            act[EDB_OPT_ACTION_TYPE],
            act[EDB_OPT_ACTION_PARAM]);
    return act[EDB_OPT_ACTIONID];
}

public varargs <int|int*> query_list_errnums(string listid,int cntflag)
{
    mixed r;
    if (cntflag)
    {
        r = db_query_err("SELECT COUNT(errnum) FROM list2errnum "
            "WHERE listid = ?",listid||"");
        return sizeof(r)?get_one_int(r):0;
    }
    r = db_query("SELECT errnum FROM list2errnum "
            "WHERE listid = ?",listid||"");
    if (!sizeof(r))
        return 0;
    return map(r, (: $1[0] :) );
}

public mapping query_list_header(string listid)
{
    if (!check_security()) 
        return 0;
    mixed result = db_query_err("SELECT error_types FROM list_header "
        "WHERE listid = ?1",listid||"");
    if (!result || !sizeof(result))
        return 0;
    return ([
        EDB_OPT_LISTID : listid,
        EDB_OPT_ERROR_TYPES: result[0][0],
        EDB_OPT_ACTION_COUNT: get_list_actions(listid,-1,0),
        EDB_OPT_FILTER_COUNT: get_list_filters(listid,-1,0),
        EDB_OPT_ERRNUM_COUNT: query_list_errnums(listid,1),
    ]);
}

public int delete_listheader(string listid)
{
    if (!check_security() || !stringp(listid) 
        || stringp(check_write_access_to_listid(listid)))
        return 0;
    if (get_list_actions(listid,-1,0)>0)
        return 0;
    if (get_list_filters(listid,-1,0)>0)
        return 0;
    if (sizeof(query_list_errnums(listid))>0)
        return 0;
    db_query_err("DELETE FROM list_header WHERE listid = ?1",listid);
    return 1;
}

// ------------------------------------------------------------------------
public int set_error_trashbin(int errnum,mixed* eheader,mapping edata)
{
    if (!check_security()) 
        return 0;
    if (!pointerp(eheader) || !mappingp(edata) || errnum <= 0)
    {
        return 0;
    }
    db_query_err("INSERT OR REPLACE INTO error_trashbin "
        "(errnum,errheader,errdata,savetime) VALUES (?,?,?,?)",
        errnum, save_value(eheader),save_value(edata),time());
    return 1;
}

public mixed * query_error_trashbin(int errnum)
{
    if (!check_security()) 
        return 0;
    mixed r = db_query_err("SELECT errheader,errdata "
        "FROM error_trashbin WHERE errnum = ?",errnum);
    if (sizeof(r))
    {
        return ({restore_value(r[0][0]), 
                 restore_value(r[0][1]), });
    }
    return 0;
}

public <int|mixed*> query_errors_from_trashbin(mapping options,int limit,int offset)
{
    string q;
    mixed r;
    if (!check_security()) 
        return 0;
    if (limit < 0)
    {
        q = "SELECT COUNT(*) FROM error_trashbin ";
    }
    else
    {
        q = "SELECT errnum,savetime FROM error_trashbin ";
    }
    // TODO options...
    if (limit >= 0)
    {
        q+= "ORDER BY savetime DESC,errnum ASC ";
    }
    if (limit > 0)
    {
        q+= "LIMIT "+limit+" OFFSET "+offset;
    }
    r = db_query_err(q);
    if (limit < 0)
    {
        return sizeof(r) ? get_one_int(r) : 0;
    }
    if (!sizeof(r))
        return 0;
    return r;
}

// ------------------------------------------------------------------------
public int add_debugger_history(int errnum,string* debuggers,
    string wiz, int flag)
{
    if (!check_security()) 
        return 0;
    if (pointerp(debuggers))
        debuggers -= ({"",0});
    else
        return 0;
    if (errnum <= 0 || !sizeof(debuggers)
        || member( ({EDB_DBGF_ADD,EDB_DBGF_DEL}),flag)==-1)
        return 0;
    string debugger = ","+implode(debuggers,",")+",";
    // fuehrendes und letztes Komma fuer suchen nach %,Root,% moeglich.
    mixed r = db_query_err("SELECT MAX(history_id) FROM debugger_history");
    int seq = sizeof(r) ? (get_one_int(r)+1):1;
    db_query_err("INSERT INTO debugger_history "
        "(history_id,flags,errnum,debugger,savetime,wiz) "
        "VALUES (?,?,?,?,?,?) ",
        seq,flag,errnum,debugger,time(),wiz||"");
    return 1;
}
// ------------------------------------------------------------------------

/*
NOENZY: create_all
DEKLARATION: static void create_all(int version)
BESCHREIBUNG:
Bringt die Datenbank von Stand <version> auf einen aktuellen Stand.
VERWEISE: create_or_update_db
GRUPPEN: protokoll
*/
static void create_all(int version)
{
    if (!db_open())
        raise_error("Datenbank konnte nicht geöffnet werden: "+ERROR_ARCHIVE_DB);

    switch (version)
    {
    case 0: // Initiale Version
        db_debug("create_all 1.0 vs "+version,DB_DBGLVL_INFO,
            DB_DBG_BUFFER_MSG,"ERROR_ARCHIVE");
        if (query_db_error()) break;
        // Pruefe, ob Tabelle "earchive_1" existiert.------------------------
        db_debug("if exists earchive_1...",DB_DBGLVL_DEBUG,
            DB_DBG_BUFFER_MSG,"ERROR_ARCHIVE");
        db_query("SELECT MAX(errnum) FROM earchive_1");
        if (query_db_error()) {
            db_debug("no: create earchive_1",DB_DBGLVL_DEBUG,
            DB_DBG_BUFFER_MSG,"ERROR_ARCHIVE");
            // Ersetze bei gleicher ID aber unterschiedlichem Namen (REPLACE)
            // Rollback ein INSERT NEW, dass den gleichen Namen bringt.
            db_query_err("CREATE TABLE IF NOT EXISTS earchive_1 ("
                "errnum INTEGER CONSTRAINT pk_earchive_1 "
                "PRIMARY KEY , opened INTEGER, changed INTEGER,"
                "errtext TEXT, flagvis INTEGER )");
            if (query_db_error()) break;
            db_query_err("CREATE INDEX IF NOT EXISTS earchive_1_errnum ON earchive_1 (errnum)");
            db_query_err("CREATE INDEX IF NOT EXISTS earchive_1_opened ON earchive_1 (opened, changed)");
            db_query_err("CREATE INDEX IF NOT EXISTS earchive_1_changed ON earchive_1 (changed, opened)");
            db_query_err("CREATE INDEX IF NOT EXISTS earchive_1_flagvis ON earchive_1 (flagvis)");
        } else {
            db_debug("yes: keine Aktion",DB_DBGLVL_DEBUG,
                DB_DBG_BUFFER_MSG,"ERROR_ARCHIVE");
        }
        //----------------------------------------------------------
    case 1: // HashTags und Listen
        db_query_err("CREATE TABLE IF NOT EXISTS hashtags ("
                "errnum INTEGER, hashtag TEXT, "
                "CONSTRAINT pk_hashtags PRIMARY KEY (errnum,hashtag) )");
        db_query_err("CREATE INDEX IF NOT EXISTS hashtags_hashtag ON hashtags (hashtag)");
        // db_query_err("DROP TABLE IF EXISTS list_events"); // TODO rauswerfen
        // db_query_err("DROP TABLE IF EXISTS list_header"); // TODO rauswerfen
        // db_query_err("DROP TABLE IF EXISTS list_filters"); // TODO rauswerfen
        // db_query_err("DROP TABLE IF EXISTS listactions"); // TODO rauswerfen
        // db_query_err("DROP TABLE IF EXISTS list2errnum"); // TODO rauswerfen
        db_query_err("CREATE TABLE IF NOT EXISTS list_events ("
                "seq INTEGER PRIMARY KEY AUTOINCREMENT, "
                "event_time INTEGER, "
                "event_user TEXT, "
                "errnum INTEGER, error_type INTEGER, "
                "listid TEXT, debugger TEXT, "
                "action_type INTEGER )");
        db_query_err("CREATE TABLE IF NOT EXISTS list_header ("
                "listid TEXT PRIMARY KEY, " // todecide@Root
                "error_types INTEGER " // zu beobachtende Error-Types.
                ")");
        db_query_err("CREATE TABLE IF NOT EXISTS list_filters ("
                "listid TEXT, "
                "filter_type INTEGER, " // siehe EDB_LFT_* Defines error_db.h
                "filter_group TEXT, " // name der gruppe o.ae.
                "PRIMARY KEY (listid,filter_type,filter_group) )"
                );
        db_query_err("CREATE INDEX IF NOT EXISTS idx01_list_filters "
            "ON list_filters (filter_type)");
        db_query_err("CREATE TABLE IF NOT EXISTS listactions ("
                "actionid INTEGER PRIMARY KEY AUTOINCREMENT, "
                "listid TEXT,action_type INTEGER,action_parameter TEXT )");
        db_query_err("CREATE TABLE IF NOT EXISTS list2errnum ("
                "errnum INTEGER, listid TEXT, "
                "CONSTRAINT pk_list2errnum PRIMARY KEY (errnum,listid) )");
        db_query_err("CREATE TABLE IF NOT EXISTS list_event_stats ("
                "event_time INTEGER PRIMARY KEY, "
                "cnt_before INTEGER, "
                "cnt_after INTEGER, "
                "errmsg TEXT ) ");
    case 2: // Muelleimer
        db_query_err("CREATE TABLE IF NOT EXISTS error_trashbin ("
                "errnum INTEGER PRIMARY KEY, "
                "errheader TEXT, errdata TEXT, savetime INTEGER )");
        db_query_err("CREATE TABLE IF NOT EXISTS debugger_history ("
                "history_id INTEGER PRIMARY KEY, flags INTEGER, "
                "errnum INTEGER, debugger TEXT, savetime INTEGER,"
                "wiz TEXT )");
    case 3: // FTS
        db_query_err("CREATE VIRTUAL TABLE IF NOT EXISTS earchive_fts "
                     "USING fts5(opened UNINDEXED, changed UNINDEXED, errtext, flagvis UNINDEXED, "
                                "content='earchive_1', content_rowid='errnum')");
        db_query_err("CREATE TRIGGER IF NOT EXISTS earchive_fts_insert AFTER INSERT ON earchive_1 BEGIN\n"
                     "    INSERT INTO earchive_fts(rowid, opened, changed, errtext, flagvis) VALUES (new.errnum, new.opened, new.changed, new.errtext, new.flagvis);\n"
                     "END");
        db_query_err("CREATE TRIGGER IF NOT EXISTS earchive_fts_delete AFTER DELETE ON earchive_1 BEGIN\n"
                     "    INSERT INTO earchive_fts(earchive_fts, rowid) VALUES('delete', old.errnum);\n"
                     "END");
        db_query_err("CREATE TRIGGER IF NOT EXISTS earchive_fts_update AFTER UPDATE ON earchive_1 BEGIN\n"
                     "    INSERT INTO earchive_fts(earchive_fts, rowid) VALUES('delete', old.errnum);\n"
                     "    INSERT INTO earchive_fts(rowid, opened, changed, errtext, flagvis) VALUES (new.errnum, new.opened, new.changed, new.errtext, new.flagvis);\n"
                     "END");
        db_query_err("INSERT INTO earchive_fts(earchive_fts) VALUES('rebuild')");

        set_db_version(4);
        db_debug("create_all: Update auf version 4 erfolgreich",DB_DBGLVL_DEBUG,
            DB_DBG_FLUSH_BUFFER,"ERROR_ARCHIVE");
    case 4: // Aktueller Stand
        db_close();
        return; // ERFOLG
    }

    // Hier kommt man nur im Fehlerfall hin...
    db_debug("create_all "+version+" abgebrochen!",DB_DBGLVL_ERROR,
            DB_DBG_FLUSH_BUFFER,"ERROR_ARCHIVE");
    db_close();
}

/*
NOENZY: create_or_update_db
DEKLARATION: private void create_or_update_db()
BESCHREIBUNG:
Prueft die verfuegbare Datenbank und aktualisiert diese, falls noetig.
VERWEISE: create_all
GRUPPEN: protokoll
*/
private void create_or_update_db()
{
    if (!db_open()) {
        raise_error("Datenbank konnte nicht geöffnet werden: "+ERROR_ARCHIVE_DB);
    }
    create_all(get_db_version());
}

// ------------------------------------------------------------------------

/*
NOENZY: db_flush
DEKLARATION: public void db_flush()
BESCHREIBUNG:
Hiermit kann der Debugpuffer in die Datenbank geschrieben werden
und die Datenbank geschlossen werden.
VERWEISE: show_debug
GRUPPEN: protokoll
*/
public void db_flush()
{
    db_debug(0,0,DB_DBG_FLUSH_BUFFER,"PROTOKOLLARCHIV");
    db_close();
}

/*
NOENZY: show_debug
DEKLARATION: public void show_debug()
BESCHREIBUNG:
Es wird der Debugpuffer an TP ausgegeben.
VERWEISE: sql
GRUPPEN: protokoll
*/
public void show_debug()
{
    if (!check_security() || !adminp(TP))
        return;
    db_debug(0,0,DB_DBG_FLUSH_BUFFER,"PROTOKOLLARCHIV");
    string * result = query_all_dbg_messages(DB_DBG_FLUSH_BUFFER);
    if (result == 0) return;
    result = flatten_array(map(result,(: explode(wrap($1),"\n")[..<2] :)));
    TP->more(result,({ "Debuglog [?,w,q,z] ",
           "----------- Debuglog: --------------------------------------------------------",
           LINE, MORE_LINE, MORE_LINE
        }), 0,
        M_DO_NOT_END|M_FRAME|M_THIS_OBJECT,
        "Root:Protokollarchiv:DebugLog");
}

/*
NOENZY: sql
DEKLARATION: public int sql(string sql)
BESCHREIBUNG:
Hiermit koennen direkte Abfragen und Manipulationen an der Datenbank
durchgefuehrt werden. Nur fuer Admins.
VERWEISE: tablelist
GRUPPEN: error_db
*/
public int sql(string sql)
{
    mixed * m;
    if (!check_security(CHECK_ERROR)) {
        return 0;
    }
    if (!TI || !adminp(TI)) {
        return 0;
    }

    m = db_query(sql||"");
    
    if(!m) TP->send_message_to(TP,MT_NOTIFY,MA_USE,
       wrap("Es gab keine Rückgabe."));
    else if(!pointerp(m)) TP->send_message_to(TP,MT_NOTIFY,MA_USE,
       wrap(sprintf("Der Rückgabewert war: '%O'",m)));
    else if(sizeof(m)==0) TP->send_message_to(TP,MT_NOTIFY,MA_USE,
       wrap("Die Tabelle ist (noch) leer."));
    else 
       TP->more (map(
  build_table ( transpose_array(map(m,(: map($1,(: to_string($1) :)) :))),
                      ({"|","-"})
                ), (: space($1) :))
       );
    if(query_db_error()) 
       TP->send_message_to(TP,MT_NOTIFY,MA_USE,
          wrap("Es gab folgenden Fehler: "+ query_db_error()));
    return 1;
}


/*
NOENZY: db_get_table
DEKLARATION: int tablelist(string str)
BESCHREIBUNG:
Ohne Parameter oder mit str==0 wird eine Liste aller Tabellen ausgegeben,
mit Parameter die Definition dieser speziellen Tabelle, sofern vorhanden.
VERWEISE: sql
GRUPPEN: error_db
*/
int tablelist(string str)
{
    check_security(CHECK_ERROR);
    string tb = space(str);
    if (tb == "") {
        return sql("SELECT type, name FROM sqlite_master "
            "WHERE type <> 'index' ORDER BY type,name");
    } else {
        return sql("SELECT sql FROM sqlite_master WHERE NAME = '"+tb+"'");
    }
}

void abort_renewal()
{
}

void finish_renewal(object neu)
{
}

void prepare_renewal()
{
    db_flush();
}

int remove()
{
    db_flush();
    destruct( TO );
    return 1;
}

void reset()
{
    db_flush();
    if (find_call_out("process_event_loop")==-1)
        call_out("process_event_loop",DELAY_EVENT_LOOP);
}

void create()
{
    init_security_for_actions();
    init_security_trust_mudlib();
    add_security_condition(ERROR_DB);
    if (!init_debuglog(ERROR_ARCHIVE_DB,-1))
        raise_error(ERROR_ARCHIVE_DB+" konnte nicht geöffnet werden.");
    create_or_update_db();
    db_debug("error_archive started:"+ERROR_ARCHIVE_DB,DB_DBGLVL_INFO,
            DB_DBG_CREATE,"ERROR_ARCHIVE");
    reset();
}
