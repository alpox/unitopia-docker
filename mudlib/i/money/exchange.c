// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/i/money/exchange.c
// Description: Umrechnung verschiedener Waehrungen.
// Author:	Francis
// Modified By: Garthan: Kurse aus der Zentralbank holen, wenn nicht gesetzt.

/*
 * wert_waehrung2 = convert(wert_waehrung1,waehrung1,waehrung2)
 */

#pragma save_types

#include <money.h>

private string *valuta;
private string *valutas;
private int *kurs;

#define VALUTA  (valuta  ? valuta  : ZENTRALBANK->query_valuta_tafel())
#define VALUTAS (valutas ? valutas : ZENTRALBANK->query_valutas_tafel())
#define KURS    (kurs    ? kurs    : ZENTRALBANK->query_kurs_tafel())
#define UPDATE_VALUTA(str) (ZENTRALBANK->update_valuta((str)) || (str))
#define UPDATE_VALUTAS(str) (ZENTRALBANK->update_valutas((str)) || (str))

/*
FUNKTION: convert
DEKLARATION: int convert(int value, string ovaluta, string nvaluta)
BESCHREIBUNG:
convert() rechnet einen Geldbetrag gegeben durch value (Wert) und ovaluta
(Waehrung) in eine andere Waehrung gegeben durch nvaluta um. Zurueckgegeben
wird der Wert in dieser Waehrung.

Als Waehrung kann auch 0 angegeben werden, es wird dann die Basiswaehrung
verwendet (dies entspricht i. d. R. dem Taler).

Beispiele:

    convert(1000, "gulden", "taler") rechnet 1000 Gulden in Taler um.

    convert(ob->query_value(), 0, "riki") berechnet den Wert des Objektes
                                          ob in Riki.

VERWEISE: convert_rounded, set_kurs_tafel, set_valuta_tafel, set_valutas_tafel
GRUPPEN: handel
*/
float convert_float(float value, string ovaluta, string nvaluta)
{
    // Waehrungstabelle ermitteln; Waehrung==0 ist Basiswaehrung (Warenwert):
    string *p_valuta=({0})   +VALUTA;
    int    *p_kurs  =({1000})+KURS;

    // Indizes in der Tabelle berechnen:
    int o=member(p_valuta, UPDATE_VALUTA(ovaluta));
    int n=member(p_valuta, UPDATE_VALUTA(nvaluta));

    // Eine der Waehrungen nicht in der Tabelle enthalten? Dann Fehler:
    if(o==-1 || n==-1)
        return -1.0;

    return to_float(value)*to_float(p_kurs[n])/to_float(p_kurs[o]);
}

int convert(int value, string ovaluta, string nvaluta)
{
    return to_int(floor(convert_float(to_float(value),ovaluta,nvaluta)));
}

/*
FUNKTION: convert_rounded
DEKLARATION: int convert_rounded(int value, string ovaluta, string nvaluta)
BESCHREIBUNG:
Rechnet analog zu convert() einen Geldbetrag value in der Währung ovaluta
(falls 0, so wird die Basiswährung Taler angenommen) in die Währung nvaluta
um.

Im Gegensatz zu convert() wird der Betrag kaufmännisch gerundet.
VERWEISE: convert
GRUPPEN: handel
*/
int convert_rounded(int value, string ovaluta, string nvaluta)
{
    return to_int(floor(convert_float(to_float(value),ovaluta,nvaluta) + 0.5));
}

/*
FUNKTION: query_valuta_tafel
DEKLARATION: string *query_valuta_tafel()
BESCHREIBUNG:
Liefert alle erlaubten Zahlungsmittel.
Dies sind sofern nicht durch set_valuta_tafel() anders angegeben, alle
der Zentralbank bekannten Waehrungen.
VERWEISE: set_valuta_tafel, query_valutas_tafel, query_kurs_tafel
GRUPPEN: handel
*/
string *query_valuta_tafel() { return VALUTA; }

/*
FUNKTION: query_valutas_tafel
DEKLARATION: string *query_valutas_tafel()
BESCHREIBUNG:
Liefert alle erlaubten Zahlungsmittel im Plural.
Dies sind sofern nicht durch set_valuta_tafel() anders angegeben, alle
der Zentralbank bekannten Waehrungen.
VERWEISE: set_valutas_tafel, query_valuta_tafel, query_kurs_tafel
GRUPPEN: handel
*/
string *query_valutas_tafel() { return VALUTAS; }

