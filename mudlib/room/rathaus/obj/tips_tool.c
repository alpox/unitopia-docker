// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/room/rathaus/obj/tips_tool.c
// Description: Werkzeug fuer die Tips aus /apps/tips
//              Arbeitet eng mit /apps/tips zusammen
// Author:	Sissi (14.01.2000)

inherit "/i/tools/browser";
inherit "/i/item";
inherit "/i/install";
inherit "../i/new_tip";

#include <level.h>
#include <more.h>
#include <browser.h>
#include <apps.h>

#define MORE_FLAGS M_LINE_NUMBERS | M_DO_NOT_END | M_FRAME
 
string query_long(object who)
{
    if (adminp (previous_object()))
        return wrap (
            "Mit diesem Tips - Werkzeug kannst Du die Tips aus "
            "/apps/tips aktivieren, deaktivieren, löschen, ändern "
            "und sie den unterschiedlichen Gruppen Newbies, Spieler, "
            "Engel und Götter zuordnen. Verwende dazu \"lies tips\". "
            "Auch kannst Du mit \"füge neuen Tip hinzu\" eben dies "
            "tun.");
    return wrap (
        "Ein Tips - Werkzeug. "+
        (wizp (previous_object())
            ?"Mit seiner Hilfe kannst Du die Tips lesen. Auch kannst Du "
             "mit \"füge neuen Tip hinzu\" einen neuen hinzufügen."
            :"Echt spannend."));
}

int secure ()
{
    return adminp (this_player()) && (this_player() == this_interactive())
        && (previous_object() == this_player());
}

varargs string query_read(string rest, string str)
{
    if (!wizp(previous_object()))
        return "Die Buchstaben tanzen wie wild vor Deinen Augen umher.\n";
    // Goetter duerfen die Tips browsen, wenn sie wollen.
    browse (0);
    return "";
}

string query_read_msg()
{
    if (wizp(this_player()))
        return Der(this_player())+" liest die Tips.";
}

void create()
{
    set_name ("werkzeug für Tips");
    set_gender ("saechlich");
    set_id (({"tips-tool","werkzeug","tips-werkzeug","tips",
        "tipps-tool","tipps-werkzeug","tipps"}));
    set_invis (1);
}

static mixed get_initial_more_info(string str)
{
    return ({ ({
        ({"Aktive Newbietips", "Aktive Spielertips", "Aktive Engelstips",
          "Aktive Goettertips", "Alle aktiven Tips",
          "Inaktive Newbietips", "Inaktive Spielertips",
          "Inaktive Engelstips", "Inaktive Goettertips",
          "Alle inaktiven Tips"}),
        "Tips Hauptmenü [q,<nr>,r,?]: ",
        1,
        MORE_FLAGS,
        0
    })});
}

mixed get_menue_more_info(mixed *m, int nr)
{
    return TIPS_MASTER->get_menue_more_info(m, nr);
}

static mixed browse_action(string str, mixed *m)
{
    mixed r;
    r = TIPS_MASTER->browse_action (str, m);
    if (r) return r;
    return ::browse_action (str, m);
}

static mixed get_back_more_info(mixed m)
{
    mixed r;
    r = browser::get_back_more_info(m);
    if (r)
        r[sizeof(r)-1][BR_BEGIN_LINE]=1;
    return r;
}

void init ()
{
    if (wizp (this_player()))
        add_action ("fuege","füge",-3);
}


int fuege (string s)
{
    notify_fail ("füge was? Vielleicht einen neuen Tip hinzu?\n");
    if (!s) return 0;
    s = lower_case (s);
    if ((strstr (s,"tip") == -1) || (strstr (s,"hinzu") == -1))
        return 0;
    new_tip ();
    return 1;
}

void new_tip_done ()
{
    write ("Neuer Tip erfolgreich in /apps/tips eingetragen.\n");
}

int query_auto_load ()
{
    return wizp (environment ());
}
