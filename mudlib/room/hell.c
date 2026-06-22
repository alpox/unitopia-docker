// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/room/hell.c
// Description:
//

inherit "/i/room";

#include <config.h>

int query_prevent_shadow(object shadow) { return 1; }

void create() {
    set_short("In der Hölle");
    set_long("Du bist in der Hölle.\n");
    set_exit(DEFAULT_NIRVANA_EXIT, "hoch");
    set_own_light(1);
    add_type("kunstlicht", 1);
    add_type("schiff_erlaubt", 1);
    add_type("startraum", DEFAULT_NIRVANA_EXIT);
    add_type("teleport_rein_verboten", 1);
}
