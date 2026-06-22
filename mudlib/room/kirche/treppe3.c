// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /room/kirche/treppe3.c
// Description: Die Treppe zum Kirchturm
// Author:

inherit "/i/room";

#include <eyes.h>

string look_borsippa()
{
#ifdef UNItopia
   object room;
   room = touch("/d/Vaniorh/Tadmor/Strassen/room/old_street3");
   return "Du siehst hinunter auf die Borsippastraße:\n"+
      this_player()->describe_room(room, EYE_NO_EXITS | EYE_FORCE_LONG);
#else
   return "Du siehst nichts besonderes.\n";
#endif
}

void create()
{
   set_own_light(1);
   add_type("kunstlicht", 1);
   add_type("landeplatz","balkon");
   set_short("Auf der Treppe im Kirchturm");
   set_long("Die enge Spindeltreppe schraubt sich hier im Gegenuhrzeigersinn "
            "hinauf zum Glockenturm. "
            "Durch eine schiesschartenartige "
            "Oeffnung fällt Licht herein. Durch die Oeffnung sieht man "
            "hinaus auf die Straße nach Borsippa. Man erkennt sogar das "
            "große Westtor Tadmors.");
   add_v_item(([
      "name": "treppe",
      "gender": "weiblich",
      "id": ({ "spindeltreppe", "treppe" }),
      "long": "Die steinerne Treppe wurde von fleißigen Tadmorer Steinmetzen "
              "aus rohen Felsklötzen herrausgemeiselt. Auf der Treppe "
              "können mit Müh und Not zwei dünne Menschlein aneinander "
              "vorbeikommen."]));
   add_v_item(([
      "name": "öffnung",
      "gender": "weiblich",
      "id": ({ "öffnung", "straße" }),
      "look_msg": "$Der() schaut durch die Oeffnung",
      "long": #'look_borsippa]));
   add_v_item(([
      "name": "westtor",
      "gender": "saechlich",
      "id":({"tor", "westtor"}),
      "look_msg": "$Der() schaut durch die Oeffnung",
      "long": "Das große Westtor steht genau an der Stelle, wo die "
              "Borsippastraße Tadmor verlässt und ins gefährliche Ork"
              "land führt."]));
   
   set_exits( ({ "/room/kirche/treppe2", 
                 "/room/kirche/uhr" }),
	      ({ "runter", "hoch" }) );
   set_exit_msg("hoch", "$Der(OBJ_TP) steigt die Treppe hinauf",
			"$Ein(OBJ_TP) kommt von unten hoch gekraxelt");
   set_exit_msg("runter", "$Der(OBJ_TP) steigt die Treppe runter",
			  "$Ein(OBJ_TP) kommt von oben heruntergelaufen");

}
