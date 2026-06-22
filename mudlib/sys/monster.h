// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/sys/monster.h
// Description:	Defines fuer Monster
// Author:	Freaky (21.01.2000)

#ifndef MONSTER_H
#define MONSTER_H 1

// Aus Kompatibilitaet
#include <accept_objects.h>

// Flags (Bits) fuer set_random_pick()
#define PICK_NO_WIELD   1
#define PICK_NO_WEAR    2
#define PICK_ONLY       (PICK_NO_WIELD|PICK_NO_WEAR)

// Flags fuer set_add_parse_conversation()
#define PARSE_CONTINUE	1
#define PARSE_SAY	2
#define PARSE_TELL	4
#define PARSE_SOUL	8
#define PARSE_FOR_ME	16
#define PARSE_MESSAGE	32
#define PARSE_RE	64
#define PARSE_RE_TRADITIONAL	128
#define PARSE_RE_PCRE		256
#define PARSE_RE_CASE_SENSITIVE	512


#define PARSE_INFO_RECIPIENTS	"recipients"
#define PARSE_INFO_MSG_ACTION	"msg_action"
#define PARSE_INFO_MSG_TYPE	"msg_type"

#endif // MONSTER_H
