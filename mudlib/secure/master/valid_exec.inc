// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/secure/master/valid_exec.inc
// Description:

#pragma strict_types

/*
 * Function name:   valid_exec
 * Description:     Checks if a certain 'program' has the right to use exec()
 * Arguments:       name: Name of the 'program' that attempts to use exec()
 *                        Note that this is different from object_name(),
 *                        Programname is what 'function_exists' returns.
 *                  NOTE, the absence of a leading slash in the name.
 * Returns:         True if exec() is allowed.
 */
int valid_exec(string name, object ob, object obfrom) {
    if(name==(LOGIN_OB".c")[1..] || name=="secure/chsh.c")
    {
	mapping tsfrom = ({mapping}) obfrom->transfer_ts();
	mapping tsto = ({mapping}) ob->transfer_ts();
	if(tsfrom)
	    ob->transfer_ts(tsfrom);
	if(tsto)
	    obfrom->transfer_ts(tsto);
	return 1;
    }
}
