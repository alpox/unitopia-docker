// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/i/money/serializer.c
// Description: Fuer Nicht-Bankraeume Schliessfach konforme Objekte
//              in mapping und zurueck wandeln. 
// Bitte vor Verwendung mit den Admins abklären.
//  die müssen das erbende Objekt bei zentralbank->is_valid_serializer
//  eintragen!
// Author:	Myonara

#include <database.h>
#include <misc.h>
#include <move.h>
#include <object_info.h>
#include <properties.h>
#include <shadow.h>

#include <money.h>

// eingefuehrt, da program_name replace_program nicht erwischt :(
protected string get_program_name(object ob)
{
    string fn = ob->query_file_name();
    if (fn) return fn;
    string *name = explode(object_name(ob),"#");
    return (sizeof(name)==2)?name[0]:0;
}

private mapping get_one_detail(object what)
{
    int container_flag = 0,reset_flag = 0;
    mapping detail = ([]);
    container_flag = what->query_container();
    if (   container_flag  && what->query_con_close()
        && !what->query_transparent())
    {
        reset_flag = 1;
        what->set_transparent(1);
    }
    detail = ([
       // "environment" : TO,
          // "origin"   : what,
        "debug_info" : (what->query_debug_info()||([]))+
                         (["original" : object_name(what) ]),
          "prep"     : "in",
//        "name"     : lower_case(wer(what,ART_BLANK|ART_NO_ADJEKTIV)),
          "name"     : what->query_name(),
          "cap_name" : what->query_cap_name(),
          "gender"   : what->query_gender(),
          "adjektiv" : what->query_adjektiv(),
          "id"       : (what->query_id()||({}))+({"relief"}),
          "short"    : what->query_short(TP),
          "long"     : (container_flag ? TP->query_object_description(what)
                         : what->query_long(TP)),
          "look_msg" : "",
          "read_msg" : "",
          "take_msg" : "",
          "feel_msg" : "",
          "smell_msg": "",
          "hear_msg" : "",

          ]);
    if (reset_flag)
    {
        what->set_transparent(0);
    }
    foreach(string key : ({ "seher_info", }))
    {
        string tmp;
        if(tmp=call_other(what,"query_"+key))              
        {
            detail += ([ key : tmp ]);
        }
    }
    foreach(string key : ({ "eigen", "personal", "invis", "plural", }))
    {
        int tmp;
        if(tmp=call_other(what,"query_"+key))              
        {
            detail += ([ key : tmp ]);
        }
    }
    foreach(string key : ({ "menge", }))
    {
        mixed tmp;
        if(tmp=call_other(what,"query_"+key))              
        {
            detail += ([ key : tmp ]);
        }
    }
    return detail;
}


private varargs <mapping|string> get_einlagerungsdaten(
            object ob,mapping check_attributes)
{
    object sh;
    <string|int> str;
    string factory,identifier;
    mapping * sh_data,m_cons,m_origin,m_constraints,ob_properties;
    mixed ob_data,mud_data;
    int count;
    if (stringp(str = ZENTRALBANK->precheck_conservation(ob,0,check_attributes)))
    {
        return str;
    }
    if (!intp(str) || str != 1)
    {
        ob->abort_conservation();
        return "Interner Fehler precheck";// never happen
    }
    m_cons = ob->query(P_CONSERVATION);
    factory = m_cons[P_CONSERVATION_FACTORY];
    identifier = m_cons[P_CONSERVATION_IDENTIFIER];
    m_origin = ob->query(P_ORIGIN); // Orginal verwenden falls vorhanden
    switch (sizeof(m_origin))
    {
        case 0:
            m_origin = ([ 
                P_ORIGIN_ROOM : ob->query_first_room(),
                P_ORIGIN_PLAYER : ob->query_first_player(),
                P_ORIGIN_CREATED_ON : object_time(ob),
                P_ORIGIN_CREATOR: ob->query_creator(),
                ]);
            break;
        case 3:
            m_origin[P_ORIGIN_CREATOR] = ob->query_creator();
        case 4:
        case 5:
            break;
    }
    sh_data = ({});
    for (count=0,sh=ob;sh=object_info(sh, OI_SHADOW_NEXT);count++)
    {
        mixed m = sh->query_conservation_arg_sh(sh);
        string pn = get_program_name(sh);
        if (m && pn)
        {
            sh_data += ({ ([ pn : m ]) });
        }
    }
    if (sizeof(sh_data)!=count)
    {
        ZB_DEBUGLOG(sprintf("shadows-reject=%Q,%d,%Q",ob,count,sh_data),
            DB_DBGLVL_INFO,0,"schließfach:serializer");
        ob->abort_conservation();
        return "Eine Art Fluch verhindert aktuell leider das Einlagern.";
    }
    if (stringp(factory))
    {
        ob_data = touch(factory)->get_conservation_data(ob);
    }
    else if (function_exists("query_conservation_arg",ob))
    {
        ob_data = ob->query_conservation_arg();
    }
    else
    {
        ob_data = 1;
    }
    m_constraints = ob->query_conservation_constraints();
    mud_data = ob->query_conservation_data();
    if (sizeof(m_constraints)>0 || !ob_data || !mud_data)
    {
        ZB_DEBUGLOG(sprintf("object-reject=%Q",ob),
            DB_DBGLVL_INFO,0,"schließfach:einlagern");
        ob->abort_conservation();
        return Der(ob)+" lässt sich prinzipiell nicht einlagern.";
    }
    sh_data = filter(sh_data, function int (mapping m)
        {
            string key;
            mixed data;
            foreach (key,data : m)
            {
                if (data==0) return 0;
            }
            return 1;
        } );
    if (ob->query_container())
    {
        if (ob->query_con_close())
        {
            mud_data += ({({ "close_con", ({}) })});
        }
        else
        {
            mud_data += ({({ "open_con", ({}) })});
        }
    }
    ob_properties = ZENTRALBANK->query_saved_properties_for_conservation(ob);
    return get_one_detail(ob) + ([ 
        ARMA_FACTORY : factory,
        ARMA_FACTORY_ID : identifier,
        ARMA_SHADOW_DATA:sh_data, 
        ARMA_OBJECT_DATA:ob_data,
        ARMA_MUDLIB_DATA:mud_data,
        ARMA_PROPERTIES_DATA:ob_properties,
        ARMA_LOAD_FILE : get_program_name(ob) || object_name(ob),
        ARMA_SPECIAL_ID : ":"+time()+"::",// TODO adequate bank_id/owner
        ARMA_ORIGIN_INFO : m_origin,
        ARMA_FILE_COUNTER : ([]),
        ]);
}
private <mapping*|string> behaelter_einlagern(object b,mapping check_attributes)
{
    if (living(b)) return ({});
    object * ai = all_inventory(b),ob;
    mapping* result = ({}),m;
    foreach (ob : ai)
    {
        m = get_einlagerungsdaten(ob,check_attributes);
        if (stringp(m)) return (string)m;
        result += ({m});
    }
    return result;
}

