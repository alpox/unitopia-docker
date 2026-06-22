// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/secure/udp/ping.c
// Description:	Handles a UDP-Ping-Request
// Author:	

#include <udp.h>
#include <level.h>

#define DEBUG(msg)	if (find_player("freaky"))\
			tell_object(find_player("freaky"), msg)

mapping last_replies = ([:1]);

void udp_ping(mapping data) {

//  DEBUG("In ping: "+mixed2str(data)+"\n\n");

    INETD->send_udp(data[NAME], ([
	REQUEST: REPLY,
	RECIPIENT: data[SENDER],
	ID: data[ID],
	DATA: LOCAL_NAME + " läuft.\n"
    ]) );
}

private void do_continuous_ping(string mud)
{
    INETD->send_udp(mud, ([
        REQUEST: PING,
        SENDER: object_name(),
    ]), 1);

    call_out(#'do_continuous_ping, 60, mud);
}

void start_continuous_ping(string mud)
{
    if (!adminp(this_interactive()))
    {
        raise_error("Illegaler Aufruf.\n");
        return;
    }

    do_continuous_ping(mud);
}

void udp_reply(mapping data)
{
    if (object_name(previous_object()) != "/secure/udp/reply")
    {
        raise_error("Illegaler Aufruf.\n");
        return;
    }

    last_replies[data[NAME]] = time();
}

mapping query_replies()
{
    if (!adminp(this_interactive()))
    {
        raise_error("Illegaler Aufruf.\n");
        return 0;
    }

    return last_replies;
}
