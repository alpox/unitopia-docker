// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/sys/eyes.h
// Description: Includefile fuer Konstanten fuer die Augen.
// Author:	Garthan (04.01.94)

#ifndef EYES_H
#define EYES_H 1

// Defines fuer query_room_description
#define EYE_FORCE_LONG		0x001
#define EYE_NO_EXITS		0x002
#define EYE_NO_CONTENTS		0x004
#define EYE_NO_DESCRIPTION	0x008
#define EYE_DARK	        0x010
#define EYE_FORCE_DARK		0x010
#define EYE_FORCE_LIGHT     0x020
#define EYE_SHOW_ME		    0x040
#define EYE_NO_EXIT_VIEW	0x080
#define EYE_FORCE_SHORT     0x100

// Augenoptionen:
#define EYE_DIRINFO		"dirinfo"
#define EYE_FILE		"file"
#define EYE_KURZ		"kurz"
#define EYE_HIDDEN_EXITS	"hidden"
#define EYE_NOLIST_EXITS	"nolist"
#define EYE_LOCKED_EXITS	"locked"
#define EYE_ROOM_INVIS		"roominv"
#define EYE_MY_INVIS		"myinv"
#define EYE_OTHER_INVIS		"otherinv"
#define EYE_INVSTYLE		"invstyle"
#define EYE_V_ITEMS		"v_items"
#define EYE_SHADOWS		"shadows"
#define EYE_DEBUG		"debug"
#define EYE_ROOM_ORDER		"roomorder"

// Defines fuer query_contents
#define CONTENTS_SHOW_INVIS	0x01
#define CONTENTS_SHOW_CONTENTS	0x02

// Defines fuer den ausr-Befehl (Inventory)
#define IC_LIVING	"Lebendiges"
#define IC_WEAPON	"Waffen"
#define IC_ARMOUR	"Ruestungen"
#define IC_CONTAINER	"Behaelter"
#define IC_CLOTHES	"Kleidung"
#define IC_FOOD		"Nahrung"
#define IC_MONEY	"Geld"
#define IC_VALUEABLES	"Wertvolles"
#define IC_OTHER	"Sonstiges"

#define IF_HIDE_INVENTORY	1

// Diese Flags duerfen nicht mit den CONTENTS-Flags kollidieren
#define LOOK_SHORT		0x08
#define LOOK_COST		0x10    // Mustere-ZP-Kosten fallen an

#endif // EYES_H
