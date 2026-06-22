// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/room/rathaus/obj/adv_book.c
// Description:

#include <move.h>

inherit "/i/object/buch";

int query_no_move()
{
    return 1;
}

string query_no_move_reason()
{
    return "Das Buch ist angekettet!\n";
}

void create()
{
    "*"::create();
    set_short("Ein Buch an einer Kette");
      set_long(
         "Ein altes, abgegriffenes Buch hängt an einer soliden Kette, "
         "die an der Wand befestigt ist. "
	 "Der Titel lautet: \"Wie werde ich Abenteurer?\"");
      add_id(({"abenteurer-buch"}));
      set_page_names(({abs_path("../div/adv_book_page*")}));
      set_page_mode("morefile");
}