/*
FUNKTION: query_kurs_tafel
DEKLARATION: int *query_kurs_tafel()
BESCHREIBUNG:
Liefert die aktuellen Kurse. Die zu den Zahlen gehoerigen Werte erhaelt man
mit query_valuta_tafel(). Der Wert query_kurs_tafel()[i] gehoert also zur
Waehrung query_valuta_tafel()[i].
VERWEISE: set_kurs_tafel, query_valuta_tafel, query_valutas_tafel
GRUPPEN: handel
*/
int *query_kurs_tafel() { return KURS; }

/*
FUNKTION: set_valuta_tafel
DEKLARATION: void set_valuta_tafel(string *str)
BESCHREIBUNG:
Damit werden alle akzeptierten Waehrungen gesetzt. Wird 0 angegeben, so
werden alle, der Zentralbank bekannten Waehrungen genommen.
Der Plural der Waehrungen und ihre Umrechnungskurse sollten auch
mit set_valutas_tafel und set_kurs_tafel gesetzt werden.
VERWEISE: query_valuta_tafel, set_valutas_tafel, set_kurs_tafel
GRUPPEN: handel
*/
void set_valuta_tafel(string *str) { valuta = map(str, (:UPDATE_VALUTA($1):)); }

/*
FUNKTION: set_valutas_tafel
DEKLARATION: void set_valutas_tafel(string *str)
BESCHREIBUNG:
Damit wird der Plural aller akzeptierten Waehrungen gesetzt. Wird 0 angegeben,
so werden alle, der Zentralbank bekannten Waehrungen genommen.
Der Singular der Waehrungen und ihre Umrechnungskurse sollten auch
mit set_valuta_tafel und set_kurs_tafel gesetzt werden.
VERWEISE: query_valutas_tafel, set_valuta_tafel, set_kurs_tafel
GRUPPEN: handel
*/
void set_valutas_tafel(string *str) { valutas = map(str,(:UPDATE_VALUTAS($1):)); }

/*
FUNKTION: set_kurs_tafel
DEKLARATION: void set_kurs_tafel(string *str)
BESCHREIBUNG:
Damit werden die Umrechnungskurse gesetzt. Wird 0 angegeben, so werden
die Umrechnungskurse der Zentralbank genommen. Diese Umrechnungskurse
beziehen sich auf die mit set_valuta_tafel und set_valutas_tafel gesetzten
Waehrungen, weshalb diese auch gesetzt (bzw. auf 0 gesetzt) werden sollten.
VERWEISE: query_kurs_tafel, set_valuta_tafel, set_valutas_tafel
GRUPPEN: handel
*/
void set_kurs_tafel(int *a) { kurs = a; }

/*
FUNKTION: query_accepted_valutas
DEKLARATION: string query_accepted_valutas(string str)
BESCHREIBUNG:
Liefert 0, wenn die angegebene Waehrung nicht akzeptiert wird, ansonsten
der Plural der Waehrung.
VERWEISE: query_accepted_valuta, query_valutas_tafel
GRUPPEN: handel
*/
string query_accepted_valutas(string str)
{
   int i;

   str = lower_case(str);
   if((i = member(VALUTAS, UPDATE_VALUTAS(str))) < 0 &&
      (i = member(VALUTA, UPDATE_VALUTA(str))) < 0)
      return 0;
   return VALUTAS[i];
}

/*
FUNKTION: query_accepted_valuta
DEKLARATION: string query_accepted_valuta(string str)
BESCHREIBUNG:
Liefert 0, wenn die angegebene Waehrung nicht akzeptiert wird, ansonsten
der Singular der Waehrung.
VERWEISE: query_accepted_valutas, query_valutas_tafel
GRUPPEN: handel
*/
string query_accepted_valuta(string str)
{
   int i;

   if(!stringp(str))
      return 0;
   str = lower_case(str);
   if((i = member(VALUTA, UPDATE_VALUTA(str))) < 0 &&
      (i = member(VALUTAS,UPDATE_VALUTAS(str))) < 0)
      return 0;
   return VALUTA[i];
}
