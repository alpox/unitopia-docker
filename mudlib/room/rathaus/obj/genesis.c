// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/room/rathaus/obj/exodus.c
// Description: Einfuehrung fuer Lehrlinge
// Author:	Garthan

inherit "/i/object/buch";

#include <level.h>
#define PATH "/static/goetter/genesis"

void create() {
    ::create();
    set_page_names(({PATH"/kodex",
                     PATH"/einfuehrung",
		     PATH"/commands",
		     PATH"/zauberstab",
		     PATH"/filesystem1",
		     PATH"/filesystem2",
		     PATH"/pantheon1",
		     PATH"/pantheon2",
		     PATH"/notabene"}));
    set_page_mode("page");
    set_verzeichnis(PATH+"/inhalt");
    add_id("genesis");
    set_name("buch");
    set_gender("saechlich");
    set_weight(1);
    set_value(30);
    set_no_store(1);
    set_short("Das Buch Genesis, eine Einführung für Lehrlinge");
    set_long("Ein dickes handgeschriebenes Buch.\n"+
"Das in hellblaues Leder gebundene Buch trägt auf der Vorderseite ein\n" +
"goldenes Unendlichkeitszeichen, eine liegende Acht.\n");
    set_wizard_book (1);
}
