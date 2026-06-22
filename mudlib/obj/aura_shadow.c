// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/obj/aura_shadow.c
// Description:
// Author:	Francis
// Modified by:	Freaky (06.02.1999) test_aggression wird nur noch in Objekten
//			Aufgerufen, die noch nicht im Kampf sind.

#include <shadow.h>
#include <message.h>
#include <add_hp.h>

#undef PREVENT_ATTACK

inherit "/i/shadow";

object owner;

#ifdef PREVENT_ATTACK
int reattack;
#endif

void init_aura(object ob, int dauer)
{
    if (!ob)
	return;
    owner = ob;
    owner->stop_command("alles");
#ifdef PREVENT_ATTACK
    reattack = owner->query_reattack();
    owner->set_reattack(0);
#endif
    init_shadow(owner);
    call_out("finish",dauer);
}

int query_has_aura() { return 1; }

void finish()
{
    if (owner)
    {
#ifdef PREVENT_ATTACK
        object env, *obs;

	owner->set_reattack(reattack);
#endif
	owner->send_message(MT_LOOK,MA_MAGIC,
	    wrap(Der(owner)+" verliert "+
		seinen((["gender":"weiblich","name":"aura"]),0,owner)+
	      " der Unantastbarkeit."),
	   "Deine Aura verschwindet.\n",owner);
#ifdef PREVENT_ATTACK
	if (env = environment(owner))
	{
	    obs = all_inventory(env);
	    for (int i = sizeof(obs); i--; )
	    	if (! obs[i]->query_in_fight())
		    obs[i]->test_aggression(owner);
	}
#endif
    }
    destruct_shadow();
}

varargs int add_hp(int hp, mapping infos)
{
    if(hp < 0 && infos && infos[AH_ATTACKER])
    {
	owner->send_message_to(owner,MT_NOTIFY,MA_MAGIC, wrap(
		"Deine Aura schützt dich teilweise vor " +
		dem(infos[AH_ATTACKER])+"."));
	return owner->add_hp(hp/2, infos);
    }
    return owner->add_hp(hp, infos);
}

// Freaky: Die Aura ist sonst zu schwach
// Garthan: nein sie ist zu gut :) 25.04.95
// Garthan: also gut mit hp/2 statt ohne Schaden...
#ifdef PREVENT_ATTACK
int attackiere_command(string str) {
    tell_object(owner,
"Durch die ungeheure Konzentration, die das Halten der Aura dir abverlangt,\n"+
"bist du nicht in der Lage einen Kampf zu beginnen.\n");
    return 1;
}

int werfe_command(string str) {
    return attackiere_command(str);
}

int schiesse_command(string str) {
    return attackiere_command(str);
}

int toggle_reattack(string str) {
    write("Im Zustand der Unantastbarkeit ist das nicht möglich.\n");
    return 1;
}
#endif
