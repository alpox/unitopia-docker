// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /room/kirche/treppe1.c
// Description: Die Treppe zum Kirchturm
// Author:

#ifdef UNItopia
#include "/d/Vaniorh/sys/vaniorh_exports.h"
#endif

inherit "/i/room";

#include <eyes.h>

string look_kartudingens()
{
#ifdef UNItopia
   object room;
   room = touch(K_PLATZ_1);
   return "Unten auf dem Platz siehst Du:\n"+
      this_player()->describe_room(room, EYE_NO_EXITS | EYE_FORCE_LONG);
#else
   return "Da unten siehst Du nicht viel.\n";
#endif
}

void create()
{
   set_own_light(1);
   add_type("kunstlicht", 1);
   add_type("landeplatz","balkon");
   set_short("Auf der Treppe im Kirchturm");
   set_long("Die enge Spindeltreppe schraubt sich hier im Gegenuhrzeigersinn "
            "hinauf zum Glockenturm. Durch eine schießschartenartige "
            "Öffnung fällt Licht herein. Durch die Öffnung sieht man "
            "hinaus auf den Kartukultininurta-Platz.");
   add_v_item(([
      "name": "treppe",
      "gender": "weiblich",
      "id": ({ "spindeltreppe", "treppe" }),
      "long": "Die steinerne Treppe wurde von fleißigen Tadmorer Steinmetzen "
              "aus rohen Felsklötzen herausgemeißelt. Auf der Treppe "
              "können mit Müh und Not zwei dünne Menschlein aneinander "
              "vorbeikommen."]));
   add_v_item(([
      "name": "öffnung",
      "gender": "weiblich",
      "id": ({ "öffnung", "platz", "kartukultininurta-platz" }),
      "look_msg": "$Der() schaut durch die Öffnung auf den Platz hinunter.",
      "long": #'look_kartudingens]));

   set_exits( ({ "/room/kirche/unten", 
                 "/room/kirche/treppe2" }),
              ({ "runter", "hoch" }) );
   set_exit_msg("hoch", "$Der(OBJ_TP) steigt die Treppe hinauf", 
                        "$Ein(OBJ_TP) kommt von unten hoch gekraxelt");
   set_exit_msg("runter", "$Der(OBJ_TP) steigt die Treppe runter", 
                          "$Ein(OBJ_TP) kommt von oben heruntergelaufen");
}
