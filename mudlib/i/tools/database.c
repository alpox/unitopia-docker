// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /i/tools/database.c
// Description: Datenbankfunktionen, die die Fehlerbehandlung kapseln.
// Author:      Myonara (11.Apr.2013)

#pragma save_types

#include <database.h>
#include <error.h>
#include <level.h>
#include <message.h>
#include <rtlimits.h>
#ifdef Orbit
#include <properties.h>
#endif

private nosave string dbfile;
private nosave mixed * dbg_buffer = ({ });
private nosave string db_error;
private nosave mapping debuggers = 0; // 0 =load from database
private nosave mapping explainplans = ([]),old_dbg,new_dbg;
private nosave int db_opened = 0;
private nosave int db_transaction_active = 0;
private nosave int db_seq = 0;
private nosave int db_disable_foreign_keys = 0;

protected int db_close();
protected int db_open();
protected void send_to_debuggers(string msg, int acls);
protected int check_debuggers(int acls);
protected varargs void db_debug(string msg, int severity, int flags, 
            string user);
protected int validate_debugger(string rname);


/*
FUNKTION: query_database_description
DEKLARATION: public string query_database_description()
BESCHREIBUNG:
Dient zur Anzeige einer Beschreibung der Datenbank inkl. spezieller 
Funktionen oder Befehle, die zur Wartung da sind. Sollte von jeder
Datenbank ueberlagert werden.
VERWEISE: init_database
GRUPPEN: database
*/
public string query_database_description()
{
    return "(Bitte query_database_description ueberlagern)";
}

/*
FUNKTION: init_database
DEKLARATION: protected varargs int init_database(string databasefile,int disable_foreign_keys)
BESCHREIBUNG:
Initialisiert die Datenbankdatei. Falls die Datenbank geoeffnet ist, 
wird sie erst geschlossen, bevor die neue Datei zugewiesen wird.
Das erbende Objekt braucht Schreibrechte auf Datenbankdatei.
Bei disable_foreign_keys 0 werden bei jedem Oeffnen foreign keys aktiviert,
bei 1 nicht. 
Rueckgabewert:
* 0 wenn kein gueltiger neuer Name angegeben wurde oder Datenbank nicht oeffnen
    kann. Bei diesem Rueckgabewert braucht gar nicht mehr auf die DB 
    zugegriffen werden.
* 1 wenn die Datenbank mit gueltigem Namen das erste Mal aufgerufen wurde.
* -1 wenn init_database schonmal mit gueltigem Namen aufgerufen wurde.
VERWEISE: db_open, db_close, query_database_description
GRUPPEN: database
*/
protected varargs int init_database(string databasefile, 
        int disable_foreign_keys)
{
    int flag = (dbfile == 0) ? 1 : -1;
    db_close();
    if (!stringp(databasefile)) return 0;
    dbfile = databasefile;
    db_disable_foreign_keys = disable_foreign_keys;
    debuggers = 0; // reinit debuggers with new database.
    if (db_open()==0) return 0;
    db_close();
    return flag;
}

/*
FUNKTION: query_db_error
DEKLARATION: public string query_db_error()
BESCHREIBUNG:
Gibt den Fehler der letzten Datenbankoperation zurueck, falls ein Fehler 
vorlag. Lag kein Fehler vor, wird 0 zurueckgegeben.
VERWEISE: db_open, db_close
GRUPPEN: database
*/
public string query_db_error()
{
    return db_error;
}

#ifdef __SQLITE__ 
/*
FUNKTION: db_close
DEKLARATION: protected int db_close()
BESCHREIBUNG:
Falls die Datenbank offen ist, wird sie hiermit geschlossen.
1 bei Erfolg.
VERWEISE: db_open
GRUPPEN: database
*/
protected int db_close()
{
    db_error = 0;
    if(db_opened) 
    {
        db_opened = 0;
        sl_close();
    }
    db_opened = 0;
    return 1;
}

/*
FUNKTION: db_open
DEKLARATION: protected int db_open()
BESCHREIBUNG:
Falls die Datenbank geschlossen ist, wird sie hiermit geoeffnet.
1 bei Erfolg (also offen).
VERWEISE: db_close
GRUPPEN: database
*/
protected int db_open()
{
    int ret;
    db_error = 0;
    if (!stringp(dbfile))
    {
        return 0;
    }
    if (db_opened) 
    {
        return 1;
    }
    if (db_error = catch( ret = sl_open(dbfile);publish )) 
    {
        catch(sl_close(); nolog); // im Falle eines already open.
        return 0;
    }
    if (!db_disable_foreign_keys &&
        db_error = catch( sl_exec("PRAGMA foreign_keys = ON") )) 
    {
        db_debug("sl_exec foreign_keys failed: "+dbfile+ " error:"+db_error,
            DB_DBGLVL_ERROR,DB_DBG_BUFFER_MSG,"DATABASE");
        sl_close();
        return 0;
    }
    db_opened = 1;
    send_to_debuggers(".sl_open: "+dbfile, DB_DBG_ACL_QUERY);
    return ret;
}

