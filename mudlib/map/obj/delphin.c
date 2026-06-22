// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /map/obj/delphin.c
// Description: Ein verspielter Delphin fuer die Ozean-Map
// Author:      Pif (20.08.97)
// Modified by: Pif (5.4.98) Erbt jetzt das Stranden-Inherit und
//                           wurde mit entspechenden Meldungen
//                           und Beschreibungen versorgt.

inherit "/map/i/stranden";

#include <stats.h>
#include "d_seele.h"

string query_agonie()
{
  return Der()+" windet sich in Qualen.";
}
string query_sterben()
{
  return "Der Körper "+des()+" bäumt sich noch ein letztes Mal auf.";
}

string query_leiche_kurz()
{
  return "Die Leiche eines Delphines";
}

string query_leiche_lang()
{
  return "Die Leiche eines Delphines, den jemand auf das Land geschafft "
         "haben muss. Seine Haut ist etwas vertrocknet.";
}

void create()
{
  ::create();
  initialize("delphin");
#ifndef NEW_STATS
  give_hands(1);
  give_hp(50);
  give_weapon_level(20);
  give_armour_level(1);
  set_one_stat(STAT_STR,50);
  set_one_stat(STAT_INT,80);
  set_one_stat(STAT_CON,60);
  set_one_stat(STAT_DEX,90);
#endif
  set_align(100);
  set_gender("maennlich");
  set_id(({"wal","delphin","delphinus","zahnwal","delfin", "map fish"}));
  set_smell("Der Delphin riecht etwas fischig.");
  set_feel("Die nasse Delphinhaut fühlt sich an, als "
           "bestünde sie aus besonders glattem Gummi.");
  set_long("Der Delphin besitzt einen schlanken Körper, der ihn "
           "befähigt, flink und behende durchs Wasser zu schießen. "
           "Trotz seiner schnellen Schwimmweise taucht er von Zeit "
           "zu Zeit auf, so dass man einen genaueren Blick auf ihn "
           "werfen kann.");

  add_random_activities(([
     "!echo $Der() schwimmt blitzschnell um Dich herum." : 0,
     "!echo $Der() taucht mit dem Vorderkörper aus dem Wasser "
     "auf, bleibt auf der Stelle stehen, schnattert eine Weile "
     "und lässt sich schließlich wieder ins Wasser sinken." : 0,
     "!echo $Der() schießt direkt neben Dir aus dem Wasser "
     "und verschwindet gleich darauf wieder im Meer." : 0,
     "!echo $Der() macht einen großen Sprung und taucht mit dem "
     "Kopf voran wieder ins Wasser." : 0,
     "!echo $Der() steckt den Kopf aus dem Wasser und betrachtet "
     "Dich eine Weile aufmerksam, bevor er wieder wegtaucht." : 0,
     "!echo Während er Dich langsam umkreist, lässt "
     "$der() Dich nicht aus den Augen." : 0
     ]));
  set_activity(8);
#include "d_detail.h"
  add_controller(({"notify_seele","forbidden_seele"}), this_object());
}
