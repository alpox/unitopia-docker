// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/secure/master/preload.inc
// Description:

#pragma strict_types

/*
 * Load an Object at Startup.
 */
#define DMSG_PRELOAD (DMSG_STDOUT | DMSG_LOGFILE)
protected void preload(string file)
{
    mixed *us, us2;
    int t1, ev;
    string err;

    if (file[0] == '!')
    {
	// Einfach Meldungen ausgeben...
	debug_message(file[1..]+"\n", DMSG_PRELOAD);
	return;
    }

    if (file[<2..] == ".c")
	file = file[0..<3];
    if (file_size(file + ".c") == -1)
    {
	debug_message("Preloading: File '" + file + "' not found.\n", DMSG_PRELOAD);
	return;
    }

    debug_message(sprintf("Preloading: %-49s",file), DMSG_PRELOAD);
    ev = get_eval_cost();
    us = rusage();
    err = catch(load_object(file));
    us2 = rusage();
    if (err)
	debug_message("\nGot error " + err + " while loading " + file, DMSG_PRELOAD);
    t1 = us2[0] + us2[1] - us[0] - us[1];
    debug_message(sprintf(" %4d ms %6d ev\n",t1,ev - get_eval_cost()), DMSG_PRELOAD);
    // Damit der Offset stimmt fuer den ersten Fehler.
    get_debugfile_offset();
}
