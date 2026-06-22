// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/i/player/diary.c
// Description: Tagebuch fuer Spieler, wird von /apps/achievement gefuettert.
// Author:      Sissi

#pragma save_types
#pragma strong_types

#define ACHIEVEMENT_APPS  "/apps/achievements"

#include <achievements.h>
#include <level.h>

private mixed diary_list;

mixed query_diary ()
{
    if (!diary_list) return ({});
    return deep_copy (diary_list);
}

void announce_achievement(int type, mapping info)
{
if (this_object()->query_real_name()=="sissi")
tell_object(this_object(),"announce_achievement called "+type);
    if (!info || (type == AT_KILLED)
        || (object_name (previous_object()) != ACHIEVEMENT_APPS)) return;
    string msg = info[AI_TEXT_ME] || info[AI_TEXT_OTHER];
    if (!msg || msg == "") return;
    string gebiet;
    if (environment(this_object()))
        gebiet = environment(this_object())->query_room_domain();
    if (!diary_list) {
        int bday = this_object()->query_level_dates(LVL_PLAYER);
        if (bday && bday > 0) {
            diary_list = ({({bday, 0, "Mein Geburtstag"})});
        } else {
            diary_list = ({});
        }
    }
    diary_list +=
        ({({time(), gebiet, msg})});
}
