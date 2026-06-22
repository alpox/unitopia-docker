// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/i/item/noise.c
// Description:

#pragma save_types
#pragma strong_types

inherit "/i/tools/description";

#include <description.h>

private mixed noise;

/*
FUNKTION: set_noise
DEKLARATION: void set_noise(mixed geraeusch)
BESCHREIBUNG:
Setzt das Geraeusch, das ein Objekt erzeugt.
Wenn der String nicht von einem "\n" abgeschlossen wird, wird automatisch
auf 75 Zeichen Breite umgebrochen.

Statt eines Strings kann auch eine komplexere Beschreibung mit Bedingungen
angegeben werden. Siehe dazu /doc/funktionsweisen/beschreibungen.

VERWEISE: query_noise, T_LISTE
GRUPPEN: grundlegendes
*/
void set_noise(mixed str)
{
    noise = compile_desc(str, 0);
    if (stringp(noise) || !noise)
    {
        this_object()->add_setter_conservation("set_noise",({noise}) );
    }
    else
    {
        this_object()->add_setter_conservation("set_noise", 0);
        this_object()->set_conservation_constraint("set_noise",1);
    }
}

/*
FUNKTION: query_noise
DEKLARATION: string query_noise()
BESCHREIBUNG:
Gibt das Geraeusch, das ein Objekt erzeugt zurueck.
VERWEISE: set_noise, query_hear_msg
GRUPPEN: grundlegendes
*/
string query_noise()
{
    mapping info = get_desc_info_mapping(this_player());
    string res;

    res = funcall(noise, info);
    if(strlen(res) && res[<1]!='\n')
        res = wrap(trim(res));

    return res;
}
