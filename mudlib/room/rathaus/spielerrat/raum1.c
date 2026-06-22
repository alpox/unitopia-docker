// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/room/rathaus/spielerrat/raum1.c
// Description: Rathaus, Spielerratsraeume, Konferrenzraum
// Author:      Goldie (10.2.2001)

inherit "%room";
inherit "%rauswurf";

#include <apps.h>
#include <level.h>
#include <config.h>
#include <invis.h>
#include <move.h>
#include <message.h>
#include <simul_efuns.h>

void reset()
{
    object tisch;
    
    "*"::reset();
    
    if(!present("Tisch1"))
    {
#ifdef UNItopia
        tisch = clone_object("/p/Item/Moebel/Sessel/obj/sitztisch");
        tisch->set_max_persons(sizeof(ADMINS+
            (SPIELERRAT->query_spielerrat())));
#else
        tisch = clone_object("/obj/tisch");
#endif
        tisch->set_name("konferenztisch");
        tisch->set_gender("maennlich");
        tisch->set_id(({"Tisch1","tisch","konferenztisch"}));
        tisch->set_long ("Der Tisch ist rund und steht in der Mitte des "
            "Raumes. Er erinnert Dich irgendwie an die legendäre Tafelrunde. "
            "An den einzelnen Sitzplätzen sind Namen zu lesen, so hat "
            "hier wohl jeder einen für ihn vorgesehen Sitzplatz.");
        tisch->set_smell ("Er riecht angenehm nach Holz.");
        tisch->set_feel  ("Du fühlst glattes poliertes Holz.");
        tisch->set_read  ("Du kannst in weißer Schrift die Namen "
            +liste(map(SPIELERRAT->query_spielerrat(), #'capitalize))+
            " lesen und gegenüber in silberner Schrift die Namen "
            +liste(map(ADMINS, #'capitalize))+".");               
        tisch->set_no_move_reason ("Und woran sollen die Leute dann sitzen, "
            "wenn du den Tisch einfach so mitnimmst?");
        tisch->set_invis(V_NOLIST);
	tisch->set_sitting_invis(V_VIS);
        tisch->set_put_verb("leg");
        tisch->move(this_object());
    }
}

void create()
{
    "*"::create();
    set_short("Konferenzzimmer des Spielerrates");
    set_long("Dies ist der Konferenzraum des Spielerrates. In der Mitte steht "
        "ein kreisrunder Tisch, an welchem sich die Mitglieder des "
        "Spielerrates und des Obersten Rates für wichtige Konferenzen "
        "versammeln. Ansonsten ist hier bis auf die kleine Tafel an der Wand "
        "eigentlich nichts weiter zu erkennen.");

   add_v_item(([
        "name":   "namen",
        "gender": "maennlich",
        "plural": 1,
        "id":   ({"name","namen"}),
        "long":   "Die Namen stehen ringsum an den Sitzplätzen.\n"
                  "Vielleicht solltest Du sie mal lesen.",
        "feel":   "Du kannst keine Vertiefungen oder so spüren, Du fragst "
                  "Dich, wie diese Namen dort hingekommen sind.",
        "read":   "Du kannst in weißer Schrift die Namen "
                  +liste(map(SPIELERRAT->query_spielerrat(), #'capitalize))+
                  " lesen und gegenüber in silberner Schrift die Namen "
                  +liste(map(ADMINS, #'capitalize))+".",
        "look_msg":  "$Der() schaut sich die Namen auf dem Tisch an.",
        "read_msg":  "$Der() liest sich die Namen auf dem Tisch durch." ]));

    set_tuer_richtung("süden");
    set_exit("gang1","süden");
    reset();
}

void init()
{
    "*"::init();
}
