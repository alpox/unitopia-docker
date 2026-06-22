// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /room/kirche/obj/wandnagel.c
// Description: Der Nagel in der Wand, wo nur der Kirchturmschluessel
//              daranhaengt. (auf mehrfachen Spielerwunsch ;)

#ifdef UNItopia
#include "/p/sys/kiste.h"
#define FILE MOEBELPFAD_I+"haken.c"
#else
#define FILE "/i/object/kiste"
#endif

inherit FILE;

#include <invis.h>
#include <message.h>

string query_long(object viewer)
{
    if (sizeof(all_inventory(this_object()))==0)
    {
        return wrap("Normalerweise hängt hier der Schlüssel zum Kirchturm.");
    }
    return ::query_long(viewer);
}

void create() {
    ::create();
    set_name("nagel");
    set_id(({ "nagel", "wandnagel", "kirche # wandnagel" }) );
    set_gender("maennlich");
    set_weight(1);
    set_long("Ein Nagel in der Wand.");
    set_no_door(1);
    set_max_internal_encumbrance(1);
    set_take_prepos("von");
    set_put_prepos("an");
    set_material(({"metall"}));
    set_smell("Pass auf, dass Du Deine Nase nicht einhängst!");
    set_feel("Klein, rund und spitz.");
    set_no_move(1);
    set_no_move_reason("Er ist zu gut befestigt!");
    allow_only(({"kirchturm#key"}),
            "Nur den Kirchturmschluessel kann man an diesen Nagel hängen.");
#ifdef MOEBELPFAD_I
    set_content_messages(({"\tAn dem Nagel hängt:",
                           "\tAn dem Nagel hängen:"}));
#endif
    add_controller("forbidden_put_into", this_object());
    set_invis(V_NOLIST);
}

int forbidden_put_into(object who, object ob, object where)
{
    if (!living(who) || !ob || where != this_object())
        return 0;
    if (sizeof(all_inventory(this_object()))==0)
        return 0;
    if (!ob->id("kirchturm#key"))
        return 0; // macht allow_only...
    send_message(MT_LOOK, MA_PUT, 
        wrap(Der(who)+" versucht vergeblich, einen weiteren Schlüssel auf den "
        "kleinen Nagel zu hängen."), wrap("Das ist ein Schlüssel zuviel."),
        who);
    return 1;
}