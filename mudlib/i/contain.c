// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/i/contain.c
// Description: Inherit fuer Container
// Modified by:	Freaky (21.04.97) add_enc() nur noch bei gewicht > 0 testen
//              Sissi  (11./13.05.98) set/query_put_verb,
//                                    set/query_put_verb_case
//		Freaky (01.06.1998) propagate_message_to_env und message_filter
//		Freaky (16.02.2000) Defaults fuer put_verb, put_verb_case,
//			take_prepos, put_prepos eingebaut.
//		Freaky (17.02.2000) Defaults wieder raus, da sonst die Annamhe
//			des Verbes abhaengig vom Kommando nicht klappt
//		Freaky (01.03.2000) add_encumbrance() verbessert

/*
FUNKTIONSWEISEN: /doc/funktionsweisen/container
*/

#pragma save_types

#include <move.h> 
#include <message.h> 
#include <deklin.h> 

static private int internal_encumbrance;
static private int is_closed;
private int max_internal_encumbrance, min_weight, max_weight;
private int transparent;
private string content_message;
private int message_filter;
static private closure allow_only_msg;
static private string take_prepos;
static private string put_prepos;
static private mixed put_verb;
static private int put_verb_case;
static private string *only;

varargs string closure_to_string(closure c, mixed *args);

/*
FUNKTION: set_min_weight
DEKLARATION: void set_min_weight(int min_weight)
BESCHREIBUNG:
Damit kann man das Mindestgewicht eines Containers setzen.
Dieses Gewicht hat der Container, wenn er leer ist und sich nicht
direkt in einem Lebewesen befindet.
(Achtung: query_weight liefert nicht das berechnete Gewicht zwischen
Mindest- und Maximalgewicht, sondern das Gewicht, wenn der Container sich
direkt in einem Lebewesen befindet.)
VERWEISE: set_max_weight, query_min_weight, query_weight, query_collapsible,
	  query_max_internal_encumbrance, query_internal_encumbrance
GRUPPEN: taschen
*/
void set_min_weight(int mw)
{
    min_weight=mw;
    this_object()->add_setter_conservation("set_min_weight",({min_weight}));
}

/*
FUNKTION: query_min_weight
DEKLARATION: int query_min_weight()
BESCHREIBUNG:
Liefert das mit set_min_weight gesetzte Mindestgewicht des Containers.
VERWEISE: query_max_weight, set_min_weight, query_weight, query_collapsible,
	  query_max_internal_encumbrance, query_internal_encumbrance
GRUPPEN: taschen
*/
int query_min_weight() {return min_weight;}

/*
FUNKTION: set_max_weight
DEKLARATION: void set_max_weight(int max_weight)
BESCHREIBUNG:
Damit kann man das Maximalgewicht eines Containers setzen.
Dieses Gewicht hat der Container, wenn er voll ist und sich nicht
direkt in einem Lebewesen befindet.
(Achtung: query_weight liefert nicht das berechnete Gewicht zwischen
Mindest- und Maximalgewicht, sondern das Gewicht, wenn der Container sich
direkt in einem Lebewesen befindet.)
VERWEISE: set_min_weight, query_max_weight, query_weight, query_collapsible,
	  query_max_internal_encumbrance, query_internal_encumbrance
GRUPPEN: taschen
*/
void set_max_weight(int mw)
{
    max_weight=mw;
    this_object()->add_setter_conservation("set_max_weight",({max_weight}));
}

/*
FUNKTION: query_max_weight
DEKLARATION: int query_max_weight()
BESCHREIBUNG:
Liefert das mit set_max_weight gesetzte Maximalgewicht des Containers.
VERWEISE: query_min_weight, set_max_weight, query_weight, query_collapsible,
	  query_max_internal_encumbrance, query_internal_encumbrance
GRUPPEN: taschen
*/
int query_max_weight() {return max_weight;}

