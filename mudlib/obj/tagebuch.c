// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /obj/tagebuch.c
// Description: Ein Tagebuch
// Author:      Sissi 2016, 2017

inherit "/i/item";
inherit "/i/install";

#include <level.h>
#include <message.h>
#include <misc.h>
#include <notify_fail.h>

object get_owner()
{
    if (environment() && playerp (environment()))
        return environment();
    else
        return 0;
}

void create() {
    ::create();
    set_name ("tagebuch");
    add_id (({"buch","tagebuch","tagesliste"}));
    set_gender("saechlich");
    set_no_move(1);
    set_no_move_reason(
        "Ein Tagebuch gibt man niemals aus der Hand. "
        "Und nein, man klaut auch keine Tagebücher.");
    set_weight(0);
}

void just_moved()
{
    object o = get_owner();
    if (o) {
        set_long ("Ein dickes Buch, das darauf wartet, durch "
        "glorreiche Taten, gefährliche Abenteuer, wagemutige "
        "Erkundungen und gewonnene Spiele noch ein bisschen "
        "mehr gefüllt zu werden. "
        "In großen, freundlichen Buchstaben steht "
        +o->query_real_cap_name()+" drauf.");
    }
}


string ort (mixed irgendwo)
{
    if (!irgendwo) return "";
    else return irgendwo+": ";
}

varargs string query_read(string a, string b, object leser)
{
    object owner = get_owner();
    if (a && (a != "") && wizp(this_player())) owner = find_player(a);
    if (!owner) return "Das Tagebuch ist leer.";

    mixed* textrepl = "/apps/achievements"->query_textrepl();
    mixed diary = owner->query_diary ();
    if (diary == 0 || sizeof(diary) == 0) 
        return "Das Tagebuch ist noch leer";
    if (query_input_pending(owner) || query_editing (owner))
        return "Du liest bereits einen Text. Eines nach dem Anderen.\n";

    string titel = "Tagebuch von "+owner->query_real_cap_name();
    string* result = ({titel,copies("=",strlen(titel)),""});
    for (int i = 0; i < sizeof (diary); i++) {
        string msg = diary[i][2];
        foreach(string* repl: textrepl)
            msg = regreplace(msg, repl[0], repl[1], 1);
        result += ({ort(diary[i][1]),
            "    "+vtimestr(time_to_vtime(diary[i][0]))+
            " ("+timestr(diary[i][0])+"):"});
        string *entry = explode(wrap(msg,70),"\n");
        for (int j = 0; j < sizeof (entry); j++) {
            result += ({"    "+entry[j]});
        }
    }
    this_player()->more(result);
    return "";
}
    
string query_short (object betrachter)
{
    object o = get_owner();
    if (o && o == betrachter) return "Dein Tagebuch";
    return "Ein Tagebuch";
}

void init()
{
    add_action("wegdamit","zerreiße", -7);
    add_action("wegdamit","verschrotte", -10);
    add_action("wegdamit","zerstöre", -7);
}

int wegdamit (string s)
{
    if (!me(s))
        FAILWP("Was willst du "+query_verb(1)+"n?", FAIL_NOT_OBJ);

    // Um Unfälle zu vermeiden (welche es bereits gab), machen wir
    // es etwas komplizierter. Es muss namentlich als Tagebuch
    // angesprochen werden und es muss wirklich gewollt sein.
    if (strstr(LOW(s), "tagebuch") < 0)
        FAILWP("Willst du etwa dein TAGEbuch "+query_verb(1)+"n?",
            FAIL_WRONG_ARG);

    if (strstr(LOW(s), "wirklich") < 0)
        FAILWP("Wenn du dein Tagebuch tatsächlich loswerden möchtest, "
            "dann musst du es WIRKLICH "+ query_verb(1)+"n.",
            FAIL_WRONG_ARG);

    send_message_to(TP, MT_NOTIFY, MA_REMOVE,
        "Als "+der()+" merkt, dass Du "+ihn()+" nicht mehr "
       "haben möchtest, löst "+er()+" sich sofort in eine Wolke aus "
       "Buchstaben auf und verschwindet.");
    remove();
    return 1;
}

mixed query_auto_load()
{
    return 1;
}
