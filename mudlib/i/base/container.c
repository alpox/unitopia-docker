// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /i/base/container.c
// Description: Die Funktionalitaet eines Containers mit Tuer und Schloss
// Author:	Freaky (16.02.2000) extrahiert aus /i/object/kiste.c
// Modified by:	Freaky (07.05.2000) auf Message-System umgestellt
//              Sissi  (17.06.2000) an Schluesselbund angepasst

#pragma save_types
#pragma strong_types

inherit "/i/contain";

#include <move.h>
#include <deklin.h>
#include <message.h>
#include <notify_fail.h>
#include <description.h>

// Prototypes von /i/item.c
string query_long_string();
int query_long_has_tag(string tag);
string me(string str);
// Prototypes
void set_locked(int i);
int query_locked();
void set_no_lock(int i);
int query_no_lock();


private int locked, no_lock, no_door, crack, collapsible;
private string *keys;                  /* Schluessel, die zum Schloss passen */

private varargs void notify_msg(string str, object who)
{
    who = who || this_player();
    who->send_message_to(who,MT_NOTIFY,MA_UNKNOWN,str);
}

private varargs void send_msg(string str, object who)
{
    who = who || this_player();
    who->send_message(MT_LOOK,MA_UNKNOWN,str);
}

/*
FUNKTION: set_no_door
DEKLARATION: void set_no_door(int no_door)
BESCHREIBUNG:
Hiermit setzt man, ob der Container keine Tuer/keinen Deckel hat.
Wenn man keine Tuer setzt, wird der Container automatisch aufgeschlossen
und geoeffnet und das Schloss wird abgebaut.
VERWEISE: query_no_door, set_no_lock, open_con
GRUPPEN: taschen
*/
void set_no_door(int i)
{
    if (i)
    {
        if (query_locked())
            set_locked(0);
        if (!query_no_lock())
            set_no_lock(1);
        if (query_con_close())
            open_con();
        no_door = 1;
    }
    else
        no_door = 0;
    this_object()->add_setter_conservation("set_no_door", ({no_door}));
}

/*
FUNKTION: query_no_door
DEKLARATION: int query_no_door()
BESCHREIBUNG:
Hiermit kann man abfragen, ob der Container keine Tuer/keinen Deckel hat.
VERWEISE: set_no_door, set_no_lock, open_con
GRUPPEN: taschen
*/
int query_no_door() { return no_door; }


/*
FUNKTION: set_no_lock
DEKLARATION: void set_no_lock(int no_lock)
BESCHREIBUNG:
Hiermit setzt man, ob der Container kein Schloss hat.
Wenn der Container keine Tuer hat, hat er auch kein Schloss.
Das Schloss wird automatisch geoeffnet, wenn der Container
kein Schloss hat.
VERWEISE: query_no_lock, set_no_door, set_locked
GRUPPEN: taschen
*/
void set_no_lock(int i) 
{
    if (i)
    {
        if (query_locked())
            set_locked(0);
        no_lock = 1;
    }
    else
        no_lock = 0 || query_no_door();
    this_object()->add_setter_conservation("set_no_lock", ({no_lock}));
}

/*
FUNKTION: query_no_lock
DEKLARATION: int query_no_lock()
BESCHREIBUNG:
Hiermit kann man abfragen, ob der Container kein Schloss hat.
Der Container hat automatisch kein Schloss, wenn er keine Tuer hat.
VERWEISE: set_no_lock, set_no_door, set_locked
GRUPPEN: taschen
*/
int query_no_lock() { return no_lock; }

/*
FUNKTION: set_locked
DEKLARATION: void set_locked(int locked)
BESCHREIBUNG:
Hiermit setzt man, ob der Container abgeschlossen ist.
Hierbei muss man sich um alles (Tests, ob der Container geschlossen ist,
Controlleraufrufe etc.) selber kuemmern. Daher ist diese Funktion
nur zur Initialisierung ratsam. Ansonsten sollte man besser lock_con nutzen.
VERWEISE: query_locked, set_no_lock
GRUPPEN: taschen
*/
// Wenn das Teil kein Schloss hat, dann ist es auch nicht abgeschlossen
void set_locked(int i)
{
    locked = i && !query_no_lock();
    this_object()->add_setter_conservation("set_locked", ({locked}));
}

