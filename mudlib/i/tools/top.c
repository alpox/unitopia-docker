// This file is part of UNItopia Mudlib.
// ---------------------------------------------------------------------
// File:	/i/tools/top.c
// Description:	Liefert Informationen zur Aktivität von UIDs
// Author:	Gnomi

#pragma strong_types

struct top_uid_info
{
    string uid;
    int evals;
    int data;
    int callouts;
};

/*
FUNKTION: get_top_info
DEKLARATION: struct top_uid_info* get_top_info()
BESCHREIBUNG:
Liefert Statistiken über die vorhandenen UIDs. Das Ergebnis ist ein Array
aus Strukturen mit folgenden Elementen:
    string uid          Die UID
    int evals           Verbrauchte Evals dieser seit MUD-Start
    int data            Summe aller Datenstrukturen (gemessen in Anzahl an
                        Elementen in jeder Datenstruktur) dieser UID.
    int callouts        Anzahl an laufenden call_outs.

Achtung: Bitte das Array und seine Elemente nicht verändern, sondern ggf.
         erst eine Kopie davon machen.

GRUPPEN: Statistik
*/
struct top_uid_info* get_top_info()
{
    return "/apps/top".get_top_info();
}
