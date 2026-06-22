// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/sys/simul_efuns.h
// Description: Defines fuer verschiedene simul-efuns
// Author:	Gnomi (15.12.2001)

#ifndef SIMUL_EFUNS_H
#define SIMUL_EFUNS_H

// Fuer (|v|short|shortv)timestr
#define TIMESTR_ALL		0
#define TIMESTR_ONLY_TIME	1
#define TIMESTR_ONLY_DATE	2

// Fuer ist()
#define IST_SPACE_BEFORE	1
#define IST_SPACE_AFTER		2

// Fuer player_present
#define PPRESENT_NO_WIZARDS	1
#define PPRESENT_NO_SHIMMER	2
#define PPRESENT_NO_INVIS	4
#define PPRESENT_NO_HIDDEN	8
#define PPRESENT_ONLY_LISTED	16
#define PPRESENT_STATUES	32

// Fuer die Funktionen zu den Ausgaengen
#define DIR_NUR_HIMMELSRICHTUNGEN	1
#define DIR_ALS_DEFAULT			2

// Fuer cond_deep_present
#define CDP_DEPTH_FIRST		1
#define CDP_BREADTH_FIRST	2
#define CDP_FLAT		3

// Die Bits, welche von CDP_DEPTH_FIRST usw. benutzt werden.
#define CDP_TRAVERSE_BITS	3

// Fuer escape_string()
#define ESCAPE_REGEXP           1
#define ESCAPE_PCRE             2
#define ESCAPE_GETDIR           4
#define ESCAPE_ANY              8
#define ESCAPE_WORD             16
#define ESCAPE_CASE             32
#define ESCAPE_NOCASE           64
#define ESCAPE_WILDCARD         128
#define ESCAPE_EXACT            256
#define ESCAPE_REPLACE          512
#define ESCAPE_LISTE            1024

// Fuer search_object()
#define SO_V_ITEM       1
#define SO_DONT_LOAD    2
#define SO_ENV_FIRST    4

#endif // SIMUL_EFUNS_H