/*
NOENZY: db_query_internal
DEKLARATION: private mixed *db_query_internal(int err, string query, varargs mixed *args)
BESCHREIBUNG:
Fuehrt eine query mit Argumenten aus, siehe zb sl_exec.
Bei err = -1 wird kein Fehler ausgegeben, bei 0 ueber db_debug
und bei 1 mit do_error. Ist insb. zur Entkopplung von db_debug
(Rekursion) gedacht.
Bei err = -2 wird sogar kein Logeintrag generiert, weil zb fuer db_check_table.
Bei Fehler wird 0 zurueckgegeben, ansonsten die Daten.
VERWEISE: db_open, sl_exec
GRUPPEN: database
*/
private mixed *db_query_internal(int err, string query, varargs mixed *args)
{
    mixed *data;
    int *ut1,*ut2,*ut3,eval; 
    string uts;
#ifdef Orbit
    ut1 = utime();
    old_dbg = this_object()->query(P_DEBUG_INFO) ||([]);
    new_dbg = deep_copy(old_dbg);
    new_dbg["db_query"] = query;
    new_dbg["db_args"] = mixed2str(args);
    eval = get_eval_cost();
    if (eval <= 65536)
    {
        db_error = "db_query-error: Not enough eval time left for catch(): "
            "required 65536, available "+eval;
        new_dbg["err"] = db_error;
        this_object()->set(P_DEBUG_INFO,new_dbg);
        switch (err)
        {
            case 1:
                do_error(wrap(db_error+"\n"+".q="+query));
                break;

            default:
                db_debug(wrap(db_error+"\n"+".q="+query),
                    DB_DBGLVL_ERROR, DB_DBG_BUFFER_MSG,"DATABASE");
                break;

            case -1:
            case -2:
                break;
        }
        return 0;
    }
    if (!db_open()) 
    {
        return 0;
    }
    if (err == -2)
    {
        db_error = catch( data = sl_exec(query, args ...);nolog );
    }
    else
    {
        db_error = catch( data = sl_exec(query, args ...) );
    }
    if (db_error)
    {
        new_dbg["err"] = db_error;
        ut2 = utime();
        ut3 = ({ ut2[0]-ut1[0], ut2[1]-ut1[1]}); 
        if (ut3[1]<0) 
        {
            ut3[0]--;
            ut3[1]=1000000+ut3[1];
        } 
        uts = sprintf("%d.%06d", ut3[0],ut3[1]);
        new_dbg["uts"] = uts;
        if (strstr(db_error,"sl_exec: SQL query caused")==0)
        {
            if (!member(explainplans,query))
            {
                mixed plan = sl_exec("EXPLAIN QUERY PLAN "+query,args...);
                explainplans[query] = mixed2str(plan);
            }
            new_dbg["explain"] = explainplans[query];
        }
        this_object()->set(P_DEBUG_INFO,new_dbg);
        switch (err)
        {
        case 1:
            do_error(wrap("sl_exec failed: "+query+" error:"+db_error));
            break;
        default:
            db_debug("sl_exec failed: "+query+" error:"+db_error,
                DB_DBGLVL_ERROR, DB_DBG_BUFFER_MSG,"DATABASE");
        case -1:
        case -2:
            break;
        }
        return 0;
    }
    else if (strstr(lower_case(trim(query)),"insert")==0 && !data)
    {
        data = ({ sl_insert_id() });
    }
    return data;

#else

    int dbgquery = check_debuggers(DB_DBG_ACL_QUERY);
    if (dbgquery)
    {
        ut1 = utime();
        send_to_debuggers(
                ".q="+wrap(query)
                +(sizeof(args)?".args="+mixed2str(args):""),
                DB_DBG_ACL_QUERY);
    }
    eval = get_eval_cost();
    if (eval <= 65536)
    {
        db_error = "db_query-error: Not enough eval time left for catch(): "
            "required 65536, available "+eval;
        send_to_debuggers(".err[]="+wrap(db_error), DB_DBG_ACL_QUERY);
        switch (err)
        {
            case 1:
                do_error(wrap(db_error+"\n"+".q="+query));
                break;

            default:
                db_debug(wrap(db_error+"\n"+".q="+query),
                    DB_DBGLVL_ERROR, DB_DBG_BUFFER_MSG,"DATABASE");
                break;

            case -1:
            case -2:
                break;
        }
        return 0;
    }

    if (!db_open()) 
    {
        return 0;
    }
    if (err == -2)
    {
        db_error = catch( data = sl_exec(query, args ...);nolog );
    }
    else
    {
        db_error = catch( data = sl_exec(query, args ...) );
    }
    if (db_error)
    {
        if (dbgquery)
        {
            ut2 = utime();
            ut3 = ({ ut2[0]-ut1[0], ut2[1]-ut1[1]}); 
            if (ut3[1]<0) 
            {
                ut3[0]--;
                ut3[1]=1000000+ut3[1];
            } 
            uts = sprintf("%d.%06d", ut3[0],ut3[1]);
            send_to_debuggers(".err["+uts+"]="+wrap(db_error), DB_DBG_ACL_QUERY);
        }
        switch (err)
        {
        case 1:
            do_error(wrap("sl_exec failed: "+query+" error:"+db_error));
            return 0;
        default:
            db_debug("sl_exec failed: "+query+" error:"+db_error,
                DB_DBGLVL_ERROR, DB_DBG_BUFFER_MSG,"DATABASE");
        case -1:
        case -2:
            return 0;
        }
    }
    else if (strstr(lower_case(trim(query)),"insert")==0 && !data)
    {
        data = ({ sl_insert_id() });
    }
    else if (strstr(lower_case(space(query)),"select")==0 
            && !member(explainplans,query) // bekannte plaene ausblenden
            && check_debuggers(DB_DBG_ACL_EXPLAIN))
    {
        mixed plan = sl_exec("EXPLAIN QUERY PLAN "+query);
        explainplans[query] = mixed2str(plan);
        send_to_debuggers(wrap(query)+explainplans[query],
            DB_DBG_ACL_EXPLAIN);
    }
    if (dbgquery)
    {
        ut2 = utime();
        ut3 = ({ ut2[0]-ut1[0], ut2[1]-ut1[1]}); 
        if (ut3[1]<0) 
        {
            ut3[0]--;
            ut3[1]=1000000+ut3[1];
        } 
        uts = sprintf("%d.%06d", ut3[0],ut3[1]);
        send_to_debuggers(".data["+uts+"]="+mixed2str(data),
                DB_DBG_ACL_QUERY);
    }
    return data;
#endif
}

