// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /room/wahlen/rangurne.c
// Description: Urne zur Rangnamenswahl
// Author:

#pragma no_clone

inherit "/i/object/urne";

#include <level.h>

static mixed may_add_subject(object who, string subject)
{
    return adminp(who);
}

varargs static int may_delete(object who, string subject, mixed subject_data, string topic, mixed topic_data)
{
    return adminp(who);
}

static void do_add_topic(closure callback, object who, string subject, mixed subject_data, string topic)
{
    // Wir merken uns den Namen in den Nutzerdefinierten Daten
    funcall(callback, who->query_real_name());
}

void create()
{
   ::create();
   set_id(({"urne", "rurne", "rangurne", "rangwahlurne", 
	    "rangnamenswahl", "rangnamenswahlurne"}));
   set_name("rangnamenswahlurne");
   set_long("Eine Urne mit einer Liste zum Vorschlagen und Abstimmen "
      "über neue Namen der Spielerränge.");
   set_read("Lies am besten die Listen.");
}
