// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /i/tools/coroutine.c
// Author:      Gnomi
// Description: Hilfsfunktionen für Coroutinen

#ifdef __LPC_COROUTINES__

#include <input_to.h>

struct enumerated
{
    int number;
    mixed value;
};

/*
FUNKTION: sleep
DEKLARATION: async void sleep(int sec)
BESCHREIBUNG:
Unterbricht eine Coroutine für die angegebene Anzahl an Sekunden.

Beispiel:
    await(sleep(10));

VERWEISE: call_out
GRUPPEN: coroutinen
*/
async void sleep(int sec)
{
    call_out(#'call_coroutine, sec, this_coroutine());

    yield();
}

/*
FUNKTION: get_input
DEKLARATION: async string get_input(string prompt)
BESCHREIBUNG:
Unterbricht eine Coroutine für eine Eingabe des aktuellen Spielers
(this_player()) zu warten und liefert diese Eingabe zurück.

Beispiel:
    string choice = await(get_input("Auswahl: "));

VERWEISE: input_to
GRUPPEN: coroutinen
*/
async string get_input(string prompt)
{
    input_to(function string(string msg) : coroutine cr = this_coroutine()
    {
        call_coroutine(cr, msg);
    }, INPUT_PROMPT, prompt);

    return yield();
}

/*
FUNKTION: ensure_eval
DEKLARATION: async void ensure_eval(int reserve=100000)
BESCHREIBUNG:
Wenn die verbleibenden Evals unter das angegebene Limit fallen,
wird die Coroutine unterbrochen und eine Sekunde später fortgesetzt.

Beispiel:
    await(ensure_eval());

VERWEISE: get_eval_cost, call_out
GRUPPEN: coroutinen
*/
async void ensure_eval(int reserve=100000)
{
    while (get_eval_cost() < reserve)
    {
        call_out(#'call_coroutine, __ALARM_TIME__, this_coroutine());
        yield();
    }

    return;
}

/*
FUNKTION: range
DEKLARATION: async void range(int start, int stop, int step = 1)
BESCHREIBUNG:
Liefert einen Zahlengenerator, der von start (inkl.) bis stop (exkl.)
in Schritten von step zählt. Solch ein Generator kann z.B. mit foreach
verwendet werden.

Beispiel:
    foreach (int num: range(10, 100, 5))
        printf("Schritt: %d\n", num);

VERWEISE: foreach, enumerate
GRUPPEN: coroutinen
*/
async void range(int start, int stop, int step = 1)
{
    for (int i = start; i < stop; i+= step)
        yield(i);
}

/*
FUNKTION: enumerate
DEKLARATION: async void enumerate(coroutine|mixed* values, int start = 0)
BESCHREIBUNG:
Liefert die Werte aus values in einer Struktur mit fortlaufender Numerierung.

Die Struktur enumerated hat folgende Member:
    int number;
    mixed value;

Beispiel:
    foreach (struct enumerated arg: enumerate(args))
        printf("Argument %d: %Q\n", arg.number, arg.value);

VERWEISE: foreach, range
GRUPPEN: coroutinen
*/
async void enumerate(coroutine|mixed* values, int start = 0)
{
    int i = start;
    foreach(mixed value: values)
        yield((<enumerated> number: i++, value: value));
}

/*
FUNKTION: reversed
DEKLARATION: async void reversed(mixed* values)
BESCHREIBUNG:
Liefert die Werte aus values in umgekehrter Reihenfolge.

Beispiel:
    foreach (int num: reversed(({1,2,3,4,5}))
        printf("Wert: %d\n", num);

VERWEISE: foreach, range, enumerate
GRUPPEN: coroutinen
*/
async void reversed(mixed * values)
{
    for (int i = sizeof(values); i--;)
        yield(values[i]);
}

/*
FUNKTION: latch
DEKLARATION: async mixed* latch(coroutine* crs, int num)
BESCHREIBUNG:
Unterbricht die aktuelle Coroutine solange, bis mindestens num
Coroutinen aus der übergebenen Liste fertig sind. Die Funktion
liefert ein Array gleicher Größe mit den Ergebnissen zurück
(die ggf. noch 0 sein können, wenn die Coroutine nicht beendet
ist).

Die Funktionen for_any und for_all sind spezielle Varianten
dieser Funktion.

VERWEISE: for_any, for_all
GRUPPEN: coroutinen
*/
async mixed* latch(coroutine* crs, int num)
{
    mixed* result = ({0}) * sizeof(crs);
    int count;

    for (int i = 0; i < sizeof(crs); i++)
    {
        call_coroutine(async function void() : int count = &count;
                                               coroutine cr = crs[i];
                                               coroutine outer = this_coroutine()
        {
            result[i] = await(cr);

            count++;
            if (count == num)
                call_coroutine(outer);
        });
    }

    yield();

    return result;
}

/*
FUNKTION: for_any
DEKLARATION: coroutine for_any(varargs coroutine *crs)
BESCHREIBUNG:
Unterbricht die Coroutine bis eine der angegebenen Routinen fertig ist.
Die Funktion liefert ein Array aus den Ergebnissen zurück (der erste
Eintrag entspricht der ersten Coroutine usw.), die ggf. aber noch 0
sein können, wenn die jeweiligen Coroutinen noch nicht beendet sind.

Beispiel:
    await(for_any(sleep(2), sleep(10)));
    // Wartet auf den schnellsten, also insgesamt 2 Sekunden.

VERWEISE: for_all, latch
GRUPPEN: coroutinen
*/
coroutine for_any(varargs coroutine *crs)
{
    return latch(crs, 1);
}

/*
FUNKTION: for_all
DEKLARATION: coroutine for_all(varargs coroutine *crs)
BESCHREIBUNG:
Unterbricht die Coroutine bis die angegebenen Routinen fertig sind.
Die Funktion liefert ein Array aus den Ergebnissen zurück (der erste
Eintrag entspricht der ersten Coroutine usw.).

Beispiel:
    await(for_all(sleep(2), sleep(10)));
    // Wartet auf alle, also insgesamt 10 Sekunden.

VERWEISE: for_any, latch
GRUPPEN: coroutinen
*/
coroutine for_all(varargs coroutine *crs)
{
    return latch(crs, sizeof(crs));
}

/*
FUNKTION: read_bytes_async
DEKLARATION: async protected void read_bytes_async(string file, int start = 0, int block_size = 100000)
BESCHREIBUNG:
Eine Coroutine, welche die Datei blockweise liest.
VERWEISE: read_lines, read_bytes
GRUPPEN: coroutinen
*/
async protected void read_bytes_async(string file, int start = 0, int block_size = 100000)
{
    int pos = start;

    while (1)
    {
        bytes result = read_bytes(file, pos, block_size);
        if (!result)
            return;

        yield(result);
        if (sizeof(result) < block_size)
            return;

        pos += block_size;
    }
}

/*
FUNKTION: read_lines
DEKLARATION: async protected void read_lines(string file, string encoding = "UTF-8")
BESCHREIBUNG:
Liest die angegebene Datei ein und liefert einzelne Zeilen als Strings.
Die gelieferten Zeilen beinhalten alle den Zeilenumbruch am Ende.

Beispiel:
    foreach (string line: read_lines(__FILE__))
        write(line);

VERWEISE: read_file, read_bytes_async
GRUPPEN: coroutinen
*/
async protected void read_lines(string file, string encoding = "UTF-8")
{
    bytes rest = b"";

    foreach (bytes block: read_bytes_async(file))
    {
        bytes* lines = explode(rest + block, b"\n");

        rest = lines[<1];
        foreach (bytes line: lines[..<2])
            yield(to_text(line, encoding) + "\n");
    }

    if (sizeof(rest))
        yield(to_text(rest, encoding));
}

#endif