/*
FUNKTION: db_query
DEKLARATION: protected mixed *db_query(string query, varargs mixed *args)
BESCHREIBUNG:
Fuehrt eine query mit Argumenten aus, siehe zb sl_exec.
Bei Fehler wird 0 zurueckgegeben, ansonsten die Daten.
Falls die Anweisung ein INSERT ist, so wird die Row-ID der zuletzt
eingefügten Zeile in einem Array zurückgegeben.
VERWEISE: db_open, sl_exec
GRUPPEN: database
*/
protected mixed *db_query(string query, varargs mixed *args)
{
    mixed *data = db_query_internal(0, query, args ...);
#ifdef Orbit
    this_object()->set(P_DEBUG_INFO,old_dbg);
#endif
    return data;
}

/*
FUNKTION: db_rollback
DEKLARATION: protected int db_rollback()
BESCHREIBUNG:
Macht alle db_query als eine Transaktion ab db_begin rueckgaengig, siehe dort.
VERWEISE: db_begin, db_commit
GRUPPEN: database
*/
protected int db_rollback()
{
    if (db_transaction_active) 
    {
        db_query("ROLLBACK TRANSACTION");
        db_transaction_active = 0;
        if (db_error) 
        {
            return 0;
        }
    }
    return 1;
}

/*
FUNKTION: db_commit
DEKLARATION: protected int db_commit()
BESCHREIBUNG:
Die Transaktion (alle db_query's) ab dem letzten db_begin werden nun in die
Datenbank umgesetzt. Das dient zum konsistenten Multi-Table-Update.
VERWEISE: db_begin, db_rollback.
GRUPPEN: database
*/
protected int db_commit()
{
    if (db_transaction_active) 
    {
        db_query("COMMIT TRANSACTION");
        db_transaction_active = 0;
        if (db_error) 
        {
            return 0;
        }
    }
    return 1;
}

/*
FUNKTION: db_begin
DEKLARATION: protected int db_begin()
BESCHREIBUNG:
Eine Transaktion wird begonnen. Das dient zum konsistenten Multi-Table-Update.
VERWEISE: db_commit, db_rollback
GRUPPEN: database
*/
protected int db_begin()
{
    if (!db_transaction_active) 
    {
        db_query("BEGIN TRANSACTION");
        db_transaction_active = 1;
        if (db_error) 
        {
            return 0;
        }
    }
    return 1;
}

/*
FUNKTION: db_query_err
DEKLARATION: protected mixed *db_query_err(string query, varargs mixed *args)
BESCHREIBUNG:
Fuehrt eine query mit Argumenten aus, siehe zb sl_exec.
Bei Fehler wird bei einer Transaktion automatisch ein Rollback initiiert
und dieser Fehler geworfen, ansonsten die Daten zurueckgegeben.
VERWEISE: db_open, sl_exec
GRUPPEN: database
*/
protected mixed *db_query_err(string query, varargs mixed *args)
{
    mixed * data = db_query_internal(0, query, args ...);
    string err = db_error;
    if(err)
    {
        err = err[0] == '*' ? err[1..] : err;
        err+= "\n.query: "+query+"\.nargs: "+mixed2str(args)+"\n";
        if (db_transaction_active)
        {
            db_rollback();// setzt db_error zurueck!
            db_debug(err, DB_DBGLVL_ERROR, DB_DBG_BUFFER_MSG,"DATABASE");
        }
        raise_error(err);
    }
#ifdef Orbit
    this_object()->set(P_DEBUG_INFO,old_dbg);
#endif
    return data;
}


/*
FUNKTION: db_escape_string
DEKLARATION: protected string db_escape_string(string s)
BESCHREIBUNG:
Soll ein String ueber db_query gespeichert werden, muessen enthaltene Zeichen,
hier das ' geschuetzt werden.

ACHTUNG: Diese Funktion sollte nicht (mehr) verwendet werden. Strings können
         problemlos als Argumente an das db_query() übergeben werden.

         Unbekannte Strings sollten aus Performance- und Sicherheitsgründen
         nie direkt in einen SQL-Befehl eingebettet werden!

VERWEISE: db_query,db_query_err,convert_array_to_sql_list
GRUPPEN: database
*/
deprecated protected string db_escape_string(string s)
{
  return regreplace(s||"","'","''",1);
}

