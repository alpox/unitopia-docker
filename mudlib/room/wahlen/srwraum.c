// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /room/wahlen/srwraum.c
// Description: Wahlraum fuer die Wahl des Spielerrats
// Author:      Myonara, 09.05.2026


// TODO save/Restore/zern
// TODO include datei srwahl.h

inherit "/i/room";
private inherit "/i/tools/security";

#include "/sys/editor.h"
#include "/sys/invis.h"
#include "/sys/message.h"
#include "/sys/more.h"
#include "/sys/move.h"
#include "/sys/notify_fail.h"
#include "/sys/room_types.h"
#include "/sys/misc.h"

#include "/sys/srwahl.h"

#include "?/z/Gilden/Hexenvolk/sys/hv_export.h"

#ifdef HV_INTERFACE_INH
#define my_animalp(ob) (HV_INTERFACE_INH.animalp(ob))
#else
#define my_animalp(ob) (0==1)
#endif

#define PLIR_OLD_ROOM   "player:old_room"
#define PLIR_DOMAIN     "player:domain"

private mapping plir = ([]);
#ifdef Orbit
private nosave int manuelle_wahl = 0;
#endif

public int query_known_plir(object|string rn)
{
    if (objectp(rn)) {
        if (playerp(rn)){
            rn = rn.query_real_name();
        } else {
            return 0;
        }
    }
    return member(plir,rn) ? 1 : 0;
}

private int _plir_set_old_room(object pl,object woher)
{
    if (!playerp(pl)||!objectp(woher))
        return 0;
    if (!woher->query_room())
        return 0;
    string rn = pl.query_real_name();
    string wo = object_name(woher);
    string dom = woher.query_room_domain();
    if (member(plir,rn)){
        plir[rn][PLIR_OLD_ROOM] = wo;
        plir[rn][PLIR_DOMAIN] = dom;
    } else {
        if (wo=="/room/void") 
            return 1;
        plir[rn] = ([
            PLIR_OLD_ROOM:wo,
            PLIR_DOMAIN:dom,
        ]);
    }
    return 1;
}

private string _plir_get_old_room(object pl)
{
    if (!playerp(pl))
        return 0;
    string rn = pl.query_real_name();
    if (member(plir,rn)){
        return plir[rn][PLIR_OLD_ROOM];
    } else {
        return 0;
    }

}

int get_wahl_flag(int wahlselektion)
{
#ifdef Orbit
    return (manuelle_wahl&wahlselektion);
#else
    int* now;
   
    now = timearray(time()); 
    int vorwahl = now[TM_MON]==11 && now[TM_MDAY]<15;
    int wahl = now[TM_MON]==12 || (now[TM_MON]==11 && now[TM_MDAY]>=15);
    int nachwahl = now[TM_MON]==1;
    
    return (vorwahl && (wahlselektion&SRWAHL_VORWAHL)) 
        || (wahl && (wahlselektion&SRWAHL_WAHL))
        || (nachwahl && (wahlselektion&SRWAHL_NACHWAHL));
#endif
}

/*
FUNKTION: install_srw_plakat
DEKLARATION: void install_srw_plakat(object room)
BESCHREIBUNG:
Installiert das Plakat zur Spielerratswahl im Raum.
Ausserhalb der Wahl ist es INVIS 15, also nicht auffindbar.
VERWEISE: SR_WAHLRAUM, SR_WAHLPLAKAT_HIER
GRUPPEN: move, grundlegendes
*/
void install_srw_plakat(object room)
{
    room ||= PO;
    if (!room->query_room()) return;
    object plakat = present(ID_SR_WAHLPLAKAT,room);
    if (plakat) return;
    plakat = clone_object(SR_WAHLPLAKAT);
    plakat.move(room,([MOVE_FLAGS:MOVE_ERR_REMOVE]));
}

