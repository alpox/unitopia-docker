// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /i/object/leuchte.c
// Description: Eine Standardfackel/Laterne/bla.
//              Sollte dringend ueberarbeitet werden.
// Modified:    Monty (27.11 96): Fackeln gehen jetzt stillschweigend aus, wenn
//                      man sie in einen nicht-living, nicht-raum bewegt.
//              Freaky (10.03.1998) message auf send_message umgebaut.
//              Freaky (07.12.1999) aus und an besser gemacht

/*
 * Dieser File ist fuer Objekte wie Fackeln, Lampen etc vorgesehen.
 *
 *   void set_destruct_at_out_of_fuel(int flag)    Soll sich die leere Fackel
 *                                                 selbst zerstoeren ?
 *
 *   void set_fuel(int brenndauer)                 Setzt die Brenndauer in
 *                                                 Realzeit-Sekunden.
 *
 *   void add_fuel(int nachfuellmenge)             Zum Regenerieren. Ueber den
 *                                                 bei set_fuel() angegebenen
 *                                                 Wert kommt man allerdings
 *                                                 nicht hinaus.
 *
 *   void set_power(int lux)                       Um wieviel soll das Licht
 *                                                 geschaltet werden ?
 *                                                 (Bei normalen Fackeln 1)
 *                                                 Man kann hier auch negative
 *                                                 Werte angeben, zB fuer
 *                                                 Schwarzlampen.....
 *
 * void set_value(int max, int min)              Der Wert der Leuchte im vollen
 *                                               und leeren Zustand.
 *
 *
 * void an()                                    Zum diretken An- oder Ausmachen
 * void aus()                                   der Leuchte OHNE Meldungen,
 *                                              zB beim kreieren.
 * int query_is_lighted()                       1, wenn die Fackel brennt,
 *                                              sonst 0.
 *
 * void set_burn_under_water                    Leuchtet die Leuchte auch
 * int query_burn_under_water                   Unterwasser?
 *
 * Folgende Routinen werden, sofern sie existieren, aufgerufen:
 *
 *       wenn die Leuchte sich innerhalb eines Lebewesens befindet:
 *                 void player_message(int Prozent, object Spieler)
 *                 void other_message(int Prozent, object Spieler)
 *
 *       wenn die Leuchte sich im Raum herumliegt:
 *                 void room_message(int Prozent)
 *
 *  Beispiele fuer obige Routinen findet man in /obj/fackel.c.
 *
 * Erlischt die Leuchte infolge Brennstoffmangels, so wird im environment
 * die Funktion:
 *                      void leuchte_leer()
 *
 * aufgerufen. Dies kann man zB dazu benutzen, eine neue zu clonen.
 *
 */

#pragma save_types

inherit "/i/item";
inherit "/i/move";
inherit "/i/value";

#include <deklin.h>
#include <description.h>
#include <landschaft.h>
#include <message.h>
#include <misc.h>
#include <notify_fail.h>

private int fuel, max_fuel;
private int power;
private int destruct_at_out_of_fuel;
private mixed not_extinguishable;
private int max_value, min_value;
private int last_percent;
private int burn_under_water;
private mixed light_string = "brennend";
private object owner;
private string *light_verbs = ({"brennt","brennen"}),
               *ext_verbs=({"erlischt","erlöschen"});

#define LIGHT_VERB plural(light_verbs[0],light_verbs[1])
#define EXT_VERB plural(ext_verbs[0],ext_verbs[1])
#define MIN_EVALS 100000

/*
FUNKTION: query_not_extinguishable
DEKLARATION: mixed query_not_extinguishable()
BESCHREIBUNG:
Liefert einen Wert != 0, wenn diese Leuchte nicht ausgeht.
Wird ein String geliefert, so ist dies die Erklaerung dafuer.
VERWEISE: set_not_extinguishable
GRUPPEN: leuchte
*/
mixed query_not_extinguishable() { return not_extinguishable; }