/*
FUNKTION: set_max_internal_encumbrance
DEKLARATION: void set_max_internal_encumbrance(int max)
BESCHREIBUNG:
Mit dieser Funktion setzt man die maximale Traglast eines Behaelters
z.B. einer Tasche. Wenn sie auf 0 gesetzt wird, kann man unendlich
viel in das Objekt legen.
Diese Funktion ruft auch set_min_weight und set_max_weight auf.
VERWEISE: query_internal_encumbrance, query_max_internal_encumbrance,
	  test_add_encumbrance, set_collapsible, query_collapsible
GRUPPEN: taschen
*/
void set_max_internal_encumbrance(int max)
{
    if (max < 0)
	max = 0;
    max_internal_encumbrance = max;
    this_object()->add_setter_conservation("set_max_internal_encumbrance",
        ({max_internal_encumbrance}));
    if(max)
    {
        set_max_weight(max);
        if(!this_object()->query_collapsible())
            set_min_weight(max);
    }
}

/*
FUNKTION: query_max_internal_encumbrance
DEKLARATION: int query_max_internal_encumbrance()
BESCHREIBUNG:
Mit dieser Funktion kann man die maximale Traglast eines Behaelters
z.B. einer Tasche abfragen.
VERWEISE: query_internal_encumbrance, set_max_internal_encumbrance,
	  test_add_encumbrance, set_collapsible, query_collapsible
GRUPPEN: taschen
*/
int query_max_internal_encumbrance() { return max_internal_encumbrance; }

/*
FUNKTION: query_internal_encumbrance
DEKLARATION: int query_internal_encumbrance()
BESCHREIBUNG:
Mit dieser Funktion kann man die momentane Traglast eines Behaelters
z.B. einer Tasche abfragen, also, wieviel gerade drin ist.
Die Maximallast erfragt man mit query_max_internal_encumbrance().
VERWEISE: query_max_internal_encumbrance, set_max_internal_encumbrance,
	  test_add_encumbrance, set_collapsible, query_collapsible
GRUPPEN: taschen
*/
int query_internal_encumbrance() { return internal_encumbrance; }

/*
FUNKTION: query_container
DEKLARATION: int query_container()
BESCHREIBUNG:
Wenn diese Funktion 1 returned, ist das Objekt ein Behaelter z.B. eine Tasche.
GRUPPEN: taschen
*/
int query_container() { return 1; }

/*
FUNKTION: query_con_close
DEKLARATION: int query_con_close()
BESCHREIBUNG:
Wenn diese Funktion 1 returned, ist der Behaelter zu, bei 0 ist er offen.
VERWEISE: close_con, open_con
GRUPPEN: taschen
*/
int query_con_close() { return is_closed; }

/*
FUNKTION: open_con
DEKLARATION: void open_con()
BESCHREIBUNG:
Mit dieser Funktion oeffnet man den Behaelter.
VERWEISE: query_con_close, close_con
GRUPPEN: taschen
*/
void open_con() { is_closed = 0; }

/*
FUNKTION: close_con
DEKLARATION: void close_con()
BESCHREIBUNG:
Mit dieser Funktion schliesst man den Behaelter.
VERWEISE: query_con_close, open_con
GRUPPEN: taschen
*/
void close_con() { is_closed = 1; }

