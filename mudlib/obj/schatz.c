// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/obj/schatz.c
// Description:

inherit "/i/item";
inherit "/i/move";
inherit "/i/value";

void create() {
    set_id("schatz");
    set_name("schatz");
    set_gender("maennlich");
    set_long("Du siehst nichts Besonderes.");
    set_value(10);
    clear_initial_conservation_data();
}
