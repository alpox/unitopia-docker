// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/obj/flugraum.c
// Description:	Flugraum fuer HLPs
// Author:	Sissi (1996)
// Modified by:	Freaky (05.08.1998) ::moved_in,::moved_out,::let_not_in
//              Parsec (05.06.00)  L_LUFT

inherit "/i/room";

#include <apps.h>
#include <level.h>
#include <move.h>
#include <room_types.h>
#include <landschaft.h>

object origin, target;

void create ()
{
   set_short ("In der Luft");
   add_type(RT_KEIN_CLEANUP,1);
   // nur noetig, weil der Raum sonst vom ftool zerstoert wird
   // (waehrend ein Spieler drin ist)
   add_type(RT_MAGIE_VERBOTEN,
       "Während des Fliegens ist das zuviel auf einmal.\n");
   add_type(RT_HANDWERK_VERBOTEN,
       "Während des Fliegens ist das zuviel auf einmal.\n");
   add_type(RT_KEIN_KOMPASS, 1);
   add_type(RT_GRABEN_VERBOTEN,1);
   add_type(RT_KEIN_STARTRAUM,1);
   add_type(RT_LANDSCHAFT, L_LUFT);
   set_room_domain("Himmel");
   set_own_light(1);
}

int query_flugraum()
{
    return 1;
}

<int|string> let_not_in(mapping mv_infos)
{
    object was = mv_infos[MOVE_OBJECT];
    if (player_present() && living (was))
    {
        if (wizp (was))
           return "Aus programmtechnischen Gründen "
              "können Götter keine fliegenden Wesen anfliegen.";
        else
           return "Das ist viel zu hoch in der Luft "
              "für Dich.";
    }
    return ::let_not_in(mv_infos);
}

void moved_in(mapping mv_infos)
{
    object was = mv_infos[MOVE_OBJECT];
    ::moved_in(mv_infos);
    if (!living(was) && !was->query_invis())
        call_out("runterfallen",2,was);
    if (playerp(was))
       origin = mv_infos[MOVE_OLD_ROOM];
}

void runterfallen (object was)
{
    object inv;

    if (was && present(was, this_object()))
    {
	mixed contr = CONTROL->concerned("hlpitem_falls", was, this_object());
	if(closurep(contr))
	    funcall(contr, "do_hlpitem_falls", was, this_object());
	else if(contr)
	    contr->do_hlpitem_falls(was, this_object());
    }
        
    if (was && (environment (was) == this_object())) {
        tell_room (this_object(),
           wrap (Der(was) + plural(" segelt"," segeln",was) +
		" durch die Lüfte hinunter."));
        while (inv = first_inventory(was)) {
            inv->remove ();
            if (inv) destruct (inv);
        }
        was->remove ();
        if (was) destruct (was);
    }
}

/*
FUNKTION: concerned_hlpitem_falls
DEKLARATION: int concerned_hlpitem_falls(object item, object flugraum)
BESCHREIBUNG:
Wenn ein Engel gerade einfach so durch den Himmel fliegt, dann
voellig unachtsam einen Gegenstand fallen laesst und dieser sich
dann in Eigenregie auf den Boden zubewegt, dann werden alle mit
CONTROL->add_controller("concerned_hlpitem_falls", ob) angemeldeten
Objekte gefragt, ob sie sich um diesen Gegenstand kuemmern wollen.

Dazu wird dort die Funktion ob->concerned_hlpitem_falls(item, flugraum)
aufgerufen. Dasjenige Objekt, welches die groesste Zahl zurueckliefert,
erhaelt den Zuschlag. Dort wird dann Funktion do_hlpitem_falls aufgerufen.
Diese Funktion muss das Objekt aus dem Flugraum rausnehmen und
eine entsprechende Meldung an den Engel im Flugraum ausgeben.
VERWEISE: do_hlpitem_falls, concerned
GRUPPEN: level
*/

/*
FUNKTION: do_hlpitem_falls
DEKLARATION: void do_hlpitem_falls(object item, object flugraum)
BESCHREIBUNG:
Diese Funktion wird beim Sieger von concerned_hlpitem_falls aufgerufen.

Sie sollte nun selbst das Objekt aus dem Flugraum entfernen und dort eine
entsprechend Meldung ausgeben. Tut sie das nicht, wird das Objekt
anschliessend vom Flugraum mit der Standardmeldung zerstoert.
VERWEISE: concerned_hlpitem_falls, concerned
GRUPPEN: level
*/
void moved_out(mapping mv_infos)
{
    ::moved_out(mv_infos);
    if (playerp(mv_infos[MOVE_OBJECT]))
	call_out(#'remove,0);
}

// Das folgende is dafuer da: wenn ein Spieler waehrend des Fluges
// "ende" macht, dann darf dem Spieler sein environment nicht
// weggezogen werden, es sollte jedoch trotzdem "gleich" verschwinden;
// daher ist hier ein call out noetig.

void remove_flugraum ()
{
    call_out ("remove_flugraum_co",2);
}

void remove_flugraum_co ()
{
    remove ();
}

void set_target(object ziel)
{
  target = ziel;
}

object query_target()
{
  return target;
}

object query_origin()
{
  return origin;
}