/*
FUNKTION: add_encumbrance
DEKLARATION: int add_encumbrance(object ob, int enc_type, int enc_diff)
BESCHREIBUNG:

  ACHTUNG: add_encumbrance DARF NICHT DIREKT AUFGERUFEN WERDEN.
  ZUM TESTEN GIBT ES test_add_encumbrance!
  
Damit wird der Platz, den das Objekt ob in diesem Container wegnimmt,
veraendert. Folgende Werte (Defines in move.h) fuer enc_type sind moeglich:

  ENC_ADD         Gewicht von ob wird dazuaddiert.
  ENC_REMOVE      Gewicht von ob wird abgezogen.
  ENC_MODIFY      Gewicht von ob aendert sich um enc_diff.
  
  ENC_TEST        Es findet keine Aktion statt, sondern nur ein Test.
  ENC_TEST_ADD    Testen, ob das Objekt ob in diesen Container passt.
  ENC_TEST_REMOVE Testen, ob das Objekt ob aus diesem Container entfernt
                  werden kann.
  ENC_TEST_MODIFY Testen, ob das Objekt ob sein Gewicht um enc_diff Einheiten
                  veraendern darf.

Bei Erfolg liefert add_encumbrance einen Wert !=0.
VERWEISE: test_add_encumbrance,
          query_internal_encumbrance, query_max_internal_encumbrance,
          set_weight, query_weight
GRUPPEN: taschen
*/
int add_encumbrance(object ob, int enc_type, int enc_diff)
{
    int max_enc, enc, test_enc;

    if (!objectp(ob))
	raise_error("Invalid argument 1 to add_encumbrance()\n");

    if (enc_type & ENC_MODIFY)
	enc = enc_diff;
    else
    {
	enc = ob->query_weight();

	// Hier wird das enc anders berechnet bei Containern.
	if (ob->query_container() && !living(this_object()) && !living(ob))
	{
	    int tmp;

	    if (!ob->query_max_internal_encumbrance())
	    {
		tmp = min(ob->query_min_weight() + ob->query_internal_encumbrance(),
			  ob->query_max_weight());
#if 0
		if(enc_type & ENC_ADD)
		    sys_log("infiniteenc",
			sprintf("%O (%d,%d;%d) in %O\n", ob, 
			ob->query_min_weight(), ob->query_max_weight(),
			ob->query_internal_encumbrance(), this_object()));
#endif
	    }
	    else
	    {
		int minw = ob->query_min_weight();
		tmp = minw + (ob->query_max_weight() - minw + 1)
		    * ob->query_internal_encumbrance()
		    / (ob->query_max_internal_encumbrance() + 1);
	    }

	    if (tmp > enc)
		enc = tmp;
	}
	if (enc_type & ENC_REMOVE)
	    enc = -enc;
    }

    max_enc = query_max_internal_encumbrance();

    if((enc_type & ENC_ADD) && ob->query_count())
    {
	// Soso, ein Count-Ob. 
	// Wir achten nicht darauf, welches Gewicht die Countobs
	// jetzt haben, sondern haben wuerden, wenn es ein Countob waere.
	string ctype = ob->query_count_type();
	int ct_weight;
	int ct_count = ob->query_count();
	
	for(object inv = first_inventory(); inv; inv = next_inventory(inv))
	{
	    if(inv->query_count_type()==ctype)
	    {
		ct_count += inv->query_count();
		ct_weight += inv->query_weight();
	    }
	}
	
	test_enc = ob->query_count_weight(ct_count) - ct_weight;
	if(test_enc <= 0 && internal_encumbrance > max_enc)
	    // Wir sind ueberladen, also verhindern wir das.
	    test_enc = 1;
    }
    else
	test_enc = enc;

    if (max_enc && test_enc > 0 && test_enc + internal_encumbrance > max_enc &&
        !(enc_type & ENC_FORCE))
	return 0;

    if (enc != 0 && !living(this_object()) &&
        environment() && !living(environment()) &&
        query_min_weight() != query_max_weight())
    {
        // Änderung nach außen propagieren.
        int minw = query_min_weight();
        int maxw = query_max_weight();
        int maxe = query_max_internal_encumbrance();
        int w = this_object()->query_weight();
        int diff;

        if (!maxe)
        {
            diff = max(w, min(minw + internal_encumbrance + enc, maxw))
                 - max(w, min(minw + internal_encumbrance, maxw));
        }
        else
        {
            diff = max(w, (maxw - minw + 1) * (internal_encumbrance + enc) / (maxe + 1))
                 - max(w, (maxw - minw + 1) * internal_encumbrance / (maxe + 1));
        }

        if (diff && !environment()->add_encumbrance(this_object(), ENC_MODIFY|(enc_type & ENC_TEST), diff))
            return 0;
    }

    if (!(enc_type & ENC_TEST))
	internal_encumbrance += enc;
    return 1;
}

