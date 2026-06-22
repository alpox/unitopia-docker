// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /secure/dbus/player.c
// Description: RPC-Funktionen
// Author:      Gnomi

#include <apps.h>
#include <message.h>

static int player_tell(string src, string dest, string text)
{
    object pl = find_player(dest);
    if(!pl)
        return 0;

    pl->send_message_to(pl, MT_NOTIFY|MT_SENSE|MT_FAR|MT_INDENT, MA_COMM,
        sprintf("%s redet zu Dir: %s", capitalize(src), text));
    return 1;
}

static int player_start_banishing_wiz(string wiz)
{
    return GOETTER_REGISTER->start_banishing_wiz(wiz);
}
