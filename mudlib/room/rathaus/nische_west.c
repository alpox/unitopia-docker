// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/room/rathaus/nische_west.c
// Description: Westteil des Rathauses

inherit "/i/room";

void create()
{
   add_type("kunstlicht", 1);
   add_type("kaempfen_verboten", 1);
   add_type("landeplatz","treppe");
   set_own_light(1);
   set_short("In einer Fensternische des Rathauses");
   set_long("Du stehst in einer Fensternische. "
            "Von hier aus hast du einen schönen Blick auf das Postgebäude.");
   set_exits( ({ "gilden5" }), ({ "osten" }) );
   add_v_item(([
      "name": "gebäude",
      "gender": "saechlich",
      "id": ({ "gebäude", "postgebäude" }),
      "long": "Die Tadmorer Hauptpost erfreut sich auch heute wieder "
              "an regem Besucherverkehr."]));
}
