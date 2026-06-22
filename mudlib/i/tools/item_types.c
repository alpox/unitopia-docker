// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /i/tools/item_types.c
// Description: Hilfsfunktionen für Eigenschaften von Gegenständen
// Created by : Gnomi

#pragma strong_types
#pragma save_types

#include <material.h>

#define BRENNMATERIAL ({ MAT_HOLZ, MAT_PAPIER, MAT_TEXTIL, MAT_KUNSTSTOFF, MAT_OEL, MAT_LEDER })

/*
FUNKTION: query_is_brennbar
DEKLARATION: int query_is_brennbar(object ob)
BESCHREIBUNG:
Liefert 1, wenn ob angezündet werden kann / brennt, ansonsten 0.
Dabei wird vor allem das Material geprüft.
GRUPPEN: feuer
*/
int query_is_brennbar(object ob)
{
    if (ob->query_no_move() || ob->query_invis())
        return 0;
    if (ob->query_streichholz())
        return 1;

    foreach (string brennstoff: BRENNMATERIAL)
        if (ob->material(brennstoff))
            return 1;
    return 0;
}
