// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /obj/statue_shadow.c
// Description: Shadow fuer die Statuen

inherit "/i/shadow";

#include <misc.h>

string query_short(object viewer)
{
    return Ihr((["name":"statue","gender":"weiblich"]),0,QSO);
}

// Damit Statuen nicht getoetet werden koennen.
varargs int add_hp(int i, mapping infos)
{
    return -1;
}

<int|string> let_not_in(mapping mv_infos)
{
    return 1;
}

<int|string> let_not_out(mapping mv_infos)
{
    return 1;
}

int id(string str)
{
    if (str == "statue")
        return 1;
    return QSO->id(str);
}

string *query_id()
{
    return (QSO->query_id() || ({}))+({"statue"});
}

string query_noise()
{
    return "Du vernimmst kein einziges Geräusch von der Statue.\n"
           "Doch - wenn Du ganz still bist, hörst Du etwas Knacken und Rascheln,\n"
           "als wenn sich Risse bilden würden.\n";
}

string query_smell()
{
    return "Du riechst - gar nichts. Es ist fast so, als ob die Statue nie gelebt hätte.\n";
}

string query_long(object betrachter)
{
    string text;
    int sec = time()-QSO->query_statue_time();
    int min = sec / 60;
    int hours = min / 60;

    if (min < 5)
        text = CNAME(QSO)+" wurde eben erst zur Statue.";
    else if (min < 15)
        text = CNAME(QSO)+" ist noch nicht lange eine Statue.";
    else if (min < 60)
        text = CNAME(QSO)+" ist noch nicht sehr lange eine Statue.";
    else if (hours < 2)
        text = CNAME(QSO)+" ist schon eine Weile eine Statue.";
    else if (hours < 12)
        text = CNAME(QSO)+" ist schon einige Stunden eine Statue.";
    else if (hours < 48)
        text = CNAME(QSO)+" ist schon viele Stunden eine Statue.";
    else 
        text = CNAME(QSO)+" ist schon einige Tage lang eine Statue.";

    return QSO->query_long(betrachter) + text + "\n";
}

