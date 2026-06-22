// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/room/rathaus/gilden5.c
// Description: Rathaus, Gildenraeume #5

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
      "Nördlich von hier siehst Du einen ziemlich leeren Raum, "
      "im Süden den Raum der alten Beschwörergilde.");
    set_exits( ({ "gilden4",
		  "nische_west",
		  "/z/Gilden/Metamorpher/d/Vaniorh/room/praesentation",
		  "/z/Gilden/OrdenDerFinsternis/d/Vaniorh/Praesentation/room/inforaum" }),
	       ({ "osten", "westen", "norden", "süden" }) );
}
