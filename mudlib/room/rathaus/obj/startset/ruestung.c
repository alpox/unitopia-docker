// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	    /room/rathaus/obj/knochenmesser.c
// Description: Teil des Startersets für kleine Spieler, welches sie 
//              nach Armageddon zugesteckt bekommen.
        
inherit "/i/armour/armour";

//--------------------------------------------------------------------
void create()
{
    "*"::create();

    init_armour("oberkoerper", 30, 70, 70);
    set_name("lederrüstung");
    set_gender("weiblich");
    set_id( ({ "lederrüstung", "cloth # meta # alle" }) );
    set_adjektiv( ({ "leicht" }) );
    set_material("leder");

    set_long("Dies ist eine Rüstung aus weichem, aber doch stabilen "
        "Schweinsleder, welche deinen Oberkörper gut schützt.");
    set_smell("Du riechst Lederfett.");
    set_feel("Das Leder ist weich gegerbt.");
    set_noise("Die Rüstung raschelt bei Bewegungen leise.");


    set_broken_message("RITSCH!!! Deine Lederrüstung ist aufgerissen.");
    set_broken_adjektiv("aufgerissen");

    add_v_item(([
    "name":     "schweinsleder",
    "gender":   "saechlich",
    "id":       ({"schweinsleder","schweineleder","leder" }),
    "adjektiv": ({"hellbraun" }),
    "long":     "Die Rüstung wurde aus hellbraunem Schweinsleder "
                "gefertigt.",
    "feel":     "Das Leder ist weich gegerbt.",
    "noise":    "Das Leder raschelt bei Bewegungen leise.",
    "smell":    "Du riechst Lederfett.",
    ]));

}

