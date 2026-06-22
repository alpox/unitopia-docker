// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/secure/master/inherit_file.inc
// Description:	Regelt den Filenamen bei inherit "..."
// Author:	Freaky (01.12.1999)
// Modified by:	Freaky (13.01.2000) rel_path, search_file eingebaut.

int no_inherit_file(string file, string compiled_file)
{
    if (no_load_file(file))
	return 1;
    // Files aus /w duerfen nur von Files aus /w inherited werden.
    if (file[0..2] == "/w/" && compiled_file[0..2] != "/w/")
	return 1;
    // /secure/ darf alles (sicherheitshalber).
    if (compiled_file[0..7] == "/secure/")
	return 0;
    // Nur aus /i/ darf man inheriten.
    return strstr(file,"/i/") < 0;
}

mixed inherit_file(string file, string compiled_file)
{
    string ret;

    if (file[0] == '%')
    {
	if (file[1] == '/')
	    ret = file;
	else
	    // Wenn die Klasse mit einem '%' anfaengt, dann suchen.
	    ret = search_file("/i/" + file[1..] + ".c",compiled_file) || file;
    }
    else
	// Sonst relativen Pfad berechnen
	ret = rel_path(file,compiled_file);

    if (no_inherit_file(ret,compiled_file))
	return 1;

    return ret;
}