/*
FUNKTION: set_not_extinguishable
DEKLARATION: void set_not_extinguishable(mixed why)
BESCHREIBUNG:
Damit setzt man, ob die Leuchte wieder ausgehen kann.
Ist why ein String, so ist dies die Begruendung fuer den Spieler.
VERWEISE: query_not_extinguishable
GRUPPEN: leuchte
*/
void set_not_extinguishable(mixed why)
{
    not_extinguishable = why;
}

/*
FUNKTION: query_destruct_at_out_of_fuel
DEKLARATION: int query_destruct_at_out_of_fuel()
BESCHREIBUNG:
Liefert einen Wert != 0, wenn sich die Leuchte zerstoert, wenn sie leer ist.
VERWEISE: set_destruct_at_out_of_fuel,
          set_fuel, add_fuel, query_fuel, query_max_fuel
GRUPPEN: leuchte
*/
int query_destruct_at_out_of_fuel() { return destruct_at_out_of_fuel; }

/*
FUNKTION: set_destruct_at_out_of_fuel
DEKLARATION: void set_destruct_at_out_of_fuel(int a)
BESCHREIBUNG:
Mit einem Wert != 0 zerstoert sich die Leuchte, wenn sie leer ist.
VERWEISE: query_destruct_at_out_of_fuel,
          set_fuel, add_fuel, query_fuel, query_max_fuel
GRUPPEN: leuchte
*/
void set_destruct_at_out_of_fuel(int a)
{
    destruct_at_out_of_fuel = a != 0;
}

/*
FUNKTION: set_fuel
DEKLARATION: void set_fuel(int brenndauer)
BESCHREIBUNG:
Damit wird die Brenndauer in Sekunden gesetzt.
VERWEISE: add_fuel, query_fuel, query_max_fuel,
          set_destruct_at_out_of_fuel, query_destruct_at_out_of_fuel,
          notify_burned_out
GRUPPEN: leuchte
*/
void set_fuel(int f)
{
    max_fuel = f;
    fuel = f;
}

/*
FUNKTION: add_fuel
DEKLARATION: void add_fuel(int brenndauer)
BESCHREIBUNG:
Diese Brenndauer wird zur vorhandenen dazu addiert. Ueber den mit set_fuel
gesetzten Wert kommt man aber nicht hinaus.
VERWEISE: set_fuel, query_fuel, query_max_fuel,
          set_destruct_at_out_of_fuel, query_destruct_at_out_of_fuel,
          notify_burned_out
GRUPPEN: leuchte
*/

void add_fuel(int a)
{
   fuel += a;
   if (fuel > max_fuel)
        fuel = max_fuel;
}

/*
FUNKTION: query_max_fuel
DEKLARATION: int query_max_fuel()
BESCHREIBUNG:
Liefert die maximale Brenndauer der Leuchte. Dies ist die Brenndauer,
die mit dem letzten set_fuel gesetzt wurde.
VERWEISE: set_fuel, add_fuel, query_fuel,
          set_destruct_at_out_of_fuel, query_destruct_at_out_of_fuel
GRUPPEN: leuchte
*/
int query_max_fuel() { return max_fuel; }

/*
FUNKTION: set_power
DEKLARATION: void set_power(int power)
BESCHREIBUNG:
Damit wird die Leuchtstaerke gesetzt. Auch negative Werte sind moeglich.
VERWEISE: query_power, set_fuel
GRUPPEN: leuchte
*/
void set_power(int a) { power = a; }

/*
FUNKTION: query_power
DEKLARATION: int query_power()
BESCHREIBUNG:
Liefert die Leuchtstaerke zurueck.
VERWEISE: set_power, query_fuel, query_lamp
GRUPPEN: leuchte
*/
int query_power() { return power; }

