// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/i/object/countob.c
// Description: Objekt, das sich teilen laesst
// Author:	Freaky	(24.11.95)
// Modified By:	Freaky	(26.11.95) singular_name, personal_title, split_object
//		Freaky	(24.02.96) query_enable_cleanup()
//              Gnomi   (04.01.01) Ausgabe der Anzahl ueber set_menge geregelt

#pragma save_types
#pragma strong_types

virtual inherit "/i/item";
virtual inherit "/i/move";

#include <deklin.h>
#include <description.h>
#include <move.h>

// private int init_countob();

#ifndef AUTO_COUNTOB
private int count, transaction_count;
#else
private int count, transaction_count, already_moved;
#endif /* AUTO_COUNTOB */
private string plural_name, singular_name, count_type;
private object ancestor;

/*
FUNKTION: set_plural_name
DEKLARATION: void set_plural_name(string str)
BESCHREIBUNG:
Setzt den Plural-Namen eines Count-Objektes.
VERWEISE: set_singular_name, query_singular_name, set_plural_name, query_plural_name
GRUPPEN: countob
*/

void set_plural_name(string str)
{
    plural_name = str;
    add_setter_conservation("set_plural_name",({str}) );
    if (count > 1)
        set_name(plural_name);
}

/*
FUNKTION: query_plural_name
DEKLARATION: string query_plural_name()
BESCHREIBUNG:
Liefert den Plural-Namen eines Count-Objektes.
VERWEISE: set_singular_name, query_singular_name, set_plural_name, query_plural_name
GRUPPEN: countob
*/

string query_plural_name() { return plural_name; }

/*
FUNKTION: set_singular_name
DEKLARATION: void set_singular_name(string str)
BESCHREIBUNG:
Setzt den Singular-Namen eines Count-Objektes.
VERWEISE: set_singular_name, query_singular_name, set_plural_name, query_plural_name
GRUPPEN: countob
*/

void set_singular_name(string str)
{
    singular_name=str;
    add_setter_conservation("set_singular_name",({str}) );
    if (count == 1)
        set_name(singular_name);
}

/*
FUNKTION: query_singular_name
DEKLARATION: string query_singular_name()
BESCHREIBUNG:
Liefert den Singular-Namen eines Count-Objektes.
VERWEISE: set_singular_name, query_singular_name, set_plural_name, query_plural_name
GRUPPEN: countob
*/

string query_singular_name() { return singular_name; }

/*
FUNKTION: set_count_type
DEKLARATION: void set_count_type(string str)
BESCHREIBUNG:
Setzt den Count-Type eines Count-Objektes.

Der Count-Type entscheidet darueber, ob zwei Count-Objekte zu einem
verschmolzen werden, wenn eines in die Umgebung des anderen bewegt wird.
Deshalb muessen Count-Types in UNItopia einmalig sein.

Beispiel:
Moechte man einen eigenen Pfeil mit einem Extra-Damage bauen, so darf man
ihm nicht den Count-Type "pfeil" geben, sonst werden die guten Pfeile zu
normalen oder umgekehrt, wenn man zwei entsprechende Pfeil-Haufen in einen
Container legt.

Man waehle einen moeglichst eindeutigen Count-Type, es empfiehlt sich zB.
der Pfad der Datei/Load-Name:

        set_count_type(load_name());
VERWEISE: set_count, add_count, query_count, query_transaction_count, set_count_type, query_count_type
GRUPPEN: countob
*/

void set_count_type(string str) 
{
    count_type=str;
    add_setter_conservation("set_count_type",({str}) );
}

/*
FUNKTION: query_count_type
DEKLARATION: string query_count_type()
BESCHREIBUNG:
Liefert den Count-Type eines Count-Objektes.
VERWEISE: set_count, add_count, query_count, query_transaction_count, set_count_type, query_count_type
GRUPPEN: countob
*/

string query_count_type() { return count_type; }

/*
FUNKTION: query_count_weight
DEKLARATION: int query_count_weight(int count)
BESCHREIBUNG:
Liefert das Gewicht eines Count-Objektes mit der Anzahl count. Diese
Funktion wird intern verwendet, um bei einer Aenderung der Anzahl das
Gewicht neu zu setzen.

Man kann diese Funktion ueberlagern, wenn man fuer sein Count-Objekt andere
Gewichte haben moechte.
VERWEISE:
GRUPPEN: countob
*/

