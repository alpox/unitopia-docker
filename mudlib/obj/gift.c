// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /obj/gift.c
// Description: Gift fuer Brunnen und Flasche, infiziert den Spieler beim
//              Trinken mit einem Virus (mehr zu Viren in /obj/virus.c)
// Author:      Jafar & Yellow

inherit "/i/wasser/gift";


void create() 
{
   replace_program("/i/wasser/gift");
   ::create();
}

