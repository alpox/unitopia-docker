// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/room/rathaus/keller/keller1.c
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
             "Fackeln an den Wänden tauchen das Gewölbe in ein "
             "atmosphärisches Licht. Nach Norden und Westen führen "
             "dunkle Gänge. Eine Wendeltreppe führt wieder nach oben.");

    add_v_item(([
        "name":   "fackeln",
        "gender": "weiblich",
        "plural": 1,
        "id":     ({"fackel", "fackeln", "licht"}),
        "long":   "Die Fackeln hüllen den Keller in ein gespenstisches Licht "
                  "und flackern dabei unheilvoll im gleichen Takt.",
        "feel":   "Die Fackeln sind eiskalt.",
        "smell":  "Du riechst nicht das geringste.",
        "take":   "Die Fackeln sind fest an den Wänden montiert.",
    ]));

    add_v_item(([
        "name":   "wände",
        "gender": "weiblich",
        "plural": 1,
        "id":     ({"wand", "wände", "gewölbe", "decke", "stein", "steine"}),
        "long":   "Das Gewölbe besteht aus einzelnen Steinen, die kunstvoll "
                  "zu einem Bogen zusammengesetzt wurden, und die ehrenvolle "
                  "Aufgabe haben, den Besucher davor zu schützen, von den "
                  "darüberliegenden Erdmassen verschüttet zu werden.",
        "feel":   "Die Steine fühlen sich recht warm an.",
        "smell":  "Du riechst gar nichts.",
        "take":   "Das Gewölbe bleibt hier.",
    ]));

    add_v_item(([
        "name":   "wendeltreppe",
        "gender": "weiblich",
        "id":     ({"wendeltreppe", "treppe"}),
        "long":   "Eine Treppe wendelt sich hier nach oben. "
                  "Auf ihrer Oberseite sind Stufen eingearbeitet, womit man "
                  "also tatsächlich nach oben gelangen könnte.",
        "feel":   "Sie fühlt sich merkwürdig weich und kuschelig an.",
        "smell":  "Du riechst überhaupt nichts.",
    ]));

    set_exits(({ "keller2", "keller3", "../foyer" }),
              ({ "norden",  "westen",  "hoch" }));
}
