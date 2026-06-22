// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/room/hlp/obj/gedenktafel.c
// Description:	Gedenktafel fuer die Engels-Wolke
// Author:	Sissi

inherit "/i/item";
inherit "/i/install";

#include <apps.h>

void create ()
{
    ::create ();
    set_name("gedenktafel");
    set_id(({"tafel","gedenktafel"}));
    set_gender("weiblich");
    set_long("Mitten vor Deiner Nase schwebt eine Gedenktafel in der "
        "Luft herum, auf welcher die Untaten der Schutzengel verzeichnet "
        "sind.");
}

varargs string query_read (string pr, string s, object leser)
{
    leser->more (STATISTIK->query_letzte_rettungen(),0,0,0x10);
    return "";
}
