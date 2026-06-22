// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	    /room/rathaus/obj/startset.c
// Description: Ein Rucksack mit Startausrüstung für kleine Spieler 
//              nach Arma
        
inherit "/i/object/rucksack";

#include <misc.h>

//--------------------------------------------------------------------
void fuelle_rucksack()
{
    object obj; 

    open_con();
    
    clone_object("knochenmesser").move_or_remove(TO);
    
    clone_object("ruestung").move_or_remove(TO);

    clone_object("schild").move_or_remove(TO);

    obj = clone_object("/obj/flasche");
    obj.set_max_content(8);
    obj.set_content(8);
    obj.set_water(clone_object("/obj/wasser"));
    obj.set_value(0);
    obj.move_or_remove(TO);

    obj = clone_object("/obj/fackel");
    obj.set_value(0);
    obj.move_or_remove(TO);

    obj = clone_object("/obj/nahrung");
    obj.set_long("Ein ordentlicher Kanten Brot.");
    obj.set_feel("Das Brot ist ganz weich.");
    obj.set_smell("Es riecht ganz frisch.");
    obj.set_menge((["name":"kanten","gender":"maennlich"]));
    obj.set_amount(100);
    obj.set_dauer(150);
    obj.move_or_remove(TO);

    close_con();
}

//--------------------------------------------------------------------
void create()
{
    "*"::create();

    add_id( ({ "cloth # meta # alle" }) );
    set_adjektiv( ({ "robust" }) );

    set_long("Das ist ein einfacher, aber sehr robuster Rucksack aus "
        "grünbraunem Stoff. Darauf ist ein Schriftzug aus schwarzen "
        "Buchstaben zu sehen.");
    set_smell("Der Rucksack riecht nicht mehr ganz frisch.");
    set_feel("Du fühlst rauen Stoff.");
    set_noise("Der Rucksack raschelt, wenn du ihn bewegst.");
    set_read("Darauf steht der Schriftzug \"ARMAGEDDON\".");

    set_max_internal_encumbrance(8);
    set_weight(1);

    add_v_item(([
    "name":     "material",
    "gender":   "saechlich",
    "id":       ({"stoff","material" }),
    "adjektiv": ({"grünbraun" }),
    "material": "textil",
    "long":     "Das ist grob gewebter Stoff in mehreren Lagen. "
                "Er sieht sehr robust aus.",
    "feel":     "Der Stoff ist rau.",
    "noise":    "Der Stoff macht ein leises, knatterndes Geräusch, "
                "wenn du mit der Hand darüber fährst.",
    "hear_msg": "$Der(OBJ_TP) fährt mit der Hand über den rauen "
                "Stoff $des(OBJ_TO), es macht ein leises, "
                "knatterndes Geräusch.",
    "smell":    "Der Stoff hat allerlei Gerüche aufgenommen, es ist "
                "kaum möglich, da etwas Bestimmtes "
                "herauszuschnuppern.",
    ]));

    add_v_item(([
    "name":     "schriftzug",
    "gender":   "maennlich",
    "id":       ({"schriftzug","schrift","beschriftung",
                  "druckbuchstaben","buchstaben" }),
    "adjektiv": ({"schwarz" }),
    "long":     "In schwarzen Druckbuchstaben wurde etwas "
                "auf den Rucksack geschrieben. Man kann es lesen.",
    "read":     "ARMAGEDDON",
    ]));

    fuelle_rucksack();
}

