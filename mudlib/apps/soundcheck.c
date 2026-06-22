// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:    /apps/soundcheck.c
// Description: Master zur Verwaltung von Sounds
//
// UID: Apps

inherit "/i/tools/security";
inherit "/i/tools/build_table";
inherit "/i/tools/debuglog_db";
inherit "/i/item/message";

#include <database.h>
#include <level.h>
#include <message.h>
#include <misc.h>
#include <security.h>

#include <soundcheck.h>

#define DB_FILE "/var/adm/soundcheck.db"
#define SOUND_FILES "/var/SOUND_FILES"
// SELECT sqlite_version()

#define SOUND_WHERE ({SC_SOUNDFILE_TARGET,SC_SOUNDFILE_PLAY,SC_SOUNDFILE_PROP})



private void init_db()
{
    string err;
    if (init_debuglog(DB_FILE, 30) == 0)
    {
        err = query_db_error();
        // Abfrage auf 0 wichtig, da dann keine DB Zugriffe moeglich sind!
        raise_error("Datenbank kann nicht geöffnet werden!\n"+wrap(err));
    }
    db_debug(0,0,DB_DBG_CREATE);
    db_query_err("CREATE TABLE IF NOT EXISTS hints ("
        "hint_level INTEGER PRIMARY KEY, "
        "hint_text TEXT)");
    db_query_err("CREATE TABLE IF NOT EXISTS soundfiles ("
        "sf_filename TEXT PRIMARY KEY, "    // relative filename
        "sf_status INTEGER, "               // del,act,target,play,prop
        "sf_source TEXT, "                  // nickname des autors
        "sf_copyright TEXT )");             // copyright vermerk.
    db_query_err("CREATE TABLE IF NOT EXISTS wishlist ("
        "wl_id INTEGER PRIMARY KEY, "   // ids may be reused...
        "wl_status INTEGER, "           // del,act,open,fulfilled
        "wl_priority INTEGER, "         // vergeben durch admins.
        "wl_from TEXT, "                // wer fordert an.(TP_RN)
        "wl_wish TEXT, "                // Wunschbeschreibung.
        "sf_filename TEXT, "            // null or concrete file.
        "CONSTRAINT wl_sf FOREIGN KEY (sf_filename) "
        "REFERENCES soundfiles (sf_filename) ON DELETE SET NULL )");
    db_query_err("CREATE TABLE IF NOT EXISTS proposals ("
        "pr_id INTEGER PRIMARY KEY, "   // ids may be reused
        "pr_status INTEGER, "           // del,act,open,
        "pr_from TEXT, "                // proposal from
        "pr_description TEXT, "         // Kurzbeschreibung
        "sf_filename TEXT, "            // null or concrete file.
        "wl_id INTEGER, "               // Referenz zu einem Wunsch. oder null
        "CONSTRAINT pr_sf FOREIGN KEY (sf_filename) "
        "REFERENCES soundfiles (sf_filename) ON DELETE SET NULL,"
        "CONSTRAINT pr_wl FOREIGN KEY (wl_id) "
        "REFERENCES wishlist (wl_id) ON DELETE SET NULL )");
}

public int update_hint(int lvl,string txt)
{
    if (!check_security()||!adminp(TP))
        return 0;
    if (!stringp(txt))
    {
        db_query_err("DELETE FROM hints WHERE hint_level = ?",lvl);
        return 2;
    }
    db_query_err("INSERT OR REPLACE INTO hints (hint_level,hint_text) "
        "VALUES(?,?)",lvl,txt);
    return 1;
}

public string* get_one_hint(int lvl)
{
    mixed r = db_query_err("SELECT hint_text FROM hints WHERE hint_level = ?",
        lvl);
    return sizeof(r) ? explode(r[0][0],"\n") : ({});
}

public string* get_hint(int lvl)
{
    string *lines = ({});
    if (!check_security())
        return ({});
    if (lvl == SC_LVL_ADMIN)
    {
        mixed r = db_query_err("SELECT hint_level FROM hints "
            "ORDER BY hint_level");
        if (sizeof(r))
        {
            int *levels = map(r,(: $1[0] :));
            int level;
            foreach(level : levels)
            {
                lines += ({ sprintf("___Level_%02d:___",level) });
                lines += get_one_hint(level);
            }
            lines += ({"___ENDE___"});
            return lines;
        }
        return ({});
    }
    else
    {
        lines += get_one_hint(0);
        lines += get_one_hint(lvl);
    }
    return lines;
}

