// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/i/move.c
// Description: Bewegung von Objekten
// Author:	Francis, Freaky, Garthan (23.12.93)
// Modified by: Garthan	(10.01.94) set_no_move ueberarbeitet
//              Pulami	(27.06.94) moved_in/out bekommen jetzt auch 'dir'
//              Freaky	(22.10.94) Fehler, wenn 1. move nicht geklappt hat
//              Freaky	(22.02.95) NO_GLOBAL_ARRAY definiert
//              Freaky	(05.12.95) let_not_in wird nicht mehr ignoriert
//              Sissi	(26.04.96) no_move_reason, not_moved_reason
//              Garthan	(16.10.96) das ueberfluessige " " wegbeschissen durch ^A
//		                   statt leerem dir_string (oh weia)
//		Garthan	(03.01.97) Shimmer
//              Kurdel  (14.10.97) first_player
//		Freaky  (21.01.98) Messages nach /i/tools/move_msg.c
//		Freaky  (21.01.98) In-Msg wird vor Move evaluiert.
//		Freaky  (08.03.98) Teleport-Log wieder eingebaut
//              Sissi   (03.08.99) notify_move_out, notify_move_in
//		Freaky  (23.02.2000) notify_follow repariert
//		Freaky  (01.03.2000) an das neue add_encumbrance() angepasst
//		Freaky  (02.05.2000) query_enable_cleanup: Inhalt ueberpruefen
//              Gnomi   (28.06.2000) Doku zu den ganzen Move - Controller -
//                                   Methoden angehaengt
//
////////////////////////////////////////////////////////////////////////////
//
//  Das Inheritfile /i/move ist fuer
//  Objekte, die unbegrenzt beweglich sein sollen, im Gegensatz zu
//  /i/install, welches nur eine einmalige Bewegung zulaesst.
//
// Dokumentation siehe /doc/funktionsweisen/move
//
////////////////////////////////////////////////////////////////////////////


#pragma save_types

#include <config.h>
#include <move.h>
#include <room.h>
#include <invis.h>
#include <level.h>
#include <error.h>
#include <message.h>
#include <properties.h>
#include <simul_efuns.h>

private        int     weight = 1;
private static int     prevent_cleanup, make_visible_at_next_move, no_move;
private static string  first_room, first_player, no_move_reason,
		       not_moved_reason, last_player;
private static string  creator = sprintf("%s (UID: %s, EUID: %s)",
			 object_name(previous_object()),
			 getuid(previous_object())||"0",
			 geteuid(previous_object())||"0");
private static mixed   *return_move = ({});

string query_no_move_reason();

/* ================= WEIGHT ============================ */

/*
FUNKTION: set_weight
DEKLARATION: int set_weight(int weight)
BESCHREIBUNG:
Setzt das Gewicht eines Monsters, Spielers oder Objekts. Ein Newbie kann
10 Einheiten Gewicht mit sich rumschleppen, ein Experte bis zu 30, Engel
bis zu 45. Goetter koennen 100 Einheiten schleppen. Alle beweglichen
Objekte sollten ein Gewicht haben, Spieler muessen ein Gewicht von 30 haben.
Zu dem Gewicht von Autoloadern -> /doc/richtlinien/autoloader.
VERWEISE: query_weight, set_min_weight, set_max_weight
GRUPPEN: move, grundlegendes
*/
int set_weight(int enc)
{
    if (enc < 0)
        enc = 0;

    if (enc == weight)
        return 1;

    if (environment())
        environment()->add_encumbrance(this_object(), ENC_MODIFY|ENC_FORCE, enc - weight);

    weight = enc;
    this_object()->add_setter_conservation("set_weight",({weight}) );
    return 1;
}

/*
FUNKTION: query_weight
DEKLARATION: nomask int query_weight()
BESCHREIBUNG:
Liefert das Gewicht eines Monsters, Spielers oder Objekts.
VERWEISE: set_weight, query_min_weight, query_max_weight
GRUPPEN: move, grundlegendes
*/
nomask int query_weight() { return weight; }

/*
FUNKTION: query_weight_description
DEKLARATION: string query_weight_description()
BESCHREIBUNG:
Diese Funktion kann man im Objekt definieren, damit beim Betrachten
des Objektes ein besonderer String anstelle der Standardmeldung
("Es scheint schwer zu sein." usw.) ausgegeben wird.
VERWEISE: query_weight, query_content_message, query_object_description
GRUPPEN: augen
*/

/* ================ MOVE UTILITY FUNCTIONS ============= */

mixed move_destination(mixed dest)
{
    object old_room;
    mixed *exit_info;

    if (stringp(dest) && strlen(dest) && dest[0] != '/' &&
	    (old_room = environment()) &&
	    (exit_info = old_room->query_exit_info(dest)))
	return exit_info[EI_EXIT];
    else
	return dest;
}

/* ======================= MOVE =========================== */
#define DO_REMOVE_ON_ERR \
        if(mv_infos[MOVE_FLAGS]&MOVE_ERR_REMOVE) \
            this_object()->remove();
#define DO_NOTIFY_MOVE_FAIL(ret) if (this_object()) \
        this_object()->notify("move_failed",\
        mv_infos+([__FILE__:__LINE__]),ret);
