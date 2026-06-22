// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/i/object/rucksack.c
// Description: Ein Rucksack
// Author:	Sissi ?
// Modified by:	Freaky (16.02.2000) auf /i/base/container umgebaut

#pragma save_types

inherit "/i/clothes/kleidung";
inherit "/i/base/container";

#include <deklin.h>
#include <move.h>

void create()
{
    container::create();
    kleidung::create();
    set_class_id( ({"sack","rucksack"}) );
    set_id(({"sack","rucksack","wanderrucksack"}));
    set_name("rucksack");
    set_gender("maennlich");
    set_typ("rucksack");
    set_material("textil");
    set_worn_adjektiv("aufgesetzt");
    set_long("Ein großer Wanderrucksack mit Verstärkungen aus Leder.\n");
    set_max_internal_encumbrance(12);
    set_weight(2);
    set_value(80);
    set_schutz(0);
    set_put_verb("verstau");
    set_put_verb_case(FALL_DAT);
    set_no_lock(1);
    set_collapsible(1);
    add_controller("forbidden_open",this_object());
    add_controller("forbidden_close",this_object());
    set_content_visible_when_worn (0);
    if (program_name(this_object())==__FILE__) // fuer replace_program-Objekte
        clear_initial_conservation_data();
}

void init()
{
    kleidung::init();
    container::init();
}

protected string query_long_postprocess(string msg, mapping info)
{
    return container::query_long_postprocess(
        kleidung::query_long_postprocess(msg,info), info);
}

private int forbidden_open_close(object ob, object who, string str)
{
    if (ob != this_object())
	return 0;

    if (query_worn())
    {
        tell_object(who,wrap("Um " + den() + " zu " + str + ", musst Du " +
		    ihn() + " ersteinmal absetzen."));
        return 1;
    }
    return 0;
}

int forbidden_close(object ob, object who)
{
    return forbidden_open_close(ob,who,"schließen");
}

int forbidden_open(object ob, object who)
{
    return forbidden_open_close(ob,who,"öffnen");
}

<int|string> let_not_out(mapping mv_infos)
{
    if ((this_player() == environment()) && query_worn())
    {
        return
            "Um aus " + dem() + " etwas rauszunehmen, musst Du " +
            ihn() + " erst einmal absetzen.";
    }
    return ::let_not_out(mv_infos);
}

<int|string> let_not_in(mapping mv_infos)
{
    if ((this_player() == environment()) && query_worn())
    {
        return 
            "Um in " + den() + " etwas reinzutun, musst Du " +
            ihn() + " erst einmal absetzen.";
    }
    return ::let_not_in(mv_infos);
}
