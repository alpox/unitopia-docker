// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /i/living/sp.c
// Description:
// Modified by: Jesaia (26.10.97) sp_name, sp_short_name
//		Freaky (13.10.1999) query_sp: headache wird staerker berechnet

#pragma save_types
#pragma strong_types

#include <stats.h>
#include <message.h>
#include <hpspview.h>

// Prototypes
nomask int query_wiz_level();
int query_extended_hp_view();
int query_hp_view();
void hp_sp_view();
varargs void notify_message(string msg, int type);

private int max_sp;
private int sp;
private int sum_sp;
private static string sp_name, sp_short_name;

/*
FUNKTION: set_sp_name
DEKLARATION: void set_sp_name(string name)
BESCHREIBUNG:
set_sp_name veraendert die sp-Bezeichung.
Standard : Zauberpunkte
Variationen : Mentalpunkte, geistige Punkte, ...
VERWEISE: query_sp_name, set_sp_short_name, query_sp_short_name
GRUPPEN: spieler, monster, zaubern
*/
void set_sp_name(string name)
{
   sp_name = name;
}
   
/*
FUNKTION: query_sp_name
DEKLARATION: string query_sp_name()
BESCHREIBUNG:
query_sp_name liefert die sp-Bezeichung, die von Gilden unterschiedlich
gesetzt werden kann.
Standard : Zauberpunkte
Variationen : Mentalpunkte, geistige Punkte, ...
VERWEISE: set_sp_name, set_sp_short_name, query_sp_short_name
GRUPPEN: spieler, monster, zaubern
*/
string query_sp_name()
{
   return sp_name;
}

/*
FUNKTION: query_sp_short_name
DEKLARATION: string query_sp_short_name()
BESCHREIBUNG:
query_sp_short_name liefert die sp-Kurzbezeichnung, die von Gilden
unterschiedlich gesetzt werden kann.
Standard : ZP
Variationen : MP, GP, ...
VERWEISE: set_sp_short_name, set_sp_name, query_sp_name
GRUPPEN: spieler, monster, zaubern
*/
string query_sp_short_name()
{
   return sp_short_name;
}

/* 
FUNKTION: set_sp_short_name  
DEKLARATION: void set_sp_short_name(string name)
BESCHREIBUNG:
set_sp_short_name veraendert die sp-Kurzbezeichung.                               
Standard : ZP
Variationen : MP, GP, ...
VERWEISE: query_sp_short_name, set_sp_name, query_sp_name
GRUPPEN: spieler, monster, zaubern
*/
void set_sp_short_name(string name)
{
   sp_short_name = name;
}

/*
FUNKTION: set_max_sp
DEKLARATION: void set_max_sp(int max_sp)
BESCHREIBUNG:
Mit set_max_sp kann man die maximal moegliche Anzahl von Zauberpunkten, die
ein Spieler oder Monster haben kann, setzen.
VERWEISE: query_max_sp, query_sp,  add_sp
GRUPPEN: spieler, monster, zaubern
*/
void set_max_sp(int tmp)
{
    int old = max_sp;
    if(!intp(tmp))
        raise_error("Bad argument 1 to set_max_sp: not an integer\n");

    if (tmp <= 0)
	max_sp = 0;
    else
	max_sp = tmp;

    if(max_sp == old)
        return;

    if (query_extended_hp_view())
       notify_message(wrap("Deine maximale "+query_sp_name()+"zahl "+
		       "hat sich soeben auf "+max_sp+" geändert."));
    if (sp > (tmp = this_object()->query_max_sp()))
        sp = tmp;

    this_object()->update_points_display();
}

/*
FUNKTION: query_max_sp
DEKLARATION: int query_max_sp()
BESCHREIBUNG:
query_max_sp liefert die maximal moegliche Anzahl von Zauberpunkten, die
ein Spieler oder Monster haben kann, zurueck.
VERWEISE: set_max_sp, query_sp, add_sp
GRUPPEN: spieler, monster, zaubern
*/
int query_max_sp() { return max_sp; }

void update_max_sp()
{
    int tmp, old = max_sp;
#ifdef NEW_STATS
    int intel;

    intel = this_object()->query_stat(STAT_INT);
    if (intel < 30)
    	tmp = intel * 5 / 3 + 4;
    else if (intel < 81)
	tmp = intel * 25 / 8 - 40;
    else
    	tmp = intel * 2 + 50;
#else
    tmp = 50 + this_object()->query_stat(STAT_INT) * 2;
#endif
    if (query_wiz_level())
    {
	if (tmp > max_sp)
	    max_sp = tmp;
    }
    else
	max_sp = tmp;
    if (playerp (this_object()) && (sp > (tmp = this_object()->query_max_sp())))
        sp = tmp;

    if(old != max_sp)
        this_object()->update_points_display();
}