/*
FUNKTION: move
DEKLARATION: varargs nomask int move(mixed ziel, mapping mv_infos)
BESCHREIBUNG:
Die exakte Beschreibung ->/doc/funktionsweisen/move

/i/move::move() ist die einzige Funktion in UNItopia die Objekte von einem
Ort zu einem anderen bewegen kann. Jedes Objekt, das bewegt werden soll,
muss also diese Datei inheriten. "ziel" kann ein Objekt oder der Filename des
Ziels als string sein oder aber, sofern sich das Objekt in einem Raum befindet,
auch ein Ausgang des Raumes. Alle Defines kommen aus /sys/move.h.

Der zweite Parameter ist optional und kann folgende Eintraege mit
Informationen zur Bewegung beinhalten:

Name            Typ             Bedeutung
--------------- --------------- -----------------------------------------------
MOVE_FLAGS      int             Eine Kombination von Flags. Eine Liste der
                                moeglichen Flags gibt es weiter unten.

MOVE_TYPE       string          Enthaelt das Verb fuer die Art der Bewegung
                                (siehe MOVE_TYPE_LISTE).

MOVE_MSG_LEAVE  string          Eine Bewegungsmeldung an die Umgebenden im
                                Ausgangsraum.

MOVE_MSG_ENTER  string          Eine Bewegungsmeldung an die Umgebenden im
                                Zielraum.

MOVE_MSG_ME     string          Bewegungsmeldung an den Bewegenden.


MOVE_MSG_LEAVE_OTHERS
                mixed*          Mit diesem Array kann man Bewegungsmeldungen
                                an Dritte im Ausgangsraum formulieren:
                                ({
                                    <object|object*> whom1,
                                    <string> meldung_an_whom1,

                                    <object|object*> whom2,
                                    <string> meldung_an_whom2,

                                    ...
                                })

MOVE_MSG_ENTER_OTHERS
                mixed*          Gleicher Aufbau und Funktion wie
                                MOVE_MSG_LEAVE_OTHERS fuer den Zielraum.

MOVE_MSG_ARGS   mapping         Ein Mapping mit zusätzlichen Symbolen
                                für die Move-Meldungen (der einem Symbol
                                zugeordnete Wert muß demnach ein Objekt oder
                                V-Item sein):
                                ([
                                    'vehikel: ([ "name": "boot",
                                                 "gender": "saechlich",
                                              ]),
                                ])


Folgende Eintraege werden spaeter fuer die Controller ergaenzt:
---------------------------------------------------------------
MOVE_CALLER     object          Das Objekt, dass das move aufruft.

MOVE_OBJECT     object          Das bewegte Objekt

MOVE_OLD_ROOM   object          Der Ausgangsraum

MOVE_NEW_ROOM   object          Der Zielraum

MOVE_DEST_STR   string          Der Parameter 'ziel' vom Aufruf, sofern es
                                ein String war (also entweder ein Dateiname
                                oder der Name des Ausgangs).

MOVE_DIRECTION  string          Der Name des Ausgangs (sofern bekannt)

MOVE_EXIT_INFO  mixed*          Ein Array mit Informationen ueber den Ausgang
                                (siehe query_exit_info())


Folgende Flags kann man bei MOVE_FLAGS angeben:
-----------------------------------------------

    MOVE_NORMAL:        Standard-Move, normale Bewegungsmeldungen, also
                        die mit set_msg_in() und set_msg_out() gesetzten,

    MOVE_MAGIC:         Teleport, es werden die Bewegungsmeldungen
                        erzeugt, die mit set_mmsg_in() und set_mmsg_out()
                        gesetzt wurden,

    MOVE_SECRET:        Es werden keine Bewegungsmeldungen erzeugt und
                        nicht just_moved() aufgerufen (d.h. es gibt
                        keine Ausgabe der Raumbeschreibung bei Spielern).

    0:                  Default, keine Bewegungsmeldungen.
    
    MOVE_GHOST:         Beim Bewegen von Geistern durch Tueren wird dieser Typ
                        verwendet. Ist ein erweitertes MOVE_MAGIC.

Diese Flags kann man noch durch Oder | mit folgenden Werten verknuepfen:
    MOVE_FORCE:         Nur fuer priviligierte Objekte eine Bewegung ohne
                        modify und forbidden-Controller.

    MOVE_ERR_REMOVE:    Konnte die Bewegung nicht ausgefuehrt werden,
                        wird der Gegenstand entfernt.


Meldungen
---------
Diese Bewegungsmeldungen (MOVE_MSG_*-Eintraege) sind Pseudoclosures,
wobei fuer den Bewegenden kein Argument in den Grammatikfunktionen
(z.B. $Der(), $Ein()) anzugeben ist und $dir() fuer die Richtung
(samt Praeposition) steht.

Enthaelt diese Bewegungsmeldung kein Dollarzeichen, so wird automatisch
der Name des bewegten Objektes ("$Ein() " bzw. "$Der() ") voran- und
die Richtung ("$dir().") hintenrangestellt. Ebenso wird ein fehlendes
Satzzeichen angefuegt.
ACHTUNG: Diese Bewegungsmeldungen (ohne Name und Richtung bzw. ohne
Satzendezeichen) gelten als veraltet!

Ein einzelnes Dollarzeichen am Ende dieser Meldungen wird abgeschnitten.
Diese Meldungen werden automatisch umgebrochen.

Achtung: Die Meldungen werden nur ausgegeben, wenn zusaetzlich noch
/i/tools/move_msg inheritet wurde. (Lebewesen inheriten es automatisch.)


Rueckgabewert
-------------
Move liefert nach der Ausfuehrungen folgende Werte:

    MOVE_OK:            Move wurde ausgefuehrt.

    MOVE_NOT_ALLOWED:   Move wurde nicht erlaubt (zb Gewicht im Ziel
                        ueberschritten oder let_not_in im Zielraum returnt 1.)

    MOVE_NO_ROOM:       Das angegebene Ziel ist kein Container oder nicht
                        genug Platz.

    MOVE_DEST_CLOSED:   Das Ziel ist ein geschlossener Container, kein move.

    MOVE_ENV_CLOSED:    Der Ursprungsraum ist ein geschlossener Container,
                        kein Move.

    MOVE_NO_DEST:       Kein Zielraum angegeben oder dest == 0.

    MOVE_DESTRUCTED:    Das Objekt wurde beim move zerstoert.

Alle Defines sind in /sys/move.h definiert.

Nach Beendigung eines moves, der nicht erfolgreich war,
kann mit query_not_moved_reason eine Klartext - Fehlermeldung
abgefragt werden, sofern eine vorliegt; naeheres siehe dort.
Kurz vor der Beendigung wird notify_move_failed mit den obigen Werte aufgerufen,
Ausnahme MOVE_DESTRUCTED (da ist kein this_object mehr da...).

Ob ein Move erfolgreich ist haengt von vielem ab; let_not_in, let_not_out,
filtern, forbidden-Controllern, usw.; siehe dort. Ebenso kann ein move
noch per modify-Controllern beeinflusst werden. (modify_move(|_in|_out))

Wird ein Move ausgefuehrt, so wird unmittelbar davor im alten "Ort" ein
notify_move_out aufgerufen, unmittelbar nach dem move ein notify_moved_out.
Analog wird im Zielraum ein notify_move_in unmittelbar vor dem Move und
ein notify_moved_in unmittelbar nach dem Move aufgerufen. Am Objekt wird
vor dem move notify_move und nach dem move notify_moved aufgerufen.

VERWEISE: move_return, let_not_in, let_not_out, filter_xxx, just_moved,
	  notify_follow, set_no_move, set_no_move_reason, set_not_moved_reason,
	  query_no_move, query_no_move_reason, query_not_moved_reason,
	  move_or_remove, MOVE_TYPE_LISTE
GRUPPEN: move, grundlegendes
*/

