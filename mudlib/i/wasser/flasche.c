// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        flasche.c
// Description: Eine Flasche, die man an Brunnen und Fluessen fuellen kann.
// Author:      Jafar & Yellow
// Modified by: Jafar, 21.6.96
//              fill_from_object, fill_from_vitem, drink_action und
//              empty_action koennen geshadowed werden.
//
//              Jafar, 9.7.96
//              Man kann jetzt die Meldungen, warum man nicht trinken oder
//              eine Flasche fuellen kann, frei setzen.
//        
//              Jafar, 16.12.96
//              Bug in den Meldungen.
//
//              Jafar, 26.02.97
//              Fuellstand beruecksichtig jetzt Plural-Objekte.
//              Abfrage der Fuellmenge beruecksichtigt fehlendes Wasser-Objekt.
//              Geister koennen nicht trinken.
//
//              Jafar, 26.04.97
//              Bug beim Versuch gefuellte Flaschen erneut zu fuellen.
//
//              Jafar, 31.03.98
//              Puralform wird jetzt ueberall beruecksichtigt.
//              Fix fuer die Umstellung in den Vitems. Die Mapping der Vitems
//              enthalten keine Id mehr, wenn sie mit dem Namen
//              uebereinstimmt. 
//
//              Jackie, 16.05.00
//              let_not_in() ueberarbeitet. Laeuft jetzt ueber eine
//              Funktion check_allowed_water(), damit man das nach
//              Wunsch ueberlagern kann (nebenbei noch return ::let_not_in()
//              statt return 0).

/* 

Die Flasche versteht folgende Befehle:

  fuelle flasche
  fuelle flasche mit wasser
  fuelle flasche an/am brunnen
  fuelle flasche mit wasser aus brunnen
  fuelle flasche an/am brunnen mit wasser

  trinke wasser
  trinke wasser aus flasche
  trinke aus flasche

  leere flasche
  leere flasche aus
  leere wasser weg
  leere wasser aus

  entleere flasche

  schuette wasser weg
  schuette wasser aus

*/

#pragma save_types
#pragma strong_types
#pragma verbose_errors

#include <deklin.h>
#include <invis.h>
#include <move.h>
#include <parse_com.h>
#include <wasser.h>
#include <message.h>
#include <notify_fail.h>
#include <properties.h>
#include <simul_efuns.h>

inherit "/i/contain";
virtual inherit "/i/move";
virtual inherit "/i/item";
virtual inherit "/i/value";


private int max_content = MAX_CONTENT, // Fassungsvermoegen in Schlucken
            content = 0;               // Inhalt in Schlucken

private object water;                  // die Fluessigkeit

private closure empty_message,         // Meldungen fuer das Leeren
                other_empty_message,
                success_message,       // und das Trinken
                other_success_message,
                failure_message,
                other_failure_message;

mapping query_debug_info() 
{ 
    mapping dbg =  ::query_debug_info()||([]);
    dbg["content"] = content;
    dbg["max_content"] = max_content;
    if (water)
        dbg["water_dbg"] = water->query(P_WATER_ORIGIN);
    return dbg;
}

/*
FUNKTION: set_content
DEKLARATION: void set_content(int n)
BESCHREIBUNG: 
Setzt den momentanen Inhalt der Flasche in Schlucken.
VERWEISE: query_content, set_max_content, query_max_content, set_water, 
query_water          
GRUPPEN: wasser
*/
void set_content(int n) {
    content = (n <= max_content ? n : max_content);
    if(!content && water) water->remove();
}


/*
FUNKTION: query_content
DEKLARATION: int query_content()
BESCHREIBUNG: 
Liefert den momentanen Inhalt in Schlucken zurueck.
VERWEISE: set_content, set_max_content, query_max_content, set_water, 
query_water
GRUPPEN: wasser
*/
int query_content() { return (water ? content : 0); }


/*
FUNKTION: set_max_content
DEKLARATION: void set_max_content(int n)
BESCHREIBUNG: 
Setzt den maximalen Inhalt in Schlucken, den die Flasche aufnehmen kann.
VERWEISE: set_content, query_content, query_max_content, set_water, query_water
GRUPPEN: wasser
*/
void set_max_content(int n) { max_content = n;}


/*
FUNKTION: query_max_content
DEKLARATION: int query_max_content()
BESCHREIBUNG: 
Liefert den maximalen Inhalt in Schlucken, den die Flasche aufnehmen kann.
VERWEISE: set_content, query_content, set_max_content, set_water, query_water
GRUPPEN: wasser
*/
int query_max_content() { return max_content; }


/*
FUNKTION: set_water
DEKLARATION: int set_water(object new_water)
BESCHREIBUNG: 
Setzt ein neues Wasser-Objekt und bewegt es in die Flasche. War bereits eines 
vorhanden, wird dieses zuvor geloescht. Wenn es keine Probleme gab, liefert 
die Funktion einen Wert ungleich 0, ansonsten wird das neue Wasser-Objekt 
geloescht und der Rueckgabewert ist 0.  
VERWEISE: set_content, query_content, set_max_content, query_max_content, 
query_water
GRUPPEN: wasser
*/
int set_water(object new_water) 
{ 
   if (water) 
     water->remove();

   water = new_water;

   if (water && water->move(this_object()) != MOVE_OK) {
     this_player()->send_message_to(this_player(), MT_NOTIFY, MA_USE,
       wrap("Ups. Du hast " + den(water, "ganz") + " verschüttet."));
     water->remove();
     
     return 0;
   } else
     return 1;
}


/*
FUNKTION: query_water 
DEKLARATION: object query_water()
BESCHREIBUNG: 
Liefert das Wasser-Objekt in der Flasche zurueck oder 0, wenn keines vorhanden
ist. Das Objekt sollte nicht aus der Flasche bewegt werden.
VERWEISE: set_content, query_content, set_max_content, query_max_content, 
set_water
GRUPPEN: wasser
*/
object query_water() { return water; }


/*
FUNKTION: transfer_content 
DEKLARATION: int transfer_content(object new_bottle)
BESCHREIBUNG: 
Transferiert das Wasserobjekt zur new_bottle, !=0 bei Erfolg, 0 sonst.
VERWEISE: set_water,query_water
GRUPPEN: wasser
*/
int transfer_content(object new_bottle)
{
    if (water && new_bottle->is_bottle())
    {
        water->set_no_move(0);
        int ret = new_bottle->set_water(water);
        water = 0;
        content = 0;
        return ret;
    }
    return 0;
}

/*
FUNKTION: water_me
DEKLARATION: string water_me(string s)
BESCHREIBUNG: 
water_me ist ein abgesicherter Aufruf der Funktion me im Wasser-Objekt in der
Flasche. Ist kein Wasser-Objekt vorhanden, oder ist es mit dem String nicht 
gemeint, dann liefert water_me 0, ansonsten einen String mit dem Rest des 
Kommandos. 
VERWEISE: me, set_water, query_water
GRUPPEN: wasser
*/
string water_me(string s) 
{
   if (water) 
     return water->me(s);
   else
     return 0;
}


