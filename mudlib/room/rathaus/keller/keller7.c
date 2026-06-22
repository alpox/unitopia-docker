// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/room/rathaus/keller/keller7.c
// Description: Kellerraum des Rathauses

inherit "%room";

#include <room_types.h>

void create()
{
    object portal;

    set_own_light(1);

    add_type(({RT_KUNSTLICHT, RT_KAEMPFEN_VERBOTEN}), 1);
    add_type(RT_FLUG_LANDEPLATZ, __PATH__(1) "treppe");

    set_short("Im Rathauskeller");
    set_long("Du befindest dich im Keller des Rathauses von Tadmor. "
             "Hier endet ein aus Westen kommender Gang abrupt. "
             "Dies könnte durchaus eine Sackgasse sein, wenn da am Ende "
             "nicht ein Portal stehen würde. Links und rechts vom Portal "
             "flackern Fackeln unheilvoll.");

    add_v_item(([
        "name":   "fackeln",
        "gender": "weiblich",
        "plural": 1,
        "id":     ({"fackel", "fackeln", "licht"}),
        "long":   "Die Fackeln flackern im Takt und werfen so einen warmen "
                  "Lichtschein auf das Portal.",
        "feel":   "Die Fackeln sind echt kalt.",
        "smell":  "Du riechst so gut wie nichts.",
        "take":   "Die Fackeln sind fest an den Wänden montiert.",
    ]));

    add_v_item(([
        "name":   "wände",
        "gender": "weiblich",
        "plural": 1,
        "id":     ({"wand", "wände", "gewölbe", "decke", "stein", "steine", "gang", "kuppel"}),
        "long":   "Mehrere Steine wurden hier kunstvoll zu einer Kuppel arrangiert und "
                  "geben den Eindruck einer gewissen Größe.",
        "feel":   "Die Steine fühlen sich herrlich warm an.",
        "smell":  "Du riechst schwerlich irgendetwas.",
        "take":   "Das Gewölbe bleibt hier.",
    ]));

    set_exits(({ "keller6" }),
              ({ "westen"  }));

#ifdef UNItopia
    portal = clone_object("%portal");
    portal.set_portal_name("LPPortal");
    portal.set_long("Ein Portal, welches in eine lang vergessene Vergangenheit führt. "
                    "Betritt es nicht, wenn Du kein Englisch kannst.");
    portal.move(this_object());
#endif
}
