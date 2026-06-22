// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/i/money/money.c
// Description: Pinke Pinke.
// Author:	Francis
// Modified By:	Freaky	(24.11.95) Auf countob umgestellt.
// Modified By:	Freaky	(26.11.95) query_count_name() -> query_singular_name()
//				   split_object()
//		Freaky  (28.01.2000) set_extra_long() in split_object()

#pragma save_types

inherit "/i/object/countob";

#include <money.h>
#include <level.h>
#include <notify_fail.h>
#include <message.h>
#include <deklin.h>
#include <misc.h>
#include <error.h>

private string extra_long;
private int flags;

/*
FUNKTION: set_extra_long
DEKLARATION: void set_extra_long(string str)
BESCHREIBUNG:
Setzt die ausfuehrliche Beschreibung des Geldes.
Fuer Standard-Waehrungen sollte man init_money() verwenden.
VERWEISE: set_extra_long, query_extra_long, init_money
GRUPPEN: handel
*/

void set_extra_long(string str) { extra_long = str; }

/*
FUNKTION: query_extra_long
DEKLARATION: string query_extra_long()
BESCHREIBUNG:
Liefert die ausfuehrliche Beschreibung des Geldes.
VERWEISE: set_extra_long, query_extra_long
GRUPPEN: handel
*/

string query_extra_long() { return extra_long; }

/*
FUNKTION: set_valutas
DEKLARATION: void set_valutas(string str)
BESCHREIBUNG:
Setzt den Namen die Waehrung im Plural und die Plural-IDs.
Fuer Standard-Waehrungen sollte man init_money() verwenden.
VERWEISE: set_valuta, set_valutas, query_valuta, query_valutas, init_money, set_plural_name, set_plural_id
GRUPPEN: handel
*/

void set_valutas(string str)
{
    if (stringp(str))
    {
	set_plural_id(({str = lower_case(str), "münzen"}));
	set_plural_name(str);
    }
}

/*
FUNKTION: set_valuta
DEKLARATION: void set_valuta(string str)
BESCHREIBUNG:
Setzt den Namen die Waehrung im Singular, die Singular-IDs, den Count-Type
und die ausfuehrliche Beschreibung (letztere nur dann, wenn die Waehrung
in der Zentralbank bekannt ist).
Fuer Standard-Waehrungen sollte man init_money() verwenden.
VERWEISE: set_valuta, set_valutas, query_valuta, query_valutas, init_money, set_singular_name, set_count_type, set_extra_long
GRUPPEN: handel
*/

void set_valuta(string str)
{
    if (stringp(str))
    {
	set_id(({str = lower_case(str), "münze"}));
	set_singular_name(str);
	set_count_type(str);
	set_extra_long(ZENTRALBANK->query_money_description(str));
    }
}

/*
FUNKTION: query_valuta
DEKLARATION: string query_valuta()
BESCHREIBUNG:
Liefert den Namen die Waehrung im Singular.
Identisch mit query_singular_name() und idR. mit query_count_type().
VERWEISE: set_valuta, set_valutas, query_valuta, query_valutas, query_singular_name, query_count_type
GRUPPEN: handel
*/

string query_valuta() { return query_singular_name(); }

/*
FUNKTION: query_valutas
DEKLARATION: string query_valutas()
BESCHREIBUNG:
Liefert den Namen die Waehrung im Plural.
Identisch mit query_plural_name().
VERWEISE: set_valuta, set_valutas, query_valuta, query_valutas, query_plural_name
GRUPPEN: handel
*/

string query_valutas() { return query_plural_name(); }

/*
FUNKTION: set_money
DEKLARATION: void set_money(int m)
BESCHREIBUNG:
Setzt die Anzahl des Geldes.
Identisch mit set_count().
VERWEISE: set_money, add_money, query_money, query_transaction_value, set_count
GRUPPEN: handel
*/

void set_money(int m) { set_count(m); }

/*
FUNKTION: add_money
DEKLARATION: void add_money(int m)
BESCHREIBUNG:
Aendert die Anzahl des Geldes.
Identisch mit add_count().
VERWEISE: set_money, add_money, query_money, query_transaction_value, add_count
GRUPPEN: handel
*/

void add_money(int m) { add_count(m); }

/*
FUNKTION: query_money
DEKLARATION: int query_money()
BESCHREIBUNG:
Liefert die Anzahl des Geldes.
Identisch mit query_count().

Diese Funktion dient auch dazu, Geld als solches zu erkennen.
Liefert query_money() bei einem beliebigen Objekt einen Wert ungleich 0,
so handelt es sich um Geld.
VERWEISE: set_money, add_money, query_money, query_transaction_value, query_count
GRUPPEN: handel
*/

