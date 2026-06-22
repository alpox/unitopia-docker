// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/i/living/hands.c
// Description: beinhaltet Nehmen, Geben, Legen, Kampf, Fuehren, Senken, hp
// Modified by:	Garthan	(16.07.95) combat_message
// Modified by:	Freaky (25.09.95) put_command() neu geschrieben
// Modified by: Deka (22.11.95) add_gone_ob, delete_gone_ob, query_gone_ob
//		delete_enemy, handle_attack geaendert (bei GEGNER NOCH DA?
// Modified by: Freaky (23.11.95) kleinere tunings in funs und Doku repariert
// Modified by:	Freaky (26.11.95) countob eingebaut, take und put returned
//				  jetzt endlich richtig.
//		Garthan (06.03.96) show_command grammatikalisch verbessert
//		Garthan (18.03.96) Ruestungen gehen je nach schaden kaputt
//		Garthan (19.03.96) add_hp hat einen dritten Parameter...
//                                 critical hit support in combat_mess
//		Garthan (15.04.96) unsichtbaren sachen nicht mehr schiebbar
//		Garthan (22.04.96) ueberfall test bei angriff ans ende 
//              Sissi   (27.04.96) mit set_not_moved_reason gesetzte Meldungen
//                                 den Spielern anzeigen
//              Sissi   (24.05.96) Kampf Verlangsamung eingebaut
//              Sissi   (31.05.96) Monster merken sich Spieler, die sie
//                                 angreifen (sind ja nich bloede)
//		Monty	(07.06 96) in add_hp() die via call_other(), da
//				   Thor in den Vampyren die() shadowen muss.
//              Sissi	(12.07.96) push_command benutzt 
//                                 query_locked_reason
//		Garthan	(16.07.96) show_command: swap ob, pl, wenn sinnvoll.
//              Sissi   (12.11.96) "attack" und "attack_msg" in v-items
//              Kurdel  (19.02.97) fuehren/senken mehrerer Waffen auf einmal,
//                                 nicht nur die erste Waffe versucht
//              Sissi   (24.02.97) Fluchtmodus variabel einstellbar
//              Kurdel  (24.04.97) v_items als Waffen (natural#weapon)
//              Sissi   (02.05.97) (be-)fuehlen, (be-)tasten, beruehren,
//                                 jemanden an etwas schnuppern, horchen,
//                                 fuehlen lassen
//              Sissi   (22.05.97) jemanden an etwas fuehlen, riechen, horchen
//                                 lassen wieder ausgebaut
//              Sissi   (30.07.97) schiesse <geschoss> auf <ziel> geht jetzt
//                                 statt schiesse <schusswaffe> auf <ziel>.
//              Jesaia  (26.10.97) extended_hp_view
//		Freaky  (10.03.98) message auf send_message umgebaut.
//		Sissi	(03.04.98) Schluesselbundlogik ("haenge...")
//		Sissi	(11.05.98) "stelle..." fuer put_command
//		Sissi	(29.05.98) query_put/take_messages
//              Sissi   (16.12.98) "schenke" fuer put_command
//		Freaky  (12.01.99) query_in_fight mit Parameter
//		Parsec  (19.01.99) show_command komplett neu: 4. Partner
//                                 beruecksichigen, 'dir', 'dich' in messages,
//                                 vitems an Lebenden Objekten zeigen, forbidden
//		Freaky (23.04.1999) push_command(): move(rdir) anstelle
//				    move(target_ob)
//
//              Jesaia (20.03.2000) Invis=7 im kampf unmoeglich
//		Freaky (29.03.2000) attack() ADD_ATTACK_LIST global weg.
//              Sissi  (05.06.2000) catch_command -> wear_command aus
//                                  Kleidung hierher verschoben;
//                                  bessere Formatierung der Meldungen;
//                                  ausserdem Meldungen ohne Adjektive

#pragma save_types
#pragma strong_types

#include <config.h>
#include <deklin.h>
#include <stats.h>
#include <move.h>
#include <parse_com.h>
#include <properties.h>
#include <invis.h>
#include <hlp.h>
#include <time.h>
#include <add_hp.h>
#include <message.h>
#include <landschaft.h>
#include <hpspview.h>
#include <notify_fail.h>
#include <level.h>
#include <error.h>
#include <simul_efuns.h>
#include <fight_options.h>
#include <control.h>

#ifdef TestMUD
#include <apps.h>
// #define DEBUGGER "myonara"
// #include <debug.h>
#define DEBUG(x)
#define FIGHT_X_DEBUG(fun,msg) CONTROL->notify("fight_debug", \
                    this_object(), fun, msg) 
#else
#define FIGHT_X_DEBUG(fun,msg)
#define DEBUG(x)
#endif

#define MIN_EVAL 100000
#define CHECK_EVAL_COST \
	if (get_eval_cost() < MIN_EVAL) \
	{ \
	    notify_message("Uff... Soviel auf einmal schaffst du nicht.\n"); \
	    return 1; \
	}
#define CHECK_KAEMPFEN_VERBOTEN \
    {	mixed k_tmp; \
	if ((k_tmp=environment()) && (k_tmp=k_tmp->query_type("kaempfen_verboten"))) { \
	    if (!stringp(k_tmp)) \
		k_tmp="Hier sind keine Kaempfe erlaubt.\n"; \
	    notify_fail(wrap(k_tmp)); \
	    return 0; \
	    } \
    }

// Prototypes:
static void die(mapping infos);
nomask int query_wiz_level();
int forbidden(string message, varargs mixed data);
void notify(string message, varargs mixed data);
void do_notifies(int flags, string message, mixed *postfixes, mixed *obs, varargs mixed *params);
string query_sp_short_name();
nomask string query_real_name();
varargs void notify_message(string msg, int type);

#define CAP(x) capitalize(x)
#define FAIL(x) return notify_fail(x)
#define TEST_FAIL(x) do { if (query_notify_fail()) return 0; else return notify_fail(x); } while(0)

#ifdef TestMUD
#define PRIVATE private
#else
#define PRIVATE private
#endif

PRIVATE static object *hand_objects = ({ 0, 0 });
PRIVATE static object *hand_enemies = ({ 0, 0 });
PRIVATE static int    *hand_hits    = ({ 0, 0 });

private static object *armours = ({});

private static mapping fight_options = ([]);

/* Hands */
private static int num_hands  =  2;
private static int min_damage =  3;
private static int max_damage = 10;
PRIVATE static int just_got_visible = 0;

private static int damage;
PRIVATE static int current_hand;
private static int lost_hp;
#if FIGHT_SPEED > 1
private static int fight_count; // Kampfgeschwindigkeit
#endif

private static object *gone_ob; // Diese werden bei Abwesenheit des
			// Gegners darueber informiert

private mapping damage_percentages;

private int reattack;
private int likes_attacks;
private int whimpy;
private int hp_view;
private int hp;
private int max_hp;
private int sum_hp;
private int short_combat_msg;

private static int extended_hp_view;

// Die Wahrscheinlichkeit fuer den Schutzengel ({Spieler, unsterbliche Engel})
private int *probability_for_guardian_angel = ({1,99});

// Variablen fuers Beschuetzen
nosave object protectee;
nosave object* caught_attackers;
nosave string* protected_ids;

/* ==================== Allgemeine Hilfsfunktionen ==============*/

#define notify_fail my_notify_fail
private varargs int my_notify_fail(mixed msg, int prio)
{
    // Die Prioritaet um eine Stufe herabsetzen.
    // Damit geben wir bereits gesetzten Meldungen der gleichen Prioritaet
    // Vorrang. (Ist also so, als ab diese Meldung mit dieser Prioritaet
    // als allererstes gesetzt wurde.)
    return funcall(symbol_function("notify_fail"), msg,
	(prio==FAIL_NOT_CMD)?prio:(prio-1));
}

string wrap_if_not_empty (string s)
{
    return s == "" ? s : wrap (s);
}
    
/* ====================  S E T T I N G S =============== */

#ifdef FIGHT_DEBUG
   private static object debugger;

   void set_debugger(object person) { debugger = person; }
   object query_debugger()          { return debugger; }
#endif

void add_sum_hp(int i) { sum_hp = to_int(sum_hp + i); }
int query_sum_hp() { return sum_hp; }

/*
FUNKTION: query_max_hp
DEKLARATION: int query_max_hp()
BESCHREIBUNG:
Liefert die maximalen Ausdauerpunkte eines Monsters oder Spielers zurueck.
VERWEISE: set_max_hp, set_hp, query_hp
GRUPPEN: spieler, monster, kampf
*/
int query_max_hp() { return max_hp; }

/*
FUNKTION: query_hp
DEKLARATION: int query_hp()
BESCHREIBUNG:
Liefert die momentanen Ausdauerpunkte eines Monsters oder Spielers zurueck.
VERWEISE: query_max_hp, set_max_hp, set_hp, query_hp
GRUPPEN: spieler, monster, kampf
*/
int query_hp()     { return hp; }

/*
FUNKTION: set_hp_view
DEKLARATION: void set_hp_view(int hp_view)
BESCHREIBUNG:
Schaltet die AP-Anzeige eines Spielers (oder Monsters, aber das lassen wir
doch lieber bleiben) ein oder aus.
Fuer die Bedeutung der Werte siehe /sys/hpspview.h, diese Werte werden
bei set_hp_view verodert.
BEISPIEL:
    set_hp_view (HP_SP_VIEW_HP_PLUS | HP_SP_VIEW_SP_MINUS)
    schaltet Anzeige bei HP Gewinn und SP Verlust ein.
Mit 0 wird die automatische Anzeige ausgeschaltet.
VERWEISE: set_hp_view
GRUPPEN: spieler, haende, kampf
*/
void set_hp_view(int i) { hp_view = i; }

/*
FUNKTION: query_hp_view
DEKLARATION: int query_hp_view()
BESCHREIBUNG:
Liefert zuruck, ob und wie die AP-Anzeige eingeschaltet ist.
Rueckgabewert ist eine Veroderung der eingeschalteten Funktionen
aus /sys/hpspview.h. Siehe auch set_hp_view.
VERWEISE: set_hp_view
GRUPPEN: spieler, haende, kampf
*/
int query_hp_view()     { return hp_view; }

/*
FUNKTION: set_extended_hp_view
DEKLARATION: void set_extended_hp_view(int extended_hp_view)
BESCHREIBUNG:
Schaltet die erweiterte AP-Anzeige eines Spielers an oder aus.
Mit 1 wird eingeschaltet, mit 0 aus.
VERWEISE: query_extended_hp_view, set_hp_view
GRUPPEN: spieler, haende, kampf
*/
void set_extended_hp_view(int i)
{
  extended_hp_view = i ? 1 : 0;
}

/*
FUNKTION: query_extended_hp_view
DEKLARATION: int query_extended_hp_view()
BESCHREIBUNG:
Liefert zurueck, ob die erweiterte AP-Anzeige eingeschaltet ist.
VERWEISE: set_extended_hp_view, set_hp_view
GRUPPEN: spieler, haende, kampf
*/
int query_extended_hp_view() { return extended_hp_view; }


/*
FUNKTION: query_num_hands
DEKLARATION: int query_num_hands()
BESCHREIBUNG:
Liefert die Anzahl der Haende eines Monsters (oder Spielers (!?!)) zurueck.
VERWEISE: set_num_hands 
GRUPPEN: spieler, monster, kampf

*/
int query_num_hands() { return num_hands; }

/*
FUNKTION: set_num_hands
DEKLARATION: void set_num_hands(int num_hands)
BESCHREIBUNG:
Damit kann man die Anzahl der Haende eines Monsters (oder Spielers (!?!)) 
setzen.
VERWEISE: query_num_hands
GRUPPEN: spieler, monster, kampf
*/
void set_num_hands(int a)
{
    object *old_hand_objects, *old_hand_enemies;
    int *old_hand_hits;
    int old_num_hands;
    int i;
    if (!a || (num_hands == a))
        return;
    old_hand_objects = hand_objects;
    old_hand_enemies = hand_enemies;
    old_hand_hits = hand_hits;
    old_num_hands = num_hands;
    if (a > 0)
    {
        hand_objects = allocate(a);
        hand_enemies = allocate(a);
        hand_hits = allocate(a);
        num_hands = a;
        for (i = 0; i < old_num_hands; i++)
            if (i < a)
            {
                hand_objects[i] = old_hand_objects[i];
                hand_enemies[i] = old_hand_enemies[i];
                hand_hits[i]    = old_hand_hits[i];
            }
            else if (old_hand_objects[i])
            {
                old_hand_objects[i]->do_remove();
                notify_message(wrap(old_hand_objects[i]->query_unwield_msg()||
                    ("Du senkst "+deinen(old_hand_objects[i])+".")),
                MA_UNWIELD);
            }
    }
}

/*
FUNKTION: set_min_damage
DEKLARATION: void set_min_damage(int min_damage)
BESCHREIBUNG:
Setzt den minimalen (bei 0 skill,offensiv,haende...) Handkampfwert eines 
Monsters/Spielers.
VERWEISE: set_max_damage, query_min_damage, query_max_damage
GRUPPEN: spieler, monster, kampf, haende
*/
void set_min_damage(int a) { if (a > 0) min_damage = a; }

/*
FUNKTION: set_max_damage
DEKLARATION: void set_max_damage(int max_damage)
BESCHREIBUNG:
Setzt den maximalen (bei 4500 skill,offensiv,haende...) Handkampfwert eines
Monsters/Spielers.
VERWEISE: set_min_damage, query_min_damage, query_max_damage
GRUPPEN: spieler, monster, kampf, haende
*/
void set_max_damage(int a) { if (a > 0) max_damage = a; }

/*
FUNKTION: query_min_damage
DEKLARATION: int query_min_damage()
BESCHREIBUNG:
Liefert den minimalen Handkampfwert eines Monsters/Spielers zurueck.
VERWEISE: set_min_damage, set_max_damage, query_max_damage
GRUPPEN: spieler, monster, kampf, haende
*/
int query_min_damage() { return min_damage; }

/*
FUNKTION: query_max_damage
DEKLARATION: int query_max_damage()
BESCHREIBUNG:
Liefert den maximalen Handkampfwert eines Monsters/Spielers zurueck.
VERWEISE: set_min_damage, set_max_damage, query_min_damage
GRUPPEN: spieler, monster, kampf, haende
*/
int query_max_damage() { return max_damage; }

/*
FUNKTION: set_whimpy
DEKLARATION: void set_whimpy(int whimpy)
BESCHREIBUNG:
Schaltet den Fluchtmodus eines Spielers/Monsters(!) an oder aus: Bei whimpy=0
wird der Fluchtmodus ausgeschaltet, bei whimpy>0 auf whimpy APs gesetzt.
VERWEISE: query_whimpy
GRUPPEN: spieler, monster, kampf
*/
void set_whimpy(int i)
{
    if (i >= 0)
   	whimpy = i;
}

/*
FUNKTION: set_reattack
DEKLARATION: void set_reattack(int reattack)
BESCHREIBUNG:
Setzt den Verteidigungsmodus eines Spielers/Monsters. Als 'reattack' koennen
folgende (in add_hp definierte) Werte angegeben werden:

    REATTACK_DONT		Verteidigungsmodus ausschalten.
    REATTACK_ONLY_SELF_DEFENSE	Nur bei Selbstverteidigung einschalten.
				(Bei NPCs entspricht dies dem REATTACK_ALWAYS.)
    REATTACK_ALWAYS		Verteidigungsmodus generell einschalten.

VERWEISE: query_reattack
GRUPPEN: spieler, monster, kampf
*/
void set_reattack(int i)
{
    reattack = i;
}

/*
FUNKTION: query_whimpy
DEKLARATION: int query_whimpy()
BESCHREIBUNG:
Fragt ab, ob der Fluchtmodus eines Spielers/Monsters gesetzt ist.
Bei Returnwert 0 ist er es nicht.
VERWEISE: set_whimpy
GRUPPEN: spieler, monster, kampf
*/
int query_whimpy()   { return whimpy; }

/*
FUNKTION: query_reattack
DEKLARATION: int query_reattack()
BESCHREIBUNG:
Fragt ab, ob der Verteidigungs-Modus eines Spielers/Monsters gesetzt ist.
Bei Returnwert 0 ist er es nicht.
VERWEISE: set_reattack
GRUPPEN: spieler, monster, kampf
*/
int query_reattack() { return reattack; }

/*
FUNKTION: set_wants_to_get_attacked_by_monsters
DEKLARATION: void set_wants_to_get_attacked_by_monsters(int i)
BESCHREIBUNG:
Hiermit kann man die Götter-Einstellung
  Aggressive Monster greifen an
setzen.
VERWEISE: query_wants_to_get_attacked_by_monsters
GRUPPEN: spieler, monster, kampf
*/
void set_wants_to_get_attacked_by_monsters(int i)
{
    likes_attacks = i ? 1 : 0;
}

/*
FUNKTION: query_wants_to_get_attacked_by_monsters
DEKLARATION: int query_wants_to_get_attacked_by_monsters()
BESCHREIBUNG:
Hiermit kann man die Götter-Einstellung
  Aggressive Monster greifen an
abfragen.
VERWEISE: set_wants_to_get_attacked_by_monsters
GRUPPEN: spieler, monster, kampf
*/
int query_wants_to_get_attacked_by_monsters ()
{
    return likes_attacks;
}

void set_short_combat_msg(int i)
{
   short_combat_msg = i ? 1 : 0;
}

int query_short_combat_msg()
{
   return short_combat_msg;
}

protected int is_aggressive_against(object player) {}

/*
FUNKTION: query_damage_percentages
DEKLARATION: mapping query_damage_percentages()
BESCHREIBUNG:
Liefert das mit set/add_damage_percentages gesetzte Mapping zurueck.
VERWEISE: set_damage_percentages, add_damage_percentages, add_hp
GRUPPEN: monster, kampf
*/
mapping query_damage_percentages()
{
    return damage_percentages || ([:1]);
}

/*
FUNKTION: add_damage_percentages
DEKLARATION: void add_damage_percentages(mapping damage_percentages)
BESCHREIBUNG:
Damit kann man weitere Resistenzen/Empfindlichkeiten denen mit
set_damage_percentages gesetzten hinzufuegen.
VERWEISE: set_damage_percentages, query_damage_percentages, add_hp
GRUPPEN: monster, kampf
*/
void add_damage_percentages(mapping dp)
{
    if(!damage_percentages)
	damage_percentages = dp;
    else
	damage_percentages += dp;
}

/*
FUNKTION: set_damage_percentages
DEKLARATION: void set_damage_percentages(mapping damage_percentages)
BESCHREIBUNG:
Mit dieser Funktion kann man besondere Resistenzen oder Empfindlichkeiten
gegenueber bestimmten Schadenstypen angeben. Im uebergebenen Mapping
wird dazu jedem Schadenstyp ein Faktor (in Prozent) angegeben, mit welchen
ein solcher Schaden multipliziert wird. Ein Eintrag von 100 ist der
Standardwert (entspricht der Empfindlichkeit von Menschen), ein
Eintrag < 100 verringert den Schaden, > 100 vergroessert ihn. 

Beispiel: ([ "heilig": 200, "daemonisch": 50 ]).

Es sollte auch bei hartnaeckigen Resistenzen eine Chance geben, dass
Schaeden durchkommen. Faktoren < 10% sollten daher vermieden werden.
Es gibt auch ein Schluessel "default" fuer alles uebrige.
VERWEISE: query_damage_percentages, add_damage_percentages, add_hp
GRUPPEN: monster, kampf
*/
void set_damage_percentages(mapping dp)
{
    damage_percentages = dp;
}

/*
FUNKTION: set_fight_options
DEKLARATION: nomask void set_fight_options(mapping fo)
BESCHREIBUNG:
Damit kann man die Optionen zum Kampf setzen, nicht bei Spielern aktiv. 
Einzelheiten siehe add_fight_option.
VERWEISE: add_fight_option,query_fight_option,query_fight_options
GRUPPEN: monster, kampf
*/
nomask void set_fight_options(mapping fo)
{
    if (mappingp(fo) && !playerp(this_object()))
    {
        fight_options = fo;
    }
}

/*
FUNKTION: add_fight_option
DEKLARATION: nomask void add_fight_option(string key, mixed value)
BESCHREIBUNG:
Damit kann man eine Option zum Kampf setzen. Folgende Schluessel aus
/sys/fight_options.h sind in Verwendung:
FIO_BROKEN_WEAPON : Wert 1 bedeutet, kaputte Waffe senken, 2 hinlegen.
FIO_PREVENT_ONLY_DEFENSIVE: Wert 1 verhindert das Fuehren von nur Schildern.
FIO_WIELD_WEAPON: Wert 1 laesst das Monster eine andere Waffe fuehren.
FIO_TAKE_WEAPON: Wert 1 laesst das Monster eine Waffe aufheben und fuehren.
FIO_WIELD_RETRY: -1 bedeutet, immer wieder versuchen, eine Waffe zu fuehren
                 0 keine Wiederholung, und >0 Anzahl der Versuche.
FIO_USE_FAR_WEAPON: Wert 1 signalisiert, das Nutzen von Bogen, Armbruesten,
                           Wurfwaffen...
FIU_CL_ARROW_FACTORY: Eine Closure welche Pfeile o.ae.(id) in das Monster 
                bewegt und zur Nutzung von dem Bogen(weapon) zurueckgibt: 
                        object function(object npc,object weapon, string id)
VERWEISE: query_fight_option,query_fight_options,set_fight_options
GRUPPEN: monster, kampf
*/
nomask void add_fight_option(string key, mixed value)
{
    if (playerp(this_object())) return;
    if (value)
        fight_options[key] = value;
    else
        m_delete(fight_options, key);
}

/*
FUNKTION: query_fight_option
DEKLARATION: nomask mixed query_fight_option(string key)
BESCHREIBUNG:
Damit kann man eine Option zum Kampf abfragen.
Einzelheiten und Schluessel siehe add_fight_option
VERWEISE: add_fight_option,query_fight_options,set_fight_options
GRUPPEN: monster, kampf
*/
nomask mixed query_fight_option(string key)
{
    return fight_options[key];
}

/*
FUNKTION: query_fight_options
DEKLARATION: nomask mapping query_fight_options()
BESCHREIBUNG:
Damit kann man die Optionen zum Kampf abfragen. 
Einzelheiten siehe add_fight_option
VERWEISE: add_fight_option,query_fight_option,set_fight_options
GRUPPEN: monster, kampf
*/
nomask mapping query_fight_options()
{
    return copy(fight_options);
}

/* ================== H A N D S -- U T I L I T I E S ================ */


/*
FUNKTION: free_hand
DEKLARATION: int free_hand()
BESCHREIBUNG:
Liefert den index der ersten freien Hand zurueck oder -1, wenn keine 
Hand mehr frei ist
VERWEISE: query_hand_objects, query_num_free_hands
GRUPPEN: spieler, monster, haende
*/
int free_hand()
{
    int a;

    for (a=0; a<num_hands; a++)
    {
        if (!hand_objects[a])
        {
            return a;
        }
        if (!present(hand_objects[a],this_object()))
        {
            hand_objects[a] = 0;
            return a;
        }
    }
    return -1;
}

/*
FUNKTION: free_fight
DEKLARATION: int free_fight()
BESCHREIBUNG:
Liefert die erste Hand, mit der man kaempfen koennte, das heisst, eine Hand,
mit der nicht gekaempft wird und in der nichts anderes ist als eine 
Nahkampfwaffe. Liefert -1 zurueck, wenn keine Hand zu einem Kampf bereit 
ist.
VERWEISE: 
GRUPPEN: spieler, monster, kampf, haende
*/
int free_fight()
{
    int a;

    // Zuerst nach Waffen suchen.
    for (a=0; a<num_hands; a++)
    {
        if (hand_objects[a] && !present(hand_objects[a],this_object()))
            hand_objects[a] = 0;
        else if(!hand_enemies[a] && hand_objects[a] &&
            hand_objects[a]->query_weapon_class("nahkampf") &&
            hand_objects[a]->query_life() > 0)
                return a;
    }
    
    // Dann Haende ohne Waffen nehmen.
    for (a=0; a<num_hands; a++)
        if (!hand_enemies[a] && !hand_objects[a])
            return a;

    return -1;
}


/*
FUNKTION: query_hand_objects
DEKLARATION: object *query_hand_objects()
BESCHREIBUNG:
Liefert alle gefuehrten Objekte zurueck. Der Rueckgabewert ist immer
ein Array mit einem Eintrag pro Hand. Es kann Nullen enthalten,
wenn die entsprechende Hand keine Waffe fuehrt.
VERWEISE: free_hand, query_num_free_hands
GRUPPEN: spieler, monster, haende
*/
object *query_hand_objects()
{
    int a;

    for (a=0; a<num_hands; a++)
        if (hand_objects[a] && !present(hand_objects[a],this_object()))
            hand_objects[a] = 0;
    return copy(hand_objects);
}

varargs int set_hand_object(object ob,int flags)
{
    int a,otherhand;

    if (!(flags&2))
    {
        for(otherhand = 0;otherhand < sizeof(hand_objects);otherhand++)
        {
            if (ob && hand_objects[otherhand]==ob)
            {
                return 0;
            }
        }
    }

    if ((a=free_hand()) >= 0)
    {
        hand_objects[a]=ob;
        return 1;
    }
}

void delete_hand_object(object ob)
{
    int a;

    for (a=0; a<num_hands; a++)
        if (hand_objects[a]==ob)
            hand_objects[a]=0;
}


/*
FUNKTION: query_num_free_hands
DEKLARATION: int query_num_free_hands()
BESCHREIBUNG:
Liefert die Anzahl freier Haende.
VERWEISE: free_hand, query_hand_objects
GRUPPEN: spieler, monster, haende
*/

int query_num_free_hands()
{
    return sizeof(filter(query_hand_objects(), #'!));
}


/*
FUNKTION: query_hand_enemies
DEKLARATION: object *query_hand_enemies()
BESCHREIBUNG:
Liefert alle momentan bekaempften Gegener zurueck.
VERWEISE: 
GRUPPEN: spieler, monster, kampf
*/
object *query_hand_enemies()
{
    return copy(hand_enemies);
}

/*
FUNKTION: query_hand_hits
DEKLARATION: object *query_hand_hits()
BESCHREIBUNG:
Liefert alle momentan gelieferten Schlaege.
VERWEISE: 
GRUPPEN: spieler, monster, kampf
*/
object *query_hand_hits()
{
    return copy(hand_hits);
}

/*
FUNKTION: delete_enemy
DEKLARATION: void delete_enemy(object enemy)
BESCHREIBUNG:
Loescht den Gegner enemy aus allen Haenden.
VERWEISE:
GRUPPEN: spieler, monster, kampf
*/
void delete_enemy(object enemy)
{
    int a;

    for (a=0; a<num_hands; a++)
        if (hand_enemies[a]==enemy)
        {
            hand_enemies[a] = 0;
            hand_hits[a] = 0;
        }
}

/*
FUNKTION: add_gone_ob
DEKLARATION: void add_gone_ob(object ob)
BESCHREIBUNG:
Auch in ob wird bei Abwesenheit des Gegners im Kampf 'enemy_gone(object enemy)'
aufgerufen.
VERWEISE: delete_gone_ob, query_gone_ob
GRUPPEN: spieler, monster, kampf
*/
void add_gone_ob(object ob)
{
    if (!gone_ob)
        gone_ob=({ob});
    else
        gone_ob=gone_ob-({ob,0})+({ob});
}

/*
FUNKTION: delete_gone_ob
DEKLARATION: void delete_gone_ob(object ob)
BESCHREIBUNG:
ob wird nicht mehr ueber fehlende Gegner informiert.
VERWEISE: add_gone_ob, query_gone_ob
GRUPPEN: spieler, monster, kampf
*/
void delete_gone_ob(object ob) { if(gone_ob) gone_ob-=({ob,0}); }

/*
FUNKTION: query_gone_ob
DEKLARATION: object *query_gone_ob()
BESCHREIBUNG:
Gibt die in add_gone_ob naeher spezifizierten Objekte zurueck.
VERWEISE: add_gone_ob, delete_gone_ob
GRUPPEN: spieler, monster, kampf
*/
object *query_gone_ob() { return gone_ob; }

/*
FUNKTION: query_in_fight
DEKLARATION: varargs int query_in_fight(object who)
BESCHREIBUNG:
Liefert 1 zurueck wenn der Spieler/das Monster gerade kaempft, sonst 0.
Wenn 'who' uebergeben wird, liefert query_in_fight 1, wenn mit 'who'
gekaempft wird.
VERWEISE: query_hand_enemies
GRUPPEN: spieler, monster, kampf
*/
varargs int query_in_fight(object who)
{
    int a;

    if (who)
    {
        for (a = 0; a < num_hands; a++)
            if (hand_enemies[a] == who)
                return 1;
    }
    else
    {
        for (a = 0; a < num_hands; a++)
            if (hand_enemies[a])
                return 1;
    }
    return 0;
}


/*
FUNKTION: query_wielded_object
DEKLARATION: int query_wielded_object(object ob)
BESCHREIBUNG:
Liefert 1 zurueck, wenn ob gefuehrt ist, sonst 0.
VERWEISE: 
GRUPPEN: spieler, monster, haende, kampf
*/
int query_wielded_object(object ob)
{
    return ob && member(hand_objects, ob) >= 0;
}


object check_outer_containers(object con,object tp)
{
    object econ = con ? environment(con) : 0;
    object etp = environment(tp);
    // DEBUG(sprintf("check_outer_containers-1:\n"
        // "con: %Q\n"
        // "tp:  %Q\n"
        // "etp: %Q\n"
        // "econ:  %Q\n"
        // "close: %d\n",
        // con,tp,etp,econ,con?con->query_con_close():0 ));
    while (con && econ && tp && etp && econ != etp)
    {
        // DEBUG(sprintf("check_outer_containers-2:\n"
            // "econ:  %Q\n"
            // "close: %d\n",
            // econ,econ->query_con_close() ));
        if (econ->query_con_close())
        {
            return econ;
        }
        econ = environment(econ);
    }
    // DEBUG(sprintf("check_outer_containers-3:\n"
        // "econ:  %Q\n"
        // "close: %d\n",
        // econ,econ->query_con_close() ));
    return 0;
}



/* =====================  T  A  K  E  ======================== */

/*
FUNKTION: no_take
DEKLARATION: int no_take (object who, object where)
BESCHREIBUNG:
Bevor ein Objekt genommen wird (nimm Xyz, nimm Xyz aus Tasche) wird in dem zu
nehmenden Objekt die Funktion no_take aufgerufen, who ist das Lebewesen,
das zu nehmen versucht, where ist der Container, aus dem who genommen wird.
Antwortet no_take mit 0, so wird das Objekt zu nehmen versucht (Filter,
Gewicht usw. koennten das ja trotzdem verhindern), liefert no_take einen
Wert ungleich 0 so wird das Objekt nicht genommen, no_take muss eine
Meldung an who ausgeben, kann in den Raum ebenfalls eine ausgeben
(warum es nicht ging) oder eine entsprechende Reaktion ausloesen.
BEISPIEL:
Die Banane in der Magiergilde glitscht davon, wenn man sie nehmen will:
    int no_take()
    {
       return !runaway();
    }
Gelingt runaway, sprich, schafft es die Banane, davonzulaufen, so kann
man sie nicht nehmen.
Eine Fehlermeldung eruebrigt sich, da der Spieler ja sieht, warum
es nicht geklappt hat.
Noch ein Beispiel: Dynamit, das bereits gezuendet ist, sollte man ebenfalls
nicht nehmen (es sei denn, man ist Selbstmoerder):
    int no_take(object who, object woher)
    {
       if(gezuend)
       {
          send_message_to(who, MT_NOTIFY, MA_TAKE,
              wrap("Im gegenwaertigen Zustand sollte man das Dynamit "
                   "lieber nicht anruehren!"));
          return 1;
       }
    }

VERWEISE: take, put, no_put, put_all, take_all
GRUPPEN: spieler, monster, haende
*/


/*
FUNKTION: take
DEKLARATION: varargs int take(mixed ob, object con, int flag)
BESCHREIBUNG:
Mit take kann man das Objekt ob aus dem Container con herausholen
Wenn flag == 1, dann werden keine Meldungen an andere erzeugt.
VERWEISE: take_all, no_take, put, put_all, no_put
GRUPPEN: monster, spieler, haende
*/


/*
FUNKTION: query_take_messages
DEKLARATION: string *query_take_messages(object wer, object was)
BESCHREIBUNG:
Soll ein Objekt <was> von einem Spieler <wer> aus einem Container genommen
werden, so wird im Container diese Funktion aufgerufen. Hiermit kann der
Container die Meldungen erzeugen, die dabei ausgegeben werden sollen.
Der Rueckgabewert dieser Funktion ist ein Array aus zwei Strings, der erste
String ist der, welchen der nehmende Spieler erhaelt, der zweite geht an
die Umgebung des Spielers.
Diese Funktion wird aufgerufen, bevor der Gegenstand aus dem Container
genommen werden soll. Da es daher noch nicht feststeht, ob der
Gegenstand ueberhaupt aus dem Container genommen werden kann darf diese
Funktion auch keinerlei Nebeneffekte besitzen!
Ist ein String in dem Array 0, so wird statt dessen die Standardmeldung
erzeugt, so ist es moeglich, nur die Meldung, die der Spieler bekommen
soll, zu beeinflussen. Ist ein String in dem Array "" (leer), so wird
keine Meldung ausgegeben. Dadurch ist es moeglich, Meldungen zu unterdruecken.
Es gibt kein set_take_messages und wird auch keines geben. Diese Funktion
ist fuer spezielle Spezialfaelle gedacht.

BEISPIEL: Rucksack.
    string *query_take_messages (object wer, object was)
    {
        return ({"Obwohl es in "+deinem()+" stockfinster ist, ziehst "
            "Du nach fuenf Minuten "+einen(was)+" aus ihm heraus.",
            "Obwohl es in "+seinem()+" stockfinster ist, zieht "+der(wer)
            +" nach fuenf Minuten "+einen(was)+" aus "+ihm()+" heraus."});
    }

VERWEISE: take, no_take, query_put_messages
GRUPPEN: monster, spieler, haende
*/

varargs int take(mixed ob, object con, int no_ok_message)
{
    int res, prio, flag_concerned, flag_display;
    string desc, where, mywhere, why_not, prepos, *take_messages,
           tmp, tmp_other;
    mixed zustaendig, zufun, zutmp, zumsg;
    object econ;

    if (!objectp(ob))
    {
        if (mappingp (ob))
        {
            tmp = QUERY("take",ob);
            if((tmp_other = QUERY("take_msg",ob)) && tmp_other != "")
                this_object()->send_message(MT_LOOK,MA_TAKE,wrap(tmp_other));
        }
        notify_message(tmp ? tmp : "Das geht nicht.\n",MA_TAKE);
	return MOVE_NOT_ALLOWED;
    }

    if(!con)
        con = environment(ob);
    if(ob == con)
    {
	if (ob == environment())
	   notify_message(wrap(Den(ob)+" kannst Du nicht nehmen."), MA_TAKE);
	else
	   notify_message("Etwas aus sich selbst herausholen?\n",MA_TAKE);
	return MOVE_NOT_ALLOWED;
    }

    if(this_object()->forbidden("take", ob, con) ||
       ob->forbidden("take_me", this_object(), con) ||
       (objectp(con) && con->forbidden("take_from", this_object(), ob)) ||
       ob->no_take(this_object(), con))
       return MOVE_NOT_ALLOWED;

   if (econ = check_outer_containers(con,this_object())) // take
   {
      notify_message(wrap(Der(econ)+" ist geschlossen."),MA_PUT);
      return MOVE_NOT_ALLOWED;
   }
   
    zustaendig = this_object()->concerned(&prio, "take", ob, con);
    zufun = ({ "do_take", ob, con, this_object() });
    zumsg = ({ "display_messages_take", ob, con, this_object() });
    zutmp = ob->concerned(&prio, "take_me", this_object(),con);
    if (zutmp)
    {
        zustaendig = zutmp;
        zufun = ({ "do_take_me", this_object(), con, ob });
        zumsg = ({ "display_messages_take_me", this_object(), con, ob });
    }
    zutmp = con->concerned(&prio, "take_from", this_object(), ob);
    if (zutmp)
    {
        zustaendig = zutmp;
        zufun = ({ "do_take_from", this_object(), ob, con });
        zumsg = ({ "display_messages_take_from", this_object(), ob, con });
    }
    flag_concerned = 0;
    if (zustaendig)
    {
        if (objectp(zustaendig))
            res = apply(#'call_other, zustaendig, zufun);
        else
            res = apply(zustaendig, zufun);

        flag_concerned = 1;
    }

    if (!flag_concerned)
    {
      if(living(ob) && ob->query_weight() > 1 &&
	   (playerp(ob) || !ob->allowed("take_living", ob, this_object())))
      {
        notify_message(wrap(Der(ob)+plural(" lässt"," lassen",ob)+
            " sich nicht so einfach mitnehmen."),MA_TAKE);
        return MOVE_NOT_ALLOWED;
      }
    }

    if(ob->query_name())
	desc = ob->query_invis() & V_ATOM_HIDDEN || auto_owner_search(ob) ?
	    einen(ob) :
	    wen(ob, ART_AAA);
    else
	desc = "irgendetwas";

    if ((take_messages = con->query_take_messages(this_object(),ob))
	&& !(pointerp (take_messages) || (sizeof(take_messages)!=2)))
	take_messages = 0;

    if (!flag_concerned)
        res = ob->move(this_object(), ([MOVE_TYPE: MOVE_TYPE_NEHMEN]));

    switch(res)
    {
       case MOVE_OK :
       case MOVE_DESTRUCTED :
          flag_display = 0;
          if (zustaendig)
          {
              if (objectp(zustaendig))
                  flag_display = apply(#'call_other, zustaendig, zumsg);
              else
                  flag_display = apply(zustaendig, zumsg);
          }
          if(!no_ok_message && !flag_display)
	  {
            if (!con || con == environment() || con->query_room())
	    {
        	where = ""; mywhere = "";
	    }
	    else if (prepos = con->query_take_prepos())
	    {
		where = " "+prepos+" "+seinem (con);
                mywhere = " "+prepos+" "+deinem(con);
	    }
	    else
	    {
		where = (living(con) ? " von " : " aus ") + seinem(con);
                mywhere = (living(con) ? " von " : " aus ") + deinem(con);
            }
	    this_object()->send_message(MT_LOOK,MA_TAKE,
		  take_messages && take_messages[1]
		? wrap_if_not_empty(take_messages[1])
	        : wrap(Der()+" nimmt "+desc+where+"."));
	    notify_message(
		  take_messages && take_messages[0]
	        ? wrap_if_not_empty(take_messages[0])
	        : wrap("Du nimmst "+desc+mywhere+"."),MA_TAKE);
	  }
	  if(this_object())
              this_object()->notify("take", ob, con);
	  if(ob)
	     ob->notify("take_me", this_object(), con);
          if(con)
             con->notify("take_from", this_object(), ob);
	  break;

       default:
	  if (ob && (why_not = ob->query_not_moved_reason()))
             why_not = wrap(why_not);
	  switch(res)
	  {
	     case MOVE_NOT_ALLOWED :
		notify_message(why_not || "Das geht nicht.\n",MA_TAKE);
		break;
	     case MOVE_NO_ROOM :
		if(ob && ob->query_weight() > 100)
		   notify_message(why_not || "So schwere Sachen kannst Du nicht tragen.\n",MA_TAKE);
		else
		   notify_message(why_not || "Mehr kannst Du nicht tragen.\n",MA_TAKE);
		break;
	     case MOVE_DEST_CLOSED :
		notify_message(why_not || "Du stehst unter einem Bann.\n",MA_TAKE);
		break;
	     case MOVE_ENV_CLOSED :
		notify_message(why_not ||
		      wrap(Dein(con)+ (living(con) ? plural(" steht"," stehen",con)+
			  " unter einem Bann." :
			  ist(con,1) + " geschlossen.")),MA_TAKE);
	  }
	  break;
    }
    return res;
}

/*
FUNKTION: forbidden_take
DEKLARATION: int forbidden_take(object ob, object where, object who)
BESCHREIBUNG:
Bevor ein Objekt ob von einem Lebewesen who aus dem Container where genommen
werden kann, wird who->forbidden("take", ob, where) aufgerufen, liefert
dieser Aufruf einen Wert ungleich 0 zurueck, wird ob nicht genommen.

Die Funktion forbidden ruft in allen mit who->add_controller("forbidden_take",
 other) angemeldeten Objekten other die Funktionen other->forbidden_take(ob,
where, who) auf. Liefert auch nur eine dieser Funktionen einen Wert ungleich
0, dann returnt forbidden diesen und das Objekt ob kann nicht genommen werden.

Fuer die Ausgabe einer Meldung an das Lebewesen who muss der Programmierer
in forbidden_take oder forbidden, falls er diese Funktion ueberlagern will,
sorgen.

Bemerkung: Es wird auch ob->forbidden("take_me", who, where) 
           und where->forbidden("take_from", who, ob) aufgerufen.
VERWEISE: forbidden, notify, notify_take, take, no_take
GRUPPEN: spieler, monster, haende
*/

/*
FUNKTION: forbidden_take_me
DEKLARATION: int forbidden_take_me(object who, object where, object ob)
BESCHREIBUNG:
Bevor ein Objekt ob von einem Lebewesen who aus dem Container where genommen
werden kann, wird ob->forbidden("take_me", who, where) aufgerufen, liefert
dieser Aufruf einen Wert ungleich 0 zurueck, wird ob nicht genommen.

Die Funktion forbidden ruft in allen mit ob->add_controller(
"forbidden_take_me", other) angemeldeten Objekten other die Funktionen
other->forbidden_take_me(who, where, ob) auf. Liefert auch nur eine dieser
Funktionen einen Wert ungleich 0, dann returnt forbidden diesen und das Objekt
ob kann nicht genommen werden.

Fuer die Ausgabe einer Meldung an das Lebewesen who muss der Programmierer
in forbidden_take_me oder forbidden, falls er diese Funktion ueberlagern will,
sorgen.

Bemerkung: Es wird auch who->forbidden("take", ob, where) 
           und where->forbidden("take_from", who, ob) aufgerufen.
VERWEISE: forbidden, notify, notify_take, take, no_take
GRUPPEN: spieler, monster, haende
*/

/*
FUNKTION: forbidden_take_from
DEKLARATION: int forbidden_take_from(object who, object ob, object where)
BESCHREIBUNG:
Bevor ein Objekt ob von einem Lebewesen who aus dem Container where genommen
werden kann, wird where->forbidden("take_from", who, ob) aufgerufen, liefert
dieser Aufruf einen Wert ungleich 0 zurueck, wird ob nicht genommen.

Die Funktion forbidden ruft in allen mit where->add_controller(
"forbidden_take_from", other) angemeldeten Objekten other die Funktionen
other->forbidden_take_from(who, ob, where) auf. Liefert auch nur eine dieser
Funktionen einen Wert ungleich 0, dann returnt forbidden diesen und das
Objekt ob kann nicht genommen werden.

Fuer die Ausgabe einer Meldung an das Lebewesen who muss der Programmierer
in forbidden_take_from oder forbidden, falls er diese Funktion ueberlagern will,
sorgen.

Bemerkung: Es wird auch who->forbidden("take", ob, where) 
           und ob->forbidden("take_me", who, where) aufgerufen.
VERWEISE: forbidden, notify, notify_take, take, no_take
GRUPPEN: spieler, monster, haende
*/

/*
FUNKTION: notify_take
DEKLARATION: void notify_take(object ob, object where, object who)
BESCHREIBUNG:
Nachdem ein Objekt ob von einem Lebewesen who aus dem Container where genommen
wurde, wird who->notify("take", ob, where) aufgerufen.

Die Funktion notify ruft in allen mit who->add_controller("notify_take",
 other) angemeldeten Objekten other die Funktionen other->notify_take(ob,
where, who) auf. Sowohl who als auch other haben dann die Moeglichkeit, auf
das Nehmen von ob aus where zu reagieren.

Bemerkung: Es wird auch ob->notify("take_me", who, where) 
           und where->notify("take_from", who, ob) aufgerufen.
VERWEISE: forbidden, notify, forbidden_take, take, no_take
GRUPPEN: spieler, monster, haende
*/

/*
FUNKTION: notify_take_me
DEKLARATION: void notify_take_me(object who, object where, object ob)
BESCHREIBUNG:
Nachdem ein Objekt ob von einem Lebewesen who aus dem Container where genommen
wurde, wird ob->notify("take_me", who, where) aufgerufen.

Die Funktion notify ruft in allen mit ob->add_controller(
"notify_take_me", other) angemeldeten Objekten other die Funktionen
other->notify_take_me(who, where, ob) auf. Sowohl ob als auch other haben
dann die Moeglichkeit, auf das Nehmen von ob aus where zu reagieren.

Bemerkung: Es wird auch who->notify("take", ob, where) 
           und where->notify("take_from", who, ob) aufgerufen.
VERWEISE: forbidden, notify, forbidden_take, take, no_take
GRUPPEN: spieler, monster, haende
*/

/*
FUNKTION: notify_take_from
DEKLARATION: void notify_take_from(object who, object ob, object where)
BESCHREIBUNG:
Nachdem ein Objekt ob von einem Lebewesen who aus dem Container where genommen
wurde, wird where->notify("take_from", who, ob) aufgerufen.

Die Funktion notify ruft in allen mit where->add_controller(
"notify_take_from", other) angemeldeten Objekten other die Funktionen
other->notify_take_from(who, ob, where) auf. Sowohl where als auch other haben
dann die Moeglichkeit, auf das Nehmen von ob aus where zu reagieren.

Bemerkung: Es wird auch who->notify("take", ob, where) 
           und ob->notify("take_me", who, where) aufgerufen.
VERWEISE: forbidden, notify, forbidden_take, take, no_take
GRUPPEN: spieler, monster, haende
*/

/*
FUNKTION: concerned_take
DEKLARATION: int concerned_take(object ob, object where, object who)
BESCHREIBUNG:
Bevor ein Objekt ob von einem Lebewesen who aus dem Container where genommen
wird, wird who->concerned("take", ob, where) aufgerufen.

Die Returnwerte der per controller bei 'ob' angemeldeten Objekten 
werden als Prioritaten angesehen, wobei das Objekt 'cob' mit der 
hoechsten Prioritaet den Zuschlag erhaelt und per 
cob->do_take(ob,where,who); dazu aufgefordert wird
sich um das Nehmen von 'ob' zu kuemmern. Zur Ausgabe der Erfolgsmeldungen
wird zusaetzlich cob->display_messages_take(ob,where,who) aufgerufen,
falls vorhanden.

Bemerkung: Es wird auch where->concerned("take_from", who, ob) 
           und ob->concerned("take_me", who, where) aufgerufen.
VERWEISE: concerned, concerned_take_from, concerned_take, do_take
GRUPPEN: spieler, monster, haende
*/

/*
FUNKTION: concerned_take_me
DEKLARATION: int concerned_take_me(object who, object where, object ob)
BESCHREIBUNG:
Bevor ein Objekt ob von einem Lebewesen who aus dem Container where genommen
wird, wird ob->concerned("take_me", who, where) aufgerufen.

Die Returnwerte der per controller bei 'ob' angemeldeten Objekten 
werden als Prioritaten angesehen, wobei das Objekt 'cob' mit der 
hoechsten Prioritaet den Zuschlag erhaelt und per 
cob->do_take_me(who,where,ob); dazu aufgefordert wird
sich um das Nehmen von 'ob' zu kuemmern. Zur Ausgabe der Erfolgsmeldungen
wird zusaetzlich cob->display_messages_take_me(who,where,ob) aufgerufen,
falls vorhanden.

Bemerkung: Es wird auch who->concerned("take", ob, where) 
           und where->concerned("take_from", who, ob) aufgerufen.
VERWEISE: concerned, concerned_take_from, concerned_take, do_take_me
GRUPPEN: spieler, monster, haende
*/

/*
FUNKTION: concerned_take_from
DEKLARATION: int concerned_take_from(object who, object ob, object where)
BESCHREIBUNG:
Bevor ein Objekt ob von einem Lebewesen who aus dem Container where genommen
wird, wird where->concerned("take_from", who, ob) aufgerufen.

Die Returnwerte der per controller bei 'where' angemeldeten Objekten 
werden als Prioritaten angesehen, wobei das Objekt 'cob' mit der 
hoechsten Prioritaet den Zuschlag erhaelt und per 
cob->do_take_from(who,ob,where); dazu aufgefordert wird
sich um das Nehmen aus 'where' zu kuemmern. Zur Ausgabe der Erfolgsmeldungen
wird zusaetzlich cob->display_messages_take(who,ob,where) aufgerufen,
falls vorhanden.

Bemerkung: Es wird auch who->concerned("take", ob, where) 
           und ob->concerned("take_me", who, where) aufgerufen.
VERWEISE: concerned, concerned_take_me, concerned_take, do_take_from
GRUPPEN: spieler, monster, haende
*/


/*
FUNKTION: do_take_from
DEKLARATION: int do_take_from(object who, object ob, object where)
BESCHREIBUNG:
Diese Funktion wird in dem Objekt aufgerufen, welches ueber 
concerned_take_from die hoechste Prioritaet zurueckgeliefert hat 
und sich somit dafuer zustaendigst erklaert hat sich um das Nehmen 
durch 'who' von 'ob' aus 'where' zu kuemmern.
Es sollte also etwas bewegt werden und der Returnwert des move
zurueckgegeben werden. 
VERWEISE: concerned_take_from, do_take_me, take
GRUPPEN: spieler, monster, haende
*/

/*
FUNKTION: do_take_me
DEKLARATION: int do_take_me(object who, object where, object ob)
BESCHREIBUNG:
Diese Funktion wird in dem Objekt aufgerufen, welches ueber 
concerned_take_me die hoechste Prioritaet zurueckgeliefert hat 
und sich somit dafuer zustaendigst erklaert hat sich um das Nehmen 
durch 'who' von 'ob' aus 'where' zu kuemmern.
Es sollte also etwas bewegt werden oder auch nicht 
und der Returnwert des move zurueckgegeben werden. (siehe /sys/move.h)
VERWEISE: concerned_take_me, do_take_from, take
GRUPPEN: spieler, monster, haende
*/

/*
FUNKTION: do_take
DEKLARATION: int do_take(object ob, object where, object who)
BESCHREIBUNG:
Diese Funktion wird in dem Objekt aufgerufen, welches ueber 
concerned_take die hoechste Prioritaet zurueckgeliefert hat 
und sich somit dafuer zustaendigst erklaert hat sich um das Nehmen 
durch 'who' von 'ob' aus 'where' zu kuemmern.
Es sollte also etwas bewegt werden und der Returnwert des move
zurueckgegeben werden. 
VERWEISE: concerned_take_from, do_take_from, take
GRUPPEN: spieler, monster, haende
*/


/*
FUNKTION: display_messages_take
DEKLARATION: int display_messages_take(object ob, object where, object who)
BESCHREIBUNG:
Diese Funktion wird in dem Objekt aufgerufen, welches ueber 
concerned_take die hoechste Prioritaet zurueckgeliefert hat 
und sich somit dafuer zustaendigst erklaert hat sich um das Nehmen 
durch 'who' von 'ob' aus 'where' zu kuemmern.
Es sollen nun die Erfolgsmeldungen ausgegeben und ein Wert != 0
zurueckgegeben werden.
VERWEISE: concerned_take_from, display_messages_take_me, take
GRUPPEN: spieler, monster, haende
*/

/*
FUNKTION: display_messages_take_me
DEKLARATION: int display_messages_me(object who, object where, object ob)
BESCHREIBUNG:
Diese Funktion wird in dem Objekt aufgerufen, welches ueber 
concerned_take_me die hoechste Prioritaet zurueckgeliefert hat 
und sich somit dafuer zustaendigst erklaert hat sich um das Nehmen 
durch 'who' von 'ob' aus 'where' zu kuemmern.
Es sollen nun die Erfolgsmeldungen ausgegeben und ein Wert != 0
zurueckgegeben werden.
VERWEISE: concerned_take_me, display_messages_take_from, take
GRUPPEN: spieler, monster, haende
*/

/*
FUNKTION: display_messages_take_from
DEKLARATION: int display_messages_from(object who, object ob, object where)
BESCHREIBUNG:
Diese Funktion wird in dem Objekt aufgerufen, welches ueber 
concerned_take_from die hoechste Prioritaet zurueckgeliefert hat 
und sich somit dafuer zustaendigst erklaert hat sich um das Nehmen 
durch 'who' von 'ob' aus 'where' zu kuemmern.
Es sollen nun die Erfolgsmeldungen ausgegeben und ein Wert != 0
zurueckgegeben werden.
VERWEISE: concerned_take_from, display_messages_take, take
GRUPPEN: spieler, monster, haende
*/


/*
FUNKTION: take_all
DEKLARATION: int take_all(mixed *ob_list, object con)
BESCHREIBUNG:
Mit take_all kann man mehrere Objekte obj_list aus den Container con nehmen.
VERWEISE: take, no_take, put, put_all, no_put
GRUPPEN: spieler, monster, haende
*/
int take_all(mixed *ob_list, object con)
{
    string *names = ({});
    int i, envflag;
    object env;
    string where, mywhere, prepos;

    envflag = present(con, this_object())?0:1;
    env = environment();
    // Wenn 1, dann Abbruch bei Bewegung
	
    if (con == environment() || con->query_room())
    {
        where = ""; mywhere = "";
    }
    else if (prepos = con->query_take_prepos())
    {
        where = " "+prepos+" "+seinem (con);
        mywhere = " "+prepos+" "+deinem(con);
    }
    else
    {
        where = (living(con) ? " von " : " aus ") + seinem(con);
        mywhere = (living(con) ? " von " : " aus ") + deinem(con);
    }


    for (i=0; i<sizeof(ob_list); i++)
    {
        CHECK_EVAL_COST
        if (ob_list[i] && con && ob_list[i] != this_object())
        {
            string name = einen(ob_list[i]);
            mixed tmsg;
            int res;
            
            tmsg = con->query_take_messages(this_object(),ob_list[i]);
            if(!pointerp(tmsg) || sizeof(tmsg)!=2)
                tmsg = 0;
            res = take(ob_list[i],con,tmsg?0:1);
            if(!tmsg && (res==MOVE_OK || res==MOVE_DESTRUCTED))
                names += ({name});
            if(envflag && env!=environment())
                break;
        }
    }
    
    if(sizeof(names))
    {
        this_object()->send_message(MT_LOOK, MA_TAKE,
            wrap(Der()+" nimmt "+liste(names)+where+"."));
        notify_message(
            wrap("Du nimmst "+liste(names)+mywhere+"."), MA_TAKE);
    }
    return sizeof(ob_list) && 1;
}

#define WFAIL(x) { notify_message(x,MA_TAKE); return 1; }

varargs int take_countob(string ob_name, object ob, object con, 
			int no_ok_message ) {
    object countob;
    int count, max, ret;

    max = ob->query_count();

    if (sscanf(ob_name,"%d %s",count,ob_name) != 2 || max == count)
	return take(ob,con, no_ok_message);

    if (count < 1)
	WFAIL("Wie soll das gehen?\n");
    if (count > max)
	WFAIL("Soviel ist da nicht.\n");

    countob=ob->split_object(count);
    if (!countob)
	WFAIL("Das geht nicht.\n");
    ret=take(countob,con, no_ok_message);
    if (ret != MOVE_OK) {
	if (countob)
	    countob->remove();
	ob->add_count(count);
	}
    return ret;
}

static int no_players(object ob)
{
    return !living(ob) || ob->query_weight()<2;
}

int take_command(string str)
{
    string trenner;
    mixed *parsed, *obs, con, tmp;
    string tmp_other;

    if (free_hand() < 0) 
        return notify_fail("Du hast keine Hand mehr frei.\n", FAIL_INTERNAL);
    if (!str) 
        return notify_fail("Nimm was?\n", FAIL_NOT_OBJ);

    str = lower_case(str);

    parsed = parse_com(str,0,({"von","aus","vom"}),PARSE_AFTER_TRENNER);
    if (parse_com_error(parsed,"Nimm was von wem?\n"))
        return 0;
    // DEBUG(sprintf("take: parsed-1: %Q",parsed));
    if ((trenner = parsed[PARSE_TRENNER]) != "")
    {
        if (sizeof(parsed[PARSE_OBS]) > 1)
            return notify_fail("Doch nicht immer alles auf einmal!\n", FAIL_NOT_OBJ);
        if (parsed[PARSE_BEYOND_TRENNER] == "")
            return notify_fail("Nimm was "+trenner+" wem?\n", FAIL_WRONG_ARG);
        con = parsed[PARSE_OBS][0];
        // DEBUG(sprintf("take: con-1: %Q",con));
        if (!objectp(con))
        {
            // Testen wir mal, ob es ein V-Item an einem V-Item ist.    
            parsed = parse_com(str);
            // DEBUG(sprintf("take: parsed-2obj: %Q",parsed));
            if(parse_com_error(parsed,"Nimm was wovon?\n"))
                return notify_fail(wrap(CAP(trenner)+" "+dem(con)+" lässt sich nichts nehmen."), FAIL_WRONG_ARG);
            con = 0;
        }
        else
        {
            parsed = parse_com(parsed[PARSE_BEYOND_TRENNER],con);
            // DEBUG(sprintf("take: parsed-2vitem: %Q",parsed));
            if (parse_com_error(parsed,wrap("Nimm was "+trenner+" "+dem(con)+"?")))
                return 0;
        }
    }
    
    obs = parsed[PARSE_OBS];
    
    if(!con)	// Kein Trenner
    {
        if (mappingp(obs[0]))
        {
            if(!tmp = QUERY("take",obs[0]))
                if (!(tmp = QUERY("far",obs[0])))
                    tmp = "Da gibt es nichts zu holen.\n";
                else if(!stringp(tmp))
                    tmp = wrap (Der(obs[0])+ ist(obs[0],1) +
                        " viel zu weit weg, um "+ihn(obs[0])+" zu nehmen.");
            notify_message(tmp,MA_TAKE);
            if((tmp_other = QUERY("take_msg",obs[0])) && tmp_other != "")
                this_object()->send_message(MT_LOOK,MA_TAKE,wrap(tmp_other));
            return 1;
        }
        else
        {
            con = environment(obs[0]);
            // DEBUG(sprintf("take: con-3: %Q %Q",obs[0],con));

            if(con==this_object())
            {
                if(parsed[PARSE_ID]=="")
                    return notify_fail("Hier gibt es nichts.\n",
                        FAIL_WRONG_ARG);
                else
                    return notify_fail("Hier gibt es nichts Derartiges.\n",
                        FAIL_WRONG_ARG);
            }
        }
    }
    
    
    if (sizeof(obs) <= 0)
        return notify_fail("Da gibt es nichts zu holen.\n", FAIL_WRONG_ARG);
    if (con == this_object())
        return notify_fail("Von Dir selber?\n",FAIL_WRONG_ARG);
    if (obs[0] == this_object())
        return notify_fail("Dich selbst?\n", FAIL_WRONG_ARG);
    if (living(con) && !con->allowed("take_from_living", this_object())) 
        return notify_fail(wrap(Der(con)+" hätte sicher etwas dagegen."), FAIL_INTERNAL);
    if (con->query_con_close()) 
        return notify_fail(wrap(Dein(con)+ (living(con)? plural(" steht"," stehen",con) +
                " unter einem Bann." : ist(con,1) + " geschlossen.")), FAIL_INTERNAL);
    if (living(this_object()) && this_object()->query_con_close())
        return notify_fail("Du stehst unter einem Bann.\n",FAIL_INTERNAL);

    if (sizeof(obs) > 1)
    {
        if (!take_all(filter(obs,"no_players"),con))
            return notify_fail("Da gibt es nichts zu holen.\n", FAIL_WRONG_ARG);
    }
#if 0
    else if (objectp(obs[0]) && obs[0]->query_count())
        take_countob(parsed[PARSE_ID],obs[0],con);
#endif
    else
    {
        //DEBUG(sprintf("take: obs0-con-3: %Q %Q %Q %Q",
        //     obs[0],con,environment(this_player()),environment(con)));
        take(obs[0],con);
    }
    return 1;
}



/* =====================  P  U  T  ======================== */

/*
FUNKTION: no_put
DEKLARATION: int no_put (object who, object where)
BESCHREIBUNG:
Bevor ein Objekt gelegt wird (lege Xyz, lege Xyz in Tasche) wird in dem zu
legenden Objekt die Funktion no_put aufgerufen, who ist das Lebewesen,
das zu legen versucht, where ist das Ziel des legens.
Antwortet no_put mit 0, so wird das Objekt zu legen versucht (Filter,
Gewicht usw. koennten das ja trotzdem verhindern), liefert no_put einen
Wert ungleich 0 so wird das Objekt nicht geschoben, no_put muss eine
Meldung an who ausgeben und sollte in den Raum ebenfalls eine ausgeben
(warum es nicht ging) oder eine entsprechende Reaktion ausloesen.
BEISPIEL:
Schwert aus Doerrland - Labyrinth: Wenn man das Schwert fuehrt, entdeckt
man, dass es verwunschen ist. Dann kann man es zwar wieder senken, es aber
nicht mehr wieder ablegen.
    int no_put(object who, object where)
    {
        if (!cursed)
            return 0;
        tell_object(who,wrap(Dein()+" klebt wie ein schlechter Ruf an "
	    "deiner Hand. Du bekommst es nicht ab."));
        who->send_message(MT_LOOK,MA_UNKNOWN,wrap(Der(this_player())+" fuchtelt verzweifelt mit "+
	   seinem()+" herum. Was soll das wohl?"));
        return 1;
    }
VERWEISE: put, put_all, take, tak_all, no_take
GRUPPEN: spieler, monster, haende
*/

/*
FUNKTION: query_put_messages
DEKLARATION: string *query_put_messages(object wer, object was)
BESCHREIBUNG:
Soll ein Objekt <was> von einem Spieler <wer> in einen Container getan
werden, so wird im Container diese Funktion aufgerufen. Hiermit kann der
Container die Meldungen erzeugen, die dabei ausgegeben werden sollen.
Alles weitere analog zu query_take_messages.
Es gibt kein set_put_messages und wird auch keines geben. Diese Funktion
ist fuer spezielle Spezialfaelle gedacht.
VERWEISE: put, no_put, query/set_put_prepos, query/set_put_verb usw.,
          query_take_messages
GRUPPEN: monster, spieler, haende
*/

/*
FUNKTION: allowed_put_living
DEKLARATION: int allowed_put_living(object was, object wohin)
BESCHREIBUNG:
Wenn das Lebewesen 'was' in den Container 'wohin' bewegt werden soll, und
'was' etwas mehr als 1 wiegt, wird was->allowed("put_living", was, wohin)
gefragt. allowed ruft dann in allen mit was->add_controller(
"allowed_put_living", other) angemeldeten Objekten other die
Funktion other->allowed_put_living(was, wohin) auf.

Wurde mindestens ein solches Objekt angemeldet und liefern alle
Funktionen einen Wert != 0, dann wird die Bewegung gestattet, ansonsten
verboten.
Somit kann man also Lebewesen bauen, welche man weitergeben kann, und die
doch schwerer als 1 sind, indem man einen solchen Controller anmeldet,
welcher 1 liefert.
VERWEISE: allowed, add_controller, take, forbidden_take, forbidden_take_me,
          allowed_take_living
GRUPPEN: monster, haende
*/

/*
FUNKTION: put
DEKLARATION: varargs int put(mixed was, object wohin, int flag, string verb)
BESCHREIBUNG:
Mit put kann man das Objekt was in den Container wohin legen.
Wenn flag == 1, dann werden keine Meldungen an andere erzeugt.
VERWEISE: no_out, put_all, take, take_all, no_take
GRUPPEN: spieler, monster, haende
*/
varargs int put(mixed ob, object con, int flag, string verb, string desc)
{
   int res, hand, con_verb_fall;
   string prepos, why_not, *put_messages, qverb, move_type;
   mixed con_verb;
   object econ;

   if(!objectp(ob))
   {
      notify_message("Das geht nicht.\n",MA_PUT);
      return MOVE_NOT_ALLOWED;
   }

   if(ob == con)
   {
      notify_message("Etwas in sich selbst reintun?\n",MA_PUT);
      return MOVE_NOT_ALLOWED;
   }
   qverb = convert_umlaute(verb) || query_verb_ascii();

   if(con != environment() && living(ob) && ob->query_weight() > 1 &&
      !ob->allowed("put_living", ob, con))
   {
      notify_message(wrap(Er(ob)+" hätte das sicher nicht gerne."),MA_PUT);
      return MOVE_NOT_ALLOWED;
   }

   if(this_object()->forbidden("put", ob, con) ||
      ob->forbidden("put_me", this_object(), con) ||
      con->forbidden("put_into", this_object(), ob) ||
      ob->no_put(this_object(), con))
      return MOVE_NOT_ALLOWED;

   if (econ = check_outer_containers(con,this_object()))// put
   {
      notify_message(wrap(Der(econ)+" ist geschlossen."),MA_PUT);
      return MOVE_NOT_ALLOWED;
   }

   desc = ob->query_name() ? desc = einen(ob) : "irgendetwas";
   if (put_messages = con->query_put_messages(this_object(),ob))
      if ((!pointerp(put_messages)) || (sizeof(put_messages)!=2))
         put_messages = 0;

   switch(qverb[0..2])
   {
      case "geb":
      case "gib":
         verb = "gebe";
         move_type = MOVE_TYPE_GEBEN;
         break;

      case "ste":
         if ((strlen(qverb) > 3) && (qverb[3] == 'l'))
         {
             verb = "stelle";
             move_type = MOVE_TYPE_STELLEN;
         }
         else
         {
             verb = "stecke";
             move_type = MOVE_TYPE_STECKEN;
         }
         break;

      case "sch":
         verb = "schenke";
         move_type = MOVE_TYPE_SCHENKEN;
         break;

      case "hae":
         verb = "hänge";
         move_type = MOVE_TYPE_HAENGEN;
         break;

      case "ver":
         verb = "verstaue";
         move_type = MOVE_TYPE_VERSTAUEN;
         break;

      default:
         verb = "lege";
         move_type = MOVE_TYPE_LEGEN;
         break;
   }

   res = ob->move(con, ([MOVE_TYPE: move_type]));
   if (!this_object()) return MOVE_DESTRUCTED;
   if ((res != MOVE_OK) && ob && (why_not = ob->query_not_moved_reason()))
      why_not = wrap(why_not);

   switch(res)
   {
       case MOVE_OK:
       case MOVE_DESTRUCTED:
	  if (!flag)
	  {
	      if(!con || con == environment()) {
		 if (verb == "stelle") {
		    this_object()->send_message(MT_LOOK,MA_PUT,
		       put_messages && put_messages [1]
		       ? wrap_if_not_empty(put_messages [1])
		       : wrap(Der()+" stellt "+desc+" hin."));
		    notify_message(
		       put_messages && put_messages [0]
		       ? wrap_if_not_empty(put_messages [0])
		       : wrap("Du stellst "+desc+" hin."),MA_PUT);
		 } else {
		    this_object()->send_message(MT_LOOK,MA_PUT,
		       put_messages && put_messages [1]
		       ? wrap_if_not_empty(put_messages [1])
		       : wrap(Der()+" legt "+desc+" hin."));
		    notify_message(
		       put_messages && put_messages [0]
		       ? wrap_if_not_empty(put_messages [0])
		       : wrap("Du legst "+desc+" hin."),MA_PUT);
		 }
	      }
	      else if(living(con))
	      {
		 if (verb == "schenke") {
		    this_object()->send_message(MT_LOOK,MA_PUT,
		       put_messages && put_messages [1]
		       ? wrap_if_not_empty(put_messages [1])
		       : wrap(Der()+" schenkt "+dem(con)+" "+desc+"."),
		       wrap(Der()+" schenkt dir "         +desc+"."),con);
		    notify_message(
		       put_messages && put_messages [0]
		       ? wrap_if_not_empty(put_messages [0])
		       : wrap("Du schenkst "+dem(con)+" "+desc+"."),MA_PUT);
		 } else {
		    this_object()->send_message(MT_LOOK,MA_PUT,
		       put_messages && put_messages [1]
		       ? wrap_if_not_empty(put_messages [1])
		       : wrap(Der()+" gibt "+dem(con)+" "+desc+"."),
		       wrap(Der()+" gibt dir "         +desc+"."),con);
		    notify_message(
		       put_messages && put_messages[0]
		       ? wrap_if_not_empty(put_messages[0])
		       : wrap("Du gibst "+dem(con)+" "+desc+"."),MA_PUT);
		 }
	      }
	      else
	      {
		 prepos = con->query_put_prepos() || "in";
		 if (con_verb = con->query_put_verb())
		 {
		    if(pointerp(con_verb))
		    {
			mixed verb_candidaten = filter(con_verb, (:!strstr($2,$1):), qverb);
			if(sizeof(verb_candidaten))
			    con_verb = verb_candidaten[0];
			else
			    con_verb = con_verb[0];
		    }
		    con_verb_fall = con->query_put_verb_case() || FALL_AKK;
		    this_object()->send_message(MT_LOOK,MA_PUT,
		       put_messages && put_messages[1]
		       ? wrap_if_not_empty(put_messages[1])
		       : wrap(Der()+" "+con_verb+"t "+desc+" "+prepos+" "+
		       (con_verb_fall == FALL_DAT ? seinem(con) : seinen(con))
		       +"."));
		    notify_message(
		       put_messages && put_messages[0]
		       ? wrap_if_not_empty(put_messages[0])
		       : wrap("Du "+
		          (con_verb[<1]=='z'?con_verb+"t ":con_verb+"st ")
		          +desc+" "+prepos+" "+
			 (con_verb_fall == FALL_DAT ? deinem(con) : deinen(con))
			 +"."),MA_PUT);
		 }
		 else
		 {
		    this_object()->send_message(MT_LOOK,MA_PUT,
		       put_messages && put_messages[1] 
		       ? wrap_if_not_empty(put_messages[1])
		       : wrap(Der()+(verb == "stecke" ? " steckt " : 
			     (verb == "hänge" ? " hängt " :
			     (verb == "stelle" ? " stellt " : " legt ")))+
		       desc+" "+prepos+" "+seinen(con)+"."));
		    notify_message(
		       put_messages && put_messages [0]
		       ? wrap_if_not_empty(put_messages [0])
		       : wrap("Du"+(verb == "stecke" ? " steckst " :
				 (verb == "hänge" ? " hängst " :
				 (verb == "stelle" ? " stellst " : " legst ")))+
			 desc+" "+prepos+" "+deinen(con)+"."),MA_PUT);
		 }
	      }
	  }
	  if ((hand = member(hand_objects, ob)) > -1)
	  {
	     if(ob)
		ob->do_remove();
	     hand_objects[hand] = 0;
	     hand_enemies[hand] = 0;
         hand_hits[hand] = 0;
	  }
	  if(this_object())
              this_object()->notify("put", ob, con);
	  if(ob)
	     ob->notify("put_me", this_object(), con);
          if(con)
             con->notify("put_into", this_object(), ob);
	  break;  
       case MOVE_NOT_ALLOWED:
	  notify_message(why_not || "Das geht nicht.\n",MA_PUT);
	  return -1;
       case MOVE_NO_ROOM:
	  notify_message(why_not || "Nicht genug Platz.\n",MA_PUT);
	  break;
       case MOVE_ENV_CLOSED:
	  notify_message(why_not || "Du stehst unter einem Bann.\n",MA_PUT);
	  break;
       case MOVE_DEST_CLOSED:
	  if (con == environment())
	     notify_message("Hier kannst du nichts fallenlassen.\n",MA_PUT);
	  else if (living(con))
	     notify_message(wrap(Der(con)+ " steht unter einem Bann; "+ihm(con)+
		   " kannst du nichts geben."),MA_PUT);
	  else 
	     notify_message(wrap(Der(con) + ist(con,1) + " geschlossen."),MA_PUT);
   }
   return res;
}

/*
FUNKTION: forbidden_put
DEKLARATION: int forbidden_put(object ob, object where, object who)
BESCHREIBUNG:
Bevor ein Objekt ob von einem Lebewesen who in den Container where gelegt
werden kann, wird who->forbidden("put", ob, where) aufgerufen, liefert
dieser Aufruf einen Wert ungleich 0 zurueck, wird ob nicht hineingelegt.

Die Funktion forbidden ruft in allen mit who->add_controller("forbidden_put",
 other) angemeldeten Objekten other die Funktionen other->forbidden_put(ob,
where, who) auf. Liefert auch nur eine dieser Funktionen einen Wert ungleich
0, dann returnt forbidden diesen und das Objekt ob kann nicht hineingelegt
werden.

Fuer die Ausgabe einer Meldung an das Lebewesen who muss der Programmierer
in forbidden_put oder forbidden, falls er diese Funktion ueberlagern will,
sorgen.

Bemerkung: Es wird auch ob->forbidden("put_me", who, where) 
           und where->forbidden("put_into", who, ob) aufgerufen.
VERWEISE: forbidden, notify, notify_put, put, no_put
GRUPPEN: spieler, monster, haende
*/

/*
FUNKTION: forbidden_put_me
DEKLARATION: int forbidden_put_me(object who, object where, object ob)
BESCHREIBUNG:
Bevor ein Objekt ob von einem Lebewesen who in den Container where gelegt
werden kann, wird ob->forbidden("put_me", who, where) aufgerufen, liefert
dieser Aufruf einen Wert ungleich 0 zurueck, wird ob nicht hineingelegt.

Die Funktion forbidden ruft in allen mit ob->add_controller(
"forbidden_put_me", other) angemeldeten Objekten other die Funktionen
other->forbidden_put_me(who, where, ob) auf. Liefert auch nur eine dieser
Funktionen einen Wert ungleich 0, dann returnt forbidden diesen und das Objekt
ob kann nicht hineingelegt werden.

Fuer die Ausgabe einer Meldung an das Lebewesen who muss der Programmierer
in forbidden_put_me oder forbidden, falls er diese Funktion ueberlagern will,
sorgen.

Bemerkung: Es wird auch who->forbidden("put", ob, where) 
           und where->forbidden("put_into", who, ob) aufgerufen.
VERWEISE: forbidden, notify, notify_put, put, no_put
GRUPPEN: spieler, monster, haende
*/

/*
FUNKTION: forbidden_put_into
DEKLARATION: int forbidden_put_into(object who, object ob, object where)
BESCHREIBUNG:
Bevor ein Objekt ob von einem Lebewesen who in den Container where gelegt
werden kann, wird where->forbidden("put_into", who, ob) aufgerufen, liefert
dieser Aufruf einen Wert ungleich 0 zurueck, wird ob nicht hineingelegt.

Die Funktion forbidden ruft in allen mit where->add_controller(
"forbidden_put_into", other) angemeldeten Objekten other die Funktionen
other->forbidden_put_into(who, ob, where) auf. Liefert auch nur eine dieser
Funktionen einen Wert ungleich 0, dann returnt forbidden diesen und das
Objekt ob kann nicht hineingelegt werden.

Fuer die Ausgabe einer Meldung an das Lebewesen who muss der Programmierer
in forbidden_put_into oder forbidden, falls er diese Funktion ueberlagern will,
sorgen.

Bemerkung: Es wird auch who->forbidden("put", ob, where) 
           und ob->forbidden("put_me", who, where) aufgerufen.
VERWEISE: forbidden, notify, notify_put, put, no_put
GRUPPEN: spieler, monster, haende
*/

/*
FUNKTION: notify_put
DEKLARATION: void notify_put(object ob, object where, object who)
BESCHREIBUNG:
Nachdem ein Objekt ob von einem Lebewesen who in den Container where gelegt
wurde, wird who->notify("put", ob, where) aufgerufen.

Die Funktion notify ruft in allen mit who->add_controller("notify_put",
 other) angemeldeten Objekten other die Funktionen other->notify_put(ob,
where, who) auf. Sowohl who als auch other haben dann die Moeglichkeit, auf
das Legen von ob in where zu reagieren.

Bemerkung: Es wird auch ob->notify("put_me", who, where) 
           und where->notify("put_into", who, ob) aufgerufen.
VERWEISE: forbidden, notify, forbidden_put, put, no_put
GRUPPEN: spieler, monster, haende
*/

/*
FUNKTION: notify_put_me
DEKLARATION: void notify_put_me(object who, object where, object ob)
BESCHREIBUNG:
Nachdem ein Objekt ob von einem Lebewesen who in den Container where gelegt
wurde, wird ob->notify("put_me", who, where) aufgerufen.

Die Funktion notify ruft in allen mit ob->add_controller(
"notify_put_me", other) angemeldeten Objekten other die Funktionen
other->notify_put_me(who, where, ob) auf. Sowohl ob als auch other haben
dann die Moeglichkeit, auf das Legen von ob in where zu reagieren.

Bemerkung: Es wird auch who->notify("put", ob, where) 
           und where->notify("put_into", who, ob) aufgerufen.
VERWEISE: forbidden, notify, forbidden_put, put, no_put
GRUPPEN: spieler, monster, haende
*/

/*
FUNKTION: notify_put_into
DEKLARATION: void notify_put_into(object who, object ob, object where)
BESCHREIBUNG:
Nachdem ein Objekt ob von einem Lebewesen who in den Container where gelegt
wurde, wird where->notify("put_into", who, ob) aufgerufen.

Die Funktion notify ruft in allen mit where->add_controller(
"notify_put_into", other) angemeldeten Objekten other die Funktionen
other->notify_put_into(who, ob, where) auf. Sowohl where als auch other haben
dann die Moeglichkeit, auf das Legen von ob in where zu reagieren.

Bemerkung: Es wird auch who->notify("put", ob, where) 
           und ob->notify("put_me", who, where) aufgerufen.
VERWEISE: forbidden, notify, forbidden_put, put, no_put
GRUPPEN: spieler, monster, haende
*/

/*
FUNKTION: put_all
DEKLARATION: int put_all(mixed *ob_list, object con)
BESCHREIBUNG:
Mit put_all kann man mehrere Objekte obj_list in den Container con legen.
VERWEISE: put, no_put, take, take_all, no_take
GRUPPEN: spieler, monster, haende
*/
int put_all(mixed *ob_list, object con)
{
    int i, f;
    string *names = ({});

    for (i=0; i<sizeof(ob_list); i++)
    {
	CHECK_EVAL_COST
	if (ob_list[i] != con && objectp (ob_list[i]) &&
	    !ob_list[i]->query_no_move())
	{
	    string name = einen(ob_list[i]);
	    mixed pmsg;
	    int res;
	    
	    pmsg = con->query_put_messages(this_object(),ob_list[i]);
	    if(!pointerp(pmsg) || sizeof(pmsg)!=2)
		pmsg = 0;
	    res = put(ob_list[i],con,pmsg?0:1);
	    if(!pmsg && (res==MOVE_OK || res==MOVE_DESTRUCTED))
		names += ({name});
            f++;
        }
    }
    if(sizeof(names))
    {
	if(con == environment())
	{
	    if ((query_verb()||"")[0..3] == "stel")
	    {
		this_object()->send_message(MT_LOOK,MA_PUT,
			wrap(Der()+" stellt "+liste(names)+" hin."));
		notify_message(wrap("Du stellst "+liste(names)+" hin."),MA_PUT);
	    }
	    else
	    {
		this_object()->send_message(MT_LOOK,MA_PUT,
			wrap(Der()+" legt "+liste(names)+" hin."));
		notify_message(wrap("Du legst "+liste(names)+" hin."),MA_PUT);
	    }
	}
	else if(living(con))
	{
	    if ((query_verb()||"")[0..2] == "sch")
	    {
		this_object()->send_message(MT_LOOK,MA_PUT,
		    wrap(Der()+" schenkt "+dem(con)+" "+liste(names)+"."),
		    wrap(Der()+" schenkt dir "+liste(names)+"."),con);
		notify_message(
		    wrap("Du schenkst "+dem(con)+" "+liste(names)+"."),MA_PUT);
	    }
	    else
	    {
		this_object()->send_message(MT_LOOK,MA_PUT,
		    wrap(Der()+" gibt "+dem(con)+" "+liste(names)+"."),
		    wrap(Der()+" gibt dir "+liste(names)+"."),con);
		notify_message(
		    wrap("Du gibst "+dem(con)+" "+liste(names)+"."),MA_PUT);
	    }
	}
	else
	{
	    string prepos = con->query_put_prepos() || "in";
	    mixed con_verb = con->query_put_verb();
	    int con_verb_fall;
	    if(!con_verb)
	    {
		switch((query_verb()||"")[0..3])
		{
		    case "stel":
			con_verb = "stell";
			break;
		    case "stec":
			con_verb = "steck";
			break;
		    case "haen":
			con_verb = "häng";
			break;
		    default:
			con_verb = "leg";
		}
		con_verb_fall = FALL_AKK;
	    }
	    else
	    {
		if(pointerp(con_verb))
		{
		    mixed tmp = filter(con_verb, (:!strstr($2,$1) || !strstr($1,$2):), query_verb());
		    if(sizeof(tmp))
		        con_verb = tmp[0];
		    else
		        con_verb = con_verb[0];
		}
		con_verb_fall = con->query_put_verb_case() || FALL_AKK;
	    }

	    this_object()->send_message(MT_LOOK,MA_PUT,
		wrap(Der()+" "+con_verb+"t "+liste(names)+" "+prepos+" "+
		    (con_verb_fall == FALL_DAT ? seinem(con) : seinen(con))
		    +"."));
	    notify_message(
		wrap("Du "+(con_verb[<1]=='z'?con_verb+"t ":con_verb+"st ")+
		    liste(names)+" "+prepos+" "+
		    (con_verb_fall == FALL_DAT ? deinem(con) : deinen(con))
		    +"."),MA_PUT);
	}
    }
    if (!f)
	notify_message("Du hast nichts bei Dir, mit dem das ginge.\n",MA_PUT);
    return 1;
}

varargs int put_countob(string ob_name, object ob, object con, int flag, string verb, string desc)
{
    int count, max, ret;
    object countob;

    max = ob->query_count();

    if (sscanf(ob_name,"%d %s",count,ob_name) != 2 || count == max)
	return put(ob,con,flag,verb,&desc);

    if (count < 1)
	WFAIL("Wie soll das gehen?\n");
    if (count > max)
	WFAIL("Soviel hast du nicht.\n");

    countob=ob->split_object(count);
    if (!countob)
	WFAIL("Das geht nicht.\n");
    ret=put(countob,con,flag,verb,&desc);
    if (ret != MOVE_OK) {
	if (countob)
	    countob->remove();
	ob->add_count(count);
	}
    return ret;
}

mixed parse_put_command(string str, string verb)
{
    string trenner, error_msg, ob_name;
    int trenner_verlangt;
    string * moegliche_trenner;
    mixed con, *parsed, *nparsed, *obs, *tobs;

    if (!verb)
	verb = "lege";
    else 
        verb = convert_umlaute(verb);

    switch(verb[0..2]) {
	case "geb":
	case "gib": verb = "gebe";
		    break;
	case "ste": if ((strlen(query_verb()) > 3) && (query_verb()[3] == 'l'))
			 verb = "stelle";
		    else verb = "stecke";
		    break;
	case "hae": verb = "hänge";
		    break;
        case "sch": verb = "schenke";
                    break;
	default:    verb = "lege";
	}

    if (!str)
    {
	if (query_notify_fail())
	    return 0;
	switch(verb) {
	   case "gebe":
	      return notify_fail("Gib was an wen?\n", FAIL_NOT_OBJ);
	   case "stecke":
	      return notify_fail("Stecke was wohin?\n", FAIL_NOT_OBJ);
	   case "hänge":
	      return notify_fail("Hänge was woran?\n", FAIL_NOT_OBJ);
	   case "stelle":
	      return notify_fail("Stelle was wohin?\n", FAIL_NOT_OBJ);
	   case "schenke":
	      return notify_fail("Schenke wem was?\n", FAIL_NOT_OBJ);
	   default:
	      return notify_fail("Lege was?\n", FAIL_NOT_OBJ);
	   }
    }

    str = space(lower_case(str));

    switch(verb)
    {
      case "lege":
	moegliche_trenner=({"in","ins","auf","aufs"});
      case "stelle":
	if (str[<4..]==" hin")
       	{
	    str=str[0..<5];
	    con = environment();
	}
	else if (str[<3..]==" ab")
       	{
	    str=str[0..<4];
	    con = environment();
	}
	else
	    trenner_verlangt=-1;
	break;
      case "hänge":
	if (str[<5..]==" dran")
       	{
	    str=str[0..<6];
	}
        else if (str[<5..]==" dazu")
       	{
	    str=str[0..<6];
	}
	else if (str[<3..]==" an")
       	{
	    str=str[0..<4];
	}
	break;
      case "stecke":
	trenner_verlangt=1;
	break;
    }

    if (trenner_verlangt)
    {
	// verb == "stecke" | "lege" | "stelle"
	// Hier muss in parsed dann das Objekt stehen, in das gelegt werden
	// soll
#ifdef SUCH_ERST_IN_MIR
	// Erstmal in einem selber suchen, dann erst im environment
	// Hier ist die Frage, ob das seim muss ? (Freaky)
	parsed = parse_com(str,this_object(),moegliche_trenner||({"in","ins","auf","aufs","an","ans"}),
			   PARSE_AFTER_TRENNER | PARSE_NOT_WITHOUT_TRENNER);
#else
	parsed = parse_com(str,0,moegliche_trenner||({"in","auf","an","ins","aufs","ans"}),
			   PARSE_AFTER_TRENNER | PARSE_NOT_WITHOUT_TRENNER);
#endif
	// Wenn kein Trenner da war, aber zwingend ein Trenner verlangt war,
	// dann ist was falsch angegeben worden.
	// Ansonsten ist es etwas in der Art 'lege WAS'
	if (parsed[PARSE_RET_CODE] == PARSE_NOT_SEARCHED)
       	{
	    if (trenner_verlangt == 1)
		TEST_FAIL(CAP(query_verb())+" was wohin?\n");
	    else
		con = environment();
	}
	else
       	{
	    // Hier war ein Trenner angegeben. Mal sehen, ob etwas gefunden
	    // wurde.
#ifdef SUCH_ERST_IN_MIR
	    // wenn in mir nichts gefunden wurde, im environment suchen
	    if (!parsed[PARSE_NUM_OBS])
		parsed = parse_com(str,environment(),({"in","auf","an","ins","aufs","ans"}),
			     PARSE_AFTER_TRENNER | PARSE_NOT_WITHOUT_TRENNER);
#endif
	    trenner = parsed[PARSE_TRENNER];
	    if (parse_com_error(parsed,CAP(query_verb())+" was "+trenner+
								" was?\n",
		    CAP(trenner)+" mehrere Sachen gleichzeitig was "+
								verb+"n?\n"))
		return 0;
	    con = parsed[PARSE_OBS][0];
	}
    }
    else if (!con)
    {
	// verb == "gebe" || "schenke"
	// tja.. hier wird es kompliziert :(
	// hier wird als 'con' IMMER ein Living verlangt
	// Faelle:
	// gebe WAS (an|zu) WEM
	// gebe WEM WAS
	// gebe WAS WEM
	// dito schenke
	// Ich gehe hier davon aus, dass das Living NICHT in mir ist !!!
	parsed = parse_com(str,environment(),({"an","zu"}),
			   PARSE_AFTER_TRENNER | PARSE_NOT_WITHOUT_TRENNER);
	if (parsed[PARSE_RET_CODE] == PARSE_NOT_SEARCHED)
       	{
	    // Ohje.. jetzt wird es uebel: kein Trenner :(
	    // jetzt sollte auch noch gehen: 'gebe alles freaky'
	    // parse_com bekommt das aber nicht hin...
	    // schwierig ist auch: 'gebe alles rote freaky'
	    // was tun ? Ich lasse es erstmal weg.
	    parsed = parse_com(str);
	    if (parse_com_error(parsed,CAP(query_verb())+" was an wen?\n"))
		return 0;
	    tobs=parsed[PARSE_OBS];
	    // Mal sehen, ob das Teil schon 'con' sein koennte
	    if ((sizeof(tobs) == 1) && objectp(tobs[0]) && living(tobs[0]))
		con = tobs[0];
	    // Syntax: 'gebe OBJEKT1 OBJEKT2'
	    // in parsed steht das Ergebnis von OBJEKT1
	    // in nparsed steht das Ergebnis von OBJEKT2
	    nparsed = parse_com(parsed[PARSE_REST],con ? this_object() : 0);
	    if (con)
		error_msg = CAP(query_verb())+" was an "+den(con)+"?\n";
	    else
		error_msg = CAP(query_verb())+" was an wen?\n";
	    if (parse_com_error(nparsed,error_msg))
		return 0;
	    
	    if (!con)
	    {
		// Mal sehen, ob das neue Teil ein Lebewesen ist.
		tobs = nparsed[PARSE_OBS];
		if ((sizeof(tobs) == 1) && objectp(tobs[0]) && living(tobs[0]))
		    con = tobs[0];
		else
	       	{
		    // huh. Jetzt ist es aber komisch.. er will einem nicht-
		    // living etwas geben....
		    if (parsed[PARSE_NUM_OBS] == 1)
			con = parsed[PARSE_OBS][0];
		    else if (nparsed[PARSE_NUM_OBS] == 1)
			con = nparsed[PARSE_OBS][0];
		    else
			return notify_fail("Bitte nicht immer alles auf einmal.\n", FAIL_NOT_OBJ);
		    return notify_fail(wrap(Dem(con)+" kannst Du nichts "+verb+"n."), FAIL_INTERNAL);
		}
		// So jetzt ist 'con' gefunden. (con war OBJEKT2)
		// Ueberpruefen, ob die gefundenen Objekte aus mir sind,
		// wenn nicht, neu suchen.
		obs = parsed[PARSE_OBS];
		if ((objectp(obs[0]) && (environment(obs[0]) != this_object()))
			||
		    (mappingp(obs[0]) &&(obs[0]["environment"]!=this_object()))
		   )
	       	{
		    parsed = parse_com(str,this_object());
		    if (parse_com_error(parsed,CAP(query_verb())+" was an "+
						    dem(con)+"?\n"))
			return 0;
		    obs = parsed[PARSE_OBS];
		}
		ob_name = parsed[PARSE_ID];
	    }
	    else
	    {
		// jetzt muss man str abschneiden.
		str = parsed[PARSE_REST];
	    }
	}
	else
       	{
	    // Super.. Trenner gefunden :)
	    trenner = parsed[PARSE_TRENNER];
	    if (!parsed[PARSE_NUM_OBS])
		// mhh.. er hat es nicht gefunden.. suchen wir mal in mir.
		// um im Zweifelsfall eine bessere Fehlermeldung zu haben
		parsed = parse_com(str,0,({trenner}),PARSE_AFTER_TRENNER);
	    if (parse_com_error(parsed,CAP(query_verb())+" was "+trenner+
			" wen?\n","Etwas an mehrere Leute gleichzeitig "+
			verb+"n?\n"))
		return 0;
	    con = parsed[PARSE_OBS][0];
	}
    }

    if (!obs)
    {
	// Alle Objekte finden, die in 'con' gelegt werden sollen
	parsed = parse_com(str,this_object(),trenner ? ({trenner}) : 0);
	if (parse_com_error(parsed,CAP(query_verb()) + " was?\n"))
	    return 0;
	obs = parsed[PARSE_OBS];
	ob_name = parsed[PARSE_ID];
    }

    if (!objectp(con))
    {
	if (trenner)
	    return notify_fail(wrap(CAP(trenner)+" "+den(con)+" kann man nichts "+verb+"n."), FAIL_WRONG_ARG);
	return notify_fail(wrap(Dem(con) + " kann man nichts " + verb + "n."), FAIL_WRONG_ARG);
    }
    if (con == this_object()) 
	return notify_fail("Dir selbst?\n", FAIL_WRONG_ARG);
    if (sizeof(obs & hand_objects) <= 0 && free_hand() < 0) 
	return notify_fail("Du hast keine Hand frei.\n", FAIL_INTERNAL);
    return ({obs,con,ob_name});
}

int put_command(string str)
{
    mixed obs, con, *ret;

    ret = this_object()->parse_put_command(str,query_verb());
    if (!ret)
	return 0;
    obs = ret[0];
    con = ret[1];

    if (sizeof(obs) > 1)
	return put_all(obs,con);
    if (!objectp(obs[0]))
	return notify_fail(wrap("Mit " + dem(obs[0]) + " geht das nicht."), FAIL_WRONG_ARG);
    if (obs[0]==this_object())
    {
	if(con==environment())
	    return notify_fail("Wohin willst Du Dich legen?\n", FAIL_NOT_OBJ);
	else
	    return notify_fail("Darauf kannst Du Dich nicht legen.\n", FAIL_NOT_OBJ);
    }
#if 0
    else if (obs[0]->query_count())
	put_countob(ret[2],obs[0],con);
#endif
    else if (environment(obs[0]) == con) 
	return notify_fail(wrap(Der(obs[0]) + ist(obs[0],1) + " doch schon dort."), FAIL_INTERNAL);
    else
	put(obs[0],con);
    return 1;
}

/* =====================  E  M  P  T  Y  ======================== */
/*
FUNKTION: allowed_empty_living
DEKLARATION: int allowed_empty_living(object woraus,object room)
BESCHREIBUNG:
Wenn das Lebewesen 'woraus' ausgeleert werden soll, wird 
woraus->allowed("empty_living", woraus,room) gefragt. 

Wurde mindestens ein solches Objekt angemeldet und liefern alle
Funktionen einen Wert != 0, dann wird die Bewegung gestattet, ansonsten
verboten.
Somit kann man also Lebewesen bauen, welche man leeren kann, indem man einen 
solchen Controller anmeldet, welcher 1 liefert.
VERWEISE: allowed, add_controller, empty, allowed_take_living, 
        allowed_put_living
GRUPPEN: monster, haende
*/

/*
FUNKTION: forbidden_empty_container
DEKLARATION: int forbidden_empty_container(object con, object room, object who)
BESCHREIBUNG:
Bevor ein Container con von einem Lebewesen who in den Raum room geleert
werden kann, wird who->forbidden("empty_container", con,room) aufgerufen, 
liefert dieser Aufruf einen Wert ungleich 0 zurueck, wird con nicht geleert.

Fuer die Ausgabe einer Meldung an das Lebewesen who muss der Programmierer
in forbidden_empty_container oder forbidden, falls er diese Funktion 
ueberlagern will, sorgen.

Bemerkung: Es wird auch con->forbidden("empty_container_me", who, room) 
           und room->forbidden("empty_container_into", who, con) aufgerufen.
VERWEISE: forbidden, notify
GRUPPEN: spieler, monster, haende
*/
/*
FUNKTION: forbidden_empty_container_me
DEKLARATION: int forbidden_empty_container_me(object who, object room, object con)
BESCHREIBUNG:
Bevor ein Container con von einem Lebewesen who in den Raum room geleert
werden kann, wird con->forbidden("empty_container_me", who,room) aufgerufen, 
liefert dieser Aufruf einen Wert ungleich 0 zurueck, wird con nicht geleert.

Fuer die Ausgabe einer Meldung an das Lebewesen who muss der Programmierer
in forbidden_empty_container_me oder forbidden, falls er diese Funktion 
ueberlagern will, sorgen.

Bemerkung: Es wird auch who->forbidden("empty_container", con, room) 
           und room->forbidden("empty_container_into", who, con) aufgerufen.
VERWEISE: forbidden, notify
GRUPPEN: spieler, monster, haende
*/
/*
FUNKTION: forbidden_empty_container_into
DEKLARATION: int forbidden_empty_container_into(object who, object con, object room)
BESCHREIBUNG:
Bevor ein Container con von einem Lebewesen who in den Raum room geleert
werden kann, wird room->forbidden("empty_container_into", who,con) aufgerufen, 
liefert dieser Aufruf einen Wert ungleich 0 zurueck, wird con nicht geleert.

Fuer die Ausgabe einer Meldung an das Lebewesen who muss der Programmierer
in forbidden_empty_container_into oder forbidden, falls er diese Funktion 
ueberlagern will, sorgen.

Bemerkung: Es wird auch who->forbidden("empty_container", con, room) 
           und con->forbidden("empty_container_me", who, room) aufgerufen.
VERWEISE: forbidden, notify
GRUPPEN: spieler, monster, haende
*/

/*
FUNKTION: notify_empty_container
DEKLARATION: void notify_empty_container(object con, object room, object who)
BESCHREIBUNG:
Nachdem ein Container con von einem Lebewesen who in den Raum room geleert
wurde, wird who->notify("empty_container", con,room) aufgerufen.

Bemerkung: Es wird auch room->notify("empty_container_into", who, con) 
           und con->notify("empty_container_me", who, room) aufgerufen.
VERWEISE: forbidden, notify,forbidden_empty_container_into
GRUPPEN: spieler, monster, haende
*/

/*
FUNKTION: notify_empty_container_me
DEKLARATION: void notify_empty_container_me(object who, object room, object con)
BESCHREIBUNG:
Nachdem ein Container con von einem Lebewesen who in den Raum room geleert
wurde, wird con->notify("empty_container_me", who,room) aufgerufen.

Bemerkung: Es wird auch who->notify("empty_container", con, room) 
           und room->notify("empty_container_into", who, con) aufgerufen.
VERWEISE: forbidden, notify,forbidden_empty_container_into
GRUPPEN: spieler, monster, haende
*/

/*
FUNKTION: notify_empty_container_into
DEKLARATION: void notify_empty_container_into(object who, object con, object room)
BESCHREIBUNG:
Nachdem ein Container con von einem Lebewesen who in den Raum room geleert
wurde, wird room->notify("empty_container_into", who,con) aufgerufen.

Bemerkung: Es wird auch who->notify("empty_container", con, room) 
           und con->notify("empty_container_me", who, room) aufgerufen.
VERWEISE: forbidden, notify,forbidden_empty_container_into
GRUPPEN: spieler, monster, haende
*/

private int empty(object ob,object con,object room)
{
    int res;
    string desc,why_not;
    if(ob->query_name())
        desc = ob->query_invis() & V_ATOM_HIDDEN || auto_owner_search(ob) ?
            einen(ob) :
	    wen(ob, ART_AAA);
    else
        desc = "irgendetwas";
    desc = capitalize(desc);
    if (ob == con || con == room)
    {
        notify_message(desc+": Das geht nicht.",MA_TAKE);
        return MOVE_NOT_ALLOWED;
    }
    // take-Controller aus con beruecksichtigen:
    if(this_object()->forbidden("take", ob, con) ||
       ob->forbidden("take_me", this_object(), con) ||
       con->forbidden("take_from", this_object(), ob) ||
       ob->no_take(this_object(), con))
       return MOVE_NOT_ALLOWED;
    // put Controller in room beruecksichtigen.
    if(this_object()->forbidden("put", ob, room) ||
      ob->forbidden("put_me", this_object(), room) ||
      room->forbidden("put_into", this_object(), ob) ||
      ob->no_put(this_object(), room))
      return MOVE_NOT_ALLOWED;
      

    res = ob->move(room, ([MOVE_TYPE: MOVE_TYPE_LEEREN]));
    switch (res)
    {
        case MOVE_OK:
            ({ this_object() })->notify("take", ob, con);
            ({ ob })->notify("take_me", this_object(), con);
            ({ con })->notify("take_from", this_object(), ob);
            ({ this_object() })->notify("put", ob, room);
            ({ ob })->notify("put_me", this_object(), room);
            ({ room })->notify("put_into", this_object(), ob);
            break;
        case MOVE_DESTRUCTED:
            break;
        default:
            if (ob)
                why_not=ob->query_not_moved_reason();
            switch (res)
            {
                case MOVE_NOT_ALLOWED :
                    notify_message(wrap(desc+": "+
                        (why_not || "Das geht nicht.")),MA_MOVE_IN);
                    break;
                case MOVE_NO_ROOM :
                    notify_message(wrap(desc+": Kein Platz."),MA_MOVE_IN);
                    break;
                default:
                    notify_message(wrap(desc+": Das geht nicht!"),MA_MOVE_IN);
                    break;
            }
            break;
    }
    return res;
}

private int empty_all(object *ob_list, object con,object room)
{
    string * names = ({});
    string where,mywhere;
    if (present(con,this_object()))
    {
        where = " aus "+seinem(con)+" aus";
        mywhere = " aus "+deinem(con)+" aus";
    }
    else
    {
        where = " aus "+dem(con)+" aus";
        mywhere = " aus "+dem(con)+" aus";
    }
    foreach(object ob: ob_list)
    {
        CHECK_EVAL_COST
        if (objectp(ob))
        {
            string name = einen(ob);
            int res = empty(ob,con, room);
            if (res==MOVE_OK || res == MOVE_DESTRUCTED)
                names += ({name});
        }
    }
    if(sizeof(names))
    {
        this_object()->send_message(MT_LOOK, MA_MOVE_IN,
            wrap(Der()+" leert "+liste(names)+where+"."));
        notify_message(
            wrap("Du leerst "+liste(names)+mywhere+"."), MA_MOVE_IN);
    }
    if (this_object())
        this_object()->notify("empty_container", con, room);
    if (con)
        con->notify("empty_container_me", this_object(), room);
    if (room)
        room->notify("empty_container_into", this_object(), con);
    return sizeof(ob_list) && 1;
}

int empty_command(string str)
{
    mixed * parsed;
    object con,room,*inv;

    parsed = parse_com(str,0,0,PARSE_NO_V_ITEMS);
    if(parse_com_error(parsed, "Was willst Du leeren?\n", 1))
            return 0;
    if (sizeof(parsed[PARSE_OBS])==1 && objectp(parsed[PARSE_OBS][0]))
    {
        con = parsed[PARSE_OBS][0];
    }
    else
    {
        return notify_fail("Was willst du leeren?\n", FAIL_NOT_OBJ);
    }
    if (!con->query_container())
    {
        return notify_fail(wrap(Der(con)+plural(" lässt "," lassen ",con)+
            "sich nicht leeren."), 
            FAIL_WRONG_ARG);
    }
    if (con->query_con_close())
    {
        return notify_fail(wrap(Der(con)+plural(" lässt "," lassen ",con)
            +"sich geschlossen nicht leeren."), 
            FAIL_INTERNAL);
    }
    room = environment(this_player());
    if (!room || !room->query_room())
    {
        return notify_fail(wrap("Hier"+plural(" lässt "," lassen ",con)
            +"sich "+der(con)+" nicht leeren."), 
            FAIL_INTERNAL);
    }
    inv = all_inventory(con);
    if (!sizeof(inv))
    {
        return notify_fail(wrap(Der(con)+plural(" ist "," sind ",con)
            +"schon leer."), 
            FAIL_INTERNAL);
    }
    if(this_object()->forbidden("empty_container", con, room) ||
      con->forbidden("empty_container_me", this_object(), room) ||
      room->forbidden("empty_container_into", this_object(), con))
        return 1;
    if (living(con) && !con->allowed("empty_living", con, room))
    {
        return notify_fail(wrap(Der(con)+plural(" lässt "," lassen ",con)
            +"sich nicht leeren!"), 
            FAIL_INTERNAL);
    }
    empty_all(inv,con,room);
    return 1;
}


/* =====================  S  H  O  W  ======================== */



/*
FUNKTION: notify_shown
DEKLARATION: void notify_shown(mixed what, object who)
BESCHREIBUNG:

ACHTUNG! Diese Funktion ist veraltet. Stattdessen sollte notify_show_whom
genutzt werden, welches dem Controllerkonzept entspricht.

Wird einem Monster irgendetwas <what> von einem Lebewesen <who> gezeigt,
so wird in dem Monster die Funktion notify_shown aufgerufen. Deren erster
Parameter ist das, was gezeigt wurde, dies kann vom Typ object (Gegenstand)
oder aber auch ein mapping (v-item) sein. Der zweite Parameter ist das
Lebewesen, das dem Monster etwas gezeigt hat.
VERWEISE: show_command
GRUPPEN: monster
*/

/*
FUNKTION: show_command
DEKLARATION: int show_command(string str)
BESCHREIBUNG:
Mit show_command kann man ein Monster/einen Spieler etwas zeigen lassen. 
Was genaum, wird aus str herausgeparst. show_command liefert 1 zurueck, wenn
etwas gezeigt wurde, sonst 0.
VERWEISE: notify_shown
GRUPPEN: spieler, monster, haende
*/
int show_command(string str)
{
    mixed   *parsed, ob, pl;
    object  owner_ob, owner_pl, *excludes;
    string  tmp, wem_str, was_str;
    mixed   zustaendig, zufun, zutmp;
    int     prio;

    parsed = parse_com(str);
    if(parse_com_error(parsed,CAP(query_verb())+" wem was?\n",1))
	return 0;
    pl = parsed[PARSE_OBS][0];
    parsed = parse_com(parsed[PARSE_REST]);
    if(parse_com_error(parsed,CAP(query_verb())+" wem was?\n",1))
	return 0;
    ob = parsed[PARSE_OBS][0];

//    raus, weil das konfusion schaft:  zeige vitem an ob1 vitem an ob2
//    geht dann nicht
//    if(objectp(ob) && living(ob) && !(objectp(pl) && living(pl)))
//    {    object swap = pl, pl = ob, ob = swap;    }

    if(((objectp(ob) && member(hand_objects,ob)<0) || mappingp(ob))
	 && free_hand() < 0)
	return notify_fail("Du hast keine Hand frei.\n", FAIL_INTERNAL);

    if(objectp(pl) && pl == this_object())
	return notify_fail("Dir selbst was zeigen? Kannst du nicht schauen?\n", FAIL_WRONG_ARG);

    if(this_object()->do_forbiddens(C_RESORT, "show", 
        ({"", "_me", "_whom"}), ({this_object(), ob, pl})))
        return 1;

    if ((objectp (pl) && !living (pl) && !pl->allowed("seele"))
        || (mappingp (pl) && !pl["living"]))
        return notify_fail(wrap(Der(pl)+" würde kaum etwas davon haben."), FAIL_WRONG_ARG);
    
    owner_ob = auto_owner_search(ob);
    owner_pl = auto_owner_search(pl);
    excludes = ({});

    // deutsch sucks.

    // erst Message an den der zeigt
    if(!owner_ob)
       was_str = wen(ob, ART_AAA);
    else if(owner_ob == this_object())
       was_str = deinen(ob);
    else if(owner_ob == pl || owner_ob && owner_ob == owner_pl)
       was_str = seinen(ob);
    else 
       was_str = ihren(ob, 0, 0, 0, 0, ART_DEIN);

    if (mappingp(pl) && owner_pl != this_object())
        wem_str = ihrem(pl, 0, 0, 0, 0, ART_DEIN);
    else
        wem_str = deinem(pl);

    if (ob == this_object())
        notify_message(wrap("Du zeigst dich "+wem_str+"."), MA_LOOK);
    else
        notify_message(wrap("Du zeigst "+wem_str+" "+was_str+"."),MA_LOOK);
    excludes += ({this_object()});

    // Message an den der was gezeigt bekommt
    if (objectp(pl) && member(excludes, pl) == -1)
    {
        if (ob == this_object())
            this_object()->send_message_to(
                pl, MT_LOOK, MA_LOOK,
                wrap(Der()+" zeigt sich dir:"));
        else if (ob == pl)
            this_object()->send_message_to(
                pl, MT_LOOK, MA_LOOK,
                wrap(Der()+" zeigt dich dir:"));
        else
        {
            if(!owner_ob)
                was_str = wen(ob, ART_AAA);
            else if(owner_ob == this_object())
                was_str = seinen(ob);
            else if(owner_ob == pl)
                was_str = deinen(ob);
            else 
                was_str = ihren(ob, 0, 0, 0, 0, ART_SEIN);
            
            this_object()->send_message_to(
                pl, MT_LOOK, MA_LOOK,
                wrap(Der()+" zeigt dir "+was_str+":"));
        }
        
        excludes += ({pl});
    }
    
    // Message an jemanden dessen v-item was gezeigt bekommt
    if (mappingp(pl) && owner_pl && member(excludes, owner_pl) == -1)
    {
        if (ob == this_object())
            this_object()->send_message_to(
                owner_pl, MT_LOOK, MA_LOOK,
                wrap(Der()+" zeigt sich "+deinem(pl)+"."));
        if (ob == owner_pl)
            this_object()->send_message_to(
                owner_pl, MT_LOOK, MA_LOOK,
                wrap(Der()+" zeigt dich "+deinem(pl)+"."));
        else
        {
            if(!owner_ob)
                was_str = wen(ob, ART_AAA);
            else if(owner_ob == this_object())
                was_str = seinen(ob);
            else if(owner_ob == owner_pl)
                was_str = deinen(ob);
            else 
                was_str = ihren(ob, 0, 0, 0, 0, ART_SEIN);
            
            this_object()->send_message_to(
                owner_pl, MT_LOOK, MA_LOOK,
                wrap(Der()+" zeigt "+deinem(pl)+" "+
                     was_str+"."));
        }
        
        excludes += ({owner_pl});
    }

    // Message an jemanden der hergezeigt wird (nicht sich selbst)
    if (objectp(ob)  &&  member(excludes, ob) == -1)
    {
        if (mappingp(pl) && owner_pl != this_object())
            wem_str = ihrem(pl, 0, 0, 0, 0, ART_DEIN);
        else
            wem_str = seinem(pl);

        this_object()->send_message_to(
            ob, MT_LOOK, MA_LOOK,
            wrap(Der()+" zeigt dich "+wem_str+"."));
        
        excludes += ({ ob });
    }
    
    // Message an jemanden dessen v-item hergezeigt wird (nicht sich selbst)
    if (mappingp(ob)  &&  owner_ob  &&  member(excludes, owner_ob) == -1)
    {
        if (mappingp(pl) && owner_pl != this_object())
            wem_str = ihrem(pl, 0, 0, 0, 0, ART_DEIN);
        else
            wem_str = seinem(pl);

        this_object()->send_message_to(
            owner_ob, MT_LOOK, MA_LOOK,
            wrap(Der()+" zeigt "+wem_str+" "+deinen(ob)+"."));
        
        excludes += ({ owner_ob });
    }
    
    // Message an Unbeteiligte
    if(!owner_ob)
       was_str = wen(ob, ART_AAA);
    else if(owner_ob == this_object())
       was_str = seinen(ob);
    else 
       was_str = ihren(ob, 0, 0, 0, 0, ART_SEIN);

    if (mappingp(pl) && owner_pl != this_object())
        wem_str = ihrem(pl, 0, 0, 0, 0, ART_SEIN);
    else
        wem_str = seinem(pl);

    if (ob == this_object())
        this_object()->send_message(
            MT_LOOK, MA_LOOK,
            wrap(Der()+" zeigt sich "+wem_str+"."),
            0, excludes);
    else
        this_object()->send_message(
            MT_LOOK, MA_LOOK,
            wrap(Der()+" zeigt "+wem_str+" "+was_str+"."),
            0, excludes);
	    
    zustaendig = this_object()->concerned(&prio, "show", ob, pl);
    zufun = ({ "do_show", ob, pl, this_object() }); 
    
    if(objectp(ob))
    {
	zutmp = ob->concerned(&prio, "show_me", this_object(), pl);
	if(zutmp)
	{
	    zustaendig = zutmp;
	    zufun = ({ "do_show_me", this_object(), pl, ob });
	}
    }
    if(objectp(pl))
    {
	zutmp = pl->concerned(&prio, "show_whom", this_object(), ob);
	if(zutmp)
	{
	    zustaendig = zutmp;
	    zufun = ({ "do_show_whom", this_object(), ob, pl });
	}
    }

    if(zustaendig)
    {
	if(objectp(zustaendig))
	    apply(#'call_other, zustaendig, zufun);
	else
	    apply(zustaendig, zufun);
    }
    else if(objectp(pl))
    {
	mixed env = ob, eenv = ob;
	
	while(eenv)
	{
	    env = eenv;
	    if(mappingp(env))
		eenv = env["environment"];
	    else
		eenv = environment(env);
	}
	
	// Manche V-Items haben keinen environment-Eintrag. :-(
	if(!objectp(env))
	    env = environment();
	
	if(QUERY("visible_in_the_dark",ob) || pl->can_see(env))
	{
	    if (tmp = pl->query_object_description(ob))
		this_object()->send_message_to(pl, MT_LOOK, MA_LOOK, tmp);
	}
	else
        {
	    this_object()->send_message_to(pl, MT_LOOK, MA_LOOK,
		"Nur leider ist es zu dunkel, du kannst nichts erkennen.\n");
	}
    }

    if (objectp(pl))
        pl->notify_shown(ob, this_object());

    this_object()->do_notifies(C_RESORT, "show",
        ({"", "_me", "_whom"}), ({this_object(), ob, pl}));

    return 1;
}

/*
FUNKTION: forbidden_show
DEKLARATION: int forbidden_show(mixed what, mixed whom, object who)
BESCHREIBUNG:
Bevor das Objekt oder Mapping (v-item) what dem Lebewesen (evntl. V-Item)
whom vom Lebewesen who gezeigt werden kann, wird who->forbidden("show", what,
whom) aufgerufen, liefert dieser Aufruf einen Wert ungleich 0 zurueck, kann
das Objekt nicht gezeigt werden.

Die Funktion forbidden ruft in allen mit who->add_controller("forbidden_show",
 other) angemeldeten Objekten other die Funktionen other->forbidden_show(what,
whom, who) auf. Liefert auch nur eine dieser Funktionen einen Wert ungleich
0, dann returnt forbidden diesen und what kann nicht gezeigt werden.

Fuer die Ausgabe einer Meldung an das Lebewesen who muss der Programmierer
in forbidden_show oder forbidden, falls er diese Funktion ueberlagern will,
sorgen.

Bemerkung: Es wird auch what->forbidden("show_me", who, whom) 
           und whom->forbidden("show_whom", who, what)
           und room->forbidden("show_here", who, what, whom) aufgerufen.
VERWEISE: forbidden, notify, notify_show
GRUPPEN: spieler, monster, haende
*/

/*
FUNKTION: forbidden_show_me
DEKLARATION: int forbidden_show_me(object who, mixed whom, object what)
BESCHREIBUNG:
Bevor das Objekt what dem Lebewesen (moeglicherweise V-Item) whom vom
Lebewesen who gezeigt werden kann, wird what->forbidden("show_me", who, whom)
aufgerufen, liefert dieser Aufruf einen Wert ungleich 0 zurueck, kann das
Objekt nicht gezeigt werden.

Die Funktion forbidden ruft in allen mit what->add_controller(
"forbidden_show_me", other) angemeldeten Objekten other die Funktionen
other->forbidden_show_me(who, whom, what) auf. Liefert auch nur eine dieser
Funktionen einen Wert ungleich 0, dann returnt forbidden diesen und what kann
nicht gezeigt werden.

Fuer die Ausgabe einer Meldung an das Lebewesen who muss der Programmierer
in forbidden_show_me oder forbidden, falls er diese Funktion ueberlagern will,
sorgen.

Bemerkung: Es wird auch who->forbidden("show", what, whom) 
           und whom->forbidden("show_whom", who, what) 
           und room->forbidden("show_here", who, what, whom) aufgerufen.
VERWEISE: forbidden, notify, notify_show
GRUPPEN: spieler, monster, haende
*/

/*
FUNKTION: forbidden_show_whom
DEKLARATION: int forbidden_show_whom(object who, mixed what, object whom)
BESCHREIBUNG:
Bevor das Objekt oder Mapping (v-item) what dem Lebewesen whom vom
Lebewesen who gezeigt werden kann, wird whom->forbidden("show_whom", who,
what) aufgerufen, liefert dieser Aufruf einen Wert ungleich 0 zurueck, kann
das Objekt nicht gezeigt werden.

Die Funktion forbidden ruft in allen mit whom->add_controller(
"forbidden_show_whom", other) angemeldeten Objekten other die Funktionen
other->forbidden_show_whom(who, what, whom) auf. Liefert auch nur eine dieser
Funktionen einen Wert ungleich 0, dann returnt forbidden diesen und what kann
nicht gezeigt werden.

Fuer die Ausgabe einer Meldung an das Lebewesen who muss der Programmierer
in forbidden_show_whom oder forbidden, falls er diese Funktion ueberlagern
will, sorgen.

Bemerkung: Es wird auch who->forbidden("show", what, whom) 
           und what->forbidden("show_me", who, whom)
           und room->forbidden("show_here", who, what, whom) aufgerufen.
VERWEISE: forbidden, notify, notify_show
GRUPPEN: spieler, monster, haende
*/

/*
FUNKTION: forbidden_show_here
DEKLARATION: int forbidden_show_here(object who, mixed what, mixed whom)
BESCHREIBUNG:
Wenn das Objekt who das Objekt/V-Item what dem Objekt/V-Item whom zeigt,
wird zuvor in der Raum, in dem diese Aktion stattfindet, mit dem Aufruf
room->forbidden("show_here", who, what, whom) gefragt, ob diese Aktion
nicht durch irgendetwas verhindert wird. Objekte koennen sich beim Raum
fuer diesen Controller anmelden.

Funktioniert ansonsten wie forbidden_show(), siehe dort.
VERWEISE: forbidden, notify, forbidden_show, notify_show
GRUPPEN: spieler, monster, haende
*/

/*
FUNKTION: notify_show
DEKLARATION: void notify_show(mixed what, mixed whom, object who)
BESCHREIBUNG:
Nachdem das Lebewesen who einem Lebewesen (evntl. V-Item) whom das Objekt
oder Mapping (v_item) what gezeigt hat, wird who->notify("show", what, whom)
aufgerufen.

Die Funktion notify ruft in allen mit who->add_controller("notify_show",
 other) angemeldeten Objekten other die Funktionen other->notify_show(what,
whom, who) auf. Sowohl who als auch other haben dann die Moeglichkeit, auf
das Zeigen von what zu reagieren.

Bemerkung: Es wird auch what->notify("show_me", who, whom) 
           und whom->notify("show_whom", who, what)
           und room->notify("show_here", who, what, whom) aufgerufen.
VERWEISE: forbidden, notify, forbidden_show, notify_shown
GRUPPEN: spieler, monster, haende
*/

/*
FUNKTION: notify_show_me
DEKLARATION: void notify_show_me(object who, mixed whom, mixed what)
BESCHREIBUNG:
Nachdem das Lebewesen who einem Lebewesen (moeglicherweise ein V-Item) whom
das Objekt oder Mapping (v_item) what gezeigt hat, wird what->notify(
"show_me", who, whom) aufgerufen.

Die Funktion notify ruft in allen mit what->add_controller(
"notify_show_me", other) angemeldeten Objekten other die Funktionen
other->notify_show_me(who, whom, what) auf. Sowohl what als auch other haben
dann die Moeglichkeit, auf das Zeigen von what zu reagieren.

Bemerkung: Es wird auch who->notify("show", what, whom) 
           und whom->notify("show_whom", who, what)
           und room->notify("show_here", who, what, whom) aufgerufen.
VERWEISE: forbidden, notify, forbidden_show, notify_shown
GRUPPEN: spieler, monster, haende
*/

/*
FUNKTION: notify_show_whom
DEKLARATION: void notify_show_whom(object who, mixed what, object whom)
BESCHREIBUNG:
Nachdem das Lebewesen who einem Lebewesen whom das Objekt oder Mapping
(v_item) what gezeigt hat, wird whom->notify("show_whom", who, what)
aufgerufen.

Die Funktion notify ruft in allen mit whom->add_controller(
"notify_show_whom", other) angemeldeten Objekten other die Funktionen
other->notify_show_whom(who, what, whom) auf. Sowohl whom als auch other haben
dann die Moeglichkeit, auf das Zeigen von what zu reagieren.

Bemerkung: Es wird auch who->notify("show", what, whom) 
           und what->notify("show_me", who, whom)
           und room->notify("show_here", who, what, whom) aufgerufen.
VERWEISE: forbidden, notify, forbidden_show, notify_shown
GRUPPEN: spieler, monster, haende
*/

/*
FUNKTION: notify_show_here
DEKLARATION: int forbidden_show_here(object who, mixed what, mixed whom)
BESCHREIBUNG:
Wenn das Objekt who das Objekt/V-Item what dem Objekt/V-Item whom gezeigt
hat, wird der Raum, in dem diese Aktion stattfindet, mit dem Aufruf
room->notify("show_here", who, what, whom) davon benachrichtigt.
Objekte koennen sich beim Raum fuer diesen Controller anmelden.

Funktioniert ansonsten wie notify_show(), siehe dort.
VERWEISE: forbidden, notify, notify_show, notify_show
GRUPPEN: spieler, monster, haende
*/

/*
FUNKTION: concerned_show
DEKLARATION: int concerned_show(object ob, object whom, object who)
BESCHREIBUNG:
Bevor ein Objekt ob von einem Lebewesen who dem whom gezeigt
wird, wird who->concerned("show", ob, whom) aufgerufen.

Die Returnwerte der per controller bei 'ob' angemeldeten Objekten 
werden als Prioritaten angesehen, wobei das Objekt 'cob' mit der 
hoechsten Prioritaet den Zuschlag erhaelt und per 
cob->do_show(ob,whom,who); dazu aufgefordert wird
sich um das Zeigen von 'ob' zu kuemmern.

Bemerkung: Es wird auch whom->concerned("show_whom", who, ob) 
           und ob->concerned("show_me", who, whom) aufgerufen.
VERWEISE: concerned, concerned_take_from, concerned_take, do_take
GRUPPEN: spieler, monster, haende
*/

/*
FUNKTION: concerned_show_me
DEKLARATION: int concerned_show_me(object who, object whom, object ob)
BESCHREIBUNG:
Bevor ein Objekt ob von einem Lebewesen who dem whom gezeigt
wird, wird ob->concerned("show_me", who, where) aufgerufen.

Die Returnwerte der per controller bei 'ob' angemeldeten Objekten 
werden als Prioritaten angesehen, wobei das Objekt 'cob' mit der 
hoechsten Prioritaet den Zuschlag erhaelt und per 
cob->do_show_me(who,whom,ob); dazu aufgefordert wird
sich um das Zeigen von 'ob' zu kuemmern.

Bemerkung: Es wird auch who->concerned("show", ob, where) 
           und whom->concerned("show_whom", who, ob) aufgerufen.
VERWEISE: concerned, concerned_show_whom, concerned_show, do_show_me
GRUPPEN: spieler, monster, haende
*/

/*
FUNKTION: concerned_show_whom
DEKLARATION: int concerned_show_whom(object who, object ob, object whom)
BESCHREIBUNG:
Bevor ein Objekt ob von einem Lebewesen who dem whom gezeigt
wird, wird whom->concerned("show_whom", who, ob) aufgerufen.

Die Returnwerte der per controller bei 'whom' angemeldeten Objekten 
werden als Prioritaten angesehen, wobei das Objekt 'cob' mit der 
hoechsten Prioritaet den Zuschlag erhaelt und per 
cob->do_show_whom(who,ob,whom); dazu aufgefordert wird
sich um das Zeigen von 'ob' zu kuemmern.

Bemerkung: Es wird auch who->concerned("show", ob, whom) 
           und ob->concerned("show_me", who, whom) aufgerufen.
VERWEISE: concerned, concerned_show_me, concerned_show, do_show_whom
GRUPPEN: spieler, monster, haende
*/

/*
FUNKTION: do_show_whom
DEKLARATION: int do_show_whom(object who, object ob, object whom)
BESCHREIBUNG:
Diese Funktion wird in dem Objekt aufgerufen, welches ueber 
concerned_show_whom die hoechste Prioritaet zurueckgeliefert hat 
und sich somit dafuer zustaendigst erklaert hat sich um das Zeigen 
von 'ob' durch 'who' an 'whom' zu kuemmern.
Es sollte 1 im Erfolgsfall zurueck gegeben werden.
VERWEISE: concerned_whow_whom, do_show_me, do_show
GRUPPEN: spieler, monster, haende
*/


/*
FUNKTION: do_show_me
DEKLARATION: int do_show_me(object who, object whom, object ob)
BESCHREIBUNG:
Diese Funktion wird in dem Objekt aufgerufen, welches ueber 
concerned_show_me die hoechste Prioritaet zurueckgeliefert hat 
und sich somit dafuer zustaendigst erklaert hat sich um das Zeigen 
von 'ob' durch 'who' an 'whom' zu kuemmern.
Es sollte 1 im Erfolgsfall zurueck gegeben werden.
VERWEISE: concerned_show_me, do_show, do_show_whom
GRUPPEN: spieler, monster, haende
*/



/*
FUNKTION: do_show
DEKLARATION: int do_show(object ob, object whom, object who)
BESCHREIBUNG:
Diese Funktion wird in dem Objekt aufgerufen, welches ueber 
concerned_show die hoechste Prioritaet zurueckgeliefert hat 
und sich somit dafuer zustaendigst erklaert hat sich um das Zeigen 
von 'ob' durch 'who' an 'whom' zu kuemmern.
Es sollte 1 im Erfolgsfall zurueck gegeben werden.
VERWEISE: concerned_show, do_show_me, do_show_whom
GRUPPEN: spieler, monster, haende
*/

/* =====================  P  U  S  H  ======================== */

/*
FUNKTION: no_push
DEKLARATION: int no_push(object who, object where)
BESCHREIBUNG:
Bevor ein Objekt geschoben wird (schiebe Xyz nach Norden) wird in dem zu
schiebenden Objekt die Funktion no_push aufgerufen, who ist das Lebewesen,
das zu schieben versucht, where ist das Ziel des Schiebens.
Antwortet no_push mit 0, so wird das Objekt zu schieben versucht (Filter,
Gewicht usw. koennten das ja trotzdem verhindern), liefert no_push einen
Wert ungleich 0 so wird das Objekt nicht geschoben, no_push muss eine
Meldung an who ausgeben (warum es nicht ging) und sollte in den Raum
ebenfalls eine geben.
Beispiel: gezuendetes Dynamit kann man nicht mehr verschieben:
    int no_push(object who, object wohin)
    {
       if(gezuend) {
          send_message_to(who,MT_NOTIFY|MT_LOOK,MA_UNKNOWN,
	     wrap("Im gegenwaertigen Zustand sollte man das Dynamit nicht"
                  " umeinanderschieben!"));
          return 1;
       }
    }
VERWEISE: 
GRUPPEN: spieler, monster, haende
*/


int push_command(string str)
{
   mixed ob, target_ob, exit_info;
   mixed * parsed;
   string target, why_not;
   string dir, rdir, reason, msg_out, msg_in, msg_self, msg_ob;
   int raumtyp, im_wasser; // da schiebts sich schlecht

   if(!str || sscanf(str, "%s nach %s", str, dir) != 2)
   {
      if(str)
          dir = lower_case(explode(str," ")[<1]);
	  
      if(dir != "hoch" && dir != "runter")
         return notify_fail("Schiebe was nach wohin?\n", FAIL_NOT_OBJ);
	 
      str = str[0..<strlen(dir)+2];
   }
   dir = expand_direction(lower_case(dir), DIR_ALS_DEFAULT);
   parsed = parse_com(str);
   if (parse_com_error(parsed, "Schiebe was nach wohin?\n",1))
      return 0;
   if (free_hand() < 0)
      return notify_fail("Du hast keine Hand frei.\n", FAIL_INTERNAL);
   if(mappingp(ob = parsed[PARSE_OBS][0]))
      return notify_fail(wrap(Den(ob)+" bekommst Du hier nie weg!"), FAIL_WRONG_ARG);
   if(!present(ob, environment()))
      return notify_fail(wrap(Der(ob)+" muss vor Dir liegen, sonst geht's nicht!"), FAIL_WRONG_ARG);
   if(IS_INVIS(ob))
      return notify_fail("So etwas kannst Du hier nirgends entdecken.\n", FAIL_NOT_OBJ);
   if(dir == "oben")
      rdir = "hoch";
   else if(dir == "unten")
      rdir = "runter";
   else 
      rdir = dir;
   // Doppelte Abfrage von query_one_exit, da einige Raeume im
   // create (aufgerufen durchs touch) Ausgaenge sperren... :-(
   if(!(target = environment()->query_one_exit(rdir)) ||
      !(target_ob = touch(target)) ||
      !environment()->query_one_exit(rdir)) {
      if (reason = environment()->query_lock_reason (rdir))
        notify_fail(wrap ("Nach "+capitalize(dir)+" kannst Du "+den(ob)
          +" nicht schieben. "+reason));
      else
        notify_fail(wrap ("Nach "+capitalize(dir)+" kannst Du "+den(ob)
          +" nicht schieben."));
      return 0;
   }
   if(ob == this_object())
      return notify_fail("Dich selbst umeinanderschieben? Probier's mal mit laufen!\n", FAIL_WRONG_ARG);

   if(interactive(this_object()) && interactive(ob) && ob->query_age() < DAY)
      return notify_fail("Solch junges Gemüse soll ruhig selbst laufen!\n", FAIL_WRONG_ARG);

   if(IS_INVIS(this_object()) && living(ob))
      return notify_fail(wrap(Der(ob)+" weicht deinem Flimmern immer aus."), FAIL_WRONG_ARG);

   if(ob->no_push(this_object(), target_ob) ||
      this_object()->do_forbiddens(C_OMIT_OBJ, "push", ({"", "_me"}),
    	 ({this_object(), ob}), target_ob, dir))
      return 1;
   
   if(ob->query_no_move())
   {
      this_object()->send_message(MT_LOOK|MT_NOISE,MA_UNKNOWN,
        wrap(Der()+" stemmt sich gegen "+den(ob)+", ächzt und lässt davon ab."));
      if (why_not = ob->query_no_move_reason())
          notify_fail(wrap(why_not));
      else
          notify_fail(wrap(Den(ob)+" wirst Du nie von der Stelle bekommen!"));
      return 0;
   }
   raumtyp = environment()->query_type (LANDSCHAFT);
   if (raumtyp &&
      ((raumtyp & (L_WASSER | L_FLIESSEND | L_UNTERWASSER)) == raumtyp))
      im_wasser = 1;
   if(living(ob))
   {
      if(!query_once_interactive(ob))
      {
	 this_object()->send_message(MT_LOOK|MT_NOISE,MA_UNKNOWN,
	    wrap(Der()+" stemmt sich gegen "+den(ob)+
		  ", aber "+er(ob)+" wehrt sich!"),
	    wrap(Der()+" versucht Dich nach "+capitalize(dir)+" zu schubsen!"),
	    ob);
         if (im_wasser)
	     this_object()->send_message(MT_LOOK|MT_NOISE,MA_UNKNOWN,
	       "Plitsch, platsch, Du wirst ganz nassgespritzt!\n","",ob);
         this_object()->send_message_to(this_object(),
	    MT_LOOK|MT_NOISE|MT_FEEL, MA_UNKNOWN,
	    wrap(Der(ob) + " weiger" + plural("t","n",ob) +
	    " sich, sich von Dir umeinanderschieben zu lassen!"));
	 return 1;
      }
      else
      {
	 if(!interactive(ob) || query_idle(ob) > 60)
	    return notify_fail(wrap("Nicht doch, Du würdest "+den(ob)+
		 " noch aufwecken dadurch."), FAIL_INTERNAL);
	 this_object()->send_message(MT_LOOK|MT_NOISE,MA_UNKNOWN,
	    wrap(Der()+" stemmt sich gegen "+den(ob)+
		  "... Ein kurzes Kräftemessen... und..."),
	    wrap(Der()+" stemmt sich gegen Dich... "
			 "Ein kurzes Kräftemessen... und..."),ob);
	 notify_message(wrap("Du stemmst Dich gegen "+den(ob)+
		     "... Ein kurzes Kräftemessen... und..."));
         if(im_wasser)
	 {
	    this_object()->send_message(MT_LOOK|MT_NOISE,MA_UNKNOWN,
               "Plitsch, platsch, spritz! Du wirst "
               "ganz nassgespritzt!\nDie beiden drücken sich gegenseitig "
               "unter Wasser, es ist zum kringeln!\n",
               wrap ("Du versuchst, "+den()+" mit dem Kopf "
               "unter Wasser zu drücken.")+
               "Plitsch, platsch, spritz, keuch, prust...\n"+
               wrap ("Dies gelingt Dir auch; ärgerlicherweise gerätst Du "
               "dabei ebenfalls mit dem Kopf unter Wasser."),ob);
            this_object()->send_message_to(this_object(),
	        MT_LOOK|MT_NOISE|MT_FEEL, MA_UNKNOWN,
	        wrap(Der(ob)+" versucht, Dich mit "
               "Deinem Kopf unter Wasser zu drücken.")+
               "Plitsch, platsch, spritz, keuch, prust...\n"+
               wrap ("Dies gelingt "+ihm(ob)+" auch; erfreulicherweise gerät "
               +er(ob)+" dabei ebenfalls mit dem Kopf unter Wasser."));
	    return 1;
         }
	 if(ob->query_weight() + ob->query_stat(STAT_STR) >
	    this_object()->query_stat(STAT_STR))
	 {
	    this_object()->send_message(MT_LOOK|MT_NOISE,MA_UNKNOWN,
	        "Nichts passiert.\n");
	    this_player()->send_message_to(this_object(),
	        MT_LOOK|MT_NOISE|MT_FEEL, MA_UNKNOWN,
		"Du bist nicht stark genug.\n");
	    return 1;
	 }
      }
   }
   if((im_wasser && 2*ob->query_weight() > this_object()->query_stat(STAT_STR))
      ||(ob->query_weight() > this_object()->query_stat(STAT_STR)))
   {
      this_object()->send_message(MT_LOOK|MT_NOISE,MA_UNKNOWN,
	  wrap(Der()+" stemmt sich gegen "+den(ob)+
	  ", ächzt und lässt davon ab."));
      this_object()->send_message_to(this_object(),
          MT_LOOK|MT_NOISE|MT_FEEL, MA_UNKNOWN,
	  (im_wasser ? "Im Wasser schiebt es sich sehr schlecht.\n":"")
         +wrap(Der(ob)+ ist(ob,1) + " einfach zu schwer für Dich!"));
      return 1;
   }

   exit_info = environment()->query_exit_info(rdir);
   mapping move_info = ([
      MOVE_OLD_ROOM:  environment(),
      MOVE_NEW_ROOM:  environment(ob),
      MOVE_FLAGS:     MOVE_NORMAL,
      MOVE_DIRECTION: rdir,
      MOVE_EXIT_INFO: exit_info,
      MOVE_MSG_LEAVE: Der(this_object())+" schiebt "+den(ob)+" $dir() davon.",
      MOVE_MSG_ENTER: Ein(this_object())+" schiebt "+einen(ob)+" $dir() herbei.",
      MOVE_MSG_ME:    "Du schiebst "+den(ob)+" $dir() davon.",
   ]);

   msg_out = this_object().query_move_out_msg(move_info);
   msg_in = this_object().query_move_in_msg(move_info);
   msg_self = this_object().query_move_msg_self(move_info);
   msg_ob = this_object().query_move_msg_self(move_info + ([
      MOVE_MSG_ME:    Der(this_object())+" hat Dich $dir() geschoben."
   ]));

   // Freaky: hier darf man NICHT target_ob nehmen sondern MUSS rdir nehmen,
   //         da sonst filter_xxx nicht aufgerufen wird!
   if(!exit_info || !objectp(ob) ||
      ob->move(rdir, ([MOVE_TYPE: MOVE_TYPE_SCHIEBEN])) != MOVE_OK)
   {
      // Menaures: Manche Moves zerstoeren Objekte
      if(!objectp(ob)) 
         notify_fail(wrap("Was immer du schieben wolltest, ist verschwunden."));
      else if (exit_info && (why_not = ob->query_not_moved_reason()))
         notify_fail(wrap(why_not));
      else
         notify_fail(wrap("Du schaffst es nicht, "+den(ob)+" dorthin zu schieben."));
      return 0;
   }

   if(!objectp(ob))
   {
       // Menaures: Manche Objekte werden nach dem Move zerstoert :(
       return notify_fail(wrap("Was immer du schieben wolltest, ist veschwunden."));
   }

   if(msg_out)
      this_object()->send_message(MT_LOOK|MT_NOISE,MA_MOVE_OUT, msg_out, msg_self, this_object());
   if(msg_in)
      ob->send_message(MT_LOOK|MT_NOISE,MA_MOVE_IN, msg_in, msg_ob, ob);

   this_object()->move(rdir);

   if (im_wasser)
	this_object()->handle_swimming();

   if (playerp (ob)) this_object()->set_handeln();

   if(this_object())
       this_object()->do_notifies(C_OMIT_OBJ, "push", ({"", "_me"}),
           ({this_object(), ob}), target_ob, dir);
   return 1;
}

/*
FUNKTION: forbidden_push
DEKLARATION: int forbidden_push(object ob, object where, string dir, object who)
BESCHREIBUNG:
Bevor das Lebewesen who ein Objekt ob in Richtung dir in den Container
(Raum) where schieben kann, wird who->forbidden("push", ob, where, dir)
aufgerufen, liefert dieser Aufruf einen Wert ungleich 0 zurueck, darf ob
nicht geschoben werden.

Die Funktion forbidden ruft in allen mit who->add_controller("forbidden_push",
other) angemeldeten Objekten other die Funktionen other->forbidden_push(ob,
where, dir, who) auf. Liefert auch nur eine dieser Funktionen einen Wert
ungleich 0, dann returnt forbidden diesen und das Objekt ob kann nicht
geschoben werden.

Fuer die Ausgabe einer Meldung an das Lebewesen muss der Programmierer in
forbidden_push oder forbidden, falls er diese Funktion ueberlagern will,
sorgen.

Bemerkung: Es wird auch ob->forbidden("push_me", who, where, dir) aufgerufen. 
VERWEISE: forbidden, notify, notify_push, no_put
GRUPPEN: spieler, monster, haende
*/

/*
FUNKTION: forbidden_push_me
DEKLARATION: int forbidden_push_me(object who, object where, string dir, object ob)
BESCHREIBUNG:
Bevor das Lebewesen who ein Objekt ob in Richtung dir in den Container
(Raum) where schieben kann, wird ob->forbidden("push_me", who, where, dir)
aufgerufen, liefert dieser Aufruf einen Wert ungleich 0 zurueck, darf ob
nicht geschoben werden.

Die Funktion forbidden ruft in allen mit ob->add_controller(
"forbidden_push_me", other) angemeldeten Objekten other die Funktionen
other->forbidden_push_me(who, where, dir, ob) auf. Liefert auch nur eine
dieser Funktionen einen Wert ungleich 0, dann returnt forbidden diesen und
das Objekt ob kann nicht geschoben werden.

Fuer die Ausgabe einer Meldung an das Lebewesen muss der Programmierer in
forbidden_push_me oder forbidden, falls er diese Funktion ueberlagern will,
sorgen.

Bemerkung: Es wird auch who->forbidden("push", ob, where, dir) aufgerufen. 
VERWEISE: forbidden, notify, notify_push, no_put
GRUPPEN: spieler, monster, haende
*/

/*
FUNKTION: forbidden_push_here
DEKLARATION: int forbidden_push_here(object who, object ob, object where, string dir)
BESCHREIBUNG:
Bevor das Lebewesen who ein Objekt ob in Richtung dir in den Container
(Raum) where schieben kann, wird environment(who)->forbidden("push_here",
who, ob, where, dir) aufgerufen, liefert dieser Aufruf einen Wert ungleich 0
zurueck, darf ob nicht geschoben werden.

Die Funktion forbidden ruft in allen mit room->add_controller(
"forbidden_push_here", other) angemeldeten Objekten other die Funktionen
other->forbidden_push_here(who, ob, where, dir) auf. Liefert auch nur eine
dieser Funktionen einen Wert ungleich 0, dann returnt forbidden diesen und
das Objekt ob kann nicht geschoben werden.

Fuer die Ausgabe einer Meldung an das Lebewesen muss der Programmierer in
forbidden_push_here oder forbidden, falls er diese Funktion ueberlagern will,
sorgen.

Bemerkung: Es wird auch who->forbidden("push", ob, where, dir) und
ob->forbidden("push_me", who, where, dir) aufgerufen. 
VERWEISE: forbidden, notify, notify_push_here, no_put
GRUPPEN: spieler, monster, haende
*/

/*
FUNKTION: notify_push
DEKLARATION: void notify_push(object ob, object where, string dir)
BESCHREIBUNG:
Nachdem ein Objekt ob von dem Lebewesen who in Richtung dir in den Container
(Raum) where geschoben wurde, wird who->notify("push", ob, where, dir)
aufgerufen.

Die Funktion notify ruft in allen mit who->add_controller("notify_push", other)
angemeldeten Objekten other die Funktionen other->notify_push(ob, where, dir,
who) auf. Sowohl who als auch other haben dann eine Moeglichkeit, auf das
Schieben von ob in den Container (Raum) where zu reagieren.

Bemerkung: Es wird auch ob->notify("push_me", who, where, dir) aufgerufen. 
VERWEISE: forbidden, notify, forbidden_push
GRUPPEN: spieler, monster, haende
*/

/*
FUNKTION: notify_push_me
DEKLARATION: void notify_push_me(object who, object where, string dir, object ob)
BESCHREIBUNG:
Nachdem ein Objekt ob von dem Lebewesen who in Richtung dir in den Container
(Raum) where geschoben wurde, wird ob->notify("push_me", who, where, dir)
aufgerufen.

Die Funktion notify ruft in allen mit ob->add_controller("notify_push_me",
other) angemeldeten Objekten other die Funktionen other->notify_push_me(who,
where, dir, ob) auf. Sowohl ob als auch other haben dann eine Moeglichkeit,
auf das Schieben von ob in den Container (Raum) where zu reagieren.

Bemerkung: Es wird auch who->notify("push", ob, where, dir) aufgerufen. 
VERWEISE: forbidden, notify, forbidden_push
GRUPPEN: spieler, monster, haende
*/

/*
FUNKTION: notify_push_here
DEKLARATION: void notify_push_here(object who, object ob, object where, string dir)
BESCHREIBUNG:
Nachdem ein Objekt ob von dem Lebewesen who in Richtung dir in den Container
(Raum) where geschoben wurde, wird environment(who)->notify("push_here",
who, ob, where, dir) aufgerufen.

Die Funktion notify ruft in allen mit environment(who)->add_controller(
"notify_push_here", other) angemeldeten Objekten other die Funktionen
other->notify_push_here(who, ob, where, dir) auf.

Bemerkung: Es wird auch who->notify("push", ob, where, dir) und
ob->notify("push_me", who, where, dir) aufgerufen. 
VERWEISE: forbidden, notify, forbidden_push_me
GRUPPEN: spieler, monster, haende
*/

//
// Krams zum Befuehlen und Betasten von Zeugs
//

/*
FUNKTION: query_feel_msg
DEKLARATION: string query_feel_msg()
BESCHREIBUNG:
Diese Funktion wird in einem Objekt aufgerufen, wenn this_player() das
Objekt befuehlt. Liefert diese Funktion 0 zurueck, wird die Standardmeldung
"<TP> befuehlt <Objekt>." generiert, ansonsten wird diese Meldung
umgebrochen und ausgegeben.
VERWEISE: set_feel, query_feel
GRUPPEN: grundlegendes
*/
int feel_command(string str)
{
    mixed *parsed, ob, tmp;
    string text;
    object where, owner, *inv;

    if (free_hand() == -1)
        return notify_fail("Du hast keine Hand mehr frei.\n", FAIL_INTERNAL);
    if (!str || str == "hier" || str == "umher" || str == "hier umher"
             || str == "herum" || str == "hier herum"
             || str == "Umgebung" || str == "umgebung")
    {
        where = environment(this_player());

	if(this_object()->do_forbiddens(C_RESORT, "feel", ({ "", "_me"}),
	    ({this_object(), where})))
		return 1;

	if(where && (text = where->query_feel()))
	{
	   if (!IS_INVIS(this_object()))
	   {
	      if (tmp=where->query_feel_msg())
	      {
		 if (tmp != "")
		    this_object()->send_message(MT_LOOK,MA_FEEL,wrap(add_dot_to_msg(tmp)));
	      }
	      else
		 this_object()->send_message(MT_LOOK,MA_FEEL,
                                    wrap(Der()+" tastet wild umher."));
	   }
	   this_object()->send_message_to(this_object(),MT_FEEL,MA_FEEL,text);

	   this_object()->do_notifies(C_RESORT, "feel", ({ "", "_me"}),
	      ({this_object(), where}));

	   return 1;
	}

        if (where)
        {
            // Der Raum selber fuehlt sich nach nichts an.
            // Ein Objekt im Inventory suchen, vielleicht finden wir ja was.
            inv = all_inventory(where) - ({this_object()});

            while(sizeof(inv))
            {
                // Per Zufall ein Objekt aussuchen.
                ob = inv[random(sizeof(inv))];
                inv -= ({ob});

                if(ob->query_invis() == 0)
                {
                    // Ein Objekt gefunden, das nicht versteckt ist.
                    // Darf es befuehlt werden?
                    if(this_object()->do_forbiddens(C_RESORT, "feel",
                           ({"", "_me"}), ({this_object(), ob})))
                    {
                        return 1;
                    }

                    // Fuehltext abfragen.
                    text = ob->query_feel();

                    // Fuehlt es sich an?
                    if(!stringp(text))
                    {
                        // Nein, weitersuchen.
                        continue;
                    }

                    // Meldung ausgeben, falls wir nicht unsichtbar sind.
                    if(!IS_INVIS(this_object()))
                    {
                        tmp = ob->query_feel_msg();

                        if(!stringp(tmp))
                        {
                            this_object()->send_message(MT_LOOK, MA_FEEL,
                                wrap(Der()+" tastet wild umher."));
                        }

                        else if(strlen(tmp))
                        {
                            this_object()->send_message(MT_LOOK, MA_FEEL,
                                wrap(add_dot_to_msg(tmp)));
                        }
                    }

                    if(strlen(text))
                    {
                        this_object()->send_message_to(this_object(), 
                            MT_FEEL, MA_FEEL,
                        "Du tastest wild umher und findest mit Deinen Fingern "
                        "auch etwas:\n"
                        +text);
                    }

                    this_object()->do_notifies(C_RESORT, "feel", ({ "", "_me"}),
                        ({this_object(), ob}));

                    return 1;
                }
            }
        }

        notify_fail("Du tastest wild umher, "
                    "fühlst aber nix Besonderes.\n");
        return 0;
    }

    sscanf(str,"an %s",str);
    parsed = parse_com(str);
    if (query_verb()[0..1] != "be")
        tmp = "be"+query_verb();
        else tmp = query_verb();
    if (parse_com_error(parsed,tmp+" was?\n",1))
	return 0;
    ob = parsed[PARSE_OBS][0];

    if(this_object()->do_forbiddens(C_RESORT, "feel", ({ "", "_me"}),
        ({this_object(), ob})))
	    return 1;

    text = QUERY("feel",ob);

    if (!text)
    {
	if (mappingp(ob) && (tmp=QUERY("far",ob)))
	    notify_message(stringp(tmp)?tmp:
	        wrap(Der(ob) + ist(ob,1) +
		" viel zu weit weg, um "+ihn(ob)+" zu befühlen."),MA_FEEL);
        else
	    this_object()->send_message_to(this_object(),MT_FEEL,MA_FEEL,
	    	"Du fühlst nichts Ungewöhnliches.\n");

        this_object()->do_notifies(C_RESORT, "feel", ({ "", "_me"}),
            ({this_object(), ob}));

	return 1;
    }

    if (text == "")
    {
        this_object()->do_notifies(C_RESORT, "feel", ({ "", "_me"}),
            ({this_object(), ob}));

	return 1;
    }
    
    this_object()->send_message_to(this_object(),MT_FEEL,MA_FEEL,text);

    /* MELDUNG fuer OTHERS */

    if(!IS_INVIS(this_object()))
       if (ob == this_object())
	  this_object()->send_message(MT_LOOK,MA_FEEL,
              wrap(Der()+" tastet sich von oben bis unten ab, "
                   "es scheint aber noch alles dran zu sein."));

       else if (tmp=QUERY("feel_msg",ob))
       {
	  if (tmp != "")
	     this_object()->send_message(MT_LOOK,MA_FEEL,wrap(tmp));
       }
       else if (objectp(ob) && living(ob))
	  this_object()->send_message(MT_LOOK,MT_FEEL,
	     ob->query_invis() & V_ATOM_HIDDEN ?
		wrap(Der()+" betastet jemanden.") : 
		wrap(Der()+" betastet "+seinen(ob)+"."),
	     wrap(Der()+" betastet dich."), ob);
       else if (QUERY("invis",ob) & V_ATOM_HIDDEN)
	  this_object()->send_message(MT_LOOK,MA_FEEL,
              wrap(Der()+" befühlt etwas."));
       else
	  if((owner = auto_owner_search(ob)) && owner != this_object())
	     this_object()->send_message(MT_LOOK,MA_FEEL,
		wrap(Der()+" befühlt "+ihren(ob)+"."),
		wrap(Der()+" befühlt "+deinen(ob)+"."),
		owner);
	  else
	     this_object()->send_message(MT_LOOK,MA_FEEL,
                 wrap(Der()+" befühlt "+seinen(ob)+"."));

    this_object()->do_notifies(C_RESORT, "feel", ({ "", "_me"}),
        ({this_object(), ob}));

    return 1;
}

/*
FUNKTION: forbidden_feel
DEKLARATION: int forbidden_feel(mixed what, object who)
BESCHREIBUNG:
Bevor das Objekt oder Mapping (v-item) what vom Lebewesen who betastet
werden kann, wird who->forbidden("feel", what) aufgerufen, liefert dieser
Aufruf einen Wert ungleich 0 zurueck, kann das Objekt nicht betastet werden.

Die Funktion forbidden ruft in allen mit who->add_controller("forbidden_feel",
 other) angemeldeten Objekten other die Funktionen other->forbidden_feel(what,
who) auf. Liefert auch nur eine dieser Funktionen einen Wert ungleich
0, dann returnt forbidden diesen und what kann nicht gefuehlt werden.

Fuer die Ausgabe einer Meldung an das Lebewesen who muss der Programmierer
in forbidden_feel oder forbidden, falls er diese Funktion ueberlagern will,
sorgen. Dabei ist zu beachten, dass das Lebewesen what nicht gezielt
betastet haben kann, sondern es durch Herumtasten im Raum erwischt haben kann.

Bemerkung: Es wird auch what->forbidden("feel_me", who) und
           room->forbidden("feel_here", who, what) aufgerufen
VERWEISE: forbidden, notify, notify_feel
GRUPPEN: spieler, monster, haende
*/

/*
FUNKTION: forbidden_feel_me
DEKLARATION: int forbidden_feel_me(object who, mixed what)
BESCHREIBUNG:
Bevor das Objekt oder Mapping (v-item) what vom Lebewesen who betastet
werden kann, wird what->forbidden("feel", who) aufgerufen, liefert dieser
Aufruf einen Wert ungleich 0 zurueck, kann das Objekt nicht betastet werden.

Die Funktion forbidden ruft in allen mit what->add_controller(
"forbidden_feel_me", other) angemeldeten Objekten other die Funktionen
other->forbidden_feel_me(who, what) auf. Liefert auch nur eine dieser
Funktionen einen Wert ungleich 0, dann returnt forbidden diesen und what kann
nicht gefuehlt werden.

Fuer die Ausgabe einer Meldung an das Lebewesen who muss der Programmierer
in forbidden_feel_me oder forbidden, falls er diese Funktion ueberlagern will,
sorgen. Dabei ist zu beachten, dass das Lebewesen what nicht gezielt betastet
haben kann, sondern es durch Herumtasten im Raum erwischt haben kann.

Bemerkung: Es wird auch who->forbidden("feel", what) und
           room->forbidden("feel_here", who, what) aufgerufen. 
VERWEISE: forbidden, notify, notify_feel_me
GRUPPEN: spieler, monster, haende
*/

/*
FUNKTION: forbidden_feel_here
DEKLARATION: int forbidden_feel_here(object who, mixed what)
BESCHREIBUNG:
Bevor das Objekt oder Mapping (v-item) what vom Lebewesen who betastet
werden kann, wird im Raum room->forbidden("feel_here", who, what)
aufgerufen, liefert dieser Aufruf einen Wert ungleich 0 zurueck,
kann das Objekt nicht betastet werden.

Die Funktion forbidden ruft in allen mit room->add_controller(
"forbidden_feel_here", other) angemeldeten Objekten other die Funktionen
other->forbidden_feel_here(who, what) auf. Liefert auch nur eine dieser
Funktionen einen Wert ungleich 0, dann returnt forbidden diesen und what kann
nicht gefuehlt werden.

Fuer die Ausgabe einer Meldung an das Lebewesen who muss der Programmierer
in forbidden_feel_here oder forbidden, falls er diese Funktion ueberlagern
will, sorgen. Dabei ist zu beachten, dass das Lebewesen what nicht gezielt
betastet haben kann, sondern es durch Herumtasten im Raum erwischt haben kann.

Bemerkung: Es wird auch who->forbidden("feel", what) und
           what->forbidden("feel_me", who) aufgerufen. 
VERWEISE: forbidden, notify, notify_feel_here
GRUPPEN: spieler, monster, haende
*/

/*
FUNKTION: notify_feel
DEKLARATION: void notify_feel(mixed what, object who)
BESCHREIBUNG:
Nachdem das Objekt oder Mapping (v-item) what vom Lebewesen who betastet
wurde, wird who->notify("feel", what) aufgerufen.

Die Funktion notify ruft in allen mit who->add_controller("notify_feel",
other) angemeldeten Objekten other die Funktionen other->notify_feel(what,
who) auf. Sowohl who als auch other haben dann die Moeglichkeit, auf
das Fuehlen von what zu reagieren.

Bemerkung: Es wird auch what->notify("feel_me", who) und
           room->notify("feel_here", who, what) aufgerufen.
VERWEISE: forbidden, notify, forbidden_feel
GRUPPEN: spieler, monster, haende
*/

/*
FUNKTION: notify_feel_me
DEKLARATION: void notify_feel_me(object who, mixed what)
BESCHREIBUNG:
Nachdem das Objekt oder Mapping (v-item) what vom Lebewesen who betastet
wurde, wird what->notify("feel_me", who) aufgerufen.

Die Funktion notify ruft in allen mit what->add_controller("notify_feel_me",
other) angemeldeten Objekten other die Funktionen other->notify_feel_me(who,
what) auf. Sowohl what als auch other haben dann die Moeglichkeit, auf das
Fuehlen von what zu reagieren.

Bemerkung: Es wird auch who->notify("feel", what) und
           room->notify("feel_here", who, what) aufgerufen.
VERWEISE: forbidden, notify, forbidden_feel
GRUPPEN: spieler, monster, haende
*/

/*
FUNKTION: notify_feel_here
DEKLARATION: void notify_feel_here(object who, mixed what)
BESCHREIBUNG:
Nachdem das Objekt oder Mapping (v-item) what vom Lebewesen who betastet
wurde, wird im Raum room->notify("feel_here", who, what) aufgerufen.

Die Funktion notify ruft in allen mit room->add_controller("notify_feel_here",
other) angemeldeten Objekten other die Funktionen other->notify_feel_here(who,
what) auf. Sowohl room als auch other haben dann die Moeglichkeit, auf das
Fuehlen von what zu reagieren.

Bemerkung: Es wird auch who->notify("feel", what) und
           what->notify("feel_me", who) aufgerufen.
VERWEISE: forbidden, notify, forbidden_feel_here
GRUPPEN: spieler, monster, haende
*/

/* =====================  K  A  M  P  F  ===================== */
 

// so auch in /i/weapon/weapon_logic
int critical_hit()
{
   return random(100) >= 96;  // 96,97,98,99:  4 % Chance
}

varargs int compute_damage(int critical) {
    int stat, skill, d, min, max,result;

    min = this_object()->query_min_damage();
    max = this_object()->query_max_damage();
    if(critical)
    {
        FIGHT_X_DEBUG("hands/compute_damage",sprintf("1:critical:%d",max));
        return max;
    }

    if(d = (max-min))
    {
       stat = (this_object()->query_stat(STAT_STR) +
               this_object()->query_stat(STAT_DEX)) / 2;
       skill = this_object()->get_one_skill(({"skill", "offensiv", "haende"}));
       result = (d*(EXP_TO_PERCENT(skill)+stat)/2 + min*100) / 100;
       FIGHT_X_DEBUG("hands/compute_damage",sprintf("2:min%d max%d:%d",
                    min,max,result));
       return result;
    }
    FIGHT_X_DEBUG("hands/compute_damage",sprintf("3:min%d",min));
    return min;
}

int attackiere_command(string str)
{
    mixed *parsed;
    mixed tmp;
    mixed enemy, weapon, o_ob, w_ob;
    string  weapon_name;
    int hand, o_found, w_found, n_found, a;

    CHECK_KAEMPFEN_VERBOTEN;


    /* GEGNER PARSEN */

    parsed = parse_com(str,environment());
    if (parse_com_error(parsed,"Attackiere wen?\n",1))
        return 0;
    enemy=parsed[PARSE_OBS][0];
    if (!objectp(enemy) || !living(enemy)) 
    {
        if (stringp(tmp = QUERY("attack",enemy))) 
        {
            this_object()->send_message_to(this_object(),MT_LOOK|MT_NOISE,
                        MA_FIGHT,tmp);
            if((tmp = QUERY("attack_msg",enemy)) && tmp != "")
                this_object()->send_message(MT_LOOK|MT_NOISE,MA_FIGHT,
                    wrap(tmp));
            return 1;
        }
        if (mappingp(enemy) && (tmp=QUERY("far",enemy)))
        {
            notify_message(stringp(tmp)?tmp:
                wrap(Der(enemy)+ ist(enemy,1) +" viel zu weit weg."),MA_FIGHT);
            return 1;
        }
        if (mappingp (enemy) && QUERY("living",enemy))
        {
            notify_message("Das geht nicht.\n",MA_FIGHT);
            return 1;
        }
        return notify_fail("Ein totes Objekt?\n", FAIL_WRONG_ARG);
    }
    if (IS_INVIS(enemy))
        return notify_fail("Attackiere wen?\n", FAIL_NOT_OBJ);
    if (enemy == this_object()) 
        return notify_fail("Selbstmord? Nein danke!\n", FAIL_WRONG_ARG);
    if (enemy->query_ghost()) 
        return notify_fail("Einen Geist?\n", FAIL_WRONG_ARG);
    if(query_once_interactive(enemy) && !interactive(enemy))
        return notify_fail("Du hackst erfolglos auf der Statue rum.\n", 
                            FAIL_WRONG_ARG);


    /* EVENTUELL WAFFE PARSEN */

    if (parsed[PARSE_REST] && parsed[PARSE_REST] != "")
    {
        if (!sscanf(lower_case(parsed[PARSE_REST]),"mit %s",weapon_name)) 
            return notify_fail("Attackiere wen womit?\n", FAIL_NOT_OBJ);
        parsed = parse_com(weapon_name,this_object());
        if (parse_com_error(parsed,"Attackiere wen womit?\n",1))
            return 0;
        if (objectp(parsed[PARSE_OBS][0])) 
        {
            weapon = parsed[PARSE_OBS][0];
            weapon_name = CAP(parsed[PARSE_ID]);
        }
    }


    /* WAFFE EXPLIZIT ANGEGEBEN? */

    if (weapon)
    {
        hand = member(hand_objects,weapon);
        if (hand < 0)
        {
            notify("weapon_fail", weapon, "not_wield");
            return notify_fail(wrap(Dein(weapon) + ist(weapon,1) 
                    + " nicht geführt."), FAIL_INTERNAL);
        }
        if (hand_enemies[hand]) 
        {
            notify("weapon_fail", weapon, "already_used");
            return notify_fail(wrap("Du kämpfst bereits mit "
                    +diesem(weapon,"")+"!"), FAIL_INTERNAL);
        }
        if (!hand_objects[hand]->query_weapon_class("nahkampf"))
        {
            notify("weapon_fail", weapon, "wrong_class");
            return notify_fail(wrap(Der(weapon) + ist(weapon,1) 
                    + " nicht für den Nahkampf gedacht!"), FAIL_NOT_CMD);
        }
    }
    else
        hand = -1;


    /* NACH BRAUCHBARER WAFFE IN HAENDEN SUCHEN, FALLS KEINE ANGEGEBEN WAR */

    if (hand < 0)
    {
        n_found = -1;
        for (a = 0; a < num_hands; a++)
            if (hand_enemies[a])
                ;
            else if (hand_objects[a])
            {
                if(!hand_objects[a]->query_weapon_class())
                {
                    o_ob = hand_objects[a];
                    o_found++;
                }
                else if(!hand_objects[a]->query_weapon_class("nahkampf"))
                {
                    w_ob = hand_objects[a];
                    w_found++;
                }
                else
                {
                    hand = a;
                    break;
                }
            }
            else if(n_found < 0)
                n_found = a;
    }


    /* KEINE BRAUCHBARE WAFFE AUFGETREIBEN? */

    if (hand < 0)
        if (n_found >= 0)
            hand = n_found;
        else if (w_found)
        {
            notify("weapon_fail", w_ob, "wrong_class");
            if (w_found > 1)
                return notify_fail(
                    "Deine Waffen sind nichts für den Nahkampf.\n", 
                    FAIL_WRONG_ARG);
            else if (w_found == 1)
                return notify_fail(wrap(Der(w_ob)+ist(w_ob,IST_SPACE_BEFORE)+
                    " nichts für den Nahkampf."), FAIL_WRONG_ARG);
        }
        else if (o_found)
        {
            notify("weapon_fail", o_ob, "no_weapon");
            return notify_fail(
            "Mit DEM Zeugs in der Hand willst Du kämpfen?\n", FAIL_WRONG_ARG);
        }
        else 
            return notify_fail("Du bist bereits genug beschäftigt!\n", 
                FAIL_INTERNAL);
    FIGHT_X_DEBUG("hands/attackiere",sprintf("1:enemy %Q hand_ob %Q hand %d",
        enemy, hand_objects[hand], hand));
    return this_object()->attack(enemy, hand_objects[hand], hand);
}

int werfe_command(string str)
{
    mixed *parsed;
    string enemy_name;
    object|mapping enemy, weapon;
    int hand;


    /* WAFFE PARSEN */

    parsed = parse_com(str,this_object());
    if (parse_com_error(parsed,"Wirf was nach wem?\n",1,1))
        return 0;
    if (!objectp(weapon = parsed[PARSE_OBS][0]))
        return notify_fail("Damit geht das nicht.\n", FAIL_WRONG_ARG,1);
    if (!parsed[PARSE_REST] || parsed[PARSE_REST] == "") 
        return notify_fail(wrap("Wirf "+den(weapon)+" nach wem?"), 
                FAIL_NOT_OBJ,1);
    if (!sscanf(lower_case(parsed[PARSE_REST]),"nach %s",enemy_name))
        sscanf(lower_case(parsed[PARSE_REST]),"auf %s",enemy_name);
    // Das muss moeglichst frueh kommen,
    // damit keine seltsamen Failmeldungen ausgegeben werden.
    // (wirf schneeball nach garthan => Selbstmord? Nein danke...)
    if (!weapon->query_weapon_class("wurf")) 
        return notify_fail(wrap(Der(weapon) + ist(weapon,1) 
                + " nicht zum Werfen gedacht!"), FAIL_NOT_CMD,1);
    

    /* GEGNER PARSEN */

    parsed = parse_com(enemy_name,environment());
    if (parse_com_error(parsed,wrap("Wirf "+den(weapon)+" nach wem?"),
        wrap("Du kannst "+den(weapon)+" nicht auf mehrere Gegner werfen.")))
            return 0;
    enemy = parsed[PARSE_OBS][0];

    if (!objectp(enemy))
        return notify_fail("Macht das Sinn?\n", FAIL_WRONG_ARG);
    if (IS_INVIS(enemy))
        return notify_fail(wrap("Wirf "+den(weapon)+" nach wem?"), FAIL_NOT_OBJ);

    CHECK_KAEMPFEN_VERBOTEN;


    if (!living(enemy)) 
        return notify_fail("Auf ein totes Objekt?\n", FAIL_WRONG_ARG);
    if (enemy == this_object()) 
        return notify_fail("Selbstmord? Nein danke!\n", FAIL_WRONG_ARG);
    if (enemy->query_ghost()) 
        return notify_fail("Auf einen Geist?\n", FAIL_WRONG_ARG);
    if(query_once_interactive(enemy) && !interactive(enemy))
        return notify_fail("Du hackst erfolglos auf der Statue rum.\n", 
            FAIL_WRONG_ARG);


    /* HAND SUCHEN / WAFFE GEFUEHRT? */

    if ((hand = member(hand_objects,weapon)) < 0) 
        return notify_fail(wrap(Der(weapon) + ist(weapon,1) 
                + " nicht geführt."), FAIL_INTERNAL);

    FIGHT_X_DEBUG("hands/werfe_command",
        sprintf("1:enemy %O weapon %Q hand %d",
        enemy, weapon, hand));
    return this_object()->attack(enemy, weapon, hand);
}

int schiesse_command(string str)
{
    mixed *parsed;
    object|mapping enemy, weapon;
    object *ai;
    string weapon_name, enemy_name, rest, pfeil_id;
    int res, hand, found, a, i;

    CHECK_KAEMPFEN_VERBOTEN;

    if (!str) 
        return notify_fail("Schieß auf wen oder womit auf wen?\n", 
                FAIL_NOT_OBJ);


    /* EINGABE ZERLEGEN */

    str = lower_case(str);
    if ((res = sscanf(str,"%s auf %s",rest,enemy_name)) == 2)
    {
        if (!sscanf(rest,"mit %s",weapon_name))
            weapon_name = rest;
    }
    else if (res == 1)
        return notify_fail("Schieß auf wen oder womit auf wen?\n", 
                FAIL_NOT_OBJ);
    else if (sscanf(str,"auf %s",enemy_name) != 1)
        enemy_name = str;


    /* ENEMY PARSEN */

    parsed = parse_com(enemy_name,environment());
    if (parse_com_error(parsed,"Schieß auf wen?\n",1)) {
        parsed = parse_com(enemy_name,this_object());
        if (parse_com_error(parsed,"Schieß auf wen oder womit auf wen?\n"))
            return 0;
        return notify_fail(wrap("Schieß mit "+dem(parsed[PARSE_OBS][0])+" auf wen?"), FAIL_NOT_OBJ);
    }
    enemy = parsed[PARSE_OBS][0];

    if (!objectp(enemy))
        return notify_fail("Macht das Sinn?\n", FAIL_WRONG_ARG);
    if (IS_INVIS(enemy))
        return notify_fail("Schieß auf wen oder womit auf wen?\n", FAIL_NOT_OBJ);
 
    if (!living(enemy)) 
        return notify_fail("Auf ein totes Objekt?\n", FAIL_WRONG_ARG);
    if (enemy == this_object()) 
        return notify_fail("Selbstmord? Nein danke!\n", FAIL_WRONG_ARG);
    if (enemy->query_ghost()) 
        return notify_fail("Auf einen Geist?\n", FAIL_WRONG_ARG);
    if(query_once_interactive(enemy) && !interactive(enemy))
        return notify_fail("Du hackst erfolglos auf der Statue rum.\n", 
            FAIL_WRONG_ARG);


    /* WAFFE PARSEN */

    if (weapon_name)
    {
        parsed = parse_com(weapon_name,this_object());
        if (parsed[PARSE_RET_CODE]!=PARSE_OK)
        {
            mixed parsed2 = 0;
            for (ai = all_inventory(), i=sizeof (ai)-1; i >= 0; i--)
                if (ai[i]->query_container() && !ai[i]->query_con_close())
                {
                    parsed2 = parse_com(weapon_name,ai[i]);
                    if (parsed2[PARSE_RET_CODE] == PARSE_OK)
                    {
                        parsed = parsed2;
                        break;
                    }
                }
        }
        if (parse_com_error(parsed,"Schieß womit auf wen?\n",
                "Du kannst nicht mehrere Dinge auf einmal abschießen.\n"))
            return 0;
        weapon = parsed[PARSE_OBS][0];

        if (!objectp(weapon))
            return notify_fail("Damit geht das nicht.\n", FAIL_WRONG_ARG);

        /* Wenns ein Geschoss ist, eine geeignete Waffe dafuer suchen */
        
        if (weapon->query_geschoss()) 
        {
            for (i=sizeof (hand_objects)-1; i >= 0; i--)
                if (hand_objects[i]
                    && hand_objects[i]->query_weapon_class("schuss")
                    && hand_objects[i]->query_pfeil_id()
                    && weapon->id(hand_objects[i]->query_pfeil_id()))
                {
                    // special pfeil_id merken,da hier vorhanden.
                    pfeil_id = weapon_name;
                    weapon = hand_objects[i];
                    found++;
                    break; // waah, ich will ein GOTO haben!!
                }
            if (!found) // Jetzt das selbe nochmal (stoehn)...
                for (ai = all_inventory(), i=sizeof (ai)-1; i >= 0; i--)
                    if (ai[i]->query_weapon_class("schuss")
                        && ai[i]->query_pfeil_id()
                        && weapon->id(ai[i]->query_pfeil_id()))
                    {
                        // special pfeil_id merken,da hier vorhanden.
                        pfeil_id = weapon_name;
                        weapon = ai[i];
                        found++;
                        break;
                    }
        }
        if (!weapon->query_weapon () && !found)
            return notify_fail(wrap("Du hast nichts dabei, mit dem Du "
                +den(weapon)+" schießen kannst."), FAIL_INTERNAL);

        /* ANGEGEBENE WAFFE IN DER HAND UND SCHUSSWAFFE? */

        hand = member(hand_objects,weapon);
        if (hand < 0) 
            return notify_fail(wrap(Der(weapon) + ist(weapon,1) 
                    + " nicht geführt."), FAIL_INTERNAL);
        if (!weapon->query_weapon_class("schuss")) 
            return notify_fail(wrap(Der(weapon) + ist(weapon,1) +
                    " nicht zum Schießen gedacht!"), FAIL_NOT_CMD);
    }
    else
    {
        hand = -1;
        /* WAFFE SELBST SUCHEN */

        for (a=0; a<num_hands; a++)
            if (hand_objects[a] && 
                hand_objects[a]->query_weapon_class() &&
                !hand_objects[a]->query_weapon_class("defensiv"))
            {
                found++;
                if (hand_objects[a]->query_weapon_class("schuss"))
                {
                    hand = a;
                    break;
                }
            }
        if (hand < 0)
            if (found <= 0)
                return notify_fail("Womit willst Du worauf schießen?\n", 
                        FAIL_NOT_OBJ);
                else if (found == 1)
                    return notify_fail(
                        "Mit deiner Waffe kannst Du nicht schießen!\n", 
                        FAIL_WRONG_ARG);
                else
                    return notify_fail(
                        "Deine Waffen sind nicht zum Schießen geeignet.\n", 
                        FAIL_WRONG_ARG);
        weapon = hand_objects[hand];
    }
    // Damit eis- und feuer-pfeile voneinander unterscheiden werden koennen:
    weapon->set_current_pfeil_id(pfeil_id);
    
    FIGHT_X_DEBUG("hands/schiesse_command",
        sprintf("1:enemy %O weapon %Q hand %d",
        enemy, weapon, hand));
    return this_object()->attack(enemy, weapon, hand);
}


/*
FUNKTION: attack
DEKLARATION: int attack(object enemy, mixed weapon, int hand)
BESCHREIBUNG:
Mit attack wird angegriffen. Angegriffen wird das Opfer enemy, und zwar mit 
der Waffe weapon, die in der Hand mit dem Index hand gesucht wird.
Attack liefert stets 1, auch wenn das frueher in der Doku anders drin
stand. Aus Kompatibilitaetsgruenden muessen wir das wohl auch so beibehalten.
VERWEISE: attackiere_command, werfe_command, schiesse_command
GRUPPEN: spieler, monster, kampf
*/

/*
FUNKTION: no_attack
DEKLARATION: int no_attack(object attacker, object weapon)
BESCHREIBUNG:
Bevor ein Monster/Spieler angegriffen wird (toete Xyz) wird in dem zu
toetendem Lebewesen die Funktion no_attack aufgerufen. Hierbei ist attacker
der Angreifer und weapon die Waffe, mit der angegriffen wird.
Antwortet no_attack mit 0, so wird ganz normal angegriffen, liefert no_attack
jedoch einen Wert ungleich 0 so wird das Living nicht angegriffen, no_attack
muss eine Meldung an attacker ausgeben (warum es nicht ging) und sollte in
den Raum ebenfalls eine geben.
Beispiel: ein Knallfrosch kann nicht mehr angegriffen werden:

    int no_attack(object attacker, object weapon)
    {
        attacker->send_message(MT_LOOK, MA_FIGHT,
            wrap(Der(attacker)+" versucht, "+den()+" anzugreifen, aber "+
                er()+" huepft viel zu verrueckt durch die Gegend, "
                "um getroffen zu werden."),
            wrap(Der(attacker)+" versucht, Dich anzugreifen, aber "
                "Du huepfst ihm total verrueckt davon."),
            this_object());
        attacker->send_message_to(attacker, MT_LOOK|MT_NOTIFY, MA_FIGHT,
            wrap("Du versuchst, "+den()+" anzugreifen, aber "+
                er()+" huepft viel zu verrueckt durch die Gegend, "
                "um getroffen zu werden."));
       return 1;
    }
VERWEISE: attack
GRUPPEN: spieler, monster, haende, kampf
*/

nomask int check_visibility_in_fight(int fightstart)
{
    // teste, ob Spieler sichtbar ist;
    // wenn er unsichtbar ist, mach ihn sichtbar.
    
    // V_ATOM_HIDDEN ist nur verstecken, man bekommt noch immer seinen Namen.
    // Daher erst ab V_ATOM_INVIS ihn sichtbar machen.
    if (IS_INVIS(this_object()))
    {
        this_object()->set_invis(0);

        if(IS_INVIS(this_object()))
        {
            if(!fightstart)
                do_warning2(
                    "check_visibility_in_fight: set_invis war erfolglos!\n",
                    __FILE__, object_name(), __LINE__);
            return 0;
        }
        else
        {
            this_object()->send_message(MT_LOOK, MA_MAGIC,
                wrap(Der(this_object())+
                    " wird mit verkrampftem Gesicht sichtbar."),
                wrap("Die feine Hülle der Unsichtbarkeit zersplittert. "
                    "Du wirst sichtbar und ein feines Stechen durchzuckt "
                    "deinen Körper."),this_object());
            just_got_visible = 1;
        }
    }
    
    return 1;
}

/*
FUNKTION: is_attack_forbidden
DEKLARATION: int is_attack_forbidden(object victim, object weapon)
BESCHREIBUNG:
Um zu testen ob 'enemy' das Lebewesen 'victim' mit der Waffe 'weapon'
angreifen darf, kann man enemy->is_attack_forbidden(victim, weapon) aufrufen.
Liefert dieser Aufruf 1 zurueck, so ist dies verboten. Eine entsprechende
Meldung wird dann ausgegeben. Ansonsten liefert es 0.

is_attack_forbidden prueft dabei die Controller forbidden_attack,
forbidden_my_attack, forbidden_use, ruft no_attack auf und testet auf
den Raumtyp "kaempfen_verboten".

Diese Funktion sollte immer dann aufgerufen werden, wenn eine Aktion
geplant ist, die einen Kampf zur Folge hat (wie z.B. ein add_hp mit
angegebenem Opfer).
VERWEISE: attack, forbidden_attack, forbidden_my_attack, forbidden_use,
        no_attack
GRUPPEN: spieler, monster, haende, kampf
*/
int is_attack_forbidden(object enemy, object weapon)
{
    mixed kv;
    string msg = 0;
    
    if(playerp(enemy))
    {
        if (IS_INVIS(enemy))
        {
            msg = "Du siehst niemanden.";
        }
        else if (enemy->query_ghost())
        {
            msg = "Einen Geist?";
        }
        else if (query_once_interactive(enemy) && !interactive(enemy))
        {
            msg = "Du hackst erfolglos auf der Statue rum.";
        }

        if(msg)
        {
            this_object()->send_message_to(this_object(), MT_NOTIFY,
                MA_UNKNOWN, wrap(msg));
            return 1;
        }
    }
    
    if(playerp(enemy) && playerp(this_object()))
    {
        if(newbiep(enemy) && !enemy->query_moerder() && !testplayerp(enemy))
            msg = "Willst Du als Kindsmörder in die Geschichte eingehen?";
        else if(newbiep(this_object()) && !testplayerp(this_object()))
            msg = "Das traust Du Dir nun wirklich noch nicht zu.";

        if(msg)
        {
            this_object()->send_message_to(this_object(), MT_NOTIFY,
                MA_UNKNOWN, wrap(msg));
            return 1;
        }
/*
       if (!environment()->query_type("arena"))
         if (!((wizp (me) || testplayerp (me))
           && (wizp (enemy) || testplayerp (enemy))))
           EVENT_MASTER->event("Ueberfall", this_object(), 
               "Info: Ueberfall: "+
               capitalize(query_real_name())+" greift "+
               capitalize(enemy->query_real_name())+" an.\n");
*/
    }


    if (enemy->forbidden("attack", this_object(), weapon) ||
        enemy->no_attack(this_object(), weapon) ||
        this_object()->forbidden("my_attack", enemy, weapon) ||
        (weapon && (weapon->forbidden("use", this_object(), enemy) ||
        weapon->no_use_weapon(this_object(), enemy))) ||
        environment()->forbidden("attack_here", this_object(), enemy, weapon))
                return 1;

    if(environment() && (kv=environment()->query_type("kaempfen_verboten")))
    {
        this_object()->send_message_to(this_object(),MT_NOTIFY,
            MA_UNKNOWN, stringp(kv)?kv:"Hier sind keine Kämpfe erlaubt.\n");
        return 1;
    }
    return 0;
}
int check_player_against_player(object me, object enemy) { return 1; } // Dummy

int attack(object enemy, mixed weapon, int hand)
{
    string wer,wen,womit;

    if (is_attack_forbidden(enemy,weapon))
        return 1;

    // Hae? (Freaky)
    /* Hand frei aber keine Waffe, also mit blosser Hand kaempfen */
    //ADD_ATTACK_LIST;

    // Erstmal sichtbarkeit testen
    if(!check_visibility_in_fight(1))
    {
        notify_message("Unsichtbar?\n",MA_FIGHT);
        return 1;
    }

    wer = Der() + " greift ";
    wen = den(enemy);

    if (!weapon)
    {
        string dwomit;
        
        enemy->become_aggression_victim(this_object());
        hand_enemies[hand] = enemy;
        hand_hits[hand] = 0;
        damage = compute_damage();
        set_heart_beat(1);
        weapon = this_object()->query_natural_weapon(hand);
        if(weapon)
        {
            womit = " mit "+seinem(weapon,0,this_object());
            dwomit = " mit "+deinem(weapon,0,this_object());
        }
        else if(this_object()->query_koerperform()=="humanoid")
            dwomit = womit = " mit bloßen Händen";
        else
            dwomit = womit = "";
        
        womit += " an.";
        FIGHT_X_DEBUG("hands/attack",
            sprintf("1 hand/natural_weapon: enemy %O",enemy));
#ifdef FILTER_MSG_BY_ATTRIBUTES
        notify_message(wrap(
                "Du greifst "+wen+dwomit+" an."),MA_FIGHT,
                ([ AH_ATTACKER: this_object(), AH_VICTIM: enemy,
                   MSG_RECEIVER_WHOM: AH_ATTACKER,MSG_FIRST_MSG:1 ]));
        this_object()->send_message(MT_LOOK,MA_FIGHT,wrap(wer+wen+womit),
                    wrap(wer+"Dich"+womit),enemy,
                    ([ AH_ATTACKER: this_object(), AH_VICTIM: enemy,
                        MSG_FIRST_MSG:1,MSG_RECEIVER_WHOM: AH_VICTIM]));
#else
        notify_message(wrap(
                "Du greifst "+wen+dwomit+" an."),MA_FIGHT);
        this_object()->send_message(MT_LOOK,MA_FIGHT,wrap(wer+wen+womit),
                    wrap(wer+"Dich"+womit),enemy);
#endif
        enemy->notify("attack", this_object(), 0);
        this_object()->notify("my_attack", enemy, 0);
        environment()->notify("attack_here", this_object(), enemy, 0);
        return 1;
    }

    /* WAFFE KAPUTT */

    if (weapon->query_weapon() && weapon->query_broken())
    {
#ifdef FILTER_MSG_BY_ATTRIBUTES
        notify_message("Mit einer kaputten Waffe?\n",MA_FIGHT,
                ([ AH_ATTACKER: this_object(), AH_VICTIM: enemy,
                   MSG_RECEIVER_WHOM: AH_ATTACKER,MSG_FIRST_MSG:1,
                   FIM_WEAPON:weapon, FIM_BROKEN: 1]));
#else
        notify_message("Mit einer kaputten Waffe?\n",MA_FIGHT);
#endif
        notify("weapon_fail", weapon, "broken");
        return 1;
    }

    switch(weapon->query_weapon_class())
    {
       case "nahkampf":
        enemy->become_aggression_victim(this_object());
        hand_enemies[hand] = enemy;
        hand_hits[hand] = 0;
        damage = 0;
        set_heart_beat(1);
        womit = " mit "+seinem(weapon,"")+" an.";
        FIGHT_X_DEBUG("hands/attack",
            sprintf("2 nahkampf: enemy %O weapon %O",enemy,weapon));
#ifdef FILTER_MSG_BY_ATTRIBUTES
        notify_message(wrap(
                "Du greifst "+wen+" mit "+deinem(weapon,"")+
                " an."),MA_FIGHT,
                ([ AH_ATTACKER: this_object(), AH_VICTIM: enemy, 
                    FIM_WEAPON: weapon,MSG_RECEIVER_WHOM: AH_ATTACKER,
                    MSG_FIRST_MSG:1 ]));
        this_object()->send_message(MT_LOOK,MA_FIGHT,wrap(wer+wen+womit),
                    wrap(wer+"Dich"+womit),enemy,
                    ([ AH_ATTACKER: this_object(), AH_VICTIM: enemy,
                        FIM_WEAPON: weapon,MSG_FIRST_MSG:1,
                        MSG_RECEIVER_WHOM:AH_VICTIM]) );
#else
        notify_message(wrap("Du greifst "+wen+" mit "+deinem(weapon,"")+
                " an."),MA_FIGHT);
        this_object()->send_message(MT_LOOK,MA_FIGHT,wrap(wer+wen+womit),wrap(wer+"Dich"+womit),enemy);
#endif
        enemy->notify("attack", this_object(), weapon);
        this_object()->notify("my_attack", enemy, weapon);
        weapon->notify("use", this_object(), enemy);          
        environment()->notify("attack_here", this_object(), enemy, weapon);
        return 1;
       case "wurf":
        enemy->become_aggression_victim(this_object());
          call_out("do_wurf",0, ({weapon, enemy}));
        FIGHT_X_DEBUG("hands/attack",
            sprintf("3 wurf: enemy %O weapon %O",enemy,weapon));
        notify_message("Ok.\n",MA_FIGHT);
          return 1;
       case "schuss":
        enemy->become_aggression_victim(this_object());
        call_out("do_schuss",0, ({weapon, enemy}));
        FIGHT_X_DEBUG("hands/attack",
            sprintf("4 schuss: enemy %O weapon %O",enemy,weapon));
        notify_message("Ok.\n",MA_FIGHT);
        return 1;
       case "defensiv":
          FIGHT_X_DEBUG("hands/attack",
            sprintf("5 defensiv: enemy %O weapon %O",enemy,weapon));
          notify_message(wrap(Der(weapon) + ist(weapon,1) +
            " nicht zum Angreifen gedacht!"),MA_FIGHT);
          notify("weapon_fail", weapon, "wrong_class");
          return 1;
       default:
          FIGHT_X_DEBUG("hands/attack",
            sprintf("5 noweapon: enemy %O weapon %O",enemy,weapon));
          notify_message(wrap(Der(weapon)+ist(weapon,1)+" keine Waffe!"),
                    MA_FIGHT);
          notify("weapon_fail", weapon, "no_weapon");
          return 1;
    }
}

void do_wurf(object *obs)
{
    if (obs[0] && obs[1] && environment()==environment(obs[1]))
    {
        obs[1]->notify("attack", this_object(), obs[0]);
        this_object()->notify("my_attack", obs[1], obs[0]);
        obs[0]->notify("use", this_object(), obs[1]);          
        environment()->notify("attack_here", this_object(), obs[1], obs[0]);
        obs[0]->do_wurf(obs[1]);
    }
}

void do_schuss(object *obs)
{
    if (obs[0] && obs[1] && environment()==environment(obs[1]))
    {
        obs[1]->notify("attack", this_object(), obs[0]);
        this_object()->notify("my_attack", obs[1], obs[0]);
        obs[0]->notify("use", this_object(), obs[1]);
        environment()->notify("attack_here", this_object(), obs[1], obs[0]);
        obs[0]->do_schuss(obs[1]);
    }
}

private static int last_shown_hp, last_shown_sp;

void hp_sp_view ()
{
    int my_sp, my_hp;
    my_sp = this_object()->query_sp();
    my_hp = this_object()->query_hp();
    if ((my_hp == last_shown_hp) && (my_sp == last_shown_sp)) return;
    last_shown_hp = my_hp; last_shown_sp = my_sp;
    if (hp_view & HP_SP_VIEW_MAX)
        notify_message(wrap(my_hp+" AP("+this_object()->query_max_hp()
            +") und "+my_sp+" "+query_sp_short_name()
            +"("+this_object()->query_max_sp()+")."));
    else
        notify_message(wrap(my_hp+" AP und "+my_sp+" "+query_sp_short_name()
            +"."));
}

nomask static int hp_sp_view_command ()
{
    if ((query_verb()=="mp")
        && (this_object()->query_gilde() != "Alchemistengilde"))
        return 0;
    else if ((query_verb()=="kp")
        && (this_object()->query_gilde() != "Diebesgilde"))
        return 0;
    last_shown_hp = -12345;
    hp_sp_view ();
    return 1;
}

/*
FUNKTION: query_extra_damage
DEKLARATION: int query_extra_damage(object feind, object monster)
BESCHREIBUNG:
Wird fuer jeden Schlag des Monsters im Monster selbst aufgerufen,
damit man den Schaden auch in Abhaengigkeit vom Opfer festlegen kann.
Oder Monstergifte verteilen. Oder ...
Was immer diese Funktion zurueck liefert, wird auf den Schaden aufaddiert.
Positive Werte bedeuten groesseren Schaden, negative Verschlechterung.
VERWEISE: set_min_damage, set_max_damage
GRUPPEN: monster, kampf, haende
*/
// Zum Berechnen des eigentlichen Schadens.
int compute_applied_damage(int my_damage, int hand, int critical)
{
    if ((critical = critical_hit()) && !newbiep(hand_enemies[hand]))
        return compute_damage(critical);
    else
        return random(my_damage + 1);
}

static int dont_hit_time;

/*
FUNKTION: modify_hit
DEKLARATION: void modify_hit(mapping info, object attacker)
BESCHREIBUNG:
Bevor das Lebewesen 'attacker' einen Schlag in einem regulaeren
Kampf ausfuehrt, wird an allen bei 'attacker' angemeldeten Controllern
diese Funktion aufgerufen.

Das uebergebene Mapping info enthaelt neben allen Eintraegen,
die an add_hp uebergeben werden, zusaetzlich den Eintrag AH_DAMAGE
mit dem anvisierten Schaden. Alle Eintraege koennen veraendert werden,
wird AH_VICTIM auf 0 gesetzt, so wird der Schlag nicht durchgefuehrt.
VERWEISE: modify_hit_weapon, modify_hit_me, add_hp, modify_damage
GRUPPEN: monster, spieler, kampf
*/
/*
FUNKTION: modify_hit_weapon
DEKLARATION: void modify_hit_weapon(mapping info, object attacker)
BESCHREIBUNG:
Bevor das Lebewesen 'attacker' einen Schlag in einem regulaeren
Kampf ausfuehrt, wird an allen an der genutzten Waffe angemeldeten
Controllern diese Funktion aufgerufen.

Das uebergebene Mapping info enthaelt neben allen Eintraegen,
die an add_hp uebergeben werden, zusaetzlich den Eintrag AH_DAMAGE
mit dem anvisierten Schaden. Alle Eintraege, auch AH_WEAPON, koennen
veraendert werden, wird AH_VICTIM auf 0 gesetzt, so wird der Schlag
nicht durchgefuehrt.
VERWEISE: modify_hit_weapon, modify_hit_me, add_hp, modify_damage
GRUPPEN: monster, spieler, kampf
*/
/*
FUNKTION: modify_hit_me
DEKLARATION: void modify_hit_me(mapping info, object attacker)
BESCHREIBUNG:
Bevor das Lebewesen 'attacker' einen Schlag in einem regulaeren
Kampf ausfuehrt, wird an allen am Opfer angemeldeten Controllern
diese Funktion aufgerufen.

Das uebergebene Mapping info enthaelt neben allen Eintraegen,
die an add_hp uebergeben werden, zusaetzlich den Eintrag AH_DAMAGE
mit dem anvisierten Schaden. Alle Eintraege ausser AH_WEAPON
koennen veraendert werden, also auch das Opfer AH_VICTIM.
Wird AH_VICTIM auf 0 gesetzt, so wird der Schlag nicht durchgefuehrt.
VERWEISE: modify_hit_weapon, modify_hit_me, add_hp, modify_damage
GRUPPEN: monster, spieler, kampf
*/

int do_hit(mapping data)
{
    mapping victim_list = ([]);
    mapping weapon_list = ([0]);
    object victim;
    int applied_damage, back;
    mixed* args;

    // So, erstmal das Mapping ueberpruefen und vervollstaendigen
    if(!data || !living(data[AH_VICTIM]))
        return ATTACK_DA_HIT_NULL;

    if(!member(data,AH_HANDNR))
    {
        // wenn data[AH_WEAPON] nicht gesetzt ist, dann holen
        // wir halt die erste freie Hand.
        int nr = member(query_hand_objects(), data[AH_WEAPON]);
        if(nr>=0)
            data[AH_HANDNR] = nr;
    }
    
    if(member(data,AH_HANDNR) && !data[AH_WEAPON])
    {
        mapping hand = this_object()->query_natural_weapon(data[AH_HANDNR]);

        if(hand)
            data[AH_WEAPON] = hand;
    }
    
    if(!data[AH_DAMAGE_TYPE])
    {
        string* dtype;
        if(data[AH_WEAPON])
            dtype = QUERY(AH_DAMAGE_TYPE, data[AH_WEAPON]); // query_damage_type
        if(!dtype)
        {
            if(objectp(data[AH_WEAPON]))
            {
                string *spath = data[AH_WEAPON]->query_skill_path();
                if(sizeof(spath)>2 && spath[2]!="scharf")
                    dtype = ({"stumpf"});
                else
                    dtype = ({ ({ "stich", "schnitt"})[random(2)] });
            }
            else
                dtype = ({"stumpf"});
        }
        data[AH_DAMAGE_TYPE] = dtype;
    }
    
    FIGHT_X_DEBUG("hands/do_hit",
            sprintf("1 before modify_hit %Q",data));
    this_object()->modify("hit", &data, this_object());
    if(!this_object() || !data[AH_VICTIM])
        return ATTACK_DA_HIT_ERROR;

    // Ketten aufloesen...
    while(objectp(data[AH_WEAPON]) && !member(weapon_list, data[AH_WEAPON]))
    {
        m_add(weapon_list, data[AH_WEAPON]);
        data[AH_WEAPON]->modify("hit_weapon", &data, this_object());
        if(!this_object() || !data[AH_VICTIM])
            return ATTACK_DA_HIT_ERROR;
    }

    while(!member(victim_list, data[AH_VICTIM]))
    {
        m_add(victim_list, data[AH_VICTIM]);
        data[AH_VICTIM]->modify("hit_me", &data, this_object());

        if(!this_object() || !data[AH_VICTIM])
            return ATTACK_DA_HIT_ERROR;
    }
    FIGHT_X_DEBUG("hands/do_hit",
            sprintf("2 after modify_hit %Q",data));
    
    applied_damage = data[AH_DAMAGE];
    victim = data[AH_VICTIM];
    m_delete(data, AH_DAMAGE);
    m_delete(data, AH_VICTIM);

    if(!just_got_visible)
        applied_damage +=
            (objectp(data[AH_WEAPON])?data[AH_WEAPON]:this_object())->
            query_extra_damage(victim, this_object());

    if (!this_object() || !victim)
        return ATTACK_DA_HIT_ERROR;

    if(applied_damage < 0)
        applied_damage = 0;
    
    args = ({ this_object(), victim, applied_damage, 
            (data[AH_FLAGS] & AH_CRITICAL)?1:0, data[AH_HANDNR],
            objectp(data[AH_WEAPON]) && data[AH_WEAPON]
        });
 
    apply(#'call_other, this_object(), "notify", "before_hit", args);
    if (!this_object() || !victim)
        return ATTACK_DA_HIT_ERROR;

    if(objectp(data[AH_WEAPON]))
    {
        apply(#'call_other, data[AH_WEAPON], "notify", "before_hit_weapon",
            args);
        if (!this_object() || !victim)
            return ATTACK_DA_HIT_ERROR;
    }

    apply(#'call_other, victim, "notify", "before_hit_me", args);
    if (!this_object() || !victim)
        return ATTACK_DA_HIT_ERROR;

    data[AH_ATTACKER] = this_object();
    if(data[AH_FLAGS] & AH_CRITICAL)
        data[AH_FLAGS] |= AH_NO_ARMOUR;
    
    FIGHT_X_DEBUG("hands/do_hit",
            sprintf("1 before add_hp damage=%d %Q",-applied_damage,data));
    back = victim->add_hp(-applied_damage, data);
    // back = victim->add_hp(-applied_damage, this_object(), data[AH_FLAGS]);

    if(back == -1)
        victim = 0;

    if(this_object())
        apply(#'call_other, this_object(), "notify", "after_hit", args);
    if(objectp(data[AH_WEAPON]) && this_object())
        apply(#'call_other, data[AH_WEAPON], "notify", "after_hit_weapon",
            args);
    if(victim && this_object())
        apply(#'call_other, victim, "notify", "after_hit_me", args);

    return back>=0 ? ATTACK_DA_HIT_OK : ATTACK_DA_HIT_NULL;    
}

void handle_attack()
{
    int a, applied_damage, critical, attack_da;
    object * obs;
  
    if(dont_hit_time==time())
    {
        dont_hit_time=0;
        return;
    }

#if FIGHT_SPEED > 1
    if (fight_count)
    {
        fight_count--;
        return;
    }
    fight_count = FIGHT_SPEED - 1;
#endif
    if(lost_hp)
        lost_hp = 0;

    if (damage <= 0)
        damage = compute_damage();

    /* Naechste Hand mit Feind suchen */
    do
    {
        current_hand++;
        current_hand %= num_hands; 
    }
    while(!hand_enemies[current_hand] && a++ < num_hands);

    /* Kein Feinde */
    if (!hand_enemies[current_hand])
        return;

    if (fight_options[FIO_BROKEN_WEAPON])
    {
        obs = filter(hand_objects-({0}), (: $1->query_broken() :));
        if (sizeof(obs))
        { // nur eine Waffe por HeartBeat senken/legen...
            if (fight_options[FIO_BROKEN_WEAPON] == 2)
            {
                this_object()->exec_command("lege ", obs[0]);
            }
            else
            {
                this_object()->exec_command("senke ", obs[0]);
            }
        }
    }        
    if (fight_options[FIO_PREVENT_ONLY_DEFENSIVE])
    {
        if (sizeof(filter(hand_objects,
                (: objectp($1) && $1->query_weapon_class()=="defensiv" :) ))
                == num_hands)
        {
            // nur Schilde gefuehrt
            obs = copy(hand_objects);
            obs = sort_array(obs, function int (object a,object b)
            {
                string *ska,*skb;
                ska = a->query_skill_path(); // letzte stelle gross/klein
                skb = b->query_skill_path(); // bei schilden...
                if (ska[<1] < skb[<1]) return 1;
                if (a->query_value() > b->query_value()) return 1;
                return (a->query_life() > b->query_life());
            });
            this_object()->exec_command("senke ",obs[<1]);
        }
    }
    
    /* Feind noch da? */
    if (!present(hand_enemies[current_hand], environment()))
    {
        // gone_ob ist ein Array
        if (gone_ob)
            gone_ob->enemy_gone(hand_enemies[current_hand]);
        delete_enemy(hand_enemies[current_hand]);
        return;
    }
    
    // Sichtbar werden...
    if(!check_visibility_in_fight(0))
        return;
    
    /* Waffe noch da? */
    if (hand_objects[current_hand] &&
            !present(hand_objects[current_hand], this_object()))
        hand_objects[current_hand] = 0;

    /* nach Waffe suchen */
    if (!hand_objects[current_hand] && fight_options[FIO_WIELD_WEAPON])
    {
        obs = filter (all_inventory(this_object()), function int (object o)
        {
            return (o->query_weapon() && !o->query_broken()
            && o->query_weapon_class() != "defensiv");
        });
        if (sizeof(obs))
        {
            obs = sort_array(obs, function int (object a, object b)
            {
                if (a->query_max_damage() < b->query_max_damage())
                    return 1;
                if (a->query_min_damage() < b->query_min_damage())
                    return 1;
                return 0;
            });
            if (obs[0]->query_max_damage() >= 
                this_object()->query_max_damage())//Waffe besser als Hand?
            {
                FIGHT_X_DEBUG("hands/handle_attack",
                    sprintf("1 FIO_WIELD_WEAPON hand %d w%Q",
                        current_hand,obs[0]));
                this_object()->wield(obs[0], current_hand);
            }
        }
    }
    if (!hand_objects[current_hand] && fight_options[FIO_TAKE_WEAPON])
    {
        obs = filter (all_inventory(environment(this_object())), 
            function int (object o)
        {
            return (o->query_weapon() && !o->query_broken()
            && o->query_weapon_class() != "defensiv");
        });
        if (sizeof(obs))
        {
            obs = sort_array(obs, function int (object a, object b)
            {
                if (a->query_max_damage() < b->query_max_damage())
                    return 1;
                if (a->query_min_damage() < b->query_min_damage())
                    return 1;
                return 0;
            });
            if (obs[0]->query_max_damage() >= 
                    this_object()->query_max_damage()) //Waffe besser als Hand?
            {
                FIGHT_X_DEBUG("hands/handle_attack",
                    sprintf("2 FIO_TAKE_WEAPON hand %d w%Q",
                        current_hand, obs[0]));
                this_object()->exec_command("nimm ",obs[0]);
                if (obs[0] && environment(obs[0]) == this_object())
                    this_object()->wield(obs[0], current_hand);
            }
        }
    }
    hand_hits[current_hand]++;
    /* Angriff mit der Waffe */
    if (hand_objects[current_hand])
    {
        if (fight_options[FIO_USE_FAR_WEAPON])
        {
            attack_da = call_other(
                hand_objects[current_hand],
                ([
                    "nahkampf"  : "do_attack",
                    "schuss"    : "do_schuss",
                    "wurf"      : "do_wurf",
                    ])[hand_objects[current_hand]->query_weapon_class()],
                hand_enemies[current_hand],
                just_got_visible
                );
            if (this_object())
            {
                FIGHT_X_DEBUG("hands/handle_attack",
                    sprintf("3 enemy %Q da%d",hand_enemies[current_hand],
                    attack_da));
                this_object()->do_attack_chat(hand_enemies[current_hand]);
            }
            if (!this_object()) // Toedliche chats und do_attacks und co...
            {
                return;
            }
            switch (attack_da) 
            {
            case  ATTACK_DA_HIT_ERROR:
            case  ATTACK_DA_HIT_NULL:
            case  ATTACK_DA_HIT_OK:
            case  ATTACK_DA_ABBRUCH:   
            case  ATTACK_DA_OK:       
            case  ATTACK_DA_KEIN_PFEIL:
            case  ATTACK_DA_KAPUTT:  
            case  ATTACK_DA_WAR_BEREITS_KAPUTT:
            default:
                break;
            }
            just_got_visible = 0;
            return;
        }
        if(hand_objects[current_hand]->do_attack(hand_enemies[current_hand],
                    just_got_visible)==0)
        {
            hand_enemies[current_hand] = 0;
            hand_hits[current_hand] = 0;
        }
        else if (this_object()) 
        {    // this_object() kann tot sein, wenn das
                    // do_attack() es getoetet hat :(((
            FIGHT_X_DEBUG("hands/handle_attack",
                    sprintf("4 enemy %Q",hand_enemies[current_hand]));
            this_object()->do_attack_chat(hand_enemies[current_hand]);
        }
        just_got_visible = 0;
        return;
    }

    /* Angriff mit der Hand */
    if(this_object())
       this_object()->do_attack_chat(hand_enemies[current_hand]);

    /* Es gibt wohl toetliche Chats... */
    if (!hand_enemies[current_hand])
    {
        hand_hits[current_hand] = 0;
        return;
    }

    // erster Treffer nach dem unsichtbar werden ist erfolglos.
    if (just_got_visible)
        applied_damage = 0;
    else
        applied_damage = compute_applied_damage(damage,current_hand,&critical);

#ifdef FIGHT_DEBUG
    if(debugger)
       tell_object(debugger,
            "HAND: COMPUTED_DAM: "+ damage+
            " APPLIED_DAM: "      + applied_damage+
            " ("+this_object()->query_name()+")\n");
#endif
    FIGHT_X_DEBUG("hands/handle_attack",
            sprintf("5 damage %d applied %d",damage,applied_damage));


    if (this_object() && !random(10))
    {
        this_object()->add_skill_points(({"skill","offensiv","haende"}),10);
        damage = compute_damage();
    }

    do_hit(
        ([
            AH_VICTIM: hand_enemies[current_hand],
            AH_DAMAGE: applied_damage,
            AH_FLAGS: critical?AH_CRITICAL:0,
            AH_HANDNR: current_hand,
        ]));

    just_got_visible = 0;
}

/*
FUNKTION: notify_before_hit
DEKLARATION: void notify_before_hit(object attacker, object victim, int damage, int critical, int hand, object weapon)
BESCHREIBUNG:
Bevor ein Schlag ausgefuehrt wird, wird beim Angreifer attacker->notify(
"before_hit", attacker, victim, damage, critical, hand, weapon) aufgerufen.
notify ruft dann in allen mit attacker->add_controller("notify_before_hit",
other) angemeldeten Objekten other->notify_before_hit(attacker, victim,
damage, critical, hand, weapon) auf. Diese Funktion kann dann entsprechend
auf den bevorstehenden Schlag reagieren. Die Parameter im einzelnen:

  attacker: Derjenige, welcher den Schlag ausfuehrt.
  victim:   Derjenige, welcher den Schlag abbekommt.
  damage:   Der Schaden in Ausdauerpunkten
  critical: Wenn 1, dann ist dieser Schlag ein kritischer.
  hand:     Die Hand, welcher den Schlag ausfuehrte. Geht von 0 bis
            num_hands-1. (siehe query_num_hands)
  weapon:   Die Waffe, mit der zugeschlagen wird. (bei 0 sind es die Haende)

VERWEISE: notify, add_controller, notify_before_hit_me,
          notify_before_hit_weapon, notify_after_hit
GRUPPEN: spieler, monster, kampf
*/

/*
FUNKTION: notify_before_hit_weapon
DEKLARATION: void notify_before_hit_weapon(object attacker, object victim, int damage, int critical, int hand, object weapon)
BESCHREIBUNG:
Bevor ein Schlag ausgefuehrt wird, wird in der Waffe weapon->notify(
"before_hit_weapon", attacker, victim, damage, critical, hand, weapon)
aufgerufen. notify ruft dann in allen mit weapon->add_controller(
"notify_before_hit_weapon", other) angemeldeten Objekten
other->notify_before_hit_weapon(attacker, victim, damage, critical, hand,
weapon) auf. Diese Funktion kann dann entsprechend auf den bevorstehenden
Schlag reagieren. Die Parameter im einzelnen:

  attacker: Derjenige, welcher den Schlag ausfuehrt.
  victim:   Derjenige, welcher den Schlag abbekommt.
  damage:   Der Schaden in Ausdauerpunkten
  critical: Wenn 1, dann ist dieser Schlag ein kritischer.
  hand:     Die Hand, welcher den Schlag ausfuehrte. Geht von 0 bis
            num_hands-1. (siehe query_num_hands)
  weapon:   Die Waffe, mit der zugeschlagen wird. (bei 0 sind es die Haende)

VERWEISE: notify, add_controller, notify_before_hit,
          notify_before_hit_me, notify_after_hit_weapon
GRUPPEN: spieler, monster, kampf
*/

/*
FUNKTION: notify_before_hit_me
DEKLARATION: void notify_before_hit_me(object attacker, object victim, int damage, int critical, int hand, object weapon)
BESCHREIBUNG:
Bevor ein Schlag ausgefuehrt wird, wird beim Opfer victim->notify(
"before_hit_me", attacker, victim, damage, critical, hand, weapon) aufgerufen.
notify ruft dann in allen mit victim->add_controller("notify_before_hit_me",
other) angemeldeten Objekten other->notify_before_hit_me(attacker, victim,
damage, critical, hand, weapon) auf. Diese Funktion kann dann entsprechend
auf den bevorstehenden Schlag reagieren. Die Parameter im einzelnen:

  attacker: Derjenige, welcher den Schlag ausfuehrt.
  victim:   Derjenige, welcher den Schlag abbekommt.
  damage:   Der Schaden in Ausdauerpunkten
  critical: Wenn 1, dann ist dieser Schlag ein kritischer.
  hand:     Die Hand, welcher den Schlag ausfuehrte. Geht von 0 bis
            num_hands-1. (siehe query_num_hands)
  weapon:   Die Waffe, mit der zugeschlagen wird. (bei 0 sind es die Haende)

VERWEISE: notify, add_controller, notify_before_hit,
          notify_before_hit_weapon, notify_after_hit_me
GRUPPEN: spieler, monster, kampf
*/

/*
FUNKTION: notify_after_hit
DEKLARATION: void notify_after_hit(object attacker, object victim, int damage, int critical, int hand, object weapon)
BESCHREIBUNG:
Nachdem ein Schlag ausgefuehrt wurde, wird beim Angreifer attacker->notify(
"after_hit", attacker, victim, damage, critical, hand, weapon) aufgerufen.
notify ruft dann in allen mit attacker->add_controller("notify_after_hit",
other) angemeldeten Objekten other->notify_after_hit(attacker, victim,
damage, critical, hand, weapon) auf. Diese Funktion kann dann entsprechend
auf den Schlag reagieren. Die Parameter im einzelnen:

  attacker: Derjenige, welcher den Schlag ausfuehrt.
  victim:   Derjenige, welcher den Schlag abbekommt.
            (Kann inzwischen 0 sein, wenn er gestorben ist.)
  damage:   Der Schaden in Ausdauerpunkten
  critical: Wenn 1, dann ist dieser Schlag ein kritischer.
  hand:     Die Hand, welcher den Schlag ausfuehrte. Geht von 0 bis
            num_hands-1. (siehe query_num_hands)
  weapon:   Die Waffe, mit der zugeschlagen wird. (bei 0 sind es die Haende)

VERWEISE: notify, add_controller, notify_after_hit_me,
          notify_after_hit_weapon, notify_before_hit
GRUPPEN: spieler, monster, kampf
*/

/*
FUNKTION: notify_after_hit_weapon
DEKLARATION: void notify_after_hit_weapon(object attacker, object victim, int damage, int critical, int hand, object weapon)
BESCHREIBUNG:
Nachdem ein Schlag ausgefuehrt wurde, wird in der Waffe weapon->notify(
"after_hit_weapon", attacker, victim, damage, critical, hand, weapon)
aufgerufen. notify ruft dann in allen mit weapon->add_controller(
"notify_after_hit_weapon", other) angemeldeten Objekten
other->notify_after_hit_weapon(attacker, victim, damage, critical, hand,
weapon) auf. Diese Funktion kann dann entsprechend auf den Schlag reagieren.
Die Parameter im einzelnen:

  attacker: Derjenige, welcher den Schlag ausfuehrt.
  victim:   Derjenige, welcher den Schlag abbekommt.
            (Kann inzwischen 0 sein, wenn er gestorben ist.)
  damage:   Der Schaden in Ausdauerpunkten
  critical: Wenn 1, dann ist dieser Schlag ein kritischer.
  hand:     Die Hand, welcher den Schlag ausfuehrte. Geht von 0 bis
            num_hands-1. (siehe query_num_hands)
  weapon:   Die Waffe, mit der zugeschlagen wird. (bei 0 sind es die Haende)

VERWEISE: notify, add_controller, notify_after_hit,
          notify_after_hit_me, notify_before_hit_weapon
GRUPPEN: spieler, monster, kampf
*/

/*
FUNKTION: notify_after_hit_me
DEKLARATION: void notify_after_hit_me(object attacker, object victim, int damage, int critical, int hand, object weapon)
BESCHREIBUNG:
Nachdem ein Schlag ausgefuehrt wurde, wird beim Opfer victim->notify(
"after_hit_me", attacker, victim, damage, critical, hand, weapon) aufgerufen.
notify ruft dann in allen mit victim->add_controller("notify_after_hit_me",
other) angemeldeten Objekten other->notify_after_hit_me(attacker, victim,
damage, critical, hand, weapon) auf. Diese Funktion kann dann entsprechend
auf den Schlag reagieren. Die Parameter im einzelnen:

  attacker: Derjenige, welcher den Schlag ausfuehrt.
  victim:   Derjenige, welcher den Schlag abbekommt.
            (Kann inzwischen 0 sein, wenn er gestorben ist.)
  damage:   Der Schaden in Ausdauerpunkten
  critical: Wenn 1, dann ist dieser Schlag ein kritischer.
  hand:     Die Hand, welcher den Schlag ausfuehrte. Geht von 0 bis
            num_hands-1. (siehe query_num_hands)
  weapon:   Die Waffe, mit der zugeschlagen wird. (bei 0 sind es die Haende)

VERWEISE: notify, add_controller, notify_after_hit,
          notify_after_hit_weapon, notify_before_hit_me
GRUPPEN: spieler, monster, kampf
*/

/* =================== S T O P  K A M P F =============== */

/*
FUNKTION: stop_all_fights
DEKLARATION: void stop_all_fights()
BESCHREIBUNG:
Tut das, wonach es klingt: Das Monster oder der Spieler stellt alle Kaempfe
ein. Und zwar ohne jede Meldung; bei Monstern wird auch die attack_list
geloescht.
VERWEISE: stop_command
GRUPPEN: spieler, monster, kampf
*/
void stop_all_fights()
{
    int a;
    
    FIGHT_X_DEBUG("hands/stop_all_fights","1");    

    for (a=0; a<num_hands; a++)
    {
        hand_enemies[a] = 0;
        hand_hits[a] = 0;
    }
    this_object()->set_attack_list(0);
}

int stop_command(string str)
{
    if (!str)
        return notify_fail("Stoppe was?\n", FAIL_NOT_OBJ);
    str=convert_umlaute(lower_case(str));
    if (strstr(str,"kampf")<0 && strstr(str,"kaempf")<0)
        return notify_fail("Stoppe was?\n", FAIL_NOT_OBJ);
    if (!query_in_fight())
        return notify_fail("Du kämpfst doch gar nicht.\n", FAIL_INTERNAL);
    stop_all_fights();
#ifdef FILTER_MSG_BY_ATTRIBUTES
    notify_message("Du hörst auf zu kämpfen.\n",MA_FIGHT, ([
        MSG_LAST_MSG:1]) );
#else
    notify_message("Du hörst auf zu kämpfen.\n",MA_FIGHT);
#endif
    return 1;
}


/* ===================  F U E H R E N   &  S E N K E N ======= */

int wield_command(string str)
{
    mixed *parsed,*obs,ob;
    string *words;
    string *alles;
    int hand,i,j,all,size;

#ifdef NEW_STATS
    if (this_object()->query_stat(STAT_INT) < MIN_INT_WIELD)
    {
        notify_fail("Dafür bist du nicht intelligent genug.\n");
        return 0;
    }
#endif
    if (!str)
        return notify_fail("Was willst du führen?\n", FAIL_NOT_OBJ);
    if ((hand = free_hand()) < 0)
        return notify_fail("Du hast keine Hand frei.\n", FAIL_INTERNAL);
    // Nun wird geprueft, ob der Spieler mehrere Waffen fuehren will
    alles = ALLES;
    size = sizeof(alles);
    words = explode(str," ");
    do all = member(words,alles[i]) >= 0; while (!all && ++i<size);
    parsed = parse_com(str,this_object(),0,PARSE_ALL_MATCHES);
    if (parse_com_error(parsed,"Führe was?\n"))
        return 0;
    size = sizeof(obs = parsed[PARSE_OBS]);
    i = 0;
    if (all)
    { // Alle gefundenen Waffen fuehren
        string *dnames = ({});
        string *snames = ({});

        notify_fail("Davon kannst du nichts führen.\n");
        do
        {
            CHECK_EVAL_COST
            if (objectp(ob = obs[i]) && ob->query_weapon_class() &&
                member(hand_objects,ob) == -1)
            {
                string sname, dname;
                if(!ob->query_wield_msg())
                {
                    dname = deinen(ob);
                        sname = seinen(ob);
                }
                if(this_object()->wield(ob, hand, dname?1:0))
                {
                    if(dname)
                    {
                        dnames += ({dname});
                        snames += ({sname});
                    }
                }
            // Auch zaehlen, wenn nicht erfolgreich,
            // denn die notify_fail-Meldung muss unterbleiben,
            // da wield() schon eine Meldung ausgibt.
            j++;
            }
        } while ((hand = free_hand()) >= 0 && ++i<size);
        if(sizeof(dnames))
        {
            notify_message(wrap("Du führst "+liste(dnames)+"."),MA_WIELD);
            this_object()->send_message(MT_LOOK,MA_WIELD,
                wrap(Der()+" führt "+liste(snames)+"."));
        }
        return j > 0;
    }
    else
    { // Die erste moegliche Waffe fuehren
        while ((!objectp(ob = obs[i]) ||
            member(hand_objects,ob) >= 0 ||
            !ob->query_weapon_class()) && ++i<size);
        if (i < size)
        {
            this_object()->wield(ob, hand);
            return 1;
        }

        ob = obs[0];
        if(!objectp(ob) || !ob->query_weapon_class())
            return notify_fail(wrap(Den(ob)+" kann man nicht führen."), 
                    FAIL_WRONG_ARG);
        if (member(hand_objects,ob) >= 0)
            return notify_fail(wrap("Du führst "+den(ob,"")+" bereits!"), 
                    FAIL_INTERNAL);
        return notify_fail(wrap(Den(ob)+" kann man nicht führen."), 
                    FAIL_WRONG_ARG);
    }
}

/*
FUNKTION: no_wield
DEKLARATION: int no_wield(object lebewesen)
BESCHREIBUNG:
Diese Funktion wird in einer Waffe vor dem Fuehren derselben aufgerufen.
Parameter ist das Lebewesen, das versucht, die Waffe zu fuehren.
Liefert die Waffe einen Wert ungleich 0 zurueck, so kann die Waffe nicht
gefuehrt werden.
Die Waffe muss im Falle eines nicht-fuehren duerfens selber eine Meldung
geben, beispielsweise mit send_message_to, nicht jedoch mit notify_fail,
warum das Lebewesen die Waffe nicht fuehren durfte.
VERWEISE: unwield, wield
GRUPPEN: spieler, monster, kampf, haende
*/

/*
FUNKTION: wield
DEKLARATION: varargs int wield(object waffe, int hand, int flags)
BESCHREIBUNG:
Mit wield wird die Waffe waffe in der Hand mit dem Index hand gefuehrt.
Mit dem optionalen flags&1==1 wird keine Erfolgsmeldung auszugeben.
Bei optionalen flags&2==2 wird das mehrhaendige Fuehren einer Waffe zugelassen.
VERWEISE: unwield, no_wield
GRUPPEN: spieler, monster, kampf, haende
*/
varargs int wield(object ob, int hand, int flags)
{
    int otherhand;
    if (hand < 0    
        || sizeof(hand_objects) <= hand || sizeof(hand_enemies)<= hand
        || sizeof(hand_hits) <= hand)
    {
        do_error("Ungültiger Parameter hand:"+hand);
        return 0;
    }

#ifdef NEW_STATS
    if (this_object()->query_stat(STAT_INT) < 25)
        return 0;
#endif

    FIGHT_X_DEBUG("hands/wield",
            sprintf("1 weapon %Q hand %d flags %d",ob,hand,flags));
    if (this_object()->forbidden("wield", ob, hand) || 
            ob->forbidden("wield_me", this_object(), hand) ||
            ob->no_wield(this_object()))
        return 0;
    if (!(flags&2))
    {
        for(otherhand = 0;otherhand < sizeof(hand_objects);otherhand++)
        {
            if (otherhand == hand)
                continue;
            if (ob && hand_objects[otherhand]==ob)
            {
                return notify_fail(wrap("Du kannst "+deinen(ob)
                    +" nicht mehrfach führen."),FAIL_NOT_OBJ);
            }
        }
    }

    if(!(flags&1))
    {
        notify_message(wrap(ob->query_wield_msg()||
            ("Du führst "+deinen(ob)+".")),MA_WIELD);
        this_object()->send_message(MT_LOOK,MA_WIELD,wrap(
            ob->query_wield_msg_other()||(Der()+" führt "+seinen(ob)+".")));
    }

    hand_objects[hand] = ob;
    ob->do_wield();
    this_object()->notify("wield", ob, hand);
    if(ob)
        ob->notify("wield_me", this_object(), hand);

    if(hand_enemies[hand])
    {
        // Kampf beenden und mit neuer Waffe neu starten.
        // (Wegen Controllern, die Kampf mit einer bestimmten Waffe
        // verhindern, die neue Meldung deswegen ist kein Problem.)
        object enemy = hand_enemies[hand];

        if (!present(enemy, environment()))
        {
            if (gone_ob)
                gone_ob->enemy_gone(enemy);
            FIGHT_X_DEBUG("hands/wield",
                sprintf("2 enemy %Q gone",enemy));

            delete_enemy(enemy);
        }
        else
        {
            hand_enemies[hand] = 0;

            attack(enemy, hand_objects[hand], hand);

            // Falls der Versuch misslang und der Gegner noch
            // existiert, andere Hand nehmen.
            if(!hand_enemies[hand] && enemy)
            {
                int nexthand = free_fight();
                FIGHT_X_DEBUG("hands/wield",
                    sprintf("3 next_hand %d",nexthand));
                if(nexthand>=0)
                    attack(enemy, hand_objects[nexthand], nexthand);
            }
        }
    }

    return 1;
}

/*
FUNKTION: forbidden_wield
DEKLARATION: int forbidden_wield(object weapon, int hand, object who)
BESCHREIBUNG:
Bevor ein Lebewesen who die Waffe weapon mit der Hand hand fuehren kann,
wird who->forbidden("wield", weapon, hand) aufgerufen, liefert dieser Aufruf
einen Wert ungleich 0 zurueck, wird die Waffe nicht gefuehrt.

Die Funktion forbidden ruft in allen mit who->add_controller("forbidden_wield",
other) angemeldeten Objekten other die Funktionen other->forbidden_wield(
weapon, hand, who) auf. Liefert auch nur eine dieser Funktionen einen Wert
ungleich 0, dann returnt forbidden diesen und die Waffe kann nicht gefuehrt
werden.

Fuer die Ausgabe einer Meldung an das Lebewesen who und evtl. den Raum muss
der Programmierer in forbidden_wield oder forbidden, falls er diese Funktion
ueberlagern will, sorgen.

Beispielanwendung: Auf Monster oder Traeger 'spezialisierte' Waffen, der
Runenbeutel der Magier sorgt dafuer, dass Magier keine Metallwaffen fuehren
koennen. Oder eine verletzte Hand, die kurzzeitig keine Waffe fuehren kann.

Bemerkung: Es wird auch weapon->forbidden("wield_me", who, hand) aufgerufen. 
VERWEISE: forbidden, notify, notify_wield, attack, forbidden_use
GRUPPEN: spieler, monster, haende, waffen
*/

/*
FUNKTION: forbidden_wield_me
DEKLARATION: int forbidden_wield_me(object who, int hand, object weapon)
BESCHREIBUNG:
Bevor ein Lebewesen who die Waffe weapon mit der Hand hand fuehren kann,
wird weapon->forbidden("wield_me", who, hand) aufgerufen, liefert dieser
Aufruf einen Wert ungleich 0 zurueck, wird die Waffe nicht gefuehrt.

Die Funktion forbidden ruft in allen mit who->add_controller(
"forbidden_wield_me", other) angemeldeten Objekten other die Funktionen
other->forbidden_wield_me(who, hand, weapon) auf. Liefert auch nur eine
dieser Funktionen einen Wert ungleich 0, dann returnt forbidden diesen und
die Waffe kann nicht gefuehrt werden.

Fuer die Ausgabe einer Meldung an das Lebewesen who und evtl. den Raum muss
der Programmierer in forbidden_wield_me oder forbidden, falls er diese
Funktion ueberlagern will, sorgen.

Beispielanwendung: Auf Monster oder Traeger 'spezialisierte' Waffen, der
Runenbeutel der Magier sorgt dafuer, dass Magier keine Metallwaffen fuehren
koennen.

Bemerkung: Es wird auch who->forbidden("wield", weapon, hand) aufgerufen. 
VERWEISE: forbidden, notify, notify_wield, attack, forbidden_use
GRUPPEN: spieler, monster, haende, waffen
*/

/*
FUNKTION: notify_wield
DEKLARATION: void notify_wield(object weapon, int hand, object who)
BESCHREIBUNG:
Nachdem das Lebewesen who die Waffe weapon in der Hand hand gefuehrt hat,
wird who->notify("wield", weapon, hand) aufgerufen.

Die Funktion notify ruft in allen mit who->add_controller("notify_wield",
other) angemeldeten Objekten other die Funktionen other->notify_wield(weapon,
hand, who) auf. Sowohl who als auch other haben dann eine Moeglichkeit, auf
das Fuehren der Waffe weapon zu reagieren.

Bemerkung: Es wird auch weapon->notify("wield_me", who, hand) aufgerufen. 
VERWEISE: forbidden, notify, forbidden_wield, notify_use
GRUPPEN: spieler, monster, haende, waffen
*/

/*
FUNKTION: notify_wield_me
DEKLARATION: void notify_wield_me(object who, int hand, object weapon)
BESCHREIBUNG:
Nachdem das Lebewesen who die Waffe weapon in der Hand hand gefuehrt hat,
wird weapon->notify("wield_me", who, hand) aufgerufen.

Die Funktion notify ruft in allen mit weapon->add_controller("notify_wield_me",
other) angemeldeten Objekten other die Funktionen other->notify_wield_me(who,
hand, weapon) auf. Sowohl weapon als auch other haben dann eine Moeglichkeit,
auf das Fuehren der Waffe zu reagieren.

Bemerkung: Es wird auch who->notify("wield", weapon, hand) aufgerufen. 
VERWEISE: forbidden, notify, forbidden_wield, notify_use
GRUPPEN: spieler, monster, haende, waffen
*/

int remove_command(string str)
{
    mixed *parsed,*obs,ob;
    string *words;
    string *alles;
    int hand,i,j,all,size;

    if ((query_verb()=="steck")||(query_verb()=="stecke")) 
    {
        if (!str || (str[<4..] != " weg")) 
            return 0;
        str = str[0..<5];
    } 
    else
        if (!str)
            return notify_fail("Was willst du senken?\n", FAIL_NOT_OBJ);
    // Es wird geprueft, ob der Spieler mehrere Waffen senken will
    alles = ALLES;
    size = sizeof(alles);
    words = explode(str," ");
    do all = member(words,alles[i]) >= 0; while (!all && ++i<size);
    parsed = parse_com(str,this_object(),0,PARSE_ALL_MATCHES);
    if (parse_com_error(parsed,"Senke was?\n"))
        return 0;
    size = sizeof(obs = parsed[PARSE_OBS]);
    i = 0;
    if (all)
    { // Alle gefundenen Waffen senken
        string *dnames = ({});
        string *snames = ({});

        notify_fail("Davon kannst du nichts senken.\n");
        while (i<size && this_object())
        {
            CHECK_EVAL_COST
            if (objectp(ob = obs[i++]) && ob->query_weapon_class() &&
                (hand = member(hand_objects,ob)) >= 0)
            {
                string sname, dname;
                if(!ob->query_unwield_msg())
                {
                    dname = deinen(ob);
                    sname = seinen(ob);
                }
                if(this_object()->unwield(ob, hand, dname?1:0))
                {
                    j++;
                    if(dname)
                    {
                        dnames += ({dname});
                        snames += ({sname});
                    }
                }
            }
        }
        if(sizeof(dnames) && this_object())
        {
            notify_message(wrap("Du senkst "+liste(dnames)+"."),MA_UNWIELD);
            this_object()->send_message(MT_LOOK,MA_UNWIELD,
                wrap(Der()+" senkt "+liste(snames)+"."));
        }
        return j > 0;
    }
    else
    { // Die erste moegliche Waffe senken
        while ((!objectp(ob = obs[i]) ||
            (hand = member(hand_objects,ob)) == -1 ||
            !ob->query_weapon_class()) && ++i<size);
        if (i<size)
        {
            this_object()->unwield(ob, hand);
            return 1;
        }
        if (member(hand_objects,ob = obs[0]) == -1)
            return notify_fail(wrap("Du führst "+den(ob,"")+" nicht!"), 
                FAIL_INTERNAL);
        return notify_fail(wrap(Den(ob)+" kann man nicht senken."), 
                FAIL_WRONG_ARG);
    }
}

/*
FUNKTION: unwield
DEKLARATION: varargs int unwield(object ob, int hand, int flags)
BESCHREIBUNG:
Mit unwield kann man das gefuehrte Objekt ob aus der der Hand mit dem Index
hand wieder senken. Beim optionalen flags==1 wird keine Meldung ausgegeben.
VERWEISE: wield, no_wield
GRUPPEN: spieler, monster, kampf, haende
*/
varargs int unwield(object ob, int hand, int flags)
{
    FIGHT_X_DEBUG("hands/unwield",sprintf("1 weapon %Q,hand %d flags %d",
                        ob,hand,flags));
    if (this_object()->forbidden("unwield", ob, hand) ||
        ob->forbidden("unwield_me", this_object(), hand))
        return 0;

    hand_objects[hand] = 0;
    ob->do_remove();

    if (!(flags&1))
    {
        notify_message(wrap(ob->query_unwield_msg()||
            ("Du senkst "+deinen(ob)+".")),MA_UNWIELD);
        this_object()->send_message(MT_LOOK,MA_UNWIELD,
            wrap(ob->query_unwield_msg_other()||
                (Der()+" senkt "+seinen(ob)+".")));
    }
    this_object()->notify("unwield", ob, hand);
    if(ob)
        ob->notify("unwield_me", this_object(), hand);
    FIGHT_X_DEBUG("hands/unwield",sprintf("2 weapon %Q,hand %d flags %d",
                        ob,hand,flags));
    return 1;
}

/*
FUNKTION: forbidden_unwield
DEKLARATION: int forbidden_unwield(object weapon, int hand, object who)
BESCHREIBUNG:
Bevor ein Lebewesen who die Waffe weapon in der Hand hand senken kann,
wird who->forbidden("unwield", weapon, hand) aufgerufen, liefert dieser Aufruf
einen Wert ungleich 0 zurueck, wird die Waffe nicht gesenkt.

Die Funktion forbidden ruft in allen mit who->add_controller("forbidden_unwield",
other) angemeldeten Objekten other die Funktionen other->forbidden_unwield(
weapon, hand, who) auf. Liefert auch nur eine dieser Funktionen einen Wert
ungleich 0, dann returnt forbidden diesen und die Waffe kann nicht gesenkt
werden.

Fuer die Ausgabe einer Meldung an das Lebewesen who und evtl. den Raum muss
der Programmierer in forbidden_unwield oder forbidden, falls er diese Funktion
ueberlagern will, sorgen.

Bemerkung: Es wird auch weapon->forbidden("unwield_me", who, hand) aufgerufen. 
VERWEISE: forbidden, notify, notify_unwield, attack, forbidden_use
GRUPPEN: spieler, monster, haende, waffen
*/

/*
FUNKTION: forbidden_unwield_me
DEKLARATION: int forbidden_unwield_me(object who, int hand, object weapon)
BESCHREIBUNG:
Bevor ein Lebewesen who die Waffe weapon in der Hand hand senken kann,
wird weapon->forbidden("unwield_me", who, hand) aufgerufen, liefert dieser
Aufruf einen Wert ungleich 0 zurueck, wird die Waffe nicht gesenkt.

Die Funktion forbidden ruft in allen mit who->add_controller(
"forbidden_unwield_me", other) angemeldeten Objekten other die Funktionen
other->forbidden_unwield_me(who, hand, weapon) auf. Liefert auch nur eine
dieser Funktionen einen Wert ungleich 0, dann returnt forbidden diesen und
die Waffe kann nicht gesenkt werden.

Fuer die Ausgabe einer Meldung an das Lebewesen who und evtl. den Raum muss
der Programmierer in forbidden_unwield_me oder forbidden, falls er diese
Funktion ueberlagern will, sorgen.

Beispielanwendung: verzauberte Waffen, die sich nicht so einfach senken lassen

Bemerkung: Es wird auch who->forbidden("unwield", weapon, hand) aufgerufen. 
VERWEISE: forbidden, notify, notify_unwield, attack, forbidden_use
GRUPPEN: spieler, monster, haende, waffen
*/

/*
FUNKTION: notify_unwield
DEKLARATION: void notify_unwield(object weapon, int hand, object who)
BESCHREIBUNG:
Nachdem das Lebewesen who die Waffe weapon in der Hand hand gesenkt hat,
wird who->notify("unwield", weapon, hand) aufgerufen.

Die Funktion notify ruft in allen mit who->add_controller("notify_unwield",
other) angemeldeten Objekten other die Funktionen other->notify_unwield(weapon,
hand, who) auf. Sowohl who als auch other haben dann eine Moeglichkeit, auf
das Senken der Waffe weapon zu reagieren.

Bemerkung: Es wird auch weapon->notify("unwield_me", who, hand) aufgerufen. 
VERWEISE: forbidden, notify, forbidden_unwield, notify_use
GRUPPEN: spieler, monster, haende, waffen
*/

/*
FUNKTION: notify_unwield_me
DEKLARATION: void notify_unwield_me(object who, int hand, object weapon)
BESCHREIBUNG:
Nachdem das Lebewesen who die Waffe weapon in der Hand hand gesenkt hat,
wird weapon->notify("unwield_me", who, hand) aufgerufen.

Die Funktion notify ruft in allen mit weapon->add_controller("notify_unwield_me",
other) angemeldeten Objekten other die Funktionen other->notify_unwield_me(who,
hand, weapon) auf. Sowohl weapon als auch other haben dann eine Moeglichkeit,
auf das Senken der Waffe zu reagieren.

Bemerkung: Es wird auch who->notify("unwield", weapon, hand) aufgerufen. 
VERWEISE: forbidden, notify, forbidden_unwield, notify_use
GRUPPEN: spieler, monster, haende, waffen
*/


/* ====================== RUESTUNGEN ====================== */

int add_armour(object armour)
{
   if(objectp(armour) &&
      environment(armour) == this_object() &&
      armour->query_armour())
    {
        armours -= ({ armour, 0 });
        armours = sort_array(armours + ({ armour }),
                   (:
                       mapping armour_sort=(["magie":0,
                                                   0:1,
                                            "haende":2,
                                              "kopf":3,
                                       "oberkoerper":4,
                                             "beine":5,
                                            "fuesse":6]);
                       return armour_sort[$1->query_armour_class()] >
                              armour_sort[$2->query_armour_class()];
                   :));
        FIGHT_X_DEBUG("hands/add_armour",sprintf("1 armours %Q",armours));
        return 1;
    }
}

int delete_armour(object armour)
{
    armours -= ({ armour });
    FIGHT_X_DEBUG("hands/delete_armour",sprintf("1 armours %Q",armours));
    return 1;
}

int armour_worn(object armour)
{
   return member(armours, armour) >= 0;
}

object *query_armours()
{
   return copy(armours);
}

int query_armour_protection()
{
    int i, armour_protection;

    for(i = sizeof(armours -= ({ 0 })); i--;)
       if(environment(armours[i]) == this_object())
        armour_protection += armours[i]->query_armour_protection();
       else
        armours[i] = 0;
    return armour_protection;
}

object armour_class(string str)
{
    int i;

    for(i = sizeof(armours -= ({ 0 })); i--;)
       if(environment(armours[i]) != this_object())
        armours[i] = 0;
       else
        if(armours[i]->armour_class(str))
             return armours[i];
}

/* ====================== H P - KRIMSKRAMS ================ */



void update_max_hp()
{
    int tmp, old = max_hp;

#ifdef NEW_STATS
    int con;

    con = this_object()->query_stat(STAT_CON);
    if (con < 30)
        tmp = con * 5 / 3 + 4;
    else if (con < 81)
            tmp = con * 25 / 8 - 40;
         else
            tmp = con * 2 + 50;
#else
    tmp = 50 + this_object()->query_stat(STAT_CON) * 2;
#endif
    if (query_wiz_level())
    {
        if (tmp > max_hp)
            max_hp = tmp;
    }
    else
        max_hp = tmp;
    if (playerp (this_object())
        && (hp > (tmp = this_object()->query_max_hp())))
        hp = tmp;

    if(old != max_hp)
        this_object()->update_points_display();
}


/*
FUNKTION: set_max_hp
DEKLARATION: void set_max_hp(int max_hp)
BESCHREIBUNG:
Damit werden die maximalen APs eines Spielers(Monsters) gesetzt.
VERWEISE: set_hp, query_hp, query_max_hp
GRUPPEN: spieler, monster, kampf
*/
void set_max_hp(int i)
{
    int tmp, old = max_hp;

    if(!intp(i))
        raise_error("Bad argument 1 to set_max_hp: not an integer\n");

    max_hp = i < 0 ? 0 : i;
    if (old == max_hp)
        return;

    if (extended_hp_view)
        notify_message(wrap("Du hast jetzt maximal " + max_hp +
                " Ausdauerpunkte."));
    if (hp > (tmp = this_object()->query_max_hp()))
        hp = tmp;

    this_object()->update_points_display();
}


/*
FUNKTION: set_hp
DEKLARATION: void set_hp(int hp)
BESCHREIBUNG:
Damit kann man die APs eines Spielers(Monsters) setzen, aber nur bis 
maximal query_max_hp().
VERWEISE: give_hp, add_hp, query_hp, query_max_hp, set_max_hp
GRUPPEN: monster, spieler, kampf
*/
void set_hp(int new_hp)
{
    int tmp_max, old = hp;

    if(!intp(new_hp))
        raise_error("Bad argument 1 to set_hp: not an integer\n");

    tmp_max = this_object()->query_max_hp ();
    if (new_hp <= 0)
        hp = 0;
    else
        hp = new_hp > tmp_max ? tmp_max : new_hp;

    if (old == hp)
        return;
    if (extended_hp_view)
        notify_message(wrap("Du hast jetzt " + hp + " Ausdauerpunkte."));

    this_object()->update_points_display();
}

string rlist(string *list)
{
   string ret;

   ret = "";
   if(sizeof(list))
   {
      if(sizeof(list) > 1)
            ret += implode(list[0..<2], ", ")+" und ";
      ret += list[<1];
   }
   return ret;
}

private int want_combat_message(object who)
{
   return who && (interactive(who) || who->query_eyes_open());
}

string *combat_message(object defender,
                       object attacker, 
                       mixed waffe,
                       mapping combat_log)
{
   // mixed waffe: 
   //    Die Waffe mit der Angreifer angeriffen hat, falls ermittelbar.
   //    Darf auch ein Mapping sein.
   // object attacker: 
   //    Der Angreifer
   // object defender:
   //    Der Verteidiger (this_object())
   // int flag
   //    das flag von add_hp

   // mapping combat_log: (fuer jeden schlag neu)
   //
   // defender_ob : schutzwirkung durch armourlevel des defenders in hp; in %%,
   // ruestung1_ob: schutzwirkung durch ruestung1 in hp; in Promille(%%),
   // ruestung2_ob: ...,
   // ...,
   // attacker_ob : urspruenglicher schaden in hp; 1000,
   // "mydamage"  : schaden der bleibt nach dem vom urspruenglichen schaden
   //               alle Ruestungen/Eigenruestung abgezogen wurden in hp, in %%
   //               (dieser Eintrag wird z:Zt noch nicht benutzt von
   //               combat_message
   // "flag"      : das flag von add_hp
   //


   int i;
   int schutz, parade_schutz; // Abgehaltener Schaden durch Ruestungen
                              // bzw defensivwaffen in Promille (%%)
   string mess_att, mess_def, mess_oth;  // returnmeldungen
   string how, what;
   object *ruest;
   mixed *r1, *r2, *r3, parade;
   int p1, p2, p3;

   how = mess_def = mess_att = mess_oth = "";
   if (random (1000) == 17) // eine etwas andere Meldung
      switch(combat_log[attacker])
      {
        case 0:      what = "bring"  ; how = " auf die Palme";    break;
        case 1:      what = "kitzel" ; how = " gemein";           break;
        case 2:      what = "streif" ; how = " ein wenig";        break;
        case 3..4:   what = "triff"  ; how = " übel";            break;
        case 5..9:   what = "triff"  ; how = " fürchterbarlich"; break;
        case 10..19: what = "triff"  ; how = " entsetzlich";      break;
        case 20..29: what = "klatsch"; how = " auf den Boden";    break;
        default:     what = "hau"    ; how = " in Fetzen";        break;
      }
   else
      switch(combat_log[attacker])
      {
        case 0:      what = "verfehl";                      break;
        case 1:      what = "kitzel" ;                      break;
        case 2:      what = "streif" ;                      break;
        case 3..4:   what = "triff"  ;                      break;
        case 5..9:   what = "triff"  ; how = " hart";       break;
        case 10..19: what = "triff"  ; how = " sehr stark"; break;
        case 20..29: what = "zerschmetter";
                how = " mit einem markerschütternden Geräusch"; break;
        default:     what = "massakrier"; how = " in Fraktale"; break;
      }
   if(want_combat_message(defender))
   {
      mess_def = "";
      if(combat_log["flag"] & AH_CRITICAL)
         mess_def += Wer(attacker,ART_AAA)+" landet einen kritischen Treffer! ";
      if(!defender->query_short_combat_msg() &&
            combat_log["flag"] & AH_NO_ARMOUR)
        mess_def += "Weder Schild noch Rüstung helfen dir "
                "bei diesem mörderischen Schlag: ";
      mess_def += Wer(attacker, ART_AAA)+" "+what+"t dich"+
          (waffe ? " mit "+seinem(waffe,mappingp(waffe)?0:({}),attacker) : "")
          + how+".";
   }
   if(want_combat_message(attacker))
   {
      mess_att = "";
      if(combat_log["flag"] & AH_CRITICAL)
         mess_att += "Du landest einen kritischen Treffer! ";
      if(!attacker->query_short_combat_msg() &&
         combat_log["flag"] & AH_NO_ARMOUR)
         mess_att += Der(defender)+" kann nicht parieren: ";
      mess_att += "Du "+what+"st "+den(defender)+
            (waffe ? " mit "+deinem(waffe,"",attacker) : "")+ how+".";
   }
   mess_oth = Wer(attacker, ART_AAA)+" "+what+"t "+ wen(defender, ART_AAA)+
              (waffe ? " mit "+seinem(waffe,mappingp(waffe)?0:({}),attacker) : "")+ how+".";
   schutz = parade_schutz = 0;
   r1 = r2 = r3 = parade = ({});
   p1 = p2 = p3 = 0;
   for(i = sizeof(ruest =
               m_indices(combat_log)-({attacker,0,"mydamage","flag"})); i--;)
   {
      if(ruest[i]->query_weapon())
      {
         parade += ({ ruest[i] });
         parade_schutz += combat_log[ruest[i],1];
      }
      else
      {
        if(ruest[i] != defender)
            switch(combat_log[ruest[i],1])
            {
            case   1.. 200:
                r1 += ({ ruest[i] });
                if(!p1)
                    p1 = strlen(plural(" ","  ",ruest[i]));
                else
                    p1++;
                break;
            case 201.. 600:
                r2 += ({ ruest[i] });
                if(!p2)
                    p2 = strlen(plural(" ","  ",ruest[i]));
                else
                    p2++;
                break;
            case 601..1000:
                r3 += ({ ruest[i] }); 
                if(!p3)
                    p3 = strlen(plural(" ","  ",ruest[i]));
                else
                    p3++;
                break;
            }
        schutz += combat_log[ruest[i],1];
      }
   }
   r1 = map(r1, #'dein,"");
   r2 = map(r2, #'dein,"");
   r3 = map(r3, #'dein,"");
   parade = map(parade, #'deinem, "");

   if(parade_schutz)
   {
      switch(parade_schutz)
      {
        case 1..100: how = " miserabel"; break;
        case 101..350: how = ""; break;
        case 351..750: how = " gut"; break;
        case 751..1000: how = " ausgezeichnet"; break;
      }
      if(!defender->query_short_combat_msg() && want_combat_message(defender))
        mess_def += " Du parierst den Schlag"+how+" mit "+rlist(parade)+".";
      if(!attacker->query_short_combat_msg() && want_combat_message(attacker))
        mess_att += " "+Er(defender)+" pariert"+how+".";
      mess_oth += " "+Der(defender)+" pariert.";
   }
   if(want_combat_message(defender) && !defender->query_short_combat_msg())
   {
      if(sizeof(r3))
        mess_def += " "+capitalize(rlist(r3))+" "+
                (p3<2?"hält":"halten")+
                " das meiste ab";
        if(sizeof(r2))
            if(sizeof(r3))
                mess_def += ", "+
                    rlist(r2)+" "+
                    (p2<2?"hilft":"helfen")+
                    " auch";
            else 
                mess_def += " "+capitalize(rlist(r2))+" "+
                    (p2<2?"hält":"halten")+
                    " ein bisschen was ab";
        if(sizeof(r1))
            if(sizeof(r2) || sizeof(r3))
                mess_def += ", "+
                        rlist(r1)+" "+
                        (p1<2?"hilft":"helfen")+
                        " dagegen nur wenig";
            else
                mess_def += " "+capitalize(rlist(r1))+" "+
                    (p1<2?"hält":"halten")+
                    " leider nur wenig ab";
        if(sizeof(r1) || sizeof(r2) || sizeof(r3))
            mess_def += ".";
   }

   // AB HIER wird die Variable parade und i reused.

   if(want_combat_message(attacker) && !attacker->query_short_combat_msg())
   {
      i = 0;
      // Wenn die Eigenpanzerung groesser als die 
      // Ruestung, dann erscheint Panzerung, sonst Ruestung...
      if(member(combat_log, defender) && schutz < 2*combat_log[defender,1])
      {
        // parade sollte besser panzerung heissen :(
        parade = defender->query_v_item(({(["name":"natural#armour"])})) ||
                (["name": "panzerung", "gender":"weiblich"]);
        what = Sein(parade, 0, defender);
        i = strlen(plural(""," ",parade));
      }
      else
        what = Sein((["name": "rüstung", "gender":"weiblich"]),0, defender);
      switch(schutz)
      {
        case 1..300:
            mess_att+=" "+what+(i ? " schützen " : " schützt ")+
            ihn(defender)+" nur wenig."; break;
        case 301..600:
            mess_att+=" "+what+(i ? " schützen " : " schützt ")+
            ihn(defender)+" recht gut."; break;
        case 601..900:
            mess_att+=" "+what+(i ? " schützen " : " schützt ")+
            ihn(defender)+" sehr gut."; break;
        case 901..1000:
            mess_att+=" "+what+(i ? " bewahren " : " bewahrt ")+
            ihn(defender)+" vor deinem Schlag.";
            break;
      }
   }
   FIGHT_X_DEBUG("hands/combat_message",sprintf("1 %s",mess_oth ));
   return ({ mess_def, mess_att, mess_oth });
} 

/*
FUNKTION: get_combat_message
DEKLARATION: string* get_combat_message(object victim, int hps, mapping infos)
BESCHREIBUNG:
Berechnet aus den angegeben Informationen die Kampfmeldungen fuer das
Opfer, alle anderen und den Angreifer und liefert sie in dieser Reihenfolge
in einem Array zurueck. Es beachtet dabei, ob Opfer oder Angreifer
kurze Kampfmeldungen haben wollen.
VERWEISE: add_hp
GRUPPEN: kampf
*/
#define PLURAL(singstr,plurstr,obs)	\
    ((sizeof(obs)>1)?(plurstr):plural((singstr),(plurstr),(obs)[0]))
string* get_combat_message(object victim, int hps, mapping infos)
{
    mapping combat_log;
    object attacker;
    string how, what;
    string* msgs = allocate(AH_MESG_SIZE, "");
    
    // Fuer die Schildmeldungen
    mixed parade;
    int paradeschutz;
    
    // Fuer die Ruestungen
    mixed *ruestlow, *ruestmed, *ruesthigh;
    int ruestschutz;
    
    if(!infos)
        return ({"", "", ""});	// Kein Kampf.

    attacker = infos[AH_ATTACKER];
    if(!attacker)
        return ({"", "", ""});

    combat_log = infos[AH_COMBAT_LOG];
    if(!combat_log)             // Wir sind sehr tolerant...
        combat_log = ([attacker: -hps; 1000]);

    if (random (1000) == 17) // eine etwas andere Meldung
        switch(combat_log[attacker])
        {
            case 0:      what = "bring"  ; how = " auf die Palme";    break;
            case 1:      what = "kitzel" ; how = " gemein";           break;
            case 2:      what = "streif" ; how = " ein wenig";        break;
            case 3..4:   what = "triff"  ; how = " übel";            break;
            case 5..9:   what = "triff"  ; how = " fürchterbarlich"; break;
            case 10..19: what = "triff"  ; how = " entsetzlich";      break;
            case 20..29: what = "klatsch"; how = " auf den Boden";    break;
            default:     what = "hau"    ; how = " in Fetzen";        break;
        }
    else
        switch(combat_log[attacker])
        {
            case 0:      what = "verfehl"; how = "";                break;
            case 1:      what = "kitzel" ; how = "";                break;
            case 2:      what = "streif" ; how = "";                break;
            case 3..4:   what = "triff"  ; how = "";                break;
            case 5..9:   what = "triff"  ; how = " hart";           break;
            case 10..19: what = "triff"  ; how = " sehr stark";	    break;
            case 20..29: what = "zerschmetter";
                    how = " mit einem markerschütternden Geräusch";
                    break;
            default:     what = "massakrier"; how = " in Fraktale"; break;
        }
    
    if(infos[AH_FLAGS] & AH_CRITICAL)
    {
        msgs[AH_MESG_VICTIM] += Wer(attacker,ART_AAA) +
                    " landet einen kritischen Treffer! ";

        msgs[AH_MESG_ATTACKER] += "Du landest einen kritischen Treffer! ";
    }
    
    if(infos[AH_FLAGS] & AH_NO_ARMOUR)
    {
        if(!victim->query_short_combat_msg())
            msgs[AH_MESG_VICTIM] += "Weder Schild noch Rüstung helfen "
                        "dir bei diesem mörderischen Schlag: ";
                        
        if(!attacker->query_short_combat_msg())
            msgs[AH_MESG_ATTACKER] += Der(victim)+" kann nicht parieren: ";
    }
    
    msgs[AH_MESG_VICTIM] += Wer(attacker, ART_AAA) + " " + what+"t dich";
    msgs[AH_MESG_ATTACKER] += "Du "+what+"st "+den(victim);
    msgs[AH_MESG_OTHER] = Wer(attacker, ART_AAA) + " " + what+"t " +
                wen(victim, ART_AAA);
    
    if(infos[AH_PROJECTILE])
    {
        string msg = " mit " + einem(infos[AH_PROJECTILE]);
        
        msgs[AH_MESG_VICTIM] += msg;
        msgs[AH_MESG_ATTACKER] += msg;
        msgs[AH_MESG_OTHER] += msg;
    }
    else if(infos[AH_WEAPON])
    {
        string msg = " mit " + seinem(infos[AH_WEAPON], 
                mappingp(infos[AH_WEAPON])?0:({}), attacker);

        msgs[AH_MESG_VICTIM] += msg;
        msgs[AH_MESG_ATTACKER] += " mit " + deinem(infos[AH_WEAPON], 
                                            ({}), attacker);
        msgs[AH_MESG_OTHER] += msg;
    }
    
    
    msgs[AH_MESG_VICTIM] += how + ".";
    msgs[AH_MESG_ATTACKER] += how + ".";
    msgs[AH_MESG_OTHER] += how + ".";
        
    // Und nun noch die Ruestungsmeldungen...
    parade = ruestlow = ruestmed = ruesthigh = ({});
    foreach(object ruestung, int schutz, int promille : combat_log)
    {
        if(ruestung == attacker || !objectp(ruestung))
            continue;
    
        if(ruestung == victim)  // natuerliche Panzerung
        {
            ruestschutz += promille;
        }
        else if(ruestung->query_weapon())	// Schilde etc.
        {
            parade += ({ ruestung });
            paradeschutz += promille;
        }
        else    // Ruestungen
        {
            switch(promille)
            {
            case   1.. 200:
                ruestlow += ({ ruestung });
                break;
            case 201.. 600:
                ruestmed += ({ ruestung });
                break;
            case 601..1000:
                ruesthigh += ({ ruestung });
                break;
            }
            ruestschutz += promille;
        }
    }
    
    if(paradeschutz)
    {
        switch(paradeschutz)
        {
            case 1..100: how = " miserabel"; break;
            case 101..350: how = ""; break;
            case 351..750: how = " gut"; break;
            case 751..1000: how = " ausgezeichnet"; break;
        }

        if(!victim->query_short_combat_msg())
            msgs[AH_MESG_VICTIM] += " Du parierst den Schlag" + how + " mit " +
            liste(map(parade, #'deinem, ({}))) + ".";

        if(!attacker->query_short_combat_msg())
            msgs[AH_MESG_ATTACKER] += " " + Er(victim) + " pariert" + how +".";
        
        msgs[AH_MESG_OTHER] += " "+Der(victim)+" pariert.";
    }
    
    if(!victim->query_short_combat_msg())
    {
        if(sizeof(ruesthigh))
            msgs[AH_MESG_VICTIM] += " " +
            capitalize(liste(map(ruesthigh, #'dein, ({})))) + " " +
            PLURAL("hält","halten",ruesthigh) + " das meiste ab";
    
        if(sizeof(ruestmed))
        {
            if(sizeof(ruesthigh))
                msgs[AH_MESG_VICTIM] += ", " +
                    liste(map(ruestmed, #'dein, ({}))) + " " +
                    PLURAL("hilft","helfen", ruestmed) + " auch";
            else 
                msgs[AH_MESG_VICTIM] += " " +
                    capitalize(liste(map(ruestmed, #'dein, ({})))) + " " +
                    PLURAL("hält","halten",ruestmed) + " ein bisschen was ab";
        }
        
        if(sizeof(ruestlow))
        {
            if(sizeof(ruestmed) || sizeof(ruesthigh))
                msgs[AH_MESG_VICTIM] += ", " +
                    liste(map(ruestlow, #'dein, ({}))) + " " +
                    PLURAL("hilft","helfen",ruestlow) + " dagegen nur wenig";
            else
                msgs[AH_MESG_VICTIM] += " " +
                    capitalize(liste(map(ruestlow, #'dein, ({})))) + " " +
                    PLURAL("hält","halten",ruestlow) + " leider nur wenig ab";
        }

        if(sizeof(ruesthigh) || sizeof(ruestmed) || sizeof(ruestlow))
            msgs[AH_MESG_VICTIM] += ".";
    }

    if(!attacker->query_short_combat_msg())
    {
        // Wenn die Eigenpanzerung groesser als die 
        // Ruestung, dann erscheint Panzerung, sonst Ruestung...
        
        if(member(combat_log, victim) && ruestschutz < 2*combat_log[victim,1])
            parade = victim->query_v_item(({ (["name":"natural#armour"]) })) ||
                (["name": "panzerung", "gender":"weiblich"]);
        else
            parade = (["name": "rüstung", "gender":"weiblich"]);
    
        switch(ruestschutz)
        {
            case 1..300:
                msgs[AH_MESG_ATTACKER] += " " + Sein(parade, 0, victim) +
                    plural(" schützt ", " schützen ", parade) +
                    ihn(victim)+" nur wenig.";
                break;
            case 301..600:
                msgs[AH_MESG_ATTACKER] += " " + Sein(parade, 0, victim) +
                    plural(" schützt ", " schützen ", parade) +
                    ihn(victim)+" recht gut.";
                break;

            case 601..900:
                msgs[AH_MESG_ATTACKER] += " " + Sein(parade, 0, victim) +
                    plural(" schützt ", " schützen ", parade) +
                    ihn(victim)+" sehr gut.";
                break;
            case 901..1000:
                msgs[AH_MESG_ATTACKER] += " " + Sein(parade, 0, victim) +
                    plural(" bewahrt ", " bewahren ", parade) +
                    ihn(victim)+" vor deinem Schlag.";
                break;
        }
    }
    FIGHT_X_DEBUG("hands/get_combat_message",sprintf("1 %s",
            msgs[AH_MESG_OTHER] ));
    
    return msgs;
}
#undef PLURAL

/*
FUNKTION: add_hp
DEKLARATION: varargs int add_hp(int hps, mapping infos)
BESCHREIBUNG:
Gibt dem Vieh, dem diese Funktion gehoert, ein paar APs oder
zieht sie ihm AP (wenn 'hps' negativ ist).

Der zweite Parameter ist optional und kann folgende Infos zur Heilung
oder zum Schaden beinhalten: (Die Eintraege sind Defines aus <add_hp.h>)

Name            Typ             Bedeutung
--------------- --------------- -----------------------------------------------
AH_ATTACKER     object          Im Kampf: Der Angreifer.
                                Ist dieser Eintrag angegeben, so zaehlt der
                                Schaden zu einer Kampfhandlung zugehoerig.
                                Es wird dann die Ruestung des Opfers und
                                sein Verteidungsmodus beruecksichtigt,
                                Kampfmeldungen werden ausgegeben.

AH_WEAPON       object/mapping  Im Kampf: Die benutzte Waffe (kann auch
                                eine natural#weapon sein)

AH_PROJECTILE   object          Im Kampf: Das Geschoss (sofern vorhanden)

AH_CAUSE        object          Aufrufer von add_hp, also z.B. der Virus,
                                das Getraenk oder die Nahrung. (wird z.B.
                                fuer die Generation der Todesmeldung genutzt.)
                                Bei Nichtangabe ist es:
                                previous_object(caller_stack_depth()-1)

AH_ORIGINATOR   string          Der real_cap_name, welcher den verursachende
                                Spieler angibt (fuer die Racheregelung und
                                M-Austeilung).

AH_FLAGS        int             Eine Oder-Kombination von Flags. Eine
                                Liste der moeglichen Flags gibt es
                                weiter unten.
   
AH_MESSAGE      string,         Die Meldung fuer das Opfer und alle anderen.
                string*,        Details dazu siehe weiter unten.
                closure

AH_ERF_TOD      string          Die erf-tod-Meldung fuer den Gestorbenen.

AH_ERF_TOD_OTHER string         Die erf-tod-Meldung fuer den Stein in der
                                Kathedrale (wo die automatische Uebersetzung
                                in die 3. Person nicht klappt.)
                                Sie sollte den real_cap_name des Opfers
                                enthalten.

AH_ERF_RETTUNG  string          Die Meldung fuer Tafel bei Lyrion. Sie
                                sollte den real_cap_name des Opfers
                                enthalten.

AH_HEAL_TYPE    int             Gibt die Art der Heilung an. Folgende
                                Moeglichkeiten gibt es:

                                AH_HEAL_NORMAL  Normale Heilung
                                AH_HEAL_MAGIC   Magische Heilung
                                AH_HEAL_MEDIC   Irdische Heilung mit
                                                Verbaende, Pflaster,
                                                Salben etc.

AH_DAMAGE_TYPE  string*         Eine Liste von Schadensarten, denen dieser
                                Schaden zugerechnet werden kann. Folgende
                                stehen zur Auswahl:
                                
                                stich, schnitt, stumpf,
                                heilig, daemonisch,
                                tod, leben,
                                feuer, wasser, erde, luft
                                magie,
                                anstrengung,
                                kaelte, waerme,
                                explosion, saeure, elektrizitaet, laerm
                                
                                Fuer die hier genannten Typen gibt es
                                Defines der Form AHD_TOD, AHD_LEBEN usw..

AH_MAPI         mapping         Bei einem MAPIL-Zauberspruch kann hier das
                                Mapping mit den erweiterten Daten uebergeben
                                werden (letzter Parameter bei mapil_forbidden
                                und mapil_notify).

AH_COMBAT_LOG   mapping         Dieses Mapping enthaelt die Informationen
                                ueber die Beanspruchung der Ruestungen.
                                Es wird von add_hp generiert und ist
                                daher nur fuer die Meldungsfunktionen
                                interessant. Jeder Eintrag (wobei das
                                Ruestungsobjekt als Schluessel fungiert)
                                enthaelt als ersten Wert die absolute
                                Schutzwirkung und als zweiten Wert die
                                relative Schutzwerkung im Verhaeltnis zum
                                Gesamtschaden in Promille.
                                Ein zusaetzlicher Eintrag mit dem
                                Angreiferobjekt enthaelt den Gesamtschaden.


Folgende Flags kann man bei AH_FLAGS angeben:
---------------------------------------------

    AH_NO_ARMOUR
        Die Ruestung des Opfers wird nicht beachtet. (Sie wird normalerweise
        beachtet, wenn ein Angreifer angegeben wurde.)
   
    AH_ARMOUR
        Die Ruestung des Opfers wird beachtet. (Sie wird normalerweise nicht
        beachtet, wenn kein Angreifer angegeben wurde.)

    AH_NO_MESSAGE
        Es wird keine Meldung ausgegeben. (Eine Kampfmeldung wird
        normalerweise ausgegeben, wenn ein Angreifer angegeben wurde.)
 
    AH_CRITICAL
        Dieser Schlag ist ein kritischer Treffer. (Dies wird nur fuer
        die Meldungsgeneration verwendet.)

    AH_NO_AGGRESSION
        Dieser AP-Abzug wird bei der Kampf-/Racheregelung nicht beachtet.
        Dieses Flag ist nur nach Absprache mit den Admins zu verwenden.

    AH_NO_GUARDIAN_ANGEL
        Falls der Spieler mit diesem Abzug stirbt, wird ihm kein Schutzengel
        der Welt helfen koennen. Dieses Flag ist privilegiert und seine
        Benutzung daher bei den Admins zu beantragen.    

    AH_DONT_DIE
        Das Lebewesen kommt damit nicht unter 0 APs. Dieses Flag ist
        privilegiert und seine Benutzung daher bei den Admins zu beantragen.

    AH_DIE
        Das Lebewesen kommt damit garantiert unter 0 APs. Dieses Flag
        impliziert AH_NO_ARMOUR und hebt AH_ARMOUR auf. Zusammen mit
        AH_DONT_DIE bewirkt es, dass das Lebewesen auf genau 0 APs kommt.

    AH_NO_SKILL
        Wenn das Opfer getoetet wird bekommt der Angreifer keine Skillpunkte
        auf skills,getoetet,kleingetier/grosswild. Gedacht ist es fuer sehr
        einfache Kills, die praktisch ohne Kampf verlaufen. Wenn man z.B.
        innerhalb einer Quest eine Saeureglas auf ein Steinmonster wirft und 
        es dadurch auf der Stelle getoetet wird, dann sollte dies keine 
        Kampfskillpunkte geben.

Meldungen
---------
Als Meldung kann bei AH_MESSAGE ein einfacher String, ein Array aus Strings
oder eine Closure angegeben werden. Wurde nur ein String angegeben,
so wird dieser nur an das Opfer ausgegeben.

Ein Array aus Strings enthaelt (in dieser Reihenfolge) die Meldungen
fuer das Opfer, alle anderen im Raum und - sofern es sich um einen
Kampf handelt - die Meldung fuer den Angreifer.

Wurde eine Closure angegeben, so wird sie mit dem Lebewesen (dem Opfer)
als erstes Argument, 'hps' als zweites und dem info-Mapping als drittes
Argument aufgerufen. Es sollte dann einen String oder String-Array
zurueckliefern.

Die Meldungen werden automatisch umgebrochen. Wenn eine Meldung 0 ist
(oder fehlt, weil nur ein String, also nur eine Meldung fuer das Opfer,
angegeben wurde) so wird die Standardmeldung genommen (dies ist im Kampf
die normale Kampfmeldung, ansonsten nix). Wenn Meldungen angegeben wurden,
wird das Flag AH_NO_MESSAGE wirkungslos.


Rueckgabewert
-------------
Ein negativer Rueckgabewert zeigt, dass derjenige bereits tot ist oder
damit gestorben ist. Ein positiver Rueckgabewert bedeutet, dass derjenige
die Heilung oder den Punktabzug lebend ueberstanden hat.
Eine 0 wird nur geliefert, wenn eine 0 als 1. Parameter uebergeben wurde.

VERWEISE: get_combat_message, give_hp, set_hp, query_hp, query_max_hp
GRUPPEN: monster, spieler, kampf
*/
private void add_hp_mesg(int hps, mapping infos, int msg_action)
{
    object enemy;
    mixed msgs;
    int i;
    int msg_type = MT_LOOK;

    if(!sizeof(infos))
        return;

    msgs = infos[AH_MESSAGE];
    if(closurep(msgs))
        msgs = funcall(msgs, this_object(), hps, infos);
    if(stringp(msgs))
        msgs = ({msgs, 0, 0});
    else if(!msgs)
        msgs = ({0}) * AH_MESG_SIZE;
    else if(!pointerp(msgs))
        return;

    if(sizeof(msgs)<AH_MESG_SIZE)
        msgs += ({0})*(AH_MESG_SIZE-sizeof(msgs));

    if(objectp(infos[AH_ATTACKER]) && !(infos[AH_FLAGS] & AH_NO_MESSAGE) &&
        ((i=member(msgs,0))>=0 && i<AH_MESG_SIZE))
    {
        // Wir muessen Standardmeldungen generieren.
        string* cmsgs = get_combat_message(this_object(), hps, infos);

        msgs = copy(msgs);

        for(i=0;i<AH_MESG_SIZE;i++)
            if(!msgs[i])
                msgs[i] = cmsgs[i];
    }
    
    if(!sizeof(msgs-({0})))
        return;

    if(this_object()->query_invis() == V_INVIS) // Invis15
        msg_type |= MT_DEBUG;
    
#ifdef FILTER_MSG_BY_ATTRIBUTES
    this_object()->send_message_to(this_object(), msg_type, msg_action,
        strlen(msgs[AH_MESG_VICTIM])?wrap(msgs[AH_MESG_VICTIM]):"",
        ([MSG_AH_INFOS:infos,MSG_RECEIVER_WHOM:AH_VICTIM]));
    enemy = want_combat_message(infos[AH_ATTACKER]) && infos[AH_ATTACKER];

    this_object()->send_message(msg_type, msg_action,
        strlen(msgs[AH_MESG_OTHER])?wrap(msgs[AH_MESG_OTHER]):"",
        0, enemy,([MSG_AH_INFOS:infos,MSG_RECEIVER_WHOM:MSG_OTHERS]));
    
    if(enemy)
        this_object()->send_message_to(enemy, MT_NOTIFY|msg_type, msg_action,
            strlen(msgs[AH_MESG_ATTACKER])?wrap(msgs[AH_MESG_ATTACKER]):"",
            ([MSG_AH_INFOS:infos,MSG_RECEIVER_WHOM:AH_ATTACKER]));
#else
    if(want_combat_message(this_object()))
        this_object()->send_message_to(this_object(), msg_type, msg_action,
            strlen(msgs[AH_MESG_VICTIM])?wrap(msgs[AH_MESG_VICTIM]):"");

    enemy = want_combat_message(infos[AH_ATTACKER]) && infos[AH_ATTACKER];

    this_object()->send_message(msg_type, msg_action,
        strlen(msgs[AH_MESG_OTHER])?wrap(msgs[AH_MESG_OTHER]):"",
        0, enemy);
    
    if(enemy)
        this_object()->send_message_to(enemy, MT_NOTIFY|msg_type, msg_action,
            strlen(msgs[AH_MESG_ATTACKER])?wrap(msgs[AH_MESG_ATTACKER]):"");
#endif
}

varargs int add_hp(int ahps, mapping infos)
{
    object attacker;
    mapping dampercent;
    int was_aggressive;
    int flags;
    
    if(!intp(ahps))
        raise_error("Bad argument 1 to add_hp: not an integer\n");

    if(!ahps && !sizeof(infos))     // Abkuerzung...
        return 0;

    if(!infos)
        infos = ([]); // Macht vieles einfacher...


    // Heilung
    if(ahps > 0)
    {
        int mhp;

        if(hp < 0)
            return -1;
        if (hp >= (mhp = this_object()->query_max_hp()))
            return 1;

        infos[AH_HEALING] = ahps;
        this_object()->modify("healing", &infos);
        ahps = infos[AH_HEALING];
        m_delete(infos, AH_HEALING);

        hp += ahps;
        if (hp > mhp)
            hp = mhp;

        add_hp_mesg(ahps, infos, MA_UNKNOWN);

        this_object()->notify("healing", infos + ([AH_HEALING: ahps])); 
                // Implizite Kopie des Mappings

        if (extended_hp_view)
            notify_message(wrap("Du hast "+hp+" Ausdauerpunkte."));
        else if (hp_view & HP_SP_VIEW_HP_PLUS && 
            find_call_out ("hp_sp_view") == -1)
                call_out ("hp_sp_view",2);
    

        this_object()->update_points_display();
        return 1;
    }


    flags = infos[AH_FLAGS];    // Brauchen wir oefters

    // Erstmal testen wir Privilegierungen.
    if(flags & (AH_NO_GUARDIAN_ANGEL|AH_DONT_DIE))
    {
        object po;
    
        foreach(po:caller_stack())
        {
            object sh = po;
            while(sh && sh!=this_object())
                sh=query_shadowing(sh);

            if(!sh) // po ist kein Shadow von uns
                break;
        }

        if((flags&AH_NO_GUARDIAN_ANGEL) &&
            !MASTER_OB->mudlib_privilege_violation("ah_no_guardian_angel",
                    po, ahps, infos))
            flags &= ~AH_NO_GUARDIAN_ANGEL;
        if((flags&AH_DONT_DIE) &&
                !MASTER_OB->mudlib_privilege_violation("ah_dont_die",
                po, ahps, infos))
            flags &= ~AH_DONT_DIE;
    }

    if(!ahps && !infos[AH_ATTACKER] && !(flags&AH_DIE))
    {
        add_hp_mesg(ahps, infos, infos[AH_DAMAGE_TYPE]?MA_FIGHT:MA_UNKNOWN);
        return 0;
    }

    infos[AH_DAMAGE] = -ahps;
    this_object()->modify("damage", &infos);
    ahps = -infos[AH_DAMAGE];
    m_delete(infos, AH_DAMAGE);
    
    flags = (infos[AH_FLAGS] & ~(AH_DONT_DIE|AH_NO_GUARDIAN_ANGEL))
          | (flags & (AH_DONT_DIE|AH_NO_GUARDIAN_ANGEL));
    infos[AH_FLAGS] = flags;
    
    attacker = infos[AH_ATTACKER];

    if(flags&AH_DIE)
        {} /* Nix machen */
    else if(ahps>0)
        return 0;
    else if(!ahps && !attacker)
        return 0;
	
    // So, dann mal die damage_perdingsda beachten
    dampercent = this_object()->query_damage_percentages();
    if(sizeof(dampercent))
    {
        float fahps = to_float(ahps); // Hier gibt's zu leicht Ueberlaeufe

        foreach(string typ: infos[AH_DAMAGE_TYPE] || ({}))
        {
            if(member(dampercent, typ))
                fahps = fahps * dampercent[typ] / 100;
            else if(member(dampercent, "default"))
                fahps = fahps * dampercent["default"] / 100;
        }

        fahps -= 0.5; // Wir runden.
        if(fahps <= __INT_MIN__)
            ahps = __INT_MIN__;
        else
            ahps = to_int(fahps);
    }
    
    if(flags&AH_DIE)
    {
        ahps = min(ahps, -hp-1);
        flags = (flags&~AH_ARMOUR)|AH_NO_ARMOUR;
    }
    
    if(ahps>0)
        return 0;
    else if(!ahps && !attacker)
        return 0;

    if ((ahps < 0) && (hp_view & HP_SP_VIEW_HP_MINUS) &&
            find_call_out ("hp_sp_view") == -1)
        call_out ("hp_sp_view",2);

    if(attacker)
    {
        if(!objectp(attacker) || !function_exists("add_hp", attacker))
        {
            object po = infos[AH_CAUSE];
            if(!objectp(po))
                po = previous_object();

            do_warning2(
        sprintf("add_hp mit ungültigem AH_ATTACKER-Eintrag aufgerufen:\n%O\n",
                    attacker), __FILE__,object_name(po),__LINE__);
            attacker = 0;
        }
        else
            was_aggressive = is_aggressive_against(attacker);
  
        if(attacker && !infos[AH_WEAPON]) // Vervollstaendigen
        {
            int handpos = member(attacker->query_hand_enemies(),
                            this_object())+1;

            if(handpos)
                infos[AH_WEAPON] = attacker->query_natural_weapon(handpos);
        }
    }


    // Aggression checken, damit bei einem ob->add_hp(-1,OBJ(who))
    // auch die aggression richtig behandelt wird.
    if (!(flags&AH_NO_AGGRESSION))
    {
        object agg = attacker || this_player() ||
            (living(infos[AH_CAUSE]) && infos[AH_CAUSE]) ||
            (living(previous_object()) && previous_object());

        this_object()->become_aggression_victim(agg);
        if(objectp(agg))
            agg->check_aggression(this_object());
    }
    
    if(playerp(attacker) && !playerp(this_object()) &&
        !IS_INVIS(attacker) && !this_object()->query_aggressive())
            this_object()->add_attack_list(attacker->query_real_name());


    // Schaden/Schutz und Promille am Gesamtschaden
    infos[AH_COMBAT_LOG] = ([attacker: -ahps; 1000]);

    // Ruestungen
    if((attacker && !(flags & AH_NO_ARMOUR) || 
       !attacker && (flags & AH_ARMOUR)) && ahps < 0)
    {
    
        mapping combat_log = infos[AH_COMBAT_LOG];
        int alevel;
        int sum_damage = -ahps;

        // Alle im living eingetragenen Ruestungen durchlaufen
        foreach(object armour: armours -= ({ 0 }))
            // Ist Ruestung noch im Player?
            if(environment(armour) != this_object())
                armours -= ({armour});
            // Ist Ruestung noch intakt?
            else if(!armour->query_broken())
            {
                // Schutz bestimmen
                int act_safe = armour->hit_action(attacker, ahps, infos);
                int max_safe = min(act_safe, -ahps);
                m_add(combat_log, armour, max_safe,
                sum_damage && (1000 * max_safe / sum_damage));

                // Ruestung demolieren (Kurdel: verschoben hinter combat_log,
                // denn manche Ruestungen zerstoeren sich beim Kaputtgehen)
                if(playerp(this_object()))
                    armour->add_life(-act_safe);

                // Schaden reduzieren
                if((ahps += act_safe) >= 0)
                {
                    ahps = 0;
                    break;
                }
            }
    
        // Jetzt noch die Ruestungsstaerke des Monsters selbst beruecksichtigen
        if((alevel = this_object()->query_armour_level()) > 0)
        {
            int act_safe = min(random(alevel + 1), -ahps);

            m_add(combat_log, this_object(), act_safe,
                sum_damage && (1000 * act_safe / sum_damage));
            ahps += act_safe;
        }
    }
    // Ende Ruestungen


    // Meldungen ausgeben.
    add_hp_mesg(ahps, infos, MA_FIGHT);

    this_object()->notify("damage", infos + ([ AH_DAMAGE: -ahps ])); 
                    // Implizite Kopie des Mappings

   // Schon tot?
    if(hp < 0)
        return -1;

    // Spielerwerte aendern
    if(interactive() && query_idle(this_object())<300 &&
        this_player()!=this_object())
            sum_hp += min(hp, -ahps);
    
    hp += ahps;
    if(flags&AH_DONT_DIE)
        hp = max(hp,0);
    
    if (extended_hp_view)
        notify_message(wrap("Du hast jetzt " + hp + " Ausdauerpunkte."));

    this_object()->update_points_display();
    
    if(ahps < 0 && (hp_view & HP_SP_VIEW_HP_MINUS))
        lost_hp = ahps;

    /* Jetzt wird gestorben? */
    if(hp < 0)
    {
        // Ein Gott
        if(query_wiz_level())
        {
            notify_message(
            "Was für ein Glück, dass du als Gott nicht sterben kannst!\n" +
            wrap("("+this_object()->determine_erf_tod_message(infos)+")"));

            hp = 0;
        }
        // Ein Engel mit Unsterblichkeitsgabe
        else if(!(flags&AH_NO_GUARDIAN_ANGEL)
              && GABE(this_object(), "us") && !this_object()->query_moerder()
              && (random(100) < probability_for_guardian_angel[1]))
            // 1 Prozent Wahrscheinlichkeit, dass Unsterblichkeit versagt
        {
            this_object()->notify("schutzengel", infos);

            call_out ("schutzengelstatistik",2,
                this_object()->determine_erf_tod_message(infos, AH_ERF_RETTUNG),
                this_object()->determine_erf_tod_message(infos, AH_ERF_TOD));

            this_object()->send_message(MT_LOOK,MA_MOVE_IN,
                "Der Tod erscheint.\n");

            this_object()->send_message(MT_NOISE,MA_COMM,
                wrap("Tod spricht zu "+dem(0,"")+": "
                "DEIN LETZES STÜNDCHEN HÄTTE GESCHLAGEN, "
                "WENN DU NICHT UNSTERBLICH WÄREST, ENGEL!"),
                wrap(
                "Tod spricht zu Dir: DEIN LETZES STÜNDCHEN HÄTTE GESCHLAGEN, "
                "WENN DU NICHT UNSTERBLICH WÄREST, ENGEL!"),
                this_object());

            this_object()->send_message(MT_NOISE|MT_LOOK,MA_MOVE_OUT,
              "Der Tod verschwindet mit einem verachtenden Zähneklappern.\n");

            this_object()->send_message(MT_NOISE|MT_LOOK,MA_MOVE_IN,
                wrap(Ihr((["name":"schutzengel", "gender":"maennlich"]), 0,
                this_object(),"")+
                " fliegt herbei und landet neben "+ihm(this_object())+"."),
                "Dein Schutzengel fliegt herbei und landet neben Dir.\n",
                this_object());

            this_object()->send_message(MT_NOISE,MA_COMM,
                wrap_say("Er spricht zu "+dem(this_object(),"")+": ",
                    "Was machst Du nur für Sachen, "+
                    this_object()->query_cap_name()+"? Willst Du Dich wirklich "
                    "mit dem Tod anlegen?"),
                wrap_say("Er sagt: ",
                    "Was machst Du nur für Sachen, "+
                    this_object()->query_cap_name()+"? Willst Du Dich wirklich "
                    "mit dem Tod anlegen?"),
                this_object());

            this_object()->send_message_to(this_object(),MT_LOOK,MA_UNKNOWN,
                "Dein Schutzengel bringt Dich in Sicherheit.\n"
                "Deine Gegenstände allerdings nicht.\n");

            filter(all_inventory(),(:!$1->query_auto_load():))->move(environment());

            this_object()->send_message(MT_LOOK, MA_MOVE_OUT, 
                wrap(Der()+plural(" wird"," werden")+" von "+
                seinem((["name":"schutzengel","gender":"maennlich"]),0,
                    this_object())+" in Sicherheit gebracht."));

            if(this_object()->move(testplayerp(this_object())?
                "/room/rathaus/forum":"/room/hlp/wolke",
                ([MOVE_FLAGS:MOVE_MAGIC|MOVE_FORCE]))==MOVE_OK)
                this_object()->send_message(MT_LOOK, MA_MOVE_IN,
                    wrap(Ein()+plural(" wird", " werden")+" von "+
                    seinem((["name":"schutzengel", "gender":"maennlich"]),0,
                        this_object())+" hier abgesetzt."));

            this_object()->set_hp(1);
            probability_for_guardian_angel=({1,99});

            return -1;
        }
        // Schutzengel rettet Spieler in 1 von 100 Faellen
        else if(!(flags&AH_NO_GUARDIAN_ANGEL) && playerp(this_object()) &&
            random(100) < probability_for_guardian_angel[0])
        {
            this_object()->notify("schutzengel", infos);

            call_out ("schutzengelstatistik",2,
            this_object()->determine_erf_tod_message(infos, AH_ERF_RETTUNG),
            this_object()->determine_erf_tod_message(infos, AH_ERF_TOD));

            this_object()->send_message(MT_LOOK|MT_NOISE,MA_MOVE_IN,
                wrap(Ihr((["name":"schutzengel", "gender":"maennlich"]), 0, 
                this_object(),"")+
                " fliegt herbei und landet neben "+ihm(this_object())+"."),
                wrap("Dein Schutzengel fliegt herbei und landet neben Dir."),
                this_object());

            this_object()->send_message(MT_NOISE,MA_COMM,
                wrap_say("Er spricht zu "+dem(this_object(),"")+":",
                    "Was machst Du nur wieder, "+
                    this_object()->query_cap_name()+"? Wenn ich nicht aufgepasst "
                    "hätte wärst Du nun tot!"),
                wrap_say("Er sagt:",
                    "Was machst Du nur wieder, "+                  
                    this_object()->query_cap_name()+"? Wenn ich nicht aufgepasst "                  
                    "hätte wärst Du nun tot!"),
                this_object());

            this_object()->send_message_to(this_object(),MT_LOOK,MA_UNKNOWN,
                "Dein Schutzengel bringt Dich in Sicherheit.\n");

            this_object()->send_message(MT_LOOK, MA_MOVE_OUT, 
                wrap(Der()+plural(" wird"," werden")+" von "+
                seinem((["name":"schutzengel","gender":"maennlich"]),0,
                this_object())+" in Sicherheit gebracht."));

            if(this_object()->move(testplayerp(this_object())?
                "/room/rathaus/forum":"/room/hlp/wolke",
                ([MOVE_FLAGS:MOVE_MAGIC|MOVE_FORCE]) )==MOVE_OK)
                this_object()->send_message(MT_LOOK, MA_MOVE_IN,
                    wrap(Ein()+plural(" wird", " werden")+" von "+
                    seinem((["name":"schutzengel", "gender":"maennlich"]),0,
                    this_object())+" hier abgesetzt."));

            this_object()->set_hp(1);
                probability_for_guardian_angel=({1,99});

            return -1;
        }
        // Ein normaler (und ziemlich sterblicher) Spieler
        else
        {
            this_object()->notify("death", infos);
            this_object()->die(infos);

            // Nur Loeschen, wenn es angewandt wurde...
            if(!(flags&AH_NO_GUARDIAN_ANGEL))
                probability_for_guardian_angel=({1,99});

            return -1;
        }
    }

    /* Flucht || Verteidigung */
    if(attacker) // ab hier tmp reused
    {
        int hand;
        int wh = this_object()->query_whimpy();

        if(wh && hp < wh && this_object()->runaway() || !this_object())
            ; // Nix machen, wenn die FLucht erfolgreich war.
        else if((was_aggressive || 
                (this_object()->query_reattack() &&
                (this_object()->query_reattack()==REATTACK_ALWAYS ||
                !playerp(this_object()) || !playerp(attacker) ||
                !(flags&AH_NO_AGGRESSION)))) &&
                member(hand_enemies, attacker) <= -1 &&
                (hand = free_fight()) > -1 &&
                attacker != this_object())
        {
            hand_enemies[hand] = attacker;
            hand_hits[hand] = 0;
            set_heart_beat(1);
   
            if(was_aggressive)
            {
                if(hand)
                    current_hand = hand-1;
                else
                    current_hand = num_hands-1;
    
                call_with_this_player(#'handle_attack);
                dont_hit_time=time();
            }
        }
        else if (was_aggressive &&
                (hand = member(hand_enemies, attacker)) >= 0 &&
                hand_hits[hand] == 0 &&
                attacker != this_object())
        {
                if(hand)
                    current_hand = hand-1;
                else
                    current_hand = num_hands-1;
    
                call_with_this_player(#'handle_attack);
                dont_hit_time=time();            
        }
    }

    return -ahps;
}
/*
FUNKTION: modify_damage
DEKLARATION: void modify_damage(mapping infos, object opfer)
BESCHREIBUNG:
Bevor add_hp einen Schaden auf das Lebewesen opfer anwendet, wird
in allen mit opfer->add_controller("modify_damage", other) angemeldeten
Objekten other die Funktion modify_damage aufgerufen.

Das uebergebene Mapping infos enthaelt neben allen Eintraegen, die add_hp
uebergeben wurden, zusaetzlich den Eintrag AH_DAMAGE, welcher die Anzahl
abzuziehender APs (als positive Zahl) angibt. Alle Eintraege koennen
geaendert, weitere Eintraege auch hinzugefuegt werden.
VERWEISE: modify, modify_healing, notify_damage, add_hp, add_controller
GRUPPEN: monster, spieler, kampf
*/
/*
FUNKTION: modify_healing
DEKLARATION: void modify_healing(mapping infos, object lebewesen)
BESCHREIBUNG:
Bevor mittels add_hp das Lebewesen geheilt wird, wird in allen
mit opfer->add_controller("modify_healing", other) angemeldeten
Objekten other die Funktion modify_healing aufgerufen.

Das uebergebene Mapping infos enthaelt neben allen Eintraegen, die add_hp
uebergeben wurden, zusaetzlich den Eintrag AH_HEALING, welcher die Anzahl
hinzuzufuegender APs angibt. Alle Eintraege koennen geaendert, weitere
Eintraege auch hinzugefuegt werden.
VERWEISE: modify, modify_damage, notify_healing, add_hp, add_controller
GRUPPEN: monster, spieler, kampf
*/
/*
FUNKTION: notify_damage
DEKLARATION: void notify_damage(mapping infos, object opfer)
BESCHREIBUNG:
Kurz bevor add_hp einen Schaden auf das Lebewesen opfer anwendet (und das
Opfer damit evntl. stirbt), wird in allen mit opfer->add_controller(
"notify_damage", other) angemeldeten Objekten other die Funktion
notify_damage aufgerufen.

Das uebergebene Mapping infos enthaelt neben allen Eintraegen, die add_hp
uebergeben wurden, zusaetzlich den Eintrag AH_DAMAGE, welcher die Anzahl
abzuziehender APs (als positive Zahl) angibt.
VERWEISE: notify, notify_healing, modify_damage, add_hp, add_controller
GRUPPEN: monster, spieler, kampf
*/
/*
FUNKTION: notify_healing
DEKLARATION: void notify_healing(mapping infos, object lebewesen)
BESCHREIBUNG:
Nachdem mittels add_hp das Lebewesen geheilt wurde, wird in allen
mit opfer->add_controller("notify_healing", other) angemeldeten
Objekten other die Funktion notify_healing aufgerufen.

Das uebergebene Mapping infos enthaelt neben allen Eintraegen, die add_hp
uebergeben wurden, zusaetzlich den Eintrag AH_HEALING, welcher die Anzahl
hinzuzufuegender APs angibt.
VERWEISE: notify, notify_damage, modify_healing, add_hp, add_controller
GRUPPEN: monster, spieler, kampf
*/

/* ===============  B E S C H U E T Z E N  =================== */

/*
FUNKTION: query_protectee
DEKLARATION: object query_protectee()
BESCHREIBUNG:
Diese Funktion liefert das Lebewesen zurueck, welches beschuetzt wird,
selbst dann, wenn das beschuetzte Lebewesen gerade nicht im gleichen
Raum ist, damit der Schutz praktisch nicht existent ist.
0 wird zurueckgegeben, wenn kein Lebewesen gerade beschuetzt wird.
VERWEISE: forbidden_protect, notify_protect, notify_unprotect
GRUPPEN: monster, spieler, kampf
*/
object query_protectee()
{
    return protectee;
}

private void remove_protection();
private void protectee_modify_hit_me(string controller, mapping data,
            object attacker)
{
    if(data[AH_VICTIM]!=protectee)
    {
        // Hmm, vergessen abzumelden?
        if(data[AH_VICTIM])
            data[AH_VICTIM]->delete_controller("modify_hit_me", 
                                    #'protectee_modify_hit_me);
        return;
    }
    
    // Nur dann, wenn wir anwesend sind und auch nicht der Angreifer
    // sind.
    if(this_object()->query_ghost() || this_object()->query_hp()<0 ||
            attacker == this_object() || !present(protectee))
        return;

    // Ketten nicht vollstaendig durchlaufen, sondern
    // eins vorm Umlauf beenden.
    if(data["protector_original_victim"] == this_object())
        return;

    data["protector_original_victim"] ||= data[AH_VICTIM];
    
    caught_attackers ||= ({});
    if(protected_ids && member(caught_attackers, attacker)<0)
    {
        int found;

        foreach(string str: protected_ids)
            if(attacker->me(str))
            {
                found = 1;
                break;
            }

        if(!found)
            return;

        caught_attackers = caught_attackers - ({0}) + ({attacker});
    }
    
    data[AH_VICTIM] = this_object();
    data[AH_FLAGS] |= AH_NO_AGGRESSION;
    // Damit dieser Schlag kein (M) bewirkt.
    data[AH_ORIGINATOR] = playerp(this_object()) && 
        this_object()->query_real_cap_name();
}

private void protectee_after_hit(string controller, object attacker,
    object victim, int damage, int critical, int hand, object weapon)
{
    if(attacker!=protectee)
    {
        if(attacker)
            attacker->delete_controller("notify_after_hit", 
            #'protectee_after_hit);
        return;
    }
    
    if(protected_ids && member(caught_attackers||({}), attacker)<0)
    {
        int found;

        foreach(string str: protected_ids)
            if(attacker->me(str))
            {
                found = 1;
                break;
            }

        if(!found)
            return;
    }

    this_object()->send_message_to(this_object(), MT_NOTIFY, MA_UNKNOWN,
        wrap("Du beschützt "+den(protectee)+" nicht mehr."));
    
    remove_protection();
}

private void protectee_notify_attack(string controller, object attacker,
    object weapon, object enemy)
{
    if(controller == "notify_protector_attack")
    {
        if(previous_object() != protectee)
        {
            previous_object()->delete_controller(({"notify_attack",
                "notify_protector_attack"}), #'protectee_notify_attack);
            return;
        }   

        // Bei einer Schleife.
        if(enemy == this_object())
            return;
    }
    else if(enemy != protectee)
    {
        if(enemy)
            enemy->delete_controller(({"notify_attack",
                "notify_protector_attack"}), #'protectee_notify_attack);
        return;
    }
    
    // Nur dann, wenn wir anwesend sind und auch nicht der Angreifer
    // sind.
    if(this_object()->query_ghost() || this_object()->query_hp()<0 ||
            attacker == this_object() || !present(protectee))
        return;
    
#ifdef FILTER_MSG_BY_ATTRIBUTES
    this_object()->send_message(MT_LOOK, MA_FIGHT,
        wrap(Der()+" stellt sich "+ihm(attacker)+" in den Weg."),
        wrap(Der()+" stellt sich dir in den Weg."),
        attacker,
                ([ AH_ATTACKER: attacker, AH_VICTIM: enemy,
                   MSG_FIRST_MSG:1,MSG_RECEIVER_WHOM: AH_VICTIM]));

    this_object()->send_message_to(this_object(),
        MT_LOOK|MT_NOTIFY, MA_FIGHT,
        wrap("Du stellst dich "+ihm(attacker)+" in den Weg."),
                ([ AH_ATTACKER: attacker, AH_VICTIM: enemy,
                   MSG_RECEIVER_WHOM: AH_ATTACKER,MSG_FIRST_MSG:1 ]));
#else
    this_object()->send_message(MT_LOOK, MA_FIGHT,
        wrap(Der()+" stellt sich "+ihm(attacker)+" in den Weg."),
        wrap(Der()+" stellt sich dir in den Weg."),
        attacker);

    this_object()->send_message_to(this_object(),
        MT_LOOK|MT_NOTIFY, MA_FIGHT,
        wrap("Du stellst dich "+ihm(attacker)+" in den Weg."));
#endif
    this_object()->notify("protector_attack", attacker, weapon, enemy);
}

private void remove_protection()
{
    if(protectee)
    {
        protectee->delete_controller("modify_hit_me", #'protectee_modify_hit_me);
        protectee->delete_controller("notify_after_hit", #'protectee_after_hit);
        protectee->delete_controller(({"notify_attack",
            "notify_protector_attack"}), #'protectee_notify_attack);

        this_object()->do_notifies(0, "unprotect",
            ({"", "_me"}), ({ this_object(), protectee }), protected_ids);
    }
    
    protectee = 0;
    protected_ids = 0;
    caught_attackers = 0;
}

private int start_protection(object who, string* against)
{
    string reason;
    
    reason = this_object()->do_forbiddens(0, "protect",
            ({"", "_me"}), ({ this_object(), who }), against);
    if(reason)
    {
        if(stringp(reason))
            this_object()->send_message_to(this_object(),
                MT_NOTIFY, MA_UNKNOWN, wrap(reason));

        return 0;
    }
    
    if(!against)	// Gegen alle schuetzen
        protected_ids = 0;
    else if(!protected_ids)
        protected_ids = against;
    else
        protected_ids += against;

    if(!protectee)
    {
        protectee = who;

        protectee->add_controller("modify_hit_me", #'protectee_modify_hit_me);
        protectee->add_controller("notify_after_hit", #'protectee_after_hit);
        protectee->add_controller(({"notify_attack",
            "notify_protector_attack"}), #'protectee_notify_attack);
    }
    
    if(playerp(protectee))
        // Nur dann abbrechen, wenn der Kampf in beide Richtungen geht.
        foreach(object enemy: protectee->query_hand_enemies())
            if(living(enemy) && enemy->query_in_fight(protectee))
            {
                int has_id;

                if(!against)
                    has_id = 1;
                else 
                    foreach(string str: against)
                        if(enemy->me(str))
                        {
                            has_id = 1;
                            break;
                        }

                if(has_id)
                    protectee->delete_enemy(enemy);
            }
    
    this_object()->do_notifies(0, "protect",
        ({"", "_me"}), ({ this_object(), who }), against);

    return 1;
}

int protect_command(string str)
{
    mixed *parsed;
    mixed victim;
    
    str=space(str||"");
    
    if(str=="nicht" || str=="nicht mehr")
    {
        if(!protectee)
        {
            return notify_fail("Du beschützt derzeit niemanden.\n",
                FAIL_WRONG_ARG);
        }
        else
        {
            this_object()->send_message(MT_LOOK, MA_UNKNOWN,
                wrap(Der()+plural(" beschützt "," beschützen ")+
                    den(protectee)+" nicht mehr."),
                wrap(Der()+plural(" beschützt "," beschützen ")+
                    "dich nicht mehr."), protectee);
            this_object()->send_message_to(this_object(), 
                MT_LOOK|MT_NOTIFY, MA_UNKNOWN,
                wrap("Du beschützt "+den(protectee)+" nicht mehr."));
            remove_protection();
            return 1;
        }
    }
    
    parsed = parse_com(str, environment(), ({"vor", "nicht"}));
    if(parse_com_error(parsed, 
            "Schütze wen (vor wem) oder schütze wen nicht mehr?\n", 1))
        return 0;

    victim = parsed[PARSE_OBS][0];
    
    if(parsed[PARSE_TRENNER]=="nicht")
    {
        if(protectee!=victim)
            return notify_fail(wrap("Du beschützt " + den(victim) + 
                " derzeit nicht."), FAIL_WRONG_ARG);

        this_object()->send_message(MT_LOOK, MA_UNKNOWN,
            wrap(Der()+plural(" beschützt "," beschützen ")+
                den(protectee)+" nicht mehr."),
            wrap(Der()+plural(" beschützt "," beschützen ")+
                "dich nicht mehr."), protectee);
        this_object()->send_message_to(this_object(), 
            MT_LOOK|MT_NOTIFY, MA_UNKNOWN,
            wrap("Du beschützt "+den(protectee)+" nicht mehr."));
        remove_protection();
        return 1;
    }
    
    if(this_object()->query_ghost())
        return notify_fail(wrap("Als Geist fällt es Dir schwer, "
            "jemanden ernsthaft zu beschützen."), FAIL_INTERNAL);
    
    if(victim == this_object())
    {
        if(protectee)
        {
            this_object()->send_message(MT_LOOK, MA_UNKNOWN,
                wrap(Der()+plural(" beschützt "," beschützen ")+
                    den(protectee)+" nicht mehr."),
                wrap(Der()+plural(" beschützt "," beschützen ")+
                    "dich nicht mehr."), protectee);
            this_object()->send_message_to(this_object(), 
                MT_NOTIFY, MA_UNKNOWN,
                wrap("Du beschützt "+den(protectee)+" nicht mehr."));
            remove_protection();
            return 1;
        }

        return notify_fail(wrap(
            ({
                "Dich selbst kannst Du nicht beschützen.",
                "Vielleicht schützt Du lieber andere?",
                "Dich zu schützen ist doch eh zwecklos.",
                "Mutig stellst Du Dich vor Dich.",
            })[random(4)]), FAIL_WRONG_ARG);
    }
    
    if(protectee && protectee != victim)
        return notify_fail(wrap("Du beschützt noch " + den(protectee) + "."),
            FAIL_INTERNAL);

    if(!objectp(victim) || !living(victim))
        return notify_fail(wrap(Der(victim) + " benötigt keinen Schutz."),
            FAIL_WRONG_ARG);

    // Es ist zu maechtig, wenn ein NPC einen Spieler beschuetzt.
    if(playerp(victim) && !playerp(this_object()))
        return notify_fail(wrap("Du kannst keinen Spieler beschützen."),
            FAIL_NOT_OBJ);
    
    if(!protectee)
        remove_protection();

    if(parsed[PARSE_TRENNER]=="vor")
    {
        string* ids = regexplode(parsed[PARSE_BEYOND_TRENNER]," und |,")
            -({" und ",","});

        ids = map(ids,
            (: 
                string * words = explode($1, " ")-({""});
                words[0..<2] = map(words[0..<2], #'lower_case);
                if(lower_case(words[<1])==words[<1])
                    words[<1]=capitalize(words[<1]);
                return implode(words, " ");
            :));

        if(start_protection(victim, ids))
        {
            this_object()->send_message(MT_LOOK, MA_UNKNOWN, // MA_CRAFT? ;-)
                wrap(Der()+plural(" beschützt "," beschützen ")+"jetzt "+
                    den(victim)+" vor "+liste(protected_ids) + "."),
                wrap(Der()+plural(" beschützt "," beschützen ")+
                    "jetzt Dich vor "+liste(protected_ids) + "."), victim);
            this_object()->send_message_to(this_object(),
                MT_LOOK|MT_NOTIFY, MA_UNKNOWN,
                wrap("Du beschützt jetzt " + den(victim) + " vor " + 
                    liste(protected_ids) + "."));
        }
    }
    else
    {
        if(start_protection(victim, 0))
        {
            this_object()->send_message(MT_LOOK, MA_UNKNOWN,
                wrap(Der()+plural(" beschützt "," beschützen ")+"jetzt "+
                    den(victim)+"."),
                wrap(Der()+plural(" beschützt "," beschützen ")+
                    "jetzt Dich."), victim);
            this_object()->send_message_to(this_object(),
                MT_NOTIFY, MA_UNKNOWN,
                wrap("Du beschützt jetzt " + den(victim) + "."));
        }
    }
    return 1;
}
/*
FUNKTION: forbidden_protect
DEKLARATION: string forbidden_protect(object protector, object protectee, string* ids)
BESCHREIBUNG:
Wenn das Lebewesen protector das Lebewesen protectee beschuetzen will,
so wird in allen mit protector->add_controller("forbidden_protect", other)
angemeldeten Objekten other die Funktion forbidden_protect aufgerufen.
Liefert einer dieser Funktionen einen Wert!=0 zurueck, so wird das Beschuetzen
verboten. Der zurueckgelieferte Wert sollte ein String sein, welcher spaeter
automatisch umgebrochen und dem Lebewesen als Fehlermeldung mitgeteilt wird.

ids enthaelt eine Liste aller IDs, vor denen beschuetzt wird.
Ist es 0, so wird vor allem geschuetzt.
VERWEISE: add_controller, notify_protect, notify_unprotect, forbidden,
          forbidden_protect_me, forbidden_protect_here
GRUPPEN: monster, spieler, kampf
*/
/*
FUNKTION: forbidden_protect_me
DEKLARATION: string forbidden_protect_me(object protector, object protectee, string* ids)
BESCHREIBUNG:
Wenn das Lebewesen protector das Lebewesen protectee beschuetzen will,
so wird in allen mit protectee->add_controller("forbidden_protect_me", other)
angemeldeten Objekten other die Funktion forbidden_protect_me aufgerufen.
Liefert einer dieser Funktionen einen Wert!=0 zurueck, so wird das Beschuetzen
verboten. Der zurueckgelieferte Wert sollte ein String sein, welcher spaeter
automatisch umgebrochen und dem Lebewesen als Fehlermeldung mitgeteilt wird.

ids enthaelt eine Liste aller IDs, vor denen beschuetzt wird.
Ist es 0, so wird vor allem geschuetzt.
VERWEISE: add_controller, notify_protect_me, notify_unprotect_me, forbidden,
          forbidden_protect, forbidden_protect_here
GRUPPEN: monster, spieler, kampf
*/
/*
FUNKTION: forbidden_protect_here
DEKLARATION: string forbidden_protect_here(object protector, object protectee, string* ids)
BESCHREIBUNG:
Wenn das Lebewesen protector das Lebewesen protectee im Raum room beschuetzen
will, so wird in allen mit room->add_controller("forbidden_protect_here", other)
angemeldeten Objekten other die Funktion forbidden_protect_here aufgerufen.
Liefert einer dieser Funktionen einen Wert!=0 zurueck, so wird das Beschuetzen
verboten. Der zurueckgelieferte Wert sollte ein String sein, welcher spaeter
automatisch umgebrochen und dem Lebewesen als Fehlermeldung mitgeteilt wird.

ids enthaelt eine Liste aller IDs, vor denen beschuetzt wird.
Ist es 0, so wird vor allem geschuetzt.
VERWEISE: add_controller, notify_protect_here, notify_unprotect_here,
	  forbidden, forbidden_protect, forbidden_protect_me
GRUPPEN: monster, spieler, kampf
*/
/*
FUNKTION: notify_protect
DEKLARATION: void notify_protect(object protector, object protectee, string* ids)
BESCHREIBUNG:
Wenn das Lebewesen protector das Lebewesen protectee beschuetzen will,
so wird in allen mit protector->add_controller("notify_protect", other)
angemeldeten Objekten other die Funktion notify_protect aufgerufen.

ids enthaelt eine Liste aller IDs, vor denen beschuetzt wird.
Ist es 0, so wird vor allem geschuetzt.
VERWEISE: add_controller, forbidden_protect, notify_unprotect, notify,
	  notify_protect_me, notify_protect_here
GRUPPEN: monster, spieler, kampf
*/
/*
FUNKTION: notify_protect_me
DEKLARATION: void notify_protect_me(object protector, object protectee, string* ids)
BESCHREIBUNG:
Wenn das Lebewesen protector das Lebewesen protectee beschuetzen will,
so wird in allen mit protectee->add_controller("notify_protect_me", other)
angemeldeten Objekten other die Funktion notify_protect_me aufgerufen.

ids enthaelt eine Liste aller IDs, vor denen beschuetzt wird.
Ist es 0, so wird vor allem geschuetzt.
VERWEISE: add_controller, forbidden_protect_me, notify_unprotect_me, notify,
	  notify_protect, notify_protect_here
GRUPPEN: monster, spieler, kampf
*/
/*
FUNKTION: notify_protect_here
DEKLARATION: void notify_protect_here(object protector, object protectee, string* ids)
BESCHREIBUNG:
Wenn das Lebewesen protector das Lebewesen protectee im Raum room beschuetzen
will, so wird in allen mit room->add_controller("notify_protect_here", other)
angemeldeten Objekten other die Funktion notify_protect_here aufgerufen.

ids enthaelt eine Liste aller IDs, vor denen beschuetzt wird.
Ist es 0, so wird vor allem geschuetzt.
VERWEISE: add_controller, forbidden_protect_here, notify_unprotect_here,
	  notify, notify_protect, notify_protect_me
GRUPPEN: monster, spieler, kampf
*/
/*
FUNKTION: notify_unprotect
DEKLARATION: void notify_unprotect(object protector, object protectee, string* ids)
BESCHREIBUNG:
Wenn das Lebewesen protector das Lebewesen protectee nicht mehr beschuetzen
will, so wird in allen mit protector->add_controller("notify_unprotect", other)
angemeldeten Objekten other die Funktion notify_unprotect aufgerufen.

ids enthaelt eine Liste aller IDs, vor denen beschuetzt wurde.
Ist es 0, so wurde vor allem geschuetzt.
VERWEISE: add_controller, forbidden_protect, notify_protect, notify,
	  notify_unprotect_me, notify_unprotect_here
GRUPPEN: monster, spieler, kampf
*/
/*
FUNKTION: notify_unprotect_me
DEKLARATION: void notify_unprotect_me(object protector, object protectee, string* ids)
BESCHREIBUNG:
Wenn das Lebewesen protector das Lebewesen protectee nicht mehr beschuetzen
will, so wird in allen mit protectee->add_controller("notify_unprotect_me",
other) angemeldeten Objekten other die Funktion notify_unprotect_me aufgerufen.

ids enthaelt eine Liste aller IDs, vor denen beschuetzt wurde.
Ist es 0, so wurde vor allem geschuetzt.
VERWEISE: add_controller, forbidden_protect_me, notify_protect_me, notify,
	  notify_unprotect, notify_unprotect_here
GRUPPEN: monster, spieler, kampf
*/
/*
FUNKTION: notify_unprotect_here
DEKLARATION: void notify_unprotect_here(object protector, object protectee, string* ids)
BESCHREIBUNG:
Wenn das Lebewesen protector das Lebewesen protectee im Raum room nicht
mehr beschuetzen will, so wird in allen mit room->add_controller(
"notify_unprotect_here", other) angemeldeten Objekten other die Funktion
notify_unprotect_here aufgerufen.

ids enthaelt eine Liste aller IDs, vor denen beschuetzt wurde.
Ist es 0, so wurde vor allem geschuetzt.
VERWEISE: add_controller, forbidden_protect_here, notify_protect_here,
	  notify, notify_unprotect, notify_unprotect_me
GRUPPEN: monster, spieler, kampf
*/

/*
FUNKTION: notify_schutzengel
DEKLARATION: void notify_schutzengel(mapping infos, object lebewesen)
BESCHREIBUNG:
Unmittelbar, bevor ein Lebewesen (durch AP-Abzug) fast gestorben waere,
dann aber vom Schutzengel gerettet wird - es hat also noch nicht die
Rettung miterlebt - wird die Funktion
    notify("schutzengel", infos)
in ihm aufgerufen. infos ist dabei das Mapping, welches beim AP-Abzug
durch add_hp angegeben wurde.

Ist ein Objekt beim Gestorbenen als Controller fuer "notify_schutzengel" mit
    lebewesen->add_controller("notify_schutzengel", this_object())
angemeldet, so wird dann in diesem Objekt die Funktion

void notify_schutzengel(mapping infos, object lebewesen)
aufgerufen.

VERWEISE: notify_died, forbidden, notify, notify_death
GRUPPEN: spieler, monster
*/
/*
FUNKTION: notify_death
DEKLARATION: void notify_death(mapping infos, object lebewesen)
BESCHREIBUNG:
Unmittelbar, bevor ein Lebewesen (durch AP-Abzug) stirbt, - es ist also
gerade noch lebendig - wird die Funktion
    notify("death", infos)
in ihm aufgerufen. infos ist dabei das Mapping, welches beim AP-Abzug
durch add_hp angegeben wurde.

Ist ein Objekt beim Gestorbenen als Controller fuer "notify_death" mittels
    lebewesen->add_controller("notify_death", this_object())
angemeldet, so wird dann in diesem Objekt die Funktion

void notify_death(mapping infos, object lebewesen)

aufgerufen.
VERWEISE: notify_died, forbidden, notify, notify_schutzengel
GRUPPEN: spieler, monster
*/

/* =====================  W  E  A  R  ======================== */


// Kurdel: Diese Funktion sorgt fuer Anziehen, Ausziehen, Aufsetzen bzw.
// Absetzen von einem Kleidungsstueck oder mehreren.
int wear_command(string str)
{
    mixed *parsed, *obs, ob;
    string fun, *words, wear_prepos, remove_prepos, prepos_fail;
    string old_notify_fail, verb;
    int i, j, all, size, typ;

    if (query_verb()[0..3] == "setz")
    {
        wear_prepos = "auf";
        remove_prepos = "ab";
        verb = "setzen";
    }
    else
    {
        wear_prepos = "an";
        remove_prepos = "aus";
        verb = "ziehen";
    }
    prepos_fail = sprintf("%s- oder %s%s?\n",wear_prepos,remove_prepos,verb);
    old_notify_fail = query_notify_fail();
    words = explode(str = space(str)," ");
    all = sizeof(map(words,#'lower_case) & ALLES);
    parsed = parse_com(str, this_object(), ({"an","aus","auf","ab"}),
            PARSE_ALL_MATCHES | PARSE_NOT_WITHOUT_TRENNER);
    // wenn parse_com fehlschlaegt, spielt der Spieler moeglicherweise
    // gerade ein Spiel mit "setze 3 4" oder "ziehe a1 b2".
    // In diesem Fall das alte notify fail gewinnen lassen.
    if (parse_com_error(parsed,old_notify_fail ? old_notify_fail :
                "Was möchtest Du " + prepos_fail))
        return 0;
    // Jetzt wird festgestellt, ob an- oder ausgezogen werden soll
    if (parsed[PARSE_TRENNER] == wear_prepos)
    {
        fun = "wear_command";
        typ = MA_WEAR;
    }
    else if (parsed[PARSE_TRENNER] == remove_prepos)
    {
        fun = "remove_command";
        typ = MA_UNWEAR;
    }
    else
        return notify_fail(capitalize(prepos_fail));
    ob = (obs = parsed[PARSE_OBS])[i = 0];
    obs = filter_objects(filter(obs,#'objectp),
            "query_cloth"); // alle Objekte, die Kleider sind, herausfiltern
    if (verb == "setzen") // dann eben noch die aufsetzbaren filtern
        obs = filter_objects(obs,"aufsetzbar");
    size = sizeof(obs);
    if (all)
    {   // mehrere Kleider
        notify_fail(sprintf("Davon kann nichts %sge%s werden.\n",
                    typ == MA_WEAR ? wear_prepos : remove_prepos,
                    verb == "setzen" ? "setzt" : "zogen"));
        while (i < size && this_object())
            j += call_other(obs[i++],fun,1,1);
        return j > 0;
    }
    // Nur eine Kleidung (entweder passende finden oder Fehler zur ersten)
    if (size)
    {   // Objekte gefunden
        do j = call_other(obs[i],fun,0); while (!j && ++i < size);
        return j || call_other(obs[0],fun,1) || 1;
    }
    return notify_fail(wrap(sprintf("%s %s nicht %sge%s werden.",Der(ob),
                    plural("kann","können",ob),
                    typ == MA_WEAR ? wear_prepos : remove_prepos,
                    verb == "setzen" ? "setzt" : "zogen")));
}

/* =====================  K  E  E  P  ======================== */
int keep_filter(mixed ob)
{
    if (!objectp(ob))
        return 0;
    if (ob->query_no_move() || ob->query_money())
        return 0;
    return 1;
}

private int unkeep_all(object* obs)
{
    object ob;
    int cnt = 0;
    foreach(ob:obs)
    {
        if (ob->query_invis())
            continue;
        if (ob->query(P_KEEP_OR_SELL)!=0)
        {
            ob->set(P_KEEP_OR_SELL,0);
            cnt++;
        }
        if (ob->query_container() && !ob->query_locked())
            cnt += unkeep_all(all_inventory(ob));
    }
    return cnt;
}

private int dir_keep_all(object* obs,object *conts)
{
    object ob;
    int cnt = 0,ix;
    string cdesc = " ";
    for (ix=sizeof(conts)-1;ix>=0;ix--)
    {
        cdesc += "in "+einem(conts[ix])+" ";
    }
    foreach(ob:obs)
    {
        if (ob->query_invis())
            continue;
        if (ob->query(P_KEEP_OR_SELL)!=0)
        {
            this_object()->send_message_to(this_object(),
                MT_NOTIFY|MT_LOOK, MA_CRAFT,
                wrap(Der(ob)+cdesc+plural("wurde ","wurden ")
                +"zum Behalten markiert."));
            cnt++;
        }
        if (ob->query_container() && !ob->query_locked())
            cnt += dir_keep_all(all_inventory(ob),conts+({ob}));
    }
    return cnt;
}

int keep_command(string str)
{
    mixed parsed;
    object *obs;
    string dem_ob,der_ob;
    int unkeep = 0,cnt;
    if (!playerp(this_player()))
        return 0; // nur fuer spieler.
    if (space(str)=="")
    {
        cnt = dir_keep_all(all_inventory(this_player()),({}));
        if (cnt==0)
            return notify_fail("Momentan wurde nichts zum Behalten markiert.");
        return 1;
    }
    if (space(str)=="alles nicht mehr")
    {
        unkeep = 2;
    }
    parsed = parse_com(str,this_object(),({"nicht"}), PARSE_AFTER_TRENNER);
    if (sizeof(parsed[PARSE_OBS])==0)
        parsed = parse_com(str,this_object(),({"nicht"}) );
    if (sizeof(parsed[PARSE_OBS])==0)
    {
        return notify_fail(wrap("Du kannst nur Dinge zum Behalten markieren, "
            "die du bei dir hast, und die du verkaufen könntest."));
    }
    if (parse_com_error(parsed,"Was möchtest Du behalten?"))
        return 0;
    obs = filter(parsed[PARSE_OBS],(: keep_filter($1) :));
    unkeep ||= parsed[PARSE_TRENNER]=="nicht"?1:0;
    if (!sizeof(obs))
    {
        return notify_fail(wrap("Du kannst nur Dinge zum Behalten markieren, "
            "die du bei dir hast, und die du verkaufen könntest."));
    }
    // printf("keep: %d %Q",unkeep,obs);
    dem_ob = dem(obs[0]);
    der_ob = Der(obs[0]);
    switch(unkeep)
    {
        case 0:
            obs = filter(obs,(: $1->query(P_KEEP_OR_SELL)==0 :));
            if (!sizeof(obs))
                return notify_fail("Keine Änderung notwendig.");
            obs->set(P_KEEP_OR_SELL,1);
            this_object()->send_message_to(this_object(),MT_NOTIFY|MT_LOOK, MA_CRAFT,
                wrap(sizeof(obs)>1
                ?(sizeof(obs)+" Gegenstände zum Behalten markiert.")
                :(der_ob+" zum Behalten markiert.")));
            return 1;
        case 1:
            obs = filter(obs,(: $1->query(P_KEEP_OR_SELL)!=0 :));
            if (!sizeof(obs))
                return notify_fail("Keine Änderung notwendig.");
            obs->set(P_KEEP_OR_SELL,0);
            this_object()->send_message_to(this_object(),MT_NOTIFY|MT_LOOK, MA_CRAFT,
                wrap(sizeof(obs)>1
                ?("Markierung von "+sizeof(obs)+" Gegenständen entfernt.")
                :("Markierung von "+dem_ob+" entfernt.")));
            return 1;
        case 2:
            cnt = unkeep_all(obs);
            if (!cnt)
                return notify_fail("Keine Änderung notwendig.");
            this_object()->send_message_to(this_object(),MT_NOTIFY|MT_LOOK, MA_CRAFT,
                wrap(cnt>1
                ?("Markierung von "+cnt+" Gegenständen entfernt.")
                :("Markierung von einem Gegenstand entfernt.")));
            return 1;
    }
    return 1;
}


/*
FUNKTION: set_probability_for_guardian_angel
DEKLARATION: nomask void set_probability_for_guardian_angel(int sterblich, int unsterblich)
BESCHREIBUNG:
Damit setzt man die Wahrscheinlichkeit fuer die Rettung durch seinen
Schutzengel. Dies duerfen nur ausgewaehlte Objekte. Nach einer Rettung
oder einem Tod wird die Wahrscheinlichkeit auf den Standardwert von 1,99
gesetzt.
VERWEISE: query_probability_for_guardian_angel, add_hp
GRUPPEN: spieler, kampf
*/
nomask void set_probability_for_guardian_angel(int sterblich, int unsterblich)
{
    if(MASTER_OB->mudlib_privilege_violation("guardian_angel", previous_object(), sterblich, unsterblich))
        probability_for_guardian_angel=({sterblich,unsterblich});
}

/*
FUNKTION: query_probability_for_guardian_angel
DEKLARATION: nomask int *query_probability_for_guardian_angel()
BESCHREIBUNG:
Liefert die Wahrscheinlichkeit fuer die Rettung durch einen Schutzengel
als Array mit dem Wert fuer sterbliche Spieler als erstes und als zweiten
Eintrag den Wert fuer unsterbliche Engel.
VERWEISE: set_probability_for_guardian_angel, add_hp
GRUPPEN: spieler, kampf
*/
nomask int *query_probability_for_guardian_angel()
{
    return deep_copy(probability_for_guardian_angel);
}
/*
FUNKTION: forbidden_attack
DEKLARATION: int forbidden_attack(object attacker, object weapon, object enemy)
BESCHREIBUNG:
Bevor ein Lebewesen enemy vom Lebewesen attacker mit der Waffe weapon
angegriffen werden kann, wird enemy->forbidden("attack", attacker, weapon)
aufgerufen, liefert dieser Aufruf einen Wert ungleich 0 zurueck, findet
der Angriff nicht statt.

Die Funktion forbidden ruft in allen mit enemy->add_controller(
"forbidden_attack", other) angemeldeten Objekten other die Funktionen
other->forbidden_attack(attacker, weapon, enemy) auf. Liefert auch nur eine
dieser Funktionen einen Wert ungleich 0, dann returnt forbidden diesen und
das Lebewesen enemy kann nicht angegriffen werden.

Fuer die Ausgabe einer Meldung an die Lebewesen attacker und enemy und den
Raum muss der Programmierer in forbidden_attack oder forbidden, falls er
diese Funktion ueberlagern will, sorgen.
Beispielanwendung: Ein Orkamulett, dass Angriffe von Orks verhindert.
VERWEISE: forbidden, notify, notify_attack, attack, no_attack,
	  forbidden_use, forbidden_my_attack, forbidden_attack_here
GRUPPEN: spieler, monster, haende, kampf
*/

/*
FUNKTION: notify_attack
DEKLARATION: void notify_attack(object attacker, object weapon, object enemy)
BESCHREIBUNG:
Nachdem das Lebewesen attacker ein Lebewesen enemy mit der Waffe weapon
attackiert hat, wird enemy->notify("attack", attacker, weapon) aufgerufen.

Die Funktion notify ruft in allen mit enemy->add_controller("notify_attack",
other) angemeldeten Objekten other die Funktionen other->notify_attack(
attacker, weapon, enemy) auf. Sowohl enemy als auch other haben dann eine
Moeglichkeit, auf den Angriff von attacker mit der Waffe weapon zu reagieren.
VERWEISE: forbidden, notify, forbidden_attack, attack, no_attack,
          notify_my_attack, notify_attack_here, notify_use
GRUPPEN: spieler, monster, haende, kampf
*/

/*
FUNKTION: forbidden_attack_here
DEKLARATION: int forbidden_attack_here(object attacker, object enemy, object weapon)
BESCHREIBUNG:
Bevor ein Lebewesen enemy vom Lebewesen attacker mit der Waffe weapon
angegriffen werden kann, wird in der Umgebung env->forbidden("attack_here",
attacker, enemy, weapon) aufgerufen, liefert dieser Aufruf einen Wert
ungleich 0 zurueck, findet der Angriff nicht statt.

Die Funktion forbidden ruft in allen mit env->add_controller(
"forbidden_attack_here", other) angemeldeten Objekten other die Funktionen
other->forbidden_attack_here(attacker, enemy, weapon) auf. Liefert auch nur
eine dieser Funktionen einen Wert ungleich 0, dann returnt forbidden diesen
und das Lebewesen enemy kann nicht angegriffen werden.

Fuer die Ausgabe einer Meldung an die Lebewesen attacker und enemy und den
Raum muss der Programmierer in forbidden_attack_here oder forbidden, falls er
diese Funktion ueberlagern will, sorgen.
VERWEISE: forbidden, notify, add_controller, forbidden_attack, notify_attack,
          notify_attack_here, attack, no_attack, forbidden_use,
	  forbidden_my_attack
GRUPPEN: spieler, monster, haende, kampf
*/

/*
FUNKTION: notify_attack_here
DEKLARATION: void notify_attack_here(object attacker, object enemy, object weapon)
BESCHREIBUNG:
Nachdem das Lebewesen attacker ein Lebewesen enemy mit der Waffe weapon
attackiert hat, wird in der Umgebung env->notify("attack_here", attacker,
enemy, weapon) aufgerufen.

Die Funktion notify ruft in allen mit env->add_controller(
"notify_attack_here", other) angemeldeten Objekten other die Funktionen
other->notify_attack_here(attacker, enemy, weapon) auf. Der Raum hat
so die Moeglichkeit darauf zu reagieren.
VERWEISE: forbidden, notify, add_controller, forbidden_attack, notify_attack,
          forbidden_attack_here, attack, no_attack
GRUPPEN: spieler, monster, haende, kampf
*/

/*
FUNKTION: forbidden_my_attack
DEKLARATION: int forbidden_my_attack(object enemy, object weapon, object attacker)
BESCHREIBUNG:
Bevor ein Lebewesen attacker das Lebewesen enemy mit der Waffe weapon
angreifen kann, wird attacker->forbidden("my_attack", enemy, weapon)
aufgerufen, liefert dieser Aufruf einen Wert ungleich 0 zurueck, findet
der Angriff nicht statt.

Die Funktion forbidden ruft in allen mit attacker->add_controller(
"forbidden_my_attack", other) angemeldeten Objekten other die Funktionen
other->forbidden_my_attack(enemy, weapon, attacker) auf. Liefert auch nur
eine dieser Funktionen einen Wert ungleich 0, dann returnt forbidden diesen
und das Lebewesen enemy kann nicht angegriffen werden.

Fuer die Ausgabe einer Meldung an die Lebewesen attacker und enemy und den
Raum muss der Programmierer in forbidden_my_attack oder forbidden, falls er
diese Funktion ueberlagern will, sorgen.
Beispielanwendung: Handfesseln, die den Kampf verhindern
VERWEISE: forbidden, notify, notify_my_attack, attack, no_attack,
          forbidden_use, forbidden_attack, forbidden_attack_here
GRUPPEN: spieler, monster, haende, kampf
*/

/*
FUNKTION: notify_my_attack
DEKLARATION: void notify_my_attack(object enemy, object weapon, object attacker)
BESCHREIBUNG:
Nachdem das Lebewesen attacker ein Lebewesen enemy mit der Waffe weapon
attackiert hat, wird attacker->notify("my_attack", enemy, weapon) aufgerufen.

Die Funktion notify ruft in allen mit attacker->add_controller(
"notify_my_attack", other) angemeldeten Objekten other die Funktionen
other->notify_my_attack(enemy, weapon, attacker) auf. Sowohl attacker als
auch other haben dann eine Moeglichkeit, auf den Angriff auf enemy mit der
Waffe weapon zu reagieren.
VERWEISE: forbidden, notify, forbidden_my_attack, attack, no_attack,
          notify_attack, notify_attack_here, notify_use
GRUPPEN: spieler, monster, haende, kampf
*/

/*
FUNKTION: forbidden_use
DEKLARATION: int forbidden_use(object attacker, object enemy, object weapon)
BESCHREIBUNG:
Bevor ein Lebewesen attacker das Lebewesen enemy mit der Waffe weapon
angreifen kann, wird weapon->forbidden("use", attacker, enemy)
aufgerufen, liefert dieser Aufruf einen Wert ungleich 0 zurueck, findet
der Angriff nicht statt.

Die Funktion forbidden ruft in allen mit weapon->add_controller(
"forbidden_use", other) angemeldeten Objekten other die Funktionen
other->forbidden_use(attacker, enemy, weapon) auf. Liefert auch nur eine
dieser Funktionen einen Wert ungleich 0, dann returnt forbidden diesen
und das Lebewesen enemy kann nicht angegriffen werden.

Fuer die Ausgabe einer Meldung an die Lebewesen attacker evtl. enemy und den
Raum muss der Programmierer in forbidden_use oder forbidden, falls er diese
Funktion ueberlagern will, sorgen.
Beispielanwendung: Auf Monster oder Traeger 'spezialisierte' Waffen
VERWEISE: forbidden, notify, notify_use, attack, forbidden_attack, forbidden_my_attack
GRUPPEN: kampf, waffen
*/

/*
FUNKTION: notify_use
DEKLARATION: void notify_use(object attacker, object enemy, object weapon)
BESCHREIBUNG:
Nachdem das Lebewesen attacker ein Lebewesen enemy mit der Waffe weapon
attackiert hat, wird weapon->notify("use", attacker, enemy) aufgerufen.

Die Funktion notify ruft in allen mit weapon->add_controller("notify_use",
other) angemeldeten Objekten other die Funktionen other->notify_use(
attacker, enemy, weapon) auf. Sowohl weapon als auch other haben dann eine
Moeglichkeit, auf den Angriff von attacker auf enemy mit der Waffe weapon zu
reagieren.
VERWEISE: forbidden, notify, forbidden_use, attack, no_attack
GRUPPEN: kampf, waffen
*/

/* --- add_actions: --- */

protected void add_actions()
{
    add_action("take_command",      "nehme",        -4);
    add_action("take_command",      "nimm");
    add_action("put_command",       "gebe",         -3);
    add_action("put_command",       "gib");
    add_action("put_command",       "lege",         -3);
    add_action("put_command",       "hänge",        -4);
    add_action("put_command",       "stecke",       -5);
    add_action("put_command",       "stelle",       -5);
    add_action("put_command",       "verstaue",     -7);
    add_action("put_command",       "schenke",      -6);
    add_action("empty_command",     "leere",        -4);
    add_action("werfe_command",     "werfe",        -4);
    add_action("werfe_command",     "wirf");
    add_action("schiesse_command",  "schieße",      -6);
    add_action("attackiere_command","töte");
    add_action("attackiere_command","attackiere",   -3);
    add_action("stop_command",      "stoppe",       -4);
    add_action("wield_command",     "führe",        -4);
    add_action("wield_command",     "zücke",        -4);
    add_action("remove_command",    "senke",        -4);
    add_action("wear_command",      "ziehe",        -4);
    add_action("wear_command",      "setze",        -4);
    add_action("feel_command",      "befühle",      -6);
    add_action("feel_command",      "betaste");
    add_action("feel_command",      "fühle",        -4);
    add_action("feel_command",      "taste");
    add_action("show_command",      "zeige",        -4);
    add_action("push_command",      "schiebe",      -6);
    add_action("push_command",      "schubse",      -6);
    add_action("protect_command",   "schütze",      -6);
    add_action("protect_command",   "beschütze",    -8);
    add_action("keep_command",      "behalte",      -6);
}


/* --- End of file. --- */
