// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:    /room/rathaus/gilden1.c
// Description: Rathaus, Gildenraeume #1

inherit "/i/room";

#include <room_types.h>
#include "/z/Gilden/Druidengilde/sys/export.h"

void reset()
{
#  ifdef UNItopia
      if(!present("siegmund"))
     touch("/z/Gilden/Ritterorden/npc/siegmund")->move(this_object());
#  endif
}

void create()
{
    add_type(RT_KUNSTLICHT,1);
    add_type(RT_KAEMPFEN_VERBOTEN,1);
    add_type(RT_FLUG_LANDEPLATZ,"treppe");
    set_own_light(1);
    set_short("Gang der Gilden");
    set_long(
      "In den Räumen links und rechts von diesem Gang präsentieren sich "
      "sämtliche, über Magyra verteilte Gilden. "
      "Nördlich von hier befindet sich der Raum der Abenteurergilde, aber die "
      "Türe ist seltsamerweise verschlossen. Und südlich ist der Raum"
      " der Druidengilde.");
    add_v_item( ([
        "name": "tür",
        "id"  : ({ "tür","türe" }),
        "gender":"weiblich",
        "long":"Ein Zettel hängt an der geschlossenen Tür:\n"
              "\n"
       "             Kommt uns doch in unserem Gilden-Haus besuchen.\n"
       "          Es befindet sich im Westen des Kartukultininurta-Platzes.\n"
        ]) );
        set_exits( ({ "foyer", "gilden2", DRUIDEN_RAUM_PRAESENTATION }),
           ({ "osten", "westen", "süden" }) );
    reset();
}

