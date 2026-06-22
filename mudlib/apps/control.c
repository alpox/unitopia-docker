// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /apps/control.c
// Description: Der control-Container
//              Empfaengt Nachrichten und leitet sie weiter
//              an angemeldete Controller
// Author:      Kurdel (19.8.99)

// UID: Apps

/******************************************************************************

/apps/control (oder besser CONTROL, definiert in <apps.h>) dient dazu,
mudweite Ereignisse ueber das Controllerkonzept mitzuteilen.

Die Funktionalitaet und Doku wurde nach /i/tools/control.c ausgelagert!

******************************************************************************/

inherit "/i/tools/control";
#define SAVE_FILE "/var/control"

void save()
{
    "*"::prepare_save();
    save_object(SAVE_FILE);
}

void save_control()// wird per call_out in /i/tools/control aufgerufen!
{
    save();
}

void restore()
{
    restore_object(SAVE_FILE);
    "*"::finalize_restore();
}

int remove()
{
    save();
    destruct(this_object());
    return 1;
}

void create()
{
    restore();
}

int clean_up(int arg)
{
   return 0;
}


void prepare_renewal() {save();}
void abort_renewal() {}
void finish_renewal(object neu)
{
    transfer_temporary_controller(neu);
}

/*
FUNKTION: notify_midnight
DEKLARATION: void notify_midnight()
BESCHREIBUNG:
Diese Funktion wird in allen mit CONTROL->add_controller("notify_midnight", ob)
angemeldeten Objekten 'ob' aufgerufen, wenn es Mitternacht geworden ist.

ob darf dabei kein Clone sein. Angemeldete Controller bleiben ueber die
Zerstoerung von 'ob' und sogar ueber ein Armageddon hinaus gespeichert.
(Das heisst, das Objekt wird wieder geladen, wenn es Mitternacht ist,
sofern es sich nicht mit delete_controller abgemeldet hatte.)
CONTROL ist in /sys/apps.h definiert.
VERWEISE: add_controller, notify,
          notify_highnoon, notify_begin_daylight, notify_begin_darkness
GRUPPEN: Zeit
*/
/*
FUNKTION: notify_highnoon
DEKLARATION: void notify_highnoon()
BESCHREIBUNG:
Diese Funktion wird in allen mit CONTROL->add_controller("notify_highnoon", ob)
angemeldeten Objekten 'ob' aufgerufen, wenn es Mittags geworden ist.

ob darf dabei kein Clone sein. Angemeldete Controller bleiben ueber die
Zerstoerung von 'ob' und sogar ueber ein Armageddon hinaus gespeichert.
(Das heisst, das Objekt wird wieder geladen, wenn es Mitternacht ist,
sofern es sich nicht mit delete_controller abgemeldet hatte.)
CONTROL ist in /sys/apps.h definiert.
VERWEISE: add_controller, notify,
          notify_midnight, notify_begin_daylight, notify_begin_darkness
GRUPPEN: Zeit
*/
/*
FUNKTION: notify_begin_daylight
DEKLARATION: void notify_begin_daylight()
BESCHREIBUNG:
Diese Funktion wird in allen mit CONTROL->add_controller(
"notify_begin_daylight", ob) angemeldeten Objekten 'ob' aufgerufen,
wenn die Sonne aufgegangen ist (4:10 VL-Zeit).

ob darf dabei kein Clone sein. Angemeldete Controller bleiben ueber die
Zerstoerung von 'ob' und sogar ueber ein Armageddon hinaus gespeichert.
(Das heisst, das Objekt wird wieder geladen, wenn es Mitternacht ist,
sofern es sich nicht mit delete_controller abgemeldet hatte.)
CONTROL ist in /sys/apps.h definiert.
VERWEISE: add_controller, notify,
          notify_midnight, notify_highnoon, notify_begin_darkness
GRUPPEN: Zeit
*/
/*
FUNKTION: notify_begin_darkness
DEKLARATION: void notify_begin_darkness()
BESCHREIBUNG:
Diese Funktion wird in allen mit CONTROL->add_controller(
"notify_begin_darkness", ob) angemeldeten Objekten 'ob' aufgerufen,
wenn die Sonne untergegangen ist (21:00 VL-Zeit).

ob darf dabei kein Clone sein. Angemeldete Controller bleiben ueber die
Zerstoerung von 'ob' und sogar ueber ein Armageddon hinaus gespeichert.
(Das heisst, das Objekt wird wieder geladen, wenn es Mitternacht ist,
sofern es sich nicht mit delete_controller abgemeldet hatte.)
CONTROL ist in /sys/apps.h definiert.
VERWEISE: add_controller, notify,
          notify_midnight, notify_highnoon, notify_begin_daylight
GRUPPEN: Zeit
*/
/*
FUNKTION: notify_mudcrash
DEKLARATION: void notify_mudcrash(int signal)
BESCHREIBUNG:
Wenn der Driver gerade (z.B. durch einen Speicherfehler) crasht, dann
reisst er sich nochmal kurz zusammen und ruft CONTROL->notify("mudcrash",
signal) auf. signal ist dabei das POSIX-Signal, das dem Driver gesendet
wurde. CONTROL ruft dann in allen angemeldeten Objekten notify_mudcrash auf.
(CONTROL ist in <apps.h> definiert.)

Da der Driver aber gerade dabei ist, zu crashen, sind keine Dateioperationen
mehr erlaubt, das schliesst sowohl die normalen Datei-Efuns (write_file,
save_object), als auch Netzwerkoperationen (z.B. das Ausgeben von
Meldungen an Spieler) ein.
VERWEISE: add_controller, notify
GRUPPEN: grundlegendes
*/
