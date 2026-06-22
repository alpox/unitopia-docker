// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/obj/tisch.c
// Description:	Ein ganz normaler Tisch
// Author:	Sissi (19.5.98), komplett neuer Tisch

inherit "/i/item";
inherit "/i/move";
inherit "/i/contain";


void create()
{
    ::create ();
    set_no_move_reason ("Einen Tisch? Einfach so mitnehmen? Wie würde "
        "denn das aussehen?");
    set_no_move(1);
    set_name("tisch");
    set_content_message ("        Auf dem Tisch liegen rum:");
    set_id ("tisch");
    set_gender("maennlich");
    set_max_internal_encumbrance(30);
    set_long ("Ein hölzerner Tisch.");
    set_material (({"holz"}));
    set_take_prepos ("von");
    set_put_prepos ("auf");
    set_put_verb (({"leg","stell"}));
    set_transparent(1);
}