#define MY_SOUNDFILES ({SOUND_OPT_FILE,SOUND_OPT_STATUS,\
    SOUND_OPT_SOURCE,SOUND_OPT_COPYRIGHT})
public <mapping*|int> get_all_soundfiles(mapping options,int countflag)
{
    string q;
    mixed r,*args=({});
    int whereflag = 0;
    if (!check_security(CHECK_LAST_OBJECT) || !mappingp(options))
    {
        debuglog("Sicherheitsfehler: "+mixed2str(caller_stack()),
            DB_DBGLVL_ERROR,TP_RN,"get_all_soundfiles");
        return 0;
    }
    if (countflag)
    {
        q = "SELECT COUNT(*) FROM soundfiles ";
    }
    else
    {
        q = "SELECT sf_filename,sf_status,sf_source,sf_copyright ";
        q+= "FROM soundfiles ";
    }
    if (member(options,SOUND_OPT_STATUS))
    {
        q+= whereflag++?"AND ":"WHERE ";
        if (pointerp(options[SOUND_OPT_STATUS]))
        {
            q+= "sf_status IN ("
               +implode( ({"?"})*sizeof(options[SOUND_OPT_STATUS]),",")+") ";
            args += options[SOUND_OPT_STATUS];
        }
        else if (member(options,SOUND_OPT_STATUS_BM))
        {
            q+= "(sf_status & ? )= ? ";
            args += ({ options[SOUND_OPT_STATUS],options[SOUND_OPT_STATUS_BM] });
        }
        else
        {
            q+= "(sf_status & ? )> 0 ";
            args += ({ options[SOUND_OPT_STATUS] });
        }
    }
    else
    {
        q+= whereflag++?"AND ":"WHERE ";
        q+= "(sf_status & ? )= 0 ";
        args += ({ SC_STATUS_DELETED });
    }
    if (member(options,SOUND_OPT_FILE))
    {
        q+= whereflag++?"AND ":"WHERE ";
        q+= "sf_filename = ? ";
        args += ({ options[SOUND_OPT_FILE] });
    }
    if (member(options,SOUND_OPT_SOURCE))
    {
        q+= whereflag++?"AND ":"WHERE ";
        q+= "sf_source LIKE ? ";
        args += ({ options[SOUND_OPT_SOURCE] });
    }
    if (member(options,SOUND_OPT_COPYRIGHT))
    {
        q+= whereflag++?"AND ":"WHERE ";
        q+= "sf_copyright LIKE ? ";
        args += ({ options[SOUND_OPT_COPYRIGHT] });
    }
    if (!countflag && member(options,DB_DBG_LIMIT))
    {
        q+= "ORDER BY sf_filename ";
        q+= "LIMIT ? OFFSET ? ";
        args += ({ options[DB_DBG_LIMIT],options[DB_DBG_OFFSET] });
    }
    r = db_query_err(q,args...);
    if (countflag)
        return get_one_int(r);
    if (!sizeof(r))
        return ({});
    return map( r, (: mkmapping(MY_SOUNDFILES,$1) :) ); 
}

public mapping get_one_soundfile(string sf_filename)
{
    mapping* result;
    if (!check_security())
        return 0;
    if (stringp(sf_filename))
    {
        result = get_all_soundfiles(([
            SOUND_OPT_FILE:sf_filename,
            SOUND_OPT_STATUS: SC_STATUS_DELETED,
            SOUND_OPT_STATUS_BM: 0]),0);
    }
    else
    {
        return 0;
    }
    return (sizeof(result)==1)?result[0]:0;
}

public mapping get_next_soundfile(string sf_filename,string plusminus)
{
    mixed r;
    string q;
    if (!check_security() || !stringp(sf_filename))
        return 0;
    q = "SELECT sf_filename FROM soundfiles WHERE ";
    switch (plusminus)
    {
        case "+":
            q+= "sf_filename > ?1 ORDER BY sf_filename ASC LIMIT 1 ";
            break;
        case "-":
            q+= "sf_filename < ?1 ORDER BY sf_filename DESC LIMIT 1 ";
            break;
        default: 
            return 0;
    }
    r = db_query_err(q,sf_filename);
    if (sizeof(r)==1)
    {
        return get_one_soundfile(r[0][0]);
    }
    return 0;
}

