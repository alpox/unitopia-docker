// File:           staedtetournier.c
// Description:    Temporaere Abstimmungsurne zur Spielabstimmung
// Author:         Gnomi (28.09.2009)
// 
// Sorcerer: uebernommen als Gesamtabstimmungsurne von Gnomi, 01.10.2009

inherit "/i/object/urne";

#include <level.h>

private int has_name(string text, string rn)
{
    return member(regexplode(lower_case(text), "[a-z]*"), rn)>=0;
}

static mixed may_add_subject(object who, string subject)
{
    if (wizp(who))
        return 1;
    // Niemand.
    return 0;
}

varargs static int may_delete(object who, string subject, mixed subject_data, string topic, mixed topic_data)
{
    if(adminp(who))
        return 1;

    if(!topic || !playerp(who))
        return 0;

    return (has_name(topic, who->query_real_name()) || topic_data == who->query_real_name());
}

varargs static int may_delete_empty_subject(object who, string subject, mixed subject_data, string topic, mixed topic_data)
{
    if (wizp(who) && !topic)
      return 1;
    return may_delete(who, subject, &subject_data, topic, &topic_data);
}

varargs static mixed may_add_topic(object who, string subject, mixed subject_data, string topic)
{
    return playerp(who);
}

static void do_add_topic(closure callback, object who, string subject, mixed subjectdata, string topic)
{
    funcall(callback, who->query_real_cap_name());
}

varargs static int may_vote(object who, string subject, mixed subject_data, string topic, mixed topic_data)
{
    if(!playerp(who))
        return 0;

    if(adminp(who))
        return 1;

    if(has_name(topic, who->query_real_name()))
    {
        topic_data = who->query_real_cap_name();
        return 1;
    }
}

static string print_topic(object who, string subject, mixed subject_data, string topic, mixed topic_data)
{
    if(lower_case(topic) == lower_case(topic_data))
        return topic;

    if(has_name(topic, lower_case(topic_data)))
        return topic;
    return sprintf("%s (von %s eingetragen)", topic, topic_data);
}

void create()
{
    ::create();
    set_short("Anmeldung für das Städtetournier");
    set_long(
        "Die Urne steht am Rand der großen Treppe.\n"
        "Um beim Städtetournier teilzunehmen, "
        "kannst Du mit Deinem eigenen Namen mit 'n' bei der Stadt Deiner Wahl "
        "eintragen und Dich wählen. Sollte Deine Stadt noch nicht aufgeführt "
        "sein, wende Dich bitte an den Schirmherrn dieser Stadt.\n"
        "Andere Spieler können auch eingetragen, aber nicht gewählt werden. "
        "Weitere Informationen zum Tournier gibt's am Brett Spieler, Allgemeines "
        "im Artikel 16522 und folgende.");
    add_id("anmeldung");
    set_name("anmeldeliste");
    set_gender("weiblich");
}