/*
FUNKTION: set_light_adjektiv
DEKLARATION: void set_light_adjektiv(mixed str)
BESCHREIBUNG:
Damit wird das Adjektiv gesetzt, welches die Leuchte bekommt, wenn sie
brennt. Standardmaessig ist "brennend" gesetzt.
VERWEISE: query_light_adjektiv, query_is_lighted, set_adjektiv
GRUPPEN: leuchte
*/
void set_light_adjektiv(mixed str) { light_string = str; }

/*
FUNKTION: query_light_adjektiv
DEKLARATION: mixed query_light_adjektiv()
BESCHREIBUNG:
Liefert das Adjektiv, welches die Leuchte bekommt, wenn sie brennt.
Standardmaessig ist "brennend" gesetzt.
VERWEISE: set_light_adjektiv, query_is_lighted, query_adjektiv
GRUPPEN: leuchte
*/
mixed query_light_adjektiv() { return light_string; }

/*
FUNKTION: set_light_verb
DEKLARATION: void set_light_verb(string singular, string plural)
BESCHREIBUNG:
Damit wird das Verb (jeweils fuer Einzahl und Mehrzahl) gesetzt, welches zur
Ausgabe der Meldungen der Leuchte genommen wird. Standardmaessig ist
"brennt" und "brennen" gesetzt.
VERWEISE: set_own_light
GRUPPEN: leuchte
*/
void set_light_verb(string singular, string plural)
{
  light_verbs=({singular, plural});
}

/*
FUNKTION: query_light_verb
DEKLARATION: string *query_light_verb()
BESCHREIBUNG:
Liefert das Verb, welches zur Ausgabe der Meldungen zum Brennen der Leuchte
genommen wird. Standardmaessig ist "brennt" und "brennen" gesetzt.
Das zurueckgelieferte Array hat die Form ({string singular, string plural}).
VERWEISE: set_own_light
GRUPPEN: leuchte
*/
string *query_light_verb()
{
  return copy(light_verbs);
}

/*
FUNKTION: set_extinguish_verb
DEKLARATION: void set_extinguish_verb(string singular, string plural)
BESCHREIBUNG:
Damit wird das Verb (jeweils fuer Einzahl und Mehrzahl) gesetzt, welches zur
Ausgabe der Meldungen beim Erloeschen der Leuchte genommen wird.
Standardmaessig ist "erlischt" und "erloeschen" gesetzt.
VERWEISE: set_own_light
GRUPPEN: leuchte
*/
void set_extinguish_verb(string singular, string plural)
{
  ext_verbs=({singular, plural});
}
/*
FUNKTION: query_extinguish_verb
DEKLARATION: string *query_extinguish_verb()
BESCHREIBUNG:
Liefert das Verb, welches zur Ausgabe der Meldungen beim Erloeschen der
Leuchte genommen wird. Standardmaessig ist "erlischt" und "erloeschen"
gesetzt. Das zurueckgelieferte Array hat die Form
({string singular, string plural}).
VERWEISE: set_own_light
GRUPPEN: leuchte
*/
string *query_extinguish_verb()
{
  return ext_verbs;
}

/*
FUNKTION: set_burn_under_water
DEKLARATION: void set_burn_under_water(int buw)
BESCHREIBUNG:
Damit wird gesetzt, ob die Leuchte unter Wasser weiterbrennt (buw != 0).
VERWEISE: query_burn_under_water, query_is_lighted
GRUPPEN: leuchte
*/
void set_burn_under_water(int x) { burn_under_water = x != 0; }

/*
FUNKTION: query_burn_under_water
DEKLARATION: int query_burn_under_water()
BESCHREIBUNG:
Falls != 0, brennt die Leuchte unter Wasser weiter.
VERWEISE: set_burn_under_water, query_is_lighted
GRUPPEN: leuchte
*/
int query_burn_under_water() { return burn_under_water; }

/*
FUNKTION: query_is_lighted
DEKLARATION: int query_is_lighted()
BESCHREIBUNG:
Liefert einen Wert != 0, wenn diese Leuchte brennt.
VERWEISE: query_light_adjektiv, an, aus, light_on, light_off, query_fuel, query_lamp
GRUPPEN: leuchte
*/
int query_is_lighted()
{
    return query_own_light() != 0;
}

