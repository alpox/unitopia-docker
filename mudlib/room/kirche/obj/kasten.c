// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /room/kirche/obj/kiste.c
// Description: Die Werkzeugkiste von Sakrote
// Author:      

inherit "/i/object/kiste";

#include <move.h>

<int|string> let_not_out(mapping mv_infos)
{
   object sakrote;

   if(sakrote = present("sakrote", environment()))
   {
      tell_room(environment(), 
	     Der(this_player())+" versucht etwas aus dem Kasten zu nehmen.\n",
		({this_player()}));
      sakrote->do_command("sage He, Finger weg von meinem Kasten!");
      return 1;
   }
   return ::let_not_out(mv_infos);
}
<int|string> forbidden_move(mapping mv_infos)
{
    object sakrote;

   if(mv_infos[MOVE_NEW_ROOM] && environment() && 
        (sakrote = present("sakrote", environment())))
   {
      tell_room(environment(),
		Der(this_player())+" versucht den Kasten zu bewegen.\n",
		({ this_player() }));
      
      sakrote->do_command("sage He, Du da, Finger weg von meinem Kasten.\n");
      return 1;
   }
}

void create()
{
    ::create();
    set_max_internal_encumbrance(5);
    set_weight(3);
}
