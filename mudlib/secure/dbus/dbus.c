// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /secure/dbus/dbus.c
// Description: D-BUS-Registration
// Author:      Gnomi

#if __EFUN_DEFINED__(dbus_publish_object)
void create()
{
    dbus_publish_object(__PATH__(0) "auth", "/de/unitopia/auth",
    ([
        "de.unitopia.Authentification": "auth_"
    ]));

    dbus_publish_object(__PATH__(0) "ftpd", "/de/unitopia/ftp",
    ([
        "de.unitopia.FTPAccess": "ftp_"
    ]));

    dbus_publish_object(__PATH__(0) "gmcp", "/de/unitopia/gmcp",
    ([
        "de.unitopia.GMCP": "gmcp_"
    ]));

    dbus_publish_object(__PATH__(0) "info", "/de/unitopia/info",
    ([
        "de.unitopia.Information": "info_"
    ]));

    dbus_publish_object(__PATH__(0) "mail", "/de/unitopia/mail",
    ([
        "de.unitopia.Mail": "mail_"
    ]));

    dbus_publish_object(__PATH__(0) "news", "/de/unitopia/news",
    ([
        "de.unitopia.News": "news_"
    ]));

    dbus_publish_object(__PATH__(0) "player", "/de/unitopia/player",
    ([
        "de.unitopia.Player": "player_"
    ]));

    dbus_publish_object(__PATH__(0) "system", "/de/unitopia/system",
    ([
        "de.unitopia.System": "system_"
    ]));

    dbus_publish_object(__PATH__(0) "webmud2", "/de/unitopia/webmud2",
    ([
        "de.unitopia.WebMUD2": "webmud2_"
    ]));
}
#endif
