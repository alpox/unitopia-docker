// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/secure/udp/who.c
// Description:	Handles a UDP-Who-Request
// Author:	Freaky (01.10.94)
// Modified by:	Freaky (02.03.96) Statistik auch fuer WWW
//		Freaky (08.12.97) HTML im short unterbinden

#include <udp.h>
#include <apps.h>
#include <level.h>
#include <invis.h>
#include <config.h>
#include <error.h>

#define WIDTH 61
#define LINE "+-------------------------------------------------------------+\n"
#define HEADER \
    header+center("Uptime: "+format_seconds(query_up_time()),WIDTH)+"|\n"+LINE

string header;

void create()
{
    header = LINE +
	"|" + center(MUD_NAME,WIDTH) + "|\n|" +
	center(HOST_NAME " (" __HOST_IP_NUMBER__ ") " +
#ifdef UNItopia
	"23"
#elif  __EFUN_DEFINED__(query_mud_port)
	query_mud_port()
#else
	driver_info(DI_MUD_PORTS)[0]
#endif
	,WIDTH) + "|\n|" +
	center("Gamedriver: LDMud "+__VERSION__,WIDTH) +
	"|\n|";

    return;
}

string volk_eintrag(object ob)
{
    string short, name;
    int l;

    short = ob->query_short();
    if(!short)
    {
        do_error2("query_short() liefert 0.\n", program_name(ob), object_name(ob), 0);
        short = "";
    }
    short = implode(explode(short,"&"),"&amp;");
    short = implode(explode(short,"<"),"&lt;");
    short = implode(explode(short,">"),"&gt;");

    name = ob->query_cap_name();
    if(!name)
    {
        do_error2("query_cap_name() liefert 0.\n", program_name(ob), object_name(ob), 0);
        name = capitalize(ob->query_name() || "");
    }

    int ende = short[<1];
    if ((ende != '.') && (ende != '!') && (ende != '?'))
        short += ".";
    if (lower_case(name) == ob->query_real_name() ||
	sizeof(regexp(({lower_case(name)}),"\\<"+ob->query_real_name()+"\\>")))
    {
	l = strlen(regreplace(lower_case(short),"\\<"+lower_case(name)+"\\>.*","",0));
	if(l==strlen(short))
    	    l = strstr(short,name);
	if (l < 0)
	    l = strstr(lower_case(short),lower_case(name));
        if (l >= 0)
    	    return "<LI>" + short[0..l-1] + "<A HREF = \"http://www." HOST_NAME 
		   "/cgi-bin/fing?name=" + ob->query_real_name() + "\">" + name + "</A>" +
		   short[l+strlen(name)..];
    }

    return "<LI>" + short;
}

void udp_who(mapping data)
{
    string *ret, snd;
    int short, anz;
    object *list;
    
    list = users();
    anz = sizeof(list);

    list = sort_array(filter(list, (: !IS_INVIS($1) &&
                                      !($1->query_no_wer() &&
                                        (wizp($1) || testplayerp($1))) &&
                                      $1->query_short() :)),
                      (: to_string($1->query_name()) > 
                         to_string($2->query_name()) :));

    if (data["HTML"])
    {
	ret = map(list, #'volk_eintrag);
	snd="<HTML>\n<HEAD>\n<TITLE>Die Liste der Leute, die gerade in "
	    MUD_NAME " eingeloggt sind</TITLE>\n</HEAD>\n<BODY>\n<PRE>\n"+
	    HEADER+"</PRE>\n"
	    "<H1>Folgende Leute sind gerade in UNItopia eingeloggt:</H1>\n"
	    "<UL>\n"+
	    implode(ret, "\n")+
	    "\n</UL>\n<B>Insgesamt "+anz+" Spielende"+
            (anz-sizeof(list)?", "+(anz-sizeof(list))+" davon unsichtbar."
                             :".")+
            "\n<BR>\n"
            "Heute waren bisher "
            +STATISTIK->query_sum_users()+
            " Leute in " MUD_NAME ", maximal "
            +STATISTIK->query_max_users()
            +" gleichzeitig.\n<BR>\nAm "
            +timestr(STATISTIK->query_max_users_ever_time())
            +" waren " + STATISTIK->query_max_users_ever() +
            " Spielende gleichzeitig da.\n</B>\n<HR>\n<ADDRESS>"+MUD_NAME+
	    " (<A HREF=\"mailto:"+EMAIL+"\">"+EMAIL+"</A>)</ADDRESS>\n"+
	    "</BODY>\n</HTML>\n";
    }
    else
    {
        if (data[DATA]) {
            if (strstr (data[DATA],"short") >= 0) short = 1;
        }
	ret = map_objects(list, short ? "query_cap_name" : "query_short");
        if (short)
            ret = explode(sprintf("%#-78s",implode(ret,"\n")),"\n");
        else
            for (int i = sizeof (ret); i--; ) {
                int ende = ret[i][<1];
                if ((ende != '.') && (ende != '!') && (ende != '?'))
                    ret[i] += ".";
                if (strlen (ret[i]) > 78) {
                    int j = 75;
                    while (j && (ret[i][j] != ' ')) j--;
                    ret[i] = ret[i][0..j-1]+"...";
                }
            }

	snd=HEADER+"\n"+
	    implode(ret, "\n")+"\n\nInsgesamt "+anz+" Spielende"+
            (anz-sizeof(list)?", "+(anz-sizeof(list))+" davon unsichtbar."
                             :".")+
            "\n"
	    +(short ? "" :
             "Heute waren bisher "
             +STATISTIK->query_sum_users()+
             " Leute in " MUD_NAME ", maximal "
             +STATISTIK->query_max_users()
             +" gleichzeitig.\nAm "
             +timestr(STATISTIK->query_max_users_ever_time())
             +" waren "
             +STATISTIK->query_max_users_ever()+
             " Spielende gleichzeitig da.\n");
    }

    INETD->send_udp(data[NAME], ([
	REQUEST: REPLY,
	RECIPIENT: data[SENDER],
	ID: data[ID],
	DATA: snd
	]) );

    return;
}