public int set_one_soundfile(mapping sound)
{
    string *opts = ({});
    mixed *args = ({});
    if (!check_security())
        return 0;
    mapping oldsound = get_one_soundfile(sound[SOUND_OPT_FILE]);
    if (!mappingp(oldsound))
    {
        return 0; // TODO neues soundfile anlegen??
    }
    if (oldsound[SOUND_OPT_STATUS] != sound[SOUND_OPT_STATUS])
    {
        opts += ({"sf_status = ? "});
        args += ({sound[SOUND_OPT_STATUS]});
    }
    if (oldsound[SOUND_OPT_SOURCE] != sound[SOUND_OPT_SOURCE])
    {
        opts += ({"sf_source = ? "});
        args += ({sound[SOUND_OPT_SOURCE]});
    }
    if (oldsound[SOUND_OPT_COPYRIGHT] != sound[SOUND_OPT_COPYRIGHT])
    {
        opts += ({"sf_copyright = ? "});
        args += ({sound[SOUND_OPT_COPYRIGHT]});
    }
    if (sizeof(opts)==0)
        return 1; // no update
    args += ({sound[SOUND_OPT_FILE]});
    db_query_err("UPDATE soundfiles SET "+implode(opts," , ")
            +" WHERE sf_filename = ?",args...);
    return 1;
}

public int register_soundfile(string filename, int where)
{
    if (!check_security(CHECK_LAST_OBJECT))
        return 0;
    if (!stringp(filename) || member (SOUND_WHERE,where)==-1)
        return 0;
    mixed r = db_query_err("SELECT sf_filename,sf_status FROM soundfiles "
        "WHERE sf_filename = ? ",filename);
    if (sizeof(r)==1)
    {
        r = r[0];
        if ( (r[1] & where)>0)
        {
            return 1; // OK
        }
        if ( (r[1] & SC_STATUS_ACTIVE)==0)
        {
            return 0;// Fehler: Sound inaktiv.
        }
        db_query_err("UPDATE soundfiles SET sf_status = ?1 "
            "WHERE sf_filename = ?2",r[1]|where,filename);
        return 1; // OK
    }
    db_query_err("INSERT INTO soundfiles (sf_status,sf_filename) VALUES (?,?)",
        where|SC_STATUS_ACTIVE,filename);
    return 1; // OK
}

public int read_sound_files()
{
    if (!check_security()) 
    {
        return 0;
    }
    db_query_err("UPDATE soundfiles SET sf_status = (sf_status & ?1)",
        ~SC_SOUNDFILE_TARGET);
    string line = read_file(SOUND_FILES);
    string *lines = explode(line||"","\n");
    int ix = 0;
    foreach(line : lines)
    {
        if (line != "")
        {
            if (register_soundfile(line,SC_SOUNDFILE_TARGET))
                ix++;
        }
    }
    return ix;
}

#define MY_WISHLIST ({SOUND_OPT_WL_ID,SOUND_OPT_STATUS,SOUND_OPT_PRIORITY,\
    SOUND_OPT_FROM,SOUND_OPT_WISH,SOUND_OPT_FILE})
