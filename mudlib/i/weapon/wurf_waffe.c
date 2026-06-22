// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/i/weapon/wurf_waffe.c
// Description:
// Modified by:	Freaky (10.03.1998) message auf send_message umgebaut.
//		Offler (30.08.1999) do_wurf korregiert.

#pragma save_types

virtual inherit "/i/weapon/weapon_logic";

#include <move.h>
#include <config.h>
#include <add_hp.h>
#include <message.h>
#include <move.h>
#include <error.h>
#ifdef MONSTER_SCHIESSEN_IM_HEARTBEAT
#include <attack.h>
#endif
#include <misc.h>

void create()
{
    "*"::create();
   set_id(({"waffe","wurfwaffe"}));
   set_class_id(({"waffe","wurfwaffe"}));
   set_weight(1);
   set_name("wurfwaffe");
   set_weapon_class("wurf");
   set_gender("weiblich");
   set_long("Du siehst nichts Besonderes.\n");
   set_value(1,5);
   set_material(({"metall", "holz"}));
   if (program_name(this_object())==__FILE__) // fuer replace_program-Objekte
        clear_initial_conservation_data();
   seteuid(getuid());
}

#ifdef MONSTER_SCHIESSEN_IM_HEARTBEAT
varargs int do_wurf(object enemy, int flag)
{
   object owner, env;
   int res, damage, extra_damage, applied_damage, critical;
   string dem_enemy;
   
   if(!enemy || 
      !(owner = environment()) || 
      !living(owner) ||
      !query_wield())
      return ATTACK_DA_ABBRUCH;

   if(query_broken())
      return ATTACK_DA_WAR_BEREITS_KAPUTT;
      
   if(!check_life())
      return ATTACK_DA_KAPUTT;

   // Offler: compute_damage verwendet das environment und muss
   //         daher vor dem move erfolgen.
   extra_damage = this_object()->query_extra_damage(enemy, owner);

   critical = critical_hit();
   damage = compute_damage(critical);
   applied_damage = (critical ? damage : random(damage + 1)) + extra_damage;
   if (applied_damage < 0)
       applied_damage = 0;

   dem_enemy = dem(enemy); // Machen wir jetzt schon, weil die Bewegung
                           // Leute toeten kann.

   if(move(enemy, ([MOVE_TYPE: MOVE_TYPE_WERFEN])) != MOVE_OK &&
      (!(env = environment(enemy)) ||
       move(env, ([MOVE_TYPE: MOVE_TYPE_WERFEN])) != MOVE_OK))
   {
      owner->send_message_to(owner, MT_NOTIFY, MA_FIGHT,
         wrap(Der(enemy)+" wird von von den Göttern beschützt!"));
      return ATTACK_DA_ABBRUCH;
   }

#ifndef FILTER_MSG_BY_ATTRIBUTES
   owner->send_message(MT_LOOK, MA_FIGHT,
        wrap(Der(owner)+" wirft mit "+seinem(0,0,owner)+" nach "+dem(enemy)+
            "."),
        wrap(Der(owner)+" wirft mit "+seinem(0,0,owner)+" nach Dir."), enemy);
   this_object()->send_message_to(owner, MT_LOOK, MA_FIGHT,
                      wrap("Du wirfst mit "+deinem(0,0,owner)+" nach "+
                           dem_enemy+"."));
#else
   owner->send_message(MT_LOOK, MA_FIGHT,
        wrap(Der(owner)+" wirft mit "+seinem(0,0,owner)+" nach "+dem(enemy)+
            "."),
        wrap(Der(owner)+" wirft mit "+seinem(0,0,owner)+" nach Dir."), enemy,
                    ([ AH_ATTACKER: owner, AH_VICTIM: enemy,
                        AH_WEAPON:this_object(),
                        MSG_RECEIVER_WHOM: AH_VICTIM]));
   this_object()->send_message_to(owner, MT_LOOK, MA_FIGHT,
                      wrap("Du wirfst mit "+deinem(0,0,owner)+" nach "+
                           dem_enemy+"."),
        ([ AH_ATTACKER: owner, AH_VICTIM: enemy,
                   MSG_RECEIVER_WHOM: AH_ATTACKER,
                   AH_WEAPON:this_object(), ]) );
#endif
   if(!enemy)
      return ATTACK_DA_OK;

   if(!random(10))
      owner->add_skill_points(query_skill_path(),LEARNING_WURF);
    
#ifdef FIGHT_DEBUG
   fight_debug(owner,damage,extra_damage,applied_damage);
#endif

   env = environment(enemy);
   res = enemy->add_hp(-applied_damage, ([
      AH_ATTACKER: owner,
      AH_WEAPON: this_object(),
      AH_FLAGS: critical ? AH_NO_ARMOUR|AH_CRITICAL_MESSAGE : 0,
      AH_DAMAGE_TYPE: query_damage_type() || ({"stich"}),
   ]));
   if (res >= 0 && res < 6 && env && env != environment())
      move(env);
   return ATTACK_DA_OK;
}
#else // MONSTER_SCHIESSEN_IM_HEARTBEAT
void do_wurf(object enemy)
{
   object owner, env;
   int res, damage, extra_damage, applied_damage, critical;
   string dem_enemy;

   if(!enemy || 
      !(owner = environment()) || 
      !living(owner) ||
      !check_life() ||
      !query_wield())
      return;

   // Offler: compute_damage verwendet das environment und muss
   //         daher vor dem move erfolgen.
   extra_damage = this_object()->query_extra_damage(enemy, owner);

   critical = critical_hit();
   damage = compute_damage(critical);
   applied_damage = (critical ? damage : random(damage + 1)) + extra_damage;
   if (applied_damage < 0)
       applied_damage = 0;

   dem_enemy = dem(enemy); // Machen wir jetzt schon, weil die Bewegung
                           // Leute toeten kann.

   if(move(enemy) != MOVE_OK &&
      (!(env = environment(enemy)) || move(env) != MOVE_OK))
   {
      this_object()->send_message_to(owner, MT_LOOK, MA_FIGHT,
                         wrap(Der(enemy)+" wird von von den Göttern "
                                "beschützt!"));
      return;
   }

#ifndef FILTER_MSG_BY_ATTRIBUTES
   owner->send_message(MT_LOOK, MA_FIGHT,
        wrap(Der(owner)+" wirft mit "+seinem(0,0,owner)+" nach "+dem(enemy)+
            "."),
        wrap(Der(owner)+" wirft mit "+seinem(0,0,owner)+" nach Dir."), enemy);
   this_object()->send_message_to(owner, MT_LOOK, MA_FIGHT,
                      wrap("Du wirfst mit "+deinem(0,0,owner)+" nach "+
                           dem_enemy+"."));
#else
   owner->send_message(MT_LOOK, MA_FIGHT,
        wrap(Der(owner)+" wirft mit "+seinem(0,0,owner)+" nach "+dem(enemy)+
            "."),
        wrap(Der(owner)+" wirft mit "+seinem(0,0,owner)+" nach Dir."), enemy,
                    ([ AH_ATTACKER: owner, AH_VICTIM: enemy,
                        AH_WEAPON:this_object(),
                        MSG_RECEIVER_WHOM: AH_VICTIM]));
   this_object()->send_message_to(owner, MT_LOOK, MA_FIGHT,
                      wrap("Du wirfst mit "+deinem(0,0,owner)+" nach "+
                           dem_enemy+"."),
        ([ AH_ATTACKER: owner, AH_VICTIM: enemy,
                   MSG_RECEIVER_WHOM: AH_ATTACKER,
                   AH_WEAPON:this_object(), ]) );
#endif

   if(!enemy)
      return;

   if(!random(10))
      owner->add_skill_points(query_skill_path(),LEARNING_WURF);
    
#ifdef FIGHT_DEBUG
   fight_debug(owner,damage,extra_damage,applied_damage);
#endif

   env = environment(enemy);
   res = enemy->add_hp(-applied_damage, ([
      AH_ATTACKER: owner,
      AH_WEAPON: this_object(),
      AH_FLAGS: critical ? AH_NO_ARMOUR|AH_CRITICAL_MESSAGE : 0,
      AH_DAMAGE_TYPE: query_damage_type() || ({"stich"}),
   ]));
   if (res >= 0 && res < 6 && env && env != environment())
      move(env);
}
#endif // MONSTER_SCHIESSEN_IM_HEARTBEAT

