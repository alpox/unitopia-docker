// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/room/rathaus/spielerrat/gang1.c
// Description: Rathaus, Spielerratsraeume

inherit "%room";
inherit "%rauswurf_tuer";

#include <apps.h>
#include <level.h>
#include <message.h>
#include <move.h>

void reset()
{
    "*"::reset();
}

void create()
{
    "*"::create();
    set_short("Gang der Spielerratsräume");
    set_long(
      "Du stehst im Ostflügel des Rathauses in einem breiten Gang, "
      "der in Ost - West - Richtung verläuft. Westlich von hier "
      "mündet der Gang ins Foyer des Rathauses, weit im Osten "
      "führt der Gang auf ein großes Fenster zu. "
      "In den Räumen südlich und nördlich des Ganges befinden sich "
      "die Räumlichkeiten des Spielerrates.");
    add_v_item ((["name":"gang","gender":"maennlich",
      "long":"Ein Gang. Er führt von Osten nach Westen, oder von Westen "
             "nach Osten, oder beides gleichzeitig. Und Du stehst mitten "
             "drin; weiter im Westen führt er ins Foyer des Rathauses, "
             "weiter entfernt im Osten führt er auf ein großes Fenster "
             "zu."
    ]));
    add_v_item((["name":"fenster","gender":"saechlich",
        "long":"Der Gang führt weiter entfernt im Osten auf ein großes "
               "Fenster zu, von hieraus kannst Du allerdings noch nichts "
               "erkennen.",
        "look_msg":"$Der(OBJ_TP) schaut in Richtung Osten zum großen "
               "Fenster hinüber, kann von hier aus allerdings anscheinend "
               "nichts erkennen"
    ]));

    set_exits( ({ abs_path("../foyer"),
            "gang2",
            "raum1",
            "raum2",
            "archivkeller",          }),
            ({ "westen", "osten", "norden", "süden","runter" }) );
            
    set_tuer_richtung("norden");   
    reset();
}

<int|string> filter_sueden(object ob)
{
    if(wizp(ob) || SPIELERRAT->is_spielerrat(ob))
    {
        return 0;
    }
    else
    {
        return wrap("Dieser Raum ist exklusiv für den "
            "Spielerrat UNItopias, Du darfst hier leider nicht hinein.");
    }
}
void moved_in(mapping mv_infos)
{ 
    if(object_name(mv_infos[MOVE_OLD_ROOM])=="/room/rathaus/foyer" 
            && this_player()==mv_infos[MOVE_OBJECT])
        call_out("send_message_to", 1, this_player(),
	    MT_NOTIFY, MA_UNKNOWN,
    	    "Dies ist ein Bereich außerhalb des Spieles.\n");
}
