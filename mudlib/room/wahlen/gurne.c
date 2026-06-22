// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /room/wahlen/urne.c
// Description: Die Goetterurne
// Author:

inherit "/i/object/urne";

#include <level.h>

// dont clone me!

void create()
{
   ::create();
   set_id(({"gurne", "wahlgurne"}));
   set_name("wahlgurne");
   set_long(
      "Eine Gurne. Sie unterscheidet sich von einer Urne nur dadurch, "
      "dass damit nur Götterthemen gewählt werden sollen!");
   set_read("Eine Gurne kann man doch nicht lesen. Sowas geht doch nicht. "
       "Wenn Du schon was lesen möchtest, dann nimm Dir doch mal die "
       "Wahllisten vor. Oder auch die Listen, das ist weniger Tipparbeit.");
}

varargs static int may_vote(object who, string subject, mixed subject_data, string topic, mixed topic_data)
{
   return wizp(who);
}

static int may_read(object who)
{
   return wizp(who);
}

static mixed may_add_subject(object who, string topic)
{
   return wizp(who);
}
