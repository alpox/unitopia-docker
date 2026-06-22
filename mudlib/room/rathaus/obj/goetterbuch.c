// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/room/rathaus/obj/goetterbuch.c
// Description: Einfuehrung fuer werdende Goetter
// Author:	Garthan

inherit "/i/object/buch";

#include <level.h>
#define PATH "/static/goetter/goetterbuch"

string owner;

void create() {
    ::create();
    set_page_names(({PATH+"/g*"}));
    set_page_mode("page");
    set_verzeichnis(PATH+"/inhalt");
    add_id("götterbuch");
    set_name("buch");
    set_gender("saechlich");
    set_weight(1);
    set_value(0);
    set_no_store(1);
    set_short("Das Götterbuch, eine Einführung für Engel");
    set_long("Ein dickes, handgeschriebenes Buch.\n"+
"Das in rotes Leder gebundene Buch trägt auf der Vorderseite ein\n" +
"goldenes Unendlichkeitszeichen, eine liegende Acht.\n");
    set_hlp_book (1);
}

void init_arg(string str) { owner = str; }
void set_owner(string str) { owner = str; }
string query_owner() { return owner; }

string query_auto_load()
{
   string name;

   name = environment() ? environment()->query_real_name() : 0;
   if(name && name == owner)
   return name && name == owner ? name : 0;
}

void notify_quit(object ob)
{
    if (ob->query_real_name() == owner && ob == environment(this_object()))
    {
	set_no_move(1);
	call_out("set_no_move",0,0);
    }
}
