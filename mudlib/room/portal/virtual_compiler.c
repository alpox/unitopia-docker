// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /room/portal/virtual_compiler.c
// Description: VC fuer die Aufbewahrungsraeume
// Author:      Gnomi
//

#include <portal.h>

object virtual_compiler(string name, string path)
{
    if (find_player(name))
        return clone_object(PORTAL_ROOM_BP);
}