/*
FUNKTION: convert_array_to_sql_list
DEKLARATION: protected string convert_array_to_sql_list(mixed list)
BESCHREIBUNG:
Fuer die Syntax WHERE key IN ('wert1','wert2'...) wird aus dem Parameter list
eine solche SQL-Liste erzeugt. Bei Strings wird noch ein ' vor und danach
angestellt und es durch db_escape_string gejagt. Alles uebrige wird einfach
nur gelistet.

ACHTUNG: Diese Funktion ist veraltet (siehe db_escape_string)

VERWEISE: db_escape_string, convert_array_to_insert_list
GRUPPEN: database
*/
deprecated protected string convert_array_to_sql_list(mixed list)
{
    string *istr;
    if (pointerp(list) && sizeof(list))
    {
        istr = map(list,function string(mixed line) {
            if (stringp(line))
            {
                return "'"+db_escape_string(line)+"'";
            }
            else
            {
                return to_string(line);
            }
        });
        return "("+liste(istr,", ")+" ) ";
    }
    if (stringp(list))
    {
        return "('"+db_escape_string(list)+"') ";
    }
    return 0;
}

/*
FUNKTION: convert_array_to_insert_list
DEKLARATION: protected string convert_array_to_insert_list(mixed list)
BESCHREIBUNG:
Fuer die Syntax INSERT INTO table (...) VALUES (...),(...)
wird in dieser Funktion alles nach VALUES erzeugt.

ACHTUNG: Diese Funktion ist veraltet (siehe db_escape_string)

VERWEISE: db_escape_string, convert_array_to_sql_list
GRUPPEN: database
*/
deprecated protected string convert_array_to_insert_list(mixed list)
{
    string *istr;
    if (pointerp(list) && sizeof(list))
    {
        istr = map(list,function string(mixed line) {
            if (stringp(line))
            {
                return "('"+db_escape_string(line)+"')";
            }
            else if (pointerp(line))
            {
                string lstr = convert_array_to_sql_list(line);
                if (lstr==0) return 0;
                return lstr;
            }
            else
            {
                return "("+to_string(line)+")";
            }
        });
        if (sizeof(istr-({0}))<sizeof(istr))
            return 0; // Interner Fehler.
        return liste(istr,", ")+" ";
    }
    if (stringp(list))
    {
        return "('"+db_escape_string(list)+"') ";
    }
    return 0;
}


/*
FUNKTION: get_one_int
DEKLARATION: protected int get_one_int(mixed * result)
BESCHREIBUNG:
wertet die Daten aus und erwartet eine Integer, 0 wenn keine Integer vorhanden
ist. 
VERWEISE: get_one_string
GRUPPEN: database
*/
protected int get_one_int(mixed * result)
{
    if (pointerp(result) && sizeof(result) == 1 &&
        pointerp(result[0]) && sizeof(result[0])==1 &&
        intp(result[0][0]) )
    {

        return result[0][0];
    }
    // Ist nicht unbedingt ein Fehler, wenn das Ergebnis nicht vorhanden ist!
    send_to_debuggers(".Result is not a ({ ({ <int> }) }) as expected.", 
        DB_DBG_ACL_QUERY);
    return 0;
}

/*
FUNKTION: get_one_string
DEKLARATION: protected string get_one_string(mixed * result)
BESCHREIBUNG:
Gibt einen String zurueck, wenn dieser in den Daten enthalten ist.
VERWEISE: get_one_int
GRUPPEN: database
*/
protected string get_one_string(mixed * result)
{
    if (pointerp(result) && sizeof(result) == 1 &&
        pointerp(result[0]) && sizeof(result[0])==1 &&
        stringp(result[0][0]) ) 
    {

        return result[0][0];
    }
    // Ist nicht unbedingt ein Fehler, wenn das Ergebnis nicht vorhanden ist!
    send_to_debuggers(".Result is not a ({ ({ <string> }) }) as expected.", 
        DB_DBG_ACL_QUERY);
    return 0;
}

/*
FUNKTION: check_sl_rc
DEKLARATION: protected int check_sl_rc(mixed result) 
BESCHREIBUNG:
Prueft, ob es mind. eine Zeile in result gibt.
VERWEISE: get_one_int
GRUPPEN: database
*/
protected int check_sl_rc(mixed arr) 
{
  if (!pointerp(arr)
      || !pointerp(arr[0])
      || !sizeof(arr[0]))
    return 0;
  return 1;
}


/*
FUNKTION: select_value
DEKLARATION: protected varargs mixed select_value(mixed arr, int row, int col)
BESCHREIBUNG:
Gibt einen bestimmten Wert zurueck.
VERWEISE: get_one_int, select_row,select_column
GRUPPEN: database
*/
protected varargs mixed select_value(mixed arr, int row, int col) 
{
  if (!check_sl_rc(arr))
    return 0;
  return arr[row][col];
}

/*
FUNKTION: select_row
DEKLARATION: protected varargs mixed *select_row(mixed arr, int row) 
BESCHREIBUNG:
Gibt eine bestimme Reihe des Ergebnisses zurueck.
VERWEISE: get_one_int, select_value,select_column
GRUPPEN: database
*/
protected varargs mixed *select_row(mixed arr, int row) 
{
  if (!check_sl_rc(arr))
    return ({});
  return arr[row];
}

/*
FUNKTION: select_column
DEKLARATION: protected varargs mixed *select_column(mixed arr, int col)
BESCHREIBUNG:
Gibt eine bestimme Reihe des Ergebnisses zurueck.
VERWEISE: get_one_int, select_value,select_row
GRUPPEN: database
*/
protected varargs mixed *select_column(mixed arr, int col) 
{
  mixed *rc = ({});;

  if (!check_sl_rc(arr))
    return ({});
  for (int i=0; i<sizeof(arr); i++)
    rc += ({ arr[i][col] });
  return rc;
}