/*
FUNKTION: query_locked
DEKLARATION: int query_locked()
BESCHREIBUNG:
Hiermit kann man abfragen, ob der Container abgeschlossen ist.
Der Container ist automatisch aufgeschlossen, wenn er kein Schloss hat.
VERWEISE: set_locked, set_no_lock
GRUPPEN: taschen
*/
int query_locked() { return locked; }

void open_con()
{
    if (query_con_close())
    {
        if (query_locked())
            set_locked(0);
        ::open_con();
        this_object()->notify("open",this_object(),this_player());
    }
}

// Wenn das Teil keine Tuer hat, kann es auch nicht zu sein
void close_con()
{
    if (!query_no_door() && !query_con_close())
    {
        ::close_con();
        this_object()->notify("close",this_object(),this_player());
    }
}

/*
FUNKTION: notify_open
DEKLARATION: void notify_open(object container, object who)
BESCHREIBUNG:
Nachdem ein Container (welcher /i/base/container inheritet) von who geoeffnet
wurde, wird im Container notify("open",container,who) aufgerufen.
notify ruft dann in allen mit container->add_controller("notify_open",other)
angemeldeten Objekten other notify_open(container,who) auf.
Diese Objekte haben dann die Moeglichkeit, darauf zu reagieren.
VERWEISE: open_con, close_con, lock_con, forbidden_open,
	  add_controller, notify, notify_close, notify_lock, notify_unlock
GRUPPEN: taschen
*/

/*
FUNKTION: notify_close
DEKLARATION: void notify_close(object container, object who)
BESCHREIBUNG:
Nachdem ein Container (welcher /i/base/container inheritet) von who
geschlossen wurde, wird im Container notify("close",container,who) aufgerufen.
notify ruft dann in allen mit container->add_controller("notify_close",other)
angemeldeten Objekten other notify_close(container,who) auf.
Diese Objekte haben dann die Moeglichkeit, darauf zu reagieren.
VERWEISE: open_con, close_con, lock_con, forbidden_close
	  add_controller, notify, notify_open, notify_lock, notify_unlock
GRUPPEN: taschen
*/

/*
FUNKTION: forbidden_open
DEKLARATION: int forbidden_open(object container, object who)
BESCHREIBUNG:
Bevor ein Container (welcher /i/base/container inheritet) von who geoeffnet
wird, wird im Container forbidden("open", container, who) aufgerufen.
forbidden ruft dann in allen mit container->add_controller("forbidden_open",
other) angemeldeten Objekten other forbidden_open(container, who) auf.
Liefert auch nur ein Objekt einen Wert != 0 zurueck, so wird das Oeffnen
verboten. Fuer die Ausgabe der Fehlermeldung ist dieses Objekt zustaendig.
VERWEISE: open_con, close_con, lock_con, notify_open, add_controller,
          forbidden, forbidden_close, forbidden_lock, forbidden_unlock
GRUPPEN: taschen
*/

/*
FUNKTION: forbidden_close
DEKLARATION: int forbidden_close(object container, object who)
BESCHREIBUNG:
Bevor ein Container (welcher /i/base/container inheritet) von who geschlossen
wird, wird im Container forbidden("close", container, who) aufgerufen.
forbidden ruft dann in allen mit container->add_controller("forbidden_close",
other) angemeldeten Objekten other forbidden_close(container, who) auf.
Liefert auch nur ein Objekt einen Wert != 0 zurueck, so wird das Schliessen
verboten. Fuer die Ausgabe der Fehlermeldung ist dieses Objekt zustaendig.
VERWEISE: open_con, close_con, lock_con, notify_close, add_controller,
          forbidden, forbidden_open, forbidden_lock, forbidden_unlock
GRUPPEN: taschen
*/