public <int|mapping*> get_wishlist(mapping options,int countflag)
{
    string q;
    mixed r,*args=({});
    int whereflag = 0;
    if (!check_security(CHECK_LAST_OBJECT) || !mappingp(options))
    {
        return 0;
    }
    if (countflag)
    {
        q = "SELECT COUNT(*) FROM wishlist ";
    }
    else
    {
        q = "SELECT wl_id,wl_status,wl_priority,wl_from,wl_wish,sf_filename ";
        q+= "FROM wishlist ";
    }
    if (member(options,SOUND_OPT_WL_ID))
    {
        q+= whereflag++?"AND ":"WHERE ";
        q+= "wl_id = ? "; 
        args += ({options[SOUND_OPT_WL_ID]});
    }
    if (member(options,SOUND_OPT_PRIORITY))
    {
        q+= whereflag++?"AND ":"WHERE ";
        q+= "wl_priority = ? ";
        args += ({options[SOUND_OPT_PRIORITY]});
    }
    if (member(options,SOUND_OPT_FROM))
    {
        q+= whereflag++?"AND ":"WHERE ";
        q+= "wl_from LIKE ? ";
        args += ({options[SOUND_OPT_FROM]});
    }
    if (member(options,SOUND_OPT_STATUS))
    {
        q+= whereflag++?"AND ":"WHERE ";
        if (pointerp(options[SOUND_OPT_STATUS]))
        {
            q+= "wl_status IN ("
                +implode(({"?"})*sizeof(options[SOUND_OPT_STATUS]),",")+") ";
            args += options[SOUND_OPT_STATUS];
        }
        else if (member(options,SOUND_OPT_STATUS_BM))
        {
            q+= "(wl_status & ? )= ? ";
            args += ({ options[SOUND_OPT_STATUS],options[SOUND_OPT_STATUS_BM] });
        }
        else
        {
            q+= "(wl_status & ? )> 0 ";
            args += ({ options[SOUND_OPT_STATUS] });
        }
    }
    else
    {
        q+= whereflag++?"AND ":"WHERE ";
        q+= "(wl_status & ? )=0 ";
        args += ({ SC_STATUS_DELETED });
    }
    if (member(options,SOUND_OPT_FILE))
    {
        q+= whereflag++?"AND ":"WHERE ";
        q+= "sf_filename = ? ";
        args += ({ options[SOUND_OPT_FILE] });
    }
    if (member(options,SOUND_OPT_WISH))
    {
        q+= whereflag++?"AND ":"WHERE ";
        q+= "wl_wish LIKE ? ";
        args += ({ options[SOUND_OPT_WISH] });
    }
    if (!countflag && member(options,DB_DBG_LIMIT))
    {
        q+= "ORDER BY wl_priority DESC,wl_id ASC ";
        q+= "LIMIT ? OFFSET ? ";
        args += ({ options[DB_DBG_LIMIT],options[DB_DBG_OFFSET] });
    }
    r = db_query_err(q,args...);
    if (countflag)
        return get_one_int(r);
    if (!sizeof(r))
        return ({});
    return map( r, (: mkmapping(MY_WISHLIST,$1) :) ); 
}

public mapping get_one_wish(int wl_id)
{
    mapping* result;
    if (!check_security())
        return 0;
    result = get_wishlist(([
        SOUND_OPT_WL_ID:wl_id,
        SOUND_OPT_STATUS: SC_STATUS_DELETED,
        SOUND_OPT_STATUS_BM: 0]),0);
    return (sizeof(result)==1) ? result[0] : 0;
}

public int set_one_wish(mapping new_wish)
{
    if (!check_security() || !mappingp(new_wish))
        return 0;
    int wl_id = new_wish[SOUND_OPT_WL_ID];
    mapping old_wish = get_one_wish(wl_id);
    mixed r,*args=({});
    string *opts=({});
    if (!mappingp(old_wish))
    {
        r = db_query_err("SELECT wl_id FROM wishlist "
            "WHERE (wl_status & ?1)>0 ORDER BY wl_id LIMIT 1",
            SC_STATUS_DELETED);
        if (sizeof(r))
        {
            wl_id = r[0][0];
        }
        else
        {
            r = db_query_err("SELECT MAX(wl_id) FROM wishlist");
            wl_id = sizeof(r)?r[0][0]:0;
            wl_id++;
        }
        if (member(new_wish,SOUND_OPT_FILE))
        {
            db_query_err("INSERT OR REPLACE INTO wishlist "
                "(wl_id,wl_status,wl_priority,wl_from,wl_wish,sf_filename) "
                "VALUES (?,?,?,?,?,?) ",
                wl_id,new_wish[SOUND_OPT_STATUS],new_wish[SOUND_OPT_PRIORITY],
                new_wish[SOUND_OPT_FROM]||"",new_wish[SOUND_OPT_WISH]||"",
                new_wish[SOUND_OPT_FILE]);
        }
        else
        {
            db_query_err("INSERT OR REPLACE INTO wishlist "
                "(wl_id,wl_status,wl_priority,wl_from,wl_wish) "
                "VALUES (?,?,?,?,?) ",
                wl_id,new_wish[SOUND_OPT_STATUS],new_wish[SOUND_OPT_PRIORITY],
                new_wish[SOUND_OPT_FROM]||"",new_wish[SOUND_OPT_WISH]||"");
        }
        return wl_id;
    }

    if (old_wish[SOUND_OPT_STATUS] != new_wish[SOUND_OPT_STATUS])
    {
        opts += ({"wl_status = ? "});
        args += ({new_wish[SOUND_OPT_STATUS]});
    }
    if (old_wish[SOUND_OPT_PRIORITY] != new_wish[SOUND_OPT_PRIORITY])
    {
        opts += ({"wl_priority = ? "});
        args += ({new_wish[SOUND_OPT_PRIORITY]});
    }
    if (old_wish[SOUND_OPT_FROM] != new_wish[SOUND_OPT_FROM])
    {
        opts += ({"wl_from = ? "});
        args += ({new_wish[SOUND_OPT_FROM]});
    }
    if (old_wish[SOUND_OPT_WISH] != new_wish[SOUND_OPT_WISH])
    {
        opts += ({"wl_wish = ? "});
        args += ({new_wish[SOUND_OPT_WISH]});
    }
    if (member(new_wish,SOUND_OPT_FILE))
    {
        if (old_wish[SOUND_OPT_FILE] != new_wish[SOUND_OPT_FILE])
        {
            opts += ({"sf_filename = ? "});
            args += ({new_wish[SOUND_OPT_FILE]});
        }
    }
    else
    {
        opts += ({ "sf_filename = null" });
    }
    if (sizeof(opts)==0)
        return wl_id; // no update
    args += ({wl_id});
    db_query_err("UPDATE wishlist SET "+implode(opts," , ")
            +" WHERE wl_id = ?",args...);
    return wl_id;
}

