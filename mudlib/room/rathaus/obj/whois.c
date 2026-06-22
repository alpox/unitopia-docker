// This file is part of UNItopia Mudlib.
// ---------------------------------------------------------------------
// File:          /room/rathaus/obj/whois.c
// Description:   Frontend des Who-is-Who-Buchs, siehe auch /apps/whois.
// Author:	  Sissi, Mai 1998

#pragma save_types
#define BACKEND "/apps/whois"


string *whois_liste, inhalt;


inherit "/i/object/buch";


void reset()
{
    whois_liste = BACKEND->query_whois_liste();
    set_max_page (sizeof (whois_liste));
    inhalt = BACKEND->query_inhalt();
}


void create ()
{
    buch::create();
    set_page_mode ("more");
    set_name ("who-is-who-Buch");
    add_id (({"who-is","who-is-buch","who-is-who","who-is-who-buch"}));
    set_long ("Ein schneeweißes Buch, seine Seiten sind in allen "
        "möglichen und unmöglichen Farben gehalten, keine Seite hat "
        "dieselbe Farbe wie eine andere."); 
    set_wizard_book (1);
    set_verzeichnis (1);
}


string query_verzeichnis_inhalt()
{
    reset ();
    return inhalt;
}


string query_page_inhalt (int nummer)
{
    if ((nummer < 1) || (nummer > sizeof (whois_liste))) return "";
    return BACKEND->query_whois(whois_liste[nummer-1]);
}