/*
FUNKTION: lock_con
DEKLARATION: void lock_con(int lock, object who, object key)
BESCHREIBUNG:
Damit wird der Container auf- oder zugeschlossen. Sollte der Container im
letzteren Falle offen sein, wird er automatisch geschlossen.
Falls der Container kein Schloss besitzt, macht diese Funktion gar nix.
Eine Meldung wird nicht ausgegeben.

Die Parameter:
    lock:   0 schliesst den Container auf
	    1 schliesst den Container zu.
    who:    Das Lebewesen, welche den Container auf-/zuschliesst.
    key:    Der dazu benutzte Schluessel.

VERWEISE: set_locked, query_locked, set_no_lock, open_con, close_con,
	  notify_lock, notify_unlock, notify_close, notify_open
GRUPPEN: taschen
*/
void lock_con(int lock, object who, object key)
{
    if (!query_no_lock())
    {
        if (lock)
        {
            if (!query_locked())
            {
                close_con(); 
                set_locked(1);
                this_object()->notify("lock",this_object(),who,key);
                if (key)
                    key->notify("lock_key",  this_object(),who,key);
            }
        }
        else
        {
            if (query_locked())
            {
                set_locked(0);
                this_object()->notify("unlock",this_object(),who,key);
                if (key)
                    key->notify("unlock_key",  this_object(),who,key);
            }
        }
    }
}

/*
FUNKTION: notify_lock
DEKLARATION: void notify_lock(object container, object who, object key)
BESCHREIBUNG:
Nachdem ein Container von who mit dem Schluessel key abgeschlossen wurde,
wird im Container notify("lock",container, who, key) aufgerufen.
notify ruft dann in allen mit container->add_controller("notify_lock",other)
angemeldeten Objekten other notify_lock(container, who, key) auf.
Diese Objekte haben dann die Moeglichkeit, darauf zu reagieren.
VERWEISE: lock_con, set_locked, query_locked, set_no_lock, open_con, close_con,
	  add_controller, notify, notify_unlock, notify_close, notify_open,
	  notify_lock_key
GRUPPEN: taschen
*/

/*
FUNKTION: notify_unlock
DEKLARATION: void notify_unlock(object container, object who, object key)
BESCHREIBUNG:
Nachdem ein Container von who mit dem Schluessel key aufgeschlossen wurde,
wird im Container notify("unlock",container, who, key) aufgerufen.
notify ruft dann in allen mit container->add_controller("notify_unlock",other)
angemeldeten Objekten other notify_unlock(container, who, key) auf.
Diese Objekte haben dann die Moeglichkeit, darauf zu reagieren.
VERWEISE: lock_con, set_locked, query_locked, set_no_lock, open_con, close_con,
	  add_controller, notify, notify_lock, notify_close, notify_open,
	  notify_unlock_key
GRUPPEN: taschen
*/

/*
FUNKTION: forbidden_lock
DEKLARATION: int forbidden_lock(object container, object who, object key)
BESCHREIBUNG:
Bevor ein Container von who mit dem Schluessel key abgeschlossen wird,
wird im Container forbidden("lock", container, who, key) aufgerufen.
forbidden ruft dann in allen mit container->add_controller("forbidden_lock",
other) angemeldeten Objekten other forbidden_lock(container, who, key) auf.
Liefert auch nur ein Objekt einen Wert != 0 zurueck, so wird das Abschliessen
verboten. Fuer die Ausgabe der Fehlermeldung ist dieses Objekt zustaendig.
VERWEISE: lock_con, set_locked, query_locked, set_no_lock, open_con, close_con,
          notify_close, add_controller, forbidden, forbidden_unlock,
	  forbidden_close, forbidden_open, forbidden_lock_key
GRUPPEN: taschen
*/

/*
FUNKTION: forbidden_unlock
DEKLARATION: int forbidden_unlock(object container, object who, object key)
BESCHREIBUNG:
Bevor ein Container von who mit dem Schluessel key aufgeschlossen wurde,
wird im Container forbidden("unlock", container, who, key) aufgerufen.
forbidden ruft dann in allen mit container->add_controller("forbidden_unlock",
other) angemeldeten Objekten other forbidden_unlock(container, who, key) auf.
Liefert auch nur ein Objekt einen Wert != 0 zurueck, so wird das Aufschliessen
verboten. Fuer die Ausgabe der Fehlermeldung ist dieses Objekt zustaendig.
VERWEISE: lock_con, set_locked, query_locked, set_no_lock, open_con, close_con,
	  notify_unlock, add_controller, forbidden, forbidden_lock,
	  forbidden_close, forbidden_open, forbidden_unlock_key
GRUPPEN: taschen
*/

