// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/room/rathaus/gilden2.c
// Description: Rathaus, Gildenraeume #2

inherit "/i/room";

void reset()
{
#  ifdef UNItopia
      if(!present("irrlicht"))
	 touch("/z/Gilden/Magiergilde/npc/irrlicht")->move(this_object());
#  endif
}

void create()
{
    add_type("kunstlicht", 1);
    add_type("kaempfen_verboten", 1);
    add_type("landeplatz","treppe");
    set_own_light(1);
    set_short("Gang der Gilden");
    set_long(
      "In den Räumen links und rechts von diesem Gang präsentieren sich "
      "sämtliche, über Magyra verteilte Gilden. "
      "Südlich befindet sich der Präsentationsraum des Hexenvolks. "
      "Aus der nördlichen Kammer weht Dir feuchte Luft entgegen.");
    set_exits( ({ "gilden1",
		  "gilden3",
		  "/z/Gilden/Vampyrgilde/room/praesentation",
		  "/z/Gilden/Hexenvolk/d/Vaniorh/Rathaus/praesent" }),
	       ({ "osten", "westen", "norden", "süden" }) );
    reset();
}
