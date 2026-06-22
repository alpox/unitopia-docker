// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /i/tools/chat.c
// Description: Zufaellige Ausgaben fuer Monster (und andere Objekte)
// Author:      Jesper (13.09.2000)

#pragma save_types
#pragma strong_types

#include <apps.h>

/*   prototypes   */
private varargs mixed add_n(mixed chat_msg, object enemy);
private varargs void add_chat(mixed chat, int flag);
private varargs void add_chat2(mixed chat, int flag);
private varargs void do_chat(mixed msgs, mixed last_msgs, string contr, int chance, int delay, object enemy, int pausedtime);
void add_chats(mixed msgs);
void add_a_chats(mixed msgs);
mixed query_chats();
mixed query_a_chats();
void set_chance(int chance);
int query_chance();
void set_a_chance(int chance);
int query_a_chance();
int set_start_chat(int i);
int query_start_chat();
void load_chat(int chance, mixed chat);
void handle_chat();
void do_attack_chat(object enemy);
void pause_chat(int seconds);
int query_chat_paused();
void pause_a_chat(int seconds);
int query_a_chat_paused();
/* end prototypes */

int exec_command(varargs mixed command);

private int chat_chance, a_chat_chance, start_chat;
private int chatdelay, a_chatdelay, chat_timestamp;
private int chatpaused, a_chatpaused; // time stamps (paused until ...)
private mixed *chats, *last_chats = ({});
private mixed *a_chats, *last_a_chats = ({});


// Fuehrt alle Closures aus und wrapt sie. Meldungen, die per "sage" ausge-
// geben werden sollen werden dann als String zurueckgeliefert, wenn nichts
// ausgegeben werden soll, kommt eine Null zurueck und ansonsten ein Array
// mit Messagetype, Messageaction und Message.
private varargs mixed add_n(mixed chat_msg, object enemy)
{
  mixed arr;
  
  arr = copy(chat_msg);
  
  // Zuerst die Closures ausfuehren:
  if (closurep(arr))
  {
    arr = apply(CLOSURE_CONTAINER->do_bind(arr),
	    enemy ? ({ enemy, this_object() }) : ({ this_object() }));
    
    if (stringp(arr) &&
	(!living(this_object()) ||
	 (enemy && strstr(arr, "!")) ||
	 !strstr(arr, "#")))
       arr = ({0, 0, arr});
  }
  // gimli:
  else if (pointerp(arr))
  {
    if (closurep(arr[2]))
      arr[2] = apply(CLOSURE_CONTAINER->do_bind(arr[2]),
	    enemy ? ({ enemy, this_object() }) : ({ this_object() }));
    if ((sizeof(arr) > 3) && closurep(arr[3]))
      arr[3] = apply(CLOSURE_CONTAINER->do_bind(arr[3]),
	    enemy ? ({ enemy, this_object() }) : ({ this_object() }));
  }

  // Jetzt sind die Meldungen schon Strings oder Arrays fuer send_message
  if (stringp(arr))
  {
    if(!strlen(arr))
	return 0;
    // Mit "#" fangen die normalen Echos an. Mit "!" fangen Befehle an.
    if (!living(this_object()))  arr = ({0, 0, wrap(arr)});
  }
  else
  {
    if (pointerp(arr) && stringp(arr[2]))
    {
      if (!strstr(arr[2], "#") || !strstr(arr[2], "!"))
        arr[2] = arr[2][1..];  // Hier werden die "#" und "!" weggemacht

      // gimli:
      if (enemy && sizeof(arr) == 4)
        arr += ({ enemy });

      arr = map(arr, (: return stringp($1) ? (strlen($1)?wrap($1):$1) : $1; :) );
    }
    else return 0;  // Keine Meldung...
  }

  return arr;
}

