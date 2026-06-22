// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/secure/master/connections.inc
// Description:

#pragma strict_types

nosave int time_this_cycle = 0;
mapping connections_this_cycle;

/*
 * This function is called every time a player connects.
 * input_to() can't be called from here.
 */
protected object connect()
{
#if __EFUN_DEFINED__(query_mud_port)
    int port = efun::query_mud_port();
#else
    int port = efun::interactive_info(this_player(), II_MUD_PORT);
#endif
#if __EFUN_DEFINED__(query_ip_number)
    string ip = efun::query_ip_number();
#else
    string ip = efun::interactive_info(this_player(), II_IP_NUMBER);
#endif

#ifdef AUX_PORT
    if(port == AUX_PORT)
    {
        /* Derzeit keine lokale Verwendung. Portal-Verbindungen
         * duerfen nicht von localhost kommen, sonst koennen wir sie
         * keinem MUD zuordnen.
         */
        if(member(({"127.0.0.1", "::1", "::ffff:127.0.0.1"}), ip) >= 0)
            return 0;

#ifdef __TLS__ 
        // Ohne TLS keine Portale
        if(tls_available())
        {
            int err;

#ifdef PORTAL_CERT
            configure_driver(DC_TLS_CERTIFICATE, PORTAL_CERT);
#endif

            err = tls_init_connection();
            if(err < 0)
            {
                debug_message(tls_error(err));
                return 0;
            }
            
            return clone_object(PORTAL_CONNECTION);
        }
#endif
    }
#endif

     if(member(({"127.0.0.1", "::1", "::ffff:127.0.0.1"}), ip) < 0)
     {
        /* Fuer nicht-lokale Verbindungen eine Verbindung pro Backend-Zyklus zulassen. */
        if (time_this_cycle != time())
        {
            connections_this_cycle = ([:0]);
            time_this_cycle = time();
        }
        if (member(connections_this_cycle, ip))
        {
            write("Zuviele Verbindungen auf einmal.\n");
            write_file("/var/adm/MASS_CONNECTIONS", sprintf("[%s] %s\n", ctime(time()), ip));
            return 0;
        }
        else
            m_add(connections_this_cycle, ip);
     }

#ifdef __TLS__ 
#ifdef Orbit
    if(port == 9988 || port == 9987)
#else
    if(port == 992 || port == 3332)
#endif
    {
	int err;
	
	if(!tls_available())
	    return 0;
	
#ifdef TELNET_CERT
        configure_driver(DC_TLS_CERTIFICATE,
            (get_extra_wizinfo(0)||([]))["telnet_cert"] || TELNET_CERT);
#endif

        err = tls_init_connection();
        if(err < 0)
        {
    	    debug_message(tls_error(err));
	    return 0;
	}
    }
#endif
    return clone_object(LOGIN_OB);
}

/* Diese Funktion wird aufgerufen, wenn einem Spieler die TCP-Connection
 * abbricht.
 */
protected void disconnect(object ob, string remaining)
{
#if __VERSION__ < "3.6.3-U052"
    configure_interactive(ob, IC_ENCODING, "UTF-8");
#endif
    PLAYER_SECOND->logout(ob->query_real_name(), ob);
    STATISTIK->logoff(ob->query_real_name());
    ob->net_dead();
}

/* Diese Funktion wird anstatt player->quit() aufgerufen */
protected void remove_player(object victim)
{
    catch(victim->quit(); publish);
    if (victim)
	destruct(victim);
}

// Incoming UDP-Paket
//
#ifdef RELAYD
int relays;
int query_relays() { return relays; }
#endif

#if __VERSION__ > "3.5.2"
protected void receive_udp(string host, bytes buf, int port)
{
    string msg;
    if (catch(msg = to_text(buf, "UTF-8"); reserve 1000))
        msg = to_text(buf, "ISO8859-1");
#else
protected void receive_udp(string host, string msg, int port)
{
#endif
#if 0
    if (find_player("freaky"))
	tell_object(find_player("freaky"),
	    sprintf("UDP:%s:%d EVAL:%d TI:%O\n",host,port,get_eval_cost(),
		this_interactive()));
#endif

#ifdef RELAYD
   if(msg[0..6]=="relayd:")
      relays=to_int(msg[7..]);
   else
#endif
   {
      INETD->receive_udp(host,msg,port);
   }
}

protected void stale_erq(closure callback)
{
    if (find_player("freaky"))
	tell_object(find_player("freaky"),sprintf("stale_erq:%O:\n",callback));
    funcall(callback,-1,-1);
}

private void erq_stop()
{
    if (find_player("freaky"))
    	tell_object(find_player("freaky"),sprintf("ERQ-Stop\n"));
#ifdef __ERQ_MAX_SEND__ /* ERQ ist optional */
    attach_erq_demon("",0);
#endif
}
