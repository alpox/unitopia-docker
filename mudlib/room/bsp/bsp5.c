// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/room/bsp/bsp5.c
// Description:	Beispielraum
// Author:	

inherit "/i/room";

#ifdef UNItopia
#include "/d/Vaniorh/sys/vaniorh_exports.h"
#endif
#include <stats.h>
#include <message.h>

#include <monster.h>
//      Enthaelt Defines wie PARSE_SAY fuer set_parse_conversation.

#include <room_types.h>
// Das brauchen wir spaeter fuer ein Define bei add_type().

void reset() 
{
    object mon;
//	Es wird eine Objekt-Variable mon deklariert, welche das Monster
//	aufnehmen soll.

    if(!present("waschbär",this_object()))
    {
//	Diese Abfrage wie gehabt. Wenn sich nicht schon ein Waschbaer in
//	diesem Raum befindet, wird das folgende ausgefuehrt.

      mon = clone_object("/obj/monster");
//	Hier wird ein Standard-Monster gecloned. Von der Datei /obj/monster.c
//	wird eine Kopie angefertigt und deren Adresse an die Objekt-Variable
//	mon uebergeben. Beachte: Mit clone_object() wird immer eine KOPIE
//	eines Objektes angefertigt, mon ist nicht mit /obj/monster.c
//	identisch! (dazu auch /doc/funktionsweisen/clones)
//	Wer genaueres ueber die Funktionen in /obj/monster.c erfahren will
//	schaue sich dieses File mit 'more /i/monster/monster.c' durch!

      mon.initialize("tier", 20);
//	Der Aufruf der Funktion initialize() ist immer als erstes zu
//	taetigen. Im ersten Argument wird die Rasse des Monsters uebergeben,
//	im zweiten Argument die Faehigkeitsstufe des Monsters. Hier ist die
//	Rasse "tier" und die Stufe ist 20 (Stufen sind im Bereich 1-100% 
//	moeglich).
//	Zur detaillierteren Beschreibung der Eigenschaften eines Monster
//	gibt es noch weitere Funktionen, die nach initialize aufgerufen
//	werden koenne. Sie sind in /doc/funktionsweisen/monster/design
//	beschrieben.
//	Die restlichen Werte werden in diesem Fall der Stufe des
//	Monsters entsprechend initialisiert.
//	Diese Funktion ist fuer alle Monster im Spiel PFLICHT!
//      [In anderen Muds heisst das etwas vornehmer NPC, 
//       NonPlayerCharacter, aber wir moegens halt richtig derb hier...,
//       man kann also auch intelligente Lebensformen mit /obj/monster
//       erschaffen. :-) ]

      mon.set_name("waschbär");
//	Hier bekommt das Monster einen Namen. 
//	Namen werden immer in den Meldungen eingesetzt, 
//	in denen das Monster etwas tut oder etwas mit dem Monster getan wird.
//	Diese Funktion ist fuer jeden im Spiel vorkommenden Gegenstand/
//	Monster PFLICHT!

      mon.set_gender("maennlich");
//	Die Funktion set_gender() setzt das Geschlecht des Objektes. Das
//	ist vor allem im Zusammenhang mit der Grammatik von Bedeutung.
//	Mehr dazu mit 'more /doc/funktionsweisen/grammatik/deklin'.
//	Diese Funktion ist fuer jeden im Spiel vorkommenden Gegenstand/
//	Monster PFLICHT!

      mon.set_id( ({"waschbär","bär"}) );
//	Hier werden die IDs fuer das Monster uebergeben. Auf diese
//	IDs reagiert der Parser und ermittelt damit das Monster, bzw. das
//	Objekt, welches durch ein Spieler-Kommando angesprochen wird.
//	Mit der Eingabe 'haetschle baer' erkennt somit der Parser, das hier
//	unser Objekt mon gemeint ist und haetschelt dieses! Es kommt dann
//	die Meldung an den Spieler zurueck: 'Du haetschelst den Waschbaer',
//	d.h. hier wird der mit set_name() gesetzte Name verwendet (nach 
//	dem er durch ein wenig deutsche Grammatik verschoenert wurde.)
//	Jeder Gegenstand/Monster im Spiel sollte ein ID haben.
//	Sinnvoll ist es auch die Teilstuecke eines Wortes als ID zu setzen,
//	damit man als Spieler nicht soviel tippen muss.
//      Bei waschbaer also baer, bei eintagsfliege also auch fliege, usw.

      mon.set_short("Ein kleiner Waschbär");
//	Um die Verwirrung komplett zu machen, gibt es hier noch einmal so
//	etwas wie einen Namen fuer das Objekt mon! Mit set_short() wird die
//	Kurzbeschreibung des Objektes uebergeben. Diese erscheint immer
//	in Raumbeschreibungen bzw. im Ausruestungs-Verzeichnis. 
//	Waehrend set_name(), set_gender() und set_id() immer gesetzt sein
//	muessen, dient set_short() nur der weiteren Ausschmueckung.
//	(Ist set_short nicht gesetzt, dann wird aus dem Namen, den Adjektiven
//	und dem Geschlecht des Waschbaers ein passende Beschreibung 
//	erzeugt, die meist auch ihren Zweck erfuellt.)
//	Meistens ist es NICHT sinnvoll set_short zu setzen, von Raeumen
//	abgesehen, dort ist es Pflicht.

//	Dazu kommt noch...

      mon.set_long(
	 "Ein kleiner quietschvergnügter Waschbär. Er "
         "schaut Dich mit seinen dunklen großen Augen an und schlägt "
         "ein paar unbeholfene Purzelbäume. Das ist wirklich der "
         "absolute Gipfel der Drolligkeit!");
//	Das ist die ausfuehrlichere Beschreibung des Objekts mon. Diese
//	erhaelt der Spieler wenn er das Objekt anschaut (bei Raeumen war
//	das ja etwas anders: da kommt die lange Beschreibung, wenn man in
//	einen Raum hineinlaeuft.).
//	Auch hier erfolgt der Zeilenumbruch automatisch.

      mon.set_feel(
          "Als Du den Waschbären berührst, zuckt er ängstlich zusammen, "
          "er läuft jedoch glücklicherweise nicht davon. Er hat ein "
          "weiches, wuschliges Fell.");
//	Genauso geht das auch beim Geruch (set_smell), Geraeusch (set_noise)
//	usw, hier exemplarisch mal mit set_feel vorgefuehrt.

      mon.set_one_stat(STAT_INT,200);
//      Unser Waschbaer ist ueberdurchschnittlich intelligent, damit er
//      sprechen kann.

      mon.set_aggressive(0);
//	Dieser Funktionsaufruf ist eigentlich ueberfluessig, da Monster
//	per Default nicht aggressiv sind. Wuerde man statt der '0' eine '1'
//	uebergeben, waere das Monster mon aggressiv, d.h. es wuerde sofort
//	alle Spieler angreifen, die sich im Raum blicken lassen. Mit der
//	uebergebenen '0' wird das Monster wieder auf nicht-aggressiv
//	geschaltet.

      mon.set_parse_conversation( this_object(),
         ({ "hallo:     hi || guten && tag || hallo", PARSE_SAY|PARSE_CONTINUE,
            "hallo:     <begruess>", PARSE_SOUL,
            "befinden:  wie && geht", PARSE_SAY,
            "emote:     <wuschel>", PARSE_SOUL }) );
//	Ein (sehr) kleines Beispiel zum Setzen des Conversation-Parsers.
//	Das Monster reagiert auf die Meldungen der Spieler, die jeweils
//	rechts vom Doppelpunkt aufgelistet sind und springt dann, wenn
//	die Meldungen passen, zur Funktion mit dem Namen links vom Doppel-
//	punkt. Mit 'more /doc/funktionsweisen/monster/kommunikation' erhaeltst
//	Du genaueres zu diesem recht umfangreichen Befehl. Im Raum kannst
//	Du ja einmal 'sage Hi!' 'sage hallo wie geht es' 'sage wie geht es' 
//      und 'wuschel waschbaer'eingeben...

      mon.add_random_activities( ([
		"!sage Ich bin ein sprechender Waschbär." : 0,
		"!sage Holla! Ich wasche so wahnsinnig gerne!" : 0,
		"!echo $Der() schaut sich um." : 0,
		"!echo $Der() wäscht seine Pfoten im Bach." : 0,
		"!nehme alles" : 0
		]) );
      mon.set_activity(20);
      mon.set_start_random(1);
//	Mit add_random_activities(..) bringt man einem Monster
//      Zufallsgesteuertes Handeln bei. Naeheres findet man im File
//	/i/monster/random_start.c.
//
//       Mit set_activity bestimmt man die
//      Wahrscheinlichkeit in %, mit der etwas ausgefuehrt wird.
//      [Praeziser: Alle 2 Sekunden wird mit 20 prozentiger Chance eine
//		    der uebergebenen Aktivitaeten ausgefuehrt.]
//
//	Mit set_start_random(..) kann man das ganze ein- und wieder
//	ausschalten.


      mon.load_a_chat(50,
	 ({ "Der Waschbär macht ein böses Gesicht!",
	    "Der Waschbär rubbelt Dich durch sein Waschbrett!",
	    "!nehme alles",
	    "Der Waschbär spritzt Dir Seife in die Augen!" }) );
//      Mit load_a_chat() kann man das Monster bestimmte Aktionen ausfuehren
//	lassen, waehrend es in einem Kampf verwickelt ist.
//	random_activities werden waehrend eines Kampfes NICHT ausgefuehrt.
//
//      Argument (die 50) gibt an, wie oft das Monster handeln soll. Je
//      hoeher die Zahl, desto oefter handelt das Monster.
//      [Praeziser: Alle 2 Sekunden wird mit 50 prozentiger Chance einer
//                  der uebergebenen Texte gesagt.]
//      Zweites Argument ist ein String-Array. Aus diesem Array sucht sich 
//      spaeter das Monster einen zufaelligen String aus,
//      welchen das Monster dann mit echo ausfuehrt.
//
//	AUSNAHME: Ein ! am Anfang laesst das nachfolgende als Kommando
//		  ausfuehren.
//
//		  Man kann auch Closures uebergeben; diese werden zur dann
//		  zur Laufzeit ausgewertet.
//		  Naeheres zu Closures findet man in /doc/LPC/closures.


      mon.add_v_item(
         ([ "name":   "pfote",
	    "id":     ({"pfote","natural#weapon"}),
	    "gender": "weiblich",
	    "long":   "Eine niedliche, kleine Pfote, welche der Waschbär "
	              "da hat."
	 ]));
	 
      mon.add_v_item(
         ([ "name":   "pfote",
	    "id":     ({"pfote","natural#weapon"}),
	    "gender": "weiblich",
	    "long":   "Oh, noch eine zweite niedliche, kleine Pfote, welche "
	              "der Waschbär auch besitzt."
	 ]));
//      Der Waschbaer erhaelt 2 Details "Pfote". Die ID "natural#weapon"
//      bewirkt, dass die Pfoten anstatt von Haenden in den Kampfmeldungen
//      genutzt werden.

      mon.move(this_object());
//	Jetzt wird das Objekt mon noch in diesen Raum bewegt...
//	... damit wir auch was davon haben :)

   }
// Ende des if s
}
// Ende des reset()



