// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /room/bsp/bsp_sound1.c
// Description: Einfuehrungskurs - fuer Sounds Teil 1
// Author:      Unbekannt.

inherit "/i/room";

#include <properties.h>
#include <message.h>
#include <misc.h>

void reset() 
{
    object tuer;
  if (!present("tür # westen"))
  {
    tuer=clone_object("/obj/tuer");
    tuer.init_door("bsp_sound2","westen");
    tuer.set_keys(0);
    tuer.add_id( ({ "tür # westen"}) );
    tuer.set_long("Tür mit anderem Klopfen..");
    tuer.set_door_height(-1); 
    // Defaultsound ist "Basis/tuer_anklopfen.wav"
    // wir setzen ihn nur nochmal.
    // P_SOUND_ACTIONS ist fuer durchfuehrbare Aktionen an bestimmten Objekten
    // siehe ? P_SOUND_ACTIONS
    tuer.add(P_SOUND_ACTIONS,"anklopfen","Basis/tuer_anklopfen.wav");
    // beim Nutzen von P_SOUND_ACTIONS in neuen Standardobjekten kann als
    // attributes-Parameter zu den send_message-Varianten 
    // ACTION_SOUND_TO("anklopfen", "Basis/tuer_anklopfen.wav")
    // verwendet werden. (Hier ohne Beispiel, siehe /i/object/tuer.c)
    // siehe ? ACTION_SOUND_TO, ? ACTION_SOUND_OB und ? ACTION_SOUND_ITEM
    tuer.move(this_object());
  }
}

int bimmel(string str) {
  // alle send_message-Varianten haben einen zusaetzlichen Mapping Parameter
  // fuer Attribute, hier das Define MSG_SOUND aus /sys/message.haben
  // Damit wird bei allen Empfaengern des message der Sound abgespielt,
  // sofern MXP oder GMCP das beim Mudclient annimmt.
  send_message(MT_LOOK,MA_USE,
    wrap(Der(TP)+" drückt auf die Klingel und es läutet."),
    wrap("Du drückst auf die Klingel und es läutet."),TP,
        ([MSG_SOUND:"Druidengilde/Ramas_klingeln.wav"]));
  return 1;
}

void init()
{
  add_action("bimmel", "drück", 1);
  add_action("bimmel", "bimmel", 1);
  add_action("bimmel", "laeut", 1);
}

void create() 
{
   set_short("Beispielraum Tondateien Nr. 1");
   set_long("Du bist im Beispielraum Nr. 1 des Kurses über Tondateien für "
      "Götter. Mit dem Zauberstab-Kommando 'zmore hier' kannst Du den "
      "Quellcode dieses Raumes anschauen. Klopfen an der Tür und "
      "bimmel erzeugen Töne.");
   // das Folgende setzt fuer das lauschen einen Klang, den der Hoerende 
   // ausgeliefert bekommt, hier ein Klopfen.
   set_noise("Du hörst ein Klopfen."); // muss gesetzt sein, sonst kein Sound.
   add(P_SOUND_ACTIONS,"lauschen","Basis/tuer_anklopfen.wav");
   set_exits( ({"bsp_eingang"}), 
            ({"osten"}) );
   reset();
}
