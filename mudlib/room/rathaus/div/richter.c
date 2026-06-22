// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/room/rathaus/div/richter.c
// Description:	Friedensrichter Salomo, der Moerder von ihrer Schuld befreit.
// Author:	Francis (1993)

inherit "/i/monster/monster";

#include <deklin.h>
#include <invis.h>
#include <message.h>
#include <monster.h>
#include <time.h>

#include "rollen_weg.inc"
#ifdef UNItopia
#include "/d/Vaniorh/Tadmor/Gericht/sys/gericht.h"
#endif

// Salomo kann Tarnungen durchschauen.
#define ANREDE(ob) (ob->query_real_cap_name())

int state, per_rede;
object taeter, opfer;

private void reset_vars()
{
    state = 0;
    taeter = 0;
    opfer = 0;
    per_rede = 0;
}

private int check_vars()
{
    if(!state || !taeter || !opfer)
    {
	reset_vars();
	return 1;
    }
}

int nein_danke(object ob, object woher, object monster, object tp)
{
    if(!woher || IS_INVIS(ob))
	return 0;
    
    if(ob->query_money())
	exec_command("sage Geldstrafen können bei Baltasar bezahlt werden.");
    else
	exec_command("sage Nein danke, das brauche ich nicht.");
}

void create()
{
    ::create();
    initialize("salomo",100);
    give_hp(3000);
    give_armour_level(10);
    set_personal(1);
    set_gender("maennlich");
    set_title(", der Friedensrichter");
    set_short(0);
    set_long("Ein uralter weißhaariger Mann mit einem langen weißen Bart\n"+
	"lächelt dich verständnisvoll an.\n");
    set_align(10000);
    load_chat(2,({"Meine lieben Kinder, was kann ich für euch tun?"}));

    set_only_parse_players(1);
    set_parse_conversation(this_object(), ({
"gruss:       hallo || hi || guten && tag || grüß && gott", PARSE_SAY|PARSE_TELL|PARSE_CONTINUE,
"abschied:    tschüss || ciao || bye || auf && wiedersehen", PARSE_SAY|PARSE_TELL,
"taeter1:     ich && entschuldige && bei",        PARSE_SAY|PARSE_TELL,
"taeter2:     meine && untaten && verspreche",    PARSE_SAY|PARSE_TELL,
"taeter3:     niemals && spieler && unrecht",     PARSE_SAY|PARSE_TELL,
"taeter4:     wahr && götter && helfen",         PARSE_SAY|PARSE_TELL,
"opfer:       verzeihe && begangenen && untaten", PARSE_SAY|PARSE_TELL,
"allesandere: []",                                 PARSE_SAY|PARSE_TELL,
"taeter0:     schuld || sühne || mord || vergebung || verzeihung", PARSE_SAY|PARSE_TELL,
"opferbegin:  will,möchte && verzeihen,vergeben", PARSE_SAY|PARSE_TELL,
#ifdef UNItopia
"strafe:      geldstrafe || strafe || diebstahl || spar", PARSE_SAY,
#endif
	}));
    set_accept_objects(({#'nein_danke, #'refuse}));
    reset_vars();
}

#ifdef UNItopia
string query_finger_info_text()
{
    object env = environment();
    string str = "Er befreit Mörder von ihrer Schuld, "
		 "nachdem sie ihre Tat gesühnt haben.\n";
    
    if(!env && find_object(GERICHT_HOF))
	// Wir sind im Nirvana, der Gerichtshof ist aber geladen.
	str += "Er ist gerade nicht da.\n";
    else if(env && object_name(env) != GERICHT_HOF)
	// Wir sind woanders
	str += wrap_say("Er befindet sich an folgendem Ort:",
		add_dot_to_msg(env->query_short()));
    else
	str += "Er ist gerade im Gerichtshof von Tadmor.\n";
    
    return str;
}
#endif

void add_sp(int kosten)
{
    // Salomo hat beliebig viele ZPs.
}

private void reply(string msg, object whom, int flags, mapping info)
{
    if(!whom)
	return;

    if(flags & (PARSE_SAY|PARSE_SOUL))
    {
	if(present(whom, environment()) && !IS_INVIS(whom))
	    exec_command("sage zu",whom,msg);
	else
	    exec_command("sage",msg);
    }
    else if(flags & PARSE_TELL)
//	exec_command("rede",whom->query_real_name(),msg);
	exec_command("rede",
	    implode((({whom})+filter(info[PARSE_INFO_RECIPIENTS],#'playerp))->query_real_name(),","),
	    msg);
}

static int gruss(string str, string verb, object monster, object player, int flags, mapping info)
{
    if(flags & PARSE_TELL)
	reply((IS_NIGHT?"Hallo, ":"Einen schönen, guten Tag, ")+ANREDE(player)+".",
	    player,flags,info);
    else
	exec_command("sag Willkommen, "+ANREDE(player)+".");
}

static int abschied(string str, string verb, object monster, object player, int flags, mapping info)
{
    reply("Alles Gute und auf Wiedersehen, "+ANREDE(player)+".",
	player, flags, info);
}

static int opferbegin(string str, string verb, object monster, object player, int flags, mapping info)
{
    reply("Ein Mörder muss aber selbst um Verzeihung bitten, "
	  "wenn er sich von seiner Schuld befreien will.",
	player, flags, info);
}

static int taeter0(string str, string verb, object monster, object player, int flags, mapping info)
{
    string *opfer_liste;
    string opfer_name;
    object das_opfer;

    check_vars();

    opfer_liste = player->query_opfer();
    if (!sizeof(opfer_liste))
    {
	reply("Du hast dir doch keine Schuld aufgeladen, "+ANREDE(player)+".",
	    player, flags, info);
	return 0;
    }
    else if (sizeof(opfer_liste) == 1)
    {
	opfer_name = opfer_liste[0];
	das_opfer = find_player(opfer_name);
	
	if(!das_opfer || ((flags&PARSE_SAY) && !present(das_opfer, environment()))
		      || ((flags&PARSE_TELL) && member(info[PARSE_INFO_RECIPIENTS], das_opfer)<0))
	{
	    reply("Du hast "+capitalize(opfer_name)+" auf dem Gewissen, "+ANREDE(player)+".",
		player, flags, info);
	
	    reply("Wenn Du die Befreiung von deiner Schuld erlangen "
		  "willst, so suche dein Opfer, versöhne dich und "+
		  ((flags&PARSE_TELL)?"rede gemeinsam mit ihm zu mir."
				     :"kehre mit ihm hierher zurück."),
		  player, flags, info);
	    return 0;
	}
    }
    else
    {
	reply("Wahrlich, Du hast wirklich schwer an deiner Last zu tragen, "+ANREDE(player)+"!",
	    player, flags, info);
	
	foreach(opfer_name: opfer_liste)
	{
	    das_opfer = find_player(opfer_name);

	    if(das_opfer && (!(flags&PARSE_SAY) || present(das_opfer, environment()))
		         && (!(flags&PARSE_TELL) || member(info[PARSE_INFO_RECIPIENTS], das_opfer)>=0))
		break;
	}
	
	if (!das_opfer)
	{
	    reply("Wenn Du die Befreiung von deiner Schuld erlangen "
		  "willst, so suche deine Opfer, versöhne dich und "+
		  ((flags&PARSE_TELL)?"rede gemeinsam mit ihnen zu mir."
				     :"kehre mit ihnen hierher zurück."),
		  player, flags, info);
	    return 0;
	}
    }
    
    if (taeter && opfer && state && (per_rede || present(taeter,environment())))
    {
	if (this_player() != taeter)
	{
	    reply("Warte bitte, bis ich mit "+taeter->query_real_cap_name()+" fertig bin.",
		player, flags, info);
	    return 0;
	}
	else
	    reply("Na gut, dann beginnen wir von vorne, "+ANREDE(taeter)+".",
		player, flags, info);
    }
    
    reply("Wenn Du die Befreiung von deiner Schuld gegenüber " +
	    das_opfer->query_real_cap_name() + " erlangen willst, "+ANREDE(player)+
	    ", so sprich mir nach:", player, flags, info);

    reply("\"Ich, "+player->query_real_cap_name()+", entschuldige mich bei "+
	das_opfer->query_real_cap_name()+"\"", player, flags, info);

    opfer = das_opfer;
    taeter = player;
    state = 1;
    per_rede = (flags&PARSE_TELL);
}

private int check_tell(int flags, mapping info)
{
    if(flags&PARSE_TELL)
    {
	if(member(info[PARSE_INFO_RECIPIENTS],opfer)<0)
	{
	    reply("Bitte sprich nicht nur zu mir, sondern auch zu deinem Opfer.",
		taeter, flags, info);
	    return 1;
	}
	per_rede = 1;
    }
    else if(per_rede)
    {
	if(!present(opfer,environment()))
	{
	    reply("Bitte kehre gemeinsam mit deinem Opfer zurück oder rede zu mir und ihm.",
		taeter, flags, info);
	    return 1;
	}
	per_rede = 0;
    }
    else
	return 0;
}

static int taeter1(string str, string verb, object monster, object player, int flags, mapping info)
{
    if(check_vars() || player!=taeter || state != 1 || check_tell(flags, info))
	return PARSE_CONTINUE;

    reply("\"für meine an "+opfer->query_real_cap_name()+" begangenen Untaten und verspreche\"",
	taeter, flags, info);
    state = 2;
}

static int taeter2(string str, string verb, object monster, object player, int flags, mapping info)
{
    if(check_vars() || player!=taeter || state != 2 || check_tell(flags, info))
	return PARSE_CONTINUE;

    reply("\"niemals wieder einem anderen Spieler ein Unrecht zuzufügen,\"",
	taeter, flags, info);
    state = 3;
}

static int taeter3(string str, string verb, object monster, object player, int flags, mapping info)
{
    if(check_vars() || player!=taeter || state != 3 || check_tell(flags, info))
	return PARSE_CONTINUE;

    reply("\"so wahr mir die Götter Magyras helfen.\"",
	taeter, flags, info);
    state = 4;
}

static int taeter4(string str, string verb, object monster, object player, int flags, mapping info)
{
    if(check_vars() || player!=taeter || state != 4 || check_tell(flags, info))
	return PARSE_CONTINUE;

    if (!per_rede && !present(opfer,environment()))
    {
	reply("Schön. Aber wo ist "+opfer->query_real_cap_name()+"?",
	    taeter, flags, info);
	    
	reset_vars();
	return 0;
    }
    
    reply("Und nun, "+ANREDE(opfer)+", sprich mir nach:",
	    taeter, flags, info);
    reply("\"Ich, "+opfer->query_real_cap_name()+", verzeihe "+taeter->query_real_cap_name()+" die an mir begangenen Untaten.\"",
	    taeter, flags, info);
    state = 5;
}

private void rehabilitiert(object wer)
{
    if (!wer->query_moerder())
    {
	shout("Salomo verkündet: "+wer->query_real_cap_name()+" ist wieder rehabilitiert!\n");
	if(!this_player()->query_echomode())
	    send_message_to(this_player(), MT_NOISE, MA_COMM, 
		"Salomo verkündet: "+wer->query_real_cap_name()+" ist wieder rehabilitiert!\n");
    }
}

static int opfer(string str, string verb, object monster, object player, int flags, mapping info)
{
    if(check_vars() || player!=opfer || state != 5)
	return PARSE_CONTINUE;

    if (!per_rede && !present(taeter,environment()))
    {
	reply("Schön. Aber wo ist "+taeter->query_real_cap_name()+"?",
	    opfer, flags, info);
	reset_vars();
	return 0;
    }
    
    reply("So sei es!", opfer, flags, info);
    
    taeter->delete_opfer(opfer->query_real_name());
    sys_log("SALOMO", timestr(time())+" "+opfer->query_real_name()+
		" hat "+taeter->query_real_name()+" verziehen.\n");
#ifdef UNItopia
    GERICHT_MASTER->cancel_urteil(taeter->query_real_name(),
	opfer->query_real_name());
#endif

    rehabilitiert(taeter);
    reset_vars();
}

static int allesandere(string str, string verb, object monster, object player, int flags, mapping info)
{
    if (!check_vars())
    {
	if (player == taeter && state < 5)
	{
	    reply("Du hast die Formel nicht richtig nachgesprochen.",
		player, flags, info);
	    reset_vars();
	    return 0;
	}
	else if (player == opfer && state == 5)
	{
	    reply("Wiederhols nochmal, "+ANREDE(opfer)+".",
		player, flags, info);
	    return 0;
	}
    }
    
#ifdef UNItopia
    if(flags&PARSE_SAY)
    {
	
	string* deleted;
	
	deleted = GERICHT_MASTER->check_player(this_player());
	if(sizeof(deleted))
	{
	    exec_command("sage Da Du Deine Strafen verbüßt hast, "+
		(sizeof(deleted)>1?"seien Deine Morde an "
				  :"sei Dein Mord an ")+
		liste(map(sort_array(m_indices(mkmapping(deleted)),#'>),#'capitalize))+
		" verziehen.");
	    
	    sys_log("SALOMO", timestr(time())+" "+
		this_player()->query_real_name()+
		" hat durch Urteil verbüßt: "+implode(deleted,", ")+"\n");
	
	    rehabilitiert(this_player());
	}
	else
	    return PARSE_CONTINUE;
    }
    else
#endif
	return PARSE_CONTINUE;
}

#ifdef UNItopia
static int strafe(string str, string verb, object monster, object player, int flags, mapping info)
{
    reply("Geldstrafen kannst Du bei Baltasar in der Gerichtskasse begleichen.",
	player, flags, info);
}
#endif
