// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /room/protokoll/virtual_compiler.c
// Description: Raeume und Protokollanten
// Author:      Myonara

#include <protokoll.h>

object virtual_compiler(string name, string path)
{
    string rn;
    object ob;

    if(sscanf(name, "klausur_%s", rn) && player_exists(rn) ) {
        ob = clone_object(OBJ_KLAUSUR_RAUM);
        return ob;
    }
    if(sscanf(name, "protokollant_%s", rn) && player_exists(rn) ) {
        ob = clone_object(OBJ_PROTOKOLLANT);
        return ob;
    }
}
