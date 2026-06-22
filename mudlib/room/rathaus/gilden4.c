// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/room/rathaus/gilden3.c
// Description: Rathaus, Gildenraeume #4

inherit "/i/room";

void create() {
    add_type("kunstlicht", 1);
    add_type("kaempfen_verboten",1);
    add_type("landeplatz","treppe");
    set_own_light(1);
    set_short("Gang der Gilden");
    set_long(
      "In den Räumen links und rechts von diesem Gang präsentieren sich "
      "sämtliche, über Magyra verteilte Gilden. "
      "Nördlich von hier befindet sich der Raum der Sehergilde; "
      "im Süden hörst Du Musik aus dem Raum der Bardengilde.");
    set_exits( ({ "gilden3",
		  "gilden5",
		  "/z/Gilden/Sehergilde/d/Vaniorh/room/praesentation",
		  "/z/Gilden/Bardengilde/d/Vaniorh/praesentation" }),
	       ({ "osten", "westen", "norden", "süden" }) );
}
