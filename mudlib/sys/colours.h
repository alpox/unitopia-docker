// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/sys/colours.h
// Description:	Die Definitionen fuer die Einfaerbung bei den Spielern
// Author:	Freaky (30.03.2000) uebernommen aus /i/player/colours.c

#ifndef COLOURS_H
#define COLOURS_H 1

#include <term.h>

#define ACT_FIGHT   MA_FIGHT
#define ACT_MOVE    MA_MOVE
#define ACT_EMOTE   MA_EMOTE
#define ACT_COMM    MA_COMM
#define ACT_NOTIFY      99
#define ACT_CONFIG      98
#define ACT_FARCOMM     97
#define ACT_DEBUG       96
#define ACT_FAILMSG     95  // Fehlschlaege
#define ACT_EVENT   0x1000      // Kanal ist dann (ACT_EVENT & kanal)
#define ACT_NAMES     (["kampf":	     ACT_FIGHT, \
			"seele":	     ACT_EMOTE, \
			"bewegung":	     ACT_MOVE, \
			"kommunikation":     ACT_COMM, \
			"meldungen":	     ACT_NOTIFY, \
			"fehlschlaege":	     ACT_FAILMSG, \
			"fernkommunikation": ACT_FARCOMM ])
#define ACT_WIZ_NAMES (["debug":	     ACT_DEBUG])

// Bitfeld fuer ACT_CONFIG
// Bit        Bedeutung
//  1         Farben sind ausgeschaltet
//  2         Sounds im WebMUD ausgeschaltet
#define CO_CONFIG_OFF	1
#define CO_CONFIG_MUTE	2

// Das Bitfeld ist wie folgt aufgebaut:
//  Bit       Bedeutung
//   1 -  3   Einruecktiefe
//   4        Bold
//   5        Low
//   6        --
//   7        Underline
//   8        Blink
//   9        --
//  10        Revers
//  11        --
//  12 - 14   Vordergrundsfarbe
//  15        Vordergrundsfarbe gesetzt
//  16 - 18   Hintergrundsfarbe
//  19        Hintergrundsfarbe gesetzt
//  20        Piepton
//  21 - 32   --

#define CO_BOLD		0x000010
#define CO_LOW		0x000020
#define CO_UNDERLINE	0x000080
#define CO_BLINK	0x000100
#define CO_REVERS	0x000400
#define CO_FG_SET	0x008000
#define CO_BG_SET	0x080000
#define CO_BEEP		0x100000

#define CO_INDENT_MASK  7
#define CO_COLOUR_MASK  7
#define CO_SET		8		// Ist die Farbe ueberhaupt gesetzt
#define CO_OFFSET_MOD	4		// Von wo bis wo gehen die Modifier
#define CO_END_MOD      11		//   wie fett, ..
#define CO_OFFSET_FG	12		// Offset der Vordergrundsfarbe
#define CO_OFFSET_BG	16		// Offset der Hintergrundsfarbe

#define CO_TO_BG(x)	(((x) >> CO_OFFSET_BG) & CO_COLOUR_MASK)
#define CO_TO_FG(x)	(((x) >> CO_OFFSET_FG) & CO_COLOUR_MASK)
#define CO_TO_INDENT(x)	((x) & CO_INDENT_MASK)

// Erweitertes Bitfeld:
//  Bit		Bedeutung
//  1-8		256-Bit-Vordergrundfarbe
//  9-16	256-Bit-Hintergrundfarben
//  17		256-Bit-Vordergrundfarbe gesetzt
//  18		256-Bit-Hintergrundfarbe gesetzt
//  19-24	Piepton nur nach Idlezeit

#define COE_FG256_MASK		0x000000FF
#define COE_BG256_MASK		0x0000FF00
#define COE_FG256_SET		0x00010000
#define COE_BG256_SET		0x00020000
#define COE_IDLEBEEP_MASK	0x00FC0000

#define COE_OFFSET_FG256	0
#define COE_OFFSET_BG256	8
#define COE_OFFSET_IDLEBEEP	18

#define COE_TO_FG256(x)		((x) & COE_FG256_MASK)
#define COE_TO_BG256(x)		(((x) & COE_BG256_MASK) >> COE_OFFSET_BG256)
#define COE_TO_IDLEBEEP(x)	(((x) & COE_IDLEBEEP_MASK) >> COE_OFFSET_IDLEBEEP)

// Die Namen der Farben und die Ansi-Farbnummer
#define CO_COL_NAMES  (["schwarz":	VT_BLACK, \
			"rot":		VT_RED, \
			"grün":		VT_GREEN, \
			"gelb":		VT_YELLOW, \
			"blau":		VT_BLUE, \
			"magenta":	VT_MAGENTA, \
			"cyan":		VT_CYAN, \
			"weiß":		VT_WHITE])
#define CO_COL_NAMES_ASCII    (["schwarz":	VT_BLACK, \
				"rot":		VT_RED, \
				"gruen":	VT_GREEN, \
				"gelb":		VT_YELLOW, \
				"blau":		VT_BLUE, \
				"magenta":	VT_MAGENTA, \
				"cyan":		VT_CYAN, \
				"weiss":	VT_WHITE])

// Die Namen der Hervorhebungen und die Ansi-Nummer
#define CO_NAMES      (["fett":		CO_BOLD, \
			"dunkel":	CO_LOW, \
			"unterstrichen":CO_UNDERLINE, \
			"blinkend":	CO_BLINK, \
			"invers":	CO_REVERS])

#endif // COLOURS_H