varargs nomask int move(<string|object> dest,mapping mv_infos)
{
    not_moved_reason = 0; // erstmal initialisieren
    mv_infos ||= ([]);
    int flag_force = (mv_infos[MOVE_FLAGS]&MOVE_FORCE)>0;
    mixed *exit_info;
    <int|string> res; // Rueckgabewert der Controller.
    mapping secured_mv_infos;// Absicherung der Parameter nach callouts...
    
    mv_infos[MOVE_CALLER] = extern_call() ? previous_object() : this_object();
    // Move Privileges
    if (!MASTER_OB->mudlib_privilege_violation("move", previous_object(),
                                               dest, mv_infos[MOVE_FLAGS]))
    {
        DO_NOTIFY_MOVE_FAIL(MOVE_NOT_ALLOWED);
        DO_REMOVE_ON_ERR;
        return MOVE_NOT_ALLOWED;
    }
    // Wird das Objekt zum ersten Mal bewegt? -> first_room setzen
    if (!first_room && find_call_out("set_first_room")<0)
        call_out("set_first_room",0,
            sprintf("%O",previous_object()),sprintf("%O",dest));
    // force move
    if(flag_force
        && !MASTER_OB->mudlib_privilege_violation("force_move",
            previous_object(), dest, mv_infos[MOVE_FLAGS]))
    {
        do_error("Privilege violation: MOVE_FORCE\n");
        DO_NOTIFY_MOVE_FAIL(MOVE_NOT_ALLOWED);
        DO_REMOVE_ON_ERR;
        return MOVE_NOT_ALLOWED;
    }

    mv_infos[MOVE_OLD_ROOM] = environment();
    if (!flag_force && mv_infos[MOVE_OLD_ROOM]
            && this_object()->query_no_move())
    {
        // TODO MOVE_OWN_FAIL_MESSAGE?
        not_moved_reason = this_object()->query_no_move_reason() ||
                Der()+plural(" lässt", " lassen", this_object()) +
                " sich nicht bewegen.";
        if (living (this_object()))
            this_object()->send_message_to(this_object(), MT_FEEL|MT_NOTIFY,
                MA_MOVE, wrap(not_moved_reason));
        DO_NOTIFY_MOVE_FAIL(MOVE_NOT_ALLOWED);
        DO_REMOVE_ON_ERR;
        return MOVE_NOT_ALLOWED;
    }

    /* Query Exit Info from room */
    if (stringp(dest))
        mv_infos[MOVE_DEST_STR]  = dest;

    if (!flag_force && stringp(dest) && strlen(dest) && dest[0] != '/'
            && mv_infos[MOVE_OLD_ROOM] )
    {
        if (exit_info = mv_infos[MOVE_OLD_ROOM]->query_exit_info(
                dest, (mv_infos[MOVE_FLAGS] & MOVE_ATOM_GHOST)?1:0))
        {
            /* Regular Movement */
            mv_infos[MOVE_DIRECTION] = (sizeof(exit_info) > EI_DIRECTION)
                    && exit_info[EI_DIRECTION] || dest;
            dest = exit_info[EI_EXIT];
        }
        else if(mv_infos[MOVE_FLAGS] & MOVE_ATOM_GHOST)
        {
            mv_infos[MOVE_DIRECTION] = dest;
            dest = mv_infos[MOVE_OLD_ROOM]->query_one_exit(dest, 1);
        }
        else
            dest = 0;

        if(!dest)
        {
            DO_NOTIFY_MOVE_FAIL(MOVE_NO_DEST);
            DO_REMOVE_ON_ERR;
            return MOVE_NO_DEST;
        }
    }
    else
        exit_info = allocate(EI_SIZE);
    mv_infos[MOVE_EXIT_INFO] = exit_info;

    // Wenn der Zielraum nicht geladen werden konnte -> MOVE_NO_DEST
    if (!(mv_infos[MOVE_NEW_ROOM] = touch(dest)))
    {
        DO_NOTIFY_MOVE_FAIL(MOVE_NO_DEST);                                
        DO_REMOVE_ON_ERR;
        return MOVE_NO_DEST;
    }

    // Nur Aufnehmen von Spielern verhindern.
    // Den Rest uebernimmt take().
    if(!flag_force && playerp(this_object()) && living(mv_infos[MOVE_NEW_ROOM]))
    {
        not_moved_reason = Der() + plural(" lässt", " lassen", this_object())
            + " sich nicht so einfach nehmen.";
        DO_NOTIFY_MOVE_FAIL(MOVE_NOT_ALLOWED);                                    
        DO_REMOVE_ON_ERR; // wirklich ein Player?
        return MOVE_NOT_ALLOWED;
    }

    mv_infos[MOVE_OBJECT] = this_object();
    secured_mv_infos = mv_infos & MOVE_PROTECTED_LIST;
    if (!flag_force)
    {
        // modify_move...
        if (this_object() && objectp(mv_infos[MOVE_OLD_ROOM]))
            mv_infos[MOVE_OLD_ROOM]->modify("move_out",mv_infos);
        if (this_object())
            this_object()->modify("move",mv_infos);
        if (this_object() && objectp(mv_infos[MOVE_NEW_ROOM]))
            mv_infos[MOVE_NEW_ROOM]->modify("move_in",mv_infos);
        if(!this_object())
            return MOVE_DESTRUCTED;
        
        secured_mv_infos = mv_infos & MOVE_PROTECTED_LIST;
        
        // forbidden_move und Co...
        if ( (this_object() && mv_infos[MOVE_OLD_ROOM]
            && (res = mv_infos[MOVE_OLD_ROOM]->let_not_out(mv_infos))) ||
            (this_object()&&mv_infos[MOVE_OLD_ROOM]
            && (res = mv_infos[MOVE_OLD_ROOM]->forbidden("move_out",mv_infos))) ||
            (this_object()&&mv_infos[MOVE_NEW_ROOM]
            && (res = mv_infos[MOVE_NEW_ROOM]->let_not_in(mv_infos))) ||
            (this_object()&&mv_infos[MOVE_NEW_ROOM]
            && (res = mv_infos[MOVE_NEW_ROOM]->forbidden("move_in",mv_infos))) ||
            (this_object()
            && (res = this_object()->forbidden("move",mv_infos))) )
        {
            mv_infos+=secured_mv_infos;
            if (stringp(res))
                not_moved_reason = res;
            if (not_moved_reason && living(this_object()))
                this_object()->send_message_to(this_object(), MT_FEEL|MT_NOTIFY,
                    MA_MOVE, wrap(not_moved_reason));
            DO_NOTIFY_MOVE_FAIL(MOVE_NOT_ALLOWED);                                
            DO_REMOVE_ON_ERR;
            if(!intp(res) && !stringp(res))
            {
                do_my_error(sprintf(
                    "Ungültiger Rückgabewert res: %Q.\nmv_infos:%Q\n",
                    res, mv_infos));
                return MOVE_NOT_ALLOWED;
            }
            return stringp(res) ? MOVE_NOT_ALLOWED :
                      (res == 1 ? MOVE_NOT_ALLOWED : res);
        }
    }
    if(!this_object())
        return MOVE_DESTRUCTED;
    mv_infos+=secured_mv_infos;
    // Gewicht im neuen Raum dazuaddieren
    if (flag_force)
    {
        mv_infos[MOVE_NEW_ROOM]->add_encumbrance(
                this_object(),ENC_ADD|ENC_FORCE,0);
    }
    else if (!mv_infos[MOVE_NEW_ROOM]->add_encumbrance(this_object(), ENC_ADD,0))
    {
        if(living(mv_infos[MOVE_NEW_ROOM]))
        {
            // Nur eine einfache Heuristik.
            if(2*mv_infos[MOVE_NEW_ROOM]->query_internal_encumbrance()>=
                    this_object()->query_weight())
            {
                if(this_player() == mv_infos[MOVE_NEW_ROOM])
                    not_moved_reason = "Du bist viel zu bepackt, um "
                        +den()+" noch tragen zu können.";
                else
                    not_moved_reason = Der(mv_infos[MOVE_NEW_ROOM])
                        +ist(mv_infos[MOVE_NEW_ROOM], IST_SPACE_BEFORE)
                        +" viel zu bepackt, um "+
                        den()+" auch noch tragen zu können.";
            }
            else
            {
                if(this_player() == mv_infos[MOVE_NEW_ROOM])
                    not_moved_reason = Der()
                        +ist(this_object(), IST_SPACE_BEFORE)
                        +" viel zu schwer für Dich.";
                else
                    not_moved_reason = Der(mv_infos[MOVE_NEW_ROOM])
                        +plural(" kann "," können ", mv_infos[MOVE_NEW_ROOM])
                        +den()+" nicht tragen.";
            }
        }
        else if(!mv_infos[MOVE_NEW_ROOM]->query_container())
        {
            if (!mv_infos[MOVE_NEW_ROOM]->query_name()
                    ||!mv_infos[MOVE_NEW_ROOM]->query_gender())
            {
                not_moved_reason = "Dafür ist das Ziel nicht gedacht.";
            }
            else
            {
                not_moved_reason = "Dafür "
                    +ist(mv_infos[MOVE_NEW_ROOM],IST_SPACE_AFTER)
                    +der(mv_infos[MOVE_NEW_ROOM])+" nicht gedacht.";
            }
        }
        else
            not_moved_reason = "Da ist nicht genug Platz für " + den() + ".";
        DO_NOTIFY_MOVE_FAIL(MOVE_NO_ROOM);                                
        DO_REMOVE_ON_ERR;
        return MOVE_NO_ROOM;
    }
    if (this_object())
        this_object()->notify("move", mv_infos); // war notify_before_move!!
    mv_infos+=secured_mv_infos;
    if (this_object() && mv_infos[MOVE_NEW_ROOM])
        mv_infos[MOVE_NEW_ROOM]->notify("move_in",mv_infos);
    if(!this_object())
        return MOVE_DESTRUCTED;
    mv_infos+=secured_mv_infos;

    if (mv_infos[MOVE_OLD_ROOM])
    {
        // Meldungen an self
        if (!flag_force && (member(mv_infos,MOVE_MSG_ME) 
                || mv_infos[MOVE_FLAGS] & MOVE_ATOM_MESSAGE))
            this_object()->send_move_msg_self(mv_infos);
        mv_infos+=secured_mv_infos;

        mv_infos[MOVE_OLD_ROOM]->notify("move_out",mv_infos);
        mv_infos+=secured_mv_infos;

        if(!this_object())
            return MOVE_DESTRUCTED;

        // Gewicht im alten Raum abziehen
        if (mv_infos[MOVE_OLD_ROOM])
            mv_infos[MOVE_OLD_ROOM]->add_encumbrance(
                this_object(),ENC_REMOVE,0);

        if (!flag_force && (member(mv_infos,MOVE_MSG_LEAVE) 
                || mv_infos[MOVE_FLAGS] & MOVE_ATOM_MESSAGE)
                && this_object())
        {
            this_object()->send_move_out_msg(mv_infos);
            mv_infos+=secured_mv_infos;
        }

        // Log, wegen Teleportieren
        if (query_once_interactive(this_object()) &&
            this_interactive() != this_object() &&
            wizp(this_interactive()) &&
            !wizp(this_object()) &&
            !testplayerp(this_object()))
            "/secure/log_move"->log_move(
                mv_infos[MOVE_OLD_ROOM], mv_infos[MOVE_NEW_ROOM]);

        // Log zur Information
        if (playerp(this_object()))
            "/secure/log_move"->log_move_player(
                mv_infos[MOVE_OLD_ROOM], mv_infos[MOVE_NEW_ROOM],
                mv_infos[MOVE_FLAGS]);
    }
    if(!this_object())
        return MOVE_DESTRUCTED;
    // Eigentlicher Move
    efun::move_object(this_object(), mv_infos[MOVE_NEW_ROOM]);

    if (!flag_force &&(member(mv_infos,MOVE_MSG_ENTER) 
            || mv_infos[MOVE_FLAGS] & MOVE_ATOM_MESSAGE)
            && this_object())
    {
        this_object()->send_move_in_msg(mv_infos);
        mv_infos+=secured_mv_infos;
    }


    if (make_visible_at_next_move)
    {
        make_visible_at_next_move = 0;
        this_object()->set_invis(V_VIS);
    }

    if (mv_infos[MOVE_OLD_ROOM])
    {
        mv_infos[MOVE_OLD_ROOM]->moved_out(mv_infos);
        if (mv_infos[MOVE_OLD_ROOM] && this_object())
        {
            mv_infos[MOVE_OLD_ROOM]->notify("moved_out", mv_infos);
            mv_infos+=secured_mv_infos;
        }
    }

    if(mv_infos[MOVE_NEW_ROOM] && this_object())
    {
    // Das ist ein Hack, damit die temp_traces vor dem just_moved gesetzt sind.
    // Zu entfernen, wenn es bessere Loesungen gibt.
        mv_infos[MOVE_NEW_ROOM]->early_moved_in(mv_infos);
        mv_infos+=secured_mv_infos;
    }

    if (!(mv_infos[MOVE_FLAGS] & MOVE_ATOM_NOT_NOTIFY) && this_object())
        this_object()->just_moved();

    if(mv_infos[MOVE_NEW_ROOM] && this_object()) // Manche just_moved reagieren explosiv... :(
    {
        mv_infos[MOVE_NEW_ROOM]->moved_in(mv_infos);
        mv_infos+=secured_mv_infos;
        if(mv_infos[MOVE_NEW_ROOM] && this_object())
        {
            mv_infos[MOVE_NEW_ROOM]->notify("moved_in", mv_infos);
            mv_infos+=secured_mv_infos;
        }
    }

    // followers entfernt!
    if (this_object())
        this_object()->notify("moved", mv_infos);// Achtung umbenannt!
    return MOVE_OK;
}

