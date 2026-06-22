// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /room/kirche/treppe5.c
// Description: Die Treppe zum Kirchturm
// Author:

inherit "/i/room";

#include <eyes.h>

string look_bridge()
{
#ifdef UNItopia
   object room;
   room = touch("/d/Vaniorh/Tadmor/Strassen/room/new_bridge");
   return "Du siehst hinunter auf die neue Brücke:\n"+
      this_player()->describe_room(room, EYE_NO_EXITS | EYE_FORCE_LONG);
#else
   return "Dort unten siehst Du nicht viel.\n";
#endif
}

void create()
{
   set_own_light(1);
   add_type("kunstlicht", 1);
   add_type("landeplatz","balkon");
   set_short("Auf der Treppe im Kirchturm");
   set_long("Die klapprige Stiege schraubt sich hier im Gegenuhrzeigersinn "
            "hinauf zur Verdombden, die Du schon von hier aus sehen kannst. "
            "Durch eine schiesschartenartige "
            "Oeffnung fällt Licht herein. Durch die Oeffnung sieht man "
            "hinaus auf die neue Brücke über den Dijala.");
   add_v_item(([
      "name": "öffnung",
      "gender": "weiblich",
      "id": ({ "öffnung", "brücke" }),
      "look_msg": "$Der() schaut durch die Oeffnung",
      "long": #'look_bridge]));
   add_v_item(([
      "name": "verdombde",
      "gender": "weiblich",
      "id":({"verdombde", "glocke", "totenglocke"}),
      "long": "Die Verdombde ist die größte aller Glocken, die jemals "
	      "in einer Glockengießerei in Magyra gegossen wurde. Von hier "
	      "aus siehst Du nur, dass sie in der Tat sehr groß ist. "]));
   set_exits( ({ "/room/kirche/treppe4", 
                 "/room/kirche/verdombde" }),
	      ({ "runter", "hoch" }) );
   set_exit_msg("hoch", "$Der(OBJ_TP) klettert die Stiege hinauf",
			"$Ein(OBJ_TP) kommt von unten hoch gekraxelt");
   set_exit_msg("runter", "$Der(OBJ_TP) klettert die Stiege runter",
			  "$Ein(OBJ_TP) kommt von oben heruntergelaufen");
}
