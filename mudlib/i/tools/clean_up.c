// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /i/tools/clean_up.c
// Author:      Kurdel (1998)
// Description: Inheritfile fuer Objekte, die sich im clean_up
//              zerstoeren sollen.
//              Zerstoeren passiert nur ohne Umgebung.
//              Anwendung: v_item-Master, blueprints, inherits, shadows

#pragma save_types

int remove()
{
    object obj;
    if ( (obj = this_object()->query_shadow_owner()) && obj != this_object())
         return obj->remove();
    if(this_object())
	destruct(this_object());
    return 1;
}

int clean_up(int arg)
{
    // query_enable_cleanup funktioniert ohnehin nur mit Umgebung
    if (!environment() && !this_object()->query_shadow_owner() && remove())
       return 0;
    return 1;
}
