// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /room/wahlen/urne.c
// Description: Die Engelsurne
// Author:      

inherit "/i/object/urne";

#include <level.h>

// dont clone me!

void create()
{
   ::create();
   set_long(
      "Eine Wahlurne. Hier kann über engelspezifische Themen "
      "abgestimmt werden.");
}

varargs static int may_vote(object who, string subject, mixed subject_data, string topic, mixed topic_data)
{
   return hlpp(who);
}

static mixed may_add_subject (object who, string subject)
{
   return hlpp(who) || adminp(who);
}
