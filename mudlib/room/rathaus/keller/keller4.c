// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/room/rathaus/keller/keller4.c
// Description: Kellerraum des Rathauses

inherit "%room";

#include <level.h>
#include <room_types.h>

void create()
{
    set_own_light(1);

    add_type(({RT_KUNSTLICHT, RT_KAEMPFEN_VERBOTEN}), 1);
    add_type(RT_FLUG_LANDEPLATZ, __PATH__(1) "treppe");

    set_short("Im Rathauskeller");
    set_long("Du befindest dich im Keller des Rathauses von Tadmor. "
             "Auf der einen Seite siehst Du einen Haufen Spanferkel, "
             "daneben stehen Fässer voll Nektar und Ambrosia, "
             "in einem hinteren Winkel stehen Kisten voller Obst "
             "neben einem Haufen voller Schokolade. "
             "An der Decke hängen Engelsflügel und auf der Seite "
             "stehen ein paar nagelneue Blitze. "
             "Bei diesem ganzen Chaos hier verlierst Du doch glatt den Überblick!");

    add_v_item(([
        "name":   "fackeln",
        "gender": "weiblich",
        "plural": 1,
        "id":     ({"fackel", "fackeln", "licht"}),
        "long":   "Fackeln sorgen für einen angenehmen Kontrast zu den Blitzen.",
        "feel":   "Die Fackeln sind erstaunlich kalt.",
        "smell":  "Du riechst nix.",
        "take":   "Die Fackeln sind fest an den Wänden montiert.",
    ]));

    add_v_item(([
        "name":   "wände",
        "gender": "weiblich",
        "plural": 1,
        "id":     ({"wand", "wände", "gewölbe", "decke", "stein", "steine"}),
        "long":   "Du kannst die Wände vor lauter unnützer Nahrung kaum noch erkennen.",
        "feel":   "Die Steine fühlen sich erstaunlich warm an.",
        "smell":  "Du riechst wirklich nix.",
        "take":   "Das Gewölbe bleibt hier.",
    ]));

    add_v_item(([
        "name":   "spanferkel",
        "gender": "saechlich",
        "plural": 1,
        "long":   "Viele leckere Spanferkel.",
        "smell":  "Da läuft dir das Wasser im Mund zusammen.",
        "take":   "Ohne den richtigen Zauberspruch geht das nicht.",
    ]));

    add_v_item(([
        "name":   "nektar",
        "gender": "maennlich",
        "id":     ({"nektar", "fass", "fässer"}),
        "long":   "Mehrere Fässer voller Nektar.",
        "smell":  "Die Fässer riechen nach Spuren von Holz.",
        "take":   "Ohne den richtigen Zauberspruch geht das nicht.",
    ]));

    add_v_item(([
        "name":   "ambrosia",
        "gender": "weiblich",
        "id":     ({"ambrosia", "fass", "fässer"}),
        "long":   "Mehrere Fässer voller Ambrosia.",
        "smell":  "Die Fässer riechen nach Spuren von Holz.",
        "take":   "Ohne den richtigen Zauberspruch geht das nicht.",
    ]));

    add_v_item(([
        "name":   "obst",
        "gender": "saechlich",
        "id":     ({"obst", "kisten"}),
        "long":   "Mehrere Kisten voller Obst.",
        "smell":  "Die Kisten riechen nach Spuren von Holz.",
        "take":   "Ohne den richtigen Zauberspruch geht das nicht.",
    ]));

    add_v_item(([
        "name":   "schokolade",
        "gender": "weiblich",
        "id":     ({"schokolade", "tafelschokolade", "haufen"}),
        "long":   "Ein großer Haufen voller Pralinen und in Tafeln abgepackter Schokolade.",
        "smell":  "Du riechst ... Papier.",
        "take":   "Ohne den richtigen Zauberspruch geht das nicht.",
    ]));

    add_v_item(([
        "name":   "engelsflügel",
        "gender": "männlich",
        "plural": 1,
        "id":     ({"flügel", "engelsflügel"}),
        "long":   "Etliche Engelsflügel hängen mit einem Strick von der Gewölbedecke.",
        "smell":  "Sie riechen seltsam.",
        "take":   "Ohne den richtigen Zauberspruch geht das nicht.",
    ]));

    add_v_item(([
        "name":   "blitze",
        "gender": "männlich",
        "plural": 1,
        "id":     ({"blitz", "blitze"}),
        "long":   "Viele niegelnagelneue, noch unbenutzte, sehr frische Blitze.",
        "smell":  "Sie riechen neu.",
        "take":   "Ohne den richtigen Zauberspruch geht das nicht.",
    ]));

    set_exits(({ "keller3", "keller4" }),
              ({ "süden",   "norden"  }));
}
