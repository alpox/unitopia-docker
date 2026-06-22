// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/i/living/legs.c
// Description:
// Modified by:	Monty (06.12 96) Die graessliche alte Funktion random_move()
//			ersetzt. Mann war das Teil angestaubt!
//		Freaky (29.07.97) Random-move und runaway geaendert, so dass
//				  NCS's automatisch zurueckkommen.
//		Freaky (10.03.1998) message auf send_message umgebaut.

#pragma save_types
#pragma strong_types

#include <commands.h>
#include <control.h>
#include <move.h>
#include <room.h>
#include <landschaft.h>
#include <message.h>
#include <monster_master.h>
#include <notify_fail.h>
#include <parse_com.h>
#include <misc.h>
#include <add_hp.h>

private static int last_room_was_wet;


/*
FUNKTION: query_schwimmhilfe
DEKLARATION: int query_schwimmhilfe()
BESCHREIBUNG:
Verhindert bei Returnwert != 0 den AP-Abzug beim Schwimmen des 
umgebenden Lebewesens.
Bei anziehbaren Schwimmhilfen sollte auch query_worn() beachtet 
werden.
VERWEISE: move, query_worn
GRUPPEN: spieler
*/

// Prototypes:
void add_sum_move(int i);
varargs void notify_message(string msg, int type);

#define DIRS ({ "norden","nordwesten","westen","suedwesten", \
		"sueden","suedosten","osten","nordosten", \
		"hoch","runter","rein","raus","ausgang" })

static void handle_swimming()
{
    int raumtyp;

    if (!this_object() || !environment())
	return;
    raumtyp = environment()->query_type(LANDSCHAFT);
    if (raumtyp &&
       ((raumtyp & (L_WASSER | L_FLIESSEND | L_UNTERWASSER)) == raumtyp) &&
       !this_object()->query_ghost() &&
       !(this_object()->query_abilities() & MD_AB_SWIM) &&
       ((raumtyp & L_UNTERWASSER) || !(this_object()->query_abilities() & MD_AB_FLY)) )
    {
       if (!sizeof(filter_objects(all_inventory(),"query_schwimmhilfe"))
              && last_room_was_wet)
       {
          this_object()->add_hp(
	     -1-random(this_object()->query_internal_encumbrance()/5),
	     ([
	        AH_ERF_TOD: "Du hast Dich beim Schwimmen übernommen und bist ertrunken.",
		AH_DAMAGE_TYPE: ({ "anstrengung" }),
	     ]));
	  if (!this_object())
	      return;
          else if (this_object()->query_hp()>50)
	      notify_message("Das Schwimmen nagt an Deiner Ausdauer.\n",
	          MA_MOVE);
          else if (this_object()->query_hp()>20)
	      notify_message("Das Schwimmen setzt Dir ganz unschön zu.\n"
	          "Du solltest Dich erstmal wieder etwas erholen.\n"
	          "Vielleicht hilft es auch, Ballast loszuwerden.\n",
	          MA_MOVE);
          else if (this_object()->query_hp()>=0)
              notify_message("Das Schwimmen setzt Dir gewaltig zu.\n"
                  "Du solltest Dich unbedingt erholen und Dich unnötigen "
                  "Ballasts entledigen.\n",
	          MA_MOVE);
        }
	last_room_was_wet = 1;
    }
    else
	last_room_was_wet = 0;
}