void get_urnen()
{
    object urne,liste;
    int here;

    if (get_wahl_flag(SRWAHL_WAHL|SRWAHL_NACHWAHL)!=0)
    {
        urne = find_object(SRWAHL_URNE);
        here = objectp(urne) && present(urne,TO) && 1;
        if (urne && ENV(urne) && !here) {
            urne->remove();
        }
        if (!urne)
        {
            urne = touch(SRWAHL_URNE);
        }
        urne->move(this_object());
    }
     
    if (get_wahl_flag(SRWAHL_VORWAHL)!=0) {
        liste = find_object(SR_KANDIDATENLISTE);
        here = objectp(liste) && present(liste,TO) && 1;
        if (liste && ENV(liste) && !here) {
            liste->remove();
        }
        if (!liste) {
            liste = touch(SR_KANDIDATENLISTE);
        }
        liste->move(this_object());
    } else {
        liste = find_object(SR_KANDIDATENLISTE);
        if(ENV(liste)) { // doppelfunktion master und objekt
            liste->remove();
        }
    }
}

string query_all_wahlplakate_long(mapping v_item, object viewer)
{
    string* names=SR_KANDIDATENLISTE->query_wahlplakat_namen();
    string ktxt = "";
    if (SR_KANDIDATENLISTE->ist_kandidat(viewer->query_real_name()))
        ktxt = " Als Kandidat kannst du Dein Wahlplakat verfassen.";
    switch (sizeof(names))
    {
        case 0:
            return wrap("Kein Kandidat hat ein Wahlplakat zur Verfügung "
                "gestellt."+ktxt);
        case 1:
            return wrap("Es gibt nur ein Wahlplakat von "+names[0]
                +". Man kann es mit 'lese plakat' lesen."+ktxt);
        default:
            return wrap_say("Es gibt "+sizeof(names)+" Wahlplakate "
                "('lese plakate'):", implode(names,", ")+"."+ktxt);
    }    
}

string read_one_wahlplakat(string parse_rest, 
            string str,mapping vitem, object leser)
{
    string * lines;
    if (member(vitem,"WAHLPLAKAT_NAME"))
    {
        lines = SR_KANDIDATENLISTE->query_one_plakat(vitem["WAHLPLAKAT_NAME"]);
        if (sizeof(lines))
        {
            leser->more(lines,"--Mehr--",0,M_AUTO_END);
            return "";
        }
    }
    return "Kein Wahlplakat gefunden.";
}

string read_all_wahlplakate(string parse_rest, 
            string str,mapping vitem, object leser)
{
    string* names=SR_KANDIDATENLISTE->query_wahlplakat_namen();
    string* lines = ({}),name;
    foreach (name:names)
    {
        lines += SR_KANDIDATENLISTE->query_one_plakat(name);
    }
    if (!sizeof(lines))
        return "Keine Wahlplakate gefunden!\n";
    leser->more(lines,"--Mehr--",0,M_AUTO_END);
    return "";
}

private mapping get_one_plakat(string name)
{
    return ([
        "name" : "wahlplakat",
        "WAHLPLAKAT_NAME":name,
        "id" : ({ "wahlplakat","plakat"}),
        "gender": "saechlich",
        "long": "Ein Wahlplakat von "+name+". Man kann es lesen.",
        "read": #'read_one_wahlplakat,//'
    ]);
}

private mapping *get_wahlplakate_v_items()
{
    string* names=SR_KANDIDATENLISTE->query_wahlplakat_namen();
    string name;
    mapping m,*result = ({});
    if (!sizeof(names))
    {
        return ({});
    }
    foreach (name : names)
    {
        m = get_one_plakat(name);
        if (mappingp(m))
            result += ({ m });
    }
    return result;
}

private int my_check_entry(mapping entry, string ids, string *adj)
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

// Die testende Drachenpyrom ANIN sagt zu dir: varargs mapping
//         query_v_item(mixed *pfad, int flag) wird nicht mehr dynamisch
//         gemacht, nur noch mixed *query_all_v_items() siehe
//         /p/Doc/Lehre/Demo/room/vitem_dynamisch.c

