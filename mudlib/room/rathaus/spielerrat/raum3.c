// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/room/rathaus/spielerrat/raum.c
// Description: Rathaus, Spielerratsraeume,Archiv

inherit "%room";

#include <apps.h>
#include <config.h>
#include <time.h>
#include <move.h>

#define BUCH "/room/rathaus/spielerrat/obj/buch"

#ifdef UNItopia 
#define MUELLEIMER "/p/Item/obj/muelleimer"
#endif

void reset()
{
    object regal,buch,kiste;
    int akt_jahr;
    akt_jahr = timearray(time())[TM_YEAR];
    
#ifdef MUELLEIMER
    if (!present("sr # mülleimer"))
    {
        object muelleimer = clone_object(MUELLEIMER);
        muelleimer->add_id(({"sr # mülleimer"}));
        muelleimer->move(this_object());
    }
#endif
    
    if(!(regal = present("Spielerratsregal")))
    {
	regal = clone_object("/obj/kiste");
	regal->set_name("regal");
	regal->set_id(({"regal", "bücherregal","Spielerratsregal"}));
	regal->set_gender("saechlich");
	regal->set_no_move(1);
	regal->set_long("Ein Bücherregal mit dem Archiv des Spielerrates.");
	regal->set_content_message("\tDarin stehen:");
        regal->set_no_door(1);
	regal->move(this_object());
    }
    if(!kiste = present("archiv # kiste"))
    {
        kiste = clone_object("/obj/kiste");
        kiste->set_name("kiste");
        kiste->set_id(({"kiste","archivkiste","buecherkiste","truhe",
            "archiv # kiste"}));
        kiste->set_gender("weiblich");
        kiste->add_adjektiv("staubig");
        kiste->set_no_move(1);
        kiste->set_max_internal_encumbrance(akt_jahr - 1996);
        kiste->set_material(({"holz","metall"}));
        kiste->set_no_move_reason("Die Archivkiste gehört ins "
            "Spielerratsarchiv - und nirgends sonst hin.");
        kiste->set_long("Eine große Kiste aus dunklem Holz mit schweren "+
            "Eisenbeschlägen. Hier bewahrt der Spielerrat alte Bücher "+
            "drin auf. Es ist die Archivkiste.");
        kiste->move(this_object());
        kiste->open_con();
    }
/*
 * Um hier neue Buecher hinzuzufuegen, muss man die Id des Buches
 * (der erste Parameter bei init_book) im archiv anmelden:
 * zc /apps/spielerratsarchiv->set_pages(id,({}));
 */
    if(!present("neues Buch",regal))
    {
        buch = clone_object(BUCH);
	buch->set_name("buch");
	buch->set_gender("saechlich");
	buch->set_id(({"buch","neues Buch"}));
	buch->set_adjektiv(({"neu"}));
	buch->init_book("Ideenbuch","Ideen, Vorschläge, Neuerungen, "
	   "Änderungen und Erweiterungen");
	buch->move(regal);
    }
    if(!present("Spielerratsbuch",regal))
    {
        buch = clone_object(BUCH);
	buch->set_name("buch des Spielerrats");
	buch->set_gender("saechlich");
	buch->set_id(({"buch","Spielerratsbuch","spielerratsbuch"}));
	buch->init_book("Spielerratsbuch","Organisatorisches und Sonstiges\n"
	    "uebern Spielerrat");
	buch->move(regal);
    }
    if(!present("Strafgesetzbuch",regal))
    {
        buch = clone_object(BUCH);
	buch->set_name("strafgesetzbuch");
	buch->set_gender("saechlich");
	buch->set_id(({"buch","Strafgesetzbuch","strafgesetzbuch"}));
	buch->init_book("Schuld und Suehne","Schuld und Sühne");
	buch->move(regal);
    }
    if(!present("Problembuch",regal))
    {
        buch = clone_object(BUCH);
	buch->set_name("problembuch");
	buch->set_gender("saechlich");
	buch->set_id(({"buch","Problembuch","problembuch"}));
	buch->init_book("Problembuch","Probleme, die Unitopia beschäftigen");
	buch->move(regal);
    }
    if(!present("Buch der Fälle",regal))
    {
        buch = clone_object(BUCH);
	buch->set_name("buch der Fälle");
	buch->set_gender("saechlich");
	buch->set_id(({"buch","Buch der Fälle","fallbuch"}));
	buch->init_book("buch der Faelle","Anklagen, Beschwerden, Probleme\n"
	    "von, mit und über bestimmte Spieler");
	buch->move(regal);
    }
    if(!present("log # buch", regal))
    {
        buch=clone_object(BUCH);
        buch->set_name("logbuch");
        buch->set_gender("saechlich");
        buch->set_id(({"logbuch","buch","log # buch"}));
        buch->init_book("log # buch","Aufzeichnungen, Mitschrfiten und Logs\n"+
            "zu Fällen des Spielerrats");
        buch->move(regal);
    }

    // Es folgen die alten Buecher fuer die Kiste
    for(int jahr = 1997; jahr <= akt_jahr; jahr++)
	if(!present("archivbuch # "+jahr, 1, kiste))
	{
            SPIELERRATSARCHIV->add_book("Fallbuch "+jahr);
            
            buch=clone_object(BUCH);
            buch->set_name("buch der Fälle ("+jahr+")");
            buch->set_gender("saechlich");
            buch->set_id(({"buch","archivbuch # "+jahr, "fallbuch", to_string(jahr)}));
            buch->init_book("Fallbuch "+jahr,"Anklagen, Beschwerden, Probleme\n"
	        "von, mit und über bestimmte Spieler\n"
            "Jahrgang "+jahr);
            buch->move(kiste, ([MOVE_FLAGS:MOVE_ERR_REMOVE]));
        }
}

