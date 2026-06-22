// This file is part of UNItopia mudlib.
// -------------------------------------------------------------------
// File:        /room/bank/virtual_compiler.c
// Description: vc fuer die persoenlichen Mietuebersichten.
// Author:      Myonara (15.10.2015)

#include <database.h>
#include <misc.h>
#include <room.h>

#include <money.h>

#define DEBUGGER "myonara"
#include <debug.h>

object virtual_compiler(string name, string path)
{
    DEBUG("vc-name: "+name);
    if (name[<2..] == ".c")
        name = name[..<3];

    string *split = explode(name,"_");
    if (sizeof(split) == 2 && split[0] == "mietuebersicht")
    {
        return clone_object(ZB_MIETUEBERSICHT);
    }
    return 0;
}

void prepare_renewal() {}
void abort_renewal() {}
void finish_renewal(object neu) {}
