// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/room/rathaus/obj/wiedereinstieg.c
// Description: Einfuehrung fuer rueckkehrende Goetter
// Author:	Myonara

inherit "/i/object/buch";

#include <level.h>
#define PATH "/static/goetter/wiedereinstieg"

void create() {
    ::create();
    set_page_names(({PATH+"/w*"}));
    set_page_mode("page");
    set_verzeichnis(PATH+"/inhalt");
    add_id("wiedereinstieg");
    set_name("buch");
    set_gender("saechlich");
    set_weight(1);
    set_value(0);
    set_no_store(1);
    set_short("Das Wiedereinstiegsbuch für Rueckkehrer");
    set_long("Ein dickes, handgeschriebenes Buch.\n"+
"Das in rotes Leder gebundene Buch trägt auf der Vorderseite ein\n" +
"Unendlichkeitszeichen aus Mithril, eine liegende Acht.\n");
    set_wizard_book(1);
}
