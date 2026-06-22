// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/sys/move.h
// Description:	Defines fuer /i/move::move()

#ifndef MOVE_H
#define MOVE_H 1

// retcode defines for /i/move::move() 

#define MOVE_NOT_ALLOWED	0
#define MOVE_OK			1
#define MOVE_NO_ROOM		2
#define MOVE_DESTRUCTED		3
#define MOVE_DEST_CLOSED        4
#define MOVE_ENV_CLOSED         5
#define MOVE_NO_DEST            6


// move mode defines for /i/move: move(dest, MODE, ...) 

// internals 
#define MOVE_ATOM_MESSAGE       0x01
#define MOVE_ATOM_MAGIC         0x02
#define MOVE_ATOM_NOT_NOTIFY    0x04
#define MOVE_ERR_REMOVE         0x08
#define MOVE_ATOM_GHOST         0x10
#define MOVE_FORCE              0x20

// fuer den Aufruf von move 
#define MOVE_MAGIC   (MOVE_ATOM_MESSAGE | MOVE_ATOM_MAGIC)
#define MOVE_NORMAL  (MOVE_ATOM_MESSAGE)
#define MOVE_SECRET  (MOVE_ATOM_NOT_NOTIFY)
// Darf *NUR*(!) von der Mudlib genutzt werden:
#define MOVE_GHOST   (MOVE_ATOM_MESSAGE | MOVE_ATOM_MAGIC | MOVE_ATOM_GHOST)

// Zur Selektion der normalen Bewegungssdaten vom Rest.
#define MOVE_FLAGS_HOW (MOVE_ATOM_MESSAGE | MOVE_ATOM_MAGIC \
                        | MOVE_ATOM_NOT_NOTIFY)

// fuer add_encumbrance
#define ENC_ADD         0x01
#define ENC_REMOVE      0x02
#define ENC_MODIFY      0x04
#define ENC_TEST        0x10
#define ENC_TEST_ADD    0x11
#define ENC_TEST_REMOVE 0x12
#define ENC_TEST_MODIFY 0x14
#define ENC_FORCE       0x20        // NICHT VERWENDEN, NUR FUER /i/move!

#define MOVE_MSG_IN     "msg_in"
#define MOVE_MSG_OUT    "msg_out"

// Fuer Aufruf von move...
#define MOVE_FLAGS              "/i/move::move:flags"
#define MOVE_MSG_LEAVE          "/i/move::move:msg:leave"
#define MOVE_MSG_LEAVE_OTHERS   "/i/move::move:msg:leave:others"
#define MOVE_MSG_ENTER          "/i/move::move:msg:enter"
#define MOVE_MSG_ENTER_OTHERS   "/i/move::move:msg:enter:others"
#define MOVE_MSG_SELF           "/i/move::move:msg:self"
#define MOVE_MSG_ME             "/i/move::move:msg:self" /*Synonym*/
#define MOVE_MSG_ARGS           "/i/move::move:msg:args"
#define MOVE_TYPE               "/i/move::move:type"
// wird von move gefuellt und an Controller+Co weitergereicht.
#define MOVE_CALLER     "/i/move::move:caller"
#define MOVE_OBJECT     "/i/move::move:object"
#define MOVE_DEST_STR   "/i/move::move:dest:string"
#define MOVE_DIRECTION  "/i/move::move:direction"
#define MOVE_OLD_ROOM   "/i/move::move:old:room"
#define MOVE_NEW_ROOM   "/i/move::move:new:room"
#define MOVE_EXIT_INFO  "/i/move::move:exit:info"
// Liste an Schluesseln, die im modify geaendert werden duerfen.
#define MOVE_MODIFY_LIST ([MOVE_MSG_LEAVE,MOVE_MSG_LEAVE_OTHERS,\
        MOVE_MSG_ENTER,MOVE_MSG_ENTER_OTHERS,MOVE_MSG_SELF,MOVE_TYPE,\
        ])
// Liste an alle geschuetzten Schluesseln, die nach dem modify sich nicht
// mehr aendern duerfen.
#define MOVE_PROTECTED_LIST ([MOVE_CALLER,MOVE_FLAGS,MOVE_MSG_LEAVE,\
    MOVE_MSG_LEAVE_OTHERS,MOVE_MSG_ENTER,MOVE_MSG_ENTER_OTHERS,\
    MOVE_MSG_SELF,MOVE_TYPE,MOVE_OBJECT,MOVE_DEST_STR,MOVE_DIRECTION,\
    MOVE_OLD_ROOM,MOVE_NEW_ROOM,MOVE_EXIT_INFO])
// #endif

// Haende
#define MOVE_TYPE_GEBEN     "geben"
#define MOVE_TYPE_LEGEN     "legen"
#define MOVE_TYPE_HAENGEN   "h\u00e4ngen"
#define MOVE_TYPE_STECKEN   "stecken"
#define MOVE_TYPE_STELLEN   "stellen"
#define MOVE_TYPE_VERSTAUEN "verstauen"
#define MOVE_TYPE_SCHENKEN  "schenken"
#define MOVE_TYPE_NEHMEN    "nehmen"
#define MOVE_TYPE_LEEREN    "leeren"
#define MOVE_TYPE_WERFEN    "werfen"
#define MOVE_TYPE_SCHIESSEN "schie\u00dfen"
#define MOVE_TYPE_SCHIEBEN  "schieben"
#define MOVE_TYPE_FUETTERN  "f\u00fcttern"

// Fuesse
#define MOVE_TYPE_GEHEN     "gehen"
#define MOVE_TYPE_FLUECHTEN "fl\u00fcchten"
#define MOVE_TYPE_BETRETEN  "betreten"
#define MOVE_TYPE_VERLASSEN "verlassen"
#define MOVE_TYPE_FLIEGEN   "fliegen"

/*
FUNKTION: MOVE_TYPE_LISTE
DEKLARATION: Liste der Move-Typen.
BESCHREIBUNG:

Bei move() kann als MOVE_TYPE angegeben werden, welcher Art die
Bewegung ist. Dies ist ueblicherweise ein Verb im Infinitiv. Fuer
die bisher verwendeten Bewegungen gibt folgende Defines, um Typos
auszuschliessen:

Bewegungen eines Gegenstandes:
    MOVE_TYPE_GEBEN
    MOVE_TYPE_LEGEN
    MOVE_TYPE_HAENGEN
    MOVE_TYPE_STECKEN
    MOVE_TYPE_STELLEN
    MOVE_TYPE_VERSTAUEN
    MOVE_TYPE_SCHENKEN
    MOVE_TYPE_FUETTERN
    MOVE_TYPE_NEHMEN
    MOVE_TYPE_LEEREN
    MOVE_TYPE_WERFEN
    MOVE_TYPE_SCHIESSEN
    MOVE_TYPE_SCHIEBEN

Bewegungen von Personen:
    MOVE_TYPE_GEHEN
    MOVE_TYPE_FLUECHTEN
    MOVE_TYPE_BETRETEN
    MOVE_TYPE_VERLASSEN
    MOVE_TYPE_FLIEGEN
    MOVE_TYPE_SCHIEBEN

VERWEISE: move
GRUPPEN: move
*/

#endif // MOVE_H
