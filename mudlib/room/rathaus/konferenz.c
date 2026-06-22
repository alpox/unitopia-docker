// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/room/rathaus/konferenz.c
// Description:

inherit "/i/room";

void create() {
    add_type("kunstlicht",1);
    set_own_light(1);
    set_short("Das Konferenzzimmer");
    set_long("Das Konferenzzimmer zu Tadmor.\n");
}