private varargs void add_chat(mixed chat, int flag)
{
  // Hier werden Pseudo-Closures verarbeitet, ich merk mir grundsaetzlich
  // alle Ausgaben nur als Closure:
  // gimli:
  if (pointerp(chat))
  {
    chat = copy(chat); // Falls das Array aus einem Master kommt...

    if (stringp(chat[2]))
      chat[2] = mixed_to_closure(chat[2], flag ? ({'enemy, 'tp}) : ({'tp}), 1, 1);
    if ((sizeof(chat) > 3) && stringp(chat[3]))
      chat[3] = mixed_to_closure(chat[3], flag ? ({'enemy, 'tp}) : ({'tp}), 1, 1);
  }
  else if (stringp(chat))
      chat = mixed_to_closure(chat, flag ? ({'enemy, 'tp}) : ({'tp}), 1, 1);

  if (flag) a_chats += ({chat});
  else chats += ({chat});
}

private varargs void add_chat2(mixed msgs, int flag)
{
  // Es wurde ein String oder eine Closure uebergeben:
  if (stringp(msgs) || closurep(msgs))  add_chat(msgs, flag);

  // Es wurde ein Array uebergeben:
  if (pointerp(msgs) && sizeof(msgs))
  {
    // Jedes Element des Array ist ein anderer Array oder ein String,
    // dh. jedes Element ist ne Meldung:
    if (pointerp(msgs[0]) || stringp(msgs[0]) || closurep(msgs[0]))
    {
      map(msgs, #'add_chat2, flag);
    }
    else
    {
      // Das erste Element ist ein Integer (Message-Type):
      if (intp(msgs[0]))   add_chat(msgs, flag);
    }
  }  
}

private varargs void do_chat(mixed msgs, mixed last_msgs,
    string contr, int chance, int delay, object enemy, int pausedtime)
{
    object env;
    mixed tmp;
    mixed msg;
  
    if (!this_object())
        return; // ohne objekt keine meldungen

    if (!sizeof(msgs))
        return; // keine chat-meldungen gesetzt

    if (!query_start_chat())
        return; // chat nicht eingeschaltet
  
    if (time() < pausedtime)
        return; // chat voruebergehend ausgesetzt

    if ((time() - chat_timestamp) <= delay)
        return; // mindestzeitabstand noch nicht erreicht

    if (chance >= 0)
    {
        // chance = wahrscheinlichkeit in Prozent der heart_beats
        if (random(100) >= chance)
            return;
    }
    else
    {
        // -chance = Alle wieviel Heartbeats (im Durchschnitt)
        // -3 bedeutet ca. alle 6 Sekunden
        if (random(-chance))
            return;
    }
    
    chat_timestamp = time();
     
    // Sobald in last_msgs mehr als die Haelfte der Messages enthaelt
    // fallen die ertsen Meldungen aus last_msgs heraus, so dass diese 
    // erneut auftreten koennen.
    if (2*sizeof(last_msgs) > sizeof(msgs))
        last_msgs = last_msgs[<sizeof(msgs)/2..<1];
    
    // Jedes Element aus last_msgs nur einmal aus msgs loeschen.
    foreach(msg: last_msgs)
    {
        msgs = arr_delete(msgs, member(msgs, msg));
    }
    
    // Aus den uebrigen Messages eine auswaehlen
    msg = msgs[random(sizeof(msgs))];
    last_msgs += ({msg});
   
    // Eventuelle Closures ausfuehren und Message auf Gueltigkeit pruefen.
    // Strings und Arrays sind erlaubt, bei falschen Typen (z.B. int)
    // wird von add_n 0 zureuckgegeben. Dann gibts keine Meldung.
    if (!(tmp = add_n(msg, enemy))) 
        return;

    // Abbruch, falls das Objekt in einer Closure zerstoert wurde
    if (!this_object())
        return;
  
    // forbidden-controller ausfuehren
    env = environment(this_object());
    if (this_object()->forbidden(contr, this_object(), tmp) ||
        (env && env->forbidden(contr, this_object(), tmp)))
        return; // jemand hat was dagegen

    if (stringp(tmp))
    {
        if (tmp[0] == '!') 
            exec_command(tmp[1..]);
        else 
            exec_command("sage " + tmp);
    }
    else
    {
        apply(#'call_other, this_object(), "send_message", tmp);
    }
        
    // notify-controller ausfuehren
    this_object()->notify(contr, this_object(), tmp);
    if (env) 
        env->notify(contr, this_object(), tmp);
}

/*
FUNKTION: add_chats
DEKLARATION: void add_chats(mixed msgs)
BESCHREIBUNG:
Damit kann man Meldungen zu denen aus load_chat eintragen, im Gegensatz
zu load_chat werden die alten Meldungen aber nicht geloescht.
Sie Struktur von msgs ist in load_chat beschrieben.

VERWEISE: load_chat, load_a_chat, set_chance, set_start_chat
GRUPPEN: monster
*/

void add_chats(mixed msgs)
{
  if (!pointerp(chats)) chats = ({});
  add_chat2(msgs);
  if (sizeof(chats)) set_start_chat(1);
}

/*
FUNKTION: add_a_chats
DEKLARATION: void add_a_chats(mixed msgs)
BESCHREIBUNG:
Damit kann man Meldungen zu denen aus load_a_chat eintragen, im Gegensatz
zu load_a_chat werden die alten Meldungen aber nicht geloescht.
Sie Struktur von msgs ist in load_a_chat beschrieben.

VERWEISE: load_chat, load_a_chat, set_a_chance, set_start_chat
GRUPPEN: monster
*/

void add_a_chats(mixed msgs)
{
  if (!pointerp(a_chats)) a_chats = ({});
  add_chat2(msgs, 1);
  if (sizeof(a_chats)) set_start_chat(1);
}

// Nur fuer Fans:
mixed query_chats() { return chats; }

mixed query_a_chats() { return a_chats; }

/*
FUNKTION: load_chat
DEKLARATION: void load_chat(int chance, mixed chat)
BESCHREIBUNG:
Mit load_chat kann man Monster oder andere Objekte zufaellig eine Meldung
aus chat sagen lassen.
chance gibt dabei die Wahrscheinlichkeit (in Prozent der heart_beats) an.
Falls chance negativ ist, so gibt es an, alle wieviel Heartbeats (im
Durchschnitt) die Meldung ausgegeben wird. (-3 bedeutet ca. alle 6 Sekunden.)

In chat kann man entweder einzelne Meldungen oder einen Array aus Meldungen
uebergeben. Fuer die Meldungen gibt es wieder zwei Moeglichkeiten, entweder
sind es Arrays fuer send_message(..) oder einfache Strings.

Wenn die Strings mit "#" anfangen, werden sie als normales Echo ausgegeben,
bei "!" also exec_command im Monster aufgerufen, ansonsten wird es als
exec_command("sage", string) ausgegeben.

In den Strings der Meldungen duerfen auch Pseudoclosures drin sein, also
zB. "$Der()" wird auch verstanden.

Beispiele:

  friedwart->load_chat(20,
     ({"Kennt ihr schon den Witz vom Frosch im Mixer?",
       "Kuerzester Mathematikerwitz: Sei Epsilon kleiner Null..."})
     );
  // Zwei Chats, die per "sage" ausgegeben werden.

  friedwart->load_chat(20,
     "!frage Kennt ihr schon den Witz vom Frisch im Mixer?");
  // Nur ein einzelner Chat, der Friedwart etwas fragen laesst.
  
  friedwart->load_chat(20,
    ({ ({MT_NOISE, MA_NOISE, "Die Glocken an der Kappe des Hofnarren " +
         "scheppern."}),
       "Kuerzester Musikerwitz: Zwei Musiker gehen an einer Kneipe " +
         "vorbei..."
    }) );
  // Ein Chat, der per send_message ausgegeben wird und ein Satz, den
  // Friedwart sagen soll. (also beide Varianten der Meldung gemischt)

  friedwart->load_chat(20,
    ({ ({MT_SENSE, MA_SENSE, "Du spuerst, wie dir allmaehlich dein " +
         "Herz in die Hosen rutscht. $Der() scheint das mit Freude " +
         "zu beobachten."}),
       "!frage $den() Bin ich wirklich $der()"
    }) );
  // Nochmal einer fuer send_message und ein String, der als do_command
  // ausgefuehrt wird. Beide enthalten Pseudoclosures.


Wer Lust hat, kann auch Closures verwenden. Sie stehen dann an den Stellen
der Strings. Dabei muss man eigentlich nur beachten, dass das was sie
zurueckliefern, sich eine der beiden Moeglichkeiten fuer Meldungen ergibt,
ansonsten wird einfach nichts ausgegeben. Sie erhalten den NPC als ersten
Parameter.

Beispiele:

  string eine_funktion() { return "Das ist ein String!"; }
  
  mixed eine_zweite_funktion()
  {
    switch (random(3)) {
    case 1: return "Hier kommt ein String.";
    break;
    case 2: return ({ MT_NOISE, MA_NOISE, "Ein lauter Array!" })
    break;
    default: return 0;
  }
  
  // Jetzt noch das load_chat:
  
  friedwart->load_chat(20,
    ({ ({MT_NOISE, MA_NOISE, "Friedwart laeutet eine imaginaeren Glocke."}),
       "!frage Wer bist du?",
       #'eine_funktion,
       ({MT_SENSE, MA_SENSE, #'eine_funktion}),
       #'eine_zweite_funktion,
       ({MT_UNKNOWN, MA_UNKNOWN, #'eine_zweite_funktion})
    }) );
  
  // Hier stehen die normalen Chats mit (Lfun-)Closures zusammen. Die beiden
  // Aufrufe von eine_funktion sorgen immer fuer eine Ausgabe. Bei
  // eine_zweite_funktion nur der erste, wenn er keine 0 liefert, die
  // zweite Variante (im Array) liefert nur was, wenn der String zurueck-
  // gegeben wird (ansonsten passts einfach nicht).


Wenn etwas ausgegeben werden soll, wird vorher im Monster bzw. sonstigem Objekt
und in der Umgebung noch forbidden("chat", object Objekt, mixed chat);
aufgerufen.
Wenn was ausgegeben wurde, kommt noch ein notify("chat", Objekt, chat);
dazu. chat ist dabei die einzelne Meldung (String oder Array) und Objekt
halt das Objekt, das die Meldung ausgibt.

Um load_chat auch in Nicht-Monstern nutzen zu koennen, muss man einfach
"/i/tools/chat" erben und im init chat::init() aufrufen. Sollte das Objekt
einen heart_beat haben, muss man dort auch chat::heart_beat() aufrufen.

VERWEISE: load_a_chat, add_chats, set_chance, set_start_chat
GRUPPEN:  monster
*/

void load_chat(int chance, mixed chat)
{
  chats = ({});
  set_chance(chance);
  add_chats(chat);
}

/*
FUNKTION: load_a_chat
DEKLARATION: void load_a_chat(int chance, mixed chat)
BESCHREIBUNG:
Mit load_a_chat kann man Monster im Kampf zufaellige Ausgaben erzeugen
lassen. chance gibt dabei die Wahrscheinlichkeit und chat die Meldungen an.
Falls chance negativ ist, so gibt es an, alle wieviel Heartbeats (im
Durchschnitt) die Meldung ausgegeben wird. (-3 bedeutet ca. alle 6 Sekunden.)

In chat kann man entweder einzelne Meldungen oder einen Array aus Meldungen
uebergeben. Es gibt zwei Varianten, entweder ein String oder ein Array fuer
send_message. chat hat die gleiche Struktur wie das chat aus load_chat.
Fuer den Angreifer und "den Rest" koennen abweichende Meldungen angegeben
werden, siehe Beispiel unten.

Wenn ein String mit "!" anfaengt, wird er als Kommando (do_command) aufge-
fasst, ansonsten als Echo ausgegeben.

Pseudo-Closures (wie: "$Der()", "$der('enemy)) werden auch verstanden.
Durch das 'enemy kann man auch den Gegner einsetzen, auf den gerade drauf-
gehauen wird.

Statt der Strings koennen auch Closures uebergeben werden, das Ergebnis
muss dann aber zulaessig sein. Sie erhalten als ersten Parameter den Gegner
und als zweiten Parameter das Monster selbst.

Beispiel:

  // Das ist eine Lfun, die im Kampf zufaellig aufgerufen werden soll.
  string boeser_zauberspruch(object enemy)
  {
    auf_kleiner_flamme_roesten(enemy);
    return "!sage Nimm das, du Schurke!";
  }
  
  load_a_chat(10, 
    ({ "$Der() kann nicht verstehen, warum er umgebracht werden soll.",

       "!frage $den() Warum kaempfen wir hier eigentlich",

       ({MT_NOISE, MA_COMM, "$Der() schreit: Hilfe, Hilfe. $Der('enemy) "
         "will mich umbringen"}),

       // Diese Closure wird dann mit dem Feind als Parameter aufgerufen.
       #'boeser_zauberspruch

    }) );

Beim Array kann auch noch eine weitere Meldung angegeben werden, die
dann nur der Angreifer erhaelt:

  load_a_chat(30,
    ({ ({ MT_LOOK | MT_FEEL,
          MA_USE,
          "$Der() schlaegt $dem('enemy) eine unzerbrechliche Vase an den Kopf.",
          "$Der() schlaegt dir eine unzerbrechliche Vase an den Kopf."
    }) }) );


VERWEISE: load_chat, add_a_chats, set_a_chance, set_start_chat
GRUPPEN: monster
*/

void load_a_chat(int chance, mixed chat)
{
  a_chats = ({});
  set_a_chance(chance);
  add_a_chats(chat);
}

/*
FUNKTION: set_chance
DEKLARATION: void set_chance(int chance)
BESCHREIBUNG:
Mit set_chance kann man die Wahrscheinlichkeit (in Prozent) fuer die
Ausgabe der Chats aus load_chat setzen. Wenn chance 0 ist, dann kommt
garantiert nichts durch. Bei chance == 100 wird in jedem heart_beat des
Monsters (oder anderen Objektes) was ausgegeben.

Ein negativer Wert bedeutet das Reziproke der Wahrscheinlichkeit, also alle
wieviel HeartBeats (im Durchschnitt) eine Meldung ausgegeben werden soll.
(-3 bedeutet also bei ca. jedem 3. Heartbeat also alle 6 Sekunden.)

VERWEISE: load_chat, add_chat, load_a_chat, set_start_chat
GRUPPEN: monster
*/

void set_chance(int chance) { chat_chance = chance; }

int query_chance() { return chat_chance; }

/*
FUNKTION: set_a_chance
DEKLARATION: void set_a_chance(int chance)
BESCHREIBUNG:
Mit set_chance kann man die Wahrscheinlichkeit (in Prozent) fuer die
Ausgabe der Chats aus load_a_chat setzen. Wenn chance 0 ist, dann kommt
garantiert nichts durch. Bei chance == 100 wird in jeder Runde des Kampfes
was ausgegeben.

Ein negativer Wert bedeutet das Reziproke der Wahrscheinlichkeit, also alle
wieviel HeartBeats (im Durchschnitt) eine Meldung ausgegeben werden soll.
(-3 bedeutet also bei ca. jedem 3. Heartbeat also alle 6 Sekunden.)

VERWEISE: load_a_chat, add_a_chats, load_chat, set_start_chat
GRUPPEN: monster
*/

void set_a_chance(int chance) { a_chat_chance = chance; }

int query_a_chance() { return a_chat_chance; }

/*
FUNKTION: set_chat_delay
DEKLARATION: void set_chat_delay(int delay)
BESCHREIBUNG:
Setzt die garantierte Mindestruhezeit zwischen zwei chats
auf 'delay' Sekunden.

Das bedeutet, dass 'delay' Sekunden nach einer Aktion die naechste Aktion
in jedem weiteren Herzschlag
- bei positivem query_chance() mit Wahrscheinlichkeit query_chance() von 100
- bei negativem query_chance() mit Wahrscheinlichkeit 1 von -query_chance()
kommen kann.
Nach Ausfuehrung der Aktion wird die Referenzzeit fuer 'delay' neu gesetzt.

VERWEISE: load_chat, add_chat, load_a_chat, set_start_chat, set_chance
GRUPPEN: monster
*/
void set_chat_delay(int delay) { chatdelay = max(delay,0); }

int query_chat_delay() { return chatdelay; }

/*
FUNKTION: set_a_chat_delay
DEKLARATION: void set_a_chat_delay(int delay)
BESCHREIBUNG:
Setzt die garantierte Mindestruhezeit zwischen zwei a_chats
auf 'delay' Sekunden.

Das bedeutet, dass 'delay' Sekunden nach einer Aktion die naechste Aktion
in jedem weiteren Herzschlag
- bei positivem query_a_chance() mit Wahrscheinlichkeit query_a_chance() von 100
- bei negativem query_a_chance() mit Wahrscheinlichkeit 1 von -query_a_chance()
kommen kann.
Nach Ausfuehrung der Aktion wird die Referenzzeit fuer 'delay' neu gesetzt.

VERWEISE: load_a_chat, add_a_chats, load_chat, set_start_chat, set_a_chance
GRUPPEN: monster
*/
void set_a_chat_delay(int delay) { a_chatdelay = max(delay,0); }

int query_a_chat_delay() { return a_chatdelay; }

/*
FUNKTION: set_start_chat
DEKLARATION: int set_start_chat(int i)
BESCHREIBUNG:
Mit set_start_chat kann man die Meldungen von load_chat und load_a_chat
an- und ausschalten. Es liefert den neuen Wert zurueck.

VERWEISE: query_start_chat, load_chat, load_a_chat
GRUPPEN: monster
*/

int set_start_chat(int i)
{
  if (start_chat = i)
  {
    if (!living(this_object()) ) { set_heart_beat(1); }
  }
  return start_chat;
}

/*
FUNKTION: query_start_chat
DEKLARATION: int query_start_chat()
BESCHREIBUNG:
Lierfert zurueck, ob die Meldungen von load_chat und load_a_chat grad an-
oder ausgeschaltet sind.

VERWEISE: query_start_chat, load_chat, load_a_chat
GRUPPEN: monster
*/

int query_start_chat() { return start_chat; }

/*
FUNKTION: pause_chat
DEKLARATION: void pause_chat(int seconds)
BESCHREIBUNG:
Mit pause_chat kann man die Chat-Meldungen fuer einen gewissen Zeitraum 
aussetzen. Dies hat keinen Effekt, wenn die Meldungen gar nicht aktiviert 
sind (z.B. query_start_chat()==0).
Kampf-Chat-Meldungen sind davon nicht betroffen, dafuer gibt es die Funktion
pause_a_chat.
Mit query_chat_paused kann man abfragen, ob und bis wann die Meldungen
ausgesetzt sind.
VERWEISE: query_chat_paused, pause_a_chat, set_start_chat, load_chat
GRUPPEN: monster
*/
void pause_chat(int seconds)
{
    chatpaused = time() + seconds;
}

/*
FUNKTION: query_chat_paused
DEKLARATION: int query_chat_paused()
BESCHREIBUNG:
Mit query_chat_paused kann man abfragen, wie lange die Chat-Meldungen 
ausgesetzt sind. Es wird die Restdauer der Pause in Sekunden zurueckgegeben.
VERWEISE: pause_chat, query_a_chat_paused, load_chat
GRUPPEN: monster
*/
int query_chat_paused()
{
    if (chatpaused && (chatpaused < time()))
    {
        chatpaused = 0;
    }

    if (!chatpaused)
        return 0;

    return chatpaused - time();
}

/*
FUNKTION: pause_a_chat
DEKLARATION: void pause_a_chat(int seconds)
BESCHREIBUNG:
Mit pause_a_chat kann man die Kampf-Chat-Meldungen fuer einen gewissen 
Zeitraum aussetzen. Dies hat keinen Effekt, wenn die Meldungen gar nicht 
aktiviert sind (z.B. query_start_chat()==0).
Kampfmeldungen sind damit nicht betroffen, dafuer gibt es die Funktion
pause_a_chat.
Mit query_chat_paused kann man abfragen, ob und bis wann die Meldungen
ausgesetzt sind.
VERWEISE: query_a_chat_paused, pause_a_chat, set_start_chat, load_a_chat
GRUPPEN: monster
*/
void pause_a_chat(int seconds)
{
    a_chatpaused = time() + seconds;
}

/*
FUNKTION: query_a_chat_paused
DEKLARATION: int query_a_chat_paused()
BESCHREIBUNG:
Mit query_a_chat_paused kann man abfragen, wie lange die Kampf-Chat-Meldungen 
ausgesetzt sind. Es wird die Restdauer der Pause in Sekunden zurueckgegeben.
VERWEISE: pause_a_chat, query_chat_paused, load_a_chat
GRUPPEN: monster
*/
int query_a_chat_paused()
{
    if (a_chatpaused && (a_chatpaused < time()))
    {
        a_chatpaused = 0;
    }

    if (!a_chatpaused)
        return 0;

    return a_chatpaused - time();
}


// Die naechsten beiden sind intern:

void handle_chat()
{
    object enemy = 0;
    do_chat(chats, &last_chats, "chat", 
        chat_chance, chatdelay, enemy, chatpaused);
}

void do_attack_chat(object enemy)
{
  if (living(this_object()))
        do_chat(a_chats, &last_a_chats, "a_chat", 
            a_chat_chance, a_chatdelay, enemy, a_chatpaused);
}

/*
 * Die folgenden beiden Funktionen werden gebraucht, falls das chat von
 * Nicht-Monstern geerbt wird. Bei Monstern sind sie belanglos.
 */
 
void heart_beat()
{
  if (query_start_chat() && sizeof(chats) &&
      player_present( (all_environment()||({this_object()}))[<1]) )
  {
    handle_chat();
  }
  else set_heart_beat(0);
}

void init()
{
  if (chat_chance && query_start_chat() && sizeof(chats))
  {
    set_heart_beat(1);
  }
}

// --------------------------------- EOF ----------------------------------