string read_liste()
{
  this_player()->more("/static/adm/VERWARNUNGEN");
  return "";
}

void create()
{
    "*"::create();
    set_short("Das Archiv des Spielerrates");
    set_long(
      "In diesem Raum der vollgestellt ist mit Regalen, werden wichtige "
      "Aufzeichnungen des Spielerrates gesammelt, wie z.B. Protokolle "
      "der Sitzungen. Außerdem liegt hier auf einem Podest die Liste "
      "der Sünder aus, die gegen die Spielregeln UNItopias verstoßen "
      "haben. Ach ja, an der Wand hängt da noch ein besonders wichtig "
      "aussehendes Pergamentblatt.");

    add_v_item(([
    	       "name"	: "podest",
    	       "gender" : "saechlich",
    	       "long"	: "Ein schweres hölzernes Podest, auf dem die "
    	       		  "Liste der Leute liegt, die gegen die Spielregeln "
    	       		  "verstoßen haben.",
    	       "take"	: "Nana wer wird den das schöne Podest klauen wollen?",

    	      ]));
    add_v_item(([
    	       "name"   : "liste",
    	       "id"     : ({"sünderliste","liste"}),
    	       "gender" : "weiblich",
    	       "long"   : "Auf dieser Liste sind alle Leute verzeichnet, die "
    	       		  "bereits wegen eines Verstoßes gegen die Spiel"
    	       		  "regeln verwarnt wurden.",
    	       "read"	: #'read_liste
    	       ]));

    add_v_item(([
               "name"   : "wand",
               "id"     : ({"wand"}),
               "gender" : "weiblich",
               "long"   : "Eine Wand. Sie geht von unten nach oben."
              ]));

    add_v_item(([
               "name"   : "wand",
               "id"     : ({"wand"}),
               "gender" : "weiblich",
               "long"   : "Noch eine Wand. Du fühlst Dich umzingelt.",
               "nummer" : 2
              ]));

    add_v_item(([
               "name"   : "kodex des Spielerrates",
               "id"     : ({"kodex","spielerratskodex","srkodex","pergament",
                            "blatt","papier","pergamentblatt","pergamentpapier"}),
               "gender" : "maennlich",
               "long"   : "Eine recht frisch aussehendes Pergamentblatt, "
                          "auf welchem der Kodex des Spielerrates von "
                           MUD_NAME " geschrieben steht.",
               "read"   : (:this_player()->more("/static/adm/SPIELERRATSKODEX");
                            return "";
                          :)
               ]));

    set_exit("gang2","süden");
    reset();
}



