// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/i/object/tasche.c
// Description:	Die Tasche als exemplarischer Container
// Modified by:	Garthan (10.01.94)  msgs
//		Freaky (16.02.2000) Auf /i/base/container.c umgebaut
//              Jesaia (16.02.2000) container::init ins init()
//              Freaky (07.05.2000) auf /i/object/kiste umgestellt

#pragma save_types

inherit "/i/object/kiste";

void create()
{
    ::create();
    set_name("tasche");
    set_id("tasche");
    set_gender("weiblich");
    set_material("textil");
    set_long("Eine Tasche aus grobem Leinen, in ihr kann man Dinge "
	    "transportieren.");
    set_max_internal_encumbrance(6);
    set_weight(1);
    set_value(12);
    set_no_lock(1);
    set_collapsible(1);
    if (program_name(this_object())==__FILE__) // fuer replace_program-Objekte
        clear_initial_conservation_data();
}
