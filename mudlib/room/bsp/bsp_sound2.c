// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /room/bsp/bsp_sound2.c
// Description: Einfuehrungskurs - fuer Sounds Teil 2
// Author:      Unbekannt.

inherit "/i/room";

#include <properties.h>

void reset() 
{
    object tuer;
  if (!present("tür # osten"))
  {
    tuer=clone_object("/obj/tuer");
    tuer.init_door("bsp_sound1","osten");
    tuer.set_keys(0);
    tuer.add_id( ({ "tür # osten" }) );
    tuer.set_long("Tür mit anderem Klopfen..");
    tuer.set_door_height(-1); 
    // ? unter activate_sound_profile findet man die aktiven Profile
    // hier z.B. fuer die Tuer: holztuer oder metalltuer.
    tuer.activate_sound_profile("metalltür");
    tuer.move(this_object());
  }
}

void create() 
{
   set_short("Beispielraum Tondateien Nr. 2");
   set_long("Du bist im Beispielraum Nr. 2 des Kurses über Tondateien für "
      "Götter. Mit dem Zauberstab-Kommando 'zmore hier' kannst Du den "
      "Quellcode dieses Raumes anschauen.");
   set_exits( ({"bsp_eingang"}), 
            ({"osten"}) );
   reset();
}