/*
FUNKTION: db_check_table
DEKLARATION: protected int db_check_table(string tablename)
BESCHREIBUNG:
Prueft per SELECT COUNT(*) FROM <tablename>, ob es die Tabelle gibt.
Ruecke >=0 fuer die Anzahl der Zeilen, -1 bei Fehler.
Ein Ddebugeintrag wird dafuer nicht generiert.
VERWEISE: db_query
GRUPPEN: database
*/
protected int db_check_table(string tablename)
{
#if 1
    mixed * result = db_query_internal(-2,//suppress all errors here.
           "SELECT 1 FROM " + tablename + " LIMIT 1");
    if (db_error)
        return -1;
    return 1;
#else
    mixed * result = db_query_internal(-2,//suppress all errors here.
        "SELECT COUNT(*) FROM sqlite_master WHERE name = ?1",tablename);
#ifdef Orbit
    this_object()->set(P_DEBUG_INFO,old_dbg);
#endif
    if (db_error || !sizeof(result) || result[0][0] != 1)
    {
        return -1;
    }
    return 1;
#endif
}


/*
FUNKTION: db_debug
DEKLARATION: protected varargs void db_debug(string msg, int severity, int flags, string user)
BESCHREIBUNG:
Schreibt eine Meldung ins Debuglog der Datenbank (reservierte Tabelle 
db_debug_log). 
msg beinhaltet die Debug oder Fehlermeldung, die gespeichert werden soll.
Folgende Debuglevel sind fuer severity  in /sys/database.h definiert: 
DB_DBGLVL_DEBUG, DB_DBGLVL_INFO, DB_DBGLVL_WARNING, DB_DBGLVL_ERROR
Folgende Flags sind moeglich:
- DB_DBG_BUFFER_MSG     speichere die Debugmeldung im internen Puffer und
                        noch nicht in der Datenbanktabelle. Das erlaubt es
                        Debuginformationen nur bei Fehlern zu speichern.
- DB_DBG_FLUSH_BUFFER   schreibe existiernen Puffer komplett in die Datenbank
- DB_DBG_DELETE_BUFFER  loesche den internen Puffer.
- DB_DBG_CREATE         lege die Tabelle db_debug_log an, falls sie noch nicht
                        existiert. Sollte vor der Anlage anderer Tabellen
                        gemacht werden.
VERWEISE: query_all_dbg_messages
GRUPPEN: database
*/
protected varargs void db_debug(string msg, int severity, int flags, 
            string user)
{
    if (flags & DB_DBG_CREATE) 
    {
        if (!db_open()) 
        {
            raise_error("Datenbank konnte nicht geöffnet werden: "+dbfile);
        }
        db_query_internal(-2,"SELECT COUNT(*) FROM db_debug_log");
        if (query_db_error()) 
        {
            db_query_internal(1,"CREATE TABLE IF NOT EXISTS db_debug_log ("
                "timestamp INTEGER, seq INTEGER, "
                "message TEXT, severity INTEGER, user TEXT)");
            if (query_db_error()) 
            {
                send_to_debuggers("Tabelle db_debug_log konnte nicht "
                        "erstellt werden", DB_DBG_ACL_DB_DEBUG);
                raise_error("Tabelle db_debug_log konnte nicht "
                        "erstellt werden");
            }
        } 
    }
    if (flags & DB_DBG_DELETE_BUFFER) 
    {
        dbg_buffer = ({ });
    }
    
    if (stringp(msg)){
        send_to_debuggers(
            sprintf("db_debug(%d,%s):\n%s",severity,user||"",msg),
            DB_DBG_ACL_DB_DEBUG);
        if ( (flags & DB_DBG_BUFFER_MSG) || sizeof(dbg_buffer)) 
        {
            // puffern.
            dbg_buffer += ({ ({ time(), ++db_seq, msg, severity, user }) });
        } 
        else 
        {
            // direkt schreiben.
            db_query_internal(-1,"INSERT INTO db_debug_log "
                "(timestamp,seq,message,severity,user) "
                "VALUES (?,?,?,?,?)",time(),++db_seq,msg,severity,
                    user||"SYSTEM");
            return;
        }
    }
    if ( sizeof(dbg_buffer) > 0 && ((flags & DB_DBG_FLUSH_BUFFER)
            ||sizeof(dbg_buffer)>(query_limits()[LIMIT_ARRAY]/10) ) )
    {
        foreach (mixed* dbg_buffer_entry: dbg_buffer)
            db_query_internal(-1,
                "INSERT INTO db_debug_log (timestamp,seq,message,severity,user) "
                    "VALUES (?, ?, ?, ?, ?)",
                dbg_buffer_entry...);
        dbg_buffer = ({ }); 
        return;
    }
}