void create() {
   set_short("Beispielraum Nr. 5");
   set_long(
      "Du bist im Beispielraum Nr. 5 des Einführungskurses für "
      "Götter. Mit dem Zauberstab-Kommando 'zmore hier' kannst Du den "
      "Quellcode inkl.Kommentare dieses Raumes "
      "mit 'bsp? raum_beispiel5' den Quellcode ohne Kommentare anschauen. "
      "In diesem Raum wird ein Monster "
      "erschaffen und vollständig initialisiert. Das Monster reagiert "
      "auch auf Ansprache..."
#ifdef UNItopia
      "\nNach unten verzweigt ein Hilfekursabstecher zu Parsecs Talk-NPCs, "
      "der Hauptkurs setzt sich im Süden fort."
#endif
      );
   set_exits( ({"bsp6",
                "bsp4",
#ifdef UNItopia
		TALK_NPC_KURS,
#endif
		}),
              ({"süden","osten",
#ifdef UNItopia
	      "runter",
#endif
	      }) );

    add_v_item(
	([ "name":   "bächlein",
	   "id":     ({"bach", "bächlein"}),
	   "gender": "saechlich",
	   "long":   "Ein kleines Bächlein, welches sich hier seinen "
		     "Weg durch den Beispielraum bahnt. Er plätschert "
		     "lustig vor sich hin.",
	   "noise":  "Plätscher, plätscher, plätscher.",
	   "feel":   "Ihhh, das ist ja nass.",
	 ]));


   reset();
//	Das Monster wird im reset() gecloned, damit es immer wieder erscheint
//	wenn es z.B. getoetet wird.
   add_type(RT_TELEPORT_REIN_VERBOTEN, 1);
}

