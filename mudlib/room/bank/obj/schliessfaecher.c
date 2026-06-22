// This file is part of UNItopia mudlib.
// -------------------------------------------------------------------
// File:        /room/bank/obj/schliessfaecher.c
// Description: Schliessfaecherobjekt fuer die Banken.
// Author:      Myonara (24.Apr.2016)

/*
FUNKTION: query_conservation_arg
DEKLARATION: mixed query_conservation_arg()
BESCHREIBUNG:
Folgende Rueckgabewerte unterscheidet das Schliessfach:
- 0: kein Einlagern
- != 0 Diese Daten werden gespeichert und beim Auslagern an init_conservation_arg
  uebergeben.
VERWEISE: query_conservation_arg_sh, init_conservation_arg
GRUPPEN: armageddon
*/

/*
FUNKTION: init_conservation_arg
DEKLARATION: void init_conservation_arg(mixed m)
BESCHREIBUNG:
Nach dem Auslagern == Wiedererschaffen wird diese Funktion mit den Daten
aufgerufen, die mit query_conservation_arg gesetzt wurden.
VERWEISE: query_conservation_arg_sh, query_conservation_arg
GRUPPEN: armageddon
*/

/*
FUNKTION: query_conservation_arg_sh
DEKLARATION: mixed query_conservation_arg_sh(object shadow)
BESCHREIBUNG:
Bei einem Shadow, der die Einlagerung ueberstehen sollm
liefert diese Funktion alle Variablen zurueck,
die fuer den spaeteren Gebrauch gespeichert werden sollen.
Im einfachsten Fall ist das eine 1, in komplizierteren Faellen kann ein
Array oder Mapping mit allen wichtigen Daten zurueckgeliefert werden.
Eine 0 sollte bei einem Autoloadshadow auf keinen Fall geliefert werden.

Diese Funktion sollte nur etwas liefern, wenn das Shadow direkt mit
dem 1. Parameter angesprochen wurde, ansonsten den Aufruf an query_shadow_owner
weiterleiten. Beispiel:
    
    mixed query_conservation_arg_sh(object shadow)
    {
        if(shadow!=this_object())
            return query_shadow_owner()->query_conservation_arg_sh(shadow);
        
        return irgendwelcheinteressantendatendieabgespeichertwerdensollen;
    }

VERWEISE: init_conservation_shadow_arg
GRUPPEN: armageddon
*/
/*
FUNKTION: init_conservation_shadow_arg
DEKLARATION: void init_conservation_shadow_arg(mixed m)
BESCHREIBUNG:
Beim Wiedererschaffen(auslagern) wird der Shadow geladen und vor dem 
Ueberwerfen ueber den Gegenstand mittels init_conservation_shadow_arg initialisiert,
welche als Daten durch query_conservation_arg_sh uebergeben wurde.
VERWEISE: query_conservation_arg_sh, init_conservation_arg
GRUPPEN: armageddon
*/

/*
FUNKTION: precheck_conservation
DEKLARATION: string precheck_conservation(mapping check_attributes)
BESCHREIBUNG:
Gibt ein Objekt oder Shadow einen String zurueck, so wird dieser als 
Begruendung genommen, warum dieser (zur Zeit?) nicht armafest gelagert 
werden kann. 0 sonst. Abhaengig vom environment() koennen Einlagerungen
in Behaeltern gesondert behandelt werden.
check_attributes enthaelt folgende Schlüssel für den precheck_conservation:
- ZB_CHECK_TYPE
    - ZB_CT_UNKNOWN:        ist nur während der Umstellung unbekannt.
    - ZB_CT_TEST_ONLY:      für Seherkristall und ähnliches.
    - ZB_CT_SCHLIESSFACH:   Das Bankschliessfach
    - ZB_CT_SPEZIALTRUHE:   Die Schiffsmoebel-Spezialtruhe
    - ZB_CT_AUKTION:        Eine Auktion (erzeugen/einlagern)
- ZB_CHECK_CON
    Das Objekt/Mapping der Schliessfach/Truhe/Auktionator, was das Einlagern macht.
- ZB_CHECKER_OB
    Das object/mapping, was die aktuelle Prüfung durchführt.
VERWEISE: query_conservation_arg
GRUPPEN: armageddon
*/