varargs void init_weapon(string kategorie, int max_damage_percent,
    int min_damage_percent, int life_percent)
{
    int min,max,grenze,schaden,max_wert, min_wert;
    int tmp;

    switch(lower_case(kategorie))
    {

        default:
	    do_error("Unbekannte Waffenkategorie '"+kategorie+"'!\n");
	    return;

	case "speer":
	    set_skill_path(({"skill", "offensiv", "scharf", "speer", }));
	    set_name("speer");
	    set_gender("maennlich");
	    set_id("speer");
            set_material("holz");
	    set_damage_type(({"stich"}));
	    min=13; max=25; grenze=50;
	    break;

/*
	case "ger":
	    set_skill_path(({"skill", "offensiv", "scharf", "speer", }));
	    set_name("ger");
	    set_gender("maennlich");
	    set_id("ger");
            set_material("holz");
	    min=13; max=25; grenze=50;
	    break;
*/

	case "wurfmesser":
	    set_skill_path(({"skill", "offensiv", "scharf", "wurfmesser", }));
	    set_name("wurfmesser");
	    set_gender("saechlich");
	    set_id(({"wurfmesser", "messer", }));
            set_material("metall");
	    set_damage_type(({"stich"}));
	    min=5; max=18; grenze=35;
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
    set_weight(1+(schaden-1)/5);
    
    switch(schaden)
    {
	case 0..9:
	    set_life(calc_percent(
	        life_percent,
		({1000,750,660,590,540,500,470,450,430,410})[schaden],
		({2000,1500,1320,1180,1080,1000,940,900,860,820})[schaden]
		));
	    break;
	case 10..30: // Ab hier linear.
	    set_life(calc_percent(life_percent,500-schaden*10,1000-schaden*20));
	    break;
    }

    if(max>20) set_no_store(1);
    
    max_wert = 
        ({ /* 0- 9*/ 50000,20000,12000,7200,4300,2600,1500,900,510,280,
	   /*10-19*/   160,  110,   76,  59,  49,  43,  39, 35, 31, 28,
           /*20-30*/    25,   22,   18,  14,  11,   8,   6,  5,  4,  3,  2
	})[schaden];

    max_wert = query_life() * 50 / max_wert;
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
    
    set_value(min_wert,max_wert);
}