int query_money() { return query_count(); }

/*
FUNKTION: set_money_flags
DEKLARATION: void set_money_flags(int flag)
BESCHREIBUNG:
Damit kann man bestimmte Flags fuer das Geld setzen.
Folgende Flags sind derzeit (in money.h) definiert:

    MONEY_NOT_DEKLIN:	Das Geld wird nicht dekliniert.

VERWEISE: init_money, query_money_flags, set_valuta, set_valutas
GRUPPEN: handel, bank
*/
void set_money_flags(int fl)
{
    flags = fl;
}

/*
FUNKTION: query_money_flags
DEKLARATION: int query_money_flags()
BESCHREIBUNG:
Liefert die mit set_money_flags gesetzten Flags zurueck.
VERWEISE: init_money, set_money_flags, set_valuta, set_valutas
GRUPPEN: handel, bank
*/
int query_money_flags()
{
    return flags;
}

/*
FUNKTION: query_transaction_value
DEKLARATION: int query_transaction_value()
BESCHREIBUNG:
Wird ein Geld-Objekt A in die Umgebung eines zweiten Geld-Objektes B
mit demselben query_valuta() bewegt, so wird das nicht bewegte Objekt B
zerstoert und die Anzahl von A entsprechend erhoeht. Die urspruengliche
Anzahl von A erhaelt man mit der Funktion query_transaction_value().
Identisch mit query_transaction_count().
VERWEISE: set_money, add_money, query_money, query_transaction_value, query_transaction_count
GRUPPEN: handel
*/

int query_transaction_value() { return query_transaction_count(); }

string query_long(object who)
{
    return ::query_long(who) + query_extra_long();
}

string query_smell()
{
    return ::query_smell() || "Geld stinkt nicht.\n";
}

/*
FUNKTION: init_money
DEKLARATION: void init_money(int wert, string valuta)
BESCHREIBUNG:
Mit dieser Funktion initialisiert man das Geld mit dem Wert 'wert' in der
Waehrung 'valuta'. Die Waehrung muss in der Zentralbank bekannt sein, da
die Informationen ueber Plural und Geschlecht dort geholt werden.

Das Geld kann auch direkt beim Aufruf von clone_object() initialisiert
werden, in dem die beiden Argumente dort angegeben werden. Zum Beispiel:
    object geld = clone_object("/obj/money", 200, "gulden");

VERWEISE: set_valuta, set_valutas, set_money
GRUPPEN: handel
*/
void init_money(int wert, string val)
{
    string tmp;
    mixed *info;

    if (wert <= 0)
	do_error2("Es wurde versucht, weniger als eine Geldeinheit zu erschaffen.\n",
	    __FILE__, object_name(extern_call()?previous_object():this_object()),
	    __LINE__);
    else if (!stringp(val))
	raise_error("Kein String als Währung übergeben.\n");
    else if (tmp = ZENTRALBANK->query_accepted_valuta(val))
    {
	info = ZENTRALBANK->query_money_info(tmp);
	set_valuta(info[MONEY_VALUTA]);
	set_valutas(info[MONEY_VALUTAS]);
	set_gender(info[MONEY_GENDER]);
	set_money_flags((sizeof(info) > MONEY_FLAGS) && info[MONEY_FLAGS]);
	if(sizeof(info) > MONEY_SG_IDS && info[MONEY_SG_IDS])
	    add_id(info[MONEY_SG_IDS]);
	if(sizeof(info) > MONEY_PL_IDS && info[MONEY_PL_IDS])
	    add_id(info[MONEY_PL_IDS]);
	set_count(wert);
    }
    else
	do_warning2("Die Währung '"+val+"' ist nicht bekannt.\n",
	    __FILE__, object_name(extern_call()?previous_object():this_object()),
	    __LINE__);
}

string query_dekliniert(int fall, object interessent, int fl)
{
    if(flags&MONEY_NOT_DEKLIN)
	return query_cap_name();
}

object split_object(int i)
{
    object ob;

    if ((ob = ::split_object(i)) && ob != this_object())
    {
	ob->set_plural_name(query_plural_name());
	ob->set_singular_name(query_singular_name());
	ob->set_gender(query_gender());
	ob->set_id(query_id());
	ob->set_plural_id(query_plural_id());
	ob->set_count_type(query_count_type());
	ob->set_extra_long(query_extra_long());
	ob->set_money_flags(flags);
    }
    return ob;
}