static int go_command(string dir)
{
    object env;
    string dest, reason;

    if(!(env = environment()))
	return 0;

    if(member((["laufe","lauf","geh","gehe"]), query_verb()))
    {
	if (!dir)
	    return notify_fail("Gehe wohin?\n", FAIL_NOT_OBJ);

	sscanf(dir, "nach %s", dir);
	dir = lower_case(dir);
    }
    else
    {
	if (dir)
	    return 0;

	dir = lower_case(query_verb_ascii());
    }

#if 0  //Vorerst deaktiviert
    //Bei zu hohem Alkoholpegel den falschen Ausgang erwischen...
    if(member(DIRS[0..7],dir)>=0
      && random(50)<this_object()->query_alc()-5)
    {
      string *dirs=env->query_command_list(
        EXIT_ATOM_NOT|EXIT_ATOM_LOCKED|EXIT_ATOM_HIDDEN)&DIRS[0..7];
      if(sizeof(dirs)) dir=dirs[random(sizeof(dirs))];
    }
#endif

    dest = env->query_one_exit(dir);

    if (dest && dest != "")
    {
	if (this_object()->move(dir, ([
	        MOVE_FLAGS: MOVE_NORMAL,
	        MOVE_TYPE: (dir in ({"ausgang","verlasse","verlassen"})) ? MOVE_TYPE_VERLASSEN : MOVE_TYPE_GEHEN,
	    ]) ) == MOVE_OK)
	{
	    add_sum_move(1);
	    handle_swimming();
	}
	return 1;
    }

    if(this_object()->query_ghost() && (dest = env->query_one_exit(dir,1)))
    {
	mapping info = ([]);
	
	if(this_object()->query_sp()<2)
	    return notify_fail(wrap(
		"Du hast nicht genug "+this_object()->query_sp_name()+"."));

	this_player()->send_message_to(this_player(), MT_NOTIFY, MA_MOVE,
	    "Du konzentrierst Dich...\n");
	this_object()->add_sp(-2);

	if(!environment()->allowed("ghost_pass", this_object(), dir, dest, info))
        {
    	    this_object()->send_message_to(this_object(),
            	MT_FEEL|MT_NOTIFY, MA_MOVE,
                "...leider erfolglos.\n");
	    return 1;
	}

	if (this_object()->move(dir, ([
            MOVE_FLAGS:     MOVE_GHOST,
            MOVE_TYPE:      (dir in ({"ausgang","verlasse","verlassen"})) ? MOVE_TYPE_VERLASSEN : MOVE_TYPE_GEHEN,
            MOVE_MSG_LEAVE: info[MOVE_MSG_OUT],
            MOVE_MSG_ENTER: info[MOVE_MSG_IN] ])) == MOVE_OK)
        add_sum_move(1);

	return 1;
    }

    // Fehlgeschlagen:
    
    if (reason = env->query_lock_reason_other(dir))
    {
	this_player()->send_message(MT_LOOK, MA_MOVE, wrap(reason));
	this_player()->send_message_to(this_player(), MT_NOTIFY, MA_MOVE,
	    wrap(env->query_lock_reason(dir)||"Da geht's nicht weiter."));
	return 1;
    }
    else if (reason = env->query_lock_reason(dir))
        notify_fail (wrap (reason));
    else if (member(DIRS,dir) >= 0)
        notify_fail("Da geht's nicht weiter.\n");

    return 0;
}
/*
FUNKTION: allowed_ghost_pass
DEKLARATION: int allowed_ghost_pass(object player, string dir, string room, mapping info)
BESCHREIBUNG:
Wenn ein Geist auf einen gesperrten Ausgang trifft und diesen durchqueren
will, so wird bei allen am Raum angemeldeten Controllern allowed_ghost_pass
aufgerufen. Liefert ein Controller 1, so wird dem Geist erlaubt, da durch-
zugehen. 'dir' ist der Name des Ausgangs, 'dest' der dahinterliegende Raum.

'info' ist ein Mapping, in welchem der erfolgreiche Controller folgende
Eintraege vornehmen soll:

    MOVE_MSG_OUT: Die Meldung beim Verlassen des Raumes (wie normalerweise
                  bei move() als 3. Parameter angegeben.)
    MOVE_MSG_IN:  Die Meldung beim Betreten des Raeumes (wie normalerweise
                  bei move() als 4. Paraemter angegeben).

VERWEISE: move
GRUPPEN: spieler
*/