/*
FUNKTION: test_add_encumbrance
DEKLARATION: varargs int test_add_encumbrance(object ob, int enc_type, int enc_diff)
BESCHREIBUNG:
Damit wird getestet, ob man den Platz, den das Objekt ob in diesem Container
wegnimmt, veraendert werden darf. Das Objekt ob muss sich nicht in
diesem Container befinden (dann nimmt es halt keinen Platz ein).
Folgende Werte (Defines in move.h) fuer enc_type sind moeglich:

  ENC_ADD         Gewicht von ob wird dazuaddiert.
  ENC_REMOVE      Gewicht von ob wird abgezogen.
  ENC_MODIFY      Gewicht von ob aendert sich um enc_diff Gewichtseinheiten.

Wenn das moeglich ist, liefert test_add_encumbrance einen Wert !=0.

Beispiel:

Um zu testen, ob ich jemanden eine Fackel zustecken kann:
(Dieses Beispiel vermeidet nur ein Clonen mit anschliessender Zerstoerung.
Vom Ergebnis her wuerde die eine Zeile mit dem move genuegen.)

    object referenz = touch("/obj/fackel"); //Voellig identisch zum Clone
    object ziel = this_player();
    
    if( ziel->test_add_encumbrance(referenz, ENC_ADD) ) // Alles okay
	clone_object(referenz)->move(ziel, MOVE_ERR_REMOVE);

VERWEISE: query_internal_encumbrance, query_max_internal_encumbrance,
          set_weight, query_weight, set_collapsible, query_collapsible
GRUPPEN: taschen
*/
varargs int test_add_encumbrance(object ob, int enc_type, int enc_diff)
{
    return add_encumbrance(ob, enc_type|ENC_TEST, enc_diff);
}

/*
FUNKTION: set_transparent
DEKLARATION: void set_transparent(int flag)
BESCHREIBUNG:
Mit dieser Funktion kann man setzen, ob der Behaelter durchsichtig ist,
d.h. ob man sehen kann, was in ihm ist, obwohl er geschlossen ist.
VERWEISE: query_transparent
GRUPPEN: taschen
*/
void set_transparent(int flag)
{
    transparent = flag!=0;
    this_object()->add_setter_conservation("set_transparent",
        ({transparent}));
}

/*
FUNKTION: query_transparent
DEKLARATION: int query_transparent()
BESCHREIBUNG:
Mit dieser Funktion kann man abfragen, ob der Behaelter durchsichtig ist,
d.h. ob man sehen kann, was in ihm ist, obwohl er geschlossen ist.
VERWEISE: set_transparent
GRUPPEN: taschen
*/
int query_transparent() { return transparent; }

/*
FUNKTION: set_content_message
DEKLARATION: void set_content_message(string str)
BESCHREIBUNG:
Mit dieser Funktion setzt man die Meldung, die kommt, wenn man den Behaelter
anschaut, und er enthaelt etwas. Normal ist: 'Er enthaelt:'
VERWEISE: query_content_message
GRUPPEN: taschen
*/
void set_content_message(string str) 
{
    content_message = str;
    this_object()->add_setter_conservation("set_content_message",
        ({content_message}));
}

/*
FUNKTION: query_content_message
DEKLARATION: string query_content_message()
BESCHREIBUNG:
Mit dieser Funktion kann man die Meldung, die kommt, wenn man den Behaelter
anschaut, und er enthaelt etwas, abfragen.
VERWEISE: set_content_message
GRUPPEN: taschen
*/
string query_content_message() { return content_message; }

/*
FUNKTION: allow_only
DEKLARATION: varargs void allow_only(string *ids [, string message ] )
BESCHREIBUNG:
Mit dieser Funktion kann man setzten welche, Objekte in den Behaelter duerfen.
Es wird ein Array mit den Id's der Objekte, die rein duerfen, uebergeben.
Optional kann auch die Fehlermeldung angegeben werden, die ein Spieler
erhalten soll; ist diese nicht angegeben, so wird eine Meldung der Art
"In die Pralinenschachtel passt das Schwert nicht rein." erzeugt.
Eine selber gesetzte Meldung koennte "In die Pralinenschachtel passen nur 
Pralinen rein." sein.
"message" kann auch eine Pseudoclosure sein; dann kann beispielsweise mit
"$der('what)" auf das Objekt, welches reinbewegt werden soll, zugegriffen
werden.

Eine 0 als ids deaktiviert diese Funktion.
GRUPPEN: taschen
*/
varargs void allow_only(string *str, string msg)
{
    only = str;
    this_object()->add_setter_conservation("allow_only", ({str,msg}));
    allow_only_msg = msg ? mixed_to_closure(msg,({'what})) : 0;// '}));
}

