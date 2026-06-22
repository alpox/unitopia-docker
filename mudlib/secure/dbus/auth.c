// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /secure/dbus/auth.c
// Description: RPC-Funktionen
// Author:      Gnomi

private functions inherit "/secure/i/sperre";

#include <config.h>
#include <level.h>

#pragma no_warn_unused_variables
int level;
string real_name;

private void clear_vars()
{
    "*"::clear_vars();
    level = 0;
}

private int load_player(string name)
{
    if(real_name == name)
	return 1;

    if(sizeof(name - "abcdefghijklmnopqrstuvwxyz"))
	return 0;

    clear_vars();
    if(!restore_object(PLAYER_FILE(name)))
	return 0;

    return 1;
}

static <int|string>* auth_password(string name, string pass, string whatfor)
{
    string res;
    
    if(!load_player(name))
	return ({0, "Falsches Passwort."});

    res = check_password(pass, name, 0);
    if(res)
	return ({0, res});

    switch(whatfor)
    {
	case "usenet":
	    if(level<LVL_WIZ && !testplayerp(name))
		return ({0, "Sorry, der NNTP-Zugang ist noch nicht freigeschaltet."});
	    break;

	case "wiki":
	    if(level<LVL_WIZ)
	        return ({0, "Das Wiki steht nur Göttern zur Verfügung."});
	    break;

	case "ftp":
	    if(level<LVL_WIZ)
		return ({0, "Bitte benutzte Zugang als anonymous."});
	    break;
	
	case "smtp":
	    if(level >= LVL_WIZ)
	        break;
	    if(spielerratp(name))
	        break;
	    if(member(VORSTAND||({}), name)>=0)
	        break;
	    return ({0, "Bitte benutze die Postämter."});
    }

    return ({level, ""});
}
