// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/i/object/gilden_ob.c
// Description:	Inherit-File fuer alle Gilden-Libraries
// Author:	Freaky (17.05.97)

#pragma save_types

#include <config.h>
#include <gilden.h>
#include <player.h>

/*
FUNKTION: query_entry
DEKLARATION: protected mapping query_entry()
BESCHREIBUNG:
Diese Funktion wird vom Gilden-Master (GILDEN_OB) aufgerufen, um alle
Informationen der Gilde abzufragen.
Welche Eintraege das Mapping haben kann, steht in /doc/funktionsweisen/gilden
GRUPPEN: spieler, gilde
*/
protected mapping query_entry();

/*
FUNKTION: player_died
DEKLARATION: protected void player_died(object player, mapping infos)
BESCHREIBUNG:
Wird aufgerufen, wenn ein Spieler gestorben ist.
Das Mapping ist das gleiche, welches an add_hp() und die() uebergeben
wurde.
VERWEISE: player_murdered, player_suicid, notify_login, add_hp
GRUPPEN: spieler, gilde
*/
protected void player_died(object player, mapping infos) {}

/*
FUNKTION: player_murdered
DEKLARATION: protected void player_murdered(object player, mixed opfer)
BESCHREIBUNG:
Wird aufgerufen, wenn ein Spieler einen anderen ermordet hat.
Wenn der Tod eintraf, nachdem der Spieler sich ausloggte, wird diese
Funktion beim naechsten Login des Spielers aufgerufen. Der real_name des
Opfers wird dann als String uebergeben, anstatt als Objekt, wie es sonst
der Fall ist.
VERWEISE: player_died, player_suicid, notify_login, add_hp
GRUPPEN: spieler, gilde
*/
protected void player_murdered(object player, mixed opfer) {}

/*
FUNKTION: leave_gilde
DEKLARATION: protected void leave_gilde(object player)
BESCHREIBUNG:
Wird aufgerufen, wenn ein Spieler aus der Gilde austritt.
VERWEISE: player_suicid, enter_gilde, player_new_rang
GRUPPEN: spieler, gilde
*/
protected void leave_gilde(object player) {}

/*
FUNKTION: enter_gilde
DEKLARATION: protected void enter_gilde(object player)
BESCHREIBUNG:
Wird aufgerufen, wenn ein Spieler in die Gilde eintritt.
VERWEISE: player_suicid, leave_gilde, player_new_rang
GRUPPEN: spieler, gilde
*/
protected void enter_gilde(object player) {}

/*
FUNKTION: player_new_rang
DEKLARATION: protected void player_new_rang(object player, int old_rang, int new_rang)
BESCHREIBUNG:
Wird aufgerufen, wenn ein Spieler einen neuen Rang in der Gilde erhaelt.
VERWEISE: player_suicid, leave_gilde, enter_gilde
GRUPPEN: spieler, gilde
*/
protected void player_new_rang(object player, int old_rang, int new_rang) {}

/*
FUNKTION: player_suicid
DEKLARATION: protected void player_suicid(string name)
BESCHREIBUNG:
Wird aufgerufen, wenn ein Spieler sich suizidet hat, oder vom System
geloescht wird. Der Spieler muss dabei nicht anwesend sein.
VERWEISE: enter_gilde, leave_gilde, player_died, player_murdered, notify_login
GRUPPEN: spieler, gilde
*/
protected void player_suicid(string name) {}

