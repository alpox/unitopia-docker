// This file is part of UNItopia Mudlib (partly based on Avalon Mudlib).
// ----------------------------------------------------------------
// File:        /i/tools/rollen.c
// Description: Rollenverhalten von Objekten
// Author:      Francis@UNItopia (23.12.93)

#define FUNSYMBOL "$"

#pragma save_types
#pragma strong_types

private functions inherit "/i/tools/coroutine";
inherit "/i/tools/func_parser";

#include <apps.h>
#include <config.h>
#include <error.h>
#include <level.h>
#include <message.h>
#include <regexp.h>
#include <rollen.h>
#include <strings.h>
#include <touch.h>

#define BEFEHL_IF               "if"
#define BEFEHL_PAUSE            "pause"
#define BEFEHL_AUTO_PAUSE       "auto_pause"
#define BEFEHL_STOP             "stop"
#define BEFEHL_WECHSEL_ROLLE    "wechsel_rolle"
#define BEFEHL_GOTO             "goto"
#define BEFEHL_GOSUB            "gosub"
#define BEFEHL_RETURN           "return"
#define BEFEHL_TIME_OUT         "time_out"
#define BEFEHL_STOP_TIME_OUT    "stop_time_out"
#define BEFEHL_WAIT             "wait"
#define BEFEHL_FUN_EXEC         "fun-exec"
#define BEFEHL_EXEC             "exec"
#define BEFEHL_MSG              "msg"
#define BEFEHL_MSGPARAMS        "msgparams"

#define VARSMODE_NONE           0
#define VARSMODE_LOCAL          1
#define VARSMODE_GLOBAL         2

struct statement
{
    string befehl;
    mapping debug;        /* Mapping mit den Lambdas der Befehlsparameters,   *
                           * Zusätzlich:                                      *
                           *  - "text": Original-text des Befehls             *
                           *  - "zeile": die Zeilennummer in der Datei        *
                           *  - "datei": die Include-Dateien-Liste (Array mit *
                           *    den Dateien, welche sich includen. Der letzte *
                           *    Eintrag ist die Datei mit dieser Zeile.       */
};

struct statement_if (statement)
{
    closure bedingung;
    int ende_if;              /* Index der letzten Anweisung des if-Blocks.   */
    int ende_else;            /* Index der letzten Anweisung des else-Blocks. */
};

struct statement_pause (statement)
{
    closure anzahl;           /* Sekunden.                                    */
};

struct statement_wechsel_rolle (statement)
{
    string|closure wohin;     /* Neue Rolle.                                  */
    string|closure label;     /* Label der Rolle.                             */
};

struct statement_goto (statement)
{
    string|closure wohin;     /* Sprungziel.                                  */
};

struct statement_timeout (statement_goto)
{
    closure anzahl;           /* Sekunden.                                    */
};

struct statement_wait (statement)
{
    closure worauf;
};

struct statement_fun (statement)
{
    closure was;
};

struct statement_exec (statement)
{
    int wer;                  /* Monster-Index.                               */
    mixed *was;               /* Argumente für exec_command().                */
};

struct statement_msgparams (statement)
{
    closure message_type;
    closure message_action;
};

struct rolle
{
    closure|int abbrechbar;   /* !=0, wenn die Rolle von außen abbrechbar ist.*/
    closure|int auto_pause;   /* Die Auto-Pause-Einstellung zu Beginn der     *
                               * Rolle.                                       */
    int* if_blocks;           /* Enthält die umgebenden if/else-Blöcke beim   *
                               * Einlesen einer Rolle. Falls das Array nicht  *
                               * leer ist, sind wir gerade in solch einem     *
                               * Block:                                       *
                               *  - Eine Zahl > 0 ist die Zeile mit der der   *
                               *    Block begann. text[zahl] ist also ein     *
                               *    if-Befehl.                                *
                               *  - Die Zahl 0 gibt an, daß gerade eine       *
                               *    öffnende geschweifte Klammer eingelesen   *
                               *    wurde.                                    *
                               *  - Die Zahl -1 steht für eine einzelne Zeile *
                               *    ohne geschweifte Klammern.                */
    int file_time;            /* Datum der letzten Aenderung der Rollendatei  */
    mapping global_vars;      /* Globale Variablen, die bereits durch #if's   *
                               * Werte erhalten haben.                        */
    mapping labels;           /* Mapping von Sprungmarke auf Zeilennummer.    */
    string name;              /* Dateiname dieser Rolle.                      */
    mapping* partner_kriterien;
                              /* Kriterien für die Partnerwahl, ein Mapping   *
                               * von Funktion auf erwartetes Ergebnis.        */
    closure|string start_ort; /* Dateinamen des gewünschten Startortes.       */
    struct statement* text;   /* Die einzelnen Anweisungen der Rolle.         */
    string* text_v1;          /* Text für Rollen Version 1.                   */
    int varsmode;             /* Behandlung von unbekannten Symbolen.         */
    int version;              /* Rollen-Version.                              */
};

struct rollen_status
{
    int abbrechbar;           /* !=0, wenn die Rolle von außen abbrechbar ist.*/
    int auto_pause;           /* Aktuelle Wartezeit aufgrund von Auto-Pause.  */
    mapping if_jumps;         /* Enthält zu einer Zeilennummer eine neue      *
                               * Zeilennummer. Dies wird bei der Ausführung   *
                               * von if-Blöcken verwendet, um am Ende des     *
                               * if-Blocks über den else-Block zu springen.   */
    int *ipstack;             /* Enthält die Rücksprungadressen für Return-   *
                               * Anweisungen.                                 */
    object *partner;          /* Partner für diese Ausführung.                */
    int position;             /* Bei der Ausführung die aktuelle Position.    */
    int time_out_seconds;     /* Sekunden bis zum Timeout.                    */
    closure|string time_out_label;
                              /* Sprungziel für den Timeout.                  */
    closure waitfun;          /* Wartebedingung.                              */
    int warte;                /* Anzahl an abzuwartenden Heartbeats.          */
};

private struct compile_context
{
    string header;            /* Preprocessor-Anweisungen für LPC-Ausdrücke.  */
    string partial_line;      /* Multi-Line-Ausdrücke.                        */
};

struct label
{
    string label;
    int position;
};

private nosave int     modus, zustand, macht_mit;
private nosave struct  rolle rolle = (<rolle>);
private nosave struct  rollen_status rollen_status = (<rollen_status>);
private nosave object  souffleur;
private nosave object|string debug_ob;

private nosave object  this_player_in_rolle;
private nosave int     rollen_wechsel ;
private nosave int     standard_msg_type, standard_msg_action;
private nosave string *allowed_dirs;

// Diese Variablen werden direkt vom LPC-Ausdrücken verwendet.
// Das Array selbst darf nicht ersetzt werden.
private nosave mixed  *rollen_parameter=({});
private nosave object *rollen_partner=({});

private mixed rollen_variablen(symbol name)
{
    int num;
    mixed* data;

    if (sscanf(to_string(name), "$%d%~.1s", num) == 1)
    {
        if (num == 0)
            return &this_player_in_rolle;
        data = &rollen_partner;
    }
    else if (sscanf(to_string(name), "_%d%~.1s", num) == 1)
    {
        if (num == 0)
            return 0;
        data = &rollen_parameter;
    }
    else
        return 0;

    if (num < 1 || num > 100)
        return 0;
    if (sizeof(data) < num)
        data += ({0}) * (num - sizeof(data));
    return &(data[num-1]);
}

private void init_array(mixed *src, mixed *dst)
{
    if (sizeof(src) > sizeof(dst))
        dst += ({0}) * (sizeof(src) - sizeof(dst));
    foreach (int i: sizeof(dst))
    {
        if (i < sizeof(src))
            dst[i] = src[i];
        else
            dst[i] = 0;
    }
}

private void init_rollen_parameter(mixed *params)
{
    init_array(params, &rollen_parameter);
}

private void init_rollen_partner(object *partner)
{
    init_array(partner, &rollen_partner);
}

#define DEBUG_ROLLE(x) { if( debug_ob ) debug_rolle(x); }

/*
FUNKTION: set_rollen_debugger
DEKLARATION: void set_rollen_debugger(object|string ob)
BESCHREIBUNG:
Mit set_rollen_debugger kann man das Objekt setzen, das mit debug_rolle
dann angesprochen wird. Nützlich bei der Entwicklung einer Rolle. Es werden
allerlei Fehlerinformationen an ob geschickt.
Auf Wunsch kann man auch einfach einen Filenamen uebergeben. Dadurch werden
die Informationen in eine Datei abgespeichert.
VERWEISE: query_rollen_debugger, debug_rolle
GRUPPEN: Rollen
*/
void set_rollen_debugger(object|string ob) { debug_ob = ob; }

/*
FUNKTION: query_rollen_debugger
DEKLARATION: object|string query_rollen_debugger()
BESCHREIBUNG:
Liefert, wenn vorhanden, das Objekt zurück, das mit set_rollen_debugger
gesetzt wurde.
VERWEISE: set_rollen_debugger, debug_rolle
GRUPPEN: Rollen
*/
object|string query_rollen_debugger() { return debug_ob; }

/*
FUNKTION: debug_rolle
DEKLARATION: void debug_rolle(string str)
BESCHREIBUNG:
Schickt per send_message_to() einen String an das Objekt, das mit
set_rollen_debugger gesetzt wurde, oder tut gar nichts, wenn
query_rollen_debugger() 0 zurückliefert. Der String setzt sich aus
dem Namen des Rollenmonsters und str zusammen.
VERWEISE: set_rollen_debugger, query_rollen_debugger, send_message_to
GRUPPEN: Rollen
*/
void debug_rolle(string str)
{
    string msg;

    msg=(this_object()->query_cap_name()||explode(object_name(),"/")[<1])+": "+str;

    if(objectp(debug_ob))
        debug_ob->send_message_to(debug_ob, MT_DEBUG|MT_INDENT, MA_UNKNOWN, "   " + msg);
    else if(stringp(debug_ob))
        write_file(debug_ob,msg);
}

/*
FUNKTION: set_macht_bei_rollen_mit
DEKLARATION: void set_macht_bei_rollen_mit(int flag)
BESCHREIBUNG:
Sagt einem Objekt, ob es bei Rollen mitmachen soll. Bei Flag 0 macht es nicht
mit, bei allem anderen macht es bei Rollen mit. Defaultmaessig wird NICHT
mitgemacht.
VERWEISE: query_macht_bei_rollen_mit
GRUPPEN: Rollen
*/
void set_macht_bei_rollen_mit(int a) { macht_mit = a ? 1 : 0; }

/*
FUNKTION: query_macht_bei_rollen_mit
DEKLARATION: int query_macht_bei_rollen_mit()
BESCHREIBUNG:
Liefert zurück, ob ein Objekt bei Rollen mitspielen will.
VERWEISE: set_macht_bei_rollen_mit
GRUPPEN: Rollen
*/
int query_macht_bei_rollen_mit() { return macht_mit; }

/*
FUNKTION: query_rolle
DEKLARATION: struct rolle query_rolle()
BESCHREIBUNG:
Liefert Informationen über die Rolle eines rollengesteuerten Objekts.
Vielleicht will ja mal jemand ein Tool schreiben, mit dem die Ausgabe
verständlicher wird :-)

ACHTUNG: Diese Funktion ist nur für Debugzwecke gedacht, da sich diese
Datenstruktur ändern kann.

Eine Beschreibung der Datenstruktur befindet sich am Anfang dieser Datei.
VERWEISE: in_rolle, query_rollen_status
GRUPPEN: Rollen
*/
struct rolle query_rolle() { return rolle; }

/*
FUNKTION: query_rollen_status
DEKLARATION: struct rollen_status query_rollen_status()
BESCHREIBUNG:
Liefert Informationen über die aktuelle Ausführung der Rolle.

ACHTUNG: Diese Funktion ist nur für Debugzwecke gedacht, da sich diese
Datenstruktur ändern kann.

Eine Beschreibung der Datenstruktur befindet sich am Anfang dieser Datei.
VERWEISE: in_rolle, query_rolle
GRUPPEN: Rollen
*/
struct rollen_status query_rollen_status() { return rollen_status; }

int query_modus() { return modus; }
int query_zustand() { return zustand; }

