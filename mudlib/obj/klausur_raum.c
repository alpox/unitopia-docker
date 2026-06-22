// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:            /obj/klausur_raum.c
// Description:     Ein Klausurraum pro Spieler(VC ueber realname)
// Author:          Myonara.

inherit "/i/room";
#ifdef UNItopia
inherit "/p/Misc/i/array";
#endif

#include <message.h>
#include <misc.h>
#include <more.h>
#include <move.h>
#include <notify_fail.h>
#include <parse_com.h>
#include <soul.h>
#include <protokoll.h>

#define DEBUGGER "myonara"
#include <debug.h>

#define SAVE_CALL_OUT  61

private string owner;
private int sperre;
private int pr_aktiv;
private int flag_save;
private string *iprotokoll = ({});
private mapping protokoll;

void setup_renamed_object(object vc, string old_name)
{
    string str,rn;
    str = ON(TO);
    if (sscanf(str, PROTOKOLL_VC+"klausur_%s", rn) && player_exists(rn)) 
    {
        owner = rn;
    }
}

private mixed _save_protokoll()
{
    while (remove_call_out("save_protokoll")!=-1);
    if (!flag_save || !pointerp(iprotokoll) || !sizeof(iprotokoll))
        return 0;
#ifndef UNItopia
    return 0;
#else
    mixed result;
    string * prot = flatten_array(map(iprotokoll,(: explode($1,"\n")[..<2] :)));
    if (!mappingp(protokoll)) 
    {
        protokoll = PROTOKOLL_MASTER->get_empty_protokoll();
    }
    if (!pointerp(protokoll[PR_INHALT])) 
    {
        protokoll[PR_INHALT] = prot;
    } 
    else 
    {
        protokoll[PR_INHALT] += prot;
    }
    iprotokoll = ({ });
        // protokoll[PR_STICHWORTE] = einst[PR_STICHWORTE];
        // protokoll[PR_LESEZUGRIFFE] = einst[PR_LESEZUGRIFFE];
        // protokoll[PR_SCHREIBZUGRIFFE] = einst[PR_SCHREIBZUGRIFFE];
        // protokoll[PR_TITEL] = einst[PR_TITEL];
        protokoll[PR_AUTOR] = owner;
        // protokoll[PR_ZUSTAND] = PR_ZUSTAND_AKTIV;
        // protokoll[PR_TYPKLASSE] = einst[PR_TYPKLASSE];
    result = PROTOKOLL_MASTER->speichere_protokoll(protokoll);
    if (mappingp(result))
    {
        protokoll = result;
    }
    flag_save = 0;
    return result;
#endif
}

static void save_protokoll()
{
    _save_protokoll();
}

void receive_message(int msg_type, int msg_action, object who, string msg)
{
    if (!pr_aktiv || msg_type == MT_DEBUG) 
    {
        return;
    }
    else
    {
        iprotokoll += ({ msg });
        flag_save++;
        if (find_call_out("save_protokoll")==-1)
            call_out("save_protokoll",SAVE_CALL_OUT);
    }
    DEBUG(sprintf("receive_msg %Q %d %d",msg,msg_type,msg_action));
}

// ist keine add_action sondern wird ueber notify_seele aufgerufen.
void cmd_druecke(mapping vitem, object player)
{
    if (!mappingp(vitem) || vitem["name"] != "rufknopf") {
        send_message_to(player, MT_NOTIFY, MA_CRAFT, wrap(
                        "drücke rufknopf?"));
        return;
    }
    if (RN(player) != owner) {
        send_message_to(player, MT_NOTIFY, MA_CRAFT, wrap(
                        "Nur beim Eigentümer funktioniert der Rufknopf."));
        return;
    }
    object proto = touch(PROTOKOLL_VC+"protokollant_"+owner);
    if (objectp(proto)) 
    {
        proto->move(TO,([MOVE_FLAGS:MOVE_NORMAL,
            MOVE_MSG_LEAVE:"$Ein() verschwindet.",
            MOVE_MSG_ENTER:"$Der() kommt heran." ]));
    }
}

void notify_seele(object wer, mixed wen, string what, string
                 adverb, int align, int flags, int msg_typ_wer, int
                 msg_typ_wen, int msg_typ_andere)
{
    if (mappingp(wen) && wen["name"] == "rufknopf" && what == "drück") {
        call_out("cmd_druecke",0,wen, wer);
    }
}

<int|string> forbidden_move_in(mapping mv_infos)
{
    if (!sperre) return 0; // Ohne Sperre nix pruefen.
    if (playerp(mv_infos[MOVE_OBJECT]) && RN(mv_infos[MOVE_OBJECT]) == owner)
        return 0; // Der Eigentuemer kann nicht ausgesperrt werden.
    return "Der Zugang zu dem Raum wurde gesperrt, "+
        "frage "+capitalize(owner)+", um Zugang zu erlangen.";
}

int cmd_schiebe(string str)
{
    mixed * parsed;
    str = lower_case(space(str));
    parsed = parse_com(str,TO, 0, 0);
    if(parse_com_error(parsed, "Was willst Du schieben?\n", 1)) {
        return 0;
    }
    if (parsed[PARSE_ID] != "riegel") {
        FAILWP("schiebe riegel auf/zu?", FAIL_WRONG_ARG);
    }
    switch (parsed[PARSE_REST]) {
    case "auf":
    case "zurück":
    case "zurueck":
        sperre = 0;
        send_message_to(TP, MT_NOTIFY, MA_CRAFT, wrap(
                        "Der Riegel wurde zurückgeschoben, der Raum kann "
                        "wieder betreten werden."));
        return 1;
    case "vor":
    case "zu":
        sperre = 1;
        send_message_to(TP, MT_NOTIFY, MA_CRAFT, 
                        "Der Riegel wurde vorgeschoben, der Raum kann "
                        "nicht betreten werden.");
        return 1;
    default:
        FAILWP("schiebe riegel auf/zu?", FAIL_WRONG_ARG);
    }
}