/*
NOENZY: notify_login (veraltet, s.u.)
DEKLARATION: protected void notify_login(object player, int flag)
BESCHREIBUNG:
Wird aufgerufen, wenn ein Spieler sich eingeloggt hat.
Flag:
  0 : Spieler loggt sich ein (Es wurde ein neuer Spieler erzeugt)
  1 : Spieler war Statue (reconnect)
VERWEISE: player_died, player_murdered, player_suicid, notify_quit
GRUPPEN: spieler, gilde
*/
/*
FUNKTION: notify_login
DEKLARATION: protected void notify_login(object player, int flag, int mode)
BESCHREIBUNG:
Wird aufgerufen, wenn ein Spieler sich eingeloggt hat.
Fuer Flag werden folgende Werte aus /sys/player.h genutzt:
  NL_LOGON  : Spieler loggt sich ein
  NL_STATUE : Spieler war nur Statue
Fuer Mode werden folgende Werte aus /sys/player.h verwendet:
  NL_DEFAULT        Normaler Logon.
  NL_ARMAGEDDON     Der erste Login nach einem Armageddon.
  NL_STADT          Der Spieler hat sich vorher mit ende stadt ausgeloggt.
  NL_START          Der Spieler hat sich vorher mit ende start ausgeloggt.
  NL_PORTAL_ENTER   Ein Spieler aus einem anderen Mud betritt dieses Mud.
  NL_PORTAL_REENTER Ein Spieler kehrt in dieses Mud zurueck.
VERWEISE: player_died, player_murdered, player_suicid, notify_quit
GRUPPEN: spieler, gilde
*/
#ifdef PLAYER_NOTIFY_MODES
protected void notify_login(object player, int flag, int mode) {}
#else
protected void notify_login(object player, int flag) {}
#endif

/*
NoENZY: notify_quit (veraltet, s.u.)
DEKLARATION: protected void notify_quit(object player, int flag)
BESCHREIBUNG:
Wird aufgerufen, wenn ein Spieler sich ausgeloggt hat.
Flag:
  0 : Spieler hat 'ende' eingegeben
  1 : Spieler war netztot und zerfaellt jetzt zu Staub
  2 : Spieler hat sich suizidet
VERWEISE: player_died, player_murdered, player_suicid, notify_login
GRUPPEN: spieler, gilde
*/
/*
FUNKTION: notify_quit
DEKLARATION: protected void notify_quit(object player, int flag, int mode)
BESCHREIBUNG:
Wird aufgerufen, wenn ein Spieler sich ausgeloggt hat.
Fuer Flag werden folgende Werte aus /sys/player.h genutzt:
 NQ_ENDE    : Spieler hat 'ende' eingegeben
 NQ_NETZTOT : Spieler war netztot und zerfaellt jetzt zu Staub
 NQ_SUIZID  : Spieler hat sich suizidet
Fuer Mode werden folgende Werte aus /sys/player.h verwendet:
  NQ_KEIN_ENDE     : Ein systemseitiges Beenden (siehe flag)
  NQ_ENDE_BLANK    : Ende wurde ohne Parameter angegeben.
  NQ_ENDE_STADT    : ende stadt wurde angegeben
  NQ_ENDE_START    : ende start wurde angegeben
  NQ_PORTAL_LEAVE  : Ein Spieler aus einem anderen Mud verlaesst dieses Mud.
  NQ_PORTAL_TRAVEL : Ein spieler aus diesem Mud reist in ein anderes Mud.
VERWEISE: player_died, player_murdered, player_suicid, notify_login
GRUPPEN: spieler, gilde
*/
#ifdef PLAYER_NOTIFY_MODES
protected void notify_quit(object player, int flag, int mode) {}
#else
protected void notify_quit(object player, int flag) {}
#endif
/*
NOENZY: notify_net_dead (veraltet, siehe unten)
DEKLARATION: protected void notify_net_dead(object player, int flag)
BESCHREIBUNG:
Wird aufgerufen, wenn die Verbindung unterbrochen wurde.
Flag:
  0: Spieler wird zur Statue
Bisher gibt es keine weiteren Beteutungen fuer flag.
VERWEISE: player_died, player_murdered, player_suicid, notify_login
GRUPPEN: spieler, gilde
*/
/*
FUNKTION: notify_net_dead
DEKLARATION: void notify_net_dead(object who, int flag, int mode)
BESCHREIBUNG:
Wird aufgerufen, wenn die Verbindung unterbrochen wurde. flag und mode werden 
derzeit mit dem Parameter NND_DEFAULT (=0, /sys/player.h) vorbelegt.
VERWEISE: notify_quit, notify_login, player_suicid
GRUPPEN: spieler
*/
#ifdef PLAYER_NOTIFY_MODES
protected void notify_net_dead(object player, int flag, int mode) {}
#else
protected void notify_net_dead(object player, int flag) {}
#endif

