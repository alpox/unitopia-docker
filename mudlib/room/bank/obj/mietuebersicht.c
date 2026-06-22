// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /room/bank/obj/mietkontobuch.c
// Description: Ein Kontofuehrungsbuch
// Author:	Myonara

inherit "/i/item";
inherit "/i/move";
inherit "/i/value";
inherit "/i/tools/dynamic_browser";

#include <database.h>
#include <dynamic_browser.h>
#include <invis.h>
#include <misc.h>

#include <money.h>

private string owner="";

public string query_owner() { return owner; }

void setup_renamed_object(object vc, string old_name)
{
    string on = explode(ON(TO),"/")[<1];
    string *split = explode(on,"_");
    if (sizeof(split)==2)
    {
        owner = split[1];
    }
}

string query_long(object viewer)
{
    if (RNAME(viewer)==owner)
        return wrap("Dein magisches Pergament zeigt die aktuelle Übersicht "
            "über alle Schließfachmieten und alle Banken. "
            "Auf der Rückseite befindet sich die Fächerübersicht.");
    object pl = find_player(owner);
    string cname = (pl && !IS_INVIS(pl)) 
        ? pl->query_real_cap_name()
        : capitalize(owner);
    return "Das magische Pergament über die Schließfächer von "+cname+".\n";
}

varargs string query_read(string parse_rest, string str,object leser)
{
    return ZENTRALBANK->get_konten_uebersicht(owner,leser);
}

string query_read_msg()
{
    if (TP_RN==owner) return "";
    return wrap(Der(TP)+" liest die Mietkontenübersicht von "+CAP(owner)+".");
        
}

//-----------------------------------------------------------------------------
static mixed fach_uebersicht_init(mapping old)
{
    old[B_START_LINE] =0;
    old[ZB_BANK_OWNER] = owner;
    return old;
}

static int fach_uebersicht_total(mapping menue)
{
    return ZENTRALBANK->get_fach_detailanzeige(menue,1);
}

static string * fach_uebersicht_display(mapping menue)
{
    string * lines = ZENTRALBANK->get_fach_detailanzeige(
        menue+([
            DB_DBG_LIMIT:menue[B_NUM_LINES],
            DB_DBG_OFFSET:menue[B_CURRENT_LINE]
            ]),0);
    if (lines == 0 || !pointerp(lines))
        return ({ "Keine Anzeige." });
    return lines;
}

static string fach_uebersicht_prompt(mapping menue)
{
    if (menue[ZB_BANK_SORTBY]== ZB_BANK_HINT)
    {
        return sprintf(
            "Einlagerungen für %s (sortiert nach Name) %d,%d [s,q]",
            capitalize(owner),menue[B_CURRENT_LINE],
            menue[B_END_LINE]);
    }
    else
    {
        return sprintf(
            "Einlagerungen für %s (sortiert nach Bank) %d,%d [s,q]",
            capitalize(owner),menue[B_CURRENT_LINE],
            menue[B_END_LINE]);
    }
}

static mixed fach_uebersicht_action(string str, mapping * menues)
{
    str = lower_case(space(str));
    if (!pointerp(menues) || sizeof(menues)==0 || !mappingp(menues[<1])) 
    {
        return B_QUIT; // Abbruch, TODO Fehlercode??
    }
    mapping menue = menues[<1];
    int start = menue[B_CURRENT_LINE];
    int altstart = start;
    int end = fach_uebersicht_total(menue);
    switch (str) 
    {
    case "s":
        menue[ZB_BANK_SORTBY] = 
                (menue[ZB_BANK_SORTBY] == ZB_BANK_HINT 
                ? ZB_BANK_ID : ZB_BANK_HINT);
        menue[B_CURRENT_LINE] = 0;
        menue[B_FLAGS] |= BF_DIRTY;
        menues[<1] = menue;
        return menues;
    case "":
    case "d":
        start += menue[B_NUM_LINES];
        if (start >= end) { start = end-1; }
        if (altstart == start) return B_QUIT;
        menue[B_CURRENT_LINE] = start;
        menues[<1] = menue;
        return menues;
    case "u":
        if (start <= 0) {
            browse_write_line("Anfang der Daten erreicht.\n");
        }
        start = max(0, start - menue[B_NUM_LINES]);
        menue[B_CURRENT_LINE] = start;
        menues[<1] = menue;
        return menues;
    case ">":
        menue[B_CURRENT_LINE] = max(end - menue[B_NUM_LINES],0);
        menues[<1] = menue;
        return menues;
    case "<":
        menue[B_CURRENT_LINE] = 0;
        menues[<1] = menue;
        return menues;
    case "q":
        return B_QUIT;
    }        
    return B_NOTHING;
}
//-----------------------------------------------------------------------------

string read_rueckseite(string parse_rest, string str, 
        mapping was, object leser)
{
    dynamic_browse( ([
            B_TYPE:"fach_uebersicht",
            ZB_BANK_SORTBY: ZB_BANK_ID,
            ]) );
    return "";
}


void create() 
{
    "*"::create();
    set_name("pergament");
    set_gender("saechlich");
    set_id( ({ "pergament","mietübersicht","übersicht","liste", 
        ZB_MIETUEBERSICHT_ID }) );
    set_adjektiv("magisch");
     add_v_item( ([
        "name" : "rückseite",
        "id" : ({ "rückseite","fachuebersicht" }),
        "gender": "weiblich",
        "long" : "Die Rückseite beinhaltet in sehr kleiner Schrift die "
            "Fachuebersicht zum lesen, bei der man mit s die Sortierung "
            "während dem Lesen umschalten kann.",
        "read" : #'read_rueckseite,
          "take" : "Nichts zu holen.",
          "look_msg" : "",
          "read_msg" : "",
          "take_msg" : "",
          "feel_msg" : "",
          "smell_msg": "",
          "hear_msg" : "",
    ]) );
}


void prepare_renewal() {}
void abort_renewal() {}
void finish_renewal(object neu) {}
