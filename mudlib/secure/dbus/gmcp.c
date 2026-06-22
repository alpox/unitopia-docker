// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /secure/rpc/ftpd.c
// Description: RPC-Funktionen fuer den externen GMCP-Server
// Author:      Gnomi

#include <config.h>
#include <files.h>

// Liefert den tatsaechlichen Dateinamen zurueck.
static string gmcp_check_file(string wiz, string filename, int write)
{
    int|string result;

#ifdef Orbit
    if (strstr(filename,"/var/spool/edit/"+wiz+"_")==0)
        return filename;
    if (write && filename[0..2] != "/w/")
        return 0;
#endif

    if (write)
        result = MASTER_OB->valid_write(filename, wiz, "write_file", 0);
    else
        result = MASTER_OB->valid_read(filename, wiz, "read_file", 0);

#if 0
    if (find_player("myonara"))
	tell_object(find_player("myonara"),"GMCP-Server:File name:"+filename+", wiz:"+
	  wiz+",\nwrite:"+write+", result:"+result+".\n"+sprintf("PO:%O\n",previous_object()));
#endif

    if (stringp(result))
        return result;
    else if (result)
        return filename;
    else
        return "";
}