/*
FUNKTION: query_all_dbg_messages
DEKLARATION: protected varargs string * query_all_dbg_messages(int flags)
BESCHREIBUNG:
wenn flags DB_DBG_BUFFER_MSG enthaelt, wird der interne Puffer ausgegeben.
Bei DB_DBG_FLUSH_BUFFER werden die letzten maximal 1000 Zeilen aus der 
Datenbank geladen und umgekehrt sortiert, sprich die Neueste zuerst 
zurueckgegeben.
VERWEISE: db_debug
GRUPPEN: database
*/
protected varargs string * query_all_dbg_messages(int flags)
{
    mixed result;
    if (flags & DB_DBG_BUFFER_MSG || (flags ==0 && sizeof(dbg_buffer))) 
    {
        return map(dbg_buffer, 
                (: shorttimestr($1[0])+" "+$1[1]+" "+$1[2]
                                +" ("+$1[3]+","+$1[4]+")" :));
    }
    if (flags & DB_DBG_FLUSH_BUFFER || flags == 0) 
    {
        if (!db_open()) 
            return 0;
        result = db_query_internal(-1,
            "SELECT timestamp,seq,message,severity,user "
            "FROM db_debug_log ORDER BY timestamp DESC,seq DESC LIMIT 1000");
        if (result == 0) 
            return 0;
        return map(result, 
                (: shorttimestr($1[0])+" "+$1[1]+" "+$1[2]
                                +" ("+$1[3]+","+$1[4]+")" :));
    }
}

/*
FUNKTION: query_dbg_messages
DEKLARATION: public varargs string * query_dbg_messages(mapping options)
BESCHREIBUNG:
wenn options[DB_DBG_FLAGS] DB_DBG_BUFFER_MSG enthaelt, wird der interne 
Puffer ausgegeben. Bei DB_DBG_FLUSH_BUFFER werden die letzten 
maximal 100 Zeilen aus der Datenbank geladen und umgekehrt sortiert, 
sprich die Neueste zuerst zurueckgegeben.
Die Auswahl kann mit DB_DBG_START_TIME zeitlich eingegrenzt werden.
Die Anzahl Zeilen kann mit DB_DBG_LINES und der Offset mit DB_DBG_OFFSET
festgelegt werden. Bei Angabe von DB_DBG_COUNT wird die aktuelle Auswahl
durchgezaehlt.
VERWEISE: db_debug
GRUPPEN: database
*/
public varargs <int|string *> query_dbg_messages(mapping options)
{
    mixed result;
    int flags = options[DB_DBG_FLAGS];
    int limit = options[DB_DBG_LIMIT];
    int offset = options[DB_DBG_OFFSET];
    int starttime = options[DB_DBG_START_TIME];
    if (!this_interactive() && member(DB_DBG_VALID_PO, 
        program_name(previous_object()))!=-1)
    {
        // nichts tun.
    }
    else if (!this_interactive() 
            || !validate_debugger(this_interactive()->query_real_name())) 
    {
        return 0;
    }
    if (limit <= 0) limit = 100;
    if (flags & DB_DBG_BUFFER_MSG || (flags ==0 && sizeof(dbg_buffer))) 
    {
        if (member(options,DB_DBG_COUNT))
        {
            return sizeof(dbg_buffer);
        }
        return map(dbg_buffer, 
                (: shorttimestr($1[0])+" "+$1[1]+" "+$1[2]
                                +" ("+$1[3]+","+$1[4]+")" :));
    }
    if (flags & DB_DBG_FLUSH_BUFFER || flags == 0) 
    {
        if (!db_open()) 
            return 0;
        if (flags & DB_DBG_FLUSH_BUFFER)
            db_debug(0,0, DB_DBG_FLUSH_BUFFER, 0);
        if (member(options,DB_DBG_COUNT))
        {
            result = db_query_internal(-1,
                "SELECT COUNT(*) FROM db_debug_log WHERE timestamp > "
                    +starttime);
            return pointerp(result) ? get_one_int(result) : 0;
        }
        result = db_query_internal(-1,
            "SELECT timestamp,seq,message,severity,user "
            "FROM db_debug_log "
            "WHERE timestamp > "+starttime+" "
            "ORDER BY timestamp DESC,seq DESC "
            "LIMIT "+limit+" OFFSET "+offset);
        if (result == 0) 
            return 0;
        return map(result, 
                (: shorttimestr($1[0])+" "+$1[1]+" "+$1[2]
                                +" ("+$1[3]+","+$1[4]+")" :));
    }
}

/*
FUNKTION: setup_counter
DEKLARATION: protected int setup_counter(string idstr, int initial_value))
BESCHREIBUNG:
Die Funktion legt die Tabelle db_counter an, falls sie noch nicht existiert,
und speichert dort den idstr als Zaehler mit dem initial_value als 
Startwert. Bei fehler wird -1 zurueckgegeben, ansonsten 1.
VERWEISE: increment_counter
GRUPPEN: database
*/
protected int setup_counter(string idstr, int initial_value)
{
    if (!stringp(idstr) || !db_open()) 
    {
        return -1;
    }
    db_query_internal(-2,"SELECT value FROM db_counter "
        "WHERE key ='"+idstr+"'");
    if (db_error) 
    {
        db_query_internal(1,"CREATE TABLE IF NOT EXISTS db_counter ("
                 "key TEXT CONSTRAINT pk_db_counter PRIMARY KEY, "
                 "value INTEGER NOT NULL DEFAULT 0)");
        if (db_error) 
            return -1;
    }
    db_query("INSERT OR REPLACE INTO db_counter (key, value) "
             "VALUES ('"+idstr+"',"+initial_value+")");
    return 1;
}