/*
FUNKTION: query_allow_only
DEKLARATION: mixed *query_allow_only()
BESCHREIBUNG:
Diese Funktion liefert ein Array zurueck, das zuerst die Fehlermeldung 
(falls keine speziell angegeben wurde, ergibt das eine 0) beschreibt und 
dann eine Liste von Objekt-Ids enthaelt, in den Behaelter duerfen.
VERWEISE: allow_only
GRUPPEN: taschen
*/
mixed *query_allow_only() { return ({ allow_only_msg, only}); }

private int check_allow_only(object obj)
{
    int i;

    if(!only)
	return 0;
    for(i = 0; i<sizeof(only); i++)
	if(obj->id(only[i]))
	    return 0;
    if(allow_only_msg)
        obj->set_not_moved_reason(closure_to_string(allow_only_msg,({obj})));
    else
        obj->set_not_moved_reason("In "+den()+plural(" passt "," passen ",obj)+
	    der(obj)+" nicht rein.");
    return 1;
}

/*
FUNKTION: let_not_in
DEKLARATION: <int|string> let_not_in(mapping mv_infos)
BESCHREIBUNG:
Diese Funktion wird von /i/move::move() im Zielraum eines Moves aufgerufen,
um sich 'Erlaubnis' vom Zielraum einzuholen, ob ein move erlaubt ist. Wenn 
let_not_in einen anderen Wert als 0 zurueckliefert, wird der move nicht
ausgefuehrt. Man kann auch let_not_in dazu verwenden, irgendwelche Aktionen
auszufuehren, bevor ein Objekt in den Zielraum bewegt wird. Der Rueckgabewert 
string wird als Begruendung fuer den Fehlschlag an das bewegte Objekt 
zurueckgegeben. Die Parameter in mv_infos sind in notify_moved
erlaeutert.
VERWEISE: move, let_not_out, moved_in, set_not_moved_reason
GRUPPEN: move
*/
<int|string> let_not_in(mapping mv_infos)
{
   if(query_con_close())
      return MOVE_DEST_CLOSED;
   if(check_allow_only(mv_infos[MOVE_OBJECT]))
      return 1;
}

/*
FUNKTION: let_not_out
DEKLARATION: <int|string> let_not_out(mapping mv_infos)
BESCHREIBUNG:
Diese Funktion wird von /i/move::move() im Ursprungsraum aufgerufen, um sich 
'Erlaubnis' vom Ursprungsraum einzuholen, ob ein move erlaubt ist. Wenn 
let_not_out einen anderen Wert als 0 zurueckliefert, wird der move nicht 
ausgefuehrt.  Man kann auch let_not_out dazu verwenden, irgendwelche Aktionen 
auszufuehren, bevor ein Objekt aus dem Ursprungsraum entfernt wird. Der
Rueckgabewert string wird als Begruendung an das bewegte Objekt zurueckgegeben.
Die in mv_infos verfuegbaren Parameter sind in notify_moved_out erlaeutert.
VERWEISE: move, let_not_in, moved_out, set_not_moved_reason
GRUPPEN: move
*/
<int|string> let_not_out(mapping mv_infos)
{
   if(query_con_close())
      return MOVE_ENV_CLOSED;
}

/*
FUNKTION: moved_in
DEKLARATION: void moved_in(mapping mv_infos)
BESCHREIBUNG:
Diese Funktion wird von /i/move::move() im Zielraum des Moves aufgerufen,
und zwar NACH dem Move. Damit kann man in einem Container feststellen, dass
gerade etwas hereinbewegt wurde. Die in mv_infos verwendeten Parameter sind
in notify_moved erlaeutert.
VERWEISE: move, let_not_in, moved_out, just_moved
GRUPPEN: move
*/
void moved_in(mapping mv_infos)
{
}

