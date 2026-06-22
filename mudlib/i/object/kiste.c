// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /i/object/kiste.c
// Description: Generischer Behaelter (default: kiste)
// Author:	Garthan '94
// Modified by:	Freaky (16.02.2000) Auf /i/base/container.c umgestellt
//              Jesaia (16.02.2000) container:init() eingebaut im init()
//              Jesaia (20.03.2000) query_long sauber gerouted :))

/////////////////////////////////////////////////////////////////////////
// Generischer Behaelter (default: kiste), created by Garthan '94
/////////////////////////////////////////////////////////////////////////
// 
// Ein Universalbehaelter, der sowohl tragbar, als auch 
// fest 'installiert' und unsichtbar sein kann.
//  
// Anwendungsmoeglichkeiten:
// Safe, Astloch, Kiste, Koffer, Container, Schrank,...
//
// Standardmaessig ist der Behaelter als Kiste konfiguriert,
// aber das kann man nach dem Clonen ja anpassen.
//
// Der Behaelter kann ein Schloss haben, dass mit /obj/schluessel
// 'zusammenarbeitet'.
//
/////////////////////////////////////////////////////////////////////////
//
// Alle weiteren Funktionen zur Veraenderung von Objekten aus /i/item
// und /i/base/container stehen natuerlich auch zur Verfuegung.
//
// Nuetzlich:
//   set_no_move
//   open_con		(Macht den Behaelter auf)
//   close_con		(Macht den Behaelter zu)
//   query_con_close	(Liefert den Zustand 1 zu, 0 offen)
//   set_content_message
//   set_max_internal_encumbrance
//   allow_only
//   set_name
//   add_v_item
//   set_weight
//   set_invis
//   set_no_lock
//   set_no_door
//   set_locked
//
/////////////////////////////////////////////////////////////////////////

#pragma save_types
#pragma strong_types

virtual inherit "/i/move";
virtual inherit "/i/item";
virtual inherit "/i/value";
virtual inherit "/i/base/container";

void init()
{
    item::init();
    container::init();
}
void create()
{
   item::create();
   container::create();
   set_name("kiste");
   set_id("kiste");
   set_material("holz");
   set_gender("weiblich");
   set_max_internal_encumbrance(10);
   set_collapsible(0);
   set_weight(7);
   set_value(20);
   set_crack(50);
   set_long("Eine geräumige Kiste.");
    if (program_name(this_object())==__FILE__) // fuer replace_program-Objekte
        clear_initial_conservation_data();
}

protected string query_long_postprocess(string msg, mapping info)
{
    return container::query_long_postprocess(
	item::query_long_postprocess(msg,info),info);
}