int query_rollen_modus() { return modus; }

/*
FUNKTION: in_rolle
DEKLARATION: string in_rolle()
BESCHREIBUNG:
Liefert den Namen der Rolle, der das Objekt folgt zurück, egal ob das Objekt
passiv oder aktiv teilnimmt zurueck, oder 0, wenn das Objekt an keiner Rolle
teilnimmt.
VERWEISE: query_rolle
GRUPPEN: Rollen
*/
string in_rolle()
{
   string name;

   if( modus == R_SOUFFLEUR )
      return rolle.name;

   if( modus == R_PASSIV )
   {
      if( !souffleur )
      {
         modus = R_AUS;
         zustand = R_STANDBY;
         return 0;
      }
      if( !(name=souffleur->in_rolle()) )
      {
         modus = R_AUS;
         zustand = R_STANDBY;
         return 0;
      }
      return name;
   }
   modus = R_AUS;
   zustand = R_STANDBY;
}

/*
FUNKTION: stop_rolle
DEKLARATION: int stop_rolle(int partner)
BESCHREIBUNG:
Mit stop_rolle kann man eine Rolle anhalten, in der sich ein Objekt befindet,
auch wenn das Objekt nur passiver Teilnehmer ist. Wenn das Objekt an keiner
Rolle teilnimmt, die Rolle schon gestoppt ist oder das Stoppen via
Controller verboten wurde, wird 0 zurückgeliefert, bei Erfolg 1.
(Partner ist 1, wenn der Aufruf vom stop_rolle des Souffleurs kommt.)
VERWEISE: in_rolle, query_rolle, restart_rolle, starte_rolle, breche_rolle_ab,
          notify_rollenrestart, notify_rollenstop
GRUPPEN: Rollen
*/
varargs int stop_rolle(int partner)
{
    if( !in_rolle() )
        return 0;

    if( modus == R_SOUFFLEUR )
    {
        if( zustand == R_STOPPED )
            return 0;
        if(this_object()->forbidden("rollenstop",this_object(),0,in_rolle()))
            return 0;
        foreach(object ob: rollen_status.partner)
            if(ob && ob->forbidden("rollenstop", ob, this_object(), in_rolle()))
                return 0;

        zustand = R_STOPPED;
        foreach(object ob: rollen_status.partner[1..])
            if(ob)
                ob->stop_rolle(1);
        this_object()->notify("rollenstop", this_object(), 0, in_rolle());
        return 1;
    }

    if( modus == R_PASSIV )
    {
        if( zustand == R_STOPPED )
            return 0;
        if(!partner)
        {
            souffleur->stop_rolle();
            return zustand==R_STOPPED;
        }
        zustand = R_STOPPED;
        this_object()->notify("rollenstop",this_object(),souffleur||1,in_rolle());
        return 1;
    }
}

/*
FUNKTION: forbidden_rollenstop
DEKLARATION: int forbidden_rollenstop(object wer, mixed souffleur, string rolle)
BESCHREIBUNG:
Bevor in einem Objekt wer die Rolle rolle unterbrochen wird,
wird wer->forbidden("rollenstop", wer, souffleur, rolle) aufgerufen,
wobei souffleur 0 ist, wenn wer der Souffleur ist, ansonsten das Objekt,
welches die Rolle steuert, ist. (Falls dieses Objekt nicht mehr existiert,
wird 1 geliefert.) Diese Funktion wird in allen Teilnehmern der Rolle
aufgerufen. Liefert einer dieser forbidden-Aufrufe einen Wert != 0, so
kann die Rolle nicht unterbrochen werden.

Die Funktion forbidden ruft dann in allen mit wer->add_controller(
"forbidden_rollenstop", other) angemeldeten Objekten other die Funktion
other->forbidden_rollenstop(wer, souffleur, rolle) auf. Liefert
einer dieser Aufrufe einen Wert != 0, so liefert forbidden diesen zurück.
VERWEISE: breche_rolle_ab, restart_rolle, notify_rollenstop
GRUPPEN: Rollen
*/

/*
FUNKTION: notify_rollenstop
DEKLARATION: void notify_rollenstop(object wer, mixed souffleur, string rolle)
BESCHREIBUNG:
Wenn in einem Objekt wer die Rolle rolle unterbrochen wurde,
dann wird wer->notify("rollenstop", wer, souffleur, rolle) aufgerufen,
wobei souffleur 0 ist, wenn wer der Souffleur ist, ansonsten das Objekt,
welches die Rolle steuert, ist. (Falls dieses Objekt nicht mehr existiert,
wird 1 geliefert.) Diese Funktion wird in allen Teilnehmern der Rolle
aufgerufen.

Die Funktion notify ruft dann in allen mit wer->add_controller(
"notify_rollenstop", other) angemeldeten Objekten other die Funktion
other->notify_rollenstop(wer, souffleur, rolle) auf.
VERWEISE: breche_rolle_ab, forbidden_rollenstop,
          restart_rolle, notify_rollenrestart
GRUPPEN: Rollen
*/

/*
FUNKTION: restart_rolle
DEKLARATION: int restart_rolle(int partner)
BESCHREIBUNG:
Mit restart_rolle kann man eine Rolle fortsetzen, die mit stop_rolle
angehalten wurde. Das klappt auch dann, wenn das Objekt nur passiver
Teilnehmer ist. Wenn das Objekt an keiner Rolle teilnimmt, die Rolle
schon weiterläuft oder diese Aktion mittels Controller verboten wurde,
wird 0 zurückgeliefert, bei Erfolg 1. (Partner ist 1, wenn der Aufruf
vom restart_rolle des Souffleurs kommt.)
VERWEISE: in_rolle, query_rolle, starte_rolle, breche_rolle_ab,
          notify_rollenrestart, notify_rollenstop
GRUPPEN: Rollen
*/
varargs int restart_rolle(int partner)
{
    if( !in_rolle() )
        return 0;

    if( modus == R_SOUFFLEUR )
    {
        if( zustand == R_RUNNING )
            return 0;
        if(this_object()->forbidden("rollenrestart", this_object(), 0, in_rolle()))
            return 0;
        foreach(object ob: rollen_status.partner)
            if(ob && ob->forbidden("rollenrestart", ob, this_object(), in_rolle()))
                 return 0;
        zustand = R_RUNNING;
        foreach(object ob: rollen_status.partner[1..])
            if(ob)
                ob->restart_rolle(1);
        this_object()->notify("rollenrestart",this_object(),0,in_rolle());
        return 1;
    }

    if( modus == R_PASSIV )
    {
        if( zustand == R_RUNNING )
            return 0;
        if(!partner)
        {
            souffleur->restart_rolle();
            return zustand==R_RUNNING;
        }
        zustand = R_RUNNING;
        this_object()->notify("rollenrestart",this_object(),souffleur||1,in_rolle());
        return 1;
    }
}

/*
FUNKTION: forbidden_rollenrestart
DEKLARATION: int forbidden_rollenrestart(object wer, mixed souffleur, string rolle)
BESCHREIBUNG:
Bevor in einem Objekt wer die Rolle rolle nach einer Unterbrechung
fortgesetzt wird, wird wer->forbidden("rollenrestart", wer, souffleur,
rolle) aufgerufen, wobei souffleur 0 ist, wenn wer der Souffleur ist,
ansonsten das Objekt, welches die Rolle steuert, ist. Diese Funktion wird
in allen Teilnehmern der Rolle aufgerufen. Liefert einer dieser Funktionen
einen Wert != 0, so kann die Rolle nicht fortgesetzt werden.

Die Funktion forbidden ruft dann in allen mit wer->add_controller(
"forbidden_rollenrestart", other) angemeldeten Objekten other die Funktion
other->forbidden_rollenrestart(wer, souffleur, rolle) auf. Liefert
einer dieser Aufrufe einen Wert != 0, so liefert forbidden diesen zurück.
VERWEISE: restart_rolle, breche_rolle_ab, notify_rollenrestart
GRUPPEN: Rollen
*/

/*
FUNKTION: notify_rollenrestart
DEKLARATION: void notify_rollenrestart(object wer, mixed souffleur, string rolle)
BESCHREIBUNG:
Wenn in einem Objekt wer die Rolle rolle nach einer Unterbrechung
fortgesetzt wird, dann wird wer->notify("rollenrestart", wer, souffleur,
rolle) aufgerufen, wobei souffleur 0 ist, wenn wer der Souffleur ist,
ansonsten das Objekt, welches die Rolle steuert, ist. Diese Funktion wird
in allen Teilnehmern der Rolle aufgerufen.

Die Funktion notify ruft dann in allen mit wer->add_controller(
"notify_rollenrestart", other) angemeldeten Objekten other die Funktion
other->notify_rollenrestart(wer, souffleur, rolle) auf.
VERWEISE: restart_rolle, forbidden_rollenrestart,
          breche_rolle_ab, notify_rollenstop
GRUPPEN: Rollen
*/

varargs void beende_rolle(int partner)
{
    string name;
    if(!(name=in_rolle()) )
        return;

    if( modus == R_SOUFFLEUR )
    {
        if( zustand == R_STANDBY )
            return;
        zustand = R_STANDBY;
        foreach(object ob: rollen_status.partner[1..])
            if(ob)
                ob->beende_rolle(1);
        // Erst hier, damit beim beende_rolle(1) in_rolle noch die Rolle liefert
        modus = R_AUS;
        if(this_object())
             this_object()->notify("rollenende",this_object(),0,name);
    }

    if( modus == R_PASSIV )
    {
        if( zustand == R_STANDBY )
            return;
        if(!partner)
        {
            souffleur->beende_rolle();
            return;
        }
        zustand = R_AUS;
        modus = R_AUS;
        if(this_object()) // Falls das Monster überhaupt noch lebt...
             this_object()->notify("rollenende",this_object(),souffleur||1,name);
    }
    if ( !rollen_wechsel )
        this_player_in_rolle = 0 ;

    if( this_object() )
        this_object()->rolle_beendet(rolle.name);
}

/*
FUNKTION: notify_rollenende
DEKLARATION: void notify_rollenende(object wer, mixed souffleur, string rolle)
BESCHREIBUNG:
Wenn in einem Objekt wer die Rolle rolle abgebrochen oder beendet wurde,
dann wird wer->notify("rollenende", wer, souffleur, rolle) aufgerufen,
wobei souffleur 0 ist, wenn wer der Souffleur ist, ansonsten das Objekt,
welches die Rolle steuert, ist. (Falls dieses Objekt nicht mehr existiert,
wird 1 geliefert.) Diese Funktion wird in allen Teilnehmern der Rolle
aufgerufen.

Die Funktion notify ruft dann in allen mit wer->add_controller(
"notify_rollenende", other) angemeldeten Objekten other die Funktion
other->notify_rollenende(wer, souffleur, rolle) auf.

Falls man eine neue Rolle starten will, so sollte dies mit einem call_out
geschehen.
VERWEISE: breche_rolle_ab, notify_rollenstart
GRUPPEN: Rollen
*/

/*
FUNKTION: rolle_beendet
DEKLARATION: void rolle_beendet(string name)
BESCHREIBUNG:
Nachdem eine Rolle beendet wurde, wird in allen Partnern die Funktion
rolle_beendet(rollenname) aufgerufen.
VERWEISE: breche_rolle_ab, notify_rollenende
GRUPPEN: Rollen
*/
void rolle_beendet(string name) {}

/*
FUNKTION: debug_rolle2
DEKLARATION: varargs void debug_rolle2(string str,string file, int line, mixed extended)
BESCHREIBUNG:
Ruft debug_rolle mit dem Text str auf.
Wenn file!=0 ist, dann wird der Fehler ausserdem in die FDB geschrieben.
(Falls der Rollen-Debugger nicht zufällig zuständig für das Objekt ist.)
Falls extended angegegen ist, wird es der Fehlermeldung angehängt.
VERWEISE: debug_rolle, set_rollen_debugger, query_rollen_debugger, tell_object
GRUPPEN: Rollen
*/
varargs void debug_rolle2(string str, string file, int line, mixed extended)
{
    debug_rolle(line
        ? ((str[<1]=='\n' ? str[0..<2] : str) + " (Zeile "+line+")\n")
        : str);

    if(file && (!wizp(debug_ob) || !sizeof(
        filter(MASTER_OB->query_debugger(object_name(this_object())),
            function int(string grp) : string wiz = debug_ob->query_real_name()
            {
                return grp == wiz || GROUP_MASTER.is_group_member(wiz, grp);
            }))))
        do_error2(
            extended
                ? sprintf("%=-1.65s\n\nZusatzinfos:\n"+(stringp(extended)?"%=-1.65s\n":"%=-1.65O\n"),str,extended)
                : sprintf("%=-1.65s\n",str),
            file, object_name(), line);
}

