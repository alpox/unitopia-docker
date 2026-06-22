// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/room/rathaus/keller/keller5.c
// Description: Kellerraum des Rathauses

inherit "%room";
inherit "%wasser/fluss";

#include <level.h>
#include <room_types.h>

void create()
{
    set_own_light(1);

    add_type(({RT_KUNSTLICHT, RT_KAEMPFEN_VERBOTEN}), 1);
    add_type(RT_FLUG_LANDEPLATZ, __PATH__(1) "treppe");

    set_short("Im Rathauskeller");
    set_long("Du befindest dich im Keller des Rathauses von Tadmor. "
             "Hier reiht sich ein Weinfass an dem anderen. "
             "Es spricht also vieles dafür, dass dies der Weinkeller ist.");

    add_v_item(([
        "name":   "fackeln",
        "gender": "weiblich",
        "plural": 1,
        "id":     ({"fackel", "fackeln", "licht"}),
        "long":   "Fackeln sorgen für etwas Licht in diesem Keller.",
        "feel":   "Die Fackeln sind richtig kalt.",
        "smell":  "Du riechst fast nichts.",
        "take":   "Die Fackeln sind fest an den Wänden montiert.",
    ]));

    add_v_item(([
        "name":   "wände",
        "gender": "weiblich",
        "plural": 1,
        "id":     ({"wand", "wände", "gewölbe", "decke", "stein", "steine"}),
        "long":   "Stein, noch ein Stein, und noch ein Stein. Wand.",
        "feel":   "Die Steine fühlen sich schön warm an.",
        "smell":  "Du riechst wirklich so gut wie nichts.",
        "take":   "Das Gewölbe bleibt hier.",
    ]));

    add_v_item(([
        "name":              "rotwein",
        "gender":            "maennlich",
        "id":                ({"rotwein", "wein", "# water #"}),
        "long":              "Der Wein hat eine perfekte rote Farbe.",
        "smell":             "Er hat ein angenehm fruchtiges Aroma.",
        "well_id":           "weinfass",
        "water_strength":    1,
        "water_amount":      10,
        "water_success_msg": "Solch köstlichen Rotwein hast du selten geschmeckt.",
    ]));

    foreach(int nr: 10)
        add_v_item(([
            "name":     "weinfass",
            "gender":   "saechlich",
            "id":       ({"weinfass", "fass", "# well #" }),
            "long":     "Ein riesiges Weinfass zur Aufbewahrung von Wein.",
            "feel":     "Es fühlt sich groß und stark an.",
            "smell":    "Es riecht nach Holz, Eiche mit einem Hauch von Trauben.",
            "take":     "Das Weinfass bekommst du hier nicht weg.",
            "water_id": "rotwein",
        ]));

    set_exits(({ "keller3", "keller5" }),
              ({ "osten",   "westen"  }));
}

void init()
{
    "*"::init();
}