/*
FUNKTION: set_success_message
DEKLARATION: void set_success_message(mixed s)
BESCHREIBUNG: 
Nachdem ein Spieler aus der Flasche getrunken hat, erhaelt er eine Meldung,
die aus einem Teil von der Flasche und einem Teil vom Wasser-Objekt 
zusammengesetzt wird. set_success_message legt den Teil fest, welcher 
die Flasche betrifft. Der Teil vom Wasser wird daran angehaengt, weshalb sie 
mit einem Leerzeichen enden sollte. Die Meldung kann eine Pseudoclosure oder 
Closure sein, in denen zu den ueblichen Symbolen noch 'water fuer das Wasser
hinzukommt.
VERWEISE: set_other_success_message, set_failure_message, 
set_other_failure_message, query_success_message, query_other_success_message,
query_failure_message, query_other_failure_message, drink_action
GRUPPEN: wasser
*/
void set_success_message(mixed s) 
{ 
   success_message = mixed_to_closure(s, ({'water})); 
}


/*
FUNKTION: set_other_success_message
DEKLARATION: void set_other_success_message(mixed s)
BESCHREIBUNG: 
Nachdem ein Spieler aus der Flasche getrunken hat, erhalten die Umstehenden 
eine Meldung, die aus einem Teil von der Flasche und einem Teil vom 
Wasser-Objekt zusammengesetzt wird. set_other_success_message legt den Teil 
fest, welcher die Flasche betrifft. Der Teil vom Wasser wird daran angehaengt, 
weshalb sie mit einem Leerzeichen enden sollte. Die Meldung kann eine 
Pseudoclosure oder Closure sein, in denen zu den ueblichen Symbolen noch 
'water fuer das Wasser hinzukommt.
VERWEISE: set_success_message, set_failure_message, set_other_failure_message,
query_success_message, query_other_success_message, query_failure_message, 
query_other_failure_message, drink_action
GRUPPEN: wasser
*/
void set_other_success_message(mixed s) 
{ 
   other_success_message = mixed_to_closure(s, ({'water})); 
}


/*
FUNKTION: set_failure_message
DEKLARATION: void set_failure_message(mixed s)
BESCHREIBUNG: 
Wenn ein Spieler nicht aus der Flasche trinken kann (zuviele Wasserpunkte,
zuviele Alkoholpunkte oder aehnliches), erhaelt er eine Meldung die aus einem 
Teil von der Flasche und einem Teil vom Wasser-Objekt zusammengesetzt wird. 
set_failure_message legt den Teil fest, welcher die Flasche betrifft. Der Teil
vom Wasser wird daran angehaengt, weshalb sie mit einem Leerzeichen enden 
sollte. Die Meldung kann eine Pseudoclosure oder Closure sein, in denen zu den
ueblichen Symbolen noch 'water fuer das Wasser hinzukommt.
VERWEISE: set_success_message, set_other_success_message, 
set_other_failure_message, query_success_message, query_other_success_message, 
query_failure_message, query_other_failure_message, drink_action
GRUPPEN: wasser
*/
void set_failure_message(mixed s) 
{ 
   failure_message = mixed_to_closure(s, ({'water})); 
}


/*
FUNKTION: set_other_failure_message
DEKLARATION: void set_other_failure_message(mixed s)
BESCHREIBUNG: 
Wenn ein Spieler nicht aus der Flasche trinken kann (zuviele Wasserpunkte,
zuviele Alkoholpunkte oder aehnliches), erhaelt er eine Meldung die aus einem 
Teil von der Flasche und einem Teil vom Wasser-Objekt zusammengesetzt wird. 
set_failure_message legt den Teil fest, welcher die Flasche betrifft. Der Teil
vom Wasser wird daran angehaengt, weshalb sie mit einem Leerzeichen enden 
sollte. Die Meldung kann eine Pseudoclosure oder Closure sein, in denen zu den
ueblichen Symbolen noch 'water fuer das Wasser hinzukommt.
VERWEISE: set_success_message, set_other_success_message, 
set_failure_message, query_success_message, query_other_success_message, 
query_failure_message, query_other_failure_message, drink_action
GRUPPEN: wasser
*/
void set_other_failure_message(mixed s) 
{ 
   other_failure_message = mixed_to_closure(s, ({'water})); 
}


/*
FUNKTION: set_empty_message
DEKLARATION: void set_empty_message(mixed s)
BESCHREIBUNG: 
set_empty_message setzt die Meldung die der Spieler erhaelt, wenn er die
Flasche entleert. Die Meldung kann eine Pseudoclosure oder Closure sein, in 
denen zu den ueblichen Symbolen noch 'water fuer das Wasser hinzukommt.
VERWEISE: set_other_empty_message, query_empty_message, 
query_other_empty_message, empty_action
GRUPPEN: wasser
*/
void set_empty_message(mixed s) 
{ 
   empty_message = mixed_to_closure(s, ({'water})); 
}


/*
FUNKTION: set_other_empty_message
DEKLARATION: void set_other_empty_message(mixed s)
BESCHREIBUNG: 
set_other_empty_message setzt die Meldung die Umstehende erhalten, wenn ein 
Spieler die Flasche entleert. Die Meldung kann eine Pseudoclosure oder Closure 
sein, in denen zu den ueblichen Symbolen noch 'water fuer das Wasser 
hinzukommt.
VERWEISE: set_empty_message, query_empty_message, query_other_empty_message, 
empty_action
GRUPPEN: wasser
*/
void set_other_empty_message(mixed s) 
{ 
   other_empty_message = mixed_to_closure(s, ({'water})); 
}


/*
FUNKTION: query_success_message
DEKLARATION: string query_success_message()
BESCHREIBUNG: 
Nachdem ein Spieler aus der Flasche getrunken hat, erhaelt er eine Meldung,
die aus einem Teil von der Flasche und einem Teil vom Wasser-Objekt 
zusammengesetzt wird. query_success_message setzt diese beiden Teile zusammen
und liefert die vollstaendige Meldung zurueck. Ist keine Meldung fuer die 
Flasche angegeben, wird fuer diesen Teil eine Default-Meldung verwendet.
VERWEISE: set_success_message, set_other_success_message, set_failure_message, 
set_other_failure_message, query_other_success_message, query_failure_message,
query_other_failure_message, drink_action
GRUPPEN: wasser
*/
string query_success_message() 
{
    string msg1,msg2;
    if (success_message)
    {
        msg1 = closure_to_string(success_message, ({water}))||"";
        msg2 = water->query_success_message()||"";
        if (msg1!="" && msg2!="" && msg1[<1]!=' ' && msg2[0]!=' ')
        {
            return msg1+" "+msg2;
        }
        return msg1+msg2;
    }
    else
        return "Du trinkst einen kräftigen Schluck " + wen(water, ART_KEINS) + 
         " aus " + deinem(OBJ_TO) + ". " + water->query_success_message();
}


/*
FUNKTION: query_other_success_message
DEKLARATION: string query_other_success_message()
BESCHREIBUNG: 
Nachdem ein Spieler aus der Flasche getrunken hat, erhalten die Umstehenden 
eine Meldung, die aus einem Teil von der Flasche und einem Teil vom 
Wasser-Objekt zusammengesetzt wird. query_other_success_message setzt diese 
beiden Teile zusammen und liefert die vollstaendige Meldung zurueck. Ist keine
Meldung fuer die Flasche angegeben, wird fuer diesen Teil eine Default-Meldung
verwendet.
VERWEISE: set_success_message, set_other_success_message, set_failure_message, 
set_other_failure_message, query_success_message, query_failure_message,
query_other_failure_message, drink_action
GRUPPEN: wasser
*/
string query_other_success_message() 
{ 
    string msg1,msg2;
    if (other_success_message)
    {
        msg1 = closure_to_string(other_success_message, ({water}));
        msg2 = water->query_other_success_message();
        if (msg1!="" && msg2!="" && msg1[<1]!=' ' && msg2[0]!=' ')
        {
            return msg1+" "+msg2;
        }
        return msg1+msg2;
    }
    else
        return Der(OBJ_TP) + " trinkt einen kräftigen Schluck " + 
            wen(water, ART_KEINS) + " aus " + seinem(OBJ_TO) + ". " + 
            water->query_other_success_message();
}


/*
FUNKTION: query_failure_message
DEKLARATION: string query_failure_message()
BESCHREIBUNG: 
Wenn ein Spieler nicht aus der Flasche trinken kann (zuviele Wasserpunkte,
zuviele Alkoholpunkte oder aehnliches), erhaelt er eine Meldung die aus einem 
Teil von der Flasche und einem Teil vom Wasser-Objekt zusammengesetzt wird. 
query_failure_message setzt diese beiden Teile zusammen und liefert die 
vollstaendige Meldung zurueck. Standardmaessig ist der Teil der Flasche
ein Leerstring.
VERWEISE: set_success_message, set_other_success_message, set_failure_message, 
set_other_failure_message, query_success_message, query_other_success_message,
query_other_failure_message, drink_action
GRUPPEN: wasser
*/
string query_failure_message() 
{
   if (failure_message)
     return closure_to_string(failure_message, ({water})) + 
         water->query_failure_message(); 
   else
     return water->query_failure_message();
}


/*
FUNKTION: query_other_failure_message
DEKLARATION: string query_other_failure_message()
BESCHREIBUNG: 
Wenn ein Spieler nicht aus der Flasche trinken kann (zuviele Wasserpunkte,
zuviele Alkoholpunkte oder aehnliches), erhalten die Umstehenden eine Meldung 
die aus einem Teil von der Flasche und einem Teil vom Wasser-Objekt 
zusammengesetzt wird. query_other_failure_message setzt diese beiden Teile 
zusammen und liefert die vollstaendige Meldung zurueck. Standardmaessig ist 
der Teil der Flasche ein Leerstring.
VERWEISE: set_success_message, set_other_success_message, set_failure_message, 
set_other_failure_message, query_success_message, query_other_success_message,
query_failure_message, drink_action
GRUPPEN: wasser
*/
string query_other_failure_message() 
{ 
   if (other_failure_message)
     return closure_to_string(other_failure_message, ({water})) +
         water->query_other_failure_message();
   else
     return water->query_other_failure_message();
}


/*
FUNKTION: query_fill_message
DEKLARATION: string query_fill_message(mixed well, mixed water)
BESCHREIBUNG: 
query_fill_message liefert zur Flasche und dem uebergebenen Paar von Brunnen
und Wasser die Meldung zum Fuellen fuer den Spieler. Dabei gibt es drei
Faelle:

a) Der Brunnen ist ein Objekt, dann wird ueber die Funktion query_fill_message
   im Brunnen die Meldung erfragt.
b) Sowohl Brunnen als auch Wasser sind Vitems, dann wird die Meldung aus dem
   Mapping (well_fill_message) des Brunnens genommen.
c) Es ist ein einzelnes Wasser (well == 0) als Vitem, dann wird aus dem
   Mapping des Wassers (water_fill_message) die Meldung genommen.

Enthalten in den letzten beiden Faellen die Mappings keine Eintraege fuer
Fuellmeldungen, dann wird eine Standard-Meldung benutzt.
VERWEISE: query_other_fill_message, fill_from_object, fill_from_vitem
GRUPPEN: wasser
*/
string query_fill_message(mixed well, mixed water) 
{
   if (well)
     if (objectp(well))
       return well->query_fill_message(this_object(), water);
     else if (mappingp(well) && well["well_fill_msg"])
       return closure_to_string(mixed_to_closure(well["well_fill_msg"],
                                                 ({'bottle, 'water})),
                                ({this_object(), water}));
     else 
       return "Du füllst " + deinen(OBJ_TO) + " mit " +
           wem(water, ART_KEINS) + " aus " + deinem(well) + ".";
   else 
     if (mappingp(water) && water["water_fill_msg"])
       return closure_to_string(mixed_to_closure(water["water_fill_msg"],
                                                 ({'bottle})),
                               ({this_object()}));
     else 
       return "Du füllst " + deinen(OBJ_TO) + " mit " +
           wem(water, ART_KEINS) + " auf.";
}


/*
FUNKTION: query_other_fill_message
DEKLARATION: string query_other_fill_message(mixed well, mixed water)
BESCHREIBUNG: 
query_other_fill_message liefert zur Flasche und dem uebergebenen Paar von 
Brunnen und Wasser die Meldung zum Fuellen fuer die Umstehenden. Dabei gibt es
drei Faelle:

a) Der Brunnen ist ein Objekt, dann wird ueber die Funktion 
   query_other_fill_message im Brunnen die Meldung erfragt.
b) Sowohl Brunnen als auch Wasser sind Vitems, dann wird die Meldung aus dem
   Mapping (well_other_fill_message) des Brunnens genommen.
c) Es ist ein einzelnes Wasser (well == 0) als Vitem, dann wird aus dem
   Mapping des Wassers (water_other_fill_message) die Meldung genommen.

Enthalten in den letzten beiden Faellen die Mappings keine Eintraege fuer
Fuellmeldungen, dann wird eine Standard-Meldung benutzt.
VERWEISE: query_fill_message, fill_from_object, fill_from_vitem
GRUPPEN: wasser
*/
string query_other_fill_message(mixed well, mixed water) 
{
   if (well)
     if (objectp(well))
       return well->query_other_fill_message(this_object(), water);
     else if (mappingp(well) && well["well_other_fill_msg"])
       return closure_to_string(mixed_to_closure(well["well_other_fill_msg"],
                                                 ({'bottle, 'water})),
                                ({this_object(), water}));
     else 
       return Der(OBJ_TP) + " füllt " + seinen(OBJ_TO) + " mit " + 
           wem(water, ART_KEINS) + " aus " + seinem(well) + ".";
   else 
     if (mappingp(water) && water["water_other_fill_msg"])
       return closure_to_string(mixed_to_closure(water["water_other_fill_msg"],
                                                 ({'bottle})),
                                ({this_object()}));
     else 
       return Der(OBJ_TP) + " füllt " + seinen(OBJ_TO) + " mit " + 
           wem(water, ART_KEINS) + " auf.";
}


/*
FUNKTION: query_no_fill_reason
DEKLARATION: string query_no_fill_reason(mapping well, mapping water)
BESCHREIBUNG:
query_no_fill_reason liefert zu den Vitems fuer ein Paar von Brunnen und Wasser
oder einzelnes Wasser/einzelne Brunnen den Grund zurueck, warum daran keine
Flasche gefuellt werden kann. In den Closures der Vitems fuer Brunnen steht 
der Bezeichner water fuer das Wasser im Brunnen. 
VERWEISE: query_no_fill_reason (flasche)
GRUPPEN: wasser
*/
string query_no_fill_reason(mapping well, mapping water)
{
   if (well)
     if (well["well_no_fill_reason"])
       return closure_to_string(mixed_to_closure(well["well_no_fill_reason"],
                                                 ({'bottle, 'water})),
                                ({this_object(), water}));
     else if (water)
       return "Du kannst " + deinen(OBJ_TO) + " nicht mit " + dem(water) + 
           " aus " + dem(well) + " füllen.";
     else 
       return "Du kannst " + deinen(OBJ_TO) + " nicht an " + dem(well) + 
           " füllen.";
   else if (water["water_no_fill_reason"])
     return closure_to_string(mixed_to_closure(water["water_no_fill_reason"],
                                               ({'bottle})),
                              ({this_object()}));
   else
     return "Du kannst " +deinen(OBJ_TO) + " nicht mit " + dem(water) + 
         " füllen.";
}



/*
FUNKTION: query_empty_message
DEKLARATION: void query_empty_message()
BESCHREIBUNG: 
query_empty_message liefert die Meldung fuer den Spieler beim entleeren der
Flasche. Ist keine Meldung gesetzt, wird eine Default-Meldung verwendet.
VERWEISE: set_empty_message, set_other_empty_message, 
query_other_empty_message, empty_action
GRUPPEN: wasser
*/
string query_empty_message()
{
   if (empty_message)
     return closure_to_string(empty_message, ({water}));
   else
     return "Du schüttest " + den(water) + " aus " + deinem(OBJ_TO) + " weg.";
}


/*
FUNKTION: query_other_empty_message
DEKLARATION: void query_other_empty_message()
BESCHREIBUNG: 
query_other_empty_message liefert die Meldung fuer die Umstehenden beim 
entleeren der Flasche. Ist keine Meldung gesetzt, wird eine Default-Meldung 
verwendet.
VERWEISE: set_empty_message, set_other_empty_message, 
query_empty_message, empty_action
GRUPPEN: wasser
*/
string query_other_empty_message()
{
   if (other_empty_message)
     return closure_to_string(other_empty_message, ({water}));
   else
     return Der(OBJ_TP) + " schüttet " + den(water) + " aus " +
       seinem(OBJ_TO) + " weg.";
}

/*
FUNKTION: message
DEKLARATION: void message(string player, string others)
BESCHREIBUNG: 
message nimmt die Meldungen fuer den Spieler und fuer die Umstehenden,
bricht sie um und gibt sie aus. Ist der Spieler unsichtbar oder die Meldung
fuer die Umstehenden leer, so wird sie unterdrueckt. 
VERWEISE: 
GRUPPEN: wasser
*/
void message(string player, string others)
{
   if (!this_player()) return;
   
   if (player != "")
     this_player()->send_message_to(this_player(), MT_LOOK|MT_NOTIFY, MA_DRINK,
       wrap(player));
		  
   if (!IS_INVIS(this_player()) && others != "")
     this_player()->send_message(MT_LOOK, MA_DRINK, wrap(others));
}

/*
FUNKTION: parse_error
DEKLARATION: varargs int parse_error(mixed *parsed, string error[, mixed only_one])
BESCHREIBUNG: 
parse_error ist eine Kopie von parse_com_error, die ihre Meldungen mit
write anstatt notify_fail ausgibt, damit die Befehlsbehandlung mit return 1
abgeschlossen werden kann.
VERWEISE: parse_com_error,  parse_com
GRUPPEN: wasser
*/
varargs int parse_error(mixed *parsed, string error, mixed only_one) 
{
   if (parsed[PARSE_RET_CODE] == PARSE_NOT_SEARCHED)
     return 0;
   if (parsed[PARSE_RET_CODE] != PARSE_OK) {
     if (parsed[PARSE_RET_CODE] == PARSE_NO_ARG ||
         parsed[PARSE_RET_CODE] == PARSE_WRONG_ID)
       write(wrap(error));
     else if (parsed[PARSE_RET_CODE] == PARSE_NO_OB)
       write(wrap(capitalize(parsed[PARSE_ID]) + " nicht gefunden."));
     else if (parsed[PARSE_RET_CODE] == PARSE_NO_MY_OB)
       write(wrap(capitalize(parsed[PARSE_ID]) + " hast Du nicht bei dir."));
     else if (parsed[PARSE_RET_CODE] == PARSE_NO_ALL_OB) 
       if (parsed[PARSE_ID] != "")
         write("Da gibt es nichts Derartiges.\n");
       else
         write("Da gibt es nichts.\n");
     else if (parsed[PARSE_RET_CODE] == PARSE_NO_ALL_MY_OB) 
       if (parsed[PARSE_ID] != "")
         write("Du hast nichts Derartiges bei dir.\n");
       else
         write("Du hast nichts bei dir.\n");
     else
       write("Da hat Francis wohl Mist programmiert.\n");
     
     return 1;
   }

   if (only_one && parsed[PARSE_NUM_OBS] > 1) {
     write(wrap(stringp(only_one) ? only_one : 
         "Immer eines nach dem anderen.\n"));
     return 1;
   }
   
   return 0;
}

private void handle_forbidden_msg(mixed fail, int msg_action, closure standard)
{
    if(stringp(fail))
	fail = ({fail});
    if(pointerp(fail))
    {
	if(sizeof(fail)>1)
    	    this_player()->send_message(MT_LOOK, msg_action, fail[1] && wrap(fail[1]),
        	wrap(fail[0]), this_player());
        else if(sizeof(fail)==1)
            this_player()->send_message_to(this_player(), MT_LOOK, msg_action,
        	wrap(fail[0]));
    }
    else
	this_player()->send_message_to(this_player(), MT_LOOK, msg_action,
	    funcall(standard));
}

/*
FUNKTION: drink_action
DEKLARATION: int drink_action()
BESCHREIBUNG: 
drink_action enthaelt die eigentliche Funktionalitaet fuer das Trinken. Zuerst
wird ueberprueft ob die Flasche etwas enthaelt, wenn ja, dann wird 
drink_action im Wasser-Objekt aufgerufen.
Diese Funktion liefert 1 zurueck, wenn sie sich fuer zustaendig befand und
entsprechende (Erfolgs- oder auch Misserfolgs-)Meldungen ausgegeben hat.
Ansonsten (im notify_fail-Falle) wird 0 zurueckgeliefert.
VERWEISE: drink_command, drink_action (wasser), set_success_message,
set_other_success_message, set_failure_message, set_other_failure_message, 
query_success_message, query_other_success_message, query_failure_message, 
query_other_failure_message
GRUPPEN: wasser
*/
int drink_action() 
{
   mixed fail;
   // ist die Flasche leer?

   if (content <= 0 || !water) {
     return notify_fail(wrap(Dein(OBJ_TO) + 
                             (query_plural() ? " sind" : " ist") + " leer."),
                        FAIL_INTERNAL);
   }

   // Geister koennen nicht trinken

   if (this_player()->query_ghost()) {
     return notify_fail("In deinem vergeistigten Zustand ist das nicht "
                        "möglich.\n", FAIL_INTERNAL);
   }

   fail = this_player()->forbidden("drink_water", water, this_object(), this_player())
       || this_object()->forbidden("drink_water_well", water, this_object(), this_player())
       || water->forbidden("drink_water_self", water, this_object(), this_player());
   if(fail)
   {
      handle_forbidden_msg(fail, MA_DRINK,
	(: wrap(Dein()+ist(this_object(),IST_SPACE_BEFORE)+" leer.") :));
   }
   else if (water->drink_action())
   {
     message(query_success_message(), query_other_success_message());
     this_player()->notify("drink_water",water,this_object(),this_player());
     this_object()->notify("drink_water_well",water,this_object(),this_player());
     if(water) water->notify("drink_water_self",water,this_object(),this_player());
     // Inhalt wird bei erfolgreichem Trinken weniger
     set_content(content-1);
     if(!content) this_object()->notify("bottle_empty",this_object(),0);
     if(!content && water) water->remove();
   }
   else 
     message(query_failure_message(), query_other_failure_message());

   return 1;
}


/*
FUNKTION: notify_bottle_empty
DEKLARATION: void notify_bottle_empty(object bottle, int how)
BESCHREIBUNG:
Nachdem die Flasche bottle durch Trinken (how=0) oder Ausschuetten (how=1)
entleert wurde, wird bottle->notify("bottle_empty",bottle,how) aufgerufen.

Die Funktion notify ruft in allen mit bottle->add_controller(
"notify_bottle_empty", other) angemeldeten Objekten other die Funktionen
other->notify_bottle_empty(bottle,how) auf. Sowohl bottle als auch other haben
dann eine Moeglichkeit, darauf zu reagieren.
VERWEISE: notify, notify_bottle_full, notify_drink_water
GRUPPEN: wasser
*/

/*
FUNKTION: drink_command
DEKLARATION: int drink_command(string s)
BESCHREIBUNG: 
drink_command ist der Parser fuer die Trinkbefehle. Er akzeptiert 

   trinke wasser
   trinke aus flasche
   trinke wasser aus flasche

Wird einer davon erkannt, wird drink_action aufgerufen, wo das eigentliche
Trinken behandelt wird.
VERWEISE: drink_action
GRUPPEN: wasser
*/
int drink_command(string s) 
{
   string rest;

   s = s ? lower_case(trim(s)) : "";

   if (!(sscanf(s, "aus %s", rest) == 1 && me(rest) == "" ||
       (rest = water_me(s)) && 
         (rest == "" || 
         sscanf(rest, "aus %s", rest) == 1 && me(rest) == "")))
     return notify_fail(wrap("Was oder woraus möchtest du trinken?"), FAIL_NOT_OBJ);

   // der Spieler muss die Flasche zum Trinken tragen

   if (environment() != this_player())
     return notify_fail(wrap("Du musst " + den(OBJ_TO) + 
        " erst einmal in die Hand nehmen."), FAIL_INTERNAL);

   return this_object()->drink_action();
}


/*
FUNKTION: fill_from_object
DEKLARATION: int fill_from_object(object well)
BESCHREIBUNG: 
fill_from_object wird von fill_command mit dem Brunnen aufgerufen, an dem
die Flasche gefuellt werden soll. Zuerst wird geprueft, ob die Flasche leer 
ist. Danach wird beim Brunnen mit get_water der maximale Flascheninhalt 
angefordert. Wird die Anforderung oder ein Teil davon erfuellt, wird das 
vom Brunnen erhaltene Wasser-Objekt angemeldet, der Inhalt gesetzt und 
schliesslich die Meldungen fuer das Fuellen der Flasche ausgegeben. Wird die
Anforderung vom Brunnen nicht erfuellt, bleibt alles unveraendert. Es ist
dann Aufgabe des Brunnens, entsprechende Meldungen auszugeben. Anhand des
Rueckgabewertes von fill_from_object entscheidet fill_command dann, ob der
Befehl akzeptiert (!= 0) oder weitergereicht (== 0) werden soll.
VERWEISE: get_water (brunnen), set_water, set_content, fill_from_vitem, 
fill_command, query_fill_message, query_other_fill_message
GRUPPEN: wasser
*/
int fill_from_object(object well)
{
   object new_water;
   int new_content;
   mixed fail;

   // die Flasche muss zum Fuellen leer sein

   if (water && content > 0) {
     write(wrap(Dein(OBJ_TO) + (query_plural() ? " sind" : " ist") +
         " noch mit " + wem(water, ART_KEINS) + " gefüllt."));
     return 1;
   }

   new_water = well->query_water(); // Damit wir's nicht immer wieder abfragen muessen
   fail = !new_water 
       || this_player()->forbidden("fill_water", this_object(), well, new_water, this_player())
       || well->forbidden("fill_water_well", this_object(), well, new_water, this_player())
       || new_water->forbidden("fill_water_self", this_object(), well, new_water, this_player())
       || this_object()->forbidden("fill_water_bottle", this_object(), well, new_water, this_player());
   if(fail)
   {
      handle_forbidden_msg(fail, MA_USE, lambda(0, ({
	(: wrap(Dein($1)+ist($1,IST_SPACE_BEFORE)+" leer.") :),
	well})));
      return 1;
   }
   new_water = 0;

   // maximalen Flascheninhalt beim Brunnen anfordern

   if (well->get_water(&new_water, &new_content, max_content)) {

     // der Brunnen lieferte Wasser

     if (set_water(new_water)) {
       set_content(new_content);     
        new_water->set(P_WATER_ORIGIN,
          ([
            "herkunft":object_name(environment(this_player())),
            "fueller_real_name":this_player()->query_real_name(),
            "fueller_object_name": object_name(this_player()),
            "brunnen_object_name": object_name(this_object()),
            "brunnen_description":
            ([
            "name":           this_object()->query_name(),
            "cap_name":       this_object()->query_cap_name(),
            "gender":         this_object()->query_gender(),
            "adjektiv":       this_object()->query_adjektiv(),
            "plural":         this_object()->query_plural(),
            "personal":       this_object()->query_personal(),
            "personal_title": this_object()->query_personal_title(),
            "eigen":          this_object()->query_eigen(),
            "akkusativ":      this_object()->query_akkusativ(),
            ]),
          ])
       );
       message(query_fill_message(well, new_water),
               query_other_fill_message(well, new_water));
       this_object()->notify("bottle_full",this_object(),well,0);
     }
   } 

   return 1;
}


/*
FUNKTION: fill_from_vitem
DEKLARATION: int fill_from_vitem(mapping well_mapping, mapping water_mapping)
BESCHREIBUNG: 
fill_from_vitem wird von fill_command mit zwei Mappings, einem fuer den 
Brunnen und eines fuer das Wasser, aufgerufen, an denen die Flasche gefuellt 
werden soll. Der Brunnen kann auch fehlen, dann ist well_mapping gleich 0. 
Zuerst wird geprueft, ob die Flasche leer ist. Kann an dem Vitem oder den 
Vitems keine Flasche gefuellt werden und ist ein Grund angegeben, dann wird er
dem Spieler mitgeteilt. Ansonsten wird je nachdem, ob im Mapping fuer das 
Wasser ein Virus angegeben ist ein Gift-Objekt geklont und die 
Virus_Informationen dort dort eingetragen oder ein normales Wasser-Objekt 
geklont. Falls ein "water_file"-Eintrag besteht, wird dieses geclont.
Anschliessend werden in das geklonte Objekt die Eigenschaften des 
Wassers aus dem Vitem uebernommen, das Objekt wird angemeldet und der Inhalt 
gleich dem maximalen Inhalt gesetzt. Anhand des Rueckgabewertes von 
fill_from_object entscheidet fill_command dann, ob der Befehl akzeptiert (!= 0)
oder weitergereicht (== 0) werden soll. 
VERWEISE: set_water, set_content, fill_from_object, fill_command,
query_no_fill_reason, query_fill_message, query_other_fill_message
GRUPPEN: wasser
*/
int fill_from_vitem(mapping well_mapping, mapping water_mapping)
{
   object new_water, room;
   mixed adjektiv, fail;

   // die Flasche muss zum Fuellen leer sein

   if (water && content > 0) {
     write(wrap(Dein(OBJ_TO) + (query_plural() ? " sind" : " ist") +
         " noch mit " + wem(water, ART_KEINS) + " gefüllt."));
     return 1;
   }

   room = environment(this_player());
   fail = this_player()->forbidden("fill_water", this_object(), well_mapping, water_mapping, this_player())
       || (well_mapping && room->forbidden("fill_water_well", this_object(), well_mapping, water_mapping, this_player()))
       || room->forbidden("fill_water_self", this_object(), well_mapping, water_mapping, this_player())
       || this_object()->forbidden("fill_water_bottle", this_object(), well_mapping, water_mapping, this_player());
   if(fail)
   {
      handle_forbidden_msg(fail, MA_USE, lambda(0, ({
        #'wrap, ({#'query_no_fill_reason, well_mapping, water_mapping})})));
      return 1;
   }

   string water_file=water_mapping["water_file"];

   if (water_mapping["virus_name"]) {

     // es ist ein Virus angegeben, also Gift benutzen und Werte eintragen

     new_water = clone_object(water_file||GIFT_OBJ);
     new_water->set_virus_name(water_mapping["virus_name"]);

     if (member(water_mapping, "virus_infection_chance"))
       new_water->set_virus_infection_chance(water_mapping["virus_infection_chance"]);
     if (water_mapping["virus_infection_msg"])
       new_water->set_virus_infection_message(water_mapping["virus_infection_msg"]);
     if (water_mapping["virus_other_infection_msg"])
       new_water->set_virus_other_infection_message(water_mapping["virus_other_infection_msg"]);
     if (member(water_mapping, "virus_str")) 
       new_water->set_virus_str(water_mapping["virus_str"]); 
     if (member(water_mapping, "virus_min")) 
       new_water->set_virus_min(water_mapping["virus_min"]); 
     if (member(water_mapping, "virus_dur")) 
       new_water->set_virus_dur(water_mapping["virus_dur"]); 
     if (member(water_mapping, "virus_chance"))
       new_water->set_virus_chance(water_mapping["virus_chance"]); 
     if (member(water_mapping, "virus_immun_time")) 
       new_water->set_virus_immun_time(water_mapping["virus_immun_time"]);
     if (water_mapping["virus_look"]) 
       new_water->set_virus_look(water_mapping["virus_look"]); 
     if (water_mapping["virus_heal_msg"]) 
       new_water->set_virus_heal_msg(water_mapping["virus_heal_msg"]); 
     if (water_mapping["virus_type"]) 
       new_water->set_virus_type(water_mapping["virus_type"]);
     if (water_mapping["virus_messages"]) 
       new_water->set_virus_messages(water_mapping["virus_messages"]); 
     if (water_mapping["virus_shadow_path"])
       new_water->set_virus_shadow_path(water_mapping["virus_shadow_path"]);
   } else {
  
     // sonst normales Wasser nehmen

     new_water = clone_object(water_file||WASSER_OBJ);
   }

   // Name, Geschlecht, Ids uebernehmen

   new_water->set_name(water_mapping["name"]);
   new_water->set_gender(water_mapping["gender"]);
   new_water->set_id(water_mapping[(member(water_mapping, "id") ? "id" :
                                                                  "name")]);

   // Adjektive setzen

   if (member(water_mapping, "water_adjektiv")) {
     if (intp(adjektiv = water_mapping["water_adjektiv"])) {

       // die ersten n Adjektive des Vitems

       if (water_mapping["adjektiv"]) 
         adjektiv = water_mapping["adjektiv"][0..adjektiv-1];
       else
         adjektiv = 0;
     } else if (pointerp(adjektiv) && intp(adjektiv[0]) && intp(adjektiv[1])) {

       // eine Untermenge der Adjektive des Vitems
 
       if (water_mapping["adjektiv"]) 
         adjektiv = water_mapping["adjektiv"][adjektiv[0]..adjektiv[1]];
       else
         adjektiv = 0;
     } else if (!stringp(adjektiv) && !pointerp(adjektiv))
       raise_error("Unerlaubter Typ für Wasser-Adjektiv in '" + 
           water_mapping["name"] + "'.\n");

       // neue Adjektive

   } else {

     // dieselben Adjektive wie das Vitem

     adjektiv = water_mapping["adjektiv"];
   }

   new_water->set_adjektiv(adjektiv);

   // Eigenschaften und Meldungen des Wassers eintragen

   if (member(water_mapping, "water_strength")) 
     new_water->set_strength(water_mapping["water_strength"]);
   if (member(water_mapping, "water_healing")) 
     new_water->set_healing(water_mapping["water_healing"]);
   if (member(water_mapping, "water_amount"))
     new_water->set_amount(water_mapping["water_amount"]);

   if (water_mapping["water_success_msg"]) 
     new_water->set_success_message(water_mapping["water_success_msg"]);
   if (water_mapping["water_other_success_msg"]) 
     new_water->set_other_success_message(water_mapping["water_other_success_msg"]);
   if (water_mapping["water_failure_msg"]) 
     new_water->set_failure_message(water_mapping["water_failure_msg"]);
   if (water_mapping["water_other_failure_msg"]) 
     new_water->set_other_failure_message(water_mapping["water_other_failure_msg"]);

   // Flasche mit dem Wasser fuellen

   if (set_water(new_water)) {
     set_content(max_content);
     new_water->set(P_WATER_ORIGIN,
          ([
            "herkunft":object_name(environment(this_player())),
            "fueller_real_name":this_player()->query_real_name(),
            "fueller_object_name": object_name(this_player()),
            "brunnen_object_name": object_name(this_object()),
            "brunnen_description": well_mapping,
          ])
       );
         message(query_fill_message(well_mapping, water_mapping), 
             query_other_fill_message(well_mapping, water_mapping));
     this_object()->notify("bottle_full",this_object(),well_mapping,
             water_mapping);
   }

   return 1;
}

/*
FUNKTION: notify_bottle_full
DEKLARATION: void notify_bottle_full(object bottle, mixed well[, mapping water])
BESCHREIBUNG:
Nachdem die Flasche bottle mit Wasser water aus einem Brunnen well
aufgefuellt wurde, wird bottle->notify("bottle_full",bottle,well,water)
aufgerufen. Der Brunnen well ist entweder ein Objekt, ein Mapping oder 0.
Wenn es ein Mapping oder 0 ist, wird als 3. Parameter das Wasser water als
Mapping uebergeben. Ansonsten ist water=0.

Die Funktion notify ruft in allen mit bottle->add_controller(
"notify_bottle_full", other) angemeldeten Objekten other die Funktion
other->notify_bottle_full(bottle,well,water) auf. Sowohl bottle als auch
other haben dann eine Moeglichkeit, darauf zu reagieren.
VERWEISE: notify, notify_bottle_empty
GRUPPEN: wasser
*/

/*
FUNKTION: forbidden_fill_water
DEKLARATION: string* forbidden_fill_water(object flasche, mixed brunnen, mixed wasser, object wer)
BESCHREIBUNG:
Bevor das Lebewesen 'wer' die 'Flasche' mit dem 'Wasser' des 'Brunnen's fuellen
darf, wird wer->forbidden("fill_water", flasche, brunnen, wasser, wer)
aufgerufen. Dabei ist der Brunnen entweder ein Objekt, ein Mapping oder 0.
Das Wasser ist ein Objekt (genau dann, wenn der Brunnen auch ein Objekt ist)
oder ein Mapping (wenn der Brunnen ebenfalls ein Mapping oder 0 ist).

Die Funktion forbidden ruft dann in allen mit wer->add_controller(
"forbidden_fill_water", other) angemeldeten Objekten other die Funktion
other->forbidden_fill_water(flasche, brunnen, wasser, wer) auf. Liefert
diese Funktion einen Wert ungleich 0 zurueck, so wird das Fuellen verboten.

Der Rueckgabewert der Funktion sollte ein Array ({ "Meldung fuer 'wer'",
"Meldung fuer alle Umstehenden" }) sein, welches dann automatisch umgebrochen
und ausgegeben wird.

VERWEISE: forbidden, forbidden_fill_water_self, forbidden_fill_water_well,
          forbidden_fill_water_bottle, notify_bottle_full, notify_bottle_empty
GRUPPEN: wasser
*/

/*
FUNKTION: forbidden_fill_water_well
DEKLARATION: string* forbidden_fill_water_well(object flasche, mixed brunnen, mixed wasser, object wer)
BESCHREIBUNG:
Bevor das Lebewesen 'wer' die 'Flasche' mit dem 'Wasser' des 'Brunnen's fuellen
darf, wird, sofern ein Brunnen vorhanden ist, entweder
 - wenn der Brunnen ein Objekt ist,
   brunnen->forbidden("fill_water_well", flasche, brunnen, wasser, wer) 
   oder
 - wenn der Brunnen ein V-Item ist,
   raum->forbidden("fill_water_well", flasche, brunnen, wasser, wer).
   wobei 'raum' der Raum mit diesem V-Item ist,
aufgerufen. Das Wasser ist ein Objekt (genau dann, wenn der Brunnen auch ein
Objekt ist) oder ein Mapping (wenn der Brunnen ebenfalls ein Mapping ist).

Die Funktion forbidden ruft dann in allen mit brunnen/raum->add_controller(
"forbidden_fill_water_well", other) angemeldeten Objekten other die Funktion
other->forbidden_fill_water_well(flasche, brunnen, wasser, wer) auf. Liefert
diese Funktion einen Wert ungleich 0 zurueck, so wird das Fuellen verboten.

Der Rueckgabewert der Funktion sollte ein Array ({ "Meldung fuer 'wer'",
"Meldung fuer alle Umstehenden" }) sein, welches dann automatisch umgebrochen
und ausgegeben wird.

VERWEISE: forbidden, forbidden_fill_water, forbidden_fill_water_self,
          forbidden_fill_water_bottle, notify_bottle_full, notify_bottle_empty
GRUPPEN: wasser
*/

/*
FUNKTION: forbidden_fill_water_self
DEKLARATION: string* forbidden_fill_water_self(object flasche, mixed brunnen, mixed wasser, object wer)
BESCHREIBUNG:
Bevor das Lebewesen 'wer' die 'Flasche' mit dem 'Wasser' des 'Brunnen's fuellen
darf, wird entweder
 - wenn der Brunnen und das Wasser Objekte sind,
   wasser->forbidden("fill_water_self", flasche, brunnen, wasser, wer) 
   oder
 - wenn das Wasser ein V-Item ist
   (und daher der Brunnen auch ein V-Item oder 0 ist)
   raum->forbidden("fill_water_self", flasche, brunnen, wasser, wer).
   wobei 'raum' der Raum mit diesem V-Item ist,
aufgerufen.

Die Funktion forbidden ruft dann in allen mit wasser/raum->add_controller(
"forbidden_fill_water_self", other) angemeldeten Objekten other die Funktion
other->forbidden_fill_water_self(flasche, brunnen, wasser, wer) auf. Liefert
diese Funktion einen Wert ungleich 0 zurueck, so wird das Fuellen verboten.

Der Rueckgabewert der Funktion sollte ein Array ({ "Meldung fuer 'wer'",
"Meldung fuer alle Umstehenden" }) sein, welches dann automatisch umgebrochen
und ausgegeben wird.

VERWEISE: forbidden, forbidden_fill_water, forbidden_fill_water_well,
          forbidden_fill_water_bottle, notify_bottle_full, notify_bottle_empty
GRUPPEN: wasser
*/

/*
FUNKTION: forbidden_fill_water_bottle
DEKLARATION: string* forbidden_fill_water_bottle(object flasche, mixed brunnen, mixed wasser, object wer)
BESCHREIBUNG:
Bevor das Lebewesen 'wer' die 'Flasche' mit dem 'Wasser' des 'Brunnen's fuellen
darf, wird flasche->forbidden("fill_water_bottle", flasche, brunnen, wasser,
wer) aufgerufen. Dabei ist der Brunnen entweder ein Objekt, ein Mapping oder 0.
Das Wasser ist ein Objekt (genau dann, wenn der Brunnen auch ein Objekt ist)
oder ein Mapping (wenn der Brunnen ebenfalls ein Mapping oder 0 ist).

Die Funktion forbidden ruft dann in allen mit flasche->add_controller(
"forbidden_fill_water_bottle", other) angemeldeten Objekten other die Funktion
other->forbidden_fill_water_bottle(flasche, brunnen, wasser, wer) auf. Liefert
diese Funktion einen Wert ungleich 0 zurueck, so wird das Fuellen verboten.

Der Rueckgabewert der Funktion sollte ein Array ({ "Meldung fuer 'wer'",
"Meldung fuer alle Umstehenden" }) sein, welches dann automatisch umgebrochen
und ausgegeben wird.

VERWEISE: forbidden, forbidden_fill_water, forbidden_fill_water_self,
          forbidden_fill_water_well, notify_bottle_full, notify_bottle_empty
GRUPPEN: wasser
*/

/*
FUNKTION: fill_command
DEKLARATION: int fill_command(string s)
BESCHREIBUNG:
fill_command ist der Parser fuer die Fuellbefehle. Er akzeptiert

   fuelle flasche
   fuelle flasche an/am brunnen
   fuelle flasche an/am brunnen mit wasser
   fuelle flasche mit wasser
   fuelle flasche mit wasser aus brunnen

Sind Wasser und / oder Brunnen nicht angegeben oder nicht eindeutig, wird der
Spieler und seine Umgebung nach passenden Objekten oder Vitems durchsucht.
Je nachdem, ob die Flasche an Objekten oder Vitems gefuellt werden soll, 
wird fill_from_object oder fill_from_vitem aufgerufen. Anhand des Rueckgabe-
wertes dieser Funktionen wird entschieden, ob der Befehl akzeptiert oder
weitergereicht werden soll.
VERWEISE: fill_from_object, fill_from_vitem
GRUPPEN: wasser
*/
int fill_command(string s)
{
   string *water_ids,
          rest,
          water_str,
          well_str;
   mixed *parsed,
         new_water,
         well;
   object room;
   mapping vitem;
   int deny,
       n;


   if (!(rest = me(s))) {
     notify_fail("Was möchtest du füllen?\n");
     return 0;
   }
     
   // im Nirwana gibt es nichts

   if (!(room = environment(this_player()))) {
     notify_fail("Im Nirwana gib es nichts, gar nichts.\n");
     return 0;
   }

   // der Spieler muss die Flasche zum Fuellen tragen

   if (environment() != this_player()) {
     write(wrap("Du musst " + den(OBJ_TO) + " erst einmal in die Hand "
         "nehmen."));
     return 1;
   }

   // in Wasser- und Brunnentext aufteilen
   rest = lower_case(trim(rest));
   if (sscanf(rest, "mit %s aus %s", water_str, well_str) != 2 &&
       sscanf(rest, "mit %s", water_str) != 1 &&
       sscanf(rest, "an %s mit %s", well_str, water_str) != 2 &&
       sscanf(rest, "am %s mit %s", well_str, water_str) != 2 &&
       sscanf(rest, "an %s", well_str) != 1 &&
       sscanf(rest, "am %s", well_str) != 1 &&
       rest != "") {
     write(wrap("Womit möchtest du " + deinen(OBJ_TO) + " auffüllen?"));
     return 1;
   }

   if (well_str) {     
  
     // es wurde ein Brunnen angegeben
     // gibt es den angegebenen Brunnen?

     parsed = parse_com(well_str);
     if (parse_error(parsed, "Womit möchtest du " + deinen(OBJ_TO) +
                             " füllen?"))
       return 1;

     if (parsed[PARSE_REST] != "") {
       write(wrap("Was meinst du mit '" + parsed[PARSE_REST] + "'?"));
       return 1;
     }

     well = parsed[PARSE_OBS][0];

     if (objectp(well)) {

       // der Brunnen ist ein Objekt
       // ist das Objekt wirklich ein Brunnen?

       if (!well->is_well()) {
         write(wrap("Du kannst " + deinen(OBJ_TO) + " nicht an " +
             deinem(well) + " füllen."));
         return 1;
       }

       // enthaelt er das gewuenschte?

       if (water_str && well->water_me(water_str) != "") {
         write(wrap(Dein(well) + " enthält " + 
             wen(well->query_water(), ART_KEINS) + ". " + 
             capitalize(water_str) + " findest du dort nicht."));
         return 1;
       }

       return this_object()->fill_from_object(well);
     } else {
 
       // der Brunnen ist ein Vitem
       // ist das Vitem wirklich ein Brunnen?

       if (!member(well, "id") ||
           (member(well["id"], "# well #") < 0 &&
            !(deny = (member(well["id"], "# no_well #")>=0)))) { 
         write(wrap("Du kannst " + deinen(OBJ_TO) + " nicht an " +
             deinem(well) + " füllen."));
         return 1;
       }
       
       if(objectp(well["environment"]))
           room = well["environment"];
       // zugehoeriges Wasser bestimmen

       if (well["water_id"] &&
           !(new_water = room->query_v_item(({well["water_id"]})))) 
             raise_error(sprintf("V-Item für Wasser '%s' im Brunnen '%s' ist in %O nicht vorhanden.\n",
	    	  well["water_id"], well["name"], room));

       if (water_str) {

         // gehoert das angegebene Wasser zum Brunnen?

         if (!new_water || room->here(water_str, well["water_id"]) != "") {
           if (room->here(water_str, 0, &vitem) != "") 
             write(wrap(capitalize(water_str) + " nicht gefunden."));
           else if (!water)
             write(wrap(Dein(well) + " enthält " +
                 wen(vitem, ART_KEIN) + "."));
           else
             write(wrap(Dein(well) + " enthält " + wen(vitem, ART_KEIN) + 
                 ", sondern " + wen(new_water, ART_KEINS) + "."));
                        
           return 1;
         }
       }
       
       // kann man die Flasche an dem Brunnen / mit dem Wasser nicht 
       // fuellen, eine Meldung ausgeben, ansonsten die Flasche fuellen

       if (deny) {
         write(wrap(query_no_fill_reason(well, new_water)));
         return 1;
       } else if (!new_water)
         raise_error("Brunnen '" + well["name"] + " hat kein Wasser.");
       else
         return this_object()->fill_from_vitem(well, new_water);
     }
   } else {
       
     // es wurde kein Brunnen angegeben
     // Brunnen-Objekt (evtl. mit angegebenem Inhalt) suchen

     for (well = first_inventory(room); well; well = next_inventory(well))
       if (well->is_well() && 
           (!water_str || well->water_me(water_str) == ""))
         return this_object()->fill_from_object(well);

     // es wurde kein Brunnen-Objekt gefunden

     if (water_str) {

       // es wurde Wasser angegeben
       // gibt es das Wasser?

       if (room->here(water_str, 0, &new_water) != "") {
         write(wrap(capitalize(water_str) + " nicht gefunden."));
         return 1;
       }

       // ist es wirklich Wasser?

       if (!member(new_water, "id") ||
           (member(new_water["id"], "# water #") < 0 &&
            !(deny = member(new_water["id"], "# no_water #") >=0))) {
         write(wrap("Du kannst " + deinen(OBJ_TO) + " nicht mit " +
             dem(new_water) + " füllen."));
         return 1;
       }

       // gibt es einen dazugehoerenden Brunnen?
/*
       if (!member(well, "id") ||
           (member(well["id"], "# well #") < 0 &&
            !(deny = (member(well["id"], "# no_well #")>=0)))) { 
         write(wrap("Du kannst " + deinen(OBJ_TO) + " nicht an " +
             deinem(well) + " fuellen."));
         return 1;
       }
*/
       if (new_water["well_id"]) {
         if (!(well = room->query_v_item(({new_water["well_id"]}))) ||
    	    !member(well, "id") ||
	    (member(well["id"], "# well #") < 0 &&
             !(deny = (member(well["id"], "# no_well #") >= 0))))
               raise_error(sprintf("V-Item für Brunnen '%s' des Wassers '%s' ist in %O nicht vorhanden oder kein Brunnen.\n",
	    	  new_water["well_id"], new_water["name"], room));
       } else
         well = 0;

       // kann man die Flasche an dem Brunnen / mit dem Wasser nicht 
       // fuellen, eine Meldung ausgeben, ansonsten die Flasche fuellen

       if (deny) {
         write(wrap(query_no_fill_reason(well, new_water)));
         return 1;
       } else 
         return this_object()->fill_from_vitem(well, new_water);
     }

     // es wurde nichts angegeben
     // Liste mit moeglichen Brunnen / Wasser durchsuchen

     for(water_ids = WATER_ID, n = 0; n < sizeof(water_ids); n++)
       if ((vitem = room->query_v_item(({water_ids[n]}))) &&
           member(vitem, "id")) {

         // es wurde ein Kandidat gefunden

         if (member(vitem["id"], "# water #") >= 0) {
           
           // es ist ein Wasser
           // gibt es einen dazugehoerenden Brunnen?
 
           if (vitem["well_id"]) {
             if (!(well = room->query_v_item(({vitem["well_id"]}))) ||
    	         !member(well, "id") ||
		 (member(well["id"], "# well #") < 0 &&
                  !(deny = (member(well["id"], "# no_well #") >= 0))))
               raise_error(sprintf("V-Item für Brunnen '%s' des Wassers '%s' ist in %O nicht vorhanden oder kein Brunnen.\n",
	    	  vitem["well_id"], vitem["name"], room));
	     if(!deny)
                return this_object()->fill_from_vitem(well, vitem);
           } else
             return this_object()->fill_from_vitem(0, vitem);
         } else if (member(vitem["id"], "# well #") >= 0) {

           // es ist ein Brunnen 
           // das dazugehoerige Wasser suchen

           if (!vitem["water_id"] ||        
               !(new_water = room->query_v_item(({vitem["water_id"]})))) 
             raise_error(sprintf("V-Item für Wasser '%s' im Brunnen '%s' ist in %O nicht vorhanden.\n",
	    	  vitem["water_id"], vitem["name"], room));
           
           return this_object()->fill_from_vitem(vitem, new_water);
         }
       }  

     // es wurde nichts gefunden

     write(wrap("Du kannst nichts finden, womit du " + deinen(OBJ_TO) + 
         " füllen könntest."));
     return 1;
   }
}


/*
FUNKTION: empty_action
DEKLARATION: int empty_action()
BESCHREIBUNG: 
empty_action wird von empty_command, vacate_command und pour_command
zum Leeren der Flasche aufgerufen. Zuerst wird geprueft, ob die Flasche nicht 
bereits leer ist. Danach wird der Inhalt auf 0 gesetzt und die Meldungen fuer 
das Leeren ausgegeben. Anhand des Rueckgabewertes von fill_from_object
entscheiden die aufrufenden Funktionen dann, ob der Befehl akzeptiert (!= 0) 
oder weitergereicht (== 0) werden soll.
VERWEISE: set_content, empty_command, vacate_command, pour_command,
query_empty_message, query_other_empty_message
GRUPPEN: wasser
*/
int empty_action() 
{
   if (content <= 0 || !water) {
     write(wrap(Dein(OBJ_TO) + (query_plural() ? " sind" : " ist") +
         " bereits leer."));
     return 1;
   }

   // Meldungen fuer den Spieler und die anderen

   message(query_empty_message(), query_other_empty_message());
   this_object()->notify("bottle_empty",this_object(),1);

   // Flasche leeren

   set_content(0);
   return 1;
}


/*
FUNKTION: empty_command
DEKLARATION: int empty_command(string s)
BESCHREIBUNG: 
empty_command ist einer der drei Parser fuer die Befehle zum Leeren der
Flasche. Er akzeptiert

  leere flasche
  leere flasche aus
  leere wasser weg
  leere wasser aus

Wird einer davon erkannt, wird empty_action aufgerufen, wo die Flasche
geleert wird. Anhand des Rueckgabewertes von empty_action wird dann
entschieden, ob der Befehl akzeptiert oder weitergereicht werden soll.
VERWEISE: empty_action, vacate_command, pour_command, query_empty_message, 
query_other_empty_message
GRUPPEN: wasser
*/
int empty_command(string s) 
{
   string rest;


   s = s ? lower_case(trim(s)) : "";

   if (!(((rest = me(s)) && (rest == "" || rest == "aus")) ||
         ((rest = water_me(s)) && (rest == "aus" || rest == "weg")))) {
     notify_fail(wrap("Was möchtest du ausleeren?"));
     return 0;
   }

   // der Spieler muss die Flasche zum ausleeren tragen

   if (environment() != this_player()) {
     write(wrap("Du musst " + den(OBJ_TO) + " erst einmal in die Hand "
         "nehmen."));
     return 1;
   }

   return this_object()->empty_action();
}


/*
FUNKTION: vacate_command
DEKLARATION: int vacate_command(string s)
BESCHREIBUNG: 
vacate_command ist einer der drei Parser fuer die Befehle zum Leeren der
Flasche. Er akzeptiert

  entleere Flasche

Wird der Befehl erkannt, wird empty_action aufgerufen, wo die Flasche
geleert wird. Anhand des Rueckgabewertes von empty_action wird dann
entschieden, ob der Befehl akzeptiert oder weitergereicht werden soll.
VERWEISE: empty_action, empty_command, pour_command, query_empty_message, 
query_other_empty_message
GRUPPEN: wasser
*/
int vacate_command(string s) 
{
   if (me(s) != "") {
     notify_fail(wrap("Was möchtest du entleeren?"));
     return 0;
   }

   // der Spieler muss die Flasche zum ausleeren tragen

   if (environment() != this_player()) {
     write(wrap("Du musst " + den(OBJ_TO) + " erst einmal in die Hand "
         "nehmen."));
     return 1;
   }

   return this_object()->empty_action();
}


/*
FUNKTION: pour_command
DEKLARATION: int pour_command(string s)
BESCHREIBUNG: 
pour_command ist einer der drei Parser fuer die Befehle zum Leeren der
Flasche. Er akzeptiert

  schuette wasser weg
  schuette wasser aus

Wird einer davon erkannt, wird empty_action aufgerufen, wo die Flasche
geleert wird. Anhand des Rueckgabewertes von empty_action wird dann
entschieden, ob der Befehl akzeptiert oder weitergereicht werden soll.
VERWEISE: empty_action, empty_command, vacate_command, query_empty_message, 
query_other_empty_message
GRUPPEN: wasser
*/
int pour_command(string s)
{
   string rest;

   s = s ? lower_case(trim(s)) : "";

   if (!((rest = water_me(s)) && (rest == "weg" || rest == "aus"))) {
     notify_fail(wrap("Was möchtest du wegschütten?"));
     return 0;
   }

   // der Spieler muss die Flasche zum wegschuetten tragen

   if (environment() != this_player()) {
     write(wrap("Du musst " + den(OBJ_TO) + " erst einmal in die Hand "
         "nehmen."));
     return 1;
   }

   return this_object()->empty_action();
}


/*
FUNKTION: is_bottle
DEKLARATION: int is_bottle()
BESCHREIBUNG:
Flaschen antworten darauf mit 1.
VERWEISE: is_water (wasser), is_well (brunnen)
GRUPPEN: wasser
*/
int is_bottle() { return 1; }


// nur Wasser darf in den Container

int check_allowed_water(object obj)
{
   if (!obj->is_water()) {
     if (obj->query_getraenk() || obj->material("wasser")) 
       obj->set_not_moved_reason("Auf diese Art und Weise kannst du " +
            deinen(OBJ_TO) + " nicht füllen.");
     else 
       obj->set_not_moved_reason(Dein(OBJ_TO) +
           (query_plural() ? " sind" : " ist") + " nur für "
           "Flüssigkeiten, welche man trinken kann, gedacht.");

     return 1;
   }
   return 0;
}

<int|string> let_not_in(mapping mv_infos)
{
  if(check_allowed_water(mv_infos[MOVE_OBJECT]))
     return 1;
  else
     return ::let_not_in(mv_infos);
}

string query_smell()
{
  string smell;

  smell = ::query_smell();
  if (smell && smell != "\n" && smell != "") return smell;
  if (water) return water->query_smell();
  return smell;
}

// Fuellstand und Inhalt an Beschreibung anhaengen

string query_contents(int flag, object viewer)
{
   string s, stand;


   if (content <= 0 || !water)
     s = Er() + (query_plural() ? " sind" : " ist") + " leer.";
   else if (content <= max_content / 5)
     s = Er() + (query_plural() ? " sind" : " ist") + " mit etwas " + 
         wem(water, ART_KEINS) + " gefüllt.";
   else {
     if (content <= max_content / 4)
       stand = "etwa zu einem Viertel";
     else if (content <= max_content / 3)
       stand = "etwa zu einem Drittel";
     else if (content <= max_content / 2)
       stand = "etwa zur Hälfte";
     else if (content <= 2 * max_content / 3)
       stand = "etwa zu zwei Dritteln";
     else if (content <= 3 * max_content / 4)
       stand = "etwa zu drei Vierteln";
     else if (content == max_content) 
       stand = "ganz";
     else stand = "beinahe ganz";

     s = Er() + (query_plural() ? " sind" : " ist") + " " + stand + " mit " + 
         wem(water, ART_KEINS) + " gefüllt.";
   }

   return wrap(s);
}


void create() 
{
   seteuid(getuid());    // damit die Flasche Wasser klonen kann

   set_name("feldflasche");
   set_id(({"flasche", "feldflasche"}));
   set_gender("weiblich");
   set_long("Die Feldflasche ist rund und aus Glas. Zu ihrem Schutz wurde "
       "sie mit Leder umwickelt und ein Korken bewahrt Flüssigkeiten vor "
       "dem Auslaufen.");
   set_weight(1); 
   set_max_internal_encumbrance(2); // 2 nur zur Sicherheit
   set_min_weight(1); set_max_weight(1);
   set_value(60);
   set_material(({"glas", "leder"}));
   set(P_DONT_SELL_CONTENT,1);// fuer den neuen Laden-Verkauf-myonara
}


int remove()
{
   object *objs;


   // mit der Flasche auch ihren Inhalt entfernen

   objs = all_inventory();
   map_objects(objs, "remove");

   return ::remove();
}


void init() 
{
   // die Befehle anmelden

   add_action("drink_command",  "trinke", -5);
   add_action("fill_command",   "fülle", -4);
   add_action("empty_command",  "leere", -4);
   add_action("vacate_command", "entleere", -7);
   add_action("pour_command",   "schütte", -6);
}
