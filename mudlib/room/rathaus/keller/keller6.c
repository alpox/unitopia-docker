// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/room/rathaus/keller/keller6.c
// Description: Kellerraum des Rathauses

inherit "%room";

#include <room_types.h>

void create()
{
    set_own_light(1);

    add_type(({RT_KUNSTLICHT, RT_KAEMPFEN_VERBOTEN}), 1);
    add_type(RT_FLUG_LANDEPLATZ, __PATH__(1) "treppe");

    set_short("Im Rathauskeller");
    set_long("Du befindest dich im Keller des Rathauses von Tadmor. "
             "Hier trifft ein Gang aus dem Süden auf einen Gang aus dem Osten. "
             "Was die beiden Gänge jedoch hier veranstalten, bleibt wohl ihr Geheimnis. "
             "Die Fackeln an den Wändern sind hier jedoch nicht so zahlreich wie im Süden.");

    add_v_item(([
        "name":   "fackeln",
        "gender": "weiblich",
        "plural": 1,
        "id":     ({"fackel", "fackeln", "licht"}),
        "long":   "Wenige Fackeln geben eine leichte Andeutung von Dunkelheit.",
        "feel":   "Die Fackeln sind besonders kalt.",
        "smell":  "Du riechst echt nichts.",
        "take":   "Die Fackeln sind fest an den Wänden montiert.",
    ]));

    add_v_item(([
        "name":   "wände",
        "gender": "weiblich",
        "plural": 1,
        "id":     ({"wand", "wände", "gewölbe", "decke", "stein", "steine", "gang", "bogen"}),
        "long":   "Viele Steine vollbringen hier die ehrenwerte Aufgabe, "
                  "den hier angetretenen zwei Gängen einen festen Rahmen zu geben, "
                  "um die überirdischen Massen abzuhalten, eben diese Gänge zu verdrängen.",
        "feel":   "Die Steine fühlen sich sauwarm an.",
        "smell":  "Du riechst kaum irgendetwas.",
        "take":   "Das Gewölbe bleibt hier.",
    ]));

    set_exits(({ "keller2", "keller7" }),
              ({ "süden",   "osten"  }));
}
