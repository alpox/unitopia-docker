// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/secure/udp/muds.c
// Description:	Listet alle Muds, die per UDP erreichbar sind, auf
// Author:	Freaky (06.10.94)

#include <udp.h>
#include <more.h>

void create() {
}

void print_udp_muds(int long) {
    int i, time;
    mapping hosts;
    string *muds, ret;

    hosts=INETD->query("hosts");
    muds=sort_array(m_indices(hosts),#'<);
    if(long)
    {
	ret=sprintf("%-15s %-6s %s\n",
	   "Mudname","Status","Letzter Zugriff");
	ret+=copies("-",70)+"\n";
	for (i=sizeof(muds); i--; ) {
	    time=hosts[muds[i]][HL_HOST_STATUS];
	    ret+=sprintf("%-15s %-6s %s\n",
		 hosts[muds[i]][HL_HOST_NAME],
		    time>0?"da":time<0?"weg":"",
		    time?shorttimestr(abs(time)):"unbekannt");
	    }
    }
    else
    {
       for(ret = "", i = sizeof(muds); i--;)
       {
	  if((time=hosts[muds[i]][HL_HOST_STATUS]) <= 0)
	     continue;
//	     ret += "(";
	  ret += hosts[muds[i]][HL_HOST_NAME];
//	  if(time <= 0)
//	     ret += ")";
	  ret += "\n";
       }
      ret = "Muds via Inetd:\n" + sprintf("%-79#s", ret);
    }
    this_player()->more(explode(ret,"\n"),0,0,M_AUTO_END);
}
