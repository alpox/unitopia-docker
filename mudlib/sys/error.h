// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/sys/error.h
// Description:	Defines fuer Fehlermeldungen
// Author:	Freaky

#ifndef ERROR_H
#define ERROR_H 1

/*
FUNKTION: do_error
DEKLARATION: void do_error(string err_msg)
BESCHREIBUNG:
Die Fehlermeldung err_msg wird ausgeloest und, falls der Ausloeser nicht
zustaendig ist, in der FDB gespeichert. Die Fehlermeldung sollte ordentlich
umgebrochen werden und ein abschliessendes '\n' enthalten.

Sofern vorhanden, geht diese Meldung an den Ausloeser (previous_object()),
und _nicht_ an das Objekt, in dem die Meldung steht. Fehler, fuer die der
Aufrufer nicht verantwortlich ist, koennen mit do_my_error erzeugt werden.

Spieler erhalten eine Meldung ueber ein Loch im Raum-Zeit-Gefuege.
Die Ausfuehrung wird aber nicht abgebrochen.
VERWEISE: do_my_error, do_error2, do_error3, do_warning
GRUPPEN: master
*/
#define do_error(x) __MASTER_OBJECT__->do_log_error(x,__FILE__, \
	previous_object()?object_name(previous_object()):__FILE__,__LINE__,0)

/*
FUNKTION: do_my_error
DEKLARATION: void do_my_error(string msg)
BESCHREIBUNG:
Identisch zu do_error, nur geht die Meldung nicht an den Ausloeser/Aufrufer,
sondern stets an das eigene Objekt. Sinnvoll fuer alle Fehler, fuer die der
Aufrufer nicht verantwortlich ist.

Spieler erhalten eine Meldung ueber ein Loch im Raum-Zeit-Gefuege.
Die Ausfuehrung wird aber nicht abgebrochen.
VERWEISE: do_error, do_error2, do_error3, do_warning
GRUPPEN: master
*/
#define do_my_error(x) __MASTER_OBJECT__->do_log_error(x, __FILE__, \
                           object_name()||__FILE__,__LINE__,0)

/*
FUNKTION: do_error2
DEKLARATION: void do_error2(string err_msg, string file, string ob, int line)
BESCHREIBUNG:
Die Fehlermeldung err_msg wird ausgeloest und, falls der Ausloeser nicht
zustaendig ist, in der FDB gespeichert. Dabei wird als Datei file, als Zeile
line und als Objekt ob gespeichert. Die Fehlermeldung sollte ordentlich
umgebrochen werden und ein abschliessendes '\n' enthalten.

Spieler erhalten eine Meldung ueber ein Loch im Raum-Zeit-Gefuege.
Die Ausfuehrung wird aber nicht abgebrochen.
VERWEISE: do_error, do_my_error, do_error3, do_warning
GRUPPEN: master
*/
#define do_error2(msg,file,ob,line) \
	__MASTER_OBJECT__->do_log_error(msg,file,ob,line,0)

/*
FUNKTION: do_error3
DEKLARATION: void do_error3(string err_msg, string file, string ob, int line, string *debugger)
BESCHREIBUNG:
Die Fehlermeldung err_msg wird ausgeloest und, falls der Ausloeser nicht
zustaendig ist, in der FDB gespeichert. Dabei wird als Datei file, als Zeile
line und als Objekt ob gespeichert. Zusaetzlich werden dann die in
debugger angegebenen Goetter (klein geschrieben) oder Gruppen als
Debugger eingetragen. Die Fehlermeldung sollte ordentlich
umgebrochen werden und ein abschliessendes '\n' enthalten.

Spieler erhalten eine Meldung ueber ein Loch im Raum-Zeit-Gefuege.
Die Ausfuehrung wird aber nicht abgebrochen.
VERWEISE: do_error, do_my_error, do_error2, do_warning
GRUPPEN: master
*/
#define do_error3(msg,file,ob,line,debugger) \
	__MASTER_OBJECT__->do_log_error(msg,file,ob,line,debugger)

// Das ganze nochmal, nur ohne eine "Loch im Raum-Zeit-Gefuege"-Meldung
// an Spieler

