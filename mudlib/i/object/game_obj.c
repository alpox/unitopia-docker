// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /i/object/game_obj.c
// Description: Das Spielobjekt, docs: siehe /doc/funktionsweisen/spiele
// Modified by: Croft (01.11.99) notify("set_game"), loesung entfernt

#pragma save_types

/*
 * This is a standard game object.
 * Configure it to make it look the way you want.
 */

inherit "/i/object/mission_obj";

#include <game.h>

void create()
{
    ::create();
    set_mission_type("spiel", "set_game", "Spiele");
}

private int secure_call()
{
   return previous_object() && object_name(previous_object()) == GAME_ROOM;
}

int has_test_flag()
{
    return GAME_ROOM->query_game_flags(query_name()) & GF_TEST;
}

int set_game(int points, object ob)
{
    if (!secure_call())
        return 0;

    return ::set_mission((points == G_SOLVED)?-1:points, ob);
}

int query_game() { return 1; }