/*
FUNKTION: moved_out
DEKLARATION: void moved_out(mapping mv_infos)
BESCHREIBUNG:
Diese Funktion wird von /i/move::move() im Ursprungsraum des Moves aufgerufen,
und zwar NACH dem Move. Damit kann man in einem Container feststellen, dass
gerade etwas herausbewegt wurde. Die in mv_infos verwendeten Parameter sind
in notify_moved erlaeutert.
VERWEISE: move, let_not_out, moved_in, just_moved
GRUPPEN: move
*/
void moved_out(mapping mv_infos)
{
    
}

/*
FUNKTION: set_take_prepos
DEKLARATION: void set_take_prepos(string s)
BESCHREIBUNG:
Mit dieser Funktion setzt man die Preposition, die man erhaelt,
wenn man etwas AUS dem Container nimmt.
Bei einem Tisch waere das beispielsweise "von".
VERWEISE: query_take_prepos, set_put_prepos, query_put_prepos
GRUPPEN: taschen
*/

void set_take_prepos(string s)
{
    take_prepos = s;
    this_object()->add_setter_conservation("set_take_prepos", ({take_prepos}));
}


/*
FUNKTION: query_take_prepos
DEKLARATION: string query_take_prepos()
BESCHREIBUNG:
Mit dieser Funktion fragt man die Preposition, die man erhaelt,
wenn man etwas AUS dem Container nimmt.
Bei einem Tisch waere das beispielsweise "von".
VERWEISE: set_take_prepos, set_put_prepos, query_put_prepos
GRUPPEN: taschen
*/

string query_take_prepos()
{
    return take_prepos;
}


/*
FUNKTION: set_put_prepos
DEKLARATION: void set_put_prepos(string s)
BESCHREIBUNG:
Mit dieser Funktion setzt man die Preposition, die man erhaelt,
wenn man etwas IN den Container legt.
Bei einem Tisch waere das beispielsweise "auf".
VERWEISE: query_take_prepos, set_take_prepos, query_put_prepos, set_put_verb
GRUPPEN: taschen
*/

void set_put_prepos(string s)
{
    put_prepos = s;
    this_object()->add_setter_conservation("set_put_prepos", ({put_prepos}));
}


/*
FUNKTION: query_put_prepos
DEKLARATION: string query_put_prepos()
BESCHREIBUNG:
Mit dieser Funktion fragt man die Preposition, die man erhaelt,
wenn man etwas IN den Container legt.
Bei einem Tisch waere das beispielsweise "auf".
VERWEISE: query_take_prepos, set_take_prepos, set_put_prepos, set_put_verb
GRUPPEN: taschen
*/

string query_put_prepos()
{
    return put_prepos;
}


/*
FUNKTION: set_put_verb
DEKLARATION: void set_put_verb(string s|string *s)
BESCHREIBUNG:
Mit dieser Funktion kann man das Verb oder die Verben setzen, das man erhält,
wenn man etwas in einen Container tut.
Bei einer Garderobe wäre das beispielsweise "häng", bei einem Tisch
({"leg","stell"}). (Werden mehrere angegeben, so ist das erste der
Standardwert, falls der Spieler ein ganz anderes Verb nutzte.)
Achtung, dieses Verb ist ohne "Endung"! Also "stell" statt "stelle" usw.;
wird keines angegeben, so wird "leg" verwendet.
VERWEISE: query_put_verb, query_take_prepos, set_take_prepos, query_put_prepos
GRUPPEN: taschen
*/

void set_put_verb(mixed s)
{
    put_verb = s;
    this_object()->add_setter_conservation("set_put_verb", ({put_verb}));
}


/*
FUNKTION: query_put_verb
DEKLARATION: mixed query_put_verb()
BESCHREIBUNG:
Mit dieser Funktion fragt man das Verb oder die Verben, und zwar ohne Endung,
das man erhaelt, wenn man etwas in den Container tut.
Bei einer Garderobe waere das beispielsweise "haeng".
VERWEISE: set_put_verb, query_take_prepos, set_take_prepos, set_put_prepos
GRUPPEN: taschen
*/

