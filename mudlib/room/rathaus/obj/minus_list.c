// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/room/rathaus/obj/minus_list.c
// Description:

inherit "/i/item";
inherit "/i/value";
inherit "/i/move";

#include <more.h>

#include <deklin.h>
#define TOP_LIST_NAME "/var/TOP_MINUS"

void create()
{
   set_short("Die 'ICH SOLL NICHT TELEPORTIEREN!' Liste");
   set_eigen(1);
   set_name("liste");
   set_material("papier");
   set_gender("weiblich");
   set_weight(1);
   set_id(({"liste","minus-15","minus"}));
   set_value(5);
   set_no_store(1);
   seteuid(getuid());
}

varargs string query_read(string str, string all)
{
   this_player()->more(TOP_LIST_NAME,0,0,M_AUTO_END);
   return "";
}

string query_long(object who) { return read_file(TOP_LIST_NAME); }
