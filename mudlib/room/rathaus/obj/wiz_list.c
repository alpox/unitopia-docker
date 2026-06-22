// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/room/rathaus/obj/disk_list.c
// Description:

inherit "/i/item";
inherit "/i/value";
inherit "/i/move";

#include <deklin.h>
#define TOP_LIST_NAME "/var/DISK_USAGE"

void create() {
    set_name("'Goetter-Top-20'");
    set_short("Die 'Goetter-Top-20' - Liste");
    set_material( ({"papier"}) );
    set_gender("weiblich");
    set_weight(1);
    set_id(({"liste","top-20","götter-top-20"}));
    set_value(5);
    set_no_store(1);
    seteuid(getuid());
}

varargs string query_read(string str, string all)
{
    cat(TOP_LIST_NAME);
    return "";
}

string query_long(object who) { return read_file(TOP_LIST_NAME); }