/*
FUNKTION: move_or_remove
DEKLARATION: nomask int move_or_remove(<string|object> dest)
BESCHREIBUNG:
Diese Funktion bewegt das Objekt an das angegebene Ziel. Klappt dies
nicht wird das Objekt entfernt.
Der Rueckgabewert ist ungleich 0, wenn der Move geklappt hat.

Die Funktion ist eine Abkuerzung fuer
  move(dest, ([MOVE_FLAGS:MOVE_ERR_REMOVE])) == MOVE_OK

VERWEISE: move
GRUPPEN: move, grundlegendes
*/
nomask int move_or_remove(<string|object> dest)
{
    return (move(dest, ([MOVE_FLAGS:MOVE_ERR_REMOVE])) == MOVE_OK);
}


/*
FUNKTION: allowed_take_living
DEKLARATION: int allowed_take_living(object was, object wohin)
BESCHREIBUNG:
Wenn das Lebewesen 'was' in das Lebewesen 'wohin' bewegt werden soll, und
'was' etwas mehr als 1 wiegt, wird was->allowed(was, wohin) gefragt.
allowed ruft dann in allen mit was->add_controller("allowed_take_living",
other) angemeldeten Objekten other die Funktion other->allowed_take_living(was,
wohin) auf. Wurde mindestens ein solches Objekt angemeldet und liefern alle
Funktionen einen Wert != 0, dann wird die Bewegung gestattet, ansonsten
verboten.
Somit kann man also Lebewesen bauen, die man aufnehmen kann, und die doch
schwerer als 1 sind, indem man einen solchen Controller anmeldet, welcher 1
liefert.
VERWEISE: allowed, add_controller, take, forbidden_take, forbidden_take_me
GRUPPEN: monster, haende
*/

/*
FUNKTION: remove
DEKLARATION: int remove()
BESCHREIBUNG:
Entfernt ein Objekt und zerstoert es dann. Man sollte Objekte immer erst mit
remove entfernen, erst, wenn das nicht geklappt hat, mit destruct(). Wenn
remove erfolgreich war, liefert es 1 zurueck. Remove kann in Objekten problem-
los ueberlagert werden, es dient dann dazu, noch letzte Aktionen vor dem
Ableben auszufuehren, z.B. ein save_object().
VERWEISE: move, destruct
GRUPPEN: move, grundlegendes
*/
int remove()
{
    destruct(this_object());
    return 1;
}

static void set_first_room(string po, string dest)
{
    string name;
    object ob;

    if (first_room)
	return;

    ob = this_object();
    first_room = "";
    while (ob = environment(ob))
    {
	if (!(name = ob->query_real_name()) &&
		!(name = ob->query_name()) &&
		!(name = ob->query_short()))
	    name = "*UNKNOWN*";
	first_room = object_name(ob) + "(" + name + ")|" + first_room;
    }
    if (first_room == "")
    {
	// Freaky: Hier ist wohl ein Fehler passiert :(
	if (sscanf(object_name(),"%s#%~d",name) == 2)
	    name += "#<nr>";
	else
	    name = object_name();
	do_error2(sprintf(
	    "Objekt %s (%s UID: %s) wurde nicht korrekt bewegt.\n\n"
	    "Zielraum: %s\nprevious_object(): %s\n",
	    name, Name(this_object()), getuid(), dest, po),
	    __FILE__, explode(creator," (")[0], __LINE__);
	remove();
    }
#ifdef LOG_OBJECT_STATS
    else
	this_object()->log_object_stats();
#endif
}

/*
FUNKTION: query_first_room
DEKLARATION: nomask string query_first_room()
BESCHREIBUNG:
Liefert den Raum zurueck, in den ein Objekt zum ersten Mal gemoved wurde.
Nach einem Auslagern kann das ein Schliessfachraum sein, siehe
query_real_first_room.
VERWEISE: move, query_real_first_room
GRUPPEN: move, grundlegendes
*/
nomask string query_first_room() { return first_room; }