/*
FUNKTION: random_move
DEKLARATION: varargs string random_move(int return_time, string verlassen, string ankommen)
BESCHREIBUNG:
random_move fuehrt eine Bewegung in eine offene Richtung aus dem momentanen
Raum heraus aus. In dem zurueckgelieferten String steht die gewaehlte Richtung.
Zusaetzlich kann man der Funktion Bewegungsmeldungen (wie bei move())
uebergeben. Ansonsten werden die Standard-Fluchtmeldungen verwendet.
Wenn return_time angegeben wird und nicht 0 ist, kehrt der NPC nach
'return_time' Sekunden wieder an die alte Stelle zurueck.
VERWEISE: move, runaway
GRUPPEN: spieler, monster
*/
varargs string random_move(int return_time, string verlassen, string ankommen)
{
    object env;
    string *dirs, dir;
    int flag;

    // Wer kein Environment hat, laeuft auch nicht davon!
    if (!(env = environment()))
	return 0;

    // Auf der Map sind Ausgaenge immer Nolist, Hidden oder wasweisich,
    // deshalb kommen alle Ausgaenge in Frage, die nicht Geschlossen
    // sind.
    if (sscanf(object_name(env), "/map/m%~d_%~d")!=2)
        flag = EXIT_VISIBLE;

    if (!sizeof(dirs = env->query_type("fluchtausgaenge")) &&
	!sizeof(dirs = env->query_command_list(flag)))
	    return 0;

    if (!verlassen) verlassen = "$Der() flüchtet panisch $dir().";
    if (!ankommen) ankommen = "$Ein() kommt panisch $dir() angerannt.";
    // OK, es hat Ausgaenge, dann nehmen wir einen, bewegen uns dorthin und
    // returnen die Richtung.
    dir = dirs[random(sizeof(dirs))];
    if (return_time)
    {
    	if (this_object()->move_return(dir,return_time,0,"random_move",
		MOVE_NORMAL,verlassen,ankommen,
		MOVE_NORMAL,0,0,
		MOVE_TYPE_FLUECHTEN) == MOVE_OK)
	    return dir;
    }
    else
    {
    	if (this_object()->move(dir,([
            MOVE_FLAGS:     MOVE_NORMAL,
            MOVE_TYPE:      MOVE_TYPE_FLUECHTEN,
            MOVE_MSG_LEAVE: verlassen,
            MOVE_MSG_ENTER: ankommen ])) == MOVE_OK)
	    return dir;
    }
}

/*
FUNKTION: runaway
DEKLARATION: int runaway()
BESCHREIBUNG:
Fuehrt eine Bewegung aus wie im Flucht-Modus des Players, mit allen Meldungen
und allem drum und dran. Wenn die Flucht erfolgreich war, wird 1
zurueckgeliefert, sonst 0.
VERWEISE: random_move, move
GRUPPEN: spieler, monster
*/
int runaway()
{
    if (random_move(playerp(this_object()) ? 0 : 30))
    {
	if(!this_object())
	    return 1;
	notify_message("Deine Füße sind mit dir davongelaufen!\n");
	this_object()->send_message(MT_LOOK,MA_UNKNOWN,
            wrap(Der()+" wirkt sichtlich verstört."));
	handle_swimming();
	return 1;
    }
    else
    {
	if(!this_object())
	    return 0;
	this_object()->send_message(MT_LOOK,MA_UNKNOWN,
            wrap(Der()+" versucht vergeblich, davonzulaufen."));
	notify_message(
		"Deine Füße wollen mit dir wegrennen, aber vergeblich.\n");
	return 0;
    }
}

int cmd_runaway ()
{
    runaway ();
    return 1;
}

