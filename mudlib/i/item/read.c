// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/i/item/read.c
// Description:

#pragma save_types
#pragma strong_types

inherit "/i/tools/description";

#include <description.h>
#include <strings.h>

private mixed read;

/*
FUNKTION: set_read
DEKLARATION: void set_read(mixed str)
BESCHREIBUNG:
Mit dieser Funktion setzt man den Text, den man bekommt, wenn man ein Objekt
liest.

Statt eines Strings kann auch eine komplexere Beschreibung mit Bedingungen
angegeben werden. Siehe dazu /doc/funktionsweisen/beschreibungen.

VERWEISE: query_read, query_read_msg, T_LISTE
GRUPPEN: grundlegendes
*/
void set_read(mixed str)
{
    read = compile_desc(str, 0);
    if (stringp(read) || !read)
    {
        this_object()->add_setter_conservation("set_read",({read}) );
    }
    else
    {
        this_object()->add_setter_conservation("set_read", 0);
        this_object()->set_conservation_constraint("set_read",1);
    }
}

/*
FUNKTION: query_read
DEKLARATION: varargs string query_read(string parse_rest, string str,object leser)
BESCHREIBUNG:
Diese Funktion liefert den Text, den leser bekommt, wenn er ein Objekt liest.
PARSE_REST ist dabei der Rest, der vom Spieler eingegeben wurde:
lese Buch 1. Seite -> parse_rest = '1. Seite'
		      str = 'Buch 1. Seite'
Man sollte damit rechnen, dass alle 3 Parameter 0 sind. Dies wuerde einem
query_read("", "", PL) (PL definiert in misc.h) entsprechen.
VERWEISE: set_read, query_read_msg
GRUPPEN: grundlegendes
*/
varargs string query_read(string rest, string str, object betrachter)
{
    mapping info = get_desc_info_mapping(betrachter || this_player());
    string res;

    res = funcall(read, info);
    if(strlen(res) && res[<1]!='\n')
        res = wrap(trim(res, TRIM_RIGHT));

    return res;
}