/*
FUNKTION: precheck_conservation
DEKLARATION: string precheck_conservation(object ob,mapping check_attributes)
BESCHREIBUNG:
Liefert ein Objekt ob->query(P_CONSERVATION,P_CONSERVATION_FACTORY) 
einen String, so wird touch(factory)->precheck_conservation(ob) aufgerufen.
Gibt diese Funktion einen Text zurueck, so wird dieser als Begruendung
genommen, warum dieses Objekt (zur Zeit?) nicht armafest gelagert werden
kann. Bei Rueckgabe von 0 ist das Einlagern prinzipiell moeglich.
Abhaengig vom environment() koennen Einlagerungen in Behaeltern gesondert
behandelt werden.
check_attributes siehe check_conservation
VERWEISE: get_conservation_data, get_conservation_object,check_conservation
GRUPPEN: armageddon
*/
/*
FUNKTION: get_conservation_data
DEKLARATION: mixed get_conservation_data(object ob)
BESCHREIBUNG:
Liefert ein Objekt ob->query(P_CONSERVATION,P_CONSERVATION_FACTORY) 
einen String, so wird beim Einlagern touch(factory)->get_conservation_data(ob)
aufgerufen. Gibt die Funktion 0 zurueck, so wird nicht eingelagert,
ein Grund sollte vorher mit precheck_conservation geliefert werden.
Jeder andere Wert wird beim Einlagern gespeichert und beim Auslagern
zusammen mit dem Wert der Property 
ob->query(P_CONSERVATION,P_CONSERVATION_IDENTIFIER) an 
get_conservation_object gegeben.
VERWEISE: precheck_conservation, get_conservation_object
GRUPPEN: armageddon
*/
/*
FUNKTION: get_conservation_object
DEKLARATION: varargs object get_conservation_object(string identifier,mixed data)
BESCHREIBUNG:
Zwei Aufrufarten: vom reset eines Raumes oder anderen Objektes wird
zur Erzeugung get_conservation_object(identifier) aufgerufen, dass Objekt
wird erzeugt, initialisiert, die Schluessel P_CONSERVATION_FACTORY
und P_CONSERVATION_IDENTIFIER in der Property P_CONSERVATION 
(aus properties.h) werden gesetzt und das Objekt zur weiteren Verwendung 
zurueckgegeben.
In der anderen Verwendungsart wird beim Auslagern diese Funktion
der identifier zusammen mit den vorher gespeicherten Daten aufgerufen,
um das Objekt wie oben zu initialisieren und in den durch die Zusatzdaten
gespeicherten Status versetzt.
VERWEISE: precheck_conservation, get_conservation_data
GRUPPEN: armageddon
*/

/*
FUNKTION: query_conservation_item_rent
DEKLARATION: float query_conservation_item_rent()
BESCHREIBUNG:
Gibt ein Objekt einen Wert groesser 0.0 (Taler!) zurueck, so wird diese als
aktuelle Gegenstandsmiete angenommen.
GRUPPEN: armageddon
*/

/*
FUNKTION: query_conservation_item_rent
DEKLARATION: float query_conservation_item_rent(object ob)
BESCHREIBUNG:
Gibt diese Funktion fuer eine Factory 
(ob->query(P_CONSERVATION,P_CONSERVATION_FACTORY)) fuer dieses Objekt einen 
Wert groesser 0.0 (Taler!) zurueck, so wird diese als aktuelle 
Gegenstandsmiete angenommen.
GRUPPEN: armageddon
*/

/*
FUNKTION: prepare_conservation
DEKLARATION: void prepare_conservation()
BESCHREIBUNG:
Meldet beim Objekt den Beginn der Pruefung auf Einlagerung, d.h man kann
dort controller abmelden, die eine Pruefung verhindern wuerden.
GRUPPEN: armageddon
*/

/*
FUNKTION: abort_conservation
DEKLARATION: void abort_conservation()
BESCHREIBUNG:
Meldet beim Objekt den Abbruch der Pruefung bzw. der Einlagerung.
Man kann die in prepare_conservation abgemeldeten Controller wieder anmelden.
GRUPPEN: armageddon
*/

/*
FUNKTION: done_conservation
DEKLARATION: void done_conservation()
BESCHREIBUNG:
Meldet beim Objekt den Abschluss der Einlagerung, danach wird das 
Objekt zerstoert!
GRUPPEN: armageddon
*/

/*
FUNKTION: finish_conservation
DEKLARATION: void finish_conservation()
BESCHREIBUNG:
Meldet beim Objekt den Abschluss der Auslagerung. 
Man kann die in prepare_conservation abgemeldeten Controller wieder anmelden.
GRUPPEN: armageddon
*/

inherit "/i/tools/security";
inherit "/i/item";
inherit "/i/move";

#include <database.h>
#include <invis.h>
#include <message.h>
#include <misc.h>
#include <more.h>
#include <move.h>
#include <notify_fail.h>
#include <parse_com.h>
#include <security.h>
#include <shadow.h>

#include <properties.h>
#include <money.h>

#define DEBUGGER "myonara"
#include <debug.h>

private object owner,env,fach;
private string bank_id,valuta,valutas,v_gender;
private closure calc_rent,calc_max_count;

private mapping *vitems_inhalt,*vitems_faecher;
private string *liste_faecher;
private int init_vitems = 0;

private mapping fach_design = ([]); // Design _EINES_ Schliessfaches.
private mapping faecher_design = ([]); // Design des plurals Schliessfaecher.
private closure c_fach_take = 0; // Closure fuer vitem_take eines Faches.
private closure c_item_take = 0; // Closure fuer vitem_take eines Items.

