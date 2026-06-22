// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /apps/news_index.c
// Description: Suche in den News
// Author:      Gnomi (24.03.2002)

// UID: Apps

#define SECURE	(object_name(previous_object())=="/apps/newsd" || \
                 !strstr(object_name(previous_object()),"/obj/newsreader#") || \
		    (adminp(this_player()) && this_player()==this_interactive() && \
		     geteuid(this_player())==geteuid(previous_object())))

#include <erq.h>
#include <level.h>

void add_artikel(string brett, string gruppe, int artikel)
{
    if(!SECURE)
	return;
	
#if __VERSION__ > "3.5.2"
    if(!send_erq(ERQ_EXECUTE, to_bytes("news_add " + brett + "/" + gruppe + "/" + artikel, "UTF-8")))
#else
    if(!send_erq(ERQ_EXECUTE, "news_add " + brett + "/" + gruppe + "/" + artikel))
#endif
	sys_log("newsindex",sprintf("Fehler bei add_artikel(%s,%s,%d)\n", brett, gruppe, artikel));
}

void delete_artikel(string brett, string gruppe, int artikel)
{
    if(!SECURE)
	return;
    // Fuer zukuenftige Erweiterungen.
    // Derzeit ist es unmoeglich, eine Datei aus dem Index zu entfernen,
    // daher wird der Index Nachts voellig neu gebaut.
}

#include <more.h>
private void writeit(string *dateien, object tp)
{
    if(!dateien && tp)
	tell_object(tp, "Fehler\n");
    if(tp)
	tp->more(dateien, 0, 0, M_AUTO_END | M_THIS_OBJECT);
}

private void lookup_callback(int *response, int len, closure callback, mixed var)
{
    if(!callback)
	return;
	
    if(len<0) // ERQ ist abgekratzt...
    {
	funcall(callback, 0);
	sys_log("newsindex", "Stale ERQ!\n");
	return;
    }
    
    switch(response[0])
    {
	case ERQ_STDOUT:
	    var[0]+=to_string(response[1..]);
	    break;
	case ERQ_EXITED:
	    if(response[1]!=0)
		funcall(callback, trim(regreplace(var[0],"^err:|\n\\.\n$|\n"," ",1)));
	    else
		funcall(callback, explode(var[0],"\n")-({""}));
	    break;
	case ERQ_STDERR:
	    sys_log("newsindex","STDERR:\n"+to_string(response[1..]));
	    break;
	case ERQ_OK:
	    break;
	default:
	    funcall(callback, 0);
	    sys_log("newsindex",sprintf("Received Error: %O\n", response));
	    break;
    }
}

void lookup_artikel(string text, closure callback, string brett, string gruppe)
{
    string cmd;
    if(!SECURE)
	return;

    cmd = "news_lookup \""+regreplace(text-"`$","([\"])","\\\\\\1",1)+"\" ";
    if(brett)
    {
	cmd+= " " + brett;
	if(gruppe)
	    cmd+="/"+gruppe;
    }
    	
#if __VERSION__ > "3.5.2"
    if(!send_erq(ERQ_SPAWN, to_bytes(cmd, "UTF-8"),
#else
    if(!send_erq(ERQ_SPAWN, cmd,
#endif
	lambda( ({'resp, 'len}),
	    ({#'lookup_callback, 'resp, 'len, callback || lambda(({'res}),({#'writeit,'res,this_player()})) , '({ "" }) }) )))
	sys_log("newsindex",sprintf("Fehler bei lookup_artikel(%O,%O,%O,%O)\n", text, callback, brett, gruppe));
}
