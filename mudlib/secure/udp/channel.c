// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/secure/udp/channel.c
// Description:	Channelhandler fuer die Intermud-Channels
// Author:	Freaky (01.10.94)

#define DEBUGGER "menaures"
#include <debug.h>

#include <config.h>
#include <event.h>
#include <udp.h>
#include <level.h>
#include <invis.h>

#define CHANNEL "CHANNEL"

#define DISTRIB_FILE "/static/adm/DISTRIB"
mapping distrib = ([]);

void create()
{
   string file, *lines,  rest;
   int i, nr;

   if(file = read_file(DISTRIB_FILE))
      for(i = sizeof(lines = regexp(explode(file,"\n") - ({""}),"^[^#]")); i--;)
      {
	 if(sscanf(lines[i], "%d:%s", nr, rest) != 2)
	 {
	    write("Fehler in: "+DISTRIB_FILE+".\n");
	    continue;
	 }
	 if(distrib[nr])
	    distrib[nr] += map(explode(rest,",")-({""}),#'trim);
	 else 
	    distrib[nr] =  map(explode(rest,",")-({""}),#'trim);
      }
   else
      distrib = ([1:({"*"})]);
}

mapping query_distrib()
{
   return deep_copy(distrib);
}

string last_sender;
string query_current_sender() { return last_sender; }

void udp_channel(mapping data)
{
    string channel, cmd, sender, mud, text, anrede;
    mixed *remote;
    int idx, lvl;

    // some muds send 'channel' instead of 'CHANNEL'
    if(stringp(channel = data[CHANNEL]||data[lower_case(CHANNEL)]) &&
       stringp(sender  = data[SENDER]) &&
       stringp(mud     = data[NAME]))
    {
	text = data[DATA];

        remote = EVENT_MASTER->query_remote_channels();
        if((idx = member(remote[EVR_ID_STRING],
		       (channel = lower_case(channel)))) >= 0)
        {
    	    cmd = lower_case(data["cmd"] || data["CMD"] || "");
	 
	    if(cmd == "" || cmd == "emote")
	    {
		if(cmd == "emote")
		    anrede = "";
		else
		{
		    lvl = remote[EVR_LEVEL][idx];
		    if(lvl >= 50)
	    		anrede = " redet zum Obersten Rat:";
		    else if(lvl >= LVL_WIZ)
			anrede = " redet zum Pantheon:";
		    else
			anrede = " brüllt:";
		}
		
		last_sender = sender + "@" + mud;
		
		EVENT_MASTER->event(
		    capitalize(channel),
		    this_object(),
		    //capitalize(sender)+"@"+mud, 
		    wrap_say("["+capitalize(channel)+"] "+
		    capitalize(sender)+"@"+mud+ anrede,
#if __VERSION__ > "3.5.2"
		    text));
#else
		    convert_umlaute(to_string(text))));
#endif
		
		last_sender = 0;
	    }
	    else if(cmd == "list")
	    {
		mixed *listeners;
		
		listeners = EVENT_MASTER->query_listeners(capitalize(channel));
		listeners = map(listeners, function string(object pl)
		    {
			return playerp(pl) && !(pl->query_invis()&V_ATOM_INVIS)
				           && !pl->query_no_wer()
					   && capitalize(pl->query_real_name());
		    }) - ({0});
		 
		INETD->send_udp(data[NAME],
		    ([
			REQUEST: REPLY,
			RECIPIENT: data[SENDER],
			ID: data[ID],
			DATA: sizeof(listeners)?
				sprintf("Auf %s hören in " MUD_NAME " folgende Leute zu:\n%-75#s\n",
				    capitalize(channel), implode(sort_array(listeners, #'>),"\n")):
				sprintf("Niemand hört auf %s in " MUD_NAME ".\n", capitalize(channel))
		    ]) );
	    }
	}
    }
}

varargs void send(string channel, int dist, string text, string cmd)
{
   int i;
   string name, * hosts;
   object sender;
   mapping mhosts;

   sender = previous_object();
   if(!sizeof(hosts = distrib[dist]))
      return;
   if(hosts[0] == "*")
   {
      if(!(mhosts = INETD->query("hosts")))
	 return;

      // only serve hosts that we think are alive (TLE)
      mhosts = filter(mhosts, (: $2[HL_HOST_STATUS] > 0 :));
      hosts = m_indices(mhosts);
   }
   if(channel && text && sender && interactive(sender) &&
      (name = sender->query_real_name()))
      for(i = sizeof(hosts); i--;)
	 INETD->send_udp(hosts[i], ([ REQUEST : "channel",
				      CHANNEL : channel, 
				      DATA    : text,
				      SENDER  : name ]) +
                     (cmd ? ([ "cmd" : cmd ]) : ([])));
}
