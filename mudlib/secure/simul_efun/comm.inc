// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /secure/simul_efun/comm.inc
// Description: Hier werden alle Kommunikations-Efuns ueberlagert
// Author:      

#pragma save_types
#pragma strict_types

#include <message.h>

// We previous_object(), nur ohne Beachtung von LWOs.
private object get_message_source()
{
    object|lwobject ob;
    int i = 0;
    while((ob = previous_object(i++)))
    {
        if(objectp(ob))
            return ob;
    }

    return 0;
}

nomask void tell_object(object ob, string str)
{
    ob->receive_message(MT_UNKNOWN,MA_UNKNOWN,get_message_source(),str);
}

nomask varargs void tell_room(mixed ob, string str, object *exclude)
{
    object room, source;
    if(objectp(ob))
	room=ob;
    else if(stringp(ob))
	room=touch(ob);
    else
	raise_error("Bad argument 1 type to simul-efun tell_room()\n");

    source = get_message_source();
    filter_objects(exclude ? all_inventory(room) - exclude : all_inventory(room),
    	"receive_message",MT_UNKNOWN,MA_UNKNOWN,source,str);
    room->receive_message(MT_UNKNOWN,MA_UNKNOWN,source,str);
}

nomask varargs void say(string str, mixed exclude)
{
    object ob = get_message_source();

    if (!living(ob) && this_player())
    	ob = this_player();

    if (environment(ob))
    {
    	if (objectp(exclude))
	    exclude = ({ exclude, ob });
	else if (exclude)
	    exclude += ({ ob });
	else
	    exclude = ({ ob });
	tell_room(environment(ob),str,exclude);
    }
}

nomask void write(string str)
{
    if (this_player())
	this_player()->receive_message(MT_NOTIFY,MA_UNKNOWN,
	    	get_message_source(),str);
}

nomask varargs void printf(varargs mixed *args)
{
    if (this_player())
	write(apply(#'sprintf,args));
}