int cmd_drehe(string str)
{
    mixed * parsed;
    str = lower_case(space(str));
    parsed = parse_com(str,TO, 0, 0);
    if(parse_com_error(parsed, "Was willst Du drehen?\n", 1)) {
        return 0;
    }
    if (QUERY("name",parsed[PARSE_OBS][0]) != "aufzeichnungslampe") {
        FAILWP("drehe aufzeichnungslampe?", FAIL_WRONG_ARG);
    }
    if (pr_aktiv) 
    {
        pr_aktiv = 0;
        send_message(MT_NOTIFY, MA_CRAFT, 
            "Die Aufzeichnungslampe wurde ausgedreht, und damit "
            "die Aufzeichnung pausiert.");
    }
    else
    {
        send_message(MT_NOTIFY, MA_CRAFT, 
            "Die Aufzeichnungslampe wurde angedreht, und damit "
            "die Aufzeichnung fortgesetzt.");
        pr_aktiv = 1;
    }
    return 1;
}


void init()
{
    add_action("cmd_schiebe","schiebe",-6);
    add_action("cmd_drehe","drehe",-4);
}

string long_riegel(mapping vitem, object viewer)
{
    return "Ein Riegel zum auf oder zu schieben. Bei vorgeschobenem "
                    "Riegel kann niemand den Raum betreten. "
                    "Der Riegel ist "
                    +(sperre?"vorgeschoben.":"zurückgeschoben.");
}

string long_lampe(mapping vitem, object viewer)
{
    return "Die drehbare Aufzeichnungslampe "
        +(pr_aktiv ? "leuchtet rot." : "ist aus.");
}

string read_protokoll(string parse_rest, 
            string str,mapping vitem, object leser)
{
    if (find_call_out("save_protokoll")!=-1)
    {
        save_protokoll();
    }
    if (mappingp(protokoll) && pointerp(protokoll[PR_INHALT]))
    {
        leser->more(protokoll[PR_INHALT],
            "[Zeile %d von %d, weiter mit Return]", 0, M_AUTO_END);
        return "";
    }
    else
    {
        return "Kein Protkoll vorhanden.\n";
    }
}

void create() {
    set_short("In Klausur (bitte nicht stoeren)");
    set_long("Du bist in einem Klausurraum. Für Außenstehende heißt das, "
            "dass die Anwesenden nicht gestört werden wollen.");
    // IDEE Eigentuemer im Langtext erwaehnen
    // IDEE Ruheraum an und abschaltbar und im Langtext anzeigen.
    set_exit("/room/church", "runter");
    set_own_light(1);
    add_type("kunstlicht", 1);
    set_room_domain("Pantheon");
    add_v_item( ([
            "name" : "rufknopf",
            "id" : ({ "rufknopf" }),
            "gender" : "maennlich",
            "long": "Ein drückbare Rufknopf für den Protokollanten, "
                "nur für den Eigentümer des Klausurraums.",
            "look_msg" : "",
            "read" : "Nichts zu lesen.",
            "read_msg" : "",
            "smell": "Nichts zu riechen.",
            "smell_msg" : "",
            "noise": "Nichts zu hoeren.",
            "hear_msg": "",
            "feel": "Fuehlt sich glatt an.",
            "feel_msg" : "",
        ]) );
    add_v_item( ([
            "name" : "riegel",
            "id" : ({ "riegel" }),
            "gender" : "maennlich",
            "long": #'long_riegel,
            "look_msg" : "",
            "read" : "Nichts zu lesen.",
            "read_msg" : "",
            "smell": "Nichts zu riechen.",
            "smell_msg" : "",
            "noise": "Nichts zu hoeren.",
            "hear_msg": "",
            "feel": "Fuehlt sich glatt an.",
            "feel_msg" : "",
        ]) );
    add_v_item( ([
            "name" : "aufzeichnungslampe",
            "id" : ({ "aufzeichnungslampe","lampe" }),
            "gender" : "weiblich",
            "long": #'long_lampe,
            "look_msg" : "",
            "read" : "Nichts zu lesen.",
            "read_msg" : "",
            "smell": "Nichts zu riechen.",
            "smell_msg" : "",
            "noise": "Nichts zu hoeren.",
            "hear_msg": "",
            "feel": "Fuehlt sich glatt an.",
            "feel_msg" : "",
        ]) );
    add_v_item( ([
            "name" : "protokoll",
            "id" : ({ "protokoll" }),
            "gender" : "saechlich",
            "long": "Das aufgezeichnete Protokoll zum Lesen.",
            "look_msg" : "",
            "read":#'read_protokoll,
            "read_msg" : "",
            "smell": "Nichts zu riechen.",
            "smell_msg" : "",
            "noise": "Nichts zu hoeren.",
            "hear_msg": "",
            "feel": "Fuehlt sich glatt an.",
            "feel_msg" : "",
        ]) );
    add_controller( 
        ({  "notify_seele",
            "forbidden_move_in" }), TO);
}

void abort_renewal() {}
void finish_renewal(object neu) {}
void prepare_renewal() {}