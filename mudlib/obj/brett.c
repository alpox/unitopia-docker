// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/obj/brett.c
// Description: Ein Newsbrett
// Author:	Freaky (23.12.93)

inherit "/i/object/brett";

void create() {
    replace_program("/i/object/brett");
    ::create();
}