/*
FUNKTION: increment_counter
DEKLARATION: protected int increment_counter(string idstr)
BESCHREIBUNG:
Zaehlt einen Zaehler um 1 hoch und gibt den neuen Wert zurueck.
Wenn der Zaehler nicht existiert, wird setup_counter genutzt.
Im Fehlerfall wird -1 zurueckgegeben.
VERWEISE: setup_counter
GRUPPEN: database
*/
protected int increment_counter(string idstr)
{
    mixed result;
    if (!stringp(idstr) || !db_open())      
    {
        return -1;
    }
    result = db_query_internal(-2,"SELECT value FROM db_counter "
        "WHERE key ='"+idstr+"'");
    if (db_error || result == 0) 
    {
        if (setup_counter(idstr,0)==-1) 
            return -1;
        result = ({ ({ 0 }) });
    }
    db_query("UPDATE db_counter SET value = value + 1 "
        "WHERE key = '"+idstr+"'");
    if (db_error) 
        return -1;
    int neue_id = get_one_int(result)+1;
    if (db_error) 
        return -1;
    return neue_id;
}

/*
FUNKTION: get_db_info
DEKLARATION: protected string get_db_info(string key)
BESCHREIBUNG:
Liefert einen in der Tabelle db_info unter key gespeicherten string zurueck,
0 sonst.
VERWEISE: set_db_info
GRUPPEN: database
*/
protected string get_db_info(string key)
{
    mixed result;
    if (!stringp(key) || !db_open()) 
    {
        return 0;
    }
    result = db_query_internal(-2,"SELECT value FROM db_info "
        "WHERE key ='"+key+"'");
    if (db_error || result == 0) 
        return 0;
    return get_one_string(result);
}

/*
FUNKTION: set_db_info
DEKLARATION: protected int set_db_info(string key, string value)
BESCHREIBUNG:
Setzt einen Wert value unter dem Schluessel key in der Tabelle db_info.
Legt die Tabelle db_info an, wenn sie noch nciht existiert.
Gibt 0 zurueck bei Fehler, 1 im Erfolgsfall.
VERWEISE: get_db_info
GRUPPEN: database
*/
protected int set_db_info(string key, string value)
{
    mixed result;
    if (!stringp(key) || !db_open()) 
    {
        return 0;
    }
    result = db_query_internal(-2,"SELECT value FROM db_info "
        "WHERE key ='"+key+"'");
    if (db_error || result == 0) {
        db_query_internal(1,"CREATE TABLE IF NOT EXISTS db_info ("
             "key TEXT CONSTRAINT pk_db_info PRIMARY KEY, "
             "value TEXT)");
    }
    db_query("INSERT OR REPLACE INTO db_info (key, value) "
             "VALUES ('"+key+"','"+value+"')");
    if (db_error) 
        return 0;
    return 1;
}

/*
FUNKTION: get_db_version
DEKLARATION: protected int get_db_version()
BESCHREIBUNG:
Liefert die aktuelle Version des Datenbankschemas
(SQLite User-Version).
VERWEISE: set_db_version
GRUPPEN: database
*/
protected int get_db_version()
{
    mixed result;

    if (!db_open())
        return 0;

    result = db_query_internal(-2, "PRAGMA user_version");
    return result && result[0][0];
}

/*
FUNKTION: set_db_version
DEKLARATION: protected void set_db_version(int ver)
BESCHREIBUNG:
Setzt die aktuelle Version des Datenbankschemas
(SQLite User-Version).
VERWEISE: get_db_version
GRUPPEN: database
*/
protected void set_db_version(int ver)
{
    mixed result;

    if (!db_open())
        return 0;

    db_query_internal(-2, sprintf("PRAGMA user_version = %d", ver));
}

/*
FUNKTION: check_debuggers
DEKLARATION: protected int check_debuggers(int acls)
BESCHREIBUNG:
Prueft, ob debuggers mit den aktuellen acls ueberhaupt zuhoeren:
1 wenn ja, 0 wenn nein.
Folgende defines aus database.h werden unterstuetzt:
DB_DBG_ACL_DB_DEBUG,DB_DBG_ACL_DEBUGLOG,DB_DBG_ACL_EXPLAIN,DB_DBG_ACL_OTHER,
DB_DBG_ACL_QUERY
VERWEISE: send_to_debuggers
GRUPPEN: database
*/
protected int check_debuggers(int acls)
{
    if (!stringp(dbfile)) return 0;
    if (!mappingp(debuggers))
    {
        debuggers = ([]);
        if (db_check_table("db_debuggers")==-1)
        {
            return 0; // Ohne Tabelle keine Debugger.
        }
        mixed r = db_query_internal(-1,
                "SELECT debugger,acl FROM db_debuggers");
        if (!sizeof(r)) 
        {
            return 0; // keiner eingetragen
        }
        r = transpose_array(r);
        debuggers = mkmapping(r[0],r[1]);
    }
    return sizeof(filter(
        debuggers, (: find_player($1) && ($2 & $3) > 0 :), acls) );
}

