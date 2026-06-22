// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/room/rathaus/keller/keller3.c
// Description: Kellerraum des Rathauses

inherit "%room";

#include <level.h>
#include <room_types.h>

string filter_norden(object who)
{
    if (!wizp(who))
        return "Nach jedem Schritt bist du auf der gleichen Stelle wie zuvor. "
               "Du bewegst dich kein Stück nach Norden.";

    return 0;
}

string filter_westen(object who)
{
    if (!wizp(who))
        return "Du rennst gegen eine unsichtbare Wand. Aua!";

    return 0;
}

void create()
{
    set_own_light(1);

    add_type(({RT_KUNSTLICHT, RT_KAEMPFEN_VERBOTEN}), 1);
    add_type(RT_FLUG_LANDEPLATZ, __PATH__(1) "treppe");

    set_short("Im Rathauskeller");
    set_long("Du befindest dich im Keller des Rathauses von Tadmor. "
             "Im Norden befindet sich wohl der Vorratskeller des Pantheons, "
             "im Westen schließt sich der Weinkeller des Pantheons an. "
             "An den Wändern flackern Fackeln fröhlich vor sich hin, "
             "aber auch im Westen und Norden flimmert es merkwürdig.");

    add_v_item(([
        "name":   "fackeln",
        "gender": "weiblich",
        "plural": 1,
        "id":     ({"fackel", "fackeln", "licht"}),
        "long":   "Die Fackeln leuchten in einem gemütlichen warmen Farbenspektrum. "
                  "Sie flackern so gemütlich gleichmäßig, dass du dabei einschlafen könntest.",
        "feel":   "Die Fackeln sind frostig kalt.",
        "smell":  "Du riechst nicht so gut wie nichts.",
        "take":   "Die Fackeln sind fest an den Wänden montiert.",
    ]));

    add_v_item(([
        "name":   "wände",
        "gender": "weiblich",
        "plural": 1,
        "id":     ({"wand", "wände", "gewölbe", "decke", "stein", "steine"}),
        "long":   "Viele Steine haben sich hier zusammengefunden, "
                  "um gemeinsam das Gewicht des darüberliegenden Rathauses "
                  "zu tragen und von dir fernzuhalten.",
        "feel":   "Die Steine fühlen sich angenehm warm an.",
        "smell":  "Du riechst wirklich nichts.",
        "take":   "Das Gewölbe bleibt hier.",
    ]));

    add_v_item(([
        "name":   "vorratskeller",
        "gender": "maennlich",
        "id":     ({"vorratskeller", "norden"}),
        "far":    1,
        "long":   "Im Norden erahnst Du den Vorratskeller des Pantheons. "
                  "Was da wohl gelagert wird?",
    ]));

    add_v_item(([
        "name":   "weinkeller",
        "gender": "maennlich",
        "id":     ({"weinkeller", "westen"}),
        "far":    1,
        "long":   "Im Westen befindet sich der Weinkeller des Pantheons.",
    ]));

    set_exits(({ "keller1", "keller4", "keller5" }),
              ({ "osten",   "norden",  "westen"  }));
}