/*
FUNKTION: query_real_first_room
DEKLARATION: nomask string query_real_first_room()
BESCHREIBUNG:
Liefert den Raum zurueck, in den ein Objekt zum ersten Mal gemoved wurde.
Und zwar vor dem ersten Ein-/Auslagern.
VERWEISE: move
GRUPPEN: move, grundlegendes
*/
nomask string query_real_first_room()
{
    mapping origin = this_object()->query(P_ORIGIN);
    return sizeof(origin) ? origin[P_ORIGIN_ROOM] : first_room;
}

/*
FUNKTION: query_first_player
DEKLARATION: nomask string query_first_player()
BESCHREIBUNG:
Liefert den Spieler zurueck, in dem das aktuelle Objekt zum ersten Mal
gemoved wurde. Wegen den Schliessfaechern ist besser query_real_first_player
zu verwenden.
VERWEISE: move, query_real_first_player
GRUPPEN: move, grundlegendes
*/
nomask string query_first_player() { return first_player; }

/*
FUNKTION: query_real_first_player
DEKLARATION: nomask string query_real_first_player()
BESCHREIBUNG:
Liefert den Spieler zurueck, in dem das urspruengliche Objekt zum ersten Mal
gemoved wurde, und zwar inkl. einem Ein-Auslagern in Schliessfaecher.
VERWEISE: move, query_first_player
GRUPPEN: move, grundlegendes
*/
nomask string query_real_first_player()
{
    mapping origin = this_object()->query(P_ORIGIN);
    return sizeof(origin) ? origin[P_ORIGIN_PLAYER] : first_player;
}

/*
FUNKTION: query_last_player
DEKLARATION: nomask string query_last_player()
BESCHREIBUNG:
Liefert den Spieler zurueck, der das Objekt zuletzt in den Haenden hielt.
VERWEISE: move
GRUPPEN: move, grundlegendes
*/
nomask string query_last_player() { return last_player; }

// Wird von let_not_in in player.c aufgerufen
void set_first_player()
{
    if(!playerp(previous_object()))
	return;
    last_player = previous_object()->query_real_name();
    if (!first_player)
	first_player = previous_object()->query_real_name();
}

/*
FUNKTION: query_creator
DEKLARATION: nomask string query_creator()
BESCHREIBUNG:
Liefert das Objekt, welches diesen Gegenstand erschaffen hat.
Kann beim Auslagern ein Schliessfaecher-Objekt sein, das urpsruengliche
Objekt kann mit query_real_creator abgefragt werden.
Der String hat die Form: "Dateiname (UID: uid, EUID: euid)"
VERWEISE: move, query_real_creator
GRUPPEN: move, grundlegendes
*/
nomask string query_creator() { return creator; }

/*
FUNKTION: query_real_creator
DEKLARATION: nomask string query_real_creator()
BESCHREIBUNG:
Liefert das Objekt, welches vor dem ersten Einlagern diesen Gegenstand
erschaffen hat.
Der String hat die Form: "Dateiname (UID: uid, EUID: euid)"
VERWEISE: move, query_creator
GRUPPEN: move, grundlegendes
*/
nomask string query_real_creator()
{
    mapping origin = this_object()->query(P_ORIGIN);
    return (sizeof(origin)&&stringp(origin[P_ORIGIN_CREATOR]))
                                 ? origin[P_ORIGIN_CREATOR] : creator;
}

/*
FUNKTION: query_enable_cleanup
DEKLARATION: int query_enable_cleanup()
BESCHREIBUNG:
Beim Cleanup im Raum fragt der Raum jeden Gegenstand, ob er das Zerstoeren
des Raumes erlaubt. Liefert query_enable_cleanup() bei einem Gegenstand 0,
so darf sich der Raum nicht zerstoeren. Im Normalfall liefert
query_enable_cleanup 0, wenn sich der Gegenstand nicht mehr in dem Raum
befindet, in welchem er erschaffen wurde, ansonsten eine 1. Soll ein
Gegenstand immer ein Cleanup verhindern, so muss set_prevent_cleanup()
aufgerufen werden. Daraufhin liefert query_enable_cleanup() immer 0.
VERWEISE: clean_up, set_prevent_cleanup
GRUPPEN: raum
*/
int query_enable_cleanup()
{
    string room;

    if (this_object()->query_prevent_cleanup())
	return 0;

    if (!first_room)  // objekt durch das resetten im cleanup erzeugt und der
	return 1;   // call_out noch nicht ausgefuehrt, oder wirklich kein move

    sscanf(first_room,"%s(%~s",room);

    if (object_name(all_environment()[<1]) != room)
	return 0;
    if (living(this_object()))
	return 1;
    // Hier alle Objekte untersuchen, die in dem Objekt drin sind.
    // Machen wir momentan aber nur bei Nicht-Livings
    for (object ob = first_inventory(); ob; ob = next_inventory(ob))
	if (!ob->query_enable_cleanup())
	    return 0;
    return 1;
}

/*
FUNKTION: query_prevent_cleanup
DEKLARATION: int query_prevent_cleanup()
BESCHREIBUNG:
Liefert 1, wenn set_prevent_cleanup() aufgerufen wurde.
VERWEISE: set_prevent_cleanup, clean_up, query_enable_cleanup
GRUPPEN: raum
*/
int query_prevent_cleanup() { return prevent_cleanup; }

/*
FUNKTION: set_prevent_cleanup
DEKLARATION: void set_prevent_cleanup()
BESCHREIBUNG:
Nach Aufruf dieser Funktion wird ein Raum, in welchem dieser Gegenstand
liegt, nicht mehr beim Cleanup zerstoert.
VERWEISE: clean_up, query_enable_cleanup, set_enable_cleanup
GRUPPEN: raum
*/
void set_prevent_cleanup() { prevent_cleanup = 1; }

/*
FUNKTION: set_enable_cleanup
DEKLARATION: void set_enable_cleanup()
BESCHREIBUNG:
Nach Aufruf dieser Funktion wird ein Raum, in welchem dieser Gegenstand
liegt, wieder beim Cleanup zerstoert. Diese Funktion ist das Gegenstueck
zu set_prevent_cleanup.
VERWEISE: clean_up, query_enable_cleanup, set_prevent_cleanup
GRUPPEN: raum
*/
void set_enable_cleanup() { prevent_cleanup = 0; }

/*
FUNKTION: set_hidden_until_next_move
DEKLARATION: void set_hidden_until_next_move([int invis_level])
BESCHREIBUNG:
Diese Funktion macht nix anderes, als ein Objekt so lange zu verbergen,
bis es das erste mal bewegt wird. Siehe als Beispiel: /room/bsp/bsp9.c

Ohne Parameter aufgerufen wird als invis_level V_HIDDEN angenommen.
VERWEISE: query_hidden_until_next_move, move, set_invis
GRUPPEN: move, grundlegendes
*/
void set_hidden_until_next_move(varargs mixed invis_level)
{
    int level;

    if (sizeof(invis_level) == 1)
	level = invis_level[0];
    else
	level = V_HIDDEN;
    this_object()->set_invis(level);
    make_visible_at_next_move = 1;
}

/*
FUNKTION: query_hidden_until_next_move
DEKLARATION: int query_hidden_until_next_move()
BESCHREIBUNG:
Die Funktion liefert 1, wenn das Objekt bei der naechsten Bewegung
sichtbar gemacht wird, ansonsten 0.
VERWEISE: set_hidden_until_next_move, move, set_invis
GRUPPEN: move, grundlegendes
*/
int query_hidden_until_next_move()
{
    return make_visible_at_next_move;
}

