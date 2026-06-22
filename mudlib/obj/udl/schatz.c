// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/obj/schatz.c
// Description:

inherit "/i/item";
inherit "/i/move";
inherit "/i/value";
inherit "/i/udl/udl_item";

void create()
{
    udl_item::create();
    set_id(({"schatz"}));
    set_name("schatz");
    set_gender("maennlich");
    set_long("Du siehst nichts besonderes.\n");
    set_value(10);
}