int cmd_enter(string str)
{
    mixed * parse;
    mixed obj, room, env;
    string * msg;
    string dir;

    if(stringp(str))
    {
        parse = parse_com(str, 0, ({"in","auf"}), PARSE_AFTER_TRENNER);

        if(parse[PARSE_NUM_OBS] == 1)
        {
            // Okay, ein Objekt / V-Item gefunden haben wir...
            obj = parse[PARSE_OBS][0];

            // ... aber kann man das auch betreten?
            room = QUERY_PARS("enter_room", obj, ({this_player()}));
            
            env = environment(TP);

            if(stringp(room))
            {
                if (strlen(room) && room[0] != '/')
                {
                    dir = room;
                    if (env) 
                    {
                        room = env.query_one_exit(dir);
                        if (room) room = touch(room);
                    }
                }
                else
                    room = touch(room);
            }

            if(objectp(room))
            {
                // Man kann es betreten, mal sehen mit welchen Meldungen.
                msg = QUERY_PARS("enter_messages", obj, ({this_player()}));

                if(msg == 0)
                {
                    msg = ({ "$Der(OBJ_TP) betritt "+den(obj)+".",
                             "$Ein(OBJ_TP) betritt "+den(obj)+".",
                             "Du betrittst "+den(obj)+"." });
                }
#if __BOOT_TIME__ < 1620000000
                else
                {
                    msg = copy(msg);        

                    if(sizeof(msg[0]) && msg[0][<1] != '$' &&
                        member(msg[0], '$') != -1)
                    {
                        msg[0] = TO->closure_to_string(mixed_to_closure(
                                     msg[0], ({'vehikel}), 1), ({obj})) + "$";
                    }

                    if(sizeof(msg[1]) && msg[1][<1] != '$' &&
                        member(msg[1], '$') != -1)
                    {
                        msg[1] = TO->closure_to_string(mixed_to_closure(
                                     msg[1], ({'vehikel}), 1), ({obj})) + "$";
                    }


                    if((sizeof(msg) >= 3) && sizeof(msg[2]) && 
                        msg[2][<1] != '$' && member(msg[2], '$') != -1)
                    {
                        msg[2] = TO->closure_to_string(mixed_to_closure(
                                     msg[2], ({'vehikel}), 1), ({obj}));
                    }
                }
#endif

                if(TO->do_forbiddens(C_NO_HERE, "enter", ({"","_me","","_here"}),
                                     ({TP,obj,env,room})))
                {
                    return 1;
                }

                else if(TP->move(dir || room, ([
                    MOVE_FLAGS:     MOVE_NORMAL,
                    MOVE_TYPE:      MOVE_TYPE_BETRETEN,
                    MOVE_MSG_LEAVE: msg[0],
                    MOVE_MSG_ENTER: msg[1],
                    MOVE_MSG_SELF:  (sizeof(msg) < 3) ? 0 : msg[2],
                    MOVE_MSG_ARGS:  ([
                            'vehikel: obj,
                        ]),
                    ])) == MOVE_OK && TO)
                {
                    TO->do_notifies(C_NO_HERE, "enter", ({"","_me","","_here"}),
                                    ({TP,obj,env,room}));
                    return 1;
                }

                //move selbst gibt die Fehlermeldung aus.
                //notify_message("Das hat leider nicht geklappt.\n", MA_MOVE);
                return 1;
            }
        }

        else if(parse[PARSE_NUM_OBS] > 1)
        {
            return notify_fail("Du kannst nicht mehrere Dinge auf einmal "
                               "betreten.\n");
        }
    }

    return notify_fail("Betrete was?\n", FAIL_NOT_CMD);
}

void add_actions()
{
    add_action("go_command",    "",             AA_SHORT);
    add_action("go_command",    "gehe",         -3);
    add_action("go_command",    "laufe",        -4);
    add_action("cmd_runaway",   "fliehe",       -5);
    add_action("cmd_runaway",   "flüchte");
    add_action("cmd_enter",     "entere",       -5);
    add_action("cmd_enter",     "steige",       -5);
    add_action("cmd_enter",     "betrete",      -6);
    add_action("cmd_enter",     "betritt");
}

/* --- Dokumentation: --- */

/*
FUNKTION: forbidden_enter
DEKLARATION: int forbidden_enter(object wer, mixed was, object woher, object wohin)
BESCHREIBUNG:
Ein Spieler oder NPC 'wer' versucht, ein Objekt oder V-Item 'was' zu
betreten. Dies wird zu einer Bewegung von 'wer' nach 'wohin' von 'woher'
fuehren.

Zuvor wird mit forbidden("enter", wer, was, woher, wohin) geprueft,
ob das Betreten zulaessig ist. Der Controller wird in allen Objekten
aufgerufen, die sich bei 'wer' oder 'woher' fuer "forbidden_enter"
angemeldet haben. Liefert ein Objekt einen Wert != 0 zurueck, wird
das Betreten verhindert. Fuer eine entsprechende Meldung ist zu sorgen.

In 'was' wird forbidden("enter_me"), in 'wohin' forbidden("enter_here")
zusaetzlich aufgerufen.
VERWEISE: forbidden_enter_me, forbidden_enter_here
GRUPPEN: spieler, monster
*/

/*
FUNKTION: forbidden_enter_me
DEKLARATION: int forbidden_enter_me(object wer, mixed was, object woher, object wohin)
BESCHREIBUNG:
Ein Spieler oder NPC 'wer' versucht, ein Objekt oder V-Item 'was' zu
betreten. Dies wird zu einer Bewegung von 'wer' nach 'wohin' von 'woher'
fuehren.

Zuvor wird mit forbidden("enter_me", wer, was, woher, wohin) geprueft,
ob das Betreten zulaessig ist. Der Controller wird in allen Objekten 
aufgerufen, die sich bei 'was' fuer "forbidden_enter_me" angemeldet
haben. Ist 'was' ein V-Item, so wird der Controller in dessen Umgebung
aufgerufen. Liefert ein Objekt einen Wert != 0 zurueck, wird
das Betreten verhindert. Fuer eine entsprechende Meldung ist zu sorgen.

In 'wer' und 'woher' wird forbidden("enter") zusaetzlich aufgerufen,
in 'wohin' ausserdem forbidden("enter_here").
zusaetzlich aufgerufen. 
VERWEISE: forbidden_enter, forbidden_enter_here        
GRUPPEN: spieler, monster
*/

/*
FUNKTION: forbidden_enter_here
DEKLARATION: int forbidden_enter_here(object wer, mixed was, object woher, object wohin)
BESCHREIBUNG:
Ein Spieler oder NPC 'wer' versucht, ein Objekt oder V-Item 'was' zu
betreten. Dies wird zu einer Bewegung von 'wer' nach 'wohin' von 'woher'
fuehren.

Zuvor wird mit forbidden("enter_here", wer, was, woher, wohin) geprueft,
ob das Betreten zulaessig ist. Der Controller wird in allen Objekten
aufgerufen, die sich bei 'wohin' fuer "forbidden_enter_here" angemeldet
haben. Liefert ein Objekt einen Wert != 0 zurueck, wird das Betreten
verhindert. Fuer eine entsprechende Meldung ist zu sorgen.

In 'wer' und 'woher' wird forbidden("enter") zusaetzlich aufgerufen,
in 'was' ausserdem forbidden("enter_me").
zusaetzlich aufgerufen.
VERWEISE: forbidden_enter, forbidden_enter_me
GRUPPEN: spieler, monster
*/

/*
FUNKTION: notify_enter
DEKLARATION: int notify_enter(object wer, mixed was, object woher, object wohin)
BESCHREIBUNG:
Ein Spieler oder NPC 'wer' hat das Objekt oder V-Item 'was' betreten.
Dadurch wurde 'wer' bewegt, und zwar von 'woher' nach 'wohin'.

Anschliessend werden mit notify("enter", wer, was, woher, wohin) alle
Objekte, die sich bei 'wer' oder 'woher' fuer "notify_enter" angemeldet
haben, von dem Vorgang unterrichtet.

In 'was' wird notify("enter_me"), in 'wohin' notify("enter_here")
zusaetzlich aufgerufen.
VERWEISE: notify_enter_me, notify_enter_here
GRUPPEN: spieler, monster
*/

/*
FUNKTION: notify_enter_me
DEKLARATION: int notify_enter_me(object wer, mixed was, object woher, object wohin)
BESCHREIBUNG:
Ein Spieler oder NPC 'wer' hat das Objekt oder V-Item 'was' betreten.
Dadurch wurde 'wer' bewegt, und zwar von 'woher' nach 'wohin'.

Anschliessend werden mit notify("enter_me", wer, was, woher, wohin) alle
Objekte, die sich bei 'was' fuer "notify_enter_me" angemeldet haben, von
dem Vorgang unterrichtet. Ist 'was' ein V-Item, wird der Controller in
dem Objekt aufgerufen, an dem das V-Item haengt.

In 'wer' und 'woher' wird notify("enter") aufgerufen, 
ausserdem in 'wohin' notify("enter_here").
VERWEISE: notify_enter_me, notify_enter_here
GRUPPEN: spieler, monster
*/

/*
FUNKTION: notify_enter_here
DEKLARATION: int notify_enter_here(object wer, mixed was, object woher, object wohin)
BESCHREIBUNG:
Ein Spieler oder NPC 'wer' hat das Objekt oder V-Item 'was' betreten.
Dadurch wurde 'wer' bewegt, und zwar von 'woher' nach 'wohin'.

Anschliessend werden mit notify("enter_here", wer, was, woher, wohin) alle
Objekte, die sich bei 'wohin' fuer "notify_enter_here" angemeldet haben,
von dem Vorgang unterrichtet. 

In 'wer' und 'woher' wird notify("enter") aufgerufen, 
ausserdem in 'was' notify("enter_me").
VERWEISE: notify_enter_me, notify_enter_here
GRUPPEN: spieler, monster
*/
