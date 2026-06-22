// Dieses File ist Teil der UNItopia MUDlib
// ------------------------------------------------------------------
// File:        /room/rathaus/spielerrat/obj/robe.c
// Author:      Myonara (aus dem Gericht/robe als Vorlage)


inherit "/i/clothes/kleidung";

#include <level.h>
#include <message.h>
#include <misc.h>

string extra_look()
{
    string rat = "";
    if (!query_worn()) 
      return 0;
    switch (environment()->query_gender()[0..0])
    {
        default:
        case "m": rat = " als Spielerrat";break;
        case "w": rat = " als Spielerrätin";break;
        case "s": rat = " als Spielerratendes";break;
    }
    return Er(environment())+" trägt eine blaue Robe, die "+
           ihm(environment())+" eine würdevolle Ausstrahlung verleiht und "
           "damit "+sein((["name":"autorität","gender":"weiblich",]),0,
           environment())+rat+" unterstreicht.";
}

int forbidden_wear_me(object who, object clothes)
{
    if (clothes == TO)
    {
        if (spielerratp(who))
            return 0;
        who->send_message_to(who, MT_NOTIFY,MA_UNKNOWN,
            "Diese Robe ist Spielerräten vorbehalten.");
        return 1;
    }
    return 0;
}

void notify_wear_me(object who, object clothes)
{
    if (clothes == TO && spielerratp(who))
    {
        object sh = clone_object("/room/rathaus/spielerrat/obj/robe_sh");
        sh->do_shadow(who);
    }
}

void notify_undress_me(object who, object clothes)
{
    who->no_sr_shadow();
}

void create()
{
    ::create();
    set_id(({ "robe","spielerratsrobe","cloth # meta # alle"}));
    set_name("robe");
    set_gender("weiblich");
    set_long("Eine blaue Robe für Spielerräte.");
    set_weight(1);
    set_value(0);
    set_material(({"textil"}));
    add_controller( ({
        "forbidden_wear_me",
        "notify_wear_me",
        "notify_undress_me",
        }) ,TO);
}


