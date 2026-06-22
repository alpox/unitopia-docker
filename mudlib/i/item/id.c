// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/i/item/id.c
// Description:

#pragma save_types
#pragma strong_types

#include <parse_com.h>

/* Die Arrays bestehen aus den mit set_xxx() bzw. add_xxx() gesetzten
 * Ids. Falls einige der IDs Umlaute beinhalten, so werden den gesetzten
 * Ids die Umschreibungen vorangestellt und mit einer 0 von den
 * ursprünglichen getrennt.
 */
private string *ids, *plural_ids, *class_ids;

/************************* INTERNES ****************************/
/* Die internen Funktionen stellen die Funktionalität für die
 * set/add/delete-Funktionen dar. Der erste Parameter ist stets
 * die jeweilige globale Variable, die per Referenz übergeben
 * werden soll.
 */

private void internal_set_id(string* ids_var, string setter, string|string* new_ids)
{
    string *umschreibungen;

    if (pointerp(new_ids))
        ids_var = new_ids;
    else if (stringp(new_ids))
        ids_var = ({ new_ids });
    else
        return;

    this_object()->add_setter_conservation(setter, ({ids_var}) );

    umschreibungen = map(ids_var, #'convert_umlaute) - ids_var;
    if (sizeof(umschreibungen))
        ids_var = umschreibungen + ({ 0 }) + ids_var;
}

private void internal_add_id(string* ids_var, string setter, string|string* new_ids)
{
    string *umschreibungen;
    int trenner;

    if (stringp(new_ids))
        new_ids = ({ new_ids });
    else if(!pointerp(new_ids))
        return;

    if (!ids_var)
        ids_var = ({});

    ids_var += new_ids;
    trenner = member(ids_var, 0);

    this_object()->add_setter_conservation(setter, ({ids_var[trenner+1..]}) );

    umschreibungen = map(new_ids, #'convert_umlaute) - new_ids;
    if (sizeof(umschreibungen))
    {
        if (trenner < 0)
            ids_var = umschreibungen + ({ 0 }) + ids_var;
        else
            ids_var[trenner..trenner-1] = umschreibungen;
    }
}

private void internal_delete_id(string* ids_var, string setter, string|string* strs)
{
    int trenner;

    if (!ids_var)
        return;

    if (stringp(strs))
        strs = ({ strs });
    else if(!pointerp(strs))
        return;

    trenner = member(ids_var, 0);
    if (trenner < 0)
    {
        ids_var -= strs;
        this_object()->add_setter_conservation(setter, ({ids_var}) );
    }
    else
    {
        internal_set_id(&ids_var, setter, ids_var[trenner+1..] - strs);
    }
}

private string* internal_query_id(string* ids_var)
{
    if (!ids_var)
        return ({});

    return ids_var[member(ids_var, 0)+1..];
}

/************************* PLURAL ****************************/

/*
FUNKTION: set_plural_id
DEKLARATION: void set_plural_id(string plural_id | string *plural_ids)
BESCHREIBUNG:
Hiermit koennen die Plurale der Ids angegeben werden, falls der Plural-
Generator in plural_id versagt. 
VERWEISE: id, me, add_id, delete_id, query_id, set_class_id,
          query_plural_id, add_plural_id, delete_plural_id
GRUPPEN: grundlegendes
*/
void set_plural_id(string|string* new_ids)
{
    internal_set_id(&plural_ids, "set_plural_id", new_ids);
}

/*
FUNKTION: query_plural_id
DEKLARATION: string *query_plural_id()
BESCHREIBUNG:
Hiermit kann man die Plurale der Ids abfragen. (wenn diese gesetzt sind)
VERWEISE: id, me, add_id, delete_id, query_id, set_plural_id, set_class_id
GRUPPEN: grundlegendes
*/
string *query_plural_id()
{
    return internal_query_id(plural_ids);
}

/*
FUNKTION: add_plural_id
DEKLARATION: void add_plural_id(string plural_id | string *plural_ids)
BESCHREIBUNG:
Hiermit koennen zu bereits vorhandenen Ids die Plurale von Ids hinzugefuegt
werden, falls der Plural-Generator in plural_id versagt. 
VERWEISE: id, me, add_id, delete_id, query_id, set_class_id,
	  set_plural_id, query_plural_id, delete_plural_id
GRUPPEN: grundlegendes
*/
void add_plural_id(string|string* new_ids)
{
    internal_add_id(&plural_ids, "set_plural_id", new_ids);
}

/*
FUNKTION: delete_plural_id
DEKLARATION: void delete_plural_id(string plural_id | string *plural_ids)
BESCHREIBUNG:
Hiermit koennen bereits existierende Plurale von Ids eines Objektes
geloescht werden. delete_id ist das Pendant zu add_plural_id.
VERWEISE: id, me, set_id, add_id, query_id, set_class_id,
	  set_plural_id, query_plural_id, add_plural_id
GRUPPEN: grundlegendes
*/
void delete_plural_id(string|string* strs)
{
    internal_delete_id(&plural_ids, "set_plural_id", strs);
}

/*
FUNKTION: plural_id
DEKLARATION: int plural_id(string id)
BESCHREIBUNG:
Gibt 1 zurueck, wenn der uebergebene String id die Plural-Form einer
der mit set_id, add_id oder set_class_id gestzten Ids entspricht, ansonsten 0.

VERWEISE: id, me, set_id, add_id, delete_id, query_id
GRUPPEN: grundlegendes
*/
int plural_id(string str)
{
    string geschlecht;
    string *to_check;

    if (!str)
        return 0;

    str = convert_umlaute(str);

    if (pointerp(plural_ids))
        return member(plural_ids, str) >= 0;

    to_check = ({});
    if (pointerp(ids))
       to_check += ids;
    if (pointerp(class_ids))
        to_check += class_ids;
    to_check -= ({0, ""});

    geschlecht = this_object()->query_gender();
    for (int a=0; a<sizeof(to_check); a++)
    {
	string endung, testid;
	int trunc, last;
	
	last = to_check[a][<1];
	if (member("bdfghmnstz",last) > -1)
	{
	    if (geschlecht == "maennlich")
		endung = "e";
	    else if (geschlecht == "weiblich")
		endung = "en";
	    else
		endung = "er";
	}
	else if (member("cijopquvwxy",last) > -1)
	    endung = "s";
	else if (last == 'a')
	{
	    if (geschlecht == "weiblich")
	    {
		trunc = 1;
		endung = "en";
	    }
	    else
		endung = "s";
	}
	else if (last == 'e')
	    endung = "n";
	else if (last == 'k')
	{
	    if (geschlecht == "weiblich")
		endung = "en";
	    else
		endung = "e";
	}
	else if (last == 'r')
	{
	    if (geschlecht == "maennlich")
		endung = "en";
	    else
		endung = "e";
	}
	else if (last == 'l')
	{
	    if (geschlecht == "maennlich")
		endung = "";
	    else if (geschlecht == "weiblich")
		endung = "n";
	    else
		endung = "e";
	}
	else /* Zahlen,Sonderzeichen */
	    endung = "s";

	if (trunc)
	   testid = to_check[a][0..<2]+endung;
	else
	   testid = to_check[a]+endung;
	if (testid == str ||
	    regreplace(testid, "([aou])(([aeiou]|)[^aeiouy]*(e|e[nr]))$","\\1e\\2",0) == str)
	    return 1;
    }
}


/************************* ID ****************************/

/*
FUNKTION: set_id
DEKLARATION: void set_id(string id | string *ids)
BESCHREIBUNG:
Hiermit werden die Ids eines Objektes gesetzt, bereits gesetzte Ids werden
ueberschrieben.

Ids sind die Namen eines Objektes, anhand derer es eindeutig identifiziert
werden kann. Die angegebenen Namen sollten NUR Kleinbuchstaben enthalten.

Die Mehrzahlform kann man mit set_plural_id setzen und die Klasse eines
Objektes (z.B. getraenk) mit set_class_id

Beispiel:   set_id( ({  "flasche", "bier", "bierflasche" }) );
VERWEISE: id, add_id, delete_id, query_id, me
GRUPPEN: grundlegendes
*/
void set_id(string|string* new_ids)
{
    internal_set_id(&ids, "set_id", new_ids);
}

/*
FUNKTION: add_id
DEKLARATION: void add_id(string id | string *ids)
BESCHREIBUNG:
Hiermit koennen zu bereits existierenden Ids eines Objektes weitere
hinzugefuegt werden. Das Pendant hierzu ist delete_id. Naeheres siehe id.
VERWEISE: id, me, set_id, delete_id, query_id
GRUPPEN: grundlegendes
*/
void add_id(string|string* str)
{
    internal_add_id(&ids, "set_id", str);
}

/*
FUNKTION: query_id
DEKLARATION: string *query_id()
BESCHREIBUNG:
Gibt ein Feld mit allen Ids des Objektes zurueck.
Naeheres siehe id.
VERWEISE: id, me, set_id, add_id, delete_id
GRUPPEN: grundlegendes
*/
string *query_id()
{
    return internal_query_id(ids);
}

/*
FUNKTION: delete_id
DEKLARATION: void delete_id(string id | string *ids)
BESCHREIBUNG:
Hiermit koennen bereits existierende Ids eines Objektes geloescht werden.
delete_id ist das Pendant zu add_id. Naeheres siehe id.
VERWEISE: id, me, set_id, add_id, query_id
GRUPPEN: grundlegendes
*/
void delete_id(string|string* strs)
{
    internal_delete_id(&ids, "set_id", strs);
}

/*
FUNKTION: id
DEKLARATION: int id(string id)
BESCHREIBUNG:
Gibt 1 zurueck, wenn die angegebene id mit einer der gesetzten Ids oder
Klassen-IDs uebereinstimmt, ansonsten 0.
VERWEISE: me, set_id, add_id, delete_id, query_id, set_plural_id,ids
GRUPPEN: grundlegendes
*/
int id(string str)
{
    if (!str)
        return 0;

    str = convert_umlaute(str);

    if (pointerp(ids) && member(ids, str) >= 0)
        return 1;

    if (pointerp(class_ids) && member(class_ids, str) >= 0)
        return 1;

    return 0;
}

/*
FUNKTION: id
DEKLARATION: int ids(string *ids)
BESCHREIBUNG:
Gibt 1 zurueck, wenn die angegebenen ids mit einer der gesetzten Ids oder
Klassen-IDs uebereinstimmt, ansonsten 0.
VERWEISE: me, set_id, add_id, delete_id, query_id, set_plural_id,id
GRUPPEN: grundlegendes
*/
int ids(string* str)
{
    if (!pointerp(str))
        return 0;

    str = map(str, #'convert_umlaute);

    if (pointerp(ids) && sizeof(ids & str) > 0)
        return 1;
    if (pointerp(class_ids) && sizeof(class_ids & str) > 0)
        return 1;

    return 0;
}

/**************************** CLASS ***********************/

/*
FUNKTION: set_class_id
DEKLARATION: void set_class_id(string class_id | string *class_ids)
BESCHREIBUNG:
Hiermit werden Klassen-Ids eines Objektes gesetzt, zB "waffe", "ruestung",...
Diese werden auch bei id() zur Ueberpruefung herangezogen.
VERWEISE: id, me, add_id, delete_id, query_id, set_id, query_class_id
GRUPPEN: grundlegendes
*/
void set_class_id(string|string* new_ids)
{
    internal_set_id(&class_ids, "set_class_id", new_ids);
}

/*
FUNKTION: query_class_id
DEKLARATION: string *query_class_id()
BESCHREIBUNG:
Hiermit kann man die Klassen-Ids eines Objektes abfragen.
VERWEISE: id, me, add_id, delete_id, query_id, set_id, set_class_id
GRUPPEN: grundlegendes
*/
string *query_class_id()
{
    return internal_query_id(class_ids);
}

/******************************* ME ***********************/

/*
FUNKTION: me
DEKLARATION: string me(string command)
BESCHREIBUNG:
Definiert ein Objekt eine add_action, zB eine Fackel 'zuende', so war es
praktisch unmoeglich, Kommandos wie zB 'zuende meine 2. fackel an' zu parsen;
hier fuer muesste ein umfangreicher parse_com(..) - Aufruf programmiert werden.
Dieser Aufruf ist jetzt in me(..) realisiert. me ruft intern parse_com auf und
gibt 0 zurueck, wenn nicht dieses Objekt gemeint ist, andernfalls einen String
mit dem Rest des Kommandos.

Fuer v_items gibt es die entsprechende Funktion here().

Beispiel:

   void init() {
       add_action("mach_licht","mache",-4);
   }

   int mach_licht(string str) {
	   string rest;

       if ( !(rest = me(str)) ) {
	   notify_fail("Mach was?\n");
	   return 0;
	   }
       switch(lower_case(rest)) {
	   case "an":
	       mach_licht_an();
	       break;
	   case "aus":
	       mach_licht_aus();
	       break;
	   default:
	       send_message_to(TP, MT_NOTIFY, MA_UNKNOWN, "Mache das licht an"
                                                          " oder aus?\n");
	   }
       return 1;
   }

VERWEISE: id, parse_com, set_id, set_plural_id, set_class_id, here
GRUPPEN: grundlegendes
*/
string me(string str)
{
    return id(str) ? "" : its_me(str,this_object());
}