mixed query_put_verb()
{
    return put_verb;
}

/*
FUNKTION: set_put_verb_case
DEKLARATION: void set_put_verb_case(int c)
BESCHREIBUNG:
Mit dieser Funktion setzt man den Fall, welcher fuer das mit 
set_put_verb gesetzte Verb verwendet wird. Standardfall ist Akkusativ,
daher diese Funktion nur verwenden, wenn der Fall nicht Akkusativ ist.
Konstanten siehe /sys/deklin.h.
Beispiel: set_put_verb("verstau"); set_put_verb_case(FALL_DAT);
VERWEISE: query_put_verb, set_put_verb, query_put_verb_case
GRUPPEN: taschen
*/

void set_put_verb_case(int c)
{
    put_verb_case = c;
    this_object()->add_setter_conservation("set_put_verb_case", 
        ({put_verb_case}));
}


/*
FUNKTION: query_put_verb_case
DEKLARATION: int query_put_verb_case()
BESCHREIBUNG:
Mit dieser Funktion fragt man den Fall ab, der fuer das mit set_put_verb
gesetzte Verb verwendet werden soll, ab.
VERWEISE: set_put_verb, query_take_prepos, set_take_prepos, set_put_prepos
GRUPPEN: taschen
*/

int query_put_verb_case()
{
    return put_verb_case;
}

/*
FUNKTION: set_message_filter
DEKLARATION: void set_message_filter(int filter)
BESCHREIBUNG:
Mit dem Message-Filter legt man fest, welche Arten von Meldungen der
Container durchlaesst, wenn innerhalb des Containers eine Meldung erzeugt
wird. Diese Meldungen werden nur dann an das environment weitergegeben, wenn
die entsprechenden Bits fuer den Message-Typ im Message-Filter gesetzt sind.
Normalerweise sind keine Bits gesetzt, so dass er keine Meldungen durchlaesst.
VERWEISE: query_message_filter, send_message
GRUPPEN: message
*/
void set_message_filter(int filter)
{
    message_filter = filter;
    this_object()->add_setter_conservation("set_message_filter", 
        ({message_filter}));
}

/*
FUNKTION: query_message_filter
DEKLARATION: int query_message_filter()
BESCHREIBUNG:
Liefert den gesetzten Messagefilter.
Siehe set_message_filter
VERWEISE: set_message_filter, send_message
GRUPPEN: message
*/
int query_message_filter()
{
    return message_filter;
}

/*
FUNKTION: propagate_message_to_env
DEKLARATION: void propagate_message_to_env(int msg_type, int msg_action, object ob, string msg, string msg_whom, mixed whom)
BESCHREIBUNG:
Diese Funktion propagiert die Message an alle Objekte im environment()
Diese Funktion wird von send_message aufgerufen.
Es werden nur Meldungen des Typs weitergeleitet, die nicht von dem mit
set_message_filter gesetzten Filter, gefiltert werden.
VERWEISE: send_message, send_message_to, set_message_filter, receive_message
GRUPPEN: message
*/
void propagate_message_to_env(int msg_type, int msg_action, object ob,
	string msg, string msg_whom, mixed whom)
{
    // nur machen, wenn der Container nicht alle Message-Types der Meldung
    // filtert
    if (environment() && (msg_type & message_filter & MT_MASK))
    {
	if (pointerp(whom))
	    filter_objects(all_inventory(environment()) -
		    ({this_object()}) - whom,
		    "receive_message",msg_type,msg_action,ob,msg);
	else
	    filter_objects(all_inventory(environment()) -
		    ({this_object(),whom}),
		    "receive_message",msg_type,msg_action,ob,msg);
	environment()->propagate_message_to_env(msg_type,msg_action,
		ob,msg,msg_whom,whom);
	// Nur ans Env schicken, wenn es nicht (in) whom ist.
	if ((pointerp(whom) && member(whom,environment()) < 0) ||
		(!pointerp(whom) && whom != environment()))
	    environment()->receive_message(msg_type,msg_action,ob,msg);
    }
}