/*
FUNKTION: query_erf_gilde_available
DEKLARATION: int query_erf_gilde_available()
BESCHREIBUNG:
Liefert diese Funktion 1, dann wird dem Spieler beim Befehl 'erfahrung'
noch die Option 'gilde' angeboten. Wenn der Spieler dann
'erfahrung gilde' eingibt wird query_erf_gilde(SPIELER) aufgerufen und
das Ergebnis im more angezeigt.
VERWEISE: query_erf_gilde, query_erf_gilde_options
GRUPPEN: spieler, gilde
*/

/*
FUNKTION: query_erf_gilde
DEKLARATION: string *query_erf_gilde(object who, string rest)
BESCHREIBUNG:
Diese Funktion wird abgefragt, wenn ein Spieler 'erfahrung gilde <rest>'
eingibt. Das Ergebnis muss ein String-Array sein, das dann dem Spieler per
more angezeigt wird.
Das ganze funktioniert nur, wenn query_erf_gilde_available() 1 liefert.
Moegliche Parameter von 'erfahrung gilde' (also das was in rest geliefert wird)
kann mittels query_erf_gilde_options() in die Hilfe von 'erfahrung' aufgenommen
werden.
VERWEISE: query_erf_gilde_available, query_erf_gilde_options
GRUPPEN: spieler, gilde
*/

/*
FUNKTION: query_erf_gilde_options
DEKLARATION: string query_erf_gilde_options()
BESCHREIBUNG:
Diese Funktion wird abgefragt, wenn die Kurzhilfe von 'erfahrung' angezeigt
werden soll. Der zurueckgelieferte Text wird dort hinter 'Gilde' angezeigt
und sollte alle moeglichen Optionen fuer 'erfahrung gilde' auflisten.
Die tatsaechlich angegebenen Optionen werden query_erf_gilde dann
als 2. Parameter geliefert.
VERWEISE: query_erf_gilde_available, query_erf_gilde
GRUPPEN: spieler, gilde
*/

int query_prevent_shadow(object ob)
{
    return 1;
}

void do_player_died(object player, mapping infos)
{
    if (playerp(previous_object()))
    	player_died(player, infos);
}

void do_player_murdered(object player, mixed opfer)
{
    if (playerp(previous_object()))
    	player_murdered(player,opfer);
}

#ifdef PLAYER_NOTIFY_MODES
void do_notify_login(object player, int flag, int mode)
{
    if (playerp(previous_object()))
    	notify_login(player,flag,mode);
}

void do_notify_quit(object player, int flag, int mode)
{
    if (playerp(previous_object()))
    	notify_quit(player,flag,mode);
}

void do_notify_net_dead(object player, int flag, int mode)
{
    if (playerp(previous_object()))
    	notify_net_dead(player,flag, mode);
}
#else
void do_notify_login(object player, int flag)
{
    if (playerp(previous_object()))
    	notify_login(player,flag);
}

void do_notify_quit(object player, int flag)
{
    if (playerp(previous_object()))
    	notify_quit(player,flag);
}

void do_notify_net_dead(object player, int flag)
{
    if (playerp(previous_object()))
    	notify_net_dead(player,flag);
}
#endif

void do_player_suicid(string name)
{
    if (object_name(previous_object()) == PLAYER_DELETER)
    	player_suicid(name);
}

void do_enter_gilde(object player)
{
    if (playerp(previous_object()))
    	enter_gilde(player);
}

void do_leave_gilde(object player)
{
    if (playerp(previous_object()))
    	leave_gilde(player);
}

void do_player_new_rang(object player, int old_rang, int new_rang)
{
    if (playerp(previous_object()))
    	player_new_rang(player,old_rang,new_rang);
}

mapping do_query_entry()
{
    if (object_name(previous_object()) == GILDEN_OB)
    	return query_entry();
}

void create()
{
    call_out(#'call_other, 1, GILDEN_OB, "update_entry", this_object());
}