string query_short(object betrachter)
{
    string adjektiv;

    if (query_is_lighted() && (!light_string || query_short_string()))
    {
        if (light_string)
        {
            if (pointerp(light_string))
                adjektiv = light_string[0];
            else
                adjektiv = light_string;
            return item::query_short(betrachter) + " (" + adjektiv + ")";
        }
        else
            return item::query_short(betrachter) + " (brennend)";
    }
    return item::query_short(betrachter);
}

void init()
{
    add_action("light", "entzünde");
    add_action("light", "zünde",-4);
    add_action("extinguish", "lösche",-5);
}

// Ruft das notify in der Leuchte und allen Umgebungen auf.
private void do_env_notifies(varargs mixed* args)
{
    for (object ob = this_object(); ob; ob = environment(ob))
        ob->notify(args...);
}

// Testen, ob die Leuchte brennen darf


int may_burn()
{
    object env;

    env = environment();

    // Das elektrische Licht ist noch nicht erfunden.
    if(!env) return 0;

    while(env)
    {
        if (env->allowed("burn",this_object()))
            return 1;
        else if (env->forbidden("burn",this_object()))
            return 0;
        else if ((!burn_under_water) && env->query_room() &&
                 (env->query_type(LANDSCHAFT)&L_UNTERWASSER))
            return 0;
        env = environment(env);
    }
    // zum Brennen in anderer Umgebung als Living oder Raum brauchts allowed...
    if(!living(environment()) && !environment()->query_room())
        return 0;
    return 1;
}
/*
FUNKTION: forbidden_burn
DEKLARATION: int forbidden_burn(object leuchte)
BESCHREIBUNG:
Immer wenn getestet wird, ob eine Leuchte brennen darf, werden alle Umgebungen
durchgegangen und sofern das Leuchten mit allowed_burn nicht erlaubt wurde,
env->forbidden("burn", leuchte) aufgerufen. Liefert diese Funktion einen
Wert != 0 zurueck, so kann die Leuchte nicht (mehr) brennen.

Die Funktion forbidden ruft dann in allen mit env->add_controller(
"forbidden_burn", other) angemeldeten Objekten other die Funktion
other->forbidden_burn(leuchte) auf. Liefert auch nur eine dieser Funktionen
einen Wert != 0, so wird dieser zurueckgeliefert und die Leuchte kann nicht
brennen.

Fuer die Ausgabe einer Meldung an environment(leuchte) (falls es ein Lebewesen
oder Raum ist), muss der Programmierer im forbidden_burn selber sorgen.
VERWEISE: forbidden, notify_burn, allowed_burn, notify_light, notify_extinguish
GRUPPEN: leuchte
*/

/*
FUNKTION: allowed_burn
DEKLARATION: int allowed_burn(object leuchte)
BESCHREIBUNG:
Immer wenn getest wird, ob eine Leuchte brennen darf, werden alle Umgebungen
durchgegangen und es wird env->allowed_burn(leuchte) aufgerufen.
Liefert diese Funktion einen Wert != 0 zurueck, so wird der Test abgebrochen
und die Leuchte kann brennen.

Die Funktion allowed ruft dann in allen mit env->add_controller(
"allowed_burn", other) angemeldeten Objekten other die Funktion
other->allowed_burn(leuchte) auf. Ist mindestens ein Objekt angemeldet und
liefern alle angemeldeten Objekte einen Wert != 0 zurueck, so liefert auch
allowed einen Wert != 0 und die Leuchte darf brennen.
VERWEISE: allowed, notify_burn, forbidden_burn, notify_light, notify_extinguish
GRUPPEN: leuchte
*/

