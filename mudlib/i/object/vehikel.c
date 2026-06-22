// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/i/object/vehikel.c
// Description: Ein betretbares Standard-Vehikel

/* --- Pragmas: --- */
#pragma save_types

/* --- Inherits: --- */
virtual inherit "/i/item";
virtual inherit "/i/move";
virtual inherit "/i/tools/move_msg";

/* --- Includes: --- */
#include <config.h>
#include <move.h>
#include <simul_efuns.h>
#include <misc.h>

/* --- Globale Variablen: --- */
private int vehikel_sloppy;
private mixed enter_room;
private string vehikel_type, msg_aussen, msg_innen, msg_self;

/* --- Funktionen: --- */

// Move nur in Vehikel-Type_erlaut - Raeume erlauben.
int forbidden_vehikel_move(string msg, mapping mv_infos)
{
    object wer = mv_infos[MOVE_OBJECT];
    object wohin = mv_infos[MOVE_NEW_ROOM];
    string type;

    if(wer && wohin &&
       wer == this_object() &&
       this_object()->query_vehikel_sloppy() == 0)
    {
        type = TO->query_vehikel();

        if(stringp(type))
        {
            return !wohin->query_type(type+"_erlaubt") &&
                   (member( ({"/room/void", "/room/hell"}), 
                    object_name(wohin)) == -1);
        }
    }
}

/*
FUNKTION: query_vehikel
DEKLARATION: string query_vehikel()
BESCHREIBUNG:
Liefert den Typ des Vehikels zurueck.

An dieser Funktion laesst sich feststellen, ob es sich bei einem Objekt um ein
von Spielern/NPCs bewegliches Gefaehrt handelt. Dies ist der Fall, wenn die
Funktion einen String zurueckliefert.

Der zurueckgelieferte String beschreibt ausserdem, um welche Art von Gefaehrt
es sich handelt. Bekanntestes Beispiel ist wohl der Vehikel-Typ "schiff".

Vehikel duerfen im Normalfall nur Raeume befahren, in denen der zum Vehikeltyp
passende erlaubt-Raumtyp gesetzt ist, z.B. "schiff_erlaubt" bei Schiffen. Dies
kann allerdings auch abgestellt werden, siehe set_vehikel_sloppy().
VERWEISE: set_vehikel, set_vehikel_sloppy
GRUPPEN: vehikel
*/
string query_vehikel()
{
    return vehikel_type;
}

/*
FUNKTION: set_vehikel
DEKLARATION: void set_vehikel(string typ)
BESCHREIBUNG:
Setzt den Typ des Vehikels auf 'typ', oder loescht ihn, wenn 'typ' == 0.
Bedeutung des Vehikel-Typs siehe bei query_vehikel().
VERWEISE: query_vehikel
GRUPPEN: vehikel
*/
void set_vehikel(string typ)
{
    if(stringp(typ))
    {
        vehikel_type = typ;
    }

    else
    {
        vehikel_type = 0;
    }
}

/*
FUNKTION: query_enter_room
DEKLARATION: varargs mixed query_enter_room(object betreter)
BESCHREIBUNG:
Die Funktion liefert den Raum zurueck, in den der Spieler gelangen soll, wenn
er versucht, das Vehikel zu betreten. 'betreter' ist hierbei ein optionaler
Parameter, naemlich der Spieler/NPC, der das Vehikel zu betreten versucht.
Dieser Parameter ist bei Ueberlagerung von query_enter_room() interessant.

Die Funktion hat folgende Rueckgabemoeglichkeiten:

        0           Das Vehikel kann man nicht betreten.
        string      Objektname des Raumes.
        object      Das Raumobjekt selbst.

VERWEISE: set_enter_room, query_enter_messages, set_enter_messages
GRUPPEN: vehikel
*/
varargs mixed query_enter_room(object betreter)
{
    return enter_room;
}

/*
FUNKTION: set_enter_room
DEKLARATION: void set_enter_room(mixed room)
BESCHREIBUNG:
Setzt den Raum, in den ein Spieler/NPC gelangen soll, wenn er das Vehikel
betritt. Moegliche Werte fuer room sind:

        0           Das Vehikel kann man nicht betreten.
        string      Objektname des Raumes.
        object      Das Raumobjekt selbst.

VERWEISE: query_enter_room, query_enter_messages, set_enter_messages
GRUPPEN: vehikel
*/
void set_enter_room(mixed room)
{
    enter_room = room;
}

