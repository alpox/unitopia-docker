// This file is part of UNItopia mudlib.
// -------------------------------------------------------------------
// File:        /room/bank/test_schliessfaecher.c
// Description: test raum fuer Schliessfachtests im Pantheon...
// Author:      Myonara (30.12.2015)
// Modified by:
//  Myonara 04.Sep.2016  Als Testbank uebernnommen.
//

inherit "/i/room";
inherit "/i/money/schliessfachaddon";

#include <invis.h>
#include <misc.h>
#include <move.h>
#include <properties.h>
#include <room_types.h>

#include <money.h>

#define MY_BANK_ID "w_myonara"

void reset()
{
    "*"::reset();
}

void init()
{
    "*"::init();
}

void create()
{
    "*"::create(); // "*" wichtig, da auch das addon wichtige Raumsachen setzt.
    init_schliessfaecher(MY_BANK_ID,"taler");

    set_short("In den Räumen des Bankenkonsortiums");
    set_long("Dies ein Raum mit Schließfächern.");
    add_exit("bankenaufsicht","hoch",0,"Bankenaufsicht");
    reset();
}
