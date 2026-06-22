// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/i/item/message.c
// Description: Funktionen, um Meldungen zu verteilen
// Author:	Freaky (02.11.97)
// Modified by:	Freaky (01.06.1998) propagate_message_to_env

#pragma save_types

#include <message.h>
#include <level.h>
/*
FUNKTION: MT_LISTE
DEKLARATION: Liste der Message Typen.
BESCHREIBUNG:

Message-Typ:
Der Typ definiert, welcher Sinn beim Empfaenger der Message
angesprochen wird:
   MT_UNKNOWN  Sollte nicht verwendet werden, ist eine Notloesung
   MT_LOOK     alles, was man sieht
   MT_NOISE    alles, was man hoert
   MT_FEEL     alles, was man fuehlt
   MT_TASTE    alles, was man schmeckt
   MT_SMELL    alles, was man riecht
   MT_SENSE    alle uebersinnlichen Wahrnehmungen, z.B. rede
   MT_NOTIFY   Statusmeldungen: "Ok.", "Du nimmst den Blasuelz."
   MT_CHANNEL  Kurier-Kanaele.
   MT_FAR      alles, was aus der Ferne kommt. (nicht von etwas im Raum kommt)
   MT_DEBUG    Debugmeldungen, sehen nur Goetter
   MT_FAIL     Fehlschlagsnachrichten (Debug hat mehr Prio)

   MT_INDENT   Wenn die Meldung umgebrochen wird, wird ab der 2. Zeile
               eingerueckt (gilt automatisch fuer MA_COMM-Meldungen).
   MT_NO_WRAP  Die Meldung wird nicht automatisch umgebrochen. (Dies ist fuer
               Meldungen gedacht, die z.B. nur \a oder ANSI-Codes ausgeben.)

Die Defines finden sich in <message.h>.   
VERWEISE: MA_LISTE, send_message, send_message_to
GRUPPEN: message
*/

/*
FUNKTION: MA_LISTE
DEKLARATION: Liste der Message Aktionen.
BESCHREIBUNG:

Message-Aktionen:
Die Aktion legt fest, durch welche Aktion eine Message ausgeloest wird
  MA_UNKNOWN          Die Notloesung.
  MA_LOOK             Jemand sieht etwas an
  MA_NOISE            Jemand horcht oder lauscht an etwas
  MA_FEEL             Jemand betastet etwas
  MA_TASTE            Jemand leckt etwas ab
  MA_SMELL            Jemand schnueffelt herum
  MA_SENSE            Jemand macht eine uebersinnliche Wahrnehmung
  MA_PUT              Jemand legt etwas hin und gibt jemanden etwas
  MA_TAKE             Jemand nimmt etwas
  MA_MOVE_IN          Jemand betritt den Raum
  MA_MOVE_OUT         Jemand verlaesst den Raum
  MA_MOVE             Jemand bewegt sich
  MA_EMOTE            Jemand grinst (oder so)
  MA_FIGHT            Jemand kaempft
  MA_WIELD            Jemand fuehrt eine Waffe
  MA_UNWIELD          Jemand senkt eine Waffe
  MA_WEAR             Jemand zieht etwas an
  MA_UNWEAR           Jemand zieht etwas aus
  MA_EAT              Jemand isst etwas (kotzen ist ein MA_EMOTE,
                                          kein MA_UNEAT)
  MA_DRINK            Jemand trinkt etwas
  MA_COMM             Jemand sagt was
  MA_MAGIC            Keines von alledem, is aber magisch
  MA_READ             Jemand liest etwas
  MA_USE              Jemand benutzt etwas
  MA_CRAFT            Jemand werkelt rum (bearbeitet etwas)
  MA_REMOVE	      Etwas verschwindet.

Die Defines finden sich in <message.h>.   
VERWEISE: MT_LISTE, send_message, send_message_to
GRUPPEN: message
*/