/*
FUNKTION: set_no_move
DEKLARATION: void set_no_move(int no_move)
BESCHREIBUNG:
Mit Wert 1 kann man das Objekt fest installieren, es kann dann nicht mehr
bewegt werden, mit 0 wird es wieder freigeben.
Mit set_no_move_reason kann man eine Begruendung angeben.
GRUPPEN: bewegen
VERWEISE: move, query_no_move, set_no_move_reason
*/
void set_no_move(int i) { no_move = i != 0; }

/*
FUNKTION: query_no_move
DEKLARATION: int query_no_move()
BESCHREIBUNG:
Mit dieser Funktion kann man abfragen, ob das Objekt im Moment installiert ist.
Mit query_no_move_reason kann man eine ggf. gesetzte Begruendung abfragen.
GRUPPEN: bewegen
VERWEISE: move, set_no_move, query_no_move_reason, set_no_move_reason
*/
int query_no_move() { return no_move; }

// ACHTUNG: Der Vergleich in der unteren Doku befindet sich auch in
//          der Doku zu set_not_moved_reason.
/*
FUNKTION: set_no_move_reason
DEKLARATION: void set_no_move_reason(string no_move_reason)
BESCHREIBUNG:
Mit dieser Funktion kann eine Begruendung dafuer gesetzt werden, dass ein
Objekt nicht bewegbar ist.
Beispiel: Der Baum ist mit seinen Wurzeln fest angewachsen, die Liste ist an
die Wand genagelt.

ACHTUNG: Es kommt desoefteren zur Verwechslung mit set_not_moved_reason.
Daher hier die Unterschiede:

    set_not_moved_reason:
	- Wird erst WAEHREND des Bewegungsversuches (also von
          let_not_in- oder forbidden_move*-Funktionen) gesetzt.
	- Wird vor jedem Bewegungsversuch geloescht.
	- Ist fuer die Begruendung des Fehlschlages EINER Bewegung da.

    set_no_move_reason:
        - Wird gewoehnlich bei der Initialisierung des Objektes
	  (meist zusammen mit set_no_move(1) im create()) aufgerufen.
	- Ist die Begruendung fuer alle wegen eines set_no_move(1)
	  fehlgeschlagener Bewegungen.

GRUPPEN: bewegen
VERWEISE: move, query_no_move, set_no_move, query_no_move_reason
*/
void set_no_move_reason(string s) { no_move_reason = s; }

/*
FUNKTION: query_no_move_reason
DEKLARATION: string query_no_move_reason()
BESCHREIBUNG:
Mit dieser Funktion kann man abfragen, warum ein Objekt nicht bewegbar ist.
GRUPPEN: bewegen
VERWEISE: move, set_no_move, query_no_move, set_no_move_reason
*/
string query_no_move_reason() { return no_move_reason; }

// ACHTUNG: Der Vergleich in der unteren Doku befindet sich auch in
//          der Doku zu set_no_move_reason.
/*
FUNKTION: set_not_moved_reason
DEKLARATION: void set_not_moved_reason(string not_moved_reason)
BESCHREIBUNG:
Hiermit kann beispielsweise von Filtern wie let_not_in aus in einem
Objekt eine Begruendung angegeben werden, warum es nicht bewegt
werden konnte. Diese kann mit query_not_moved_reason abgefragt werden.

ACHTUNG: Es kommt desoefteren zur Verwechslung mit set_no_move_reason.
Daher hier die Unterschiede:

    set_not_moved_reason:
	- Wird erst WAEHREND des Bewegungsversuches (also von
          let_not_in- oder forbidden_move*-Funktionen) gesetzt.
	- Wird vor jedem Bewegungsversuch geloescht.
	- Ist fuer die Begruendung des Fehlschlages EINER Bewegung da.

    set_no_move_reason:
        - Wird gewoehnlich bei der Initialisierung des Objektes
	  (meist zusammen mit set_no_move(1) im create()) aufgerufen.
	- Ist die Begruendung fuer alle wegen eines set_no_move(1)
	  fehlgeschlagener Bewegungen.

GRUPPEN: bewegen
VERWEISE: move, query_not_moved_reason, set_no_move, set_no_move_reason,
let_not_out, let_not_in
*/
void set_not_moved_reason(string s) { not_moved_reason = s; }

/*
FUNKTION: query_not_moved_reason
DEKLARATION: string query_not_moved_reason()
BESCHREIBUNG:
Mit dieser Funktion kann man abfragen, warum der letzte move fehlgeschlagen
ist, natuerlich nur dann, wenn der letzte move fehlgeschlagen ist.
Ist das Ergebnis 0, so liegt keine eigene Klartextbegruendung vor.
In diesem Fall kann dem Spieler eine Standardmeldung angezeigt werden.
GRUPPEN: bewegen
VERWEISE: move, set_not_moved_reason
*/
string query_not_moved_reason() { return not_moved_reason; }


// Nun noch einige Funktionen, die von move aufgerufen werden, aber in /i/move
// nicht selber definiert sind, sondern im Objekt dann definiert werden

/*
FUNKTION: just_moved
DEKLARATION: void just_moved()
BESCHREIBUNG:
Diese Funktion wird im bewegten Objekt nach einem erfolgreichen Move
aufgerufen.
Diese Funktion wird nicht aufgerufen, wenn der move MOVE_SECRET ist!
VERWEISE: move
GRUPPEN: move
*/

/*
FUNKTION: move_return
DEKLARATION: varargs int move_return(mixed ziel, int delay, closure call_back, mixed events, int how, string verlassen, string ankommen, int how_back, string verlassen_back, string ankommen_back, string move_type)
BESCHREIBUNG:
Mit dieser Funktion kann man das Objekt bewegen, so dass es nach 'delay'
Sekunden wieder an seinen alten Platz zurueckkehrt.

Die Parameter 'ziel, how, verlassen, ankommen' sind die Parameter fuer
den move an die neue Stelle.
Die Parameter 'how_back, verlassen_back, ankommen_back' sind die Parameter
fuer den move an die alte Stelle zurueck.
Falls how_back nicht angegeben wird, wird MOVE_NORMAL angenommen.
Bei how_back == MOVE_NORMAL wird versucht, einen Ausgang zu finden, der
zum alten Raum zurueckfuehrt.

Die Call-Back-Closure wird aufgerufen, wenn das Objekt an seinen alten Platz
zurueckgekehrt ist. Sie wird mit folgenden Parametern aufgerufen:
   this_object(), Return des moves (oder -1, wenn kein move noetig war),
   der Raum, in den das Objekt zurueckkehren sollte.

Diese Funktion kann auch mehrmals hintereinander aufgerufen werden.
Das Objekt kehrt dann nacheinander an die Stellen zurueck, an denen es
weiterbewegt wurde.

Wenn events uebergeben wird, koennen alle Moves dieses Event-Typs
auf einmal rueckgaengig gemacht werden.
Events kann ein String oder ein Array of Strings sein.
Delay < 0 bedeutet, dass dieser Move nur durch ein Event wieder
rueckgaengig gemacht werden kann.
VERWEISE: move, return_to
GRUPPEN: move
*/
varargs int move_return(mixed ziel, int delay, closure call_back, mixed events,
	int how, string verlassen, string ankommen,
	int how_back, string verlassen_back, string ankommen_back,
	string move_type)
{
    int ret;
    string old_room;

    if (environment())
        old_room = object_name(environment());

    ret = this_object()->move(ziel, ([
        MOVE_FLAGS:     how,
        MOVE_TYPE:      move_type,
        MOVE_MSG_LEAVE: verlassen,
        MOVE_MSG_ENTER: ankommen]) );
    if (ret != MOVE_OK || !this_object())
        return ret;

    remove_call_out("return_to");
    if (delay >= 0)
        call_out("return_to",delay || 1);
    return_move += ({ ({ call_back, delay < 0 ? -1 : time() + delay, events,
	old_room, how_back, verlassen_back, ankommen_back, move_type }) });
    return ret;
}

