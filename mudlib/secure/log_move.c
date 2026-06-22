// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/secure/log_move.c
// Description: Tool zur Teleportliste
// Author:	Garthan

#pragma no_inherit

#include <level.h>

void create()
{
}

void log_move(object woher, object wohin)
{
   string verb;
   if(!query_once_interactive(previous_object()))
      return ;
   if(!(verb = query_verb()))
      verb = "--";
   if(strstr("zerneuere", verb) ||
      strstr(object_name(previous_object(1)),"/obj/zauberstab"))
   {
      write_file("/var/HOLE",
		       shorttimestr(time())+" "+
		       left(Name(this_interactive()),10)+" "+
		       left(Name(previous_object()), 10)+" "+
		       object_name(woher)+" "+
		       object_name(wohin)+" "+
		       query_verb()+"\n");
   }
}

void log_move_player(object woher, object wohin, int wie)
{
    string dwoher, dwohin;
    object pl = previous_object();
    
    if(!playerp(pl) || wizp(pl) || testplayerp(pl) || !woher || !wohin)
	return;

    dwoher = woher->query_room_domain() || "Unbekannt";
    dwohin = wohin->query_room_domain() || "Unbekannt";
    
    if(!stringp(dwoher))
	dwoher = sprintf("%Q", dwoher);
    if(!stringp(dwohin))
	dwohin = sprintf("%Q", dwohin);
    
    if(dwoher != dwohin && !sizeof(({"Ozean","Himmel"}) & ({dwoher, dwohin})))
    {
	string shstr;
	
	for(int i=1;i<caller_stack_depth();i++)
	{
	    object shob = previous_object(i);
	    
	    if(!shob)
		break;
	    
	    if(!shstr)
		shstr="";
	    else
		shstr+=", ";
	    shstr += sprintf("%Q", shob);
	    if(!query_shadowing(shob))
		break;
	}
	
	// Interdomainteleport
	
	sys_log("interdomain",
	    sprintf("[%s] %s -> %s: %Q - %Q durch %s\n",
		shorttimestr(time()),
		dwoher, dwohin,
		woher, wohin,
		shstr || ""));
    }
}