/*
FUNKTION: an
DEKLARATION: void an()
BESCHREIBUNG:
Damit wird die Leuchte ohne eine Meldung entzuendet. Es findet dabei keine
Ueberpruefung statt, ob die Leuchte ueberhaupt brennen darf.
VERWEISE: aus, light_on, light_off, query_is_lighted, query_fuel
GRUPPEN: leuchte
*/
void an()
{
    if (!query_is_lighted())
    {
        call_out("burn",1);
        set_own_light(power);
        if (light_string && !this_object()->adjektiv(light_string))
            this_object()->add_adjektiv(light_string,1);
        do_env_notifies("light",this_object());
    }
}

/*
FUNKTION: notify_light
DEKLARATION: void notify_light(object leuchte)
BESCHREIBUNG:
Wenn die Leuchte leuchte entzuendet wird, so wird in der Leuchte selber und
in allen Umgebungen env der Leuchte leuchte->notify("light",leuchte) bzw.
env->notify("light",leuchte) aufgerufen.

Die Funktion notify ruft dann in allen mit leuchte-> bzw. env->add_controller(
"notify_light", other) angemeldeten Objekten other die Funktion
other->notify_light("leuchte") auf. Sowohl leuchte bzw. env als auch other
haben dann die Moeglichkeit darauf zu reagieren.
VERWEISE: an, light_on, query_is_lighted, query_own_light
GRUPPEN: leuchte, licht
*/

// Hilfsfunktion um den Lichtlevel im Raum zu berechnen
// TODO: schauen, ob die Umgebung Lichtdurchlaessig ist. Wenn nein, diese
// Umgebung als Root-Umgebung nehmen... (Dazu muss query_light() in den
// Container rein)
static int query_room_light()
{
    object env;

    if (env = environment())
	return env->query_light();
}

/*
FUNKTION: light_on
DEKLARATION: void light_on()
BESCHREIBUNG:
Damit wird die Leuchte mit einer Meldung entzuendet. Es wird this_player()
als Verursacher vorausgesetzt. (Falls this_player()==0 gibt es keine
Meldungen.) Dabei findet keine Ueberpruefung der Brenndauer statt.
VERWEISE: an, aus, light_off, query_is_lighted, query_fuel
GRUPPEN: leuchte
*/
void light_on()
{
    int old_light, new_light;

    if (query_is_lighted())
    {
	if(this_player())
    	    this_object()->send_message_to(this_player(), MT_LOOK|MT_FEEL, MA_USE,
        	wrap(Der() + " " + LIGHT_VERB + " schon.\n"));
        return;
    }
    if (!may_burn())
    {
	if(this_player())
    	    this_object()->send_message_to(this_player(),
		MT_LOOK|MT_FEEL, MA_USE,
        	wrap(Der() + plural(" lässt"," lassen") +
            	    " sich hier nicht anzünden.\n"));
        return;
    }

    old_light = query_room_light();
    new_light = old_light + power;

    if (this_player() && ENVR(TP) == ENVR(TO))
    {
        if (old_light <= 0)
        {
            if (new_light <= 0)
                this_player()->send_message(MT_LOOK,MA_USE,
                    wrap(Der(OBJ_TP)+" werkelt an "+seinem()+" herum."),
                    "Es bleibt dunkel.\n",this_player());
            else
                this_player()->send_message(MT_LOOK,MA_USE,
                    wrap(Der(OBJ_TP)+" entzündet "+seinen()+".\n"+
                    "Es wird hell."),
                    "Es wird hell.\n",this_player());
        }
        else
        {
            if (new_light <= 0)
                this_player()->send_message(MT_LOOK,MA_USE,
                    wrap(Der(OBJ_TP)+" entzündet "+seinen()+".\n"+
                    "Es wird dunkel."),
                    "Es wird dunkel.\n",this_player());
            else
                this_player()->send_message(MT_LOOK,MA_USE,
                    wrap(Der(OBJ_TP)+" entzündet "+seinen()+"."),
                    wrap("Du entzündest "+deinen()+"."),this_player());
        }
    }
    an();
}

