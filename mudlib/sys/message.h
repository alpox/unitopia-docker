// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/sys/message.h
// Description:	Definiert Message- und Messageaction-Typen
// Author:	Freaky (18.05.97)

#ifndef MESSAGE_H
#define MESSAGE_H 1

#include <parse_com.h>

// Message-Typen

#define MT_UNKNOWN	0x000
#define MT_LOOK		0x001
#define MT_NOISE	0x002
#define MT_FEEL		0x004
#define MT_TASTE	0x008
#define MT_SMELL	0x010
#define MT_SENSE	0x020	// Uebersinnlich (z.B. rede)
#define MT_NOTIFY	0x040	// Einfache Statusmeldungen (z.B. Prompt)
#define MT_CHANNEL	0x080	// Kurier-Kanal
#define MT_FAR          0x100   // Das ganze auf die Ferne 
                                // (nicht im selben Raum)
#define MT_DEBUG	0x200	// Nur fuer Goetter
#define MT_FAIL     0x400   // Fehlschlagsnachrichten (Debug hat mehr Prio)
#define MT_MASK		0x7FF

#define MT_INDENT       0x10000 // Text mit wrap_say umbrechen.
#define MT_NO_WRAP      0x20000 // Text nicht umbrechen.

// Aktions-Typen

#define MA_UNKNOWN	0
#define MA_LOOK		1
#define MA_NOISE	2
#define MA_FEEL		3
#define MA_TASTE	4
#define MA_SMELL	5
#define MA_SENSE	6
#define MA_PUT		8
#define MA_TAKE		9
#define MA_MOVE_IN     10
#define MA_MOVE_OUT    11
#define MA_MOVE        13
#define MA_EMOTE       14
#define MA_FIGHT       15
#define MA_WIELD       16
#define MA_UNWIELD     17
#define MA_WEAR        18
#define MA_UNWEAR      19
#define MA_EAT         20
#define MA_DRINK       21
#define MA_COMM        22
#define MA_MAGIC       23	// Nur verwenden, wenn es in die anderen
				// Kategorien nicht passt!
#define MA_READ        24
#define MA_USE	       25
#define MA_CRAFT       26
#define MA_REMOVE      27

#define MSG_RECEIVER_WHOM   "message:receiver:whom"
#define MSG_OTHERS          "message:others"
#define MSG_FIRST_MSG       "message:first:message"
#define MSG_LAST_MSG        "message:last:message"
#define MSG_AH_INFOS        "message:ah:infos"
#define MSG_SOUND           "message:sound"
#define MSG_MXP_ITEMS       "message:mxp:items"

#define TELL_IDLE 900 /* 15 Minuten, bevor reden als idle zaehlt.*/


#define MSG_ATTR_SOUND(s) (funcall((:sizeof($1)?([MSG_SOUND:$1]):([]):),(s)))

// ACTION_SOUND_TO/OB: Da hier <properties.h> nicht included ist kann 
// P_SOUND_ACTIONS nicht verwendet werden. Daher wird hier direkt der 
// String "Root:sound:actions" verwendet.
#define ACTION_SOUND_TO(key,defaultSound) \
    MSG_ATTR_SOUND((query("Root:sound:actions")||([]))[(key)] \
    ||(defaultSound))

#define ACTION_SOUND_OB(ob,key,defaultSound) \
    (objectp(ob)?MSG_ATTR_SOUND((((ob)->query("Root:sound:actions")\
    ||([]))[(key)])||(defaultSound)):([]))

// fuer item muss ob mixed oder <object|mapping> sein!!
#define ACTION_SOUND_ITEM(ob,key,defaultSound) \
    MSG_ATTR_SOUND(((QUERY("Root:sound:actions",(ob))||([]))[(key)])\
    ||(defaultSound))
    
/*
FUNKTION: ACTION_SOUND_ITEM
DEKLARATION: ACTION_SOUND_ITEM(<object|mapping|mixed> ob,string key,string defaultSound)
BESCHREIBUNG:
Prueft ein mapping oder object ob auf P_SOUND_ACTIONS[key] nutzt dies oder den 
default, um den Sound zu setzen.
VERWEISE ACTION_SOUND_OB,ACTION_SOUND_TO
GRUPPEN: message
*/
/*
FUNKTION: ACTION_SOUND_OB
DEKLARATION: ACTION_SOUND_OB(object ob,string key,string defaultSound)
BESCHREIBUNG:
Prueft ein reines object ob auf P_SOUND_ACTIONS[key] nutzt dies oder den 
default, um den Sound zu setzen.
VERWEISE ACTION_SOUND_OB,ACTION_SOUND_TO
GRUPPEN: message
*/
/*
FUNKTION: ACTION_SOUND_TO
DEKLARATION: ACTION_SOUND_TO(string key,string defaultSound)
BESCHREIBUNG:
Prueft this_object() auf P_SOUND_ACTIONS[key] nutzt dies oder den 
default, um den Sound zu setzen.
VERWEISE ACTION_SOUND_OB,ACTION_SOUND_TO
GRUPPEN: message
*/



#endif // MESSAGE_H
