// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/i/item/sounds.c
// Description: Unterstuetzung fuer sounds.

#pragma save_types
#pragma strong_types

#include <properties.h>

/*
FUNKTION: activate_sound_profile
DEKLARATION: int activate_sound_profile(string profile_name)
BESCHREIBUNG:
Die Funktion kann von jedem Objekt ueberlagert werden, um die 
zusammengehoerenden Tondateien auf einen Schlag per
- this_object()->set(P_SOUND_ACTIONS,([...]));
oder
- this_object()->add(P_SOUND_ACTIONS,"action","bereich/datei.wav");
zu setzen. Die Basisfunktion nutzt 0 und "" als Profilenamen,
um auf die Default-Sounds zurueckzusetzen.
VERWEISE: P_SOUND_ACTIONS
GRUPPEN: grundlegendes
*/
int activate_sound_profile(string profile_name)
{
    switch (profile_name)
    {
        case 0: 
        case "": // sounds auf default setzen:
            this_object()->set(P_SOUND_ACTIONS,([]));
            return 1;
        default:
            return 0; // Unbekannt
    }
}