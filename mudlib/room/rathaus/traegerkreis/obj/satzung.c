/* File: /d/Vaniorh/fizban/obj/satzung.c
   Autor: Fizban
   Datum: 10.05. 1996
   Bemerkung: Die Satzung des Traegerkreises.
   Geaendert:
   Myonara  17.02.2018  parse_rest
*/


inherit "/i/item";
inherit "/i/move";
inherit "/i/value";

#include <verein.h>


varargs string query_read(string parse_rest, string str) {
    SATZUNG_TEXT 
}


int query_sellable() { 
    return 0; 
}


void create() {
    set_name("satzung");
    set_gender("weiblich");
    set_id(({"satzung","vereinssatzung","info","zettel",SATZUNG_ID}));
    set_short("Die Satzung des Trägerkreises");

    set_long("Ein kleiner Zettel, auf dem die Satzung des Trägerkreises "+
      "UNItopia e.V. geschrieben steht. Diese wichtige Info solltest "+
      "Du vielleicht mal lesen.");

    set_smell("Die Satzung riecht sehr wichtig!");
    set_noise("Die Satzung redet zu Dir: Lies mich! Lies mich!");

    set_material(({"papier","pergament"}));
    set_weight(1);

    set_value(50);
    set_no_store(1);
}
