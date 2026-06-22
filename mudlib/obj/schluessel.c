// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/obj/schluessel.c
// Description: Standardschluessel
// Author:	Garthan (10.02.94) fit logic getrennt

inherit "/i/item";
inherit "/i/move";
inherit "/i/value";
inherit "/i/object/key";

void create()
{
   set_name("schlüssel");
   set_gender("maennlich");
   set_id("schlüssel");
   set_material("metall");
   set_long("Ein zierlicher, kleiner Schlüssel.");
   set_weight(1);
   set_value(5);
   set_no_store(1);
}
