// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/room/rathaus/spielerrat/gang2.c
// Description: Rathaus, Spielerratsraeume

inherit "%room";
inherit "%rauswurf_tuer";

#include <apps.h>
#include <level.h>

void reset()
{
    "*"::reset();
}

void create()
{
    "*"::create();
    set_short("Gang der Spielerratsräume");
    set_long(
      "Du stehst im Ostflügel des Rathauses am Ende eines breiten Ganges, "
      "der von hier nach Westen führt. Weit westlich von hier mündet "
      "der Gang ins Foyer des Rathauses, östlich von Dir befindet sich "
      "ein großes Fenster. "
      "In den Räumen südlich und nördlich des Ganges befinden sich "
      "die Räumlichkeiten des Spielerrates. "
      "Deutlich fällt Dir ein sehr wichtig aussehendes Dokument "
      "an der östlichen Wand auf.");
    add_v_item ((["name":"gang","gender":"maennlich",
      "long":"Ein Gang. Er führt von hier nach Westen in Richtung Foyer. "
    ]));
    add_v_item (([
	"name":"wand",
	"gender":"weiblich",
	"adjektiv":({"östlich"}),
	"long": "Eine Wand. Einzig auffällig daran ist ein Pergament, "
	        "welches schön eingerahmt an dieser Wand hängt."
    ]));
    add_v_item((["name":"fenster","gender":"saechlich",
        "id":({"fenster","aus"}),
        "long":"Von hier aus hast Du einen herrlichen Blick auf den Dijala, "
               "wie er nordöstlich von hier in östlicher Richtung "
               "dem Meer entgegen durch seine "
               "wunderschönen Uferwiesen fließt. Weiter weg im Osten "
               "siehst Du ihn unter der Nordbrücke verschwinden. "
               "Süd-östlich von hier ragt die große Kathedrale Tadmors "
               "hoch in den Himmel.",
        "look_msg":"$Der(OBJ_TP) schaut überwältigt von der herrlichen "
               "Aussicht aus dem Fenster."
    ]));

    add_v_item(([
               "name"   : "kodex des Spielerrates",
               "id"     : ({"kodex","spielerratskodex","srkodex","pergament",
                            "blatt","papier","pergamentblatt","pergamentpapier",
			    "dokument"}),
               "gender" : "maennlich",
               "long"   : "Eine recht frisch aussehendes Pergamentblatt, "
                          "auf welchem der Kodex des Spielerrates von "
                           MUD_NAME " geschrieben steht.",
               "read"   : (:this_player()->more("/static/adm/SPIELERRATSKODEX");
                            return "";
                          :)
               ]));

    set_exits( ({ "gang1",
		  "raum3",
		  "raum4" }),
	       ({ "westen", "norden", "süden" }) );
	       
    set_tuer_richtung("süden");
    reset();
}

<int|string> filter_norden(object who)
{
    if(wizp(who) || SPIELERRAT->is_spielerrat(who))
        return 0;
    else
    {
        return wrap("Dieser Raum ist exklusiv für den "
            "Spielerrat UNItopias, Du darfst hier leider nicht hinein.");
    }
}
