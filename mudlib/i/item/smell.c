// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/i/item/smell.c
// Description:

#pragma save_types
#pragma strong_types

inherit "/i/tools/description";

#include <description.h>

private mixed smell;

/*
FUNKTION: set_smell
DEKLARATION: void set_smell(mixed geruch)
BESCHREIBUNG:
Setzt den Geruch eines Objektes.

Statt eines Strings kann auch eine komplexere Beschreibung mit Bedingungen
angegeben werden. Siehe dazu /doc/funktionsweisen/beschreibungen.

VERWEISE: query_smell, T_LISTE
GRUPPEN: grundlegendes
*/
void set_smell(mixed str)
{
    smell = compile_desc(str, 0);
    if (stringp(smell) || !smell)
    {
        this_object()->add_setter_conservation("set_smell",({smell}) );
    }
    else
    {
        this_object()->add_setter_conservation("set_smell", 0);
        this_object()->set_conservation_constraint("set_smell",1);
    }
}

/*
FUNKTION: query_smell
DEKLARATION: string query_smell()
BESCHREIBUNG:
Gibt den Geruch eines Objektes zurueck.
VERWEISE: set_smell
GRUPPEN: grundlegendes
*/
string query_smell()
{
    mapping info = get_desc_info_mapping(this_player());
    string res;

    res = funcall(smell, info);
    if(strlen(res) && res[<1]!='\n')
        res = wrap(trim(res));

    return res;
}