/*
FUNKTION: do_warning
DEKLARATION: void do_warning(string err_msg)
BESCHREIBUNG:
Die Fehlermeldung err_msg wird ausgeloest und, falls der Ausloeser nicht
zustaendig ist, in der FDB gespeichert. Die Fehlermeldung sollte ordentlich
umgebrochen werden und ein abschliessendes '\n' enthalten.

Die Ausfuehrung wird nicht abgebrochen.
Spieler erhalten keine Meldung ueber ein Loch im Raum-Zeit-Gefuege.
VERWEISE: do_my_warning, do_warning2, do_warning3, do_error
GRUPPEN: master
*/
#define do_warning(x) __MASTER_OBJECT__->do_log_error(x,__FILE__, \
        previous_object()?object_name(previous_object()):__FILE__,__LINE__,0,1)

/*
FUNKTION: do_my_warning
DEKLARATION: void do_my_warning(string msg)
BESCHREIBUNG:
Identisch zu do_warning, nur geht die Meldung nicht an den Ausloeser/Aufrufer,
sondern stets an das eigene Objekt. Sinnvoll fuer alle Warnings, fuer die der
Aufrufer nicht verantwortlich ist.

Die Ausfuehrung wird nicht abgebrochen.
Spieler erhalten keine Meldung ueber ein Loch im Raum-Zeit-Gefuege.
VERWEISE: do_warning, do_warning2, do_warning3, do_error
GRUPPEN: master
*/
#define do_my_warning(x) __MASTER_OBJECT__->do_log_error(x, __FILE__, \
                           object_name()||__FILE__,__LINE__,0,1)

/*
FUNKTION: do_warning2
DEKLARATION: void do_warning2(string err_msg, string file, string ob, int line)
BESCHREIBUNG:
Die Fehlermeldung err_msg wird ausgeloest und, falls der Ausloeser nicht
zustaendig ist, in der FDB gespeichert. Dabei wird als Datei file, als Zeile
line und als Objekt ob gespeichert. Die Fehlermeldung sollte ordentlich
umgebrochen werden und ein abschliessendes '\n' enthalten.

Die Ausfuehrung wird nicht abgebrochen.
Spieler erhalten keine Meldung ueber ein Loch im Raum-Zeit-Gefuege.
VERWEISE: do_warning, do_my_warning, do_warning3, do_error
GRUPPEN: master
*/
#define do_warning2(msg,file,ob,line) \
        __MASTER_OBJECT__->do_log_error(msg,file,ob,line,0,1)

/*
FUNKTION: do_warning3
DEKLARATION: void do_warning3(string err_msg, string file, string ob, int line, string *debugger)
BESCHREIBUNG:
Die Fehlermeldung err_msg wird ausgeloest und, falls der Ausloeser nicht
zustaendig ist, in der FDB gespeichert. Dabei wird als Datei file, als Zeile
line und als Objekt ob gespeichert. Zusaetzlich werden dann die in
debugger angegebenen Goetter (klein geschrieben) oder Gruppen als
Debugger eingetragen. Die Fehlermeldung sollte ordentlich
umgebrochen werden und ein abschliessendes '\n' enthalten.

Die Ausfuehrung wird nicht abgebrochen.
Spieler erhalten keine Meldung ueber ein Loch im Raum-Zeit-Gefuege.
VERWEISE: do_warning, do_my_warning, do_warning2, do_error
GRUPPEN: master
*/
#define do_warning3(msg,file,ob,line,debugger) \
        __MASTER_OBJECT__->do_log_error(msg,file,ob,line,debugger,1)

/*
FUNKTION: do_idee
DEKLARATION: void do_idee(string text, mixed wozu, object wer)
BESCHREIBUNG:
Setzt eine Idee zum Objekt oder V-Item wozu ab. Der Ideenlieferant wer
muss dazu noch existieren! Der Text wird noch korrekt umgebrochen.
VERWEISE: do_fehler, do_error, do_warning
GRUPPEN: master
*/
#define do_idee(text, what, wer) \
	__MASTER_OBJECT__->log_fehler_new(text, what, wer, 2)

/*
FUNKTION: do_fehler
DEKLARATION: void do_fehler(string text, mixed wozu, object wer)
BESCHREIBUNG:
Setzt einen Fehler zum Objekt oder V-Item wozu ab. Der Fehlerabsetzende wer
muss dazu noch existieren! Der Text wird noch korrekt umgebrochen.
VERWEISE: do_fehler, do_error, do_warning
GRUPPEN: master
*/
#define do_fehler(text, what, wer) \
	__MASTER_OBJECT__->log_fehler_new(text, what, wer, 1)

#endif // ERROR_H
