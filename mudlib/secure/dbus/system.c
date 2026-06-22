// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /secure/dbus/system.c
// Description: RPC-Funktionen
// Author:      Gnomi

#include <apps.h>

static void system_reload_certificates(string cert)
{
    if (!sizeof(cert))
        return;

    tls_refresh_certs();
    m_add(get_extra_wizinfo(0), "telnet_cert", cert);
}

static void system_update_encyclopedia()
{
    HELP_TOOL->do_read();
}

static void system_zz(string object_name)
{
    object ob = find_object(object_name);
    if(ob)
        destruct(ob);
}
