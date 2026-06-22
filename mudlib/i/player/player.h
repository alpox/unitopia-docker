// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/i/player/player.h
// Description: Interne Defines fuer das Player-Objekt

#ifndef __I_PLAYER_H__
#define __I_PLAYER_H__

#define CLIENT_NO_EOR		1
#define CLIENT_VT100		2
#define CLIENT_VT100_NUMPAD	4
#define CLIENT_VT100_WIZSHELL	8
#define CLIENT_WEBMUD_NO_EDIT	16

#define TRIGGER_TEXT		0
#define TRIGGER_OPTIONS		1
#define TRIGGER_COLOR		2
#define TRIGGER_REGEXP		3
#define TRIGGER_ANSI		4
#define TRIGGER_ECOLOR		5
#define TRIGGER_BEEP_TYPE	6

#define TRIGGER_SIZE		7

#define TO_BELL			1  // Veraltet, nun CO_BEEP in TRIGGER_COLOR
#define TO_KEEPCOLOR		2
#define TO_CASESENSITIVE	4

// Indizes in das colours-Mapping
// (Achtung: Sind in /i/tools/colours.c noch hardkodiert.)
#define COL_ON_COLOR		0
#define COL_OFF_COLOR		1
#define COL_ON_ECOLOR		2
#define COL_OFF_ECOLOR		3
#define COL_BEEP_TYPE		4

#define COL_SIZE		5

// Indizes in das ansicolors-Mapping
#define ANSI_ON			0
#define ANSI_OFF		1
#define ANSI_INDENT		2
#define ANSI_IDLEBEEP		3 // -1 bei 0s, aber aktiviert
#define ANSI_BEEP_TYPE		4

// Indizes in das EVC_COLOUR-Array
#define EVCC_COLOR		0
#define EVCC_ANSI		1
#define EVCC_ECOLOR		2
#define EVCC_BEEP_TYPE		3

#define EVCC_SIZE		4

#define PF_NO_UEBERFALL_MSG	1

#endif