varargs mapping query_v_item(mixed *pfad, int flag)
{
    mapping ret, what, *visa;
    string id, *adj;
    int nummer;

    visa = get_wahlplakate_v_items();
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
        visa=filter(visa,#'my_check_entry,id,adj);//'
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
              "environment"   : this_object(),
              "v_item_master" : this_object(),
                              ]);
        }
    }
    return ret;
}

mixed *query_all_v_items()
{
    mapping *visa;
    visa = get_wahlplakate_v_items();
    return (::query_all_v_items()||({}))
            +map(visa,
                 function(mapping entry)
                 {
                     return entry + ([
                 "environment"   : this_object(),
                 "v_item_master" : this_object(),
                                 ]);
                     });
}

int invis_wahlplakate()
{
    return SR_KANDIDATENLISTE->get_phase() ? V_VIS : V_INVIS;
}

private void ed_end(string *lines)
{
    string cname = this_player() ? this_player()->query_real_cap_name() : 0;
    if (!cname) return;
    string result = SR_KANDIDATENLISTE->update_my_plakat(cname,lines);
    this_player()->send_message_to(this_player(),MT_NOTIFY,MA_UNKNOWN,wrap(result));
}

int cmd_verfasse(string str)
{
    if (space(str) != "mein wahlplakat")
    {
        FAILWP("verfasse mein wahlplakat?",FAIL_NOT_OBJ);
    }
    if (!check_security())
    {
        FAILWP("So nicht.",FAIL_INTERNAL);
    }
    string cname = this_player()->query_real_cap_name();
    string rname = this_player()->query_real_name();
    string *text = 0;
    if (!rname || !cname || rname != lower_case(cname))
    {
        FAILWP("Name nicht identifiziert.",FAIL_INTERNAL);
    }
    if (!SR_KANDIDATENLISTE->ist_kandidat(rname))
    {
        FAILWP("Du bist kein Kandidat, also kein Plakat.",FAIL_INTERNAL);
    }
    text = SR_KANDIDATENLISTE->query_my_plakat(cname);
    if (this_player()->mini_ed(#'ed_end, 0,0,0,([ //'
               MINI_ED_FORCE_WRAP:1,
               MINI_ED_TITLE:"Wahlplakat beschreiben"]),text))
    {
        return 1;
    }
    FAILWP("Aufruf Editor fehlgeschlagen.",FAIL_INTERNAL);
}

int ausgang(string str) // TODO: implementieren Rückkehrraum
{
    string outroom = _plir_get_old_room(TP);
    if (!outroom)
        FAILWP("Kein Ausgang gesetzt.",FAIL_INTERNAL);
    TP.move(outroom,([
        MOVE_MSG_ENTER: Der(TP)+" erscheint aus dem Wahluniversum.",
        MOVE_MSG_LEAVE: Der(TP)+" verlässt das Wahluniversum.",
        MOVE_MSG_ME: "Du verlässt das Wahluniverum.",
    ]));
    return 1;
}

<int|string> forbidden_move_in(mapping mv_infos)
{
    object ob = mv_infos[MOVE_OBJECT];
    if (playerp(ob) || my_animalp(ob)>0)
        return 0;
    string pn = program_name(ob)[..<3];
    if (pn == SR_KANDIDATENLISTE || pn == SRWAHL_URNE)
        return 0;
    return Der(ob)+" darf hier nicht rein!";
}

void notify_moved_in(mapping mv_infos)
{
    object ob = mv_infos[MOVE_OBJECT];
    object woher = mv_infos[MOVE_OLD_ROOM];
    _plir_set_old_room(ob,woher);
}

void notify_move_in(mapping mv_infos)
{
    object woher = mv_infos[MOVE_OLD_ROOM];
    if (!woher) return;
    string dom = woher->query_room_domain();
    if (!dom) return;
    set_room_domain(dom);// kein Cross-Domain-Log
}

