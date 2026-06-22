// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /secure/dbus/info.c
// Description: RPC-Funktionen mit Infos zum laufenden Spiel
// Author:      Gnomi

#pragma no_warn_unused_variables

#include <apps.h>
#include <config.h>
#include <finger.h>
#include <invis.h>
#include <level.h>
#if __EFUN_DEFINED__(driver_info)
#include <driver_info.h>
#endif

#define LINE "+-------------------------------------------------------------+\n"
#define WIDTH 61

static mapping info_who()
{
    return ([
        "header":       LINE
                        "|" + center(MUD_NAME, WIDTH) + "|\n"
                        "|" + center(HOST_NAME " (" __HOST_IP_NUMBER__ ") " +
#ifdef UNItopia
                                     "23"
#elif  __EFUN_DEFINED__(query_mud_port)
                                     query_mud_port()
#else
                                     driver_info(DI_MUD_PORTS)[0]
#endif
                                      , WIDTH) + "|\n"
                        "|" + center("Gamedriver: LDMud "+__VERSION__,WIDTH) + "|\n"
                        "|" + center("Uptime: "+format_seconds(query_up_time()),WIDTH)+"|\n"
                        LINE,
        "mudname":      MUD_NAME,
        "sumusers":     STATISTIK->query_sum_users(),
        "maxusers":     STATISTIK->query_max_users(),
        "maxevertime":  timestr(STATISTIK->query_max_users_ever_time()),
        "maxever" :     STATISTIK->query_max_users_ever(),
        "numusers":     sizeof(users()),
        "users":        sort_array(map(filter(users(),
                        function int(object pl)
                        {
                            return !IS_INVIS(pl) &&
                                   !(pl->query_no_wer() && (wizp(pl) || testplayerp(pl))) &&
                                   pl->query_short() &&
                                   1;
                        }),

                        function mapping(object pl)
                        {
                            string title=pl->query_title();

                            return ([
                                "name": pl->query_cap_name(),
                                "real_name": pl->query_real_name(),
                                "sort_name": pl->query_name(),
                                "short": pl->query_short()
                            ]);
                        }),

                        function int(mapping a, mapping b)
                        {
                            return a["sort_name"]>b["sort_name"];
                        })
    ]);
}

static string info_finger(string name)
{
    return "/secure/udp/finger"->do_local_finger(name,0,FINGER_FLAG_OTHER_EXTERN);
}