/*
FUNKTION: MSG_ATTRIBUTES
DEKLARATION: Liste der Message Attribute.
BESCHREIBUNG:

Message-Attribute (aus message.h):
Name            Typ             Bedeutung
--------------- --------------- -----------------------------------------------
  MSG_AH_INFOS  mapping         Von add_hp uebergebenes infos-Mapping.
  
  MSG_RECEVIER_WHOM  string     Im Kampf werden folgende Werte fuer den 
                                Adressaten der Melduung uebergeben:
                                ==AH_ATTACKER (Angreifer, selbst)
                                ==AH_VICTIM (der jeweilige Feind)
                                ==MSG_OTHERS (intern bei send_message)
                                Hiermit wird in /i/player/filter_messages.c 
                                unterschieden, welcher der drei Filter 
                                geprueft wird.
                                
  MSG_FIRST_MSG (int 0/1)       Flag zur Anzeige einer ersten Meldung
                                z.B. der Anfang eines Kampfes.
                                
  MSG_LAST_MSG  (int 0/1)       Flag zur Anzeige einer letzten Meldung


Kampfmeldungsattribute (aus fight_options.h):
Name            Typ             Bedeutung
--------------- --------------- -----------------------------------------------
  FIM_MAX_COUNT  int            Maximalanzahl der unterdrueckten Kampfmeldungen
  
  FIM_BROKEN     0/1            Attribut, ob Waffe/Ruestung kaputt ist.
  
  FIM_WEAPON     object         Angabe der Waffe, z.B. bei FIM_BROKEN
  
  FIM_ARMOUR     object         Angabe der Ruestung, z.B. bei FIM_BROKEN

  FIM_WHO_SELF   int            Die Flags, die unten folgen, fuer sich selbst
  
  FIM_WHO_ENEMY  int            Die Flags fuer den Feind-Filter.
  
  FIM_WHO_OTHERS int            Die Flags, wenn andere kaempfen.

Flags zum Austausch der Einstellung aus /i/player/options.c 
und /i/player/filter_messages.c (aus fight_options.h)
Name                            Bedeutung
----------------------------- -----------------------------------------------
  FIM_FILTER_FIGHT_FIRST_MSG  Flag fuer erste Meldung eines Kampfes
  
  FIM_FILTER_FIGHT_CRITICAL   Flag ob das ein kritischer Schlag gefiltert wird
  
  FIM_FILTER_WEAPON_BROKEN    Flag ob eine Waffe-Kaputt-Meldung gefiltert wird
  
  FIM_FILTER_ARMOUR_BROKEN    Gleiches Flag fuer Ruestungen-Kaputt.
  
  FIM_FILTER_FIGHT_LAST_MSG   Flag fuer die letzte Meldung eines Kampfes.
  
  
Die folgenden Flags aus fight_options.h klassifizieren die einzelnen 
Schweregrade einer Kampfmeldung und laesst sich dadurch filtern.
  FIM_FILTER_CATEGORY_0NULL
  
  FIM_FILTER_CATEGORY_1WEAK
  
  FIM_FILTER_CATEGORY_2MEDIUM
  
  FIM_FILTER_CATEGORY_3STRONG
  
  FIM_FILTER_CATEGORY_4HIGH
  
  FIM_FILTER_CATEGORY_5FRACTAL

VERWEISE: send_message, send_message_to, send_message_in
GRUPPEN: message
*/


/*
FUNKTION: send_message_to
DEKLARATION: void send_message_to(object whom | object *whom, int msg_type, int msg_action, string msg, mapping attributes)
BESCHREIBUNG:
Mit 'send_message_to' schickt man einem oder mehreren Objekten eine Meldung.
'msg_type' und 'msg_action' sind bei 'send_message' beschrieben.
'attributes' sind in ?MSG_ATTRIBUTES beschrieben.
VERWEISE: send_message, send_message_in, receive_message, MT_LISTE, MA_LISTE, MSG_ATTRIBUTES
GRUPPEN: message
*/
#ifdef FILTER_MSG_BY_ATTRIBUTES
varargs void send_message_to(object|object* who, int msg_type, int msg_action, 
    string msg,mapping attributes)
{
    if (stringp(msg) && (!(msg_type&MT_DEBUG) ||
        (objectp(who) && wizp(who)) ||
        (pointerp(who) && sizeof(who=filter(who,(:wizp($1):))))))
            who->receive_message(msg_type,msg_action,this_object(),
                msg,attributes);
}
#else
void send_message_to(object|object* who, int msg_type, int msg_action, string msg)
{
    if (stringp(msg) && (!(msg_type&MT_DEBUG) ||
        (objectp(who) && wizp(who)) ||
        (pointerp(who) && sizeof(who=filter(who,(:wizp($1):))))))
            who->receive_message(msg_type,msg_action,this_object(),msg);
}
#endif
/*
FUNKTION: send_message
DEKLARATION: varargs void send_message(int msg_type, int msg_action, string msg, [string msg_whom, object whom | object *whom, mapping attributes])
BESCHREIBUNG:
Mit send_message verteilt man die Meldung 'msg' an alle Objekte im selben
Raum (bzw. Container) ausser dem Meldungsurheber (also ausser demjenigen, in
welchem send_message aufgerufen wurde).
'msg_whom' wird an das Objekt bzw. die Objekte 'whom' verteilt, falls diese
angegeben wurden. Ausserdem wird dann 'msg' nicht an 'whom' verteilt.

'msg_type' Definiert den Typ der Meldung (also welcher Sinn des
Beobachters davon angesprochen wird)
Diese Typen sind in /sys/message.h definiert und koennen logisch verknuepft
werden, wenn die Meldung mehrere Sinne anspricht.
'msg_action' ist der Aktions-Typ, durch den diese Meldung ausgeloest wurde.
Der Aktions-Typ ist in /sys/message.h definiert.
'attributes' ist in MSG_ATTRIBUTES naeher erlaeutert.
VERWEISE: send_message_to, send_message_in, receive_message, MT_LISTE, MA_LISTE, MSG_ATTRIBUTES
GRUPPEN: message
*/
// ACHTUNG: Es gibt noch ein send_message in /i/room.
#ifdef FILTER_MSG_BY_ATTRIBUTES
varargs void send_message(int msg_type, int msg_action, string msg,
	string msg_whom, object|object* whom,mapping attributes)
{
    mapping attr_all = deep_copy(attributes);
    
    if (whom && msg_whom)
        send_message_to(whom,msg_type,msg_action,msg_whom,attr_all);
    
    if (mappingp(attr_all) && member(attr_all,MSG_RECEIVER_WHOM))
    {
        attr_all[MSG_RECEIVER_WHOM] = MSG_OTHERS;
    }
    if (stringp(msg) && environment())
    {
        object *wer;
	if (pointerp(whom))
	    wer = all_inventory(environment()) - ({this_object()}) - whom;
	else
	    wer = all_inventory(environment()) - ({this_object(),whom});
	if(msg_type & MT_DEBUG)
	    wer = filter(wer,(:wizp($1):));
	filter_objects(wer,"receive_message",msg_type,msg_action,
	    this_object(),msg,attr_all);
	environment()->propagate_message_to_env(msg_type,msg_action,
		this_object(),msg,msg_whom,whom,attr_all);

	// Message nur ans Env, wenn es nicht (in) whom ist
	if (((pointerp(whom) && member(whom,environment()) < 0) ||
	     (!pointerp(whom) && whom != environment())) &&
	    (!(msg_type&MT_DEBUG) || wizp(environment())))
	    environment()->receive_message(msg_type,msg_action,
		    this_object(),msg,attr_all);
    }
}
#else
varargs void send_message(int msg_type, int msg_action, string msg,
	string msg_whom, object|object* whom)
{
    if (whom && msg_whom)
	send_message_to(whom,msg_type,msg_action,msg_whom);

    if (stringp(msg) && environment())
    {
        object *wer;
	if (pointerp(whom))
	    wer = all_inventory(environment()) - ({this_object()}) - whom;
	else
	    wer = all_inventory(environment()) - ({this_object(),whom});
	if(msg_type & MT_DEBUG)
	    wer = filter(wer,(:wizp($1):));
	filter_objects(wer,"receive_message",msg_type,msg_action,
	    this_object(),msg);
	environment()->propagate_message_to_env(msg_type,msg_action,
		this_object(),msg,msg_whom,whom);

	// Message nur ans Env, wenn es nicht (in) whom ist
	if (((pointerp(whom) && member(whom,environment()) < 0) ||
	     (!pointerp(whom) && whom != environment())) &&
	    (!(msg_type&MT_DEBUG) || wizp(environment())))
	    environment()->receive_message(msg_type,msg_action,
		    this_object(),msg);
    }
}
#endif
/*
FUNKTION: send_message_in
DEKLARATION: varargs void send_message_in(object room, int msg_type, int msg_action, string msg, [string msg_whom, object whom | object *whom, mapping attributes])
BESCHREIBUNG:
Mit send_message_in verteilt man die Meldung 'msg' an alle Objekte im 
angegebenen Raum (bzw. Container).

'room'	ist der Raum, in welchem die Meldung ausgegeben werden soll.
'msg_type' definiert den Typ der Meldung (also welcher Sinn des
	Beobachters davon angesprochen wird)
	Diese Typen sind in /sys/message.h definiert und koennen logisch
	verknuepft werden, wenn die Meldung mehrere Sinne anspricht.
'msg_action' ist der Aktions-Typ, durch den diese Meldung ausgeloest wurde.
	Der Aktions-Typ ist in /sys/message.h definiert.
'msg' ist die Meldung.
'msg_whom' wird an das Objekt bzw. die Objekte 'whom' verteilt, falls diese
	angegeben wurden. Ausserdem wird dann 'msg' nicht an 'whom' verteilt.
'attributes' ist in MSG_ATTRIBUTES naeher erlaeutert.

VERWEISE: send_message_to, send_message, receive_message, MT_LISTE, MA_LISTE, MSG_ATTRIBUTES
GRUPPEN: message
*/
#ifdef FILTER_MSG_BY_ATTRIBUTES
varargs void send_message_in(object room, int msg_type, int msg_action,
	string msg, string msg_whom, object|object* whom,mapping attributes)
{
    mapping attr_all = deep_copy(attributes);
    if (whom && msg_whom)
	send_message_to(whom,msg_type,msg_action,msg_whom,attr_all);

    
    if (mappingp(attr_all))
    {
        attr_all[MSG_RECEIVER_WHOM] = MSG_OTHERS;
    }
    if (stringp(msg) && room)
    {
        object *wer;
	if (pointerp(whom))
	    wer = all_inventory(room) - whom;
	else
	    wer = all_inventory(room) - ({whom});
	if(msg_type & MT_DEBUG)
	    wer = filter(wer,(:wizp($1):));
	wer->receive_message(msg_type,msg_action,this_object(),msg,attr_all);
	room->propagate_message_to_env(msg_type,msg_action,
		this_object(),msg,msg_whom,whom,attr_all);

	// Message nur an room, wenn es nicht (in) whom ist
	if (((pointerp(whom) && member(whom,room) < 0) ||
	     (!pointerp(whom) && whom != room)) &&
	    (!(msg_type&MT_DEBUG) || wizp(room)))
	    room->receive_message(msg_type,msg_action,
		    this_object(),msg,attr_all);
    }
}
#else
varargs void send_message_in(object room, int msg_type, int msg_action,
	string msg, string msg_whom, object|object* whom)
{
    if (whom && msg_whom)
	send_message_to(whom,msg_type,msg_action,msg_whom);

    if (stringp(msg) && room)
    {
        object *wer;
	if (pointerp(whom))
	    wer = all_inventory(room) - whom;
	else
	    wer = all_inventory(room) - ({whom});
	if(msg_type & MT_DEBUG)
	    wer = filter(wer,(:wizp($1):));
	wer->receive_message(msg_type,msg_action,this_object(),msg);
	room->propagate_message_to_env(msg_type,msg_action,
		this_object(),msg,msg_whom,whom);

	// Message nur an room, wenn es nicht (in) whom ist
	if (((pointerp(whom) && member(whom,room) < 0) ||
	     (!pointerp(whom) && whom != room)) &&
	    (!(msg_type&MT_DEBUG) || wizp(room)))
	    room->receive_message(msg_type,msg_action,
		    this_object(),msg);
    }
}

#endif
/*
FUNKTION: receive_message
DEKLARATION: void receive_message(int msg_type, int msg_action, object who, string msg, mapping attributes)
BESCHREIBUNG:
Diese Funktion wird von send_message() bzw. send_message_to() in jedem
Objekt, das die Meldung bekommen soll, aufgerufen.
'who' ist dabei das Objekt, das die Meldung hervorgerufen hat.
'attributes' ist in MSG_ATTRIBUTES naeher erlaeutert.

Diese Funktion darf in Spielern nicht einfach durch Shadows ueberlagert werden.
Dazu werden spezielle Privilegien benoetigt (bei den Admins zu beantragen).
VERWEISE: send_message_to, receive_message, MT_LISTE, MA_LISTE, MSG_ATTRIBUTES
GRUPPEN: message
*/
