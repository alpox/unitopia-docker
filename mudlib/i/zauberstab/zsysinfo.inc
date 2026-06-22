// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/i/zauberstab/zsysinfo.inc
// Description:	Zeigt System-Informationen des Drivers an
// Author:	Freaky (1998)
// Modified by:	Freaky (20.09.2000) in den Zauberstab integriert.

#pragma strict_types

private string format_ms(int i)
{
    string ret;

    // Tausenstel Sekunden
    ret = sprintf("%03d",i % 1000);
    if (i /= 1000)
    {
	// Sekunden
	ret = sprintf("%02d.%s",i % 60,ret);
	if (i /= 60)
	{
	    // Minuten
	    ret = sprintf("%02d.%s",i % 60,ret);
	    if (i /= 60)
	    {
		// Stunden
		ret = sprintf("%d:%s",i,ret);
	    }
	}
    }
    return ret;
}

int z_obst(string str)
{
    SECURE;
    HELP("zobst");
    
    "/apps/object_db"->start_menu();
    
    return 1;
}

int z_sysinfo(string str)
{
    int *us;

    SECURE;
    HELP("zsysinfo");

    if (str)
	return 0;

    us = rusage();
    W(sprintf("Driver-Version: %28s\n",__VERSION__));
    W(sprintf("CPU-User:                         %10d ms   %13s\n",
		us[0],format_ms(us[0])));
    W(sprintf("CPU-System:                       %10d ms   %13s\n",
		us[1],format_ms(us[1])));
    W(sprintf("CPU gesamt:                       %10d ms   %13s    (%.2f %%)\n",
		us[0] + us[1],format_ms(us[0] + us[1]),
		to_float(us[0] + us[1])/to_float(query_up_time()*10)));
    W(sprintf("CPU-Auslastung (5 Minuten):       %10.2f %%\n",
		to_float(({int})"/apps/statistik"->query_five_time())/3000.0));
    W(sprintf("Load-Average:                              %s\n",
		query_load_average()));
    W(sprintf("Max-Mem-Size (resident):          %10d kb\n",us[2]));
    W(sprintf("integral shared text memory size: %10d\n",us[3]));
    W(sprintf("integral unshared data size:      %10d\n",us[4]));
    W(sprintf("integral unshared stack size:     %10d\n",us[5]));
    W(sprintf("page reclaims: (ohne I/O)         %10d\n",us[6]));
    W(sprintf("page faults:   (mit I/O)          %10d\n",us[7]));
    W(sprintf("swaps:                            %10d\n",us[8]));
    W(sprintf("block input operations:           %10d\n",us[9]));
    W(sprintf("block output operations:          %10d\n",us[10]));
    W(sprintf("messages sent:                    %10d\n",us[11]));
    W(sprintf("messages received:                %10d\n",us[12]));
    W(sprintf("signals received:                 %10d\n",us[13]));
    W(sprintf("voluntary context switches:       %10d\n",us[14]));
    W(sprintf("involuntary context switches:     %10d\n",us[15]));

#if __EFUN_DEFINED__(driver_info)
    {
	W(sprintf("Anzahl Objekte:                   %10d (%10d)\n",
	        efun::driver_info(DI_NUM_OBJECTS_IN_LIST),
	        efun::driver_info(DI_NUM_OBJECTS)));
    }
#else
    {
        int *sysinfo = efun::debug_info(DINFO_DATA,DID_STATUS);
	W(sprintf("Anzahl Objekte:                   %10d (%10d)\n",
		sysinfo[DID_ST_OBJECTS_LIST],sysinfo[DID_ST_OBJECTS]));
    }
#endif

    W(sprintf("Anzahl Spieler:                   %10d (total: %2d)\n",
	sizeof(users()),sizeof(efun::users())));

    PRINT;
    return 1;
}

struct top_uid_info* diff_top(struct top_uid_info* prev, struct top_uid_info* cur)
{
    /* Die Einträge sind sortiert nach UID. */
    int prevsize = sizeof(prev);