/*
FUNKTION: query_sp
DEKLARATION: int query_sp()
BESCHREIBUNG:
query_sp liefert die momentane Anzahl von Zauberpunkten, die ein Spieler
oder Monster hat, zurueck.
VERWEISE: set_max_sp, query_max_sp, add_sp
GRUPPEN: spieler, monster, zaubern
*/
int query_sp()
{
    int headache, tmp_max_sp;

    headache = this_object()->query_headache();
    if (headache)
    {
	tmp_max_sp = max_sp - (max_sp * headache) / 20;
	if (tmp_max_sp < 10)
	    tmp_max_sp = 10;
	if (sp > tmp_max_sp)
	    return sp = tmp_max_sp;
    }
    return sp;
}

/*
FUNKTION: set_sp
DEKLARATION: void set_sp(int new_sp)
BESCHREIBUNG:
Mit set_sp setzt man die Anzahl von Zauberpunkten, die das Lebewesen
momentan haben soll. Die Maximalanzahl an Zauberpunkten wird damit
aber nicht veraendert (und kann auch nicht ueberschritten werden).
VERWEISE: set_max_sp, query_sp, query_max_sp, add_sp
GRUPPEN: spieler, monster, zaubern
*/
void set_sp(int new_sp)
{
    int tmp_max, old = sp;

    if(!intp(new_sp))
        raise_error("Bad argument 1 to set_sp: not an integer\n");

    tmp_max = this_object()->query_max_sp ();
    if (new_sp < 0)
	sp = 0;
    else if (new_sp > tmp_max)
	sp = tmp_max;
    else
	sp = new_sp;

    if(old == sp)
        return;

    if (query_extended_hp_view())
	notify_message(wrap("Du hast jetzt "+sp+" "+query_sp_name()+"."));

    this_object()->update_points_display();
}

/*
FUNKTION: add_sp
DEKLARATION: void add_sp(int sp)
BESCHREIBUNG:
Addiert sp Zauberpunkte zu den momentanen Zauberpunkten des Spielers oder
Monsters dazu. Wenn sp negativ ist, werden ZPs abgezogen, es werden aber nicht
mehr abgezogen, als vorhanden sind und nicht mehr dazugezahlt, als ohne 
Ueberschreitung von query_max_sp moeglich waere.
VERWEISE: set_max_sp, query_max_sp, query_sp
GRUPPEN: spieler, monster, zaubern
*/
void add_sp(int tmp)
{
    // sum_sp addition geaendert,
    // um pannen wegen schlecht programmierten Tools,... zu verhindern.
    // Garthan Thu May 11 23:29:24 1995
    int old_sp;

    if(!intp(tmp))
        raise_error("Bad argument 1 to add_sp: not an integer\n");

    if (!tmp)
	return;
    // Um die Kopfschmerzen mit einzuberechnen
    query_sp();
    old_sp = sp;
    sp += tmp;
    if (sp < 0)
	sp = 0;
    if (sp > (tmp = this_object()->query_max_sp()))
	sp = tmp;
    if (sp != old_sp && query_extended_hp_view())
	notify_message(wrap("Du hast " + sp + " " + query_sp_name() + "."));
    else
       if ((sp < old_sp) && (query_hp_view() & HP_SP_VIEW_SP_MINUS)) {
          if (find_call_out ("hp_sp_view") == -1)
             call_out ("hp_sp_view",2);
       } 
       else if ((sp > old_sp) && (query_hp_view() & HP_SP_VIEW_SP_PLUS)) {
          if (find_call_out ("hp_sp_view") == -1)
             call_out ("hp_sp_view",2);
       }
    if (sp < old_sp)
	sum_sp += old_sp - sp ;

    if (sp != old_sp)
        this_object()->update_points_display();
}

/*
FUNKTION: query_sum_sp
DEKLARATION: int query_sum_sp()
BESCHREIBUNG:
query_sum_sp liefert die Anzahl von Zauberpunkten zurueck, die ein Spieler
(oder Monster) in seiner Laufbahn je verbraten hat, zurueck.
VERWEISE: add_sum_sp
GRUPPEN: spieler, monster, zaubern, skill
*/
int query_sum_sp() { return sum_sp; }

/*
FUNKTION: add_sum_sp
DEKLARATION: void add_sum_sp(int sum_sp)
BESCHREIBUNG:
Erhoeht die Summe aller jemals verbratenen Zauberpunkte eines Monsters oder
Spieler. WARNUNG: keine Ueberpruefung auf Ueberlauf! Keine Ueberpruefung
auf negative Ergabnisse. Bitte nicht auf Spieler anwenden!
VERWEISE: query_sum_sp
GRUPPEN: spieler, monster, zaubern, skill
*/
void add_sum_sp(int i) { sum_sp = to_int(sum_sp + i); }
