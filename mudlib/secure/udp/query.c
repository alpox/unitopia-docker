// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/secure/udp/query.c
// Description:	Handles a UDP-Query
// Author:	

#include <udp.h>
#include <config.h>


void udp_query(mapping data) {
    mapping ret;

    switch(data[DATA]) {
	case "commands":
	    ret = INETD->query("hosts");
	    if (ret[lower_case(data[NAME])])
		ret = ([
		DATA: implode(ret[lower_case(data[NAME])][HL_LOCAL_COMMANDS], ":")
		]);
	    else
		ret = ([ DATA: implode(INETD->query("commands"), ":") ]);
	    break;
	case "email":
	    ret = ([ DATA: EMAIL ]);
	    break;
	case "hosts":
	{
	    mapping hosts;
	    string *list;
	    string tmp;
	    int i;

	    tmp = "";
	    for(i = sizeof(list = m_indices(hosts = INETD->query("hosts"))); i--; )
	    {
	        string hostip = hosts[list[i]][HL_HOST_IP];
	        
	        if(!strstr(hostip,"::ffff:"))
	            hostip = hostip[7..<1];
	        if(member(hostip, ':')>=0)
	            continue;
		tmp +=
		    hosts[list[i]][HL_HOST_NAME] + ":" +
		    hostip + ":" +
		    hosts[list[i]][HL_HOST_UDP_PORT] + ":" +
		    implode(hosts[list[i]][HL_LOCAL_COMMANDS], ",") + ":" +
		    implode(hosts[list[i]][HL_HOST_COMMANDS], ",");
		if (i)
		    tmp += "\n";
	    }
	    ret = ([ DATA: tmp ]);
	    break;
	}
	case "inetd":
	    ret = ([ DATA: INETD_VERSION ]);
	    break;
	case "list":
	    /* List of thingsthat can be queried. */
	    ret = ([ DATA: "commands:email:hosts:inetd:mud_port:time:version:encoding" ]);
	    break;
	case "mud_port":
#if __EFUN_DEFINED__(query_mud_port)
	    ret = ([ DATA: query_mud_port() ]);
#else
	    ret = ([ DATA: driver_info(DI_MUD_PORTS)[0] ]);
#endif
	    break;
	case "time":
	    ret = ([ DATA: time() ]);
	    break;
	case "version":
	    ret = ([ DATA: __VERSION__ ]);
	    break;
	case "encoding":
	    ret = ([ DATA: "UTF-8" ]);
	    break;
	default:
	    /* Just ignore it for the time being. */
	    return;
    }
    INETD->send_udp(data[NAME], ret + ([
	REQUEST: REPLY,
	RECIPIENT: data[SENDER],
	ID: data[ID],
	"QUERY": data[DATA]
    ]) );
}