/*
FUNKTION: notify_lock_key
DEKLARATION: void notify_lock_key(object container, object who, object key)
BESCHREIBUNG:
Nachdem ein Container von who mit dem Schluessel key abgeschlossen wurde,
wird im Schluessel notify("lock_key", container, who, key) aufgerufen.
notify ruft dann in allen mit key->add_controller("notify_lock_key",other)
angemeldeten Objekten other notify_lock_key(container, who, key) auf.
Diese Objekte haben dann die Moeglichkeit, darauf zu reagieren.
VERWEISE: notify_lock, notify_unlock_key, forbidden_lock_key
GRUPPEN: taschen
*/

/*
FUNKTION: notify_unlock_key
DEKLARATION: void notify_unlock_key(object container, object who, object key)
BESCHREIBUNG:
Nachdem ein Container von who mit dem Schluessel key aufgeschlossen wurde,
wird im Schluessel notify("unlock_key", container, who, key) aufgerufen.
notify ruft dann in allen mit key->add_controller("notify_unlock_key",other)
angemeldeten Objekten other notify_unlock_key(container, who, key) auf.
Diese Objekte haben dann die Moeglichkeit, darauf zu reagieren.
VERWEISE: notify_unlock, notify_lock_key, forbidden_unlock_key
GRUPPEN: taschen
*/

/*
FUNKTION: forbidden_lock_key
DEKLARATION: int forbidden_lock_key(object container, object who, object key)
BESCHREIBUNG:
Bevor ein Container von who mit dem Schluessel key abgeschlossen wird,
wird im Schluessel forbidden("lock_key", container, who, key) aufgerufen.
forbidden ruft dann in allen mit key->add_controller("forbidden_lock_key",
other) angemeldeten Objekten other forbidden_lock_key(container, who, key) auf.
Liefert auch nur ein Objekt einen Wert != 0 zurueck, so wird das Abschliessen
verboten. Fuer die Ausgabe der Fehlermeldung ist dieses Objekt zustaendig.
VERWEISE: forbidden_lock, forbidden_unlock_key, notify_lock_key
GRUPPEN: taschen
*/

/*
FUNKTION: forbidden_unlock_key
DEKLARATION: int forbidden_unlock_key(object container, object who, object key)
BESCHREIBUNG:
Bevor ein Container von who mit dem Schluessel key aufgeschlossen wurde,
wird im Schluessel forbidden("unlock_key", container, who, key) aufgerufen.
forbidden ruft dann in allen mit key->add_controller("forbidden_unlock_key",
other) angemeldeten Objekten other forbidden_unlock_key(container, who, key) auf.
Liefert auch nur ein Objekt einen Wert != 0 zurueck, so wird das Aufschliessen
verboten. Fuer die Ausgabe der Fehlermeldung ist dieses Objekt zustaendig.
VERWEISE: forbidden_unlock, forbidden_lock_key, notify_unlock_key
GRUPPEN: taschen
*/

/*
FUNKTION: set_keys
DEKLARATION: void set_keys(string *keys)
BESCHREIBUNG:
Hiermit setzt man, mit welchen Schluesseln das Schloss des Containers
zu oeffnen ist.
VERWEISE: query_keys, set_locked, set_no_lock
GRUPPEN: taschen
*/
void set_keys(string *k) 
{
    keys = k;
    this_object()->add_setter_conservation("set_keys", ({keys}));
}

/*
FUNKTION: query_keys
DEKLARATION: string *query_keys()
BESCHREIBUNG:
Hiermit kann man abfragen, mit welchen Schluesseln das Schloss des
Containers zu oeffnen ist.
VERWEISE: set_keys, set_locked, set_no_lock
GRUPPEN: taschen
*/
string *query_keys() { return keys; }

/*
FUNKTION: set_crack
DEKLARATION: void set_crack(int chance)
BESCHREIBUNG:
Hiermit setzt man, mit welcher Wahrscheinlichkeit der Container von
einem durchschnittlichen Dieb zu knacken ist.
Je hoeher der Wert von Chance, desto leichter kann man das Schloss
knacken. Es sind Werte von 0 bis 100 zugelassen. Default: 0
Es gibt P_CRACK_CHANCES in /sys/properties.h
VERWEISE: query_crack, crack
GRUPPEN: taschen
*/
void set_crack(int i)
{
    crack = i; 
    this_object()->add_setter_conservation("set_crack", ({crack}));
}

/*
FUNKTION: query_crack
DEKLARATION: int query_crack()
BESCHREIBUNG:
Hiermit kann man abfragen, mit welcher Wahrscheinlichkeit der Container
von einem durchschnittlichen Dieb zu knacken ist.
Es gibt P_CRACK_CHANCES in /sys/properties.h
VERWEISE: set_crack, crack
GRUPPEN: taschen
*/
int query_crack() { return crack; }

