// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/secure/delete_player.c
// Description:	Loescht einen Spieler aus dem System
// Author:	Freaky (24.01.97)
// Modified by:	Copper (28.7.97) player_suicid Aufruf in do_player_suicid
//		Parsec (27.10.1998) Gilden-Skill-Ob benachrichtigen
//		Freaky (08.02.1999) player-container zerstoeren

#pragma no_inherit

#ifdef UNItopia
#include "/z/Schiffe/sys/export.h"
#endif

#include <config.h>
#include <level.h>
#include <gilden.h>
#include <apps.h>
#include <mail.h>
#include <error.h>
#include <message.h>
#include <uids.h>

int skill;
string gilde;
mapping suizid = ([]);

// wie lange dauert es zwischen der Suizid-Befehlseingabe und
// dem Suizid? Eine Woche:
#define SUICIDE_TIME 604800

#define SAVE_FILE "/var/adm/suicid"

void create()
{
    restore_object (SAVE_FILE);
    if (find_call_out ("test_suicides") == -1)
        call_out ("test_suicides",60);
}

private void save ()
{
    save_object (SAVE_FILE);
}

int secure()
{
    if (!previous_object())
    	return 0;
    if (playerp(previous_object()))
    	return 1;
    if (geteuid(previous_object()) == ROOT_UID)
        return 1;
    if (adminp(this_player()) && // this_player() == this_interactive() &&
	    geteuid(previous_object()) == geteuid(this_player()))
	return 1;
}

private int do_suicide (string real_name)
{
    object ob;
    
    sys_log("SUIZID",ctime()+"  "+real_name+"\n");
        
    // test_players = SECOND->query_testplayers_of(real_name);
    SECOND->delete_testplayer(real_name);
    PLAYER_SECOND->delete_second_char(real_name);
    LEVEL_LISTER->remove_entry(real_name);
    MAILD->erase_mailfolder(real_name);
    GILDENSKILL_OB->mark_suicide(real_name);
    if (GOETTER_REGISTER->is_wiz(real_name))
    {
    	GOETTER_REGISTER->banish_wiz(real_name);
	BANISHD->banish(real_name, 0, real_name);
	DOMAIN_INFOS->remove_wiz(real_name);
    }
    // Ex-Goetter auch sperren.
    else if(GOETTER_REGISTER->query_banished_wiz(real_name))
	BANISHD->banish(real_name, 0, real_name);

#ifdef UNItopia
    catch(WERFT_BUERO->wird_suicide(real_name);publish);
#endif

    // Auf 0 setzen, damit das restore_object richtig klappt.
    skill = 0;
    gilde = 0;
    // Spieler restoren, um skill und gilde zu erfahren
    restore_object(PLAYER_FILE(real_name));

    // Wenn der Spieler ueber 4500 EP hat: sichern
    if (skill < 4500)
    	rm(PLAYER_FILE(real_name) + ".o");
    else
    	rename(PLAYER_FILE(real_name) + ".o",
	    PLAYER_SUICID_LOCATION + "/" + real_name + "_" + time() + ".o");

    if (gilde)
    {
	// Den Gildenmaster informieren
	catch((GILDEN_OB->query_one_gilden_info(gilde,FILE_NAME))->do_player_suicid(real_name);publish);
    }

    // Jetzt noch den Container mit den Objekten des Spielers zerstoeren
    if (ob = find_object(PLAYER_INVENTORY_CONTAINER + real_name))
	ob->remove();

    // Wer will sonst noch mitbekommen, dass ein Spieler nicht mehr existiert?
    CONTROL->notify("delete_player", real_name);
    return 1;
}

/*
FUNKTION: notify_delete_player
DEKLARATION: void notify_delete_player(string real_name)
BESCHREIBUNG:
Wenn ein Spieler (kein Gast) geloescht wird, dann wird
CONTROL->notify("delete_player", real_name) aufgerufen.
notify ruft dann in allen mit CONTROL->add_controller(
"notify_delete_player", other) angemeldeten Blueprints other
die Funktion other->notify_delete_player(real_name) auf.

CONTROL ist ein Define aus <apps.h>
VERWEISE: notify, add_control,
GRUPPEN: spieler
*/
int delete_player(string real_name)
{
    if (playerp(previous_object()))
    {
	real_name = previous_object()->query_real_name();
    }
    else if (extern_call())
    {
	if (!stringp(real_name))
	    return 0;

	if (!secure())
	{
	    write("So nicht.\n");
	    return 0;
	}

	if (find_player(real_name))
	{
	    write("Der Spieler ist noch am spielen.\n");
	    return 0;
	}

	if (!player_exists(real_name))
	{
	    write("Einen Spieler mit diesem Namen gibt es nicht.\n");
	    return 0;
	}
    }
    else
    {
        write("Geht noch nicht.\n");
	return 0;
    }
    return do_suicide (real_name);
}


void suicide ()
{
    if (!previous_object() || !playerp (previous_object())) return;
    suizid [previous_object()->query_real_name()] = time ();
    if (find_call_out ("test_suicides") == -1)
        call_out ("test_suicides", SUICIDE_TIME + 4);
    save ();
}

void unsuicide ()
{
    if (!previous_object() || !playerp (previous_object())) return;
    if (member (suizid, previous_object()->query_real_name())) {
        m_delete (suizid, previous_object()->query_real_name());
        save ();
        call_out ("unsuicide_message", 10, previous_object());
    }
}

void unsuicide_message (object wer)
{
    if (wer) {
        wer->send_message_to (wer,MT_LOOK,MA_MOVE_IN,
            "Der Tod erscheint plötzlich.\n");
        wer->send_message_to (wer,MT_NOISE,MA_COMM,
            "Der Tod sagt: SOSO, DU HAST ES DIR ALSO ANDERS ÜBERLEGT.\n"
            "              DAS FREUT MICH SEHR.\n");
        wer->send_message_to (wer,MT_LOOK|MT_FEEL,MA_FEEL,
            "Du meinst, den Tod lächeln zu sehen; aber das ist natürlich "
            "Unsinn.\n");
        wer->send_message_to (wer,MT_NOISE,MA_COMM,
            "Der Tod sagt: DAS ERSPART MIR EINE GANZE MENGE ARBEIT.\n");
        wer->send_message_to (wer,MT_LOOK,MA_MOVE_OUT,
            "Der Tod verschwindet wieder genauso plötzlich wie "
            "er erschienen ist.\n");
    }
}

void test_suicides ()
{
    mixed *candi;
    int i, next;
    next = 0;
    candi = m_indices (suizid);
    for (i = sizeof (candi); i--; ) {
        if (find_player (candi[i]) || !player_exists (candi[i]))
            m_delete (suizid, candi[i]);
        else {
            if (suizid[candi[i]] + SUICIDE_TIME <= time ()) {
                do_suicide (candi[i]);
                m_delete (suizid, candi[i]);
            }
            else if (!next || (suizid[candi[i]] < next))
                next = suizid[candi[i]];
        }
    }
    if (next && (find_call_out ("test_suicides") == -1))
        call_out ("test_suicides",next + SUICIDE_TIME - time ());
    save ();
}

/*
mapping query_suizid ()
{
    return suizid;
}
*/
