// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/room/rathaus/keller/keller2.c
// Description: Kellerraum des Rathauses

inherit "%room";

#include <room_types.h>

void create()
{
    set_own_light(1);

    add_type(({RT_KUNSTLICHT, RT_KAEMPFEN_VERBOTEN}), 1);
    add_type(RT_FLUG_LANDEPLATZ, __PATH__(1) "treppe");

    set_short("Im Rathauskeller");
    set_long("Du befindest dich im Keller des Rathauses von Tadmor, "
             "genauer in einem Gang, der oft von Süden nach Norden führt, "
             "manchmal jedoch auch von Norden nach Süden.");

    add_v_item(([
        "name":   "fackeln",
        "gender": "weiblich",
        "plural": 1,
        "id":     ({"fackel", "fackeln", "licht"}),
        "long":   "Fackeln an den Wänden begleiten flackernd den Besucher.",
        "feel":   "Die Fackeln sind richtig kalt.",
        "smell":  "Du riechst nichts besonderes.",
        "take":   "Die Fackeln sind fest an den Wänden montiert.",
    ]));

    add_v_item(([
        "name":   "wände",
        "gender": "weiblich",
        "plural": 1,
        "id":     ({"wand", "wände", "gewölbe", "decke", "stein", "steine", "gang"}),
        "long":   "Mehrere Steine bilden gemeinsam diesen Gang und "
                  "führen den geneigten Besucher zielgerichtet nach Norden und Süden.",
        "feel":   "Die Steine fühlen sich sehr warm an.",
        "smell":  "Du riechst kaum etwas.",
        "take":   "Das Gewölbe bleibt hier.",
    ]));

    set_exits(({ "keller1", "keller6" }),
              ({ "süden",   "norden"  }));
}
