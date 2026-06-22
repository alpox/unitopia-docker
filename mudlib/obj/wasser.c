// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /obj/wasser.c
// Description: Wasser fuer Brunnen und die Flasche
// Author:      Jafar & Yellow

inherit "/i/wasser/wasser";

void create() 
{
   replace_program("/i/wasser/wasser");
   ::create();
}

