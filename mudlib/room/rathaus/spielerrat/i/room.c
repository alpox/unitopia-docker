// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/room/rathaus/spielerrat/i/room.c
// Description:	Inherit fuer die Spielerratsraeume
// Author:	Gnomi

inherit "%room";

#include <invis.h>
#include <message.h>
#include <move.h>

#define AUSGANG "/room/rathaus/foyer"

void create()
{
    add_type("kunstlicht",1);
    add_own_light(1);
    add_controller("forbidden_move_in",
	(:
        object who = $2[MOVE_OBJECT];
	    if(playerp(who) && who->query_invis())
	    {
            return wrap("Eine göttliche Macht hindert "
                "dich daran die Spielerratsräume unsichtbar zu betreten.");
	    }
	:));
    add_controller("notify_invis",
	(:
	    if(($4!=V_VIS) && playerp($2) && IS_HIDDEN($2))
	    {
		$2->set_invis(V_VIS);
		if($2->query_invis())
		{
		    $2->send_message_to($2, MT_NOTIFY|MT_SENSE, MA_UNKNOWN,
			wrap("Eine göttliche Macht duldet keine unsichtbaren "
			"Personen in diesen Hallen und wirft Dich raus."));
		    $2->move(AUSGANG,MOVE_MAGIC);
		}
		else
		    $2->send_message_to($2, MT_NOTIFY|MT_SENSE, MA_UNKNOWN,
			wrap("Eine göttliche Macht duldet keine unsichtbaren "
			"Personen und macht Dich wieder sichtbar."));
	    }
	:));
    // Spielerrat ist eher OOC und sollte daher entsprechend
    // geschuetzte Raeumlichkeiten haben.
    add_type("teleport_rein_verboten", 1);
    add_type("landeplatz", abs_path("../treppe"));
    add_type("kaempfen_verboten", 1);
    add_type("sperrgebiet", 1);
    add_type("keine_magie", 1);
    add_type("kein_handwerk", 1);
    add_type("kein_verbrauch", 1);
}