/*
FUNKTION: object2storagemapping
DEKLARATION: public varargs <mapping|string> object2storagemapping(object ob,mapping check_attributes)
BESCHREIBUNG:
Diese Funktion macht aus einem einlagerbaren Objekt das Mapping, 
der zu speichern ist. Im Fehlerfall wird ein String zurückgegeben.
Infos zu check_attributes siehe check_conservation
VERWEISE: storagemapping2object,check_conservation
GRUPPEN: armageddon
*/
public varargs <mapping|string> object2storagemapping(object ob,mapping check_attributes)
{
    <mapping|string> m = get_einlagerungsdaten(ob,check_attributes);
    if (stringp(m))
    {
        return m;
    }
    m[ARMA_CONTENT_DATA] = behaelter_einlagern(ob,check_attributes);
    if (stringp(m[ARMA_CONTENT_DATA]))
    {
        ob->abort_conservation();
        all_inventory(ob)->abort_conservation();
        return m[ARMA_CONTENT_DATA];
    }
    return m;
}

private <string|object> inhalt_laden(object tp, string ob_file, mixed ob_data, 
        mapping* sh_data, mixed mud_data, string factory,string factory_id, 
        mapping m_origin,mapping ob_properties)
{
    string sh_file,new_file;
    object sh,ob;
    mapping shd;
    mixed dataline;
    if (stringp(factory) && factory != "")
    {
        ob = factory->get_conservation_object(factory_id, ob_data);
        if (ob == 0) 
            raise_error(wrap(sprintf("Factory %s liefert kein Objekt auf %s",
                factory,factory_id||"")));
    }
    else
    {
        new_file = ZENTRALBANK->replace_bank_safe_file(ob_file,ZB_BANK_ACTIVE);
        if (new_file && new_file != "") ob_file = new_file;
        if (strstr(ob_file,"/obj/")==-1)
            ob = touch(ob_file);        // Fehler ueber catch abgefangen
        else
            ob = clone_object(ob_file); // Fehler ueber catch abgefangen
        mapping* tmp_setters = deep_copy(ob->backup_conservation_setters());
        ob->init_conservation_arg(ob_data);
        ob->restore_conservation_setters(tmp_setters);
    }
    if (pointerp(mud_data) && sizeof(mud_data))
    {
        foreach(dataline : mud_data)
        {
            apply(#'call_other,ob,dataline[0],dataline[1]);
        }
    }
    ZENTRALBANK->set_saved_properties_for_conservation(ob,ob_properties);
    foreach (shd : sh_data||({}) )
    {
        if (!sizeof(shd)) continue;
        sh_file = m_indices(shd)[0];
        if (shd[sh_file]==0) continue; // spaet aber besser als nie...
        new_file = ZENTRALBANK->replace_bank_safe_file(sh_file,ZB_BANK_ACTIVE);
        if (!new_file || new_file == "") new_file = sh_file;
        sh = clone_object(new_file);
        sh->init_conservation_shadow_arg(shd[sh_file]);
        if (sh && sh->init_shadow(ob)!=SHADOWING_OK)
        {
            ZB_DEBUGLOG(sprintf("shadows-reject-init=%Q,%Q,%Q",ob,sh,shd),
                DB_DBGLVL_ERROR,RNAME(tp),"schließfächer:auslagern");
            sh->remove();
            ob->remove();
            return "Ein schattenhafter Fehler verhinderte das Auslagern.";
        }
    }
    if (mappingp(m_origin)&&sizeof(m_origin))
    {
        ob->set(P_ORIGIN,m_origin);
    }
    return ob;
}

private string inhalt_auslagern(object tp,object con,mapping* content_data)
{
    mapping vi;
    <string|object> strobj;
    string str;
    int flag_close,ix;
    if (!sizeof(content_data) || !con->query_container())
        return 0; // ok bzw ist kein Container mehr.
    flag_close = con->query_con_close();
    if (flag_close)
        con->open_con();
    for (ix = sizeof(content_data)-1; ix >= 0; ix--)
    {
        vi = content_data[ix];
        strobj = inhalt_laden(tp, vi[ARMA_LOAD_FILE], 
            vi[ARMA_OBJECT_DATA], vi[ARMA_SHADOW_DATA], vi[ARMA_MUDLIB_DATA],
            vi[ARMA_FACTORY],vi[ARMA_FACTORY_ID],
            vi[ARMA_ORIGIN_INFO],vi[ARMA_PROPERTIES_DATA]);
        if (stringp(strobj))
            return strobj;
        if (objectp(strobj))
        {
            switch (strobj->move(con))
            {
                case MOVE_OK:
                    continue; // Alles ok im container.
                case MOVE_NO_ROOM:
                    str = Der(strobj)+" passt nicht in "+den(con)+".";
                    strobj->remove();
                    con->close_con();
                    con->remove();
                    return str;
                case MOVE_NOT_ALLOWED:
                case MOVE_DEST_CLOSED:
                case MOVE_ENV_CLOSED:
                case MOVE_NO_DEST:
                default:
                    str = "Fehler in "+dem(con)+": "+Der(strobj)+".";
                    strobj->remove();
                    con->close_con();
                    con->remove();
                    return str;
            }
        }
    }
    if (flag_close)
        con->close_con();
    return 0;
}

private <string|object> auslagern(object tp, string ob_file, mixed ob_data, 
        mapping* sh_data, mixed mud_data, string factory,string factory_id, 
        mapping m_origin,mapping* content_data,mapping ob_properties)
{
    // int eval = get_eval_cost();
    string str;

    object ob = inhalt_laden(tp,ob_file,ob_data,sh_data,mud_data,factory,
            factory_id,m_origin,ob_properties);
    if (objectp(ob))
    {
        str = inhalt_auslagern(tp,ob,content_data);
        if (stringp(str)) 
        {
            return str;
        }
    }
    else
    {
        return "Kein Objekt!\n";
    }
    return ob;
}

/*
FUNKTION: storagemapping2object
DEKLARATION: public <object|string> storagemapping2object(mapping vi)
BESCHREIBUNG:
Diese Funktion macht aus dem mit object2storagemapping gespeicherten
Mapping wieder ein Objekt und gibt es im Erfolgsfall zurück.
Die Funktion entspricht dem Auslagern in den Schliessfaechern.
Im Fehlerfall wird ein String zurückgegeben.
VERWEISE: object2storagemapping
GRUPPEN: armageddon
*/
public <object|string> storagemapping2object(mapping vi)
{
    <string|object> strobj;
    string err;
    if (err=catch(strobj=auslagern(TP, vi[ARMA_LOAD_FILE], 
            vi[ARMA_OBJECT_DATA], vi[ARMA_SHADOW_DATA], vi[ARMA_MUDLIB_DATA],
            vi[ARMA_FACTORY],vi[ARMA_FACTORY_ID],
            vi[ARMA_ORIGIN_INFO],vi[ARMA_CONTENT_DATA],
            vi[ARMA_PROPERTIES_DATA]); publish))
    {
        ZB_DEBUGLOG(sprintf("öffnen-fail-1=%Q %Q",
            err,vi[ARMA_SPECIAL_ID]),
            DB_DBGLVL_ERROR,RNAME(TP),"serializer:oeffnen_cmd");
        ZENTRALBANK->report_error_bank_safe(
                vi[ARMA_SPECIAL_ID],RNAME(TP),0);
        return "Aufgrund eines Lochs im RaumZeitGefüge konnte "
            +ein(vi)+" nicht ausgelagert werden.";
    }
    if (stringp(strobj))
    {
        ZB_DEBUGLOG(sprintf("öffnen-fail-2=%Q %Q",
            strobj,vi[ARMA_SPECIAL_ID]),
            DB_DBGLVL_WARNING,RNAME(TP),"schließfächer:oeffnen_cmd");
        return strobj;
    }
    return strobj;
}