/*
FUNKTION: send_to_debuggers
DEKLARATION: protected void send_to_debuggers(string msg, int acls)
BESCHREIBUNG:
Prueft, ob debuggers mit den aktuellen acls ueberhaupt zuhoeren:
1 wenn ja, 0 wenn nein.
Folgende defines aus database.h werden fuer acls unterstuetzt:
DB_DBG_ACL_DB_DEBUG (1) Ausgaben an database->db_debug...
DB_DBG_ACL_DEBUGLOG (2) Ausgaben an debuglog_db->debuglog...
DB_DBG_ACL_EXPLAIN  (4) Fuer SELECT Abfragen wird ein Ausfuehrungsplan angegeben
DB_DBG_ACL_OTHER    (8) Andere applikationsinterne send_to_debuggers...
DB_DBG_ACL_QUERY    (16) Jede Datenbankabfrage wird ausgegeben.
VERWEISE: check_debuggers
GRUPPEN: database
*/
protected void send_to_debuggers(string msg, int acls)
{
    if (!stringp(dbfile)) return;
    if (!mappingp(debuggers))
    {
        debuggers = ([]);
        if (db_check_table("db_debuggers")==-1)
        {
            return; // Ohne Tabelle keine Debugger.
        }
        mixed r = db_query_internal(-1,
                "SELECT debugger,acl FROM db_debuggers");
        if (!sizeof(r))
        {
            return; // keiner da
        }
        r = transpose_array(r);
        debuggers = mkmapping(r[0],r[1]);
    }
    if (!sizeof(debuggers)) return; // Ohne debugger keine aktion.
    object * receivers = map(m_indices(filter(
        debuggers, (: ($2 & $3) > 0 :), acls)),
        (: find_player($1) :) ) - ({0});
    if (!sizeof(receivers)) return; // fuer diese acls keiner da...
    receivers[0]->send_message_to(receivers, MT_DEBUG, MA_UNKNOWN, msg);
}

/*
FUNKTION: validate_debugger
DEKLARATION: protected int validate_debugger(string rname)
BESCHREIBUNG:
Diese Funktion wird in update_debugger verwendet, um den Debugger zuzulassen.
Per Default werden Admins zugelassen, aber die Funktion ist zum Ueberlagern
und anpassen gedacht.
VERWEISE: update_debugger
GRUPPEN: database
*/
protected int validate_debugger(string rname)
{
    return (member(ADMINS,rname)!=-1);
}

/*
FUNKTION: get_debuggers
DEKLARATION: public string* get_debuggers()
BESCHREIBUNG:
Gibt die Debugger mit aktuellen Debugsettings in Klartext zurueck,
Ein Debugger pro Zeile, alphabetisch sortiert.
VERWEISE: validate_debugger
GRUPPEN: database
*/
public string* get_debuggers()
{
    string * sdbg = sort_array(m_indices(debuggers||([])), #'>);//');
    string dbg,line,*lines=({});
    int acl,aclbit;
    foreach (dbg : sdbg)
    {
        acl = debuggers[dbg];
        line = dbg+": ";
        aclbit = 1;
        while (aclbit > 0 && aclbit <= DB_DBG_ACL_QUERY)
        {
            switch (acl & aclbit)
            {
                default: break;
                case 0: break;
                case DB_DBG_ACL_DB_DEBUG: line+="DB-Debug,"; break;
                case DB_DBG_ACL_DEBUGLOG: line+="Appl-Debug,"; break;
                case DB_DBG_ACL_EXPLAIN:  line+="Ausfuehrungsplan,"; break;
                case DB_DBG_ACL_OTHER:    line+="Sonstige,";break;
                case DB_DBG_ACL_QUERY:    line+="Abfragen,";break;
            }
            aclbit <<= 1;
        }
        lines += ({ line[..<2] });
    }
    return lines;
}

/*
FUNKTION: update_debugger
DEKLARATION: public int update_debugger(string rname,int acls)
BESCHREIBUNG:
Fuegt einen debugger der Datenbank hinzu (acls != 0) oder entfernt ihn.
Es koennen nur Gottchars eingetragen werden, die ueber validate_debugger
zugelassen werden.
Folgende defines aus database.h werden fuer acls unterstuetzt:
DB_DBG_ACL_DB_DEBUG (1) Ausgaben an database->db_debug...
DB_DBG_ACL_DEBUGLOG (2) Ausgaben an debuglog_db->debuglog...
DB_DBG_ACL_EXPLAIN  (4) Fuer SELECT Abfragen wird ein Ausfuehrungsplan angegeben
DB_DBG_ACL_OTHER    (8) Andere applikationsinterne send_to_debuggers...
DB_DBG_ACL_QUERY    (16) Jede Datenbankabfrage wird ausgegeben.
VERWEISE: validate_debugger, check_debuggers, send_to_debuggers
GRUPPEN: database
*/
public int update_debugger(string rname,int acls)
{
    int val;
    if (!stringp(rname) || !wizplayerp(rname) ||!stringp(dbfile))
    {
        return 0;
    }
    if (! (val = validate_debugger(rname))) acls = 0; // Immer austragen...
    if (db_check_table("db_debuggers")==-1)
    {
        db_query_internal(1,"CREATE TABLE IF NOT EXISTS db_debuggers ("
             "debugger TEXT CONSTRAINT pk_db_info PRIMARY KEY, "
             "acl INTEGER)");
        debuggers = ([]);
    }
    if (acls)
    {
        debuggers[rname] = acls;
        db_query_internal(-1,"INSERT OR REPLACE INTO db_debuggers "
            "(debugger, acl) VALUES (?,?)",rname,acls);
    }
    else
    {
        m_delete(debuggers,rname);
        db_query_internal(-1,"DELETE FROM db_debuggers WHERE debugger = ? ",
            rname);
    }
    if (db_error || !val) 
        return 0;
    return 1;        
}

/*
FUNKTION: flush_explain_plans
DEKLARATION: public void flush_explain_plans()
BESCHREIBUNG:
Loescht die zwischengespeicherten Ausfuehrungsplaene.
VERWEISE: update_debugger
GRUPPEN: database
*/
public void flush_explain_plans()
{
    explainplans = ([]);
}

#endif