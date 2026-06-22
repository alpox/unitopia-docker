// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/map/fish.c
// Description: Bereitstellen schwimmender Meeresbewohner fuer die Map
// Modified by: Kurdel (25.04.97) : Besser beschrieben, natural#weapon
//              Kurdel (20.08.97) : Delphin, Pottwal, Hai von Pif
//                                  eingebunden, if/else zu switch/case
//		Kurdel (28.08.97) : Clonen nur in Wasser-Raeumen

#include <level.h>
#include <message.h>
#include <landschaft.h>

void create() {
    seteuid(getuid(this_object()));
}

object get_fish_by_choice(string choice)
{
    object ob;
    switch (choice) {
      case  "pottwal"  :
        ob = clone_object("/map/obj/pottwal");
        break;
      case "thunfisch"  :
        ob = clone_object("/obj/monster");
        ob->initialize("thunfisch",5);
        ob->give_hands(1);
        ob->give_weapon_level(1);
        ob->give_armour_level(1);
        ob->set_gender("maennlich");
        ob->set_id(({"fisch","thunfisch", "map fish"}));
        ob->set_long("Ein Thunfisch. So ganz lebendig sieht er doch viel "
                     "schöner aus als in der Dose. Mit seinem kleinen "
                     "Maul kann er sich bestimmt auch etwas verteidigen.");
        ob->add_v_item(([
            "name"   : "maul",
            "gender" : "saechlich",
            "id"     : ({"maul", "mund", "natural#weapon"}),
            "long"   : "Sein Maul sieht nicht sehr gefährlich aus für "
                       "Dich."
        ]));
         ob->set_eatable_corpse(([
            "kadaver_set_smell":"Riecht nur leicht fischig.",
            "kadaver_set_noise":
                "Der ist nun noch stiller als stumm wie ein Fisch."]));
        break;
      case "killerwal" :
        ob = clone_object("/map/obj/killerwal");
        break;
      case "delphin" :
        ob = clone_object("/map/obj/delphin");
        break;
      case  "hai":  
      default :
        ob = clone_object("/map/obj/hai");
    }
    return ob;
}

void test_fish(string choice) {
    if (!wizp(this_player())) return;
    object ob = get_fish_by_choice(choice);
    ob->move(this_player());
    this_player()->send_message_to(this_player(),MT_NOTIFY,MA_USE,
        Ein(ob)+" in deinen Händen.");
}

void get_fish(object room, int x, int y) {
    object ob;
    int raumtyp;
    if (room && random(1000) >= 800 &&
       (raumtyp = room->query_type(LANDSCHAFT)) &&
       ((raumtyp & (L_WASSER | L_FLIESSEND | L_UNTERWASSER)) == raumtyp))
    {
    switch (random(1000)) {
      case  0..10  :
        ob = get_fish_by_choice("pottwal");
        break;
      case 11..30  :
        ob = get_fish_by_choice("thunfisch");
        break;
      case 31..50 :
        ob = get_fish_by_choice("killerwal");
        break;
      case 51..450 :
        ob = get_fish_by_choice("delphin");
	break;
      default :
        ob = get_fish_by_choice("hai");
        break;
    }
    ob->move(room);
    } // if
}

void get_hai(object room, int x, int y) {
    if (random(1000) < 500)
        return;
    object ob = get_fish_by_choice("hai");
    ob->move(room);
}

void prepare_renewal()
{
}

void abort_renewal()
{
}

void finish_renewal(object neu)
{
}