// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/room/rathaus/obj/plakat_srwahl.c
// Description:

inherit "/i/item";
inherit "/i/install";

#include <invis.h>

#include <srwahl.h>

string query_long(object beo)
{
    string str = "Du siehst eine Darstellung eines betretbaren Wahlraums "
                 "auf diesem Wahlplakat: ";
    switch (SR_WAHLRAUM->get_wahl_flag(SRWAHL_ALLES))
    {
        default:
            return wrap("Das Wahlplakat löst sich auf.");
        case SRWAHL_VORWAHL:
            return wrap(str+"Die Meldephase der Kandidaten zur Spielerratswahl "
             "hat begonnen.");
        case SRWAHL_WAHL:
            return wrap(str+"Die Spielerratswahl läuft auch Hochtouren.");
        case SRWAHL_NACHWAHL:
            return wrap(str+"Die Ergebnisse der Spielerratswahl stehen zur Verfügung.");
    }
}

int query_invis()
{
    return (SR_WAHLRAUM->get_wahl_flag(SRWAHL_ALLES)>0) ? V_VIS : V_INVIS;
}

varargs string query_read(string parse_rest, string str,object leser)
{
    return query_long(leser);
}

string query_short(object beo)
{
    if (SR_WAHLRAUM.query_known_plir(beo)) {
        return "Ein unscheinbares Wahlplakat";
    } else {
        return "Ein auffäliges, betretbares Wahlplakat";
    }
}

void create() {
    set_id(({"wahlplakat","plakat",ID_SR_WAHLPLAKAT}));
    set_name("wahlplakat");
    set_gender("saechlich");
    add_v_item( ([
        "name" : "wahlraum",
        "gender" : "maennlich",
        "id" : ({ "wahlraum","wahl","raum"}),
        "long" : "Ein strahlend-schöner Wahlraum erwartet Dich zum Betreten und dort Wählen.",
        "look_msg" : "$Der(OBJ_TP) schaut $seinen('vitem) an.",
        "read" : "Nichts zu lesen.", // read_v_item_cl
        "read_msg" : "",
        "smell": "Frisch.",
        "smell_msg" : "$Der(OBJ_TP) riecht an $seinem('vitem).",
        "noise": "Geschäftiges Wahlen ist zu hören.",
        "hear_msg": "$Der(OBJ_TP) lauscht an $seinem('vitem).",
        "feel": "Fuehlt sich glatt an.",
        "feel_msg" : "$Der(OBJ_TP) fuehlt $seinen('vitem).",
        "take": "Der Wahlraum auf dem Schild ist nicht greifbar.", // take_v_item_cl
        "take_msg": "$Der(OBJ_TP) ruckelt an $seinem('vitem).",
        "enter_room": SR_WAHLRAUM,
        "enter_messages": ({
            "$Der(OBJ_TP) betritt $den('vehikel) auf dem Plakat.",
            "$Der(OBJ_TP) kommt in den Wahlraum."
        })
    ]) );

}
