// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/i/object/zeitschrift.c
// Description:

#pragma save_types

#include <deklin.h>

inherit "/i/item";
inherit "/i/move";
inherit "/i/value";

private string page_name;

void set_page_name(string str) { page_name = str; }
string query_page_name() { return page_name; }

void create() {
  set_id(({"zeitschrift", "zeit", "zs", "schrift" }));
  set_name("zeitschrift");
  set_gender("weiblich");
  set_material( ({"papier"}) );
  set_long("Dies ist eine Zeitschrift. Man kann sie lesen.\n");
  seteuid(getuid(this_object()));
  set_page_name("");
}

varargs string query_read(string str, string all, object betrachter) {
    if (!page_name || page_name == "")
	return Der()+" hat nur leere Blätter.\n";

    this_player()->more(page_name,"-- Mehr -- (Hilfe mit '?') ");
    return "";
}
