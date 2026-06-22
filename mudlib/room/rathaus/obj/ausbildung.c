// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/room/rathaus/obj/ausbildung.c
// Description: Schriftrolle mit Hinweisen zur Ausbildung von Goettern
// Author:	Sissi

inherit "/i/item";
inherit "/i/move";
inherit "/i/value";

#include <level.h>
#define PATH "/static/goetter/ausbildung"

void create()
{
    item::create();
    set_id(({"ausbildung","ausbildungsschriftrolle","schriftrolle","rolle"}));
    set_name("schriftrolle über die Ausbildung von Göttern");
    set_gender("weiblich");
    set_material("papier");
    set_weight(1);
    set_value(30);
    set_no_store(1);
    set_long("Die Schriftrolle ist ziemlich einfach und schlicht; "
        "eben einfach aufgerolltes Papier. Nichts besonderes.");
}

varargs string query_read(string rest, string str)
{
    if (!wizp (this_player()))
        return "Lauter wirre Zeichen stehen da drauf, die keinen Sinn "
            "zu ergeben scheinen.\n";
    this_player()->more(PATH);
    return "";
}

string query_read_msg()
{
    if (!wizp (this_player()))
        return Der(this_player())+" entrollt "
            +seinen (0,0,this_player())+" und macht ein ausgesprochen "
            "blödes Gesicht beim vergeblichen Versuch, die Schriftzeichen "
            +des()+" zu verstehen.";
    return Der(this_player())+" entrollt "
        +seinen (0,0,this_player())+" und beginnt, "+ihn()+" zu lesen.";
}
