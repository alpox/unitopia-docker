// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	    /room/rathaus/obj/schild.c
// Description: Teil des Startersets für kleine Spieler, welches sie 
//              nach Armageddon zugesteckt bekommen.
        
inherit "/i/weapon/defensiv_waffe";

#include <misc.h>

//--------------------------------------------------------------------
string query_genitiv()
{
    return "holzschildes";
}

//--------------------------------------------------------------------
void create()
{
    "*"::create();
    
    init_weapon("kleinschild", 80, 80, 80);

    set_name("holzschild");
    set_gender("maennlich");
    set_id( ({ "holzschild", "schild" }) );
    set_adjektiv( ({ "klein" }) );
    set_material(({"holz"}));

    set_long("Die ist ein einfacher Schild aus Holz. Er ist "
        "rechteckig, es wurden einfach mehrere Bretter aneinander "
        "befestigt. An der Rückseite siehst du einen Griff. Auf die "
        "Vorderseite wurde ein Symbol gemalt.");
    set_feel("Du kannst die Maserung des Holzes erfühlen.");

    set_broken_message("KRACH!!! Dein Schild ist auseinander "
        "gebrochen.");
    set_broken_adjektiv("auseinandergebrochen");

    add_v_item(([
    "name":     "bretter",
    "gender":   "saechlich",
    "plural":   1,
    "id":       ({"bretter","holz" }),
    "long":     "Vier längliche Bretter wurden zu einem rechteckigen "
                "Schild zusammengefügt.",
    "feel":     "Du fühlst die Maserung des Holzes.",
    ]));

    add_v_item(([
    "name":     "griff",
    "gender":   "maennlich",
    "id":       ({"griff","lederriemen","riemen" }),
    "long":     "Der Griff ist ein einfacher Lederriemen. Damit "
                "kannst du den Schild führen.",
    "feel":     "Das Leder fühlt sich stabil an, damit kannst du "
                "den Schild sicherlich problemlos führen.",
    ]));

    add_v_item(([
    "name":     "maserung",
    "gender":   "maennlich",
    "id":       ({"maserung","masern" }),
    "long":     "Die Bretter sind in Längsrichtung gemasert.",
    "feel":     "Das Holz ich nicht sehr glatt, du kannst die "
                "Masern deutlich fühlen.",
    ]));

    add_v_item(([
    "name":     "symbol",
    "gender":   "saechlich",
    "id":       ({"symbol","vorderseite","bemalung","kreis","funken"}),
    "long":     "Vorn auf das Schild ist ein schwarzer Kreis mit "
                "einem goldenen Funken in der Mitte gemalt.",
    "feel":     "Mit etwas Fingerspitzengefühl lässt sich die "
                "Bemalung ertasten.",
    "smell":    "Ganz schwach riechst du noch die Farbe.",
    ]));
}

//--------------------------------------------------------------------
int query_sellable()
{
    return 0;
}

//--------------------------------------------------------------------
int query_repairable()
{
    return 0;
}