// Funktionen fuers neue Encumbrance-Handling
// Wenn collapsible 1 ist, dann wird bei add_enc nur die internal_enc
// addiert, sonst max_internal_enc
/*
FUNKTION: query_collapsible
DEKLARATION: int query_collapsible()
BESCHREIBUNG:
Liefert einen Wert != 0, wenn dieser Container zusammenquetschbar ist.
D.h. es nimmt in einem anderen Container genausoviel Platz weg,
wie sein aktueller Inhalt.
VERWEISE: set_collapsible, test_add_encumbrance,
	  query_internal_encumbrance, query_max_internal_encumbrance
GRUPPEN: taschen
*/
int query_collapsible() { return collapsible; }

/*
FUNKTION: set_collapsible
DEKLARATION: void set_collapsible(int flag)
BESCHREIBUNG:
Damit kann man setzen, ob dieser Container zusammenquetschbar ist.
Bei 1 nimmt er in einem anderen Container genausoviel Platz weg,
wie sein aktueller Inhalt. Bei 0 soviel, wie in diesem Container
hineinpassen koennte.
Diese Funktion ruft auch set_min_weight auf.
VERWEISE: query_collapsible, query_internal_encumbrance,
	  query_max_internal_encumbrance, test_add_encumbrance,
	  set_min_weight, set_max_weight.
GRUPPEN: taschen
*/
void set_collapsible(int flag)
{
    collapsible = flag;
    this_object()->add_setter_conservation("set_collapsible", ({flag}));
    set_min_weight(collapsible?0:query_max_internal_encumbrance());
}

void create()
{
    set_max_internal_encumbrance(10);
}

string query_container_long(object who)
{
    if (query_locked())
        return wrap(Er() + ist(this_object(),1) + " verschlossen.");
    else if (!query_no_door())
        return wrap(Er() + ist(this_object(),1) +
            (query_con_close() ? " geschlossen." : " offen."));
    return "";
}

// Diese Funktion kann weg, wenn wirklich niemand mehr
// auf container::query_long zugreift.
string query_long(object viewer)
{
    return query_long_string() + query_container_long(viewer);
}

protected string query_long_postprocess(string msg, mapping info)
{
    // Wenn die Beschreibung T_CON_LOCKED oder T_CON_CLOSED nicht
    // verwendet hat, dann haengen wir noch den Statustext dran.
    if((query_locked() && !query_long_has_tag(T_ATOM_TAG_CON_LOCKED)) ||
       (!query_no_door() && !query_long_has_tag(T_ATOM_TAG_CON_CLOSED)))
	msg += query_container_long(info[TI_VIEWER]);
    
    return msg;
}

void init()
{
    add_action("open_command",  "öffne");
    add_action("close_command", "schließe",-7);
    add_action("close_command", "sperre",-5);
}

private string Der_ist()
{
    return Der() + ist(this_object(),1);
}

int open_command(string str)
{
    if (!str)
	return notify_fail("Was willst Du öffnen?\n",FAIL_NOT_OBJ);
    if (!me(str))
	return notify_fail(wrap(capitalize(str) + " kannst du nicht öffnen."),FAIL_NOT_OBJ);
    if (query_no_door())
	return notify_fail(wrap(Den() + " kann man nicht öffnen."),FAIL_INTERNAL,1);
    if (query_locked())
	return notify_fail(wrap(Der_ist() + " verschlossen."),FAIL_INTERNAL,1);
    if (!query_con_close())
        return notify_fail(wrap(Der_ist() + " gar nicht zu."),FAIL_INTERNAL,1);
    if (this_player()->free_hand() < 0)
	return notify_fail(wrap("Du hast keine Hand frei, um " + deinen() + " zu öffnen."), FAIL_INTERNAL);

    if (this_object()->forbidden("open",this_object(),this_player()))
	return 1;
    notify_msg(wrap("Du öffnest " + deinen() + "."));
    send_msg(wrap(Der(this_player()) + " öffnet " + seinen() + "."));
    open_con();
    return 1;
}