void notify_move_out(mapping mv_infos)
{
    object wohin = mv_infos[MOVE_NEW_ROOM];
    if (!wohin) return;
    string dom = wohin->query_room_domain();
    if (!dom) return;
    set_room_domain(dom);// kein Cross-Domain-Log
}

void init()
{
  add_action("ausgang","ausgang",-1);
  add_action("cmd_verfasse","verfasse",-7);
}

void reset()
{
    get_urnen();
}

int query_wahl_manuell()
{
    return manuelle_wahl;
}

static void set_wahl_manuell(int wahl)
{
    switch (wahl) {
        case  SRWAHL_VORWAHL: break;
        case  SRWAHL_WAHL: break;
        case  SRWAHL_NACHWAHL: break;
        default: wahl=0;break;
    }
    if (manuelle_wahl != wahl)
    {
        manuelle_wahl = wahl;
        reset();
    }
    
}

string query_short(object betrachter)
{
    int wahl = get_wahl_flag(SRWAHL_ALLES);
    switch (wahl) {
        case  SRWAHL_VORWAHL: 
            return "Wahlraum vor der Spielerratswahl";
        case  SRWAHL_WAHL: 
            return "Wahlraum während der Spielerratswahl";
        case  SRWAHL_NACHWAHL: 
            return "Wahlraum nach der Spielerratswahl";
        default: 
            return "Wahlraum ausserhalb der Spielerratswahl";
    }
}

void create()
{
    "*"::create();
    set_short("Wahlraum");
    set_long("Ein Wahlraum mit einem Ausgang zur Rückkehr in den normalen Raum.\n"
        "      Weiter: Ausgang."
    );
    set_own_light(1);

    add_type(RT_KUNSTLICHT,1);
    add_type(RT_KAEMPFEN_VERBOTEN,1);
    add_type(RT_STEHLEN_VERBOTEN,1);
    add_type(RT_GRABEN_VERBOTEN,1);
    add_type(RT_MAGIE_VERBOTEN,1);
    add_type(RT_HANDWERK_VERBOTEN,1);
    add_type(RT_KEIN_STARTRAUM,1);
    add_type(RT_KEIN_VERBRAUCH,1);
    add_type(RT_KEIN_KOMPASS,1);
    add_type(RT_KEIN_CLEANUP,1);
    add_type(RT_VERSAND_REIN_VERBOTEN,1);
    add_type(RT_VERSAND_RAUS_VERBOTEN,1);
    add_type(RT_TELEPORT_REIN_VERBOTEN,1);
    add_type(RT_TELEPORT_RAUS_VERBOTEN,1);
    add_type(RT_SPERRGEBIET,1);

    add_type(RT_STARTRAUM,
           function string(object room)
           {
             return _plir_get_old_room(TP) || 0;
           } );

    add_controller("forbidden_move_in",TO);
    add_controller("notify_moved_in",TO);
    add_controller("notify_move_in",TO);
    add_controller("notify_move_out",TO);
    // Tafel was ist der SR
    // wer ist aktuell im SR
    // wer sind die Kandidaten
    // DONE: Plakate aller Kandidaten 
    reset();
}

void set_plir(mapping m)
{
    // printf("TO %Q\n",TO);
    // printf("PO %Q\n",PO);
    string* n1 = explode(object_name(TO),"/");
    string* n2 = explode(object_name(PO),"/");
    if (implode(n1[..<2],"/")!=implode(n2[..<2],"/"))
        return;
    if (n2[<1][0..0]!="$" || n2[<1][<1..<1]!="$" || n2[<1][1..<2] != n1[<1])
        return;
    plir = m;
    // write("DONE\n");
}

void prepare_renewal()
{
}

void abort_renewal()
{
}

void finish_renewal(object neu)
{
    neu->set_plir(plir);
}