// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/obj/level_list.c
// Description: Top-Spieler-Liste
// Author:	Freaky (23.12.93)

inherit "/i/value";
inherit "/i/move";
inherit "/i/item";

#include <deklin.h>

#define TOP_LIST_NAME "/var/TOP_PLAYERS"

void create() {
    set_name("Liste der Top-Spieler");
    set_eigen(1);
    set_material( ({"papier"}) );
    set_gender("weiblich");
    set_weight(1);
    set_id(({"liste","top","top spieler","liste der top spieler","top liste"}));
    set_value(5);
    set_no_store(1);
    set_long("Ein viel gelesenes Pergament. Die besten Spieler sind darauf verzeichnet.");
}

varargs string query_read(string str, string all) {
    if (!cat(TOP_LIST_NAME))
	write("Strengt euch an !\n");
    return "";
}
