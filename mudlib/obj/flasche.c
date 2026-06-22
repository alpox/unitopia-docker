// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /obj/flasche.c
// Description: Eine Flasche, die man an Brunnen und Fluessen fuellen kann.
// Author:      Jafar & Yellow

inherit "/i/wasser/flasche";

void create() 
{
   replace_program("/i/wasser/flasche");
   ::create();
}

