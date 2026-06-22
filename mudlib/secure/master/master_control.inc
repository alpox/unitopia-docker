// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/secure/master/master_control.inc
// Description: Funktionen zur Steuerung des Masterobjects

#pragma strict_types

protected void notify_shutdown(string crash_reason)
{
    /*
    if (previous_object() && previous_object() != this_object())
	return;
    */
#ifdef LOG_SHUTDOWN
    if(crash_reason)
	write_file("/log/sys/SHUT", "Crash: "+crash_reason+"\n");
    write_file("/log/sys/SHUT", "-- "+ctime(time())+"   "+__VERSION__+"\n");
#endif
    ERROR_DB->save_error_db();
}

#if __VERSION__ >= "3.5.1-U043"
protected int handle_external_signal(int signal)
{
    if (signal == SIGUSR1)
    {
        filter(efun::users(),#'efun::tell_object,
            "\nGame-Driver br\u00fcllt: Maschinen-Reboot!\n\n");

        "/room/rathaus/div/shut"->shut(5);
        return 1;
    }

    return 0;
}
#else
protected void external_master_reload()
{
    filter(efun::users(),#'efun::tell_object,
	"\nGame-Driver br\u00fcllt: Maschinen-Reboot!\n\n");
    "/room/rathaus/div/shut"->shut(5);
}
#endif

protected void slow_shut_down(int minutes)
{
    /*
    if (previous_object() && previous_object() != this_object())
    {
	write_file("/log/sys/MASTER",
		sprintf("slow_shut_down: PO:%O TP:%O TO:%O min:%O\n",
		previous_object(),this_player(),this_object(),minutes));
	return;
    }
    */

    write_file("/log/sys/SHUT","Calling slow_shut_down: " + ctime() +
	    " (with argument: " + minutes + ")\n");
    filter(efun::users(),#'efun::tell_object,
	"\nGame-Driver br\u00fcllt: Der Speicher wird knapp!\n\n");
    "/room/rathaus/div/shut"->shut(minutes || 5);

    MAILD->send_to_admin("Shutdown in "+minutes+" Minuten",
	sprintf("Es wurde slow_shut_down(%d) aufgerufen.\n"
	        "%s wird daher um diese Zeit neu gestartet: %s\n\n"
	        "MAX_MALLOC_SIZE (Mudlib-Grenze): %d\n"
		"__MAX_MALLOC__  (Driver-Grenze): %d\n"
	        "Vom Betriebssystem allokiert:    %d\n"
                "In Verwendung:                   %d\n"
		"Frei:                            %d\n\n",
		minutes, MUD_NAME, ctime(time()+30*60),
		MAX_MALLOC_SIZE, __MAX_MALLOC__,
#if __EFUN_DEFINED__(driver_info)
		efun::driver_info(DI_SIZE_SYS_ALLOCATED_BLOCKS),
		efun::driver_info(DI_SIZE_MEMORY_USED) + efun::driver_info(DI_SIZE_MEMORY_OVERHEAD),
		efun::driver_info(DI_SIZE_MEMORY_UNUSED))
	+efun::driver_info(DI_STATUS_TEXT_MALLOC)+"\n"
	+efun::driver_info(DI_STATUS_TEXT_TABLES));
#else
		efun::debug_info(DINFO_DATA, DID_MEMORY, DID_MEM_SBRK_SIZE),
		efun::debug_info(DINFO_DATA, DID_MEMORY, DID_MEM_ALLOCATED),
		efun::debug_info(DINFO_DATA, DID_MEMORY, DID_MEM_TOTAL_UNUSED))
	+efun::debug_info(DINFO_STATUS,"malloc")+"\n"
	+efun::debug_info(DINFO_STATUS,"tables"));
#endif
}

#ifdef MAX_MALLOC_SIZE
void reset()
{
#if __EFUN_DEFINED__(driver_info)
    if (efun::driver_info(DI_SIZE_MEMORY_USED) >= MAX_MALLOC_SIZE)
#else
    if (efun::debug_info(DINFO_DATA, DID_MEMORY)[DID_MEM_ALLOCATED] >= MAX_MALLOC_SIZE)
#endif
	slow_shut_down(1440);
}
#endif

/*
 * Diese Funktion wird aufgerufen, wenn das Laden des neuen Masters nicht
 * funktioniert.
 * Das Flag ist gesetzt wenn die alten Variablen preserved sind
 * bzw. wenn der Master bereits auf der Liste der zerstoerten Objekte war.
 */
protected void reactivate_destructed_master(int flag) {}

void read_variables(string what)
{
    if (!what)
	return;
    switch (what)
    {
      case "all":
	get_light_descriptions();
	get_skills();
	get_valid_shells();
	get_debugfile_offset();
	get_privis();
	restore_object(MASTER_SAVE_FILE);
	break;
      case "skills":
	get_skills();
	break;
    }
}

/*
 * Diese Funktion wird aufgerufen sobald der Master als Master vom Driver
 * anerkannt wurde, d.h. wenn er alle Privilegien eines Masters hat
 */
protected void inaugurate_master(int i)
{

    // if (i==0 && previous_object()==this_object())
    if (i == 0)
    {
	if (!get_extra_wizinfo(0))
	{
	    set_extra_wizinfo(0,([
	        "start":time(),
#ifdef TELNET_CERT
	        "telnet_cert": TELNET_CERT,
#endif
	        ]));
#ifdef LOG_SHUTDOWN
	    write_file("/log/sys/SHUT","%% "+MUD_NAME+" started at "+
		ctime(time())+"   "+__VERSION__+"\n");
#endif
	}
	else
	{
	    write_file("/log/sys/MASTER","Called inaugurate_master illegally:"+
		sprintf("TI:%O:\n",this_interactive()));
	}
    }

    setup_driver_hooks();
    read_variables("all");
#if __VERSION__ > "3.5.0-3000"
    // 100 MB should be enough for everyone. ;-)
#if __EFUN_DEFINED__(set_limits)
    set_limits(LIMIT_MEMORY, 100000000);
    set_limits(LIMIT_EVAL, 1500000);
#else
    int* limits = ({LIMIT_KEEP}) * LIMIT_MAX;
    limits[LIMIT_MEMORY] = 100000000;
    limits[LIMIT_EVAL]   = 1500000;
    configure_driver(DC_DEFAULT_RUNTIME_LIMITS, limits);
#endif
    // Nach 2s loggen.
    configure_driver(DC_LONG_EXEC_TIME, 2000000);
#endif

#if __VERSION__ >= "3.5.1-U043"
    configure_driver(DC_SIGACTION_SIGINT, DCS_TERMINATE);
    configure_driver(DC_SIGACTION_SIGHUP, DCS_THROW_EXCEPTION);
    configure_driver(DC_SIGACTION_SIGUSR1, DCS_INFORM_MASTER);    /* 5min shutdown */
    configure_driver(DC_SIGACTION_SIGUSR2, DCS_RELOAD_MASTER);
#endif
#if __VERSION__ > "3.5.2"
    configure_interactive(0, IC_ENCODING, "ASCII//TRANSLIT");
#endif

#if __EFUN_DEFINED__(configure_object)
    efun::configure_object(this_object(), OC_HEART_BEAT, 1);
#else
    set_heart_beat(1);
#endif
}
