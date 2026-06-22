// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/obj/portal.c
// Description:	Ein Portal
// Author:	Gnomi

inherit "/i/object/portal";

void create()
{
    replace_program();
    ::create();
}
