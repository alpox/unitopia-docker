// This file is part of UNItopia Mudlib.
// -----------------------------------------------------------------------
//  Datei:  /sys/umfragen.h
//  Autor:  Myonara 28.Sep.2014 
// -----------------------------------------------------------------------
// Beschreibung: Die Include Datei fuer Umfrage-und Ideenkobold (uiko)
// -----------------------------------------------------------------------

#ifndef __UMFRAGEN_H
#define __UMFRAGEN_H

#ifdef Orbit
#define UMFRAGE_MASTER   "/apps/umfragen_db"
#define UMFRAGE_AUTOLOAD "/room/rathaus/obj/uiko"
#define UMFRAGE_DBFILE   "/var/spool/umfragen/umfragen_orbit.db"
#define UMFRAGE_EXT_GIUMF 1
#else
#define UMFRAGE_MASTER   "/apps/umfragen_db"
#define UMFRAGE_AUTOLOAD "/room/rathaus/obj/uiko"
#define UMFRAGE_DBFILE   "/var/spool/umfragen/umfragen.db"
#endif


#define UMFRAGE_S_GELOESCHT  0
#define UMFRAGE_S_NEU        1
#define UMFRAGE_S_STANDARD   2
#define UMFRAGE_S_BEREIT     3
#define UMFRAGE_S_AKTIV      4
#define UMFRAGE_S_OFFEN      5
#define UMFRAGE_S_BEENDET    6
#define UMFRAGE_S_ABGELEHNT  7
#define UMFRAGE_S_ARCHIVIERT 8

#define IDEEN_W_ZEICHEN 0
#define IDEEN_W_I_ID    1
#define IDEEN_W_TEXT    2
#define IDEEN_W_U_ID    3

#define UI_BEITRAG_B_ID    0
#define UI_BEITRAG_WANN    1
#define UI_BEITRAG_WER     2
#define UI_BEITRAG_TITEL   3

#define UI_MENUTYPE_WAHL        1
#define UI_MENUTYPE_OWN         2
#define UI_MENUTYPE_BEREIT      3
#define UI_MENUTYPE_ALLE        4
#define UI_MENUTYPE_OFFEN       5
#define UI_MENUTYPE_DISKU       6
#define UI_MENUTYPE_NDISKU      7
#define UI_MENUTYPE_IGNORIERT   8

#define UI_OPT_COUNTALL   "ui:opt:countall"
#define UI_OPT_PANTHEON   "ui:opt:pantheon"
#define UI_OPT_SUCHFILTER "ui:opt:suchfilter"
#define UI_OPT_ERSTELLER  "ui:opt:ersteller"
#define UI_OPT_I_ID       "ui:opt:i_id"
#define UI_OPT_U_ID       "ui:opt:u_id"
#define UI_OPT_D_ID       "ui:opt:d_id"
#define UI_OPT_B_ID       "ui:opt:b_id"
#define UI_OPT_XU_ID      "ui:opt:xu_id"
#define UI_OPT_XT_ID      "ui:opt:xt_id"
#define UI_OPT_XF_ID      "ui:opt:xf_id"
#define UI_OPT_XA_ID      "ui:opt:xa_id"
#define UI_OPT_XW_ID      "ui:opt:xw_id"
#define UI_OPT_SEQUENCE   "ui:opt:sequence"
#define UI_OPT_UNIFIED_ID   "ui:opt:unified:id"
#define UI_OPT_UNIFIED_PATH "ui:opt:unified:path"
#define UI_OPT_TITLE        "ui:opt:title"
#define UI_OPT_DESCRIPTION  "ui:opt:description"
#define UI_OPT_USER_TYPE  "ui:opt:user:type"
#define UI_OPT_FRAGE      "ui:opt:frage"
#define UI_OPT_ANTWORT    "ui:opt:antwort"
#define UI_OPT_ANZ_ANTWORTEN    "ui:opt:anzahl:antworten"
#define UI_OPT_FRAGEN_TYP "ui:opt:fragen:typ"
#define UI_OPT_ITEM       "ui:opt:item"
#define UI_OPT_OFFSET     "ui:opt:offset"
#define UI_OPT_LIMIT      "ui:opt:limit"
#define UI_OPT_DATA       "ui:opt:data"
#define UI_OPT_GOTO       "ui:opt:goto"
#define UI_OPT_STATUS     "ui:opt:status"
#define UI_OPT_MC         "ui:opt:multiplechoice"
#define UI_OPT_MENUTYPE   "ui:opt:menutype"
#define UI_OPT_UMFRAGE_OFFEN  "ui:opt:umfrage:offen"
#define UI_OPT_DISKUSSION "ui:opt:diskussion"
#define UI_OPT_OHNE_DISKUSSION "ui:opt:ohne:diskussion"
#define UI_OPT_IGNORELIST "ui:opt:diskussion"
#define UI_OPT_CACHE      "ui:opt:cache"
#define UI_OPT_LOOP_MODE  "ui:opt:loop:mode"
#define UI_OPT_USER_NAME  "ui:opt:user:name"
#define UI_OPT_GILDE      "ui:opt:gilde"
#define UI_OPT_MAPPING    "ui:opt:mapping"

#define UI_LOOP_CHOOSE      0x0001
#define UI_LOOP_FROM_START  0x0002
#define UI_LOOP_RESTART     0x0004
#define UI_LOOP_PRINT       0x0100

#define UI_MSG_N_ID       "ui:msg:n_id"
#define UI_MSG_VON        "ui:msg:von"
#define UI_MSG_AN         "ui:msg:an"
#define UI_MSG_WANN       "ui:msg:wann"
#define UI_MSG_WAS        "ui:msg:was"
#define UI_MSG_ID         "ui:msg:id"
#define UI_MSG_STATUS     "ui:msg:status"
#define UI_MSG_KATEGORIE  "ui:msg:kategorie"
#define UI_MSG_NACHRICHT  "ui:msg:nachricht"

#define UI_ERHEBUNG1I_SCHLUESSEL  0
#define UI_ERHEBUNG1I_U_ID        1
#define UI_ERHEBUNG1I_I_ID        2
#define UI_ERHEBUNG1I_TEXT        3

#define UI_USER_TYPE_ADMIN  0x0001
#define UI_USER_TYPE_WIZARD 0x0002
#define UI_USER_TYPE_HLP    0x0004
#define UI_USER_TYPE_PLAYER 0x0008
#define UI_USER_TYPE_NEWBIE 0x0010
#define UI_USER_TYPE_SR     0x0020
#define UI_USER_TYPE_ALL    0x003F

#define UI_FRAGEN_TYP_FREI          0
#define UI_FRAGEN_TYP_FREI_ERWEIT   1
#define UI_FRAGEN_TYP_ZEIT3_3_10    2
#define UI_FRAGEN_TYP_SCHLECHT5GUT  3
#define UI_FRAGEN_TYP_GILDE         4
#define UI_FRAGEN_TYP_KOMMENTAR     5

#define UI_FRAGEN_TYP_LISTE ({ \
    "Frei","Frei/Erweiterbar","Zeit3_3_10","Schlecht5Gut", \
    "Gilde", "Freier Kommentar" })

#endif
// EOF