int query_count_weight(int c)
{
    if (c < 500)
	return 1;
    if (c < 1000)
	return 2;
    if (c < 5000)
	return 3;
    if (c < 10000)
	return 4;
    return 5 + c / 20000;
}

void update_weight()
{
    int weight;

    weight = query_count_weight(count);
    if (weight != query_weight())
	set_weight(weight);
}

/*
FUNKTION: set_count
DEKLARATION: void set_count(int m)
BESCHREIBUNG:
Setzt die Anzahl eines Count-Objektes.

Ist die Anzahl kleiner als 1, so wird das Objekt zerstoert.
VERWEISE: set_count, add_count, query_count, query_transaction_count, set_count_type, query_count_type
GRUPPEN: countob
*/

void set_count(int m)
{
    if (m > 0)
    {
        count = m;
        if (count > 1)
        {
            set_plural(1);
            set_name(plural_name);
            set_menge(({to_string(count),PRON_NICHT_DEKLIN}));
            update_weight();
        }
        else
        {
            set_plural(0);
            set_name(singular_name);
                set_menge(({"ein",PRON_NICHT_NACH_BEST}));
            update_weight();
        }
        add_setter_conservation("set_count",({count}) );
    }
    else
    {
        remove();
    }
}

/*
FUNKTION: add_count
DEKLARATION: void add_count(int m)
BESCHREIBUNG:
Veraendert die Anzahl eines Objektes.

Ist die verbleibende Anzahl kleiner als 1, so wird das Objekt zerstoert.
VERWEISE: set_count, add_count, query_count, query_transaction_count, set_count_type, query_count_type
GRUPPEN: countob
*/

void add_count(int m) { set_count(count + m); }

/*
FUNKTION: query_count
DEKLARATION: int query_count()
BESCHREIBUNG:
Liefert die Anzahl eines Objektes.

Diese Funktion dient auch dazu, ein Count-Objekt als solches zu erkennen.
Liefert query_count() bei einem beliebigen Objekt einen Wert ungleich 0,
so handelt es sich um ein Count-Objekt.
VERWEISE: set_count, add_count, query_count, query_transaction_count, set_count_type, query_count_type
GRUPPEN: countob
*/

int query_count() { return count; }

/*
FUNKTION: query_transaction_count
DEKLARATION: int query_transaction_count()
BESCHREIBUNG:
Wird ein Count-Objekt A in die Umgebung eines zweiten Count-Objektes B
mit demselben Count-Type bewegt, so wird das nicht bewegte Objekt B
zerstoert und die Anzahl von A entsprechend erhoeht. Die urspruengliche
Anzahl von A erhaelt man mit der Funktion query_transaction_count().
VERWEISE: set_count, add_count, query_count, query_transaction_count, set_count_type, query_count_type
GRUPPEN: countob
*/

int query_transaction_count() { return transaction_count; }

void create()
{
    set_count(1);
}

void set_ancestor(object ob) { ancestor=ob; }

/*
FUNKTION: query_ancestor
DEKLARATION: object query_ancestor()
BESCHREIBUNG:
Liefert das Count-Objekt zurueck, von dem dieses Count-Objekt abgespaltet
wurde.
VERWEISE: split_object, query_ancestor
GRUPPEN: countob
*/

object query_ancestor() { return ancestor; }

protected string query_long_exec(mapping info)
{
    // Leere Beschreibungen zulassen.
    return item::query_long_exec(info) || "";
}

protected string query_long_postprocess(string msg, mapping info)
{
    msg = item::query_long_postprocess(msg, info);
    
    if(!query_long_has_tag(T_ATOM_TAG_SHORT_DESC))
	msg = query_short(info[TI_VIEWER]) + ".\n" + msg;

    return msg;
}

int id(string str)
{
    if(!str) return 0;
    sscanf(str,"%~d %s",str);
    return ::id(str) || plural_id(str);
}

#ifndef AUTO_COUNTOB
void just_moved()
{
    object *obs;
    int a, tmp, siz;

    transaction_count = count;
    obs = all_inventory(environment())-({ this_object() });
    for (siz=sizeof(obs); a<siz; a++)
	if ((tmp=obs[a]->query_count()) &&
		obs[a]->query_count_type()==count_type)
	{
            obs[a]->notify("joined_countob", obs[a], this_object());
	    this_object()->notify("incorporated_countob", obs[a], this_object());
	    obs[a]->remove();
            add_count(tmp);
	    return;
	}
}
#else
int undo_split()
{
    if(!already_moved && query_ancestor() &&
	present(query_ancestor(), environment()) &&
	query_ancestor()->query_count_type() == count_type)
    {
	// Ein rueckgaengiggemachter Split
	query_ancestor()->join_object(this_object());
	return 1;
    }
}

