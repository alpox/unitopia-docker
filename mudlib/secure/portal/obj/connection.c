// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/secure/portal/obj/connection.c
// Description:	Portal-Verbindung
// Author:	Gnomi

#pragma strict_types

#include <commands.h>
#include <configuration.h>
#include <portal.h>

#define MAX_BPS 65000
#define MAX_LENGTH 1000

string *puffer = ({});	// Alles ueber MAX_BPS puffern.
int out;		// Anzahl an gesendeten Bytes diese Sekunde
int last;		// Zeitpunkt, wann zuletzt was gesendet wurde.

string head;		// Der Header von save_value() der Gegenseite.
int sent_head;		// Haben wir unseren schon geschickt?

string cmd_in;		// Befehl von der Gegenseite (noch nicht alles erhalten)

string mud;		// Unsere Gegenseite
int ready;

int remove()
{
    if(mud)
        PORTAL_SERVER->closing(this_object());
    destruct(this_object());
    return 1;
}

private void send_delayed();
private void ready(int result, object me)
{
    mixed cert;
    string name, role;

    if(result)
    {
        sys_log("PORTAL", sprintf("Connection refused: %Q\n", result));
        remove();
        return;
    }

#if !defined(UNItopia) || __VERSION__ > "3.5.0-3075.U025"
    cert = tls_check_certificate(this_object());

    if(!cert || cert[0])
    {
        sys_log("PORTAL", sprintf("Certificate invalid: %Q\n", cert));
        remove();
        return;
    }

    for(int i=0; i<sizeof(cert[1]); i+=3)
        switch(cert[1][i])
        {
            case "2.5.4.3":  // CommonName
                name = cert[1][i+2];
                break;

            case "2.5.4.72": // Role
                role = cert[1][i+2];
                break;
        }

    if(name == "unitopia.de" && ({int})PORTAL_SERVER->opened(this_object(), "UNItopia")); else
    if(role != "Portal" || !({int})PORTAL_SERVER->opened(this_object(), name))
    {
        sys_log("PORTAL", sprintf("Certificate's role invalid: %Q\n", cert));
        remove();
        return;
    }
#endif

    ready = 1;

    remove_call_out(#'remove);
    remove_call_out(#'send_delayed);
    send_delayed();
}

int logon(int flag)
{
    if (flag < 0 || !efun::this_interactive())
    {
#if __VERSION__ < "3.5.1"
        call_out(#'remove, 0);
#else
        remove();
#endif
        return 0;
    }

#if __EFUN_DEFINED__(enable_telnet)
    enable_telnet(0);
    set_prompt("");
#else
    configure_interactive(this_object(), IC_TELNET_ENABLED, 0);
    configure_interactive(this_object(), IC_PROMPT, "");
#endif

    add_action("input","", AA_NOSPACE);

    if(tls_query_connection_state(this_object()))
        ready(0, this_object());
    else
    {
#ifdef PORTAL_CERT
        configure_driver(DC_TLS_CERTIFICATE, PORTAL_CERT);
#endif
        tls_init_connection(this_object(), #'ready);
    }
    return 1;
}

void net_dead()
{
    remove();
}

void connect(string ip, int port)
{
    if(object_name(previous_object()) != PORTAL_SERVER)
        return;

    call_out(#'remove, 10);
    if(net_connect(ip, port))
    {
        sys_log("PORTAL", sprintf("Connection to %s:%d refused.\n", ip, port));
        remove();
    }
}

static int input(string str)
{
    mixed value;

    if(!str)
        return 1;

    if(!head)
    {
        head = str + "\n";
        return 1;
    }

    if(!cmd_in)
        cmd_in = head;
    if(str[<1] == '\t')
    {
        cmd_in += str[0..<2];
        return 1;
    }

    str = cmd_in + str + "\n";
    cmd_in = 0;

    if(catch(value = restore_value(str)) || !mappingp(value))
    {
        net_dead();
        return 1;
    }

    PORTAL_SERVER->receive_tcp(this_object(), value);

    return 1;
}

private void send_delayed()
{
    int rest;
    
    if(last != time())
    {
	last = time();
	out = 0;
    }
    
    rest = MAX_BPS - out;
        
    while(sizeof(puffer) && rest > 0 && ready)
    {
	string msg = puffer[0];
	
	out += sizeof(msg);
	
	efun::tell_object(this_object(), msg[..rest-1]);
	
	if(rest < sizeof(msg))
	{
	    puffer[0] = msg[rest..];
	    break;
	}

        rest -= sizeof(msg);
	puffer = puffer[1..];
    }
    
    if(sizeof(puffer))
    {
	if(find_call_out(#'send_delayed)<0)
	    call_out(#'send_delayed, 1);
    }
}

void send_tcp(mixed val)
{
    string msg;

    if(object_name(previous_object()) != PORTAL_SERVER)
	return;

    msg = save_value(val);
    if(sent_head)
        msg = explode(msg, "\n")[1] + "\n";
    else
        sent_head = 1;
    
    while(sizeof(msg) > MAX_LENGTH)
    {
        puffer += ({msg[0..MAX_LENGTH-2] + "\t\n"});
        msg = msg[MAX_LENGTH-1..<1];
    }

    puffer += ({msg});
    if(find_call_out(#'send_delayed)<0)
        send_delayed();
}

void receive_message_low(string str)
{
    // Nichts machen, damit niemand dazwischenquatscht.
}

void catch_tell(string str)
{
    // Dito.
}

void set_mud(string name)
{
    mud = name;
}

string query_mud()
{
    return mud;
}