#define MY_PROPOSALS ({SOUND_OPT_PR_ID,SOUND_OPT_STATUS,SOUND_OPT_FROM,\
    SOUND_OPT_DESCR,SOUND_OPT_FILE,SOUND_OPT_WL_ID})
public <int|mapping*> get_all_proposals(mapping options, int countflag)
{
    string q;
    mixed r,*args=({});
    int whereflag = 0;
    if (!check_security(CHECK_LAST_OBJECT) || !mappingp(options))
    {
        return 0;
    }
    if (countflag)
    {
        q = "SELECT COUNT(*) FROM proposals ";
    }
    else
    {
        q = "SELECT pr_id,pr_status,pr_from,pr_description,sf_filename,wl_id ";
        q+= "FROM proposals ";
    }
    if (member(options,SOUND_OPT_PR_ID))
    {
        q+= whereflag++?"AND ":"WHERE ";
        q+= "pr_id = ? ";
        args += ({options[SOUND_OPT_PR_ID]});
    }
    if (member(options,SOUND_OPT_WL_ID))
    {
        q+= whereflag++?"AND ":"WHERE ";
        q+= "wl_id = ? ";
        args += ({options[SOUND_OPT_WL_ID]});
    }
    if (member(options,SOUND_OPT_FROM))
    {
        q+= whereflag++?"AND ":"WHERE ";
        q+= "pr_from LIKE ? ";
        args += ({options[SOUND_OPT_FROM]});
    }
    if (member(options,SOUND_OPT_STATUS))
    {
        q+= whereflag++?"AND ":"WHERE ";
        if (pointerp(options[SOUND_OPT_STATUS]))
        {
            q+= "pr_status IN ("
                +implode(({"?"})*sizeof(options[SOUND_OPT_STATUS]),",")+") ";
            args += options[SOUND_OPT_STATUS];
        }
        else if (member(options,SOUND_OPT_STATUS_BM))
        {
            q+= "(pr_status & ? )= ? ";
            args += ({ options[SOUND_OPT_STATUS],options[SOUND_OPT_STATUS_BM] });
        }
        else
        {
            q+= "(pr_status & ? )> 0 ";
            args += ({ options[SOUND_OPT_STATUS] });
        }
    }
    else
    {
        q+= whereflag++?"AND ":"WHERE ";
        q+= "(pr_status & ? )= 0 ";
        args += ({ SC_STATUS_DELETED });
    }
    if (member(options,SOUND_OPT_FILE))
    {
        q+= whereflag++?"AND ":"WHERE ";
        q+= "sf_filename = ? ";
        args += ({options[SOUND_OPT_FILE]});
    }
    if (member(options,SOUND_OPT_DESCR))
    {
        q+= whereflag++?"AND ":"WHERE ";
        q+= "pr_description LIKE ? ";
        args += ({options[SOUND_OPT_DESCR]});
    }
    if (!countflag && member(options,DB_DBG_LIMIT))
    {
        q+= "ORDER BY pr_id ASC ";
        q+= "LIMIT ? OFFSET ? ";
        args += ({ options[DB_DBG_LIMIT],options[DB_DBG_OFFSET] });
    }
    r = db_query_err(q,args...);
    if (countflag)
        return get_one_int(r);
    if (!sizeof(r))
        return ({});
    return map( r, (: mkmapping(MY_PROPOSALS,$1) :) ); 
}

