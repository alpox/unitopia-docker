// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /sys/soul.h
// Description: Defines fuer die Seele
// Created by : Monty 11.11.94
// Modifyed by:
//   Parsec  02.10.98   - neu: IGNORE_FAR  (wenn's egal ist ob ein gefundenes
//                        V-Item weit weg ist.
//                      - MIES, NEUTRAL, NETT als Bewertungen von Seeleaktionen

#ifndef SOUL_H
#define SOUL_H 1

#define PARTNER 	1
#define LIVING 		2
#define OBJECT 		4
#define IGNORE_FAR      8
#define NOT_TO_ENV	16

#define SOUL_PO		PARTNER | OBJECT
#define SOUL_PL		PARTNER | LIVING

// Fuer Align von Seele-Befehlen
#define MIES            -1
#define NEUTRAL         0
#define NETT            1

// Fuer zusaetzliche Flags von Seele-Befehlen
#define PSEUDO_MOVE     1

// Befehle fuer /i/living/voice->do_change_comm(...)
#define VOICE_FLUESTERMODUS 0x00100
#define VOICE_TUSCHELMODUS  0x00200
#define VOICE_LALLEN        0x00400
#define VOICE_LISPELN       0x00800
#define VOICE_STOTTERN      0x01000
#define VOICE_SUMMEN        0x02000
#define VOICE_NUSCHELN      0x04000
#define VOICE_HISSEN        0x08000
#define VOICE_ZISCHEN       0x10000

#endif // SOUL_H