private <object|int> match_key()
{
    object key, *schluesselbuende, bund;

    if (!sizeof(keys))
        return 1; // geht ohne schluessel auf/abzuschliessen

    foreach (key : all_inventory(this_player()))
    {
        if (key->fit(this_object()))
            return key;
    }
    schluesselbuende = filter_objects(all_inventory(this_player()),
            "query_schluesselbund");
    foreach (bund : schluesselbuende)
    {
        foreach (key : all_inventory(bund))
        {
            if (key->fit(this_object()))
                return key;
        }
    }

    return 0; // keinen passenden schluessel gefunden
}

int close_command(string str)
{
    string rest;
    <object|int> key;

    if (!(rest = me(str)))
        return notify_fail(query_verb() + " was (auf/zu)?\n",FAIL_NOT_OBJ);

    switch(explode(rest," ")[<1])
    {
    case "auf":
        if (query_no_lock())
            return notify_fail(wrap(Den() + " kann man nicht aufschließen."),
                FAIL_INTERNAL);

        if (!(key = match_key()))
            return notify_fail("Ohne passenden Schlüssel geht das nicht.\n", 
                FAIL_INTERNAL);

        if (!query_locked())
            return notify_fail(wrap(Der_ist() + " bereits aufgeschlossen."),
                FAIL_INTERNAL);

        if (this_player()->free_hand() < 0)
            return notify_fail(wrap("Du hast keine Hand frei, um " + deinen() +
                    " aufzuschließen."), FAIL_INTERNAL);

        if (this_object()->forbidden("unlock",this_object(),
                    this_player(), objectp(key) && key))
            return 1;

        if (objectp(key) && key->forbidden("unlock_key",this_object(), 
                    this_player(), key))
            return 1;

        lock_con(0,this_player(), objectp(key) && key);
        notify_msg(wrap("Du schließt " + deinen() 
            + (objectp(key) ? " mit " + deinem(key) : "")
            + " auf."));
        send_msg(wrap(Der(this_player()) + " schließt " + seinen()
            + (objectp(key) ? " mit " + seinem(key) : "")
            + " auf."));
        return 1;

    case "zu":
    case "ab":
        if (query_no_lock())
            return notify_fail(wrap(Den() + " kann man nicht abschließen."),
                FAIL_INTERNAL);

        if (!(key = match_key()))
            return notify_fail("Ohne passenden Schlüssel geht das nicht.\n", 
                FAIL_INTERNAL);

        if (query_locked())
            return notify_fail(wrap(Der_ist() + " bereits abgeschlossen."),
                FAIL_INTERNAL);

        if (!query_con_close())
            return notify_fail(wrap("Du solltest "+deinen()+
                " dazu schließen."),FAIL_INTERNAL);
        
        if (this_player()->free_hand() < 0)
            return notify_fail(wrap("Du hast keine Hand frei, um "+deinen()+
                    " abzuschließen."), FAIL_INTERNAL);

        if (this_object()->forbidden("lock",this_object(),
                    this_player(), objectp(key) && key))
            return 1;
        
        if (objectp(key) && key->forbidden("lock_key",this_object(), 
                    this_player(), key))
            return 1;

        lock_con(1,this_player(), objectp(key) && key);
        notify_msg(wrap("Du schließt " + deinen()
            + (objectp(key) ? " mit " + deinem(key) : "")
            + " ab."));
        send_msg(wrap(Der(this_player()) + " schließt " + seinen()
            + (objectp(key) ? " mit " + seinem(key) : "")
            + " ab."));
        return 1;
    } 

    if (!strstr(query_verb_ascii(),"schliess"))
    {
        if (query_no_door())
            return notify_fail(wrap(Den() + " kann man nicht schließen.\n"),
                FAIL_INTERNAL);

        if (query_con_close())
            return notify_fail(wrap(Der_ist() + " doch schon zu!"),
                FAIL_INTERNAL);

        if (this_player()->free_hand() < 0)
            return notify_fail(wrap("Du hast keine Hand frei, um " + deinen() +
                    " zu schließen."), FAIL_INTERNAL);

        if (this_object()->forbidden("close",this_object(),this_player()))
            return 1;

        notify_msg(wrap("Du schließt " + deinen() + "."));
        send_msg(wrap(Der(this_player()) + " schließt " + seinen() + "."));
        close_con();
        return 1;
    }

    return notify_fail(wrap(query_verb() + " " + den() + " AUF oder ZU?"),
        FAIL_WRONG_ARG);
}