/*
FUNKTION: query_enter_messages
DEKLARATION: varargs string * query_enter_messages(object betreter)
BESCHREIBUNG:
Liefert die Meldungen zurueck, die beim Betreten des Vehikels an die Umgebung
geschickt werden. Es gibt eine Meldung ausserhalb des Vehikels, eine Meldung
innerhalb (an den Raum, den man betreten hat) und eine Meldung an das
Lebewesen selber.

    Der Rueckgabewert ist ({string aussen, string innen, string selber}).

Die beiden Meldungen koennen Pseudoclosures enthalten.
VERWEISE: set_enter_messages, query_enter_room, set_enter_room
GRUPPEN: vehikel
*/
varargs string * query_enter_messages(object betreter)
{
    return ({msg_aussen, msg_innen, msg_self});
}

/*
FUNKTION: set_enter_messages
DEKLARATION: varargs void set_enter_messages(string aussen, string innen, string self)
BESCHREIBUNG:
Setzt die Meldungen, die beim Betreten des Vehikels an die Umgebung (aussen)
sowie an den betretenen Raum (innen) geschickt werden. Optional kann auch
eine Meldung an das betretende Lebewesen selber (self) angegeben werden.

Die Benutzung von Pseudoclosures ist erlaubt.
$Der('vehikel) bezeichnet hierbei das Vehikel.
VERWEISE: query_enter_messages, query_enter_room, set_enter_room
GRUPPEN: vehikel
*/
varargs void set_enter_messages(string aussen, string innen, string self)
{
    msg_aussen = aussen;
    msg_innen = innen;
    msg_self = self;
}

/*
FUNKTION: query_vehikel_sloppy
DEKLARATION: int query_vehikel_sloppy()
BESCHREIBUNG:
Liefert 1, wenn sich das Vehikel im Sloppy-Modus befindet, sonst 0.
Erklaerung siehe set_vehikel_sloppy().
VERWEISE:
GRUPPEN: vehikel
*/
int query_vehikel_sloppy()
{
    return vehikel_sloppy;
}

/*
FUNKTION: set_vehikel_sloppy
DEKLARATION: void set_vehikel_sloppy(int flag)
BESCHREIBUNG:
Ein Vehikel, dem ein gueltiger Vehikel-Typ gesetzt wurde, darf normalerweise
nur in Raeume bewegt werden, die einen entsprechenden _erlaubt-Raumtyp gesetzt
haben (z.B. "schiff_erlaubt" bei Vehikeln vom Typ "schiff").

Mit set_vehikel_sloppy(1) laesst sich diese Einschraenkung abschalten.
VERWEISE: query_vehikel_sloppy, set_vehikel_type
GRUPPEN: vehikel
*/
void set_vehikel_sloppy(int flag)
{
    vehikel_sloppy = (flag ? 1 : 0);
    // Controller sollte wohl besser immer angemeldet sein,
    // damit Ueberlagerungen von query_vehikel_sloppy funktionieren...
    add_controller("forbidden_move", #'forbidden_vehikel_move);
}

/*
FUNKTION: fahre
DEKLARATION: int fahre(string richtung)
BESCHREIBUNG:
Das Vehikel faehrt in die angegebene Richtung.
Abkuerzungen wie "n", "no", etc. werden erweitert.

Hat das Vehikel keine Umgebung, wird MOVE_NOT_ALLOWED zurueckgeliefert.
Existiert kein entsprechender Ausgang, wird MOVE_NO_DEST zurueckgeliefert.
Weitere Returncodes entsprechen /i/move::move().
VERWEISE:
GRUPPEN: vehikel
*/
int fahre(string richtung)
{
    object env;

    richtung = expand_direction(richtung, DIR_ALS_DEFAULT);
    env = environment();

    if(env)
    {
        if(env->query_one_exit(richtung))
        {
            return move(richtung, ([ MOVE_FLAGS: MOVE_NORMAL]) );
        }

        return MOVE_NO_DEST;
    }

    return MOVE_NOT_ALLOWED;
}

/* --- Applied LFuns: --- */

void create()
{
    "*"::create();

    set_id( ({ "vehikel" }) );
    set_gender("saechlich");
    set_name("vehikel");
    set_msg("$Der() entfernt sich $dir().", "$Ein() nähert sich $dir().", 1);
    set_mmsg("$Der() verschwindet in einem Lichtblitz.",
             "$Ein() erscheint in einem Lichtblitz.");
    set_weight(10000);

    add_controller("forbidden_move", #'forbidden_vehikel_move);
}

// Aus Kompatibilitaetsgruenden:
void reset()
{
    "*"::reset();
}

void init()
{
    "*"::init();
}

/* --- End of file. --- */