//Hier wird eine gegebene Closure aufgerufen.
//Dabei werden eventuell auftretene Fehler per debug_rolle2() ausgegeben.
protected mixed myfuncall(mixed cl, string|struct statement command, varargs mixed *params)
{
    mixed back;
    string err;
    if(err=catch(back=funcall(cl, params...)))
    {
        if(structp(command))
            debug_rolle2(err[1..], rolle.name + " ("+command.debug["datei"][<1]+")", command.debug["zeile"], command);
        else
            debug_rolle2(err[1..], rolle.name, 0, command);
    }
    else
        return back;
}

private int skip_chars(string line, int pos, string chars)
{
    while (pos<sizeof(line) && line[pos] in chars)
        pos++;
    return pos;
}

private int skip_whitespace(string line, int pos)
{
    return skip_chars(line, pos, " \t");
}

private int skip_word(string line, int pos)
{
    return skip_chars(line, pos, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_");
}

private string|closure compile_expr(string line, int line_num, int pos, struct rolle rolle, struct compile_context context, int may_be_literal_name = 0)
{
    string expr;
    closure l, varfunc;
    string err;

    line = trim(line, TRIM_RIGHT, ".!?"); // Satzendezeichen nehmen wir mal raus.
    expr = context.header + "\n" + regreplace(line[pos..], "#([0-9])", "_\\1", RE_GLOBAL);

    switch (rolle.varsmode)
    {
        case VARSMODE_NONE:
            varfunc = function mixed(symbol name)
            {
                if (to_string(name)[0] in "$_")
                    return &(rollen_variablen(name));
                return 0;
            };
            break;

        case VARSMODE_LOCAL:
            varfunc = function mixed(symbol name) : mapping vars = ([])
            {
                if (to_string(name)[0] in "$_")
                    return &(rollen_variablen(name));
                if (this_object()->parser_get_symbol_variable(name))
                    return 0;

                return &(vars[name]);
            };
            break;

        case VARSMODE_GLOBAL:
            varfunc = function mixed(symbol name)
            {
                if (to_string(name)[0] in "$_")
                    return &(rollen_variablen(name));
                if (this_object()->parser_get_symbol_variable(name))
                    return 0;

                return &(rolle.global_vars[name]);
            };
            break;
    }

    err = catch(l = compile_string(0, &expr, (<compile_string_options>
        variables: varfunc,
        use_object_functions: 1,
        use_object_variables: 1,
        use_object_structs: 1,
        compile_expression: 1,
        detect_end: may_be_literal_name ? 0 : 1,
    )); nolog);

    if (may_be_literal_name)
    {
        if (err)
            return line;

        return function string() { return funcall(l) || line; };
    }

    if (err)
    {
        debug_rolle2(err[1..], rolle.name, line_num);
        return 0;
    }

    if (sizeof(line) < sizeof(expr))
    {
        debug_rolle2("Syntax error", rolle.name, line_num);
        return 0;
    }

    pos = sizeof(line) - sizeof(expr);

    return l;
}

private string|closure parse_name_or_expr(string line, int line_num, int pos, struct rolle rolle, struct compile_context context, string allowed_chars = 0)
{
    int word_start = skip_whitespace(line, pos);
    int word_end = allowed_chars ? skip_chars(line, word_start, allowed_chars) : skip_word(line, word_start);
    int next = word_end;

    if (word_start != word_end)
    {
        while (next<sizeof(line) && line[next] in " \t")
            next++;

        if (next >= sizeof(line) || line[next] == ',')
        {
            pos = next;
            return compile_expr(line[word_start..word_end-1], line_num, 0, rolle, context, 1);
        }
    }

    return compile_expr(line, line_num, &pos, rolle, context);
}

#define ERROR(x) do { debug_rolle2((x), rolle.name, line_num); return; } while(0)
private void process_one_rollen_line(string orig_line, int line_num, struct rolle rolle, struct compile_context context)
{
    string line;
    int pos = 0;

    if (context.partial_line)
    {
        // Wenn wir Leerzeichen in der vorherigen und/oder aktuellen Zeile haben,
        // dann alles zu einem Leerzeichen zusammenfassen.
        if (sizeof((context.partial_line[<1..] + orig_line[..0]) & " \t"))
            orig_line = trim(context.partial_line, TRIM_RIGHT) + " " + trim(orig_line, TRIM_LEFT);
        else
            orig_line = context.partial_line + orig_line;
    }
    line = trim(orig_line, TRIM_RIGHT, "\r\n\t ");

    if (!sizeof(line) || !structp(rolle))
        return;

    if (line[<1] == '\\')
    {
        context.partial_line = line[0] == '#' ? orig_line : line[..<2];
        return;
    }
    else
        context.partial_line = 0;

    if (line[0] == '#')
    {
        <string|int>* match = regmatch(line, "^#[ \t]*pragma[ \t]+(no|local|global)vars\\>", RE_MATCH_SUBS);
        if (match)
        {
            switch (match[1])
            {
                case "no":
                    rolle.varsmode = VARSMODE_NONE;
                    break;

                case "local":
                    rolle.varsmode = VARSMODE_LOCAL;
                    break;

                case "global":
                    rolle.varsmode = VARSMODE_GLOBAL;
                    break;
            }
        }
        else if (regmatch(line, "^#[ \t]*(define|if|ifdef|ifndef|else|elif|endif|undef|echo|pragma|line|include)\\>"))
            context.header += orig_line;
        return;
    }

    while(rolle.text) //Schon Rollentext, Version 2?
    {
        mapping debug;
        int word_start, word_end;
        string word, cmd;

        word_start = skip_whitespace(line, pos);
        word_end = skip_word(line, word_start);
        word = lower_case(line[word_start..word_end-1]);

        if(!sizeof(word) && pos >= sizeof(line))
            return;

        // Ist das der einzelne Befehl in einem if-Block?
        if(sizeof(rolle.if_blocks) && rolle.if_blocks[<1]==-1)
            rolle.if_blocks = rolle.if_blocks[0..<2];
        else
            //Ansonsten geklammerte if/else's behandeln:
            while(sizeof(rolle.if_blocks) && rolle.if_blocks[<1])
            {
                struct statement_if ifstmt = rolle.text[rolle.if_blocks[<1]-1];

                //Fall 1: Ende eines else-Blockes
                if(ifstmt.ende_if >= 0)
                {
                    ifstmt.ende_else = sizeof(rolle.text)-1;
                    rolle.if_blocks = rolle.if_blocks[..<2];
                }
                //Fall 2: Ende eines if-Blockes ohne ein else
                else if(word != "else")
                {
                    ifstmt.ende_if = sizeof(rolle.text)-1;
                    rolle.if_blocks = rolle.if_blocks[..<2];
                }
                //Fall 3: Ein else-Block
                else
                {
                    ifstmt.ende_if = sizeof(rolle.text)-1;
                    break;
                }
            }

        debug = ([ "text": line, "datei": rolle.name, "zeile": line_num]);
        pos = skip_whitespace(line, word_end);

        switch(word)
        {
            case "if":
            {
                closure l;
                int orig_pos = pos;

                if(pos >= sizeof(line) || line[pos] != '(')
                    ERROR("Syntax-Fehler: fehlende Klammer auf");
                pos++;

                l = compile_expr(line, line_num, &pos, rolle, context);
                if (!l)
                    return;

                pos = skip_whitespace(line, pos);
                if (pos >= sizeof(line) || line[pos] != ')')
                    ERROR("Fehlende schließende Klammer");
                pos++;

                debug["bedingung"] = line[orig_pos..pos-1];
                rolle.text += ({ (<statement_if> befehl: BEFEHL_IF,
                                                 debug: debug,
                                                 bedingung: l,
                                                 ende_if: -1,
                                                 ende_else: -1
                              ) });
                rolle.if_blocks += ({ sizeof(rolle.text), -1 });
                continue;
            }

            case "else":
                if(!sizeof(rolle.if_blocks))
                   ERROR("Syntax-Fehler: else ohne if");

                rolle.if_blocks += ({-1});
                continue;

            case "pause":
                cmd = BEFEHL_PAUSE;
            case "auto_pause":
            {
                closure l;
                int orig_pos = pos;

                cmd ||= BEFEHL_AUTO_PAUSE;

                if(pos >= sizeof(line) || line[pos] != ':')
                    ERROR("Syntax-Fehler: fehlender Doppelpunkt");
                pos++;

                l = compile_expr(line, line_num, &pos, rolle, context);
                if (!l)
                    return;

                debug["anzahl"] = line[orig_pos..pos-1];

                pos = skip_whitespace(line, pos);
                if (pos < sizeof(line))
                    ERROR("Extra Zeichen in der Zeile.");

                rolle.text += ({ (<statement_pause> befehl: cmd,
                                                    debug: debug,
                                                    anzahl: l
                              ) });
                break;
            }

            case "wait":
            {
                closure l;
                int orig_pos = pos;

                l = compile_expr(line, line_num, &pos, rolle, context);
                if (!l)
                    return;

                debug["worauf"] = line[orig_pos..pos-1];

                pos = skip_whitespace(line, pos);
                if (pos < sizeof(line))
                    ERROR("Extra Zeichen in der Zeile.");

                rolle.text += ({ (<statement_wait> befehl: BEFEHL_WAIT,
                                                   debug: debug,
                                                   worauf: l
                              ) });
                break;
            }

            case "wechsel_rolle":
            {
                string|closure wohin, label;
                int orig_pos = pos;

                if(pos >= sizeof(line) || line[pos] != ':')
                    ERROR("Syntax-Fehler: fehlender Doppelpunkt");
                pos++;

                wohin = parse_name_or_expr(line, line_num, &pos, rolle, context, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_/-.");
                if (!wohin)
                    return;

                debug["wohin"] = line[orig_pos..pos-1];

                pos = skip_whitespace(line, pos);

                if (pos < sizeof(line) && line[pos] == ',')
                {
                    pos++;
                    orig_pos = pos;

                    label = parse_name_or_expr(line, line_num, &pos, rolle, context);
                    if (!label)
                        return;

                    debug["label"] = line[orig_pos..pos-1];
                }

                pos = skip_whitespace(line, pos);
                if (pos < sizeof(line))
                    ERROR("Extra Zeichen in der Zeile.");

                rolle.text += ({ (<statement_wechsel_rolle> befehl: BEFEHL_WECHSEL_ROLLE,
                                                            debug: debug,
                                                            wohin: wohin,
                                                            label: label
                              ) });
                break;
            }

            case "stop":
                cmd = BEFEHL_STOP;
            case "return":
                cmd ||= BEFEHL_RETURN;
            case "stop_time_out":
                cmd ||= BEFEHL_STOP_TIME_OUT;

                if (pos < sizeof(line))
                    ERROR("Extra Zeichen in der Zeile.");

                rolle.text += ({ (<statement> befehl: cmd, debug: debug) });
                break;

            case "goto":
                cmd = BEFEHL_GOTO;
            case "gosub":
            {
                string|closure label;
                int orig_pos = pos;

                cmd ||= BEFEHL_GOSUB;

                label = parse_name_or_expr(line, line_num, &pos, rolle, context);
                if (!label)
                    return;

                debug["wohin"] = line[orig_pos..pos-1];

                pos = skip_whitespace(line, pos);
                if (pos < sizeof(line))
                    ERROR("Extra Zeichen in der Zeile.");

                rolle.text += ({ (<statement_goto> befehl: cmd,
                                                   debug: debug,
                                                   wohin: label
                              ) });
                break;
            }

            case "time_out":
            {
                closure anzahl;
                string|closure wohin;
                int orig_pos = pos;

                anzahl = compile_expr(line, line_num, &pos, rolle, context);
                if (!anzahl)
                    return;

                debug["worauf"] = line[orig_pos..pos-1];

                pos = skip_whitespace(line, pos);
                if (pos >= sizeof(line) || line[pos] != ',')
                    ERROR("Zweiter Parameter fehlt bei Time_out.");

                pos++;
                orig_pos = pos;

                wohin = parse_name_or_expr(line, line_num, &pos, rolle, context);
                if (!wohin)
                    return;

                debug["wohin"] = line[orig_pos..pos-1];

                pos = skip_whitespace(line, pos);
                if (pos < sizeof(line))
                    ERROR("Extra Zeichen in der Zeile.");

                rolle.text += ({ (<statement_timeout> befehl: BEFEHL_TIME_OUT,
                                                      debug: debug,
                                                      wohin: wohin,
                                                      anzahl: anzahl
                              ) });
                break;
            }

            case "message_type":
            {
                closure message_type, message_action;
                int orig_pos = pos;

                if(pos < sizeof(line) && line[pos] == ':')
                    pos++;

                message_type = compile_expr(line, line_num, &pos, rolle, context);
                if (!message_type)
                    return;

                debug["message_type"] = line[orig_pos..pos-1];

                pos = skip_whitespace(line, pos);
                if (pos >= sizeof(line) || line[pos] != ',')
                    ERROR("Aktionstyp fehlt bei Message_Type.");

                pos++;
                orig_pos = pos;

                message_action = compile_expr(line, line_num, &pos, rolle, context);
                if (!message_action)
                    return;

                debug["message_action"] = line[orig_pos..pos-1];

                pos = skip_whitespace(line, pos);
                if (pos < sizeof(line))
                    ERROR("Extra Zeichen in der Zeile.");

                rolle.text += ({ (<statement_msgparams> befehl: BEFEHL_MSGPARAMS,
                                                        debug: debug,
                                                        message_type: message_type,
                                                        message_action: message_action
                              ) });
                break;
            }

            case "":
                if(pos >= sizeof(line))
                    break;

                if(line[pos] == '{')
                {
                    rolle.if_blocks += ({0});

                    pos++;
                    continue;
                }
                else if(line[pos]=='}')
                {
                    if(!sizeof(rolle.if_blocks) || rolle.if_blocks[<1])
                        ERROR("Syntax-Fehler: } ohne {");
                    else
                        rolle.if_blocks = rolle.if_blocks[..<2];

                    pos++;
                    continue;
                }
                else if(line[pos]=='[')
                {
                    int start_pos = ++pos;
                    string label;

                    while (pos < sizeof(line) && line[pos] != ']')
                        pos++;

                    if(pos >= sizeof(line))
                        ERROR("Syntax-Fehler: [ ohne ]");

                    label = line[start_pos..pos-1];
                    if(member(rolle.labels, label))
                        ERROR("Label "+label+" mehrfach vorhanden");

                    rolle.labels[label] = sizeof(rolle.text);

                    pos++;
                    continue;
                }

            default:
            {
                int partner;

                pos = word_start;

                //Hier kommt nun entweder ein Funktionsaufruf oder exec_command
                if(sscanf(line[pos..], "%d: %s", partner, cmd) != 2) //fun-exec
                {
                    int orig_pos = pos;
                    closure fun = compile_expr(line, line_num, &pos, rolle, context);

                    if (!fun)
                        return;

                    debug["was"] = line[orig_pos..pos-1];

                    pos = skip_whitespace(line, pos);
                    if (pos < sizeof(line) && line[pos] == ';')
                        pos = skip_whitespace(line, pos+1);
                    if (pos < sizeof(line))
                        ERROR("Extra Zeichen in der Zeile.");

                    rolle.text += ({ (<statement_fun> befehl: BEFEHL_FUN_EXEC,
                                                      debug: debug,
                                                      was: fun
                                  ) });
                }
                else //exec_command
                {
                    string str = "";
                    <closure|string>* back = ({});
                    string* deb = ({});
                    int orig_pos;

                    if(sizeof(rolle.partner_kriterien) < partner)
                        ERROR("Rollenpartner " + partner + " gibt's nicht");

                    pos = 0;

                    while(pos >= 0)
                    {
                        closure fun;
                        int funstart = strstr(cmd, FUNSYMBOL, pos);

                        if(funstart < 0)
                        {
                            str += cmd[pos..];
                            break;
                        }
                        else if(funstart > pos)
                        {
                            if(cmd[funstart-1] == '\\')
                            {
                                str += cmd[pos..funstart-2] + FUNSYMBOL;
                                pos = funstart+1;
                                continue;
                            }
                            else
                                str += cmd[pos..funstart-1];
                        }

                        orig_pos = funstart;
                        pos = funstart+1;
                        fun = compile_expr(cmd, line_num, &pos, rolle, context);
                        if (!fun)
                            return;

                        if(sizeof(str))
                        {
                            back += ({str});
                            deb += ({str});
                            str = "";
                        }

                        back += ({fun});
                        deb += ({cmd[orig_pos..pos-1]});

                        if(pos < sizeof(cmd) && cmd[pos..pos] == FUNSYMBOL)
                            pos++;
                    }

                    if(sizeof(str))
                    {
                        back += ({str});
                        deb += ({str});
                    }

                    debug["was"] = deb;
                    rolle.text += ({ (<statement_exec> befehl: member(rolle.partner_kriterien[partner-1],"Objekt") ? BEFEHL_MSG : BEFEHL_EXEC,
                                                       debug: debug,
                                                       wer: partner,
                                                       was: back
                                  ) });
                }
                break;
            }
        }
        return;
    }

    if(rolle.text_v1) // Version 1
    {
        foreach (string part: regexplode(line,"{|}") - ({""}))
        {
            string label, rest;

            if(sscanf(trim(part),"[%s]%s", label, rest))
            {
                if(member(rolle.labels, label))
                    ERROR("Label '"+label+"' mehrfach vorhanden");
                rolle.labels[label] = sizeof(rolle.text_v1);
                part = rest;
            }

            part = trim(part);
            if(sizeof(part))
                rolle.text_v1 += ({part});
        }
    }
    else //Header
    {
        int word_start, word_end;
        string word;
        string|closure value;

        word_start = skip_whitespace(line, pos);
        word_end = skip_word(line, word_start);
        word = line[word_start..word_end-1];

        if(word == "")
            return 0;
        if(word_end >= sizeof(line))
            ERROR("Unerwartetes Ende");
        if(line[word_end] != ':')
            ERROR("Syntax-Fehler: fehlender Doppelpunkt");

        pos = skip_whitespace(line, word_end+1);
        if(line[pos..pos]==FUNSYMBOL)
        {
            pos++;
            value = compile_expr(line, line_num, &pos, rolle, context);
            if (!value)
                return;

            if (pos < sizeof(line))
                ERROR("Extra Zeichen in der Zeile.");
        }
        else
            value = trim(line[pos..]);

        switch(lower_case(word))
        {
            case "start_ort":
                rolle.start_ort = value;
                break;

            case "rolle_abbrechbar":
                rolle.abbrechbar = stringp(value) ? (lower_case(value) == "ja") : value;
                break;

            case "auto_pause":
                rolle.auto_pause = stringp(value) ? to_int(value) : value;
                break;

            case "version":
                if (closurep(value))
                    ERROR("LPC-Ausdruck ist bei Version nicht zulässig.");
                rolle.version = to_int(value);
                break;

            case "anzahl_teilnehmer":
            {
                int num;

                if (closurep(value))
                    ERROR("LPC-Ausdruck ist bei Anzahl_Teilnehmer nicht zulässig.");

                num = to_int(value)||1;
                if(!rolle.partner_kriterien)
                    rolle.partner_kriterien = map(({ ([]) }) * num, #'copy);
                else if((num = num - sizeof(rolle.partner_kriterien)) > 0)
                    rolle.partner_kriterien += map(({ ([]) }) * num, #'copy);
                else if(num < 0)
                    rolle.partner_kriterien = rolle.partner_kriterien[..<(1-num)];
                break;
            }

            case "rollen_debugger":
                if( query_rollen_debugger() )
                {
                    // Avatar: sorgt dafuer dass ein Wiz auch ohne Schreibrecht auf das
                    //         Rollenscript den Rollen-Debugger setzen kann.
                    DEBUG_ROLLE("rollen_debugger: wurde bereits festgelegt.\n");
                }
                else
                {
                    object ob;

                    if (closurep(value))
                        value = funcall(value);

                    set_rollen_debugger(
                        (!stringp(value) || !sizeof(value) || value == "0")
                            ? 0
                            : ( (ob = find_player(value))
                                ? ob
                                : ( value[0]=='/'
                                    ? abs_path(value, implode(explode(rolle.name,"/")[0..<2],"/"))
                                    : 0) ) );
                }
                break;

            case "text":
                if (rolle.version == 2)
                    rolle.text = ({});
                else
                    rolle.text_v1 = ({});
                rolle.labels = ([]);
                rolle.if_blocks = ({});
                break;

            default:
                if(lower_case(word)[..10]=="teilnehmer_")
                {
                    string func;
                    int idx;

                    if(sscanf(word[11..],"%d_%s", idx, func) != 2)
                        ERROR("Syntax Fehler");

                    if(!rolle.partner_kriterien)
                        rolle.partner_kriterien = map(({ ([]) })*idx, #'copy);
                    else if(idx > sizeof(rolle.partner_kriterien))
                        rolle.partner_kriterien += map(({ ([]) })*(idx-sizeof(rolle.partner_kriterien)), #'copy);

                    if (closurep(value) && func != "Objekt" && func != "Lebewesen")
                        value = funcall(value);

                    rolle.partner_kriterien[idx-1][func] = value;
                }
                else
                    ERROR("Unbekannter Parameter '"+word+"'");
                break;
        }
        if(!rolle.partner_kriterien)
            rolle.partner_kriterien = ({ ([]) });
    }
}

private struct rolle lese_rolle(string name, struct rolle cached)
{
    if( !cached ||
        cached.name != name || cached.file_time != file_time(name) )
    {
        struct compile_context context = (<compile_context>
            header: "#include \"/sys/message.h\"\n#include \"/sys/deklin.h\"\n");
        int last_line;

        if( !name || name == "" )
        {
            debug_rolle2("lese_rolle: Kein Filenamen übergeben.\n",__FILE__,__LINE__);
            return 0;
        }

        DEBUG_ROLLE("lese_rolle: "+shorttimestr(time())+"\n");
        rolle = (<rolle> name: name,
                         global_vars: ([])
                );

        foreach (struct enumerated arg: enumerate(read_lines(name), 1))
        {
            process_one_rollen_line(arg.value, arg.number, rolle, context);
            last_line = arg.number;
        }

        //Sind noch offene if's da?
        while(sizeof(rolle.if_blocks))
        {
            //Fall 1: Ein erwarteter Befehl nach einem if/else
            if(rolle.if_blocks[<1]==-1)
            {
                debug_rolle2("lese_rolle: Syntax-Fehler: "
                    "Fehlender Befehl nach einem if/else.\n", name, last_line, rolle.if_blocks);
                break;
            }
            //Fall 2: Offene geschweifte Klammern?
            else if(!rolle.if_blocks[<1])
            {
                debug_rolle2("lese_rolle: Syntax-Fehler: Fehlende }.\n", name, last_line, rolle.if_blocks);
                break;
            }
            else
            {
                struct statement_if ifstmt = rolle.text[rolle.if_blocks[<1]-1];
                //Fall 3: Ende eines else-Blockes
                if(ifstmt.ende_if >= 0)
                    ifstmt.ende_else = sizeof(rolle.text)-1;
                //Fall 4: Ende eines if-Blockes ohne ein else
                else
                    ifstmt.ende_if = sizeof(rolle.text)-1;
            }
            rolle.if_blocks = rolle.if_blocks[..<2];
        }

        rolle.file_time = file_time(name);
        rolle.if_blocks = 0;

        if( !sizeof(rolle.text) && !sizeof(rolle.text_v1))
        {
            debug_rolle2("lese_rolle: Im File "+name+" fehlt der Rollentext.\n",name);
            rolle = 0;
            return 0;
        }
    }
    else
    {
        rolle=cached;
        DEBUG_ROLLE(wrap("lese_rolle: Die Rolle "+name+" befand sich noch im "+
            "Speicher: "+shorttimestr(time())));

        // Globale Variablen zurücksetzen
        foreach (symbol varname, mixed value: &(rolle.global_vars))
            value = 0;
   }

   return rolle;
}

private string lies_von_rolle() // Rollen V1
{
    string command, ret;

    if( rollen_status.position >= sizeof(rolle.text_v1) )
        return 0;

    ret="";
    do
    {
        command = rolle.text_v1[rollen_status.position++];
        DEBUG_ROLLE("|" + command + "|\n");
        ret += command[<1] == '\\' ? command[0..<2] : command;
    }
    while( command[<1] == '\\' );

    return ret;
}

private string search_next_br() // Rollen V1
{
    string command;
    int ebene;

    ebene=1;

    do
    {
        if( !(command=lies_von_rolle()) )
            return 0;

        if( command[0] == '{' )
            ++ebene;
        else
            if( command[0] == '}' )
                --ebene;
    }
    while( ebene > 0 );

    return lies_von_rolle();
}

private int check_for_pause(string command) // Rollen V1
{
    int tmp;

    // 'command' ist lower_case
    if( sscanf(command, "auto_pause: %d", tmp) )
    {
        rollen_status.auto_pause = tmp;
        return -1;
    }
    else
        if( sscanf(command, "pause: %d", tmp) == 1 )
            return tmp||-1;
}

mixed do_fun_call(mixed ob, string funktion, varargs mixed *parameter)
{
    int wer;
    object who;

    if( !in_rolle() )
        return ::do_fun_call(ob,funktion,parameter);

    if( !(who=touch(ob,NO_LOG|NO_WRITE) ) &&
        (!sscanf(ob,"%d",wer) ||
         wer > sizeof(rollen_status.partner) ||
         wer < 0 ||
         wer >= 1 && !(who=rollen_status.partner[wer-1]) ||
         wer == 0 && !(who=this_player_in_rolle)
        )
     )
   {
      DEBUG_ROLLE(sprintf("do_fun_call: '%s' gibt es nicht (mehr).\n",ob));
      return 0;
   }

   return ::do_fun_call(who,funktion,parameter);
}

private mixed _do_fun_call(string funktion, string parameter)
{
    string obj, fun;

    if( (funktion=trim(funktion))[0] == '$' )
    {
        if( sscanf(funktion,"$%s->%s",obj,fun) != 2 )
        {
            DEBUG_ROLLE(sprintf("_do_fun_call: Falsches Aufrufeformat: '%s->"
                "%s'\n",funktion,parameter));
            return 0;
        }
    }
    else
    {
        obj=object_name();
        fun=funktion;
    }

    DEBUG_ROLLE(sprintf("_do_fun_call: Rufe '%s->%s(%s)' auf.\n",
        obj=trim(obj),fun=trim(fun),parameter=trim(parameter)));

    return do_fun_call(obj,fun,parameter);
}

mixed do_efun_call(string funktion, varargs mixed *parameter)
{
    int wer;
    string ob;

    if (sizeof(parameter) == 1)
        ob = parameter[0];

    if( !in_rolle() || !ob)
        return ::do_efun_call(funktion,sizeof(parameter) ? parameter : 0);

    if( !sscanf(ob,"%d",wer) || wer > sizeof(rollen_status.partner) || wer < 0 )
        return 0;

    if ( wer == 0 )
        return ( this_player_in_rolle ) ?
            ::do_efun_call(funktion,this_player_in_rolle) : 0;
    else
        return ::do_efun_call(funktion,rollen_status.partner[wer-1]);
}

private struct label get_label(string str)
{
    string fun, arg;

    if( sscanf(str=trim(str),"%s(%s(%s)%s)",str,fun,arg,str) == 4 )
    {
        if( !sizeof(str=_do_fun_call(fun,arg)) )
        {
            DEBUG_ROLLE(sprintf("Goto (%s(%s)) lieferte keinen "
                "String als Ergebnis.\n",fun,arg));
            return 0;
        }
    }

    if( !member(rolle.labels, str))
    {
        DEBUG_ROLLE(sprintf("Das Label '%s' gibt es nicht.\n",str));
        return 0;
    }

    return (<label> str, rolle.labels[str]);
}

/*
FUNKTION: set_standard_msg_params
DEKLARATION: void set_standard_msg_params(int msg_type, int msg_action)
BESCHREIBUNG:
Setzt die Standardmeldungstypen der Rollen-Meldungen bei Nicht-Lebewesen
fuer send_message.
VERWEISE: query_standard_msg_params, starte_rolle
GRUPPEN: Rollen
*/
void set_standard_msg_params(int msg_type, int msg_action)
{
    standard_msg_type=msg_type;
    standard_msg_action=msg_action;
}

/*
FUNKTION: query_standard_msg_params
DEKLARATION: int *query_standard_msg_params()
BESCHREIBUNG:
Liefert die Standardmeldungstypen in einem Array ({msg_type, msg_action}).
VERWEISE: set_standard_msg_params, starte_rolle
GRUPPEN: Rollen
*/
int *query_standard_msg_params()
{
    return ({standard_msg_type,standard_msg_action});
}

/*
FUNKTION: prepare_exec
DEKLARATION: mixed *prepare_exec(mixed *pars,mixed command)
BESCHREIBUNG:
Aus dem uebergebenen Array pars mit Strings und Closures wird
eine Parameterliste fuer exec_command gemacht, d.h. fuer jede Closure
wird myfuncall aufgerufen und das Ergebnis wird im neuen Array gespeichert.
Strings werden direkt uebernommen. Mehrere Strings hintereinander werden
zu einem Zusammengefasst. Folgt auf einen String ein Array oder Objekt
im Array, so wird dem String ein Leerzeichen am Ende weggenommen. Genauso
wird einem dem Array/Objekt folgendem String ein Leerzeichen am Anfang
entfernt. Der zweite Parameter command wird fuer Debugzwecke genutzt und
direkt an myfuncall weitergegeben.
VERWEISE: exec_command
GRUPPEN: Rollen
*/
mixed *prepare_exec(<string|closure>* pars, struct statement command) // Rollen V2
{
    string str = "";
    mixed *back = ({});

    foreach(mixed par: map(pars, #'myfuncall, command))
    {
        if(stringp(par))
            str += par;
        else
        {
            if(sizeof(str))
            {
                if(str[<1]==' ')
                    str=str[0..<2];
                if(sizeof(back) && sizeof(str) && (str[0]==' '))
                    str=str[1..<1];
                if(sizeof(str))
                {
                    back+=({str});
                    str="";
                }
            }
            back+=({par});
        }
    }

    if(sizeof(back) && sizeof(str) && (str[0]==' '))
        str=str[1..<1];
    if(sizeof(str))
        back+=({str});
    return back;
}

private int drehe_an_rolle_v2()
{
    mixed tmp;

    //Hier werden die die Sprungziele nach ende eines if-Blockes abgelegt, wenn
    //ein else-Block existiert.
    if(!rollen_status.if_jumps)
        rollen_status.if_jumps = ([]);

    do
    {
        struct statement command;

        if(rollen_status.position < 0)
        {
            do_error(sprintf("%=-1.65s\n\Rollendaten:\n%=-1.65O\n",
                "Fehler: Position ist kleiner als 0.",rolle));
            return 0;
        }

        command = rolle.text[rollen_status.position];
        DEBUG_ROLLE("| Ein "+command.befehl+"-Befehl in Zeile "+command.debug["zeile"]+": |\n");
        DEBUG_ROLLE(command.debug["text"]+"\n");

        //Ist hier zufaellig ein if/else-Block zu Ende?
        while(member(rollen_status.if_jumps, rollen_status.position))
          rollen_status.position = rollen_status.if_jumps[rollen_status.position];

        rollen_status.position++;
        switch(command.befehl)
        {
            case BEFEHL_IF:
            {
                struct statement_if ifstmt = command;
                if(myfuncall(ifstmt.bedingung, command))
                {
                    DEBUG_ROLLE("-> Ergebnis: Wahr\n");
                    if(ifstmt.ende_else >= 0)
                        rollen_status.if_jumps[ifstmt.ende_if] = ifstmt.ende_else;
                }
                else
                {
                    DEBUG_ROLLE("-> Ergebnis: Falsch\n");
                    rollen_status.position = ifstmt.ende_if + 1;
                }
                break;
            }

            case BEFEHL_PAUSE:
            {
                struct statement_pause stmt = command;
                tmp=myfuncall(stmt.anzahl, command);
                DEBUG_ROLLE("-> Ergebnis: "+tmp+"\n");
                if(tmp > 0)
                    return tmp;
                break;
            }

            case BEFEHL_AUTO_PAUSE:
            {
                struct statement_pause stmt = command;
                rollen_status.auto_pause = myfuncall(stmt.anzahl, command);
                DEBUG_ROLLE("-> Ergebnis: "+rollen_status.auto_pause+"\n");
                break;
            }

            case BEFEHL_STOP:
                return 0;

            case BEFEHL_WECHSEL_ROLLE:
            {
                struct statement_wechsel_rolle stmt = command;
                rollen_wechsel = 1;
                tmp = myfuncall(stmt.wohin, command);
                beende_rolle();
                if(stringp(tmp))
                    this_object()->starte_rolle(
                        (tmp[0]=='/') ? tmp : abs_path("../"+tmp, rolle.name),
                        myfuncall(stmt.label, command),
                        rollen_parameter...);
                else
                    debug_rolle2("Rollenname bei Wechsel_Rolle ist kein String.\n",
                        rolle.name+" ("+command.debug["datei"][<1]+")",command.debug["zeile"], command);
                rollen_wechsel = 0 ;
                return -2;
            }

            case BEFEHL_GOTO:
            {
                struct statement_goto stmt = command;
                tmp=myfuncall(stmt.wohin, command);
                if(!member(rolle.labels, tmp))
                    return -1;
                rollen_status.position = rolle.labels[tmp];
                DEBUG_ROLLE("-> Springe nach ["+tmp+"].\n");
                break;
            }

            case BEFEHL_GOSUB:
            {
                struct statement_goto stmt = command;
                tmp=myfuncall(stmt.wohin, command);
                if(!member(rolle.labels, tmp))
                    return -1;
                rollen_status.ipstack += ({ rollen_status.position });
                rollen_status.position = rolle.labels[tmp];
                DEBUG_ROLLE("-> Springe nach ["+tmp+"].\n");
                break;
            }

            case BEFEHL_RETURN:
                if( !sizeof(rollen_status.ipstack) )
                {
                    DEBUG_ROLLE("Die Rolle wird beendet.\n");
                    return 0;
                }
                DEBUG_ROLLE("-> Springe zurück...\n");
                rollen_status.position = rollen_status.ipstack[<1];
                rollen_status.ipstack = rollen_status.ipstack[..<2];
                break;

            case BEFEHL_TIME_OUT:
            {
                struct statement_timeout stmt = command;
                rollen_status.time_out_seconds = myfuncall(stmt.anzahl, command);
                rollen_status.time_out_label = stmt.wohin;
                DEBUG_ROLLE("-> Ergebnis: "+rollen_status.time_out_seconds+"\n");
                break;
            }

            case BEFEHL_STOP_TIME_OUT:
                DEBUG_ROLLE("-> Time-Out abgebrochen.\n");
                rollen_status.time_out_seconds = 0;
                rollen_status.time_out_label = 0;
                break;

            case BEFEHL_WAIT:
            {
                struct statement_wait stmt = command;
                rollen_status.waitfun = stmt.worauf;
                return -2;
            }

            case BEFEHL_FUN_EXEC:
            {
                struct statement_fun stmt = command;
                myfuncall(stmt.was, command);
                break;
            }

            case BEFEHL_EXEC:
            {
                struct statement_exec stmt = command;

                if( !rollen_status.partner[stmt.wer-1] )
                {
                    DEBUG_ROLLE("Der Partner mit der Nummer "+(stmt.wer)+" ist tot.\n");
                    return -1;
                }

                rollen_status.partner[stmt.wer-1]->exec_command(prepare_exec(stmt.was, command)...);

                if( rollen_status.auto_pause )
                    return rollen_status.auto_pause;
                break;
            }

            case BEFEHL_MSG:
            {
                struct statement_exec stmt = command;

                if( !rollen_status.partner[stmt.wer-1] )
                {
                    DEBUG_ROLLE("Der Partner mit der Nummer "+(stmt.wer)+" (ein Raum) ist nicht mehr da.\n");
                    return -1;
                }

                tmp = prepare_exec(stmt.was, command);
                if(!sizeof(tmp))
                {
                    debug_rolle2("Keine Meldung angegeben.\n",
                        rolle.name + " (" + command.debug["datei"][<1] + ")", command.debug["zeile"], command);
                    return -1;
                }
                else if(sizeof(tmp) > 1 || !stringp(tmp[0]))
                {
                    debug_rolle2("Die LPC-Ausdrücke in der Meldung liefern keinen String.\n",
                        rolle.name + " (" + command.debug["datei"][<1] + ")", command.debug["zeile"], command);
                    return -1;
                }

                rollen_status.partner[stmt.wer-1]->send_message(standard_msg_type, standard_msg_action, tmp[0]);
                if( rollen_status.auto_pause )
                    return rollen_status.auto_pause;
                break;
            }

            case BEFEHL_MSGPARAMS:
            {
                struct statement_msgparams stmt = command;

                set_standard_msg_params(
                    myfuncall(stmt.message_type, command),
                    myfuncall(stmt.message_action, command));
                break;
            }
        }
    } while(rollen_status.position < sizeof(rolle.text));
}

void rolle_v2()
{
    int pause;
    mixed res;

    if( rollen_status.position >= sizeof(rolle.text) )
    {
        DEBUG_ROLLE("rolle: Ich bin am Ende der Rolle.\n");
        beende_rolle();
        return;
    }

    if( rollen_status.time_out_seconds && !--rollen_status.time_out_seconds )
    {
        if( !res = myfuncall(rollen_status.time_out_label, "Wurde ausgeführt durch Time_Out.\n") )
        {
            beende_rolle();
            return;
        }

        rollen_status.time_out_label = 0;
        rollen_status.warte = 0;
        rollen_status.waitfun = 0;

        if(!member(rolle.labels, res))
        {
            beende_rolle();
            return;
        }

        DEBUG_ROLLE("rolle: Time-Out-Sprung nach ["+res+"].\n");
        rollen_status.position = rolle.labels[res];
    }

    if( (res=rollen_status.waitfun) && !myfuncall(res, "Wurde ausgeführt durch Wait") )
    {
        DEBUG_ROLLE("rolle: Ich warte...\n");
        return;
    }
    else
        rollen_status.waitfun = 0;

    if( rollen_status.warte > 0 )
    {
        DEBUG_ROLLE("rolle: Ich warte noch "+rollen_status.warte+" mal.\n");
        rollen_status.warte--;
        return;
    }

    if( (pause=drehe_an_rolle_v2()) <= 0 )
    {
        if( !pause )
        {
            DEBUG_ROLLE("rolle: Ende der Rolle erreicht.\n");
            beende_rolle();
        }
        if( pause == -1 )
            beende_rolle();
    }
    else
    {
        DEBUG_ROLLE("rolle: Ich beginne eine Pause ("+pause+").\n");
        rollen_status.warte = pause-1;
    }
}

private int drehe_an_rolle() // Rollen V1
{
    string  command, lcommand, new_command, str, if_fun, fun, arg;
    string  name;
    int     wer, pause, a;
    mixed   res;

    do
    {
        if( !(command=lies_von_rolle()) )
            return 0;

        /* If-Command */
        if( lower_case(command[0..1]) == "if" )
        {
            if( sscanf(command,"%s(%s(%s)%s)",str,if_fun,arg,str) != 4 )
            {
                DEBUG_ROLLE("Die Syntax der If-Abfrage ist falsch: "
                    +(str||"?")+"( "+(if_fun||"?")+" ( "+(arg||"?")+" ) ).\n");
                return -1;
            }

            res=_do_fun_call(if_fun,arg)?1:0;
            DEBUG_ROLLE(sprintf("Ergebnis: %d.\n",res));

            if( !command=lies_von_rolle() )
                return 0;

            if( command[0] != '{' )
            {
                DEBUG_ROLLE("Unter der 'if'-Anweisung fehlt "
                            "eine '{'-Klammer.\n");
                return -1;
            }

            if( !res )
            {
                command=search_next_br();

                /* Else-Command (before) */
                if( lower_case(command[0..3]) == "else" )
                {
                    if( !(command=lies_von_rolle()) )
                        return 0;

                    if( command[0] != '{' )
                    {
                        DEBUG_ROLLE("Unter der 'else'-Anweisung fehlt "
                                    "eine '{'-Klammer.\n");
                        return -1;
                    }
                }
                else
                    --rollen_status.position;
            }

            continue;
        }

        while( command[0] == '}' )
            if( !(command=lies_von_rolle()) )
                return 0;

        /* Else-Command (after) */
        while( lower_case(command[0..3]) == "else" )
        {
            if( !command=lies_von_rolle() )
                return 0;

            if( command[0] != '{' )
            {
                DEBUG_ROLLE("Unter der 'else'-Anweisung fehlt "
                            "eine '{'-Klammer.\n");
                return -1;
            }

            if( !command=search_next_br() )
                return 0;
        }

        while( command[0] == '}' )
            if( !command=lies_von_rolle() )
                return 0;

        /* Pause-Command */
        while( pause=check_for_pause(lower_case(command)) )
        {
            if( pause > 0 )
                return pause;
            else
                if( !command=lies_von_rolle() )
                    return 0;
        }

        lcommand=lower_case(command)+" ";

        if( lcommand[0..1] == "if" || lcommand[0..3] == "else" )
        {
            --rollen_status.position;
            continue;
        }

        /* Stop-Command */
        if( lcommand[0..4] == "stop ")
        {
            DEBUG_ROLLE("Die Rolle wird abgebrochen.\n");
            return 0;
        }

        /* "Wechsel_Rolle"-Command */
        if( lcommand[0..13] == "wechsel_rolle:" )
        {
            str=trim(command[14..]);
            DEBUG_ROLLE("Wechsel zu Rolle '"+str+"'.\n");
            rollen_wechsel = 1 ;
            beende_rolle();
            this_object()->starte_rolle(str);
            rollen_wechsel = 0 ;
            return -2;
        }

        /* Goto-Command */
        if( lcommand[0..3] == "goto" )
        {
            struct label label = get_label(command[4..]);
            if(!label)
                return -1;
            DEBUG_ROLLE("Springe nach ["+label.label+"].\n");
            rollen_status.position = label.position;
            continue;
        }

        /* GoSub-Command */
        if( lcommand[0..4] == "gosub" )
        {
            struct label label = get_label(command[5..]);
            if(!label)
                return -1;

            rollen_status.ipstack += ({ rollen_status.position });
            DEBUG_ROLLE("Springe nach ["+label.label+"].\n");
            rollen_status.position = label.position;
            continue;
        }

        /* Return-Command */
        if( lcommand[0..6] == "return " )
        {
            if( !sizeof(rollen_status.ipstack) )
            {
                DEBUG_ROLLE("Die Rolle wird beendet.\n");
                return 0;
            }
            DEBUG_ROLLE("Springe zurück...\n");
            rollen_status.position = rollen_status.ipstack[<1];
            rollen_status.ipstack = rollen_status.ipstack[..<2];
            continue;
        }

        /* Time-Out-Command */
        if( lcommand[0..8] == "time_out " )
        {
            if( sscanf(command[9..],"%d,%s",a,str) != 2 )
            {
                DEBUG_ROLLE("Falsches Aufrufeformat: 'Time_Out zahl, label'\n");
                return -1;
            }
            DEBUG_ROLLE("Time-Out gestartet.\n");
            rollen_status.time_out_seconds = a;
            rollen_status.time_out_label = trim(str);
            continue;
        }

        /* Break-Command */
        if( lcommand[0..13] == "stop_time_out " )
        {
            DEBUG_ROLLE("Time-Out abgebrochen.\n");
            rollen_status.time_out_seconds = 0;
            rollen_status.time_out_label = 0;
            continue;
        }

        /* Wait-Command */
        if( lcommand[0..4] == "wait " )
        {
            if( sscanf(trim(command[5..]),"%s(%s)",fun,arg) != 2 )
            {
                DEBUG_ROLLE("Kein oder falsches Argument hinter 'Wait'.\n");
                return -1;
            }
            fun = trim(fun);
            arg = trim(arg);

            rollen_status.waitfun = function mixed() { return _do_fun_call(fun, arg); };
            return -2;
        }

        /* Fun-Exec */
        if( command[0] == '$' )
        {
            func_parser(command);
            continue;
        }

        /* do_command()-Exec */
        if( sscanf(command,"%d: %s",wer,new_command) != 2 )
        {
            DEBUG_ROLLE("Mit dieser Zeile kann ich nichts anfangen:\n"+
                        command+"\n");
            return -1;
        }

        command = new_command;
        if( wer > sizeof(rollen_status.partner) )
        {
            DEBUG_ROLLE("Einen Partner mit der Nummer "+wer+
                        " gibt es nicht.\n"+command+"\n");
            return -1;
        }

        wer--;
        new_command = func_parser(command);

        if( !rollen_status.partner[wer])
        {
            DEBUG_ROLLE("Der Partner mit der Nummer "+(++wer)+" ist tot.\n");
            return -1;
        }

        if ( this_player_in_rolle &&
            ( (name = this_player_in_rolle->query_real_name()) ||
              (name = this_player_in_rolle->query_name()) )
           )
        {
            new_command = regreplace( new_command, "\\$tp", name, 1) ;
            new_command = regreplace( new_command, "\\$Tp", capitalize( name), 1);
        }

        if ( sizeof( regexp( ({ new_command }), "\\$[tT]p")) == 0 )
            rollen_status.partner[wer]->do_command(new_command);

        if( rollen_status.auto_pause )
            return rollen_status.auto_pause;
    }
    while( rollen_status.position < sizeof(rolle.text_v1) );
}

void rolle()
{
    int pause;

    set_heart_beat(1);

    if( !rolle || !in_rolle() )
    {
        DEBUG_ROLLE("rolle: Ich habe keine Rolle.\n");
        beende_rolle();
        return;
    }

    if( modus != R_SOUFFLEUR )
        return;

    if( zustand == R_STOPPED )
        return;

    if( rolle.version == 2 )
    {
        rolle_v2();
        return;
    }

    if( rollen_status.position >= sizeof(rolle.text_v1) )
    {
        DEBUG_ROLLE("rolle: Ich bin am Ende der Rolle.\n");
        beende_rolle();
        return;
    }

    if( rollen_status.time_out_seconds && !--rollen_status.time_out_seconds )
    {
        struct label label = get_label(rollen_status.time_out_label);
        if( !label )
        {
            beende_rolle();
            return;
        }

        rollen_status.time_out_seconds = 0;
        rollen_status.time_out_label = 0;
        rollen_status.warte = 0;
        rollen_status.waitfun = 0;

        DEBUG_ROLLE("rolle: Time-Out-Sprung nach ["+label.label+"].\n");
        rollen_status.position = label.position;
    }

    if( rollen_status.waitfun && !myfuncall(rollen_status.waitfun, "Wurde ausgeführt durch Wait") )
        return;
    else
        rollen_status.waitfun = 0;

    if( rollen_status.warte > 0 )
    {
        DEBUG_ROLLE("rolle: Ich warte noch "+rollen_status.warte+" mal.\n");
        rollen_status.warte--;
        return;
    }

    if( (pause=drehe_an_rolle()) <= 0 )
    {
        if( !pause )
        {
            DEBUG_ROLLE("rolle: Ende der Rolle erreicht.\n");
            beende_rolle();
        }
        if( pause == -1 )
            beende_rolle();
    }
    else
    {
        DEBUG_ROLLE("rolle: Ich beginne eine Pause ("+pause+").\n");
        rollen_status.warte = pause-1;
    }
}

/*
FUNKTION: beginne_rolle
DEKLARATION: varargs int beginne_rolle(struct rolle r, struct rollen_status rs, string label)
BESCHREIBUNG:
Diese Funktion wird im Souffleur mit der Rolle im 1. Argument und dem
Startlabel im 3. Parameter aufgerufen, wenn alle Voraussetzungen fuer den
Start der Rolle erfuellt wurden. Sie ruft dann beginne_rolle(0,0,label) in
allen weiteren Teilnehmern auf, schaltet den Heart-Beat an und startet die
Rolle sofort.

Diese Funktion wird Rollen-intern verwendet. Sie darf auf keinen Fall
"geshadowt" werden. Auch ist diese Funktion nicht dazu gedacht, von jemand
anderem als starte_rolle() aufgerufen zu werden. Um den Start einer Rolle
mitzubekommen, gibt es notify_rollenstart.

VERWEISE: starte_rolle, notify_rollenstart
GRUPPEN: Rollen
*/
varargs int beginne_rolle(struct rolle r, struct rollen_status rs, string label)
{
    if( !r )
    {
        modus = R_PASSIV;
        souffleur = previous_object();
        zustand = R_RUNNING;
        this_player_in_rolle = souffleur->query_this_player_in_rolle();
        this_object()->notify("rollenstart", this_object(), souffleur, in_rolle(), label);
        return 1;
    }

    rolle = r;
    rollen_status = rs;
    modus = R_SOUFFLEUR;
    zustand = R_RUNNING;
    if ( !rollen_wechsel )
        this_player_in_rolle = this_player();

    if(this_object()->forbidden("rollenstart", this_object(), 0, rolle.name, label))
    {
        DEBUG_ROLLE("rolle: Start der Rolle durch Controller verboten.\n");
        modus = R_AUS;
        zustand = R_STANDBY;
        return 0;
    }

    foreach(object ob: rollen_status.partner)
        if(ob && ob->forbidden("rollenstart", ob, this_object(), rolle.name, label))
    {
        DEBUG_ROLLE("rolle: Start der Rolle durch Controller verboten.\n");
        modus = R_AUS;
        zustand = R_STANDBY;
        return 0;
    }

    if( sizeof(label) )
    {
        if( !member(rolle.labels, label))
        {
            debug_rolle2(sprintf("start_rolle: Das Label '%s' gibt es nicht.\n", label), rolle.name);
            modus = R_AUS;
            zustand = R_STANDBY;
            return 0;
        }
        rollen_status.position = rolle.labels[label];
    }
    else
        rollen_status.position = 0;

    this_object()->notify("rollenstart", this_object(), 0, rolle.name, label);

    foreach (object ob: rollen_status.partner[1..])
        if (ob)
            ob->beginne_rolle(0, 0, label);

    set_heart_beat(1);
    rolle();
    return 1;
}

/*
FUNKTION: forbidden_rollenstart
DEKLARATION: int forbidden_rollenstart(object wer, object souffleur, string rolle, string label)
BESCHREIBUNG:
Bevor in einem Objekt wer die Rolle rolle ab dem Label label gestartet wird,
wird wer->forbidden("rollenstart", wer, souffleur, rolle, label) aufgerufen,
wobei souffleur 0 ist, wenn wer der Souffleur ist, ansonsten das Objekt,
welches die Rolle steuert, ist. Diese Funktion wird in allen Teilnehmern
der Rolle aufgerufen. Liefert einer dieser Funktionen einen Wert != 0,
so kann die Rolle nicht gestartet werden.

Die Funktion forbidden ruft dann in allen mit wer->add_controller(
"forbidden_rollenstart", other) angemeldeten Objekten other die Funktion
other->forbidden_rollenstart(wer, souffleur, rolle, label) auf.
Liefert einer der Aufrufe einen Wert != 0, so liefert forbidden diesen.
VERWEISE: starte_rolle, notify_rollenstart
GRUPPEN: Rollen
*/

/*
FUNKTION: notify_rollenstart
DEKLARATION: void notify_rollenstart(object wer, object souffleur, string rolle, string label)
BESCHREIBUNG:
Wenn in einem Objekt wer die Rolle rolle ab dem Label label gestartet wird,
dann wird wer->notify("rollenstart", wer, souffleur, rolle, label) aufgerufen,
wobei souffleur 0 ist, wenn wer der Souffleur ist, ansonsten das Objekt,
welches die Rolle steuert, ist. Diese Funktion wird in allen Teilnehmern
der Rolle aufgerufen.

Die Funktion notify ruft dann in allen mit wer->add_controller(
"notify_rollenstart", other) angemeldeten Objekten other die Funktion
other->notify_rollenstart(wer, souffleur, rolle, label) auf.
VERWEISE: starte_rolle, forbidden_rollenstart, notify_rollenende
GRUPPEN: Rollen
*/

protected int passt_figur(mapping kriterien)
{
    foreach(string fun, string result: kriterien - (["Objekt","Lebewesen"]))
    {
        if( call_other(this_object(), fun) != result)
            return 0;
    }

    return 1;
}

/*
FUNKTION: spiele_rolle_mit
DEKLARATION: int spiele_rolle_mit(struct rolle rolle, int *offene_positionen)
BESCHREIBUNG:
Diese Funktion wird aufgerufen, um anzufragen, ob dieses Objekt an der
im 1. Parameter übergebenen Rolle teilnehmen möchte, der 2. Parameter enthält
dazu die Indizes der noch offenen Positionen. Es ist nun Aufgabe dieses
Objektes eine offene Teilnehmernummer zu suchen, dessen Kriterien auf dieses
Objekt passen. Außerdem wird geprüft, ob dieses Objekt bereits an einer Rolle
teilnimmt oder gerade eine Kampagne spielt. Die Funktion sollte -1 liefern,
wenn das Objekt nicht teilnehmen kann, ansonsten die gewünschte Position.
VERWEISE: starte_rolle, rolle_machbar
GRUPPEN: Rollen
*/
int spiele_rolle_mit(struct rolle rolle, int *offene_positionen)
{
    if( !this_object()->query_macht_bei_rollen_mit() )
    {
        DEBUG_ROLLE("spiele_rolle_mit: Ich spiele keine Rollen mit.\n");
        return -1;
    }

    if( in_rolle() )
    {
        DEBUG_ROLLE("spiele_rolle_mit: Ich bin bereits in einer Rolle.\n");
        return -1;
    }

    if( this_object()->in_kampagne() )
    {
        DEBUG_ROLLE("spiele_rolle_mit: Ich bin gerade in einer Kampagne.\n");
        return -1;
    }

    foreach (int pos: offene_positionen)
    {
        if (passt_figur(rolle.partner_kriterien[pos]) )
        {
            DEBUG_ROLLE("spiele_rolle_mit: ich spiele mit.\n");
            return pos;
        }
    }
    DEBUG_ROLLE("spiele_rolle_mit: Ich passe nicht zu dieser Rolle.\n");
    return -1;
}

protected object* suche_partner_objekt(int position, closure|string objekt_kriterium, struct rolle rolle, int liv)
{
    mixed raum;
    if(closurep(objekt_kriterium))
    {
        raum = funcall(objekt_kriterium);
        if(objectp(raum))
            raum = ({raum});
        else if (!pointerp(raum))
            return 0;

        raum -= ({0});
    }
    else
    {
        string* path;
        int path_pos;

        if(!sizeof(objekt_kriterium))
        {
            debug_rolle2(sprintf("Kriterium für %i ist leer.\n", position), rolle.name);
            return 0;
        }
        path = map(explode(objekt_kriterium,":"), #'strip);

        if(raum=touch(path[0],NO_LOG|NO_WRITE))
            path_pos=1;
        else if(path[0]=="HIER")
        {
            raum=this_object();

            while(raum && !raum->query_room())
                raum=environment(raum);
            if(!raum)
            {
                debug_rolle2("Bin im Nirwana! (Zumindest in keinem Raum.)\n", rolle.name, 0, all_environment());
                return 0;
            }
            path_pos=1;
        }
        else if((path[0]=="ICH") || (path[0]=="TO"))
        {
            raum=this_object();
            path_pos=1;
        }
        else if(path[0]=="TP")
        {
            raum=this_player();
            path_pos=1;
        }

        if(!raum)
        {
            raum=this_object();

            while(raum && !raum->query_room()) raum=environment(raum);
            if(!raum)
            {
                debug_rolle2("Bin im Nirwana! (Zumindest in keinem Raum.)\n", rolle.name, 0, all_environment());
                return 0;
            }
        }

        raum=({raum});
        for(; sizeof(raum) && path_pos<sizeof(path); path_pos++)
        {
            object *tmp;
            if(path[path_pos]=="^")
                raum = map(raum, #'environment) - ({0});
            else if(path[path_pos]=="*")
            {
                tmp = ({});
                foreach(object ob: raum)
                    tmp += all_inventory(ob) - ({this_object()});
                raum=tmp;
            }
            else if(sizeof((tmp=map(raum, function object(object ob) : string ausgang = path[path_pos]
                                {
                                    return touch(ob->query_one_exit(ausgang), NO_LOG|NO_WRITE);
                                })-({0}))))
                raum=tmp;
            else
            {
                tmp = ({});
                foreach(object ob: raum)
                    tmp += filter_objects(all_inventory(ob) - ({this_object()}), "id", path[path_pos]);
                raum=tmp;
            }
        }
    }

    raum = filter(raum, function int(object ob) { return !playerp(ob); });
    if(liv)
        raum=filter(raum, #'living);

    return raum;
}

protected object* finde_partner(struct rolle rolle)
{
    object *inv, *partner = ({ 0 }) * sizeof(rolle.partner_kriterien);
    int *offen = map(({ 0 }) * sizeof(rolle.partner_kriterien), function int() : int i = 0  { return i++; });

    partner[0] = this_object();
    if( sizeof(offen) <= 1 )
    {
        DEBUG_ROLLE("finde_partner: Ich brauche keine Partner.\n");
        return partner;
    }

    DEBUG_ROLLE("finde_partner: Ich brauche " + (sizeof(offen)-1) + " Partner für diese Rolle.\n");

    // Alle mit Objekt/Lebewesen genau spezifizierten Teilnehmer suchen.
    for(int pos = 1; pos < sizeof(rolle.partner_kriterien); ++pos)
    {
        mapping kriterien = rolle.partner_kriterien[pos];
        closure|string objekt_kriterium = kriterien["Objekt"];
        object* kandidaten;
        int lebend = 0;

        if (!objekt_kriterium)
        {
            objekt_kriterium = kriterien["Lebewesen"];
            lebend = 1;
        }

        if (!objekt_kriterium)
            continue;

        kandidaten = suche_partner_objekt(pos, objekt_kriterium, rolle, lebend);
        if(!kandidaten)
            return 0;

        if(!sizeof(kandidaten))
        {
            DEBUG_ROLLE("Objekt \"" + objekt_kriterium + "\" nicht gefunden.\n");
            return 0;
        }

        foreach(object ob: kandidaten)
        {
            int res;

            DEBUG_ROLLE("finde_partner: Ich prüfe "+object_name(ob)+".\n");

            if (ob in partner)
                continue;

            res = ob->spiele_rolle_mit(rolle, ({pos}));
            if (res == 0)
            {
                // Objekt inheritet nicht rollen.c -> eigener Test
                int i;
                if(call_resolved(&i, ob, "query_macht_bei_rollen_mit") && !i)
                    continue;

                i = 1;
                foreach(string fun, mixed result: kriterien - (["Objekt","Lebewesen"]))
                    if( call_other(ob, fun) != result)
                    {
                        i = 0;
                        break;
                    }
                if (!i)
                    continue;
            }
            else if (res != pos)
                continue;

            partner[pos] = ob;
            DEBUG_ROLLE("finde_partner: Er passt und ist eingetragen.\n");

            break;
        }

        if (!partner[pos])
        {
            DEBUG_ROLLE("Habe kein passendes Objekt gefunden.\n");
            return 0;
        }

        offen[pos] = 0;
    }

    // Alle anderen Teilnehmer suchen.
    offen -= ({0});
    if (!sizeof(offen))
        return partner;

    if( environment() )
        inv=all_inventory(environment());
    else
        inv=all_inventory();

    foreach(object ob: inv)
    {
        int pos;

        if( ob in partner )
            continue;

        DEBUG_ROLLE("finde_partner: Ich prüfe " + ob->query_cap_name()+".\n");

        pos = ob->spiele_rolle_mit(rolle, offen);
        if (pos > 0)
        {
            DEBUG_ROLLE("finde_partner: Er passt und ist eingetragen.\n");
            offen -= ({pos});
            partner[pos] = ob;
            if( !sizeof(offen))
                return partner;
        }
    }
    return 0;
}

protected struct rollen_status rolle_machbar(struct rolle neue_rolle)
{
    int lebend, abbrechbar;
    object *partner;
    closure|string kriterium;

    // Startort prüfen
    if(neue_rolle.start_ort)
    {
        string start_ort = funcall(neue_rolle.start_ort);
        string env;

        if(!stringp(start_ort))
        {
            DEBUG_ROLLE(sprintf("rolle_machbar: Kein String bei Start_Ort (%O).\n", start_ort));
            return 0;
        }

        if (start_ort[<2..] == ".c")
            start_ort = start_ort[..<3];

        env = explode(object_name(environment()||this_object()),"#")[0];
        if (env != start_ort && map2domain(env, 1) != start_ort+".c")
        {
            DEBUG_ROLLE("rolle_machbar: Wir sind nicht am verlangten Startort\n"
                        "       (" + start_ort + ").\n");
            return 0;
        }
    }

    // Kriterien an das eigene Objekt
    kriterium = neue_rolle.partner_kriterien[0]["Objekt"];
    if(!kriterium)
    {
        kriterium = neue_rolle.partner_kriterien[0]["Lebewesen"];
        lebend = 1;
    }
    if(kriterium)
    {
        object *kandidaten = suche_partner_objekt(0, kriterium, neue_rolle, lebend);
        if(!kandidaten || !(this_object() in kandidaten))
        {
            DEBUG_ROLLE("rolle_machbar: Ich passe nicht zu dieser Rolle!\n");
            return 0;
        }
    }
    if( !passt_figur(neue_rolle.partner_kriterien[0]) )
    {
        DEBUG_ROLLE("rolle_machbar: Ich passe nicht zu dieser Rolle.\n");
        return 0;
    }

    // Andere Partner
    partner = finde_partner(neue_rolle);
    if( !partner )
    {
        DEBUG_ROLLE("rolle_machbar: Ich konnte nicht alle "
                    "Partner auftreiben.\n");
        return 0;
    }

    // Sonstige Header-Einträge auswerten
    if (closurep(neue_rolle.abbrechbar))
    {
        string|int res = funcall(neue_rolle.abbrechbar);

        abbrechbar = stringp(res) ? (lower_case(res) == "ja") : res;
    }
    else
        abbrechbar = neue_rolle.abbrechbar;

    return (<rollen_status>
        abbrechbar: abbrechbar,
        auto_pause: funcall(neue_rolle.auto_pause),
        partner: partner,
        ipstack: ({}),
        );
}

/*
FUNKTION: starte_rolle
DEKLARATION: varargs int starte_rolle(string rollenname, string label, varargs mixed *params)
BESCHREIBUNG:
Mit starte_rolle kann man ein Objekt dazu veranlassen, eine Rolle abzuspielen.
Dazu muss nur der Filename der Rolle in rollenname angegeben werden.
Der Dateiname kann auch relativ zum Objektnamen angegeben werden.
Moechte man erreichen, dass die Rolle ab einem bestimmten Label gestartet
wird, so kann man dieses in 'label' uebergeben.
Man kann nach dem 2. Parameter noch weitere Parameter fuer die Rollen der
Version 2 uebergeben, welche mit der Option 'Version: 2' in den Rollendateien
aktiviert wird. Auf die Parameter kann innerhalb der Rollen mit #1, #2 usw.
zugegriffen werden.
Die Funktion liefert 1, bei erfolgreichen Start der Rolle, ansonsten 0.
VERWEISE: in_rolle, query_rolle, restart_rolle, breche_rolle_ab,
          notify_rollenstart, notify_rollenstop
GRUPPEN: Rollen
*/
varargs int starte_rolle(string rollenname, string label, varargs mixed *params)
{
    struct rolle neue_rolle;
    struct rollen_status neuer_status;

    if(!allowed_dirs)
        raise_error("Rollen wurden nicht korrekt initialisiert.\n(Kein ::create() aufgerufen?)\n");

    if( in_rolle() )
    {
        DEBUG_ROLLE("starte_rolle: Ich bin bereits im Modus: "+modus+".\n");
        return 0;
    }

    if( !rollenname )
    {
        DEBUG_ROLLE("starte_rolle: Kein Rollenname übergeben.\n");
        return 0;
    }
    rollenname=abs_path(rollenname);

    if( !rollen_wechsel
     && !sizeof(filter(allowed_dirs, function int(string allowed_dir)
        {
            return !strstr(rollenname, allowed_dir);
        })))
    {
        debug_rolle2("starte_rolle: Rolle "+rollenname+
                     " liegt nicht in den erlaubten Verzeichnissen.\n",
                     __FILE__, __LINE__, allowed_dirs);
        return 0;
    }

    if( function_exists("rolle_erwuenscht",this_object()) &&
       !this_object()->rolle_erwuenscht(rollenname) )
    {
        DEBUG_ROLLE("starte_rolle: Rolle "+rollenname+
                    " von rolle_erwuenscht() abgelehnt.\n");
        return 0;
    }
    if( !(neue_rolle=lese_rolle(rollenname,rolle)) )
    {
        DEBUG_ROLLE("starte_rolle: Die Rolle "+rollenname+
                    " konnte ich nicht einlesen.\n");
        return 0;
    }

    neuer_status = rolle_machbar(neue_rolle);
    if (!neuer_status)
    {
        DEBUG_ROLLE("starte_rolle: Die Rolle "+rollenname+"\n"
                    "              ist nicht machbar.\n");
        return 0;
    }

    init_rollen_parameter(params);
    init_rollen_partner(neuer_status.partner);
    return beginne_rolle(neue_rolle, neuer_status, label);
}

/*
FUNKTION: breche_rolle_ab
DEKLARATION: int breche_rolle_ab()
BESCHREIBUNG:
Mit breche_rolle_ab kann man eine Rolle komplett abbrechen. Wenn dabei
kein Fehler auftritt, wird 1 zurückgeliefert, sonst 0.
Es wird abschliessend rolle_beendet aufgerufen.
VERWEISE: starte_rolle, stoppe_rolle, restart_rolle, rolle_beendet,
          notify_rollenstart, notify_rollenstop
GRUPPEN: Rollen
*/
int breche_rolle_ab()
{
    if( !in_rolle() )
    {
        DEBUG_ROLLE("breche_rolle_ab: Ich bin in keiner Rolle.\n");
        return 1;
    }

    if( modus == R_PASSIV )
    {
        if( !souffleur )
        {
            DEBUG_ROLLE("breche_rolle_ab: Kein Souffleur; ich breche ab.\n");
            beende_rolle();
            return 1;
        }
        if( !souffleur->breche_rolle_ab() )
        {
            DEBUG_ROLLE("breche_rolle_ab: Souffleur "+
                souffleur->query_cap_name()+" bricht nicht ab.\n");
            return 0;
        }

        DEBUG_ROLLE("breche_rolle_ab: Souffleur "+souffleur->query_cap_name()+
                    " hat zugestimmt und bricht ab.\n");
        beende_rolle();
        return 1;
    }

    if( modus == R_SOUFFLEUR )
    {
        if( !rollen_status.abbrechbar )
        {
            DEBUG_ROLLE("breche_rolle_ab: Die Rolle ist nicht abbrechbar.\n")//;
            return 0;
        }

        beende_rolle();
        return 1;
    }
}

/*
FUNKTION: set_this_player_in_rolle
DEKLARATION: void set_this_player_in_rolle( object tp)
BESCHREIBUNG:
Mit dieser Funktion kann das Objekt, das als 'this_player' in der Rolle
gilt, veraendert werden. Zu Beginn einer Rolle wird es automatisch auf
this_player() gesetzt, so dass man diese Funktion nur in Spezialfaellen
benoetigen duerfte.
In Rollen kann man mittels $0 auf diesen 'this_player' zugreifen.
Anwendungen in der Rolle:
  1: knuddle $$0
  1: ich streckt &dem($$0) die Zunge raus.
  1: ich gratuliert &dem($$0). &Der($$0) $(findet,findest) das prima.
  1: ich sagt zu &dem($$2): $Den($0) mag ich gern.
  1: sage Hallo, $$0->query_cap_name()!

VERWEISE: query_this_player_in_rolle
GRUPPEN: Rollen
*/
void set_this_player_in_rolle( object tp)
{
    switch( modus )
    {
        case R_SOUFFLEUR:
            this_player_in_rolle = tp ;
            rollen_status.partner[1..]->set_this_player_in_rolle(tp);
            break;

        case R_PASSIV:
            if( souffleur )
            {
                if ( previous_object() == souffleur )
                    this_player_in_rolle = tp ;
                else
                    souffleur->set_this_player_in_rolle(tp) ;
            }
    }
}

/*
FUNKTION: query_this_player_in_rolle
DEKLARATION: object query_this_player_in_rolle()
BESCHREIBUNG:
Mit dieser Funktion kann das Objekt, das als 'this_player' in der Rolle
gilt, abgefragt werden.
Genauere Beschreibung: siehe set_this_player_in_rolle.
VERWEISE: set_this_player_in_rolle
GRUPPEN: Rollen
*/
object query_this_player_in_rolle()
{
    return this_player_in_rolle;
}

/*
FUNKTION: query_rollen_partner
DEKLARATION: object query_rollen_partner(int nr)
BESCHREIBUNG:
Liefert den Partner mit der jeweiligen Nummer. (0 entspricht dem
this_player_in_rolle). Falls kein Rollenpartner mit der Nummer existiert,
oder keine Rolle aktiv ist, wird 0 geliefert.
VERWEISE: in_rolle, query_this_player_in_rolle
GRUPPEN: Rollen
*/
object query_rollen_partner(int nr)
{
    if( nr<0 || !in_rolle() )
        return 0;

    if( !nr )
        return query_this_player_in_rolle();
    else if( modus == R_SOUFFLEUR )
    {
        if(nr > sizeof(rollen_status.partner))
            return 0;
        else
            return rollen_status.partner[nr-1];
    }
    else if( modus == R_PASSIV )
    {
        if( !souffleur )
            return 0;
        else
            return souffleur->query_rollen_partner(nr);
    }
}

/*
FUNKTION: set_rollen_dirs
DEKLARATION: static void set_rollen_dirs(mixed dirs)
BESCHREIBUNG:
Damit setzt man die Verzeichnisse, aus denen die Rollendateien kommen duerfen.
Man kann entweder ein einzelnes Verzeichnis oder ein Array aus Verzeichnissen
angeben. Mit 0 deaktiviert man Rollen. Diese Bedingung gilt nur fuer
starte_rolle, nicht fuer die bei Wechsel_Rolle angegebenen Rollen.
Standardmaessig ist "/" gesetzt.
VERWEISE: query_rollen_dirs, starte_rolle
GRUPPEN: Rollen
*/
static void set_rollen_dirs(string|string* dirs)
{
    if(stringp(dirs))
        allowed_dirs = ({(sizeof(dirs) && dirs[<1]=='/') ? dirs : (dirs+"/") });
    else if(pointerp(dirs))
        allowed_dirs = map(dirs, function string(string dir)
                       {
                            if (!dir)
                                return 0;
                            if (sizeof(dir) && dir[<1] == '/')
                                return dir;
                            return dir + "/";
                        }) - ({0});
    else
        allowed_dirs = ({});
}

/*
FUNKTION: query_rollen_dirs
DEKLARATION: string *query_rollen_dirs()
BESCHREIBUNG:
Liefert die mit set_rollen_dirs gesetzten Verzeichnisse zurueck.
VERWEISE: set_rollen_dir, starte_rolle
GRUPPEN: Rollen
*/
string *query_rollen_dirs()
{
    return deep_copy(allowed_dirs);
}

void heart_beat()
{
    if(in_rolle() && query_rollen_modus()==R_SOUFFLEUR)
    {
        rolle();
        return;
    }
    set_heart_beat(0);
}

void create()
{
    set_rollen_dirs("");
}

void reset()
{
    if(!in_rolle())
    {
        // Speicher sparen.
        rolle = (<rolle>);
        rollen_status = (<rollen_status>);
    }
}
