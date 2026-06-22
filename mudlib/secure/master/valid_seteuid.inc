// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/secure/master/valid_seteuid.inc
// Description:

#pragma strict_types
 
/*
 * Function name:   valid_seteuid
 * Description:     Checks if a certain user has the right to set a certain
 *                  objects effective 'userid'. All objects has two 'uid'
 *                  - Owner userid: Wizard who caused the creation.
 *                  - Effective userid: Wizard responsible for the objects
 *                    actions.
 *                  When an object creates a new object, the new objects
 *                  'Owner userid' is set to creating objects 'Effective
 *                  userid'.
 *                  
 * Arguments:       ob:   Objekt to set 'effective' user id in.
 *                  user: Wizard responsible for the setting.
 * Returns:         True if set is allowed.
 */
int valid_seteuid(object ob, string str)
{
    /*
     * An object can always be itself.
     */
    if (getuid(ob) == str)
        return 1;

    /*
     * Root can be anyone it pleases.
     */
    if (getuid(ob) == ROOT_UID || !strstr(getuid(ob),ROOT_UID+":"))
	return 1;

    /*
     * Jedes Objekt darf die EUID auf ("n:" + getuid()) setzen
     * aber nicht, wenn es schon "n:..." als UID hat.
     * Dies ist eine quasi Nobody-UID, da sie nichts besonderes darf.
     */
    if ((str == ("n:" + getuid(ob))) && strstr(getuid(ob),"n:"))
	return 1;

    return 0;
}