public mapping get_one_proposal(int pr_id)
{
    mapping* result;
    if (!check_security())
        return 0;
    result = get_all_proposals(([
        SOUND_OPT_PR_ID:pr_id,
        SOUND_OPT_STATUS: SC_STATUS_DELETED,
        SOUND_OPT_STATUS_BM: 0]),0);
    return (sizeof(result)==1) ? result[0] : 0;
}

public int set_one_proposal(mapping new_proposal)
{
    if (!check_security() || !mappingp(new_proposal))
        return 0;
    int pr_id = new_proposal[SOUND_OPT_PR_ID];
    mapping old_proposal = get_one_proposal(pr_id);
    mixed r,*args=({});
    string *opts=({});
    if (!mappingp(old_proposal))
    {
        r = db_query_err("SELECT pr_id FROM proposals "
            "WHERE (pr_status & ?1)>0 ORDER BY pr_id LIMIT 1",
            SC_STATUS_DELETED);
        if (sizeof(r))
        {
            pr_id = r[0][0];
        }
        else
        {
            r = db_query_err("SELECT MAX(pr_id) FROM proposals");
            pr_id = sizeof(r)?r[0][0]:0;
            pr_id++;
        }
        db_query_err("INSERT OR REPLACE INTO proposals "
            "(pr_id,pr_status,pr_from,pr_description) VALUES (?,?,?,?)",
            pr_id,new_proposal[SOUND_OPT_STATUS],
            new_proposal[SOUND_OPT_FROM]||"",
            new_proposal[SOUND_OPT_DESCR]||"");
        if (!member(new_proposal,SOUND_OPT_WL_ID) 
                && !member(new_proposal,SOUND_OPT_FILE))
            return pr_id;
        old_proposal = get_one_proposal(pr_id);
        old_proposal -= ([SOUND_OPT_WL_ID,SOUND_OPT_FILE]);
    }
    if (old_proposal[SOUND_OPT_STATUS] != new_proposal[SOUND_OPT_STATUS])
    {
        opts += ({"pr_status = ? "});
        args += ({new_proposal[SOUND_OPT_STATUS]});
    }
    if (old_proposal[SOUND_OPT_FROM] != new_proposal[SOUND_OPT_FROM])
    {
        opts += ({"pr_from = ? "});
        args += ({new_proposal[SOUND_OPT_FROM]});
    }
    if (old_proposal[SOUND_OPT_DESCR] != new_proposal[SOUND_OPT_DESCR])
    {
        opts += ({"pr_description = ? "});
        args += ({new_proposal[SOUND_OPT_DESCR]});
    }
    if (member(new_proposal,SOUND_OPT_FILE))
    {
        if (old_proposal[SOUND_OPT_FILE] != new_proposal[SOUND_OPT_FILE])
        {
            opts += ({"sf_filename = ? "});
            args += ({new_proposal[SOUND_OPT_FILE]});
        }
    }
    else
    {
        opts += ({ "sf_filename = null" });
    }
    if (member(new_proposal,SOUND_OPT_WL_ID))
    {
        if (old_proposal[SOUND_OPT_WL_ID] != new_proposal[SOUND_OPT_WL_ID])
        {
            opts += ({"wl_id = ? "});
            args += ({new_proposal[SOUND_OPT_WL_ID]});
        }
    }
    else
    {
        opts += ({ "wl_id = null" });
    }
    if (sizeof(opts)==0)
        return pr_id; // no update
    args += ({pr_id});
    db_query_err("UPDATE proposals SET "+implode(opts," , ")
            +" WHERE pr_id = ?",args...);
    return pr_id;
}

int sql(string sql)
{
    mixed * m;
    if (!check_security(CHECK_ERROR)) {
        return 0;
    }
    if (!TI || !wizp(TI)) {
        return 0;
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

int tablelist(string str)
{
    check_security(CHECK_ERROR);
    string tb = space(str);
    if (tb == "") {
        return sql("SELECT name FROM sqlite_master "
            "WHERE type IN('table','view')");
    } else {
        return sql("SELECT sql FROM sqlite_master WHERE NAME = '"+tb+"'");
    }
}

void create()
{
    "*"::create();
    
    init_security_trust_mudlib();
    add_security_condition((: return adminp($1); :));
    add_security_condition("/apps/property_defs");                                              
    add_security_condition(SC_SOUNDCHECKER+"#");
    init_db();
}

void abort_renewal() {}
void finish_renewal(object neu) {}
void prepare_renewal() {}