/*
FUNKTION: notify_crack
DEKLARATION: void notify_crack(object container, object who, mixed dietrich)
BESCHREIBUNG:
Nachdem ein Container von who mit dem Dietrich dietrich (kann 0,
ein Objekt oder V-Item sein) aufgeschlossen wurde, wird im Container
notify("crack", container, who, dietrich) aufgerufen.
notify ruft dann in allen mit container->add_controller("notify_crack",other)
angemeldeten Objekten other notify_crack(container, who, dietrich) auf.
Diese Objekte haben dann die Moeglichkeit, darauf zu reagieren.
VERWEISE: crack, lock_con, set_locked, set_no_lock, open_con, close_con,
	  forbidden_crack, add_controller, notify, notify_lock, notify_unlock,
	  notify_close, notify_open
GRUPPEN: taschen
*/

/*
FUNKTION: forbidden_crack
DEKLARATION: int forbidden_crack(object container, object who, mixed dietrich)
BESCHREIBUNG:
Bevor ein Container von who mit dem Dietrich dietrich (kann 0, ein Objekt
oder V-Item sein) aufgeschlossen wurde, wird im Container
forbidden("crack", container, who, dietrich) aufgerufen.
forbidden ruft dann in allen mit container->add_controller("forbidden_crack",
other) angemeldeten Objekten other forbidden_crack(container, who, dietrich)
auf.
Liefert auch nur ein Objekt einen Wert != 0 zurueck, so wird das Knacken
verboten. Fuer die Ausgabe der Fehlermeldung ist dieses Objekt zustaendig.
VERWEISE: crack, lock_con, set_locked, set_no_lock, open_con, close_con,
	  notify_crack, add_controller, forbidden, forbidden_lock,
	  forbidden_unlock, forbidden_close, forbidden_open
GRUPPEN: taschen
*/

/*
FUNKTION: crack
DEKLARATION: varargs int crack(object who [, mixed dietrich])
BESCHREIBUNG:
Hiermit wird der Container geknackt.
who ist derjenige, der knackt, und optional kann man dietrich angeben,
mit dem geknackt wird.
Es wird this_object()->forbidden("crack",this_object(),who,dietrich)
aufgerufen, um zu ueberpruefen, ob es erlaubt ist.
Anschliessen wird this_object()->notify("crack",this_object(),who,dietrich)
aufgerufen.
VERWEISE: query_crack, set_crack
GRUPPEN: taschen
*/
varargs int crack(object who, mixed dietrich)
{
    if (query_no_lock())
    {
	notify_msg(wrap(Den() + " kann man nicht knacken."),who);
	return 0;
    }
    if (!query_locked())
    {
	notify_msg(wrap(Den() + " brauchst Du doch gar nicht knacken."),who);
	return 0;
    }
    if (this_object()->forbidden("crack",this_object(),who,dietrich))
	return 0;
    set_locked(0);
    notify_msg(wrap("Du knackst " + den() +
	    (dietrich ? " mit " + deinem(dietrich) : "") + "."),who);
    send_msg(wrap(Der(who) + " schließt " + seinen() + " auf."),who);
    this_object()->notify("crack",this_object(),who,dietrich);
    return 1;
}

/*
FUNKTION: query_tasche
DEKLARATION: int query_tasche()
BESCHREIBUNG:
Liefert bei Taschen immer 1.
Diese Funktion wird auch dazu verwendet, zu verhindern, dass man gefuellte
Taschen in eine andere Tasche legen kann.
VERWEISE: query_container
GRUPPEN: taschen
*/
int query_tasche() { return 1; }

/*
FUNKTION: T_LISTE
DEKLARATION: Liste der Container-T-Defines
BESCHREIBUNG:

Vordefinierte Bedingungen:
 - T_CON_LOCKED         Der Container ist verschlossen.
 - T_CON_CLOSED		Der Container ist geschlossen.
 - T_CON_NO_DOOR	Der Container hat keine Tuer/Deckel/Schloss.

VERWEISE: query_locked, query_con_close, query_no_door
GRUPPEN: taschen
*/

int desc_condition_con_locked(mixed info)
{
    return query_locked();
}

int desc_condition_con_closed(mixed info)
{
    return query_con_close();
}

int desc_condition_con_no_door(mixed info)
{
    return query_no_door();
}
