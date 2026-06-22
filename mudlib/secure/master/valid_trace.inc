// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/secure/master/valid_trace.inc
// Description:	Ueberprueft, ob this_interactive() die Funktion trace()
//		benutzen darf.
// Author:	Freaky (10.07.1999)

#pragma strict_types

varargs int valid_trace(string what, mixed arg)
{
    object who;

#if 1
    if (find_player("freaky"))
    	tell_object(find_player("freaky"),sprintf("TRACE: What:%O Arg:%O\n",what,arg));
#endif

    who = this_interactive();

    if (!who || !playerp(who))
	return 0;

    write_file("/log/sys/PRIVI",sprintf("Special-CMD: %s %O\n",what,who));
    switch(what)
    {
	case "trace":
	case "traceprefix":
	    return adminp(who);
	default:
	    return 0;
    }
}
