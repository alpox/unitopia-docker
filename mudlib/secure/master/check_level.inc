// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/secure/master/check_level.inc
// Description: Levelcheck und Playercheck

#pragma strict_types


static string * valid_shells;

private void get_valid_shells()
{
   valid_shells = ({ "/obj/player", "/obj/newplayer" });
}

int playerp(object ob)
{
    if(!objectp(ob))
	return 0;
    return sscanf(object_name(ob),"/obj/player#%~d")==1 ||
	   sscanf(object_name(ob),"/obj/newplayer#%~d")==1 ||
	   sscanf(object_name(ob),"/obj/wizard_shell#%~d")==1;
}

int wizardshellp(object ob)
{
    return sscanf(object_name(ob),"/obj/wizard_shell#%~d")==1;
}

string * query_valid_shells()
{
   return ({})+valid_shells;
}


/*
 * Function name:   query_real_player_level
 * Parameter:	    Objekt oder Name eines Objektes.
 * Description:     Returns 0 if object isn't an interactive player
 *		    else returns the Level.
 */

int query_real_player_level(object player) {
    if (objectp(player) && playerp(player))
	return ({int})player->query_level();
}
