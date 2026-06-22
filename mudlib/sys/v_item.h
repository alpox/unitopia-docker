// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/sys/v_item.h
// Description:	Defines fuer /i/item/v_item.c
// Author:	Gnomi

#ifndef V_ITEM_H
#define V_ITEM_H

// Befehlscodes fuer search_v_item
#define VV_ADD		0
#define VV_DELETE	2
#define VV_QUERY	4
#define VV_CHANGE	8

#define VV_COMMAND_MASK 14

// Flags fuer query_v_item
// Diese Flags duerfen keine Commandos sein
#define VV_NO_ROOM_ENV	1
#define VV_INVIS	16
#define VV_FLAT		32

#define VI_MASTER	"v_item_master"
#define VI_PATH		"v_item_path"
#define VI_SHADOW	"v_item_shadow"
#define VI_OBJECT	"v_item_object"
#define VI_ENVIRONMENT	"environment"

#endif
