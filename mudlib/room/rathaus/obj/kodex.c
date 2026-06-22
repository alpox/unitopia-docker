// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/room/rathaus/obj/kodex.c
// Description: Kodex fuer Goetter
// Author:	Garthan

inherit "/i/object/buch";

#include <level.h>
#define PATH "/static/goetter/kodex"

void create() {
    ::create();
    set_page_names(({PATH+"/kodex*", PATH+"/anhang"}));
    set_page_mode("more");
    set_verzeichnis(PATH+"/inhalt");
    add_id (({"kodex","götterkodex","götter-kodex",
        "verhaltenskodex","verhaltens-kodex"}));
    set_name("Götterkodex");
    set_gender("maennlich");
    set_weight(1);
    set_value(30);
    set_no_store(1);
    set_short ("Der Götterkodex");
    set_long("Ein dickes handgeschriebenes Buch.\n"+
"Das in weißes Leder gebundene Buch trägt auf der Vorderseite ein\n" +
"goldenes Ausrufezeichen mit einem großen Punkt.\n"
"Mitglieder des Obersten Rates hauen es gerne Göttern um die Ohren,\n"
"die sich nicht an den Kodex halten.\n");
    set_hlp_book (1);
    // Engel muessen das lesen koennen, schliesslich unterschreiben
    // sie beim Gott werden, dass sie sich an den Kodex halten werden.
}