int light(string str)
{
    string rest;

    rest = me(str);
    if (rest != "an" && rest != "")
        return notify_fail("Zünde was an?\n");
    if (query_is_lighted())
        return notify_fail(Er() + plural(" ist"," sind") +
            " bereits angezündet.\n");
    if (fuel <= 0)
        return notify_fail("Da gibt es nichts mehr zum Anzünden.\n");
    light_on();
    return 1;
}

/*
FUNKTION: aus
DEKLARATION: void aus()
BESCHREIBUNG:
Damit wird die Leuchte ohne eine Meldung ausgeknippst.
VERWEISE: an, light_on, light_off, query_is_lighted, query_fuel
GRUPPEN: leuchte
*/
void aus()
{
    if (query_is_lighted())
    {
        remove_call_out("burn");
        set_own_light(0);
        if (light_string)
            this_object()->delete_adjektiv(light_string);
        do_env_notifies("extinguish",this_object());
    }
}
/*
FUNKTION: notify_extinguish
DEKLARATION: void notify_extinguish(object leuchte)
BESCHREIBUNG:
Wenn die Leuchte leuchte erlischt, so wird in der Leuchte selber und in
allen Umgebungen env der Leuchte leuchte->notify("extinguish",leuchte) bzw.
env->notify("extinguish",leuchte) aufgerufen.

Die Funktion notify ruft dann in allen mit leuchte-> bzw. env->add_controller(
"notify_extinguish", other) angemeldeten Objekten other die Funktion
other->notify_extinguish(leuchte) auf. Sowohl leuchte bzw. env als auch
other haben dann die Moeglichkeit darauf zu reagieren.
VERWEISE: aus, light_off, query_is_lighted, query_own_light, notify_burned_out
GRUPPEN: leuchte, licht
*/

/*
FUNKTION: light_off
DEKLARATION: void light_off()
BESCHREIBUNG:
Damit wird die Leuchte mit einer Meldung ausgeknippst.
VERWEISE: an, aus, light_on, query_is_lighted, query_fuel
GRUPPEN: leuchte
*/
void light_off()
{
    int old_light, new_light;

    if (!query_is_lighted())
    {
        send_message_to(this_player(), MT_LOOK|MT_FEEL, MA_USE,
            wrap(Dein() + " " + LIGHT_VERB + " nicht."));
        return;
    }
    old_light = query_room_light();
    new_light = old_light - query_own_light();
    if (!environment())
        return;
    if (!living(environment()))
    {
        if (old_light <= 0)
        {
            if (new_light > 0)
                this_object()->send_message(MT_LOOK, MA_UNKNOWN,
                    wrap(Wer(0, ART_AAA, "") + " " + EXT_VERB +
                    " und es wird hell."));
        }
        else
        {
            if (new_light > 0)
                this_object()->send_message(MT_LOOK, MA_UNKNOWN,
                    wrap(Wer(0, ART_AAA, "") + " " + EXT_VERB + "."));
            else
                this_object()->send_message(MT_LOOK, MA_UNKNOWN,
                    wrap(Wer(0, ART_AAA, "") + " " + EXT_VERB +
                    " und es wird dunkel."));
        }
    }
    else if (old_light <= 0)
    {
        if (new_light <= 0)
            this_object()->send_message(MT_LOOK, MA_UNKNOWN,
                wrap(Ihr(0,"") + " " + EXT_VERB + "."),
                wrap(Dein(0,"") + " " + EXT_VERB + "."), environment());
        else
            this_object()->send_message(MT_LOOK, MA_UNKNOWN,
                wrap(Ihr(0,"") + " " + EXT_VERB + " und es wird hell."),
                wrap(Dein(0,"") + " " + EXT_VERB + " und es wird hell."),
                environment());
    }
    else
    {
        if (new_light <= 0)
            this_object()->send_message(MT_LOOK, MA_UNKNOWN,
                wrap(Ihr(0,"") + " " + EXT_VERB + " und es wird dunkel."),
                wrap(Dein(0,"") + " " + EXT_VERB + " und es wird dunkel."),
                environment());
        else
            this_object()->send_message(MT_LOOK, MA_UNKNOWN,
                wrap(Ihr(0,"") + " " + EXT_VERB + "."),
                wrap(Dein(0,"") + " " + EXT_VERB + "."),environment());
    }
    aus();
}