void hallo(string str, string verb, object npc, object player, int flags) 
{
   npc.send_message(MT_LOOK, MA_EMOTE,
                     "Der Waschbär verneigt sich.\n");
//	send_message() schickt eine Meldung an alle Objekt im gleichen Raum,
//	schliesst das Aufrufende aber aus (hier npc).
//
//	Da der Waschbaer was machen soll, wird send_message() im Waschbaer
//	aufgerufen. Man bekommt den Waschbaeren als Parameter npc im Aufruf
//      der Funktion hallo.
}

void befinden(string str, string verb, object npc, object player, int flags) 
{
   npc.do_command("sage Mir gehts gut, danke!");
//	Damit wird der Waschbaer aufgefordert etwas zu sagen. 
//	do_command(aktion)  laesst das Monster den Befehl  aktion 
//	ausfuehren, wie du ihn auch fuer dich eingeben wuerdest.
//
//	Natuerlich haette man auch wie oben mit
//	npc.send_message() arbeiten koennen...
}

void emote(string str, string verb, object npc, object player, int flags) 
{
   npc.send_message(MT_LOOK, MA_EMOTE,
                     wrap(Der(npc)+" strahlt "+den(player)+" verwuschelt an."),
                     wrap(Der(npc)+" strahlt dich verwuschelt an."),
                     player);
//      Hier gibts nun getrennte Meldungen fuer den 'Taeter' und alle anderen
//      im Raum. Das Objekt npc (der Waschbaer) bekommt wieder keine der
//      Meldungen, da in ihm send_message aufgerufen wird.
//      Der 'Taeter' player bekommt die unteren Meldung, alle anderen die obere.
}
