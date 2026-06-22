// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/secure/udp/reply.c
// Description:	Handles a UDP-Reply
// Author:	
// Modified by:	Freaky (11.07.95) M_* muessen mit | verknuepft werden !

#include <udp.h>
#include <more.h>

#ifdef ZEBEDEE
#include <defs.h>
#endif

#ifndef DATE
#define DATE	shorttimestr(time())
#endif

#define COMMANDS ({ "channel", "finger", "tell", "locate", "who", "mail", "news" })

void udp_reply(mapping data) {
    object ob;

    if (data[SYSTEM]) {
	switch(data[SYSTEM]) {
	case TIME_OUT:
	    if (data[SENDER]) {
		if (stringp(data[SENDER]) &&
			(ob = find_player(lower_case(data[SENDER]))))
		    tell_object(ob, "\ninetd: " + capitalize(data[REQUEST]) +
		    " request to " + (data[RECIPIENT] ?
		    capitalize(data[RECIPIENT]) + "@" + data[NAME] :
		    data[NAME]) + " timed out.\n");
		else if ((ob = (objectp(data[SENDER]) ? data[SENDER] : find_object(data[SENDER]))))
		    ob->udp_reply(data);
	    }
	    return;
	}
    }
    if (data[RECIPIENT]) {
        if(!sizeof(data[DATA]))
            return;
	/* If recipient is a player name, pass the message to them. */
	if (stringp(data[RECIPIENT]) &&
		(ob = find_player(lower_case(data[RECIPIENT]))))
            if(query_input_pending(ob) || query_editing(ob))
   	        tell_object(ob, "\n" + data[DATA]);
            else
                ob->more(explode("\n"+data[DATA],"\n")[0..<2],0,0,
		M_AUTO_END | M_THIS_OBJECT);
	/* Otherwise send it to udp_reply() in the recipient. */
	else if (objectp(ob) || (ob = find_object(data[RECIPIENT])))
	    ob->udp_reply(data);
	return;
    }
    switch(data[REQUEST]) {
	case "ping":
	    /* Ignore system ping replies. */
	    return;
	case "query":
	    switch(data["QUERY"] || data["query"])
	    {
		case "hosts":
		    foreach(mixed tmp:explode(data[DATA],"\n"))
		    {
		        mixed *host = INETD.parse_host_line(tmp,0);
		        if (!host || sizeof(host[HL_HOST_IP] & ".")!=3)
		            continue;
                        host[HL_LOCAL_COMMANDS] = COMMANDS;
		        INETD.ping_host_if_unknown(host);
		    }
		    return;

		case "list":
		    if ("encoding" in explode(data[DATA], ":"))
		        INETD.send_udp(data[NAME], ([ REQUEST: "query", DATA: "encoding" ]), 1);
		    return;

		case "encoding":
		    INETD.update_encoding(data[NAME], data[DATA]);
		    return;
	    }
	    // Fall through for unhandled query replies.
	default:
	    /* Log replies to the system. (ie. No RECIPIENT) */
	    if (data[DATA])
		sys_log(INETD_LOG_FILE, 
		    sprintf("%s:\n                   %-=60O\n",
			"Reply from " + data[NAME],
			data));
	    return;
    }
}
