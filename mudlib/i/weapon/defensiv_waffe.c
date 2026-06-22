// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/i/weapon/defensiv_waffe.c
// Description: Defensivwaffe
// Modified by:	Garthan (16.07.95) ohne shadow, wird als Ruestung behandelt
//              Offler (6.9.99) do_remove() wird nicht mehr aufgerufen, wenn
//                              der move nicht geklappt hat.

/* Die Defensivwaffe ist ein Hybrid. Sie ist Waffe und Ruestung 
   zugleich.
   Die Ruestungstaerke wird ueber set_damage(min, max) eingestellt und
   variiert wie bei der Waffe je nach Skill des Benutzers.
   Zugleich kann man wie bei der Waffe benoetigte Stats und Skillpfad
   abgeben.
   Sobald die Defensivwaffe gefuehrt wird, traegt sie sich sowohl
   in die Haende als auch in die Armourliste des Players ein.
   (query_hand_objects, query_armours)
   Mit der Defensivwaffe kann man zwar nicht angreifen, jedoch erhaelt
   man wie bei der normalen Waffe Erfahrungspunkte, und zwar dann,
   wenn man getroffen wird und die Ruestung etwas abhaelt (+random(10))
*/

virtual inherit "/i/weapon/weapon_logic";

#include <config.h>
#include <error.h>
#include <add_hp.h>

private int safe;

void do_wield()
{
   if(environment() && !environment()->armour_worn(this_object()))
   {
      environment()->add_armour(this_object());
      safe = compute_damage();
      ::do_wield();
   }
}

void do_remove()
{
   if(environment())
      environment()->delete_armour(this_object());
   ::do_remove();
}

int query_armour_protection()
{
   return safe;
}

int hit_action(object attacker, int ahps, mapping info)
{
   int rand, extra, act_safe;

   if(!info)
      info = ([ AH_ATTACKER: attacker ]);
   extra = this_object()->query_extra_armour_protection(ahps, info);
   act_safe = random(safe + extra + 1);
   rand = random(10);

   // man lernt nur, wenn auch ohne extra_armour_protection verteidigt wuerde
   if(rand == 0 && environment() && act_safe-extra > 0) 
      environment()->add_skill_points(query_skill_path(), LEARNING_DEFENSIV);
   if(rand == 5)
      safe = compute_damage();

   return act_safe;
}

int query_armour() { return 1; }

int query_worn()
{
   return environment() && environment()->armour_worn(this_object());
}

int query_wield()
{
   return query_worn();
}

private void defwa_notify_move(string ctrl,mapping mv_infos) // before_move
{
    mv_infos[__FILE__+":oldenv"] = environment();
}
private void defwa_notify_moved(string ctrl,mapping mv_infos) // after move
{
    object oldenv = mv_infos[__FILE__+":oldenv"];
    if (oldenv && oldenv != environment() && this_object())
    {
        oldenv -> delete_armour(this_object());
        ::do_remove();
    }
}

void create()
{
    "*"::create();
   set_id(({"waffe","defensivwaffe"}));
   set_class_id(({"waffe","defensivwaffe"}));
   set_weight(1);
   set_name("defensivwaffe");
   set_weapon_class("defensiv");
   set_gender("weiblich");
   set_long("Du siehst nichts Besonderes.\n");
   set_value(1,5);
   set_damage(1,3);
   set_skill_path(({"skill","defensiv","schild","klein"}));
   seteuid(getuid());
   safe = compute_damage();
   add_controller("notify_move",#'defwa_notify_move);
   add_controller("notify_moved",#'defwa_notify_moved);
   if (program_name(this_object())==__FILE__) // fuer replace_program-Objekte
        clear_initial_conservation_data();
}

int remove()
{
   do_remove();
   return ::remove();
}

varargs void init_weapon(string kategorie, int max_damage_percent,
    int min_damage_percent, int life_percent)
{
    int min,max,grenze,schaden,max_wert, min_wert;
    int tmp;

    switch(convert_umlaute(lower_case(kategorie)))
    {

        default:
	    do_error("Unbekannte Waffenkategorie '"+kategorie+"'!\n");
	    return;

	case "kleinschild":
	    set_skill_path(({"skill", "defensiv", "schild", "klein", }));
	    set_name("kleinschild");
	    set_gender("maennlich");
	    set_id(({"kleinschild", "schild",}));
            set_material("holz");
	    min=1; max=3; grenze=3;
	    break;

	case "grossschild":
	    set_skill_path(({"skill", "defensiv", "schild", "gross", }));
	    set_name("großschild");
	    set_gender("maennlich");
	    set_id(({"großschild", "schild", }));
            set_material("holz");
	    min=2; max=5; grenze=5;
	    break;

    }

    // Standards setzen.
    max_damage_percent = max_damage_percent || 100;
    min_damage_percent = min_damage_percent || max_damage_percent;
    life_percent = abs(life_percent) || 100;
    
    if(max_damage_percent<0)
    {
	if(max>20)
	    schaden = calc_percent(-max_damage_percent, 20, grenze);
	else
	    schaden = calc_percent(-max_damage_percent, max, grenze);
    }
    else
	schaden = calc_percent(max_damage_percent, max, grenze);

    schaden = max(schaden,1); // nach unten deckeln, Danke @ANIN.
    
     if(min_damage_percent<0)
    {
	if(max>20)
	    min = calc_percent(-min_damage_percent, min * 20 / max, grenze*2/3);
	else
	    min = calc_percent(-min_damage_percent, min, grenze*2/3);
    }
    else
	min = calc_percent(min_damage_percent, min, grenze*2/3);

    if(min > schaden*2/3) min = schaden*2/3;
    
    set_damage(min,schaden);

    set_weight(schaden);
    
    set_life(
        calc_percent(
            life_percent,
            ({1500, 1000, 750, 600, 500, })[schaden-1],
            ({1500, 1000, 750, 600, 500, })[schaden-1]
            )
        );

    max_wert = ({77, 177, 343, 677, 1477, })[schaden-1];

    max_wert = max_wert *
        query_life() / ({1500, 1000, 750, 600, 500, })[schaden-1];
    min_wert = max_wert / 4; //Gibts keine Richtlinie fuer...
    
    // So, nun diesen Wert ordentlich aufrunden, da einige Verkaeufer
    // diesen Wert direkt verwenden.
    // 2 Stellen genau, der Rest gerundet.
    tmp=1;
    while(max_wert>99)
    {
	max_wert=(max_wert+8)/10; // Bei 1 koennen wir ruhig abrunden... ;-)
	tmp*=10;
    }
    if(max_wert>49)
    {
	max_wert=(max_wert+4)/5;
	tmp*=5;
    }
    max_wert*=tmp;

    tmp=1;
    while(min_wert>99)
    {
	min_wert=(min_wert+8)/10;
	tmp*=10;
    }
    if(min_wert>49)
    {
	min_wert=(min_wert+4)/5;
	tmp*=5;
    }
    min_wert*=tmp;
    
    set_value(min_wert, max_wert);
}