varargs void create(int wert, string valuta)
{
    ::create();
    set_material("edelmetall");
    set_valuta("taler");
    set_valutas("taler");
    set_gender("maennlich");
    set_class_id("geld");

    if (valuta)
        init_money(wert, valuta);
}

void init()
{
    add_action("schnippe_command", "wirf");
    add_action("schnippe_command", "werfe", -4);
    add_action("schnippe_command", "schnippe", -7);
    
    if ((environment() == this_player()) && wizp(this_player()))
	add_action("geld", "geld");
}

int geld(string str)
{
    string kommando;
    int wert;
    string alt;
    
    notify_fail("geld valuta  <valuta>\n"
		"     valutas <valutas>\n"
		"     wert    <wert>\n\n"
		"geld <wert> <valuta/s>\n");
    if (!str)
	return 0;
    if (sscanf(str,"%d %s", wert, str) == 2)
    {
        alt=einen(TO);
	init_money(wert, str);
        TP->send_message_to(TP, MT_NOTIFY, MA_UNKNOWN, wrap(
            "Du hast "+alt+" in "+einen(TO)+" umgewandelt."
            ));
	return 1;
    }
    if (sscanf(str, "%s %s", kommando, str) != 2) 
	return 0;
    switch(kommando)
    {
	case "valuta":
            alt=einem(TO);
	    set_valuta(str);
            TP->send_message_to(TP, MT_NOTIFY, MA_UNKNOWN, wrap(
                "Du hast die Singular-Währung von "+alt+
                " in \""+str+"\" geändert."
                ));
	    return 1;
	case "valutas":
            alt=einem(TO);
	    set_valutas(str);
            TP->send_message_to(TP, MT_NOTIFY, MA_UNKNOWN, wrap(
                "Du hast die Plural-Währung von "+alt+
                " in \""+str+"\" geändert."
                ));
	    return 1;
	case "wert" :
	    if (sscanf(str,"%d",wert) != 1)
		return 0;
            alt=einem(TO);
	    set_money(wert);
            TP->send_message_to(TP, MT_NOTIFY, MA_UNKNOWN, wrap(
                "Du hast die Anzahl von "+alt+" in "+wert+" geändert."
                ));
	    return 1;
	default:
	    return 0;
    }
}

int schnippe_command(string str)
{
    string rest;
    int schnipp;
    mapping muenze;
    
    schnipp = (query_verb()[0..6]=="schnipp");
    
    if(schnipp && str) sscanf(str, "mit %s", str);

    if(!(rest=me(str)))
  	return notify_fail( schnipp
	    ? "Schnippe womit?\n"
	    : "Wirf was (nach wem)?\n", FAIL_NOT_OBJ, !schnipp);

    if(!schnipp && rest[0..4]=="nach ")
	return notify_fail( plural("Diese Münze ist ","Diese Münzen sind ") +
	    "doch zum Werfen viel zu wertvoll.\n", FAIL_WRONG_ARG);

    if (strlen(rest))
	return notify_fail(schnipp ? "Schnippe womit?\n" :
		"Wirf was (nach wem)?\n", FAIL_WRONG_ARG, 1);

    muenze = (["name":query_valuta(), "gender":query_gender()]);
    schnipp = random(2);
    this_player()->send_message(MT_LOOK, MA_USE,
	wrap(Der(this_player()) + " schnippt " + einen( muenze ) + " hoch " +
	"in die Luft, fängt " + ihn( muenze ) + " geschickt mit der " +
	"rechten Hand wieder ein, klatscht " + ihn( muenze ) + " auf den " +
	"Handrücken " + wessen((["name":"hand", "gender":"weiblich"]),
	    ART_NUR_SEIN, 0, this_player()) +
	" Linken und zeigt "+ihn( muenze )+" dir: "+
	({"Du erkennst eine Zahl darauf.",
	  "Das sieht ganz nach Kopf aus."})[schnipp]),
	wrap("Du schnippst " + einen( muenze ) + " hoch in die Luft, "
	"fängst " + ihn( muenze ) + " geschickt mit der rechten Hand " +
	"wieder ein und klatschst " + ihn( muenze ) + " auf den " +
	"Handrücken Deiner Linken. " + ({"Zahl!","Kopf!"})[schnipp]),
	this_player());
    this_player()->set_handeln();
    return 1;
}