void burn()
{
    int percent, step;
    object env;

    if (find_call_out("burn") > -1)
        return;
    if (get_eval_cost() < MIN_EVALS)
    {
	call_out("burn", 2);
	return;
    }
    if (!may_burn())
    {
        light_off();
        return;
    }
    if (max_fuel <= 0 || fuel <= 0)
    {
        light_off();
        do_env_notifies("burned_out", this_object());
        if (environment())
            environment()->leuchte_leer();
        if (destruct_at_out_of_fuel)
            remove();
        return;
    }
    do_env_notifies("burn",this_object());
    if (fuel >= max_fuel)
        fuel = max_fuel;
    percent = (fuel * 100) / max_fuel;
    if (percent <= 0)
    {
        call_out("burn",fuel);
        fuel = 0;
        return;
    }
    step = max_fuel / 100;
    if (step <= 0)
        fuel -= 1;
    else
        fuel -= step;
    call_out("burn", step || 1);
    if (percent == last_percent)
        return;
    last_percent = percent;
    if (env = environment())
    {
        if (living(env))
        {
            this_object()->player_message(percent,env);
            if (env && environment(env))
                this_object()->other_message(percent,env);
        }
        else
            this_object()->room_message(percent);
    }
}

/*
FUNKTION: notify_burn
DEKLARATION: void notify_burn(object leuchte)
BESCHREIBUNG:
Wenn eine Leuchte (z.B Fackel) brennt, wird ab und zu in allen Umgebungen
all_environment()->notify("burn", leuchte) aufgerufen. Danach wird auch
die verbleibende Brenndauer reduziert und es werden Meldungen ausgegeben.

Die Funktion notify ruft dann in allen mit env->add_controller("notify_burn",
other) angemeldeten Objekten other die Funktion other->notify_burn(leuchte)
auf. Sowohl env als auch other haben dann die Moeglichkeit, noch ein paar
zusaetzliche Aktionen auszufuehren.
VERWEISE: notify, forbidden_burn, allowed_burn, notify_light, notify_extinguish
GRUPPEN: leuchte
*/
/*
FUNKTION: notify_burned_out
DEKLARATION: void notify_burned_out(object leuchte)
BESCHREIBUNG:
Wenn die Leuchte leuchte erloschen ist, weil die Brenndauer abgelaufen ist,
wird in der Leuchte selber und in allen Umgebungen env der Leuchte
notify("burned_out",leuchte) aufgerufen.

Die Funktion notify ruft dann in allen mit leuchte-> bzw. env->add_controller(
"notify_burned_out", other) angemeldeten Objekten other die Funktion
other->notify_burned_out(leuchte) auf. Sowohl leuchte bzw. env als auch
other haben dann die Moeglichkeit darauf zu reagieren.
VERWEISE: set_fuel, query_fuel, notify_extinguish, notify_burn
GRUPPEN: leuchte, licht
*/

int extinguish(string str)
{
    string rest;
    mixed unloeschbar;

    rest = me(str);
    if (rest != "" && rest != "aus")
        return notify_fail("Lösche was?", FAIL_NOT_OBJ);

    if (!query_is_lighted())
        return notify_fail(Dein() + " ist nicht angezündet.",
            FAIL_INTERNAL);

    unloeschbar = this_object()->query_not_extinguishable();
    if (unloeschbar)
        return notify_fail(stringp(unloeschbar)?unloeschbar:
        Dein() + " lässt sich nicht löschen.", FAIL_INTERNAL);

    light_off();
    return 1;
}

