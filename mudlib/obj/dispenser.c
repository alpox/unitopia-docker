// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/obj/dispenser.c
// Description:
// Modified:	Offler (6.9.99) seinen() in query_long eingefuegt.

inherit "/i/money/dispenser";

void create ()
{
    replace_program ("/i/money/dispenser");
    ::create ();
}
