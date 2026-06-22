// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /i/player/webmud3.c
// Author:      Myonara (07.10.2019)
// Description: Support fuer den WebMUD3-Client

#include <telnet.h>
#if __EFUN_DEFINED__(interactive_info)
#include <interactive_info.h>
#endif

private nosave int wm3active;
private nosave mapping webmud3_infos=([]);

static void update_last_host();

static void set_webmud3_info(string key,mixed value)
{
    webmud3_infos[key] = value;
    if (wm3active && key=="real_ip")
    {
        update_last_host();
    }
}

static mapping get_webmud3_info()
{
    return webmud3_infos;
}

protected void init_webmud3()
{
    mixed client;
    string ip;

#if __EFUN_DEFINED__(query_ip_number)
    ip = efun::query_ip_number(this_object());
#else
    ip = efun::interactive(this_object()) && efun::interactive_info(this_object(), II_IP_NUMBER);
#endif
    ip||="";
    // DEBUG("IP("+ip+")");
    if( ip=="::ffff:217.11.52.246" // result from pasta/Webmud3
            && this_object()->query_telnet(TELOPT_TTYPE, &client)
            && sizeof(client)>1 && sizeof(client[1]))
    {
        wm3active = strstr(client[1][0],"webmud3")==0;
    }
    else
        wm3active = 0;

    if(wm3active)
    {
    }
}

int query_uses_webmud3()
{
    return wm3active;
}

static int uses_webmud3()
{
    return wm3active;
}

public varargs void gmcp_edit_file(string file,string title);

protected void start_webmud3_edit(string file)
{
    gmcp_edit_file(file);
}