/*
FUNKTION: return_to
DEKLARATION: varargs int return_to(string event)
BESCHREIBUNG:
Bewegt das Objekt vorzeitig an seine alte Stelle zurueck.
Returnwert:
  -3, wenn der letzte Move kein Move war, auf den das event passt.
  -2, wenn das Objekt keine alte Stelle hatte.
  -1, wenn das Objekt schon an der alten Stelle ist.
  sonst: Returnwert des Moves.

Wenn event uebergeben wurde, werden alle Moves dieses Event-Typs
rueckgaengig gemacht.
VERWEISE: move_return
GRUPPEN: move
*/
varargs int return_to(string event)
{
    string *exit_list;
    mixed last;
    string to;
    int i, ret, delay, how, event_return;

    if (event)
    {
	for (i = sizeof(return_move); i--; )
	    if ((stringp(return_move[i][2]) && return_move[i][2] == event) ||
		    (pointerp(return_move[i][2]) &&
		     member(return_move[i][2],event) >= 0))
	    {
		return_move[i][1] = 0;
		if (i == sizeof(return_move) - 1)
		    event_return = 1;
	    }
	if (!event_return)
	    return -3;
    }

    remove_call_out("return_to");

    if (!sizeof(return_move))
	return -2;

    last = return_move[<1];
    return_move = return_move[0..<2];

    if (object_name(environment()) != last[3])
    {
	how = last[4];
	if (!how)
	    how = MOVE_NORMAL;

	if (how == MOVE_NORMAL)
	{
	    exit_list = environment()->query_exit_list(EXIT_UNLOCKED);
	    if (sizeof(exit_list))
		if ((i = member(exit_list, last[3])) >= 0)
		    to = environment()->query_command_list(EXIT_UNLOCKED)[i];

	    if (to)
		this_object()->move(to,([
		    MOVE_FLAGS:     how,
		    MOVE_TYPE:      last[7],
		    MOVE_MSG_LEAVE: last[5],
		    MOVE_MSG_ENTER: last[6] ]));
	}

	if (object_name(environment()) != last[3])
        ret = this_object()->move(last[3],([
                MOVE_FLAGS:     how,
                MOVE_TYPE:      last[7],
                MOVE_MSG_LEAVE: last[5],
                MOVE_MSG_ENTER: last[6] ]));
    }
    else
	ret = -1;

    if (sizeof(return_move))
    {
	delay = return_move[<1][1];
	if (delay >= 0)
	{
	    delay -= time();
	    if (delay < 0)
		delay = 0;
	    call_out("return_to",delay || 1);
	}
    }
    funcall(last[0],this_object(),ret,last[3]);
    return ret;
}

/*
FUNKTION: notify_moved
DEKLARATION: void notify_moved(mapping mv_infos)
BESCHREIBUNG:
Nach der Ausfuehrung des moves wird am bewegten Objekt notify_moved aufgerufen.
Folgende Parameter stehen zur Information bereit:
    MOVE_FLAGS      : Die flags, wie sie an move(siehe dort) uebergeben wurden.
    MOVE_OBJECT     : Das bewegte Objekt
    MOVE_DEST_STR   : Wenn das ziel in move ein String war
    MOVE_DIRECTION  : Ein Ausgang falls dieser im ziel spezifiziert war.
    MOVE_OLD_ROOM   : Der Ausgangsraum (ehemalige Umgebung) des Objekts
    MOVE_NEW_ROOM   : Der Zielraum (jetzige Umgebung) des Objekts.
    MOVE_EXIT_INFO  : Die Ausgangsinformationen zu dem benutzen Ausgang.
    MOVE_MSG_LEAVE (string): Eine Bewegungsmeldung an die Umgebenden im 
                             Ausgangsraum ehemals der Parameter verlassen.
    MOVE_MSG_ENTER (string): Eine Bewegungsmeldung an die Umgebenden im 
                             Zielraum ehemals der Parameter ankommen.
    MOVE_MSG_ME    (string): Bewegungsmeldung an den Bewegenden (neu)
    MOVE_MSG_LEAVE_OTHERS : Mit diesem Array kann man Bewegungsmeldungen
                             an Dritte im Ausgangsraum formulieren:
        ({ <object|object*> whom1, <string> meldung_an_whom1,
           <object|object*> whom2, <string> meldung_an_whom2, ... })
    MOVE_MSG_ENTER_OTHERS : Gleicher Aufbau und Funktion fuer den Zielraum.
    MOVE_TYPE (string): Enthaelt das Verb fuer die Art der Bewegung.
Alle defines stammen aus /sys/move.h
VERWEISE: move, modify, modify_move,forbidden_move, notify_move,notify_moved_in,notify_moved_out
GRUPPEN: move
*/

/*
FUNKTION: modify_move
DEKLARATION: void modify_move(mapping mv_infos)
BESCHREIBUNG:
Vor der Ausfuehrung des moves und vor den forbidden-Controllern wird durch
modify_move am bewegenden Objekt ermoeglicht, folgende Parameter zu aendern:
    MOVE_MSG_LEAVE (string): Eine Bewegungsmeldung an die Umgebenden im 
                             Ausgangsraum ehemals der Parameter verlassen.
    MOVE_MSG_ENTER (string): Eine Bewegungsmeldung an die Umgebenden im 
                             Zielraum ehemals der Parameter ankommen.
    MOVE_MSG_ME    (string): Bewegungsmeldung an den Bewegenden (neu)
    MOVE_MSG_LEAVE_OTHERS : Mit diesem Array kann man Bewegungsmeldungen
                             an Dritte im Ausgangsraum formulieren:
        ({ <object|object*> whom1, <string> meldung_an_whom1,
           <object|object*> whom2, <string> meldung_an_whom2, ... })
    MOVE_MSG_ENTER_OTHERS : Gleicher Aufbau und Funktion fuer den Zielraum.
    MOVE_TYPE (string): Enthaelt das Verb fuer die Art der Bewegung.
Man kann mit member(MOVE_MODIFY_LIST,<param>) auch pruefen, ob der gewuenschte
Parameter auch aenderbar ist.
Folgende Parameter koennen ebenso geaendert werden mit entsprechender Vorsicht:
    MOVE_FLAGS      : Die flags, wie sie an move(siehe dort) uebergeben wurden.
    MOVE_OBJECT     : Das zu bewegende Objekt
    MOVE_DEST_STR   : Wenn das ziel in move ein String war
    MOVE_DIRECTION  : Ein Ausgang falls dieser im ziel spezifiziert war.
    MOVE_OLD_ROOM   : Der Ausgangsraum (aktuelle Umgebung) des Objekts
    MOVE_NEW_ROOM   : Der (schon geladene) Zielraum des Objekts.
    MOVE_EXIT_INFO  : Die Ausgangsinformationen zu dem benutzen Ausgang.
Die Reihefolge der modify-Controler ist modify_move_out, modify_move, 
modify_move_in. Und sie wird nur einmal durchlaufen, auch wenn z.B. der 
Zielraum im modify_move_in geaendert wird.
Alle defines stammen aus /sys/move.h
VERWEISE: move, modify, modify_move_in, modify_move_out, forbidden_move, notify_moved
GRUPPEN: move
*/
/*
FUNKTION: modify_move_out
DEKLARATION: void modify_move_out(mapping mv_infos)
BESCHREIBUNG:
Vor der Ausfuehrung des moves und vor den forbidden-Controllern wird durch
modify_move_out am Ausgangsort ermoeglicht, die in modify_move beschriebenen
Parameter zu aendern bzw. zu nutzen.
VERWEISE: move, modify, modify_move_in, modify_move, forbidden_move, notify_moved
GRUPPEN: move
*/
/*
FUNKTION: modify_move_in
DEKLARATION: void modify_move_in(mapping mv_infos)
BESCHREIBUNG:
Vor der Ausfuehrung des moves und vor den forbidden-Controllern wird durch
modify_move_in am Zielort ermoeglicht, die in modify_move beschriebenen
Parameter zu aendern bzw. zu nutzen.
VERWEISE: move, modify, modify_move, modify_move_out, forbidden_move, notify_moved
GRUPPEN: move
*/
/*
FUNKTION: forbidden_move
DEKLARATION: <int|string> forbidden_move(mapping mv_infos)
BESCHREIBUNG:
Vor der Ausfuehrung des moves und nach den modify-Controllern wird durch
forbidden_move an dem zu bewegenden Objekt geprueft, ob eine Bewegung
moeglich ist (Rueckgabewert 0). Wenn nicht, kann entweder ein String als
Begruendung zurueckgegeben oder eine 1 zusammen mit einem set_not_moved_reason
zurueckgegeben werden. Die Parameter des Mappings mv_infos ist in notify_moved
erlaeutert.
VERWEISE: move, forbidden, notify_moved, forbidden_move_in, forbidden_move_out
GRUPPEN: move
*/
/*
FUNKTION: forbidden_move_out
DEKLARATION: <int|string> forbidden_move_out(mapping mv_infos)
BESCHREIBUNG:
Vor der Ausfuehrung des moves und nach den modify-Controllern wird durch
forbidden_move_out im Ausgangsort geprueft, ob eine Bewegung
moeglich ist (Rueckgabewert 0). Wenn nicht, kann entweder ein String als
Begruendung zurueckgegeben oder eine 1 zusammen mit einem set_not_moved_reason
zurueckgegeben werden. Die Parameter des Mappings mv_infos ist in notify_moved
erlaeutert.
VERWEISE: move, forbidden, notify_moved, forbidden_move_in, forbidden_move
GRUPPEN: move
*/
/*
FUNKTION: forbidden_move_in
DEKLARATION: <int|string> forbidden_move_in(mapping mv_infos)
BESCHREIBUNG:
Vor der Ausfuehrung des moves und nach den modify-Controllern wird durch
forbidden_move_in im Zielraum geprueft, ob eine Bewegung
moeglich ist (Rueckgabewert 0). Wenn nicht, kann entweder ein String als
Begruendung zurueckgegeben oder eine 1 zusammen mit einem set_not_moved_reason
zurueckgegeben werden. Die Parameter des Mappings mv_infos ist in notify_moved
erlaeutert.
VERWEISE: move, forbidden, notify_moved, forbidden_move, forbidden_move_out
GRUPPEN: move
*/
/*
FUNKTION: notify_move_out
DEKLARATION: void notify_move_out(mapping mv_infos)
BESCHREIBUNG:
Wenn ein Objekt bewegt wird, wird im alten Raum notify("move_out", ...)
aufgerufen und zwar vor dem Move. Die in mv_infos verfuegbaren Parameter sind
in notify_moved erklaert.

Ein solcher Controller sollte nicht versuchen, ein anderes Objekt in diesen
Start- oder Zielraum zu bewegen, sondern dies erst im notify_moved erledigen.
VERWEISE: move, notify, notify_moved, forbidden_move
GRUPPEN: move
*/
/*
FUNKTION: notify_move
DEKLARATION: void notify_move(mapping mv_infos)
BESCHREIBUNG:
Wenn ein Objekt bewegt wird, wird im bewegten Objekt notify("move", ...)
aufgerufen und zwar vor dem Move. Die in mv_infos verfuegbaren Parameter sind
in notify_moved erklaert.

Ein solcher Controller sollte nicht versuchen, ein anderes Objekt in diesen
Start- oder Zielraum zu bewegen, sondern dies erst im notify_moved erledigen.
VERWEISE: move, notify, notify_moved, forbidden_move
GRUPPEN: move
*/
/*
FUNKTION: notify_move_in
DEKLARATION: void notify_move_in(mapping mv_infos)
BESCHREIBUNG:
Wenn ein Objekt bewegt wird, wird im Zielraum notify("move_in", ...)
aufgerufen und zwar vor dem Move. Die in mv_infos verfuegbaren Parameter sind
in notify_moved erklaert.

Ein solcher Controller sollte nicht versuchen, ein anderes Objekt in diesen
Start- oder Zielraum zu bewegen, sondern dies erst im notify_moved erledigen.
VERWEISE: move, notify, notify_moved, forbidden_move
GRUPPEN: move
*/
/*
FUNKTION: notify_moved_out
DEKLARATION: void notify_moved_out(mapping mv_infos)
BESCHREIBUNG:
Wenn ein Objekt bewegt wurde, wird im Ausgangsort ein notify("moved_out", ...)
aufgerufen und zwar nach dem Move. Die in mv_infos verfuegbaren Parameter sind
in notify_moved erklaert.
VERWEISE: move, notify, notify_moved, forbidden_move, notify_moved_in
GRUPPEN: move
*/
/*
FUNKTION: notify_moved_in
DEKLARATION: void notify_moved_in(mapping mv_infos)
BESCHREIBUNG:
Wenn ein Objekt bewegt wurde, wird im Zielort ein notify("moved_in", ...)
aufgerufen und zwar nach dem Move. Die in mv_infos verfuegbaren Parameter sind
in notify_moved erklaert.
VERWEISE: move, notify, notify_moved, forbidden_move, notify_moved_out
GRUPPEN: move
*/
/*
FUNKTION: notify_move_failed
DEKLARATION: void notify_move_failed(mapping mv_infos, int ret)
BESCHREIBUNG:
Konnte ein Objekt nicht bewegt werden, wird ausser im Fall MOVE_DESTRUCTED
ein notify("move_failed",...) aufgerufen. Der Rueckgabewert von move steht
in ret und die verfuegbaren Parameter sind in notify_moved erlaeutert.
VERWEISE: move, notify, notify_moved, forbidden_move, notify_move.
GRUPPEN: move
*/

/*
FUNKTION: notify_remove
DEKLARATION: void notify_remove(object ob)
BESCHREIBUNG:
Wird ob removed (via Lfun remove() oder nur Efun destruct()),
so wird vorher ob->notify("remove") aufgerufen.

Die Funktion notify ruft dann in allem mit ob->add_controller("notify_remove",
other) angemeldeten Objekten other die Funktion other->notify_remove(ob) auf.
VERWEISE: notify, remove
GRUPPEN: grundlegendes
*/

/*
BEISPIEL: move_or_remove
#include <move.h>
// ein neu angelegtes Objekt (Pfad anpassen)
    clone_object("/obj/schatz")->
        move(this_object(),([MOVE_FLAGS:MOVE_ERR_REMOVE]));
GRUPPEN: move
*/

/*
BEISPIEL: move_messages
#include <move.h>
// ein bewegtes Objekt (hier der Spieler <raum> anpassen!)
    this_player()->
        move(<raum>,([MOVE_FLAGS:MOVE_NORMAL,
         MOVE_MSG_ME:"Du stolperst noch $dir().",
         MOVE_MSG_ENTER:"$Der() stolpert heran.",
         MOVE_MSG_LEAVE:"$Der() stolpert nach $dir() davon.",
        ]));
GRUPPEN: move
*/

/*
BEISPIEL: move_normal
#include <move.h>
// ein neu angelegtes Objekt (Pfad anpassen)
    clone_object("/obj/schatz")->
        move(this_object(),([MOVE_FLAGS:MOVE_NORMAL]));
GRUPPEN: move
*/
