// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/obj/buch.c
// Description: Ein Buch
// Author:	Francis, Freaky

inherit "/i/object/buch";

void create() {
    replace_program("/i/object/buch");
    ::create();
}