public nomask int is_bank_safes()
{
    return 1;
}

int query_no_move()
{
    return 1;
}

string query_no_move_reason()
{
    return wrap(Der(TO)+plural(" lässt"," lassen",TO)+" sich nicht bewegen.");
}

int forbidden_shadow(object shadow, object victim)
{
    return victim == TO;
}

string get_leer(string gender)
{
    switch (gender[0..0])
    {
        case "m": return "leerer";
        case "w": return "leere";
        default: return "leeres";
    }
}
string get_xten(int ix)
{
#ifdef UNItopia
    return "/p/Misc/apps/zahlen"->zahl2kardinal(ix);
#endif
    return ""+ix+".";
}

int remove()
{
    if (!TO) return 1;
    destruct(TO);
    return 1;
}

void notify_remove(object ob)
{
    if (ob == fach)
    {
        init_vitems = 0; // Anzeige aktualisieren!
    }
}

string eval_diff(string was,int start_eval)
{
    int diff_eval = start_eval - get_eval_cost();
    switch (diff_eval)
    {
        case 0..10000:      return was+"["+diff_eval+"] wenig";
        case 10001..30000:  return was+"["+diff_eval+"] normal";
        case 30001..90000:  return was+"["+diff_eval+"] viel";
        case 90001..200000: return was+"["+diff_eval+"] sehr viel";
        case 200001..__MAX_EVAL_COST__:
        default:            return was+"["+diff_eval+"] kritisch";
    }
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
    ZENTRALBANK->set_saved_properties_for_conservation(ob,ob_properties);
    if (pointerp(mud_data) && sizeof(mud_data))
    {
        foreach(dataline : mud_data)
        {
            apply(#'call_other,ob,dataline[0],dataline[1]);
        }
    }
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
    int eval = get_eval_cost();
    string str;

    object ob = inhalt_laden(tp,ob_file,ob_data,sh_data,mud_data,factory,
            factory_id,m_origin,ob_properties);
    if (objectp(ob))
    {
        inhalt_auslagern(tp,ob,content_data);
    }
    else
    {
        raise_error("Kein Objekt!\n");
    }
    fach = clone_object(ZB_SCHLIESSFACH);
    fach->init_fach(bank_id,tp,calc_rent,valuta,valutas,v_gender);
    fach->add_controller("notify_remove",TO);
    fach->set_schliessfach_design(fach_design);
    fach->open_con(); // sicher ist sicher.
    switch (ob->move(fach))
    {
        case MOVE_OK:
            break; // Alles ok im fach.
        case MOVE_NO_ROOM:
            str = Der(ob)+" passt nicht in "+den(fach)+".";
            ob->remove();
            fach->remove();
            return str;
        case MOVE_NOT_ALLOWED:
        case MOVE_DEST_CLOSED:
        case MOVE_ENV_CLOSED:
        case MOVE_NO_DEST:
        default:
            str = "Fehler in "+dem(fach)+": "+Der(ob);
            ob->remove();
            fach->remove();
            return str;
    }
    switch (fach->move(tp))
    {
        case MOVE_OK:
            ob->finish_conservation();
            all_inventory(ob)->finish_conservation();
            send_message_to(owner,MT_DEBUG,MA_UNKNOWN,wrap(
                eval_diff("Auslagern-Evals:",eval)));
            return fach; // Alles ok im fach.
        case MOVE_NO_ROOM:
            str = "Du hast nicht genug Platz(?) für "+den(fach)+".";
            ob->remove();
            fach->close_con();
            fach->remove();
            return str;
        case MOVE_NOT_ALLOWED:
        case MOVE_DEST_CLOSED:
        case MOVE_ENV_CLOSED:
        case MOVE_NO_DEST:
        default:
            str = "Fehler bei "+dem(fach)+": "+Der(ob);
            ob->remove();
            fach->close_con();
            fach->remove();
            return str;
    }
}

// dynamische vitems
string vitem_take(mapping vi)
{
    
    if (HAS_ID(vi,fach_design["name"]||"schließfach"))
    {
        return wrap(c_fach_take?closure_to_string(c_fach_take):
            "Willst Du das Schließfach aus der Wand reißen? "
            "Oeffne doch das Schließfach Deiner Wahl.");
    }
    return wrap(c_item_take?closure_to_string(c_item_take):
        "Oeffne das Schließfach, um an den Gegenstand zu kommen.");
}

private void initialize_vitems()
{
    if (!bank_id || !playerp(owner))
    {
        vitems_inhalt = vitems_faecher = ({});
        liste_faecher = ({});
        return;
    }
    vitems_inhalt = ZENTRALBANK->retrieve_bank_safe(bank_id,RNAME(owner))
                    ||({});
    liste_faecher = map(vitems_inhalt, function string (mapping m)
        {
            return m["short"];
        });
    vitems_faecher = map(vitems_inhalt, function mapping (mapping m)
        {
            return ([
       "environment" : TO,
       "origin"      : TO,
        "debug_info" : (m["debug_info"]||([])),
          "name"     : fach_design["name"]||"schließfach",
          "cap_name" : fach_design["cap_name"]?
                       fach_design["cap_name"]:
                       capitalize(fach_design["name"]||"schließfach"),
          "gender"   : fach_design["gender"]||"saechlich",
          "id"       : fach_design["id"]||({"schließfach","fach"}),
          "short"    : "Ein Schließfach mit: "+m["short"],
          "long"     : m["long"],
          "seher_info": m["seher_info"],
          "look_msg" : "",
          "read_msg" : "",
          "take_msg" : "",
          "feel_msg" : "",
          "smell_msg": "",
          "hear_msg" : "",
          "take": #'vitem_take,
          ARMA_SPECIAL_ID: m[ARMA_SPECIAL_ID],
          ]);
        });
}

private int check_entry(mapping entry, string ids, string *adj)
{
    if (mappingp(entry) && HAS_ID(entry,ids)) {
        if (adj) {
            foreach(string str : adj) {
                if (member(entry["adjektiv"]||({}),str) == -1) {
                    return 0;
                }
            }
        }
        return 1;
    }
    return 0;
}

varargs mapping query_v_item(mixed *pfad, int flag)
{
    mapping ret, what, *visa;
    string id, *adj;
    int nummer;

    if (objectp(fach))
    {
        visa = ({});
        init_vitems = 0;
    }
    else if (!init_vitems)
    {
        initialize_vitems();
        visa=(vitems_faecher||({}))+(vitems_inhalt||({}));
        init_vitems = 1;
    }
    else
    {
        visa=(vitems_faecher||({}))+(vitems_inhalt||({}));
    }
    if (init_vitems && !sizeof(visa))
    {
        visa += ({ ([ 
            "name" : fach_design["name"]||"schließfach",
            "id": fach_design["id"]||({ "schließfach","fach"}),
            "gender": fach_design["gender"]||"saechlich",
            "adjektiv":({"leer"}),
            "long":fach_design["long_leer"]||
                    Ein(fach_design,"leer")+", du kannst "
                    +ihn(fach_design)+" öffnen.",
              "look_msg" : "",
              "read_msg" : "",
              "take_msg" : "",
              "feel_msg" : "",
              "smell_msg": "",
              "hear_msg" : "",
        ]) });
    }
    if(!(ret = ::query_v_item(pfad,flag)) &&
       sizeof(pfad)==1)
    {
        if(stringp(pfad[0]))
        {
            id = lower_case(pfad[0]);
        }
        else if(mappingp(pfad[0]))
        {
            id = lower_case(pfad[0]["name"]);
            adj = pfad[0]["adjektiv"];
            nummer=pfad[0]["nummer"];
        }
        visa=filter(visa,#'check_entry,id,adj);
        if(sizeof(visa))
        {
            if(nummer > 1)
            {
                if(sizeof(visa)>=nummer)
                    what = visa[nummer-1];
                else
                    return 0;
            } else {
                what = visa[0];
            }
            ret = what + ([
              "environment"   : TO,
              "v_item_master" : TO,
              "take" : #'vitem_take,
                              ]);
        }
    }
    return ret;
}

mixed *query_all_v_items()
{
    mapping *visa;
    if (objectp(fach))
    {
        visa = ({});
        init_vitems = 0;
    }
    else if (!init_vitems)
    {
        initialize_vitems();
        visa=(vitems_faecher||({}))+(vitems_inhalt||({}));
        init_vitems = 1;
    }
    else
    {
        visa=(vitems_faecher||({}))+(vitems_inhalt||({}));
    }
    if (init_vitems && !sizeof(visa))
    {
        visa += ({ ([ 
            "name" : fach_design["name"]||"schließfach",
            "id": fach_design["id"]||({ "schließfach","fach"}),
            "gender": fach_design["gender"]||"saechlich",
            "adjektiv":({"leer"}),
            "long":fach_design["long_leer"]||
                    Ein(fach_design,"leer")+", du kannst "
                    +ihn(fach_design)+" öffnen.",
              "look_msg" : "",
              "read_msg" : "",
              "take_msg" : "",
              "feel_msg" : "",
              "smell_msg": "",
              "hear_msg" : "",
        ]) });
    }
    return (::query_all_v_items()||({}))
            +map(visa,
                 function(mapping entry)
                 {
                     return entry + ([
                 "environment"   : TO,
                 "v_item_master" : TO,
                 "take" : #'vitem_take,
                                 ]);
                     });
}

int my_notify_move(string cntr, mapping mv_infos)
{
    if (owner == mv_infos[MOVE_OBJECT] && env != mv_infos[MOVE_NEW_ROOM])
    {
        remove();
    }
}

private string get_value_in_current_valuta(int amount)
{
    return ZENTRALBANK->get_value_in_current_valuta(to_float(amount),
        valuta,valutas,v_gender);
}

int bezahlen_cmd(string str)
{
    string *split = explode(lower_case(space(str))," ");
    int result,ix;
    if (!check_security() || TP!=ENV_TO)
    {
        FAILWP("Kein Zugriff auf die Bezahlung der Miete.",FAIL_INTERNAL);
    }
    if (sizeof(split)<3 || strstr(split[0],"miet")==-1 || convert_umlaute(split[1])!="fuer")
    {
        FAILWP("bezahle Miete für (x."+fach_design["cap_name"]
                +"|diese Bank|alle Banken)?",
            FAIL_NOT_OBJ);
    }
    query_all_v_items(); // initialisieren...
    if (sizeof(split)>3 && split[3]=="banken" && split[2] == "alle")
    {
        result = ZENTRALBANK->update_rent_for_safes(TP);
    }
    else if (sizeof(split)>3 && split[3]=="bank" && split[2] == "diese")
    {
        result = ZENTRALBANK->update_rent_for_safes(TP,bank_id);
    }
    else if ((sscanf(split[2],"%d.schließfach",ix)==1 )
        || (sscanf(split[2],"%d."+fach_design["name"],ix)==1 )
        || (sscanf(split[2],"%d.fach",ix)==1 )
        || ((sizeof(split)>3) 
            && sscanf(split[2]+split[3],"%d.schließfach",ix)==1 )
        || ((sizeof(split)>3) 
            && sscanf(split[2]+split[3],"%d."+fach_design["name"],ix)==1 )
        || ((sizeof(split)>3) 
            && sscanf(split[2]+split[3],"%d.fach",ix)==1 ))
    {
        if (ix < 1 || ix > sizeof(vitems_inhalt))
        {
            FAILWP("Ungültig.",FAIL_INTERNAL);
        }
        result = ZENTRALBANK->update_rent_for_safes(TP,bank_id,
            vitems_inhalt[ix-1][ARMA_SPECIAL_ID]);
    }
    else
    {
        FAILWP("bezahle Miete für (x."+fach_design["cap_name"]
                +"|diese Bank|alle Banken)?",
            FAIL_NOT_OBJ);
    }
    if (result > 0) 
    {
        send_message_to(TP,MT_NOTIFY,MA_UNKNOWN,wrap(
            "Es wurde insgesamt "
            +get_value_in_current_valuta(result)+" verrechnet."));
        return 1;
    }
    else if (result < 0)
    {
        send_message_to(TP,MT_NOTIFY,MA_UNKNOWN,wrap(
            "Dein Konto hatte für "
            +get_value_in_current_valuta(-result)+" nicht genug."));
        return 1;
    }
    else
    {
        FAILWP("Interner Fehler beim Verrechnen der Miete.",FAIL_INTERNAL);
    }
}

int oeffnen_cmd(string str)
{
    string match,err,*split;
    <string|object> strobj;
    int ix,deltakonto,max_faecher;
    if (!check_security() || TP!=ENV_TO)
    {
        FAILWP("Du darfst "+der(faecher_design)+" nicht öffnen.",FAIL_WRONG_ARG);
    }
    if (objectp(fach))
    {
        init_vitems = 0;
        FAILWP("Zur selben Zeit kann immer nur "+ein(fach_design)
            +" geöffnet sein.",FAIL_NOT_OBJ);
    }
    else if (!init_vitems)
    {
        initialize_vitems();
        init_vitems = 1;
    }
    if (this_player()->free_hand() < 0)
        return notify_fail(wrap("Du hast keine Hand frei, um "
            +ein(fach_design)+" zu öffnen."), FAIL_INTERNAL);
    str = lower_case(space(str));
    split = explode(str," ");
    if (strstr(str,"fach")==-1 && strstr(str,fach_design["name"])==-1
        && sizeof(split & fach_design["id"])<=0)
    {
        FAILWP("öffne x. "+fach_design["name"]+"|oeffne "
            +ein(fach_design,({"leer"}))+"?",
            FAIL_NOT_OBJ);
    }
    if (strstr(str,"leer")!=-1 || strstr(str,"neu")!=-1 || 
        sizeof(vitems_inhalt)==0)
    {
        max_faecher = apply(calc_max_count,this_player());
        if (sizeof(vitems_inhalt) >= max_faecher)
        {
            FAILWP("Mehr als "+max_faecher+" "+faecher_design["cap_name"]
                +" darfst du "
                "zur Zeit in dieser Bank nicht haben.",FAIL_INTERNAL);
        }
        deltakonto = 0; // ZENTRALBANK->update_rent_for_safes(TP,bank_id);
        if (deltakonto > 0)
        {
            send_message_to(TP, MT_NOTIFY, MA_UNKNOWN, wrap(
                "Es wurden ca. "
                +get_value_in_current_valuta(deltakonto)+" für die Miete "
                "aller "+faecher_design["cap_name"]
                +" dieser Bank bezahlt."));
        }
        err = ZENTRALBANK->check_bank_safe_locks(bank_id,TP);
        if (stringp(err))
        {
            FAILWP("Du erhältst kein weiteres leeres "+fach_design["cap_name"]
                +". Grund:\n"+err,
                FAIL_INTERNAL);
        }
        fach = clone_object(ZB_SCHLIESSFACH);
        fach->init_fach(bank_id,TP,calc_rent,valuta,valutas,v_gender);
        fach->set_schliessfach_design(fach_design);
        fach->add_controller("notify_remove",TO);
        switch (fach->move(TP))
        {
            case MOVE_OK:
                send_message_to(TP, MT_NOTIFY,MA_UNKNOWN,wrap(
                    "Du öffnest "+ein(fach,"leer")+"." ));
                return 1; 
            case MOVE_NO_ROOM:
                str = "Du hast nicht genug Platz(?) für "+den(fach)+".";
                fach->remove();
                FAILWP(str,FAIL_INTERNAL);
            case MOVE_NOT_ALLOWED:
            case MOVE_DEST_CLOSED:
            case MOVE_ENV_CLOSED:
            case MOVE_NO_DEST:
            default:
                str = "Fehler bei "+dem(fach)+".";
                fach->remove();
                FAILWP(str,FAIL_INTERNAL);
        }
    }
    match = regmatch(str,"[0-9]+");
    if (str2int(match||"1",&ix) || ix <= 0)
    {
        FAILWP("öffne x. "+fach_design["name"]+"schliessfach|oeffne "
            +ein(fach_design,({"leer"}))+"?",
            FAIL_NOT_OBJ);
    }
    if (ix > sizeof(vitems_inhalt))
    {
        FAILWP("So viele "+faecher_design["cap_name"]+" besitzt Du nicht.", 
            FAIL_INTERNAL);
    }
    mapping vi = vitems_inhalt[ix-1];
    deltakonto = ZENTRALBANK->update_rent_for_safes(
        TP,bank_id,vi[ARMA_SPECIAL_ID]);
    if (deltakonto > 0)
    {
        send_message_to(TP, MT_NOTIFY, MA_UNKNOWN, wrap(
            "Es wurden ca. "
                +get_value_in_current_valuta(deltakonto)+" für die Miete "
            +des(fach_design,get_xten(ix))+" dieser Bank bezahlt."));
    }
    if (ZENTRALBANK->check_bank_safe_lock(vi[ARMA_SPECIAL_ID]))
    {
        FAILWP(Der(fach_design)+" ist gesperrt, nutze 'bezahle miete' "
            "zum entsperen.", FAIL_INTERNAL);
    }
    if (err=catch(strobj=auslagern(TP, vi[ARMA_LOAD_FILE], 
            vi[ARMA_OBJECT_DATA], vi[ARMA_SHADOW_DATA], vi[ARMA_MUDLIB_DATA],
            vi[ARMA_FACTORY],vi[ARMA_FACTORY_ID],
            vi[ARMA_ORIGIN_INFO],vi[ARMA_CONTENT_DATA],
            vi[ARMA_PROPERTIES_DATA]); publish))
    {
        ZB_DEBUGLOG(sprintf("öffnen-fail-1=%Q %Q",
            err,vi[ARMA_SPECIAL_ID]),
            DB_DBGLVL_ERROR,RNAME(TP),"schließfächer:oeffnen_cmd");
        ZENTRALBANK->report_error_bank_safe(
                vi[ARMA_SPECIAL_ID],RNAME(TP),0);
        FAILWP("Aufgrund eines Lochs im RaumZeitGefüge konnte "
            +ein(vi)+" nicht ausgelagert werden.",FAIL_INTERNAL);
    }
    if (stringp(strobj))
    {
        ZB_DEBUGLOG(sprintf("öffnen-fail-2=%Q %Q",
            strobj,vi[ARMA_SPECIAL_ID]),
            DB_DBGLVL_WARNING,RNAME(TP),"schließfächer:oeffnen_cmd");
        FAILWP(strobj+" "+Der(vi)+" verbleibt in "+dem(fach_design)+".",
            FAIL_INTERNAL);
    }
    ZENTRALBANK->leave_bank_safes(vi[ARMA_SPECIAL_ID],RNAME(owner),
        vi[ARMA_FILE_COUNTER]); // TODO auf 1 pruefen!!!
    //ZB_DEBUGLOG(sprintf("oeffnen-success=%Q %Q",
    //        strobj,vi[ARMA_SPECIAL_ID]),
    //        DB_DBGLVL_INFO,RNAME(owner),"schliessfaecher:oeffnen_cmd");
    send_message_to(TP,  MT_NOTIFY,MA_UNKNOWN,wrap(
                    "Du öffnest "+dein(strobj,"")+"."));
    return 1;
}

// Aufruf vom Inherit schliessfachaddon.c
void init_faecher(string bid,object own,string val,string vals,string vgen,
    closure crent,closure cmaxcount)
{
    if (own && !owner) owner = own;
    bank_id = bid;
    valuta = val;
    valutas = vals;
    v_gender = vgen;
    calc_max_count = cmaxcount;
    calc_rent = crent;
    ZENTRALBANK->update_bank_information(bid,val,vals,vgen);
}
    
void init()
{
    "*"::init();
    if (TP && TP == ENV_TO && ENV(TP))
    {
        if (owner)
        {
            if (owner != TP)
            {
                call_out("remove",0);
                return;
            }
        }
        else
        {
            owner = TP;
        }
        env = ENV(TP);
        TP->add_controller("notify_moved",#'my_notify_move);
        add_action("oeffnen_cmd","öffne");
        add_action("oeffnen_cmd","eröffne");
        add_action("bezahlen_cmd","bezahle",-6);
        add_action("bezahlen_cmd","zahle",-4);
        return;
    }
}

string query_long(object viewer)
{
    string str,text = faecher_design["prefix_long"];
    int max_count = apply(calc_max_count,owner);
    if (objectp(fach))
    {
        init_vitems = 0;
        if (viewer != owner)
            return wrap(text+"Es ist "+ein(fach_design)+" für "+den(owner)+
                " gerade offen, "+der(faecher_design,"ander")+" sind "
                "nicht zu sehen.");
        return wrap(text+Ein(fach_design)+" ist offen, man kann "+
            er(fach_design)+" betrachten. "
            "Solange "+er(fach_design)+" offen ist, kann man "
            +der(faecher_design,"ander")+
            " nicht sehen. Daneben hängt eine Tafel mit einer Anleitung.");
    }
    else if (!init_vitems)
    {
        initialize_vitems();
        init_vitems = 1;
    }
    if (!sizeof(liste_faecher))
    {
        if (viewer != owner)
            return wrap(text+"Es sind keine "+faecher_design["cap_name"]
                +" für "+den(owner)+" eröffnet.");
        return wrap(text+"Es sind keine der "+max_count+" "
            +faecher_design["cap_name"]+" für dich eröffnet. "
            "Mit 'öffne "+get_leer(fach_design["gender"])
            +" "+fach_design["name"]
            +"' kannst du das Einlagern "
            "beginnen. Daneben hängt eine Tafel mit einer Anleitung.");
    }
    if (viewer != owner)
        text += Des(owner)+" "+faecher_design["cap_name"]
             +" mit Inhalt sind zu sehen:\n";
    else
        text += "Die Tafel mit einer Anleitung hängt neben "
            +dem(faecher_design)+". Folgende deiner bis zu "+max_count
            +" "+faecher_design["cap_name"]
            +" mit armageddon-sicherem Inhalt sind zu sehen:\n";
    int i = 0;
    foreach (str : liste_faecher)
    {
        text += wrap_say(sprintf("%2d: ",++i),str);
    }
    return text;
}

string query_anleitung_read(string parse_rest, string str, 
        mapping was, object leser)
{
    leser->more(ZB_ANLEITUNG_SF,0,0,M_AUTO_END);
    return "";
}

string query_mietuebersicht_read(string parse_rest, string str, 
        mapping was, object leser)
{
    return ZENTRALBANK->get_konten_uebersicht(RNAME(owner),leser);
}

string take_mietuebersicht(mapping was)
{
    object ob;
    if(TP && (TP==PO))
    {
        ob = touch(PATH_ROOM_BANK+"mietuebersicht_"+TP_RN);
        ob->move(TP, ([MOVE_FLAGS:MOVE_ERR_REMOVE]));
        if (!objectp(ob)) {
            return wrap("Du nimmst "+einen(was)+" nicht. Zuviel dabei?");
        }
        return wrap("Du nimmst "+einen(ob)+".");
    }
    else
    {
        return wrap("Hier scheint gerade nur eine Abfrage eines Tools "
                    "stattzufinden, also passiert nix.");
    }
}

string take_armatester(mapping was)
{
    object ob;
    if (!ZENTRALBANK->query_armatester_flag())
        return wrap("Der Armatester darf momentan nicht genommen werden.");
    if(TP && (TP==PO))
    {
        ob = clone_object(ZB_ARMATESTER);
        ob->move(TP, ([MOVE_FLAGS:MOVE_ERR_REMOVE]));
        if (!objectp(ob)) {
            return wrap("Du nimmst "+einen(was)+" nicht. Zuviel dabei?");
        }
        return wrap("Du nimmst "+einen(ob)+".");
    }
    else
    {
        return wrap("Hier scheint gerade nur eine Abfrage eines Tools "
                    "stattzufinden, also passiert nix.");
    }
}

public void set_designs(mapping plural_design,mapping single_design)
{
    if (!plural_design)
    {
        plural_design = ([
            "name" : "schließfächer",
            "cap_name": "Schließfächer",
            "gender" : "saechlich",
            "plural" : 1,
            "id" : ({"schließfächer","fächer"})
        ]);
    }
    if (!single_design)
    {
        single_design = ([
            "name" : "schließfach",
            "cap_name" : "Schließfach",
            "gender" : "saechlich",
            "id" : ({"schließfach","fach"}),
        ]);
    }
    if (!member(plural_design,"name") || !member(plural_design,"gender")
        ||!member(plural_design,"id") || plural_design["plural"]!=1)
    {
        raise_error("Fehler im plural_design\n");
    }
    if (!member(single_design,"name") || !member(single_design,"gender")
        ||!member(single_design,"id"))
    {
        raise_error("Fehler im single_design\n");
    }
    if (!member(plural_design,"cap_name")
        ||lower_case(plural_design["cap_name"])!=plural_design["name"])
    {
        plural_design["cap_name"] = capitalize(plural_design["name"]);
    }
    if (!member(single_design,"cap_name")
        ||lower_case(single_design["cap_name"])!=single_design["name"])
    {
        single_design["cap_name"] = capitalize(single_design["name"]);
    }
    if (!stringp(plural_design["prefix_long"]))
    {
        plural_design["prefix_long"] = "";
    }
    if (plural_design["c_fach_take"]!=0)
    {
        c_fach_take = mixed_to_closure(plural_design["c_fach_take"]);
    }
    else
    {
        c_fach_take = 0;
    }
    if (plural_design["c_item_take"]!=0)
    {
        c_item_take = mixed_to_closure(plural_design["c_item_take"]);
    }
    else
    {
        c_item_take = 0;
    }
    
    if (stringp(plural_design["id"])) 
        plural_design["id"] = ({ plural_design["id"] });
    if (pointerp(plural_design["id"]))
        set_id( plural_design["id"]-({ZB_SCHLIESSFAECHER_ID})
                                   +({ZB_SCHLIESSFAECHER_ID}) );
    else
        set_id( ({"schließfach","fach",ZB_SCHLIESSFACH_ID}) );
    if (member(plural_design,"adjektiv"))
    {
        set_adjektiv(plural_design["adjektiv"]);
    }
    fach_design = single_design;
    faecher_design = plural_design;
}

void create() 
{
    "*"::create();
    set_id( ({"schließfächer","fächer",ZB_SCHLIESSFAECHER_ID}) );
    set_name("schließfächer");
    set_gender("saechlich");
    set_plural(1);
    set_long("Eine Reihe von Schließfächern, daneben hängt eine Tafel "
        "mit einer Anleitung.");
    set_designs(0,0);
    set_invis(V_NOLIST);
    set_weight(0);
    set(P_LOOK_MSG,"");
    set(P_READ_MSG,"");
    set(P_SMELL_MSG,"");
    set(P_HEAR_MSG,"");
    set(P_FEEL_MSG,"");
    set(P_TAKE_MSG,"");
    
    add_v_item( ([
        "name" : "anleitung",
        "id" : ({ "anleitung","tafel" }),
        "gender": "weiblich",
        "long" : "Die Tafel mit der Anleitung kann man lesen.",
        "read" : #'query_anleitung_read,
          "look_msg" : "",
          "read_msg" : "",
          "take_msg" : "",
          "feel_msg" : "",
          "smell_msg": "",
          "hear_msg" : "",
    ]) );
    add_v_item( ([
        "name" : "mietübersicht",
        "id" : ({ "mietübersicht","übersicht","pergament","liste" }),
        "gender": "weiblich",
        "long" : "Die Mietübersicht kann man lesen oder mitnehmen.",
        "read" : #'query_mietuebersicht_read,
        "take" : #'take_mietuebersicht,
          "look_msg" : "",
          "read_msg" : "",
          "take_msg" : "",
          "feel_msg" : "",
          "smell_msg": "",
          "hear_msg" : "",
        "invis": (: present(ZB_MIETUEBERSICHT_ID,TP) ? V_INVIS : V_VIS :),
    ]) );
    add_v_item( ([
        "name" : "armatester",
        "id" : ({ "armatester","testwerkzeug" }),
        "gender": "maennlich",
        "long" : "Ein Armatester zum mitnehmen, falls freigegeben.",
        "take" : #'take_armatester,
          "look_msg" : "",
          "read_msg" : "",
          "take_msg" : "",
          "feel_msg" : "",
          "smell_msg": "",
          "hear_msg" : "",
        "invis": (: present(ZB_ARMATESTER_ID,TP) ? V_INVIS : V_VIS :),
    ]) );
    add_controller("forbidden_shadow",TO);
    init_security_for_actions();
}

void abort_renewal()
{
}

void finish_renewal(object neu)
{
    neu->init_faecher(bank_id,owner,valuta,valutas,v_gender,
        calc_rent,calc_max_count);
}

void prepare_renewal()
{
}