    return map(cur, function struct top_uid_info(struct top_uid_info entry) : int i = 0
    {
        string uid = entry.uid;
        if (i == prevsize)
            return entry;
        while (prev[i].uid < uid)
        {
            i++;
            if (i == prevsize)
                return entry;
        }
        if (prev[i].uid == uid)
        {
            struct top_uid_info pe = prev[i];
            i++;

            return (<top_uid_info>
                uid:      uid,
                evals:    entry.evals - pe.evals,
                data:     entry.data - pe.data,
                callouts: entry.callouts - pe.callouts,
            );
        }
        return entry;
    });
}

private void show_top(int command, closure sort, closure acc, int num, int repeat, int clrscr)
{
    struct top_uid_info* info = funcall(acc, get_top_info());

    info = sort_array(info, sort)[0..num-1];

    if (!command)
        begin_output("");
    if (clrscr)
        W(VT_CLR_SCR);
    W("UID                                     Evals       Data   CO\n");
    W("-------------------------------- ------------ ---------- ----\n");
    foreach (struct top_uid_info entry: info)
        W(sprintf("%-32s %12d %10d %4d\n", entry.uid, entry.evals, entry.data, entry.callouts));

    if (clrscr)
        this_player()->suspend_mudclient();
    if (!repeat)
    {
        PRINT;
    }
    else
    {
        this_player().send_message_to(this_player(), MT_UNKNOWN, MA_UNKNOWN, query_output());
        CLEAR_BUFFER;

        call_out(#'show_top, repeat, 0, sort, acc, num, repeat, clrscr);
    }
    if (clrscr)
        this_player()->restart_mudclient();
}

int z_top(string str)
{
    mapping opt;

    SECURE;
    HELP("ztop");

    if (remove_call_out(#'show_top) != -1)
    {
        W("ztop beendet.");
        PRINT;
        return 1;
    }

    opt = getopt(str || "", ([
        "e": 0, "d": 0, "c": 0,
        "t": 0, "i": 0, "a": 0,
        "n": "%d", "r": "%d", "v": "%d"
    ]), GO_FAILS);
    if (!opt)
        return 0;
    if (sizeof(opt["args"]))
        FAIL(query_verb()+" [-e|-d|-c] [-t|-i|-a] [-n <num>] [-r <sec>] [-v <sec>]");

    if (sizeof(({"e", "d", "c"}) & opt) > 1)
        FAIL(query_verb()+": Nur eines der Optionen -e, -d oder -c darf angegeben werden.");
    if (sizeof(({"t", "i", "a"}) & opt) > 1)
        FAIL(query_verb()+": Nur eines der Optionen -t, -i oder -a darf angegeben werden.");
    if (sizeof(({"i", "a"}) & opt) && !sizeof(({"r", "v"}) & opt))
        FAIL(query_verb()+": Die Optionen -i und -a benötigen -r oder -v.");

    show_top(
        /* Command or call_out */
        1,
        /* Sortierclosure */
        opt["c"] ? function int(struct top_uid_info i1, struct top_uid_info i2) { return i1.callouts < i2.callouts; } :
        opt["d"] ? function int(struct top_uid_info i1, struct top_uid_info i2) { return i1.data < i2.data; } :
                   function int(struct top_uid_info i1, struct top_uid_info i2) { return i1.evals < i2.evals; },
        /* Akkumulierung */
        opt["a"] ? function struct top_uid_info*(struct top_uid_info* info) : struct top_uid_info* last = get_top_info() { return diff_top(last, info); } :
        (opt["i"] || ((opt["r"] || opt["v"]) && !opt["t"]))
                 ? function struct top_uid_info*(struct top_uid_info* info) : struct top_uid_info* last = get_top_info() { return diff_top(last, last=info); } :
                   function struct top_uid_info*(struct top_uid_info* info) { return info; },
        /* Anzahl an Einträgen */
        (opt["n"] && opt["n"][0]) || 20,
        /* Wiederholung */
        (opt["r"] && opt["r"][0]) || (opt["v"] && opt["v"][0]),
        /* Bildschirm leeren */
        opt["v"] && opt["v"][0]
    );

    return 1;
}
