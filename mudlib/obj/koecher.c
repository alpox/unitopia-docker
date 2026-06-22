// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/obj/koecher.c
// Description:

/*
 * Ein (Beispiel-) Koecher fuer /obj/schuss_waffe.
 */
inherit "/i/object/tasche";
 
void create()
{
    ::create();
    set_id(({"köcher"}));
    set_name("köcher");
    set_gender("maennlich");
    set_long("Ein Köcher augenscheinlich für Pfeile.\n");
    set_smell("Er riecht nach Leder.\n");
    set_material(({"leder"}));
    set_weight(3);
    set_max_internal_encumbrance(10);
    allow_only(({"pfeil"}),"Der Köcher ist nur für Pfeile geeignet.");
    set_value(20);
    clear_initial_conservation_data();
}
