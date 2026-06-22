// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/i/object/quest_obj.c
// Description: Das Raetselobjekt. docs: /doc/funktionsweisen/raetsel
// Modified by: Skafloc (06.05.97) hint_array
// Modified by: Mammi (01.11.99) notify("set_quest")

#pragma save_types

/*
 * This is a standard quest object.
 * Configure it to make it look the way you want.
 */

inherit "/i/object/mission_obj";

#include <quest.h>

private int not_necessary;

void create()
{
    ::create();
    set_mission_type("raetsel", "set_quest", "Raetsel");
}

private int secure_call()
{
   return previous_object() && object_name(previous_object()) == QUEST_ROOM;
}

int has_test_flag()
{
    return QUEST_ROOM->query_quest_flags(query_name()) & QF_TEST;
}

int set_quest(int points, object ob)
{
    if (!secure_call())
        return 0;

    return ::set_mission((points == Q_SOLVED)?-1:points, ob);
}

void set_not_necessary()
{
   if (!secure_call())
      return;
   not_necessary = 1;
}
int query_not_necessary() { return not_necessary; }

int query_quest() { return 1; }