void delayed_undo_split()
{
    if(find_call_out("undo_split")<0)
	call_out("undo_split", 0);
}

void join_object(object ob)
{
    int add = ob->query_count();
    
    ob->notify("joined_countob", ob, this_object());
    this_object()->notify("incorporated_countob", ob, this_object());
    ob->remove();
    
    add_count(add);
}

void join_objects()
{
    if(!undo_split())
    {
	object next_ob;
	
	transaction_count = count;
	
	for(object ob = first_inventory(environment()); ob; ob = next_ob)
	{
	    next_ob = next_inventory(ob); // Weil ob zerstoert werden koennte.
	    if(ob != this_object() && ob->query_count() &&
		ob->query_count_type() == count_type &&
		!ob->undo_split() &&
		!environment()->forbidden("join_countob", ob, this_object()) &&
		!ob->forbidden("join_countob", ob, this_object()) &&
		!this_object()->forbidden("join_countob", ob, this_object()))
	    {
		join_object(ob);
		return;
	    }
	}
    }
}

private void notify_move_countob(string controller, mapping mv_infos)
{
    if(!(mv_infos[MOVE_FLAGS]&MOVE_ATOM_NOT_NOTIFY))
    {
	already_moved = 1;
	join_objects();
    }
}

void notify(string message, varargs mixed * data)
{
    switch(message)
    {
	case "moved":
	    apply(#'notify_move_countob, "notify_moved", data...);
	    break;
    }
    
// Wegen Bug im Driver:
#if __VERSION__ > "3.3"
    ::notify(message, data...);
#else
    apply(#'::notify, message, data);
#endif
}

void just_moved() // Aus Kompatibilitaetsgruenden fuer ::just_moved()
{
}
#endif /* AUTO_COUNTOB */

/*
FUNKTION: forbidden_join_countob
DEKLARATION: int forbidden_join_countob(object alt, object neu)
BESCHREIBUNG:
Es wird nacheinander im Environment von alt und neu, dann in alt und zum
Schluss in neu forbidden("join_countob",alt,neu) aufgerufen und bei einem
Rueckgabewert != 0 werden die count_objekte alt und neu nicht zusammengefuehrt.
VERWEISE: notify_joined_countob
GRUPPEN: countob
*/

/*
FUNKTION: notify_joined_countob
DEKLARATION: void notify_joined_countob(object alt, object neu)
BESCHREIBUNG:
Wird ein Count-Objekt neu in die Umgebung eines zweiten Count-Objektes alt
mit demselben Count-Type bewegt, so wird das nicht bewegte Count-Objekt
alt zerstoert und die Anzahl von neu entsprechend erhoeht.

Kurz vor der Zerstoerung des Count-Objektes alt wird

    alt->notify("joined_countob", alt, neu)

aufgerufen.

In allen Objekten ob, die sich vorher mittels

    alt->add_controller("notify_joined_countob", ob)

im Count-Objekt alt als Controller fuer "notify_joined_countob"
angemeldet haben, wird dann

    notify_joined_countob(alt, neu)

aufgerufen.

Es wird ausserdem neu->notify("incorporated_countob", alt, neu) aufgerufen.
VERWEISE: split_object, query_ancestor, notify_incorporated_countob,
          notify_countob_split_from, notify_countob_split_into,
	  notify, add_controller
GRUPPEN: countob
*/

/*
FUNKTION: notify_incorporated_countob
DEKLARATION: void notify_incorporated_countob(object alt, object neu)
BESCHREIBUNG:
Wird ein Count-Objekt neu in die Umgebung eines zweiten Count-Objektes alt
mit demselben Count-Type bewegt, so wird das nicht bewegte Count-Objekt
alt zerstoert und die Anzahl von neu entsprechend erhoeht.

Unmittelbar vor der Zerstoerung des Count-Objektes alt wird

    neu->notify("incorporated_countob", alt, neu)

aufgerufen.

In allen Objekten ob, die sich vorher mittels

    neu->add_controller("notify_incorporated_countob", ob)

im Count-Objekt alt als Controller fuer "notify_incorporated_countob"
angemeldet haben, wird dann

    notify_incorporated_countob(alt, neu)

aufgerufen.

Vor diesem Controller wird ausserdem alt->notify("joined_countob", alt, neu)
aufgerufen.
VERWEISE: split_object, query_ancestor, notify_joined_countob,
          notify_countob_split_from, notify_countob_split_into,
	  notify, add_controller
GRUPPEN: countob
*/

/*
FUNKTION: split_object
DEKLARATION: object split_object(int i)
BESCHREIBUNG:
Teilt ein Count-Objekt und liefert ein neues Count-Objekt mit der Anzahl i
zurueck. Die Anzahl des alten Count-Objektes ist entsprechend verkleinert.

Ist i groesser oder gleich der Anzahl des alten Count-Objektes, so wird
stattdessen das alte zurueck geliefert.
VERWEISE: split_object, query_ancestor, notify_countob_split_from, c
          notify_countob_split_into
GRUPPEN: countob
*/

object split_object(int i)
{
    object ob;

    if (i > 0)
    {
	if (i >= count)
	    return this_object();

	ob = clone_object(explode(object_name(),"#")[0]);
	ob->set_ancestor(this_object());
	ob->set_count(i);
	add_count(-i);
        this_object()->notify("countob_split_from", this_object(), ob);
        ob->notify("countob_split_into", this_object(), ob);
#ifdef AUTO_COUNTOB
	ob->move(environment(), ([ MOVE_FLAGS:MOVE_FORCE|MOVE_SECRET]) );
#endif /* AUTO_COUNTOB */
	return ob;
    }
}

/*
FUNKTION: notify_countob_split_from
DEKLARATION: void notify_countob_split_from(object alt, object neu)
BESCHREIBUNG:
Wird von einem Count-Objekt alt mittels split_object() ein Count-Onjekt neu
abgespaltet, so wird anschliessend im Count-Objekt alt

    alt->notify("countob_split_from", alt, neu)

aufgerufen.

In allen Objekten ob, die sich vorher mittels

    alt->add_controller("notify_countob_split_from", ob)

im Count-Objekt alt als Controller fuer "notify_countob_split_from"
angemeldet haben, wird dann

    ob->notify_countob_split_from(alt, neu)

aufgerufen.

Hinweis: Der Vorgang des Abspaltens ist in dem Moment, da
notify_countob_split_from() aufgerufen wird, insofern noch nicht vollstaendig
abgeschlossen, als noch nicht alle relevanten Werte des Count-Objektes alt
im Count-Objekt neu gesetzt sein muessen. (Dies ist zum Beispiel bei
multi_ob's oder anderen Count-Objekten, welche split_object ueberlagern,
der Fall.)

Es wird ausserdem noch neu->notify("countob_split_into", alt, neu)
aufgerufen.
VERWEISE: split_object, query_ancestor, notify_joined_countob,
          notify_incorporated_countob, notify_countob_split_into,
	  notify, add_controller
GRUPPEN: countob
*/

/*
FUNKTION: notify_countob_split_into
DEKLARATION: void notify_countob_split_into(object alt, object neu)
BESCHREIBUNG:
Wird von einem Count-Objekt alt mittels split_object() ein Count-Onjekt neu
abgespaltet, so wird anschliessend im neuen Count-Objekt neu

    neu->notify("countob_split_into", alt, neu)

aufgerufen.

In allen Objekten ob, die sich vorher mittels

    neu->add_controller("notify_countob_split_into", ob)

im Count-Objekt alt als Controller fuer "notify_countob_split_into"
angemeldet haben, wird dann

    ob->notify_countob_split_into(alt, neu)

aufgerufen.

Da dieser Controller kurz nach dem Clonen des neuen Objektes aufgerufen wird,
wird sich hoechstwahrscheinlich nur das Objekt selbst als Controller
anmelden koennen.

Hinweis: Der Vorgang des Abspaltens ist in dem Moment, da
notify_countob_split_into() aufgerufen wird, insofern noch nicht vollstaendig
abgeschlossen, als noch nicht alle relevanten Werte des Count-Objektes alt
im Count-Objekt neu gesetzt sein muessen. (Dies ist zum Beispiel bei
multi_ob's oder anderen Count-Objekten, welche split_object ueberlagern,
der Fall.)

Es wird ausserdem noch alt->notify("countob_split_from", alt, neu)
aufgerufen.
VERWEISE: split_object, query_ancestor, notify_joined_countob,
          notify_incorporated_countob, notify_countob_split_from,
	  notify, add_controller
GRUPPEN: countob
*/

int query_enable_cleanup() { return 1; }
