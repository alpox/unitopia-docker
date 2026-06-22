// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/secure/player_container/virtual_compiler.c
// Description:	Erzeugt die Objekt-Container fuer ausgeloggte Spieler
// Author:	Freaky (05.02.1999)

void create()
{
    seteuid(getuid());
}

object virtual_compiler(string name, string path)
{
    if (player_exists(name) || playerp(find_living(name)))
	return clone_object("/obj/player_con");
    if (!strstr(name, "chest-"))
        return clone_object("/obj/chest_con");
}