/*
FUNKTION: query_fuel
DEKLARATION: int query_fuel()
BESCHREIBUNG:
Liefert die Brenndauer in Sekunden zurueck.
VERWEISE: set_fuel, add_fuel, query_max_fuel, query_lamp,
          set_destruct_at_out_of_fuel, query_destruct_at_out_of_fuel
GRUPPEN: leuchte
*/
int query_fuel()
{
    int tc;

    if ((tc = find_call_out("burn")) >= 0)
        return fuel + tc;
    return fuel;
}

/*
FUNKTION: set_value
DEKLARATION: varargs void set_value(int max, int min)
BESCHREIBUNG:
Damit wird der Wert der Leuchte im vollen und leeren Zustand gesetzt.
VERWEISE: set_fuel, set_power
GRUPPEN: leuchte
*/
varargs void set_value(int max, int min)
{
    max_value = max;
    min_value = min;
}

int query_value()
{
    if (max_fuel <= 0)
        return min_value;
    return min_value + ((query_fuel() * (max_value-min_value)) / max_fuel);
}

private void light_moved(string controller,mapping mv_infos)
{
    if (query_is_lighted() && !may_burn())
        light_off();
}

void just_moved() // Wegen umbennenung notify_move=>notify_moved
{
    if (query_is_lighted() && !may_burn())
        light_off();
    if (owner)
	owner->delete_controller("notify_moved", #'light_moved);
    owner = auto_owner_search(this_object());
    if (owner)
	owner->add_controller("notify_moved", #'light_moved);
}

void create()
{
    ::create();
    add_controller("concerned_strike",this_object());
    add_controller("notify_moved",#'light_moved);
}

int concerned_strike(object womit, mixed was)
{
    if (was == this_object() || (mappingp(was) && here(was["name"])))
        return 80;
}

int do_strike(object womit, mixed was, mixed melder)
{
    if (was == this_object() || (mappingp(was) && here(was["name"]))) {
        if (query_is_lighted())
        {
	    if(this_player())
		send_message_to(this_player(),MT_NOTIFY,MA_USE,
            	    wrap(Er()+plural(" ist"," sind")+" bereits angezündet."));
            return 1;
        }
        if (fuel <= 0)
        {
            notify_fail("Da gibt es nichts mehr zum Anzünden.\n");
            return 0;
        }
        if (melder && this_player())
        {
            send_message_to(this_player(),MT_NOTIFY,MA_USE,
                wrap("Du stellst fest, dass Du "+den(melder)+" überhaupt "
                     "nicht zum Anzünden "+des(this_object())+" brauchst."));
        }
        light_on();
        return 1;
    }
}

int query_obvious()
{
    return query_is_lighted();
}

protected mixed desc_condition(string name, mixed info, mixed* par)
{
    switch(name)
    {
	case T_ATOM_IS_LIGHTED:
	    return query_is_lighted();
    }
    
    return ::desc_condition(name, info, par);
}

protected mixed desc_number(string name, mixed info, mixed* par)
{
    switch(name)
    {
	case TN_FUEL_PERCENT:
	    return (query_fuel()*100)/query_max_fuel();
    }
    
    return ::desc_number(name, info, par);
}

/*
FUNKTION: query_lamp
DEKLARATION: int query_lamp()
BESCHREIBUNG:
Liefert bei einer Leuchte 1 zurueck.
VERWEISE: query_fuel, query_power, query_is_lighted
GRUPPEN: leuchte
*/
int query_lamp()
{
    return 1;
}

/*
FUNKTION: T_LISTE
DEKLARATION: Liste der T-Defines fuer Leuchten
BESCHREIBUNG:

Vordefinierte Bedingungen:
 - T_IS_LIGHTED		Die Leuchte ist an.

Vordefinierte Werte zum Vergleich mit T_GREATER & Co.:
 - TN_FUEL_PERCENT	Der Fuellstand in Prozent

GRUPPEN: leuchte
*/
