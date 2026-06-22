// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /sys/srwahl.h
// Description: Defines fuer die Wahl des Spielerrats
// Author:      Myonara, 09.05.2026

#ifndef __SRWAHL_H
#define __SRWAHL_H

/*
FUNKTION: SR_WAHLRAUM
DEKLARATION: define SR_WAHLRAUM "/room/wahlen/srwraum"
BESCHREIBUNG:
Der interdimensionale/interdomäne Wahlraum für die Speilerratswahl dient
gleichzeitig als Master für die Plakate und die umgebenden Raeume.
VERWEISE: query_weight, set_min_weight, set_max_weight
GRUPPEN: wahl, spielerrat
*/
#define SR_WAHLRAUM "/room/wahlen/srwraum"
/*
FUNKTION: SR_WAHLPLAKAT_HIER
DEKLARATION: SR_WAHLPLAKAT_HIER 
BESCHREIBUNG:
Wird im Raum-Reset aufgerufen und installiert das Plakat, wenn nötig.
VERWEISE: SR_WAHLRAUM,install_srw_plakat
*/
#define SR_WAHLPLAKAT_HIER SR_WAHLRAUM.install_srw_plakat(this_object())
#define SR_WAHLPLAKAT       "/room/rathaus/obj/plakat_srwahl"
#define ID_SR_WAHLPLAKAT    "sr # wahl # plakat"
#define SRWAHL_URNE         "/room/wahlen/srwurne"
#define SRWAHL_VORWAHL  1
#define SRWAHL_WAHL     2
#define SRWAHL_NACHWAHL 4
#define SRWAHL_ALLES    7
#define SR_KANDIDATENLISTE "/apps/spielerrat_kandidatenliste"

#ifdef Orbit
#define SRWAHL_NEU_IST_AKTIV 1
#endif

#endif // __SRWAHL_H