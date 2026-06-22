// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/i/living/stats.c
// Description:

#pragma save_types
#pragma strong_types

#include <stats.h>
#include <config.h>
#include <hlp.h>
#include <level.h>
#include <apps.h>

// Prototypes:
void set_max_internal_encumbrance(int i);
nomask int query_wiz_level();

private mixed * stats = INIT_STAT_ARRAY;

void update_max_encumbrance()
{
#ifdef RETAIN_PLAYER_INVENTORY
    int maxi;
    if (query_wiz_level())
        maxi = WIZ_ENCUMBRANCE;
    else {
        maxi =
#ifdef NEW_STATS
	(5 + this_object()->query_stat(STAT_STR) / 4);
#else
	(10 + this_object()->query_stat(STAT_STR) / 5);
#endif
	if (GABE(this_object(), "la"))
	    maxi += (maxi / 2);
    }
    set_max_internal_encumbrance (maxi);
#else
    set_max_internal_encumbrance(
	query_wiz_level() ? WIZ_ENCUMBRANCE :
	((GABE(this_object(), "la")) ? 2 : 1) *
#ifdef NEW_STATS
	(2 + this_object()->query_stat(STAT_STR) * 2 / 7));
#else
	(10 + this_object()->query_stat(STAT_STR) / 5));
#endif
#endif
}

/*
FUNKTION: update_stat_dependencies
DEKLARATION: void update_stat_dependencies(int flag)
BESCHREIBUNG:
Diese Funktion berechnet Folgen aus Aenderungen von Staerke, Ausdauer,
Intelligenz oder Geschicklichkeit. D.h. diese Funktion ruft z.B.
update_max_encumbrance(), update_max_sp() und update_max_hp() auf.
Ist flag==1, so wird ebenfalls die Ausdauer- und Zauberpunktezahl auf
das Maximum angehoben (sollte man nur bei NPCs anwenden).
VERWEISE: set_one_stat, query_stat
GRUPPEN: spieler, monster, skill
*/
void update_stat_dependencies(int flag)
{
    this_object()->update_max_encumbrance();
    this_object()->update_max_sp();
    this_object()->update_max_hp();
    if(flag)
    {
	this_object()->set_hp(this_object()->query_max_hp());
	this_object()->set_sp(this_object()->query_max_sp());
    }
}

/*
FUNKTION: set_one_stat
DEKLARATION: void set_one_stat(int stat, int level)
BESCHREIBUNG:
Setzt die Faehigkeit stat auf den Betrag level. Die verschiedenen Faehigkeiten
sind als STAT_STR, STAT_INT, STAT_CON und STAT_DEX in stats.h definiert.
Diese Werte werden relativ zur Rasse angesehen, wobei ein Level von 0 dem
schwaechsten Exemplar und ein Level von 100 dem staerksten Exemplar der
Rasse entspricht.
VERWEISE: query_stat, update_stat_dependencies
GRUPPEN: spieler, monster, skill
*/
void set_one_stat(int i, int level)
{
    if ((!extern_call()
	|| !query_once_interactive(this_object())
	|| testplayerp(this_object())
	|| !strstr(object_name(previous_object()),LOGIN_OB)) &&
	(i < STAT_NUMBER && i >= 0))
    {
#ifdef NEW_STATS
	if (object_name(previous_object()) == MONSTER_MASTER)
	    stats[i] = level;
	else
	    stats[i] = MONSTER_MASTER->get_one_stat(
	    	this_object()->query_initial_race(),i,level);
#else
	if (level < 1)
	    level = 1;
	else if (level > 100)
	    level = 100;
	stats[i] = level;
#endif
    }
}

static void init_stats(int level)
{
    int i;

#ifndef NEW_STATS
    if (level < 1)
	level = 1;
    else if (level > 100)
	level = 100;
#endif
    for (i=0; i<STAT_NUMBER; i++)
#ifdef NEW_STATS
	set_one_stat(i,level);
#else
	stats[i] = level;
#endif
}

#ifdef TestMUD
/*
NOENZY: query_stat_formular
DEKLARATION: mapping query_stat_formular(int stat)
BESCHREIBUNG:
Gibt eine Formel fuer die Statsanpassung zurueck. Und zwar einen "factor":1.0
fuer 100% und "offset":0.0. Zusaetzlich kann ein min und max definiert werden,
die nich tunter bzw ueberschritten werden. Die verschiedenen Faehigkeiten
sind als STAT_STR, STAT_INT, STAT_CON und STAT_DEX in stats.h definiert.
Neue Stats-Shadows ueberlagern die Funktion wie folgt:
  mapping query_stat_formular(int stat)
  {
    mapping ret = QSO->query_stat_formular(stat)||STAT_DEFAULT_FORMULA;
    // switch (stat)... um die verschiedenen Stats unterschiedlich zu behandeln
    ret["factor"] *= 1.20;  // Beispiel Faktor 20% mehr
    ret["offset"] += 2;     // Beispiel Offset 2 Punkte mehr.
    ret["min"] = 30.0;
    ret["max"] = 120.0;
    return ret;
  }
Shadows die query_stat_formular ueberlagern, ueberlagern query_stat nicht mehr.
VERWEISE: query_stat, update_stat_dependencies
GRUPPEN: spieler, monster, skill
*/
mapping query_stat_formular(int stat)
{
    return STAT_DEFAULT_FORMULA;
}
#endif

/*
FUNKTION: query_stat
DEKLARATION: varargs mixed query_stat(int stat, int flag)
BESCHREIBUNG:
Liefert die Faehigkeit stat zurueck. Als stat sind die in stats.h definierten
Defines zu verwenden.

Es wird normalerweise eine Integer-Zahl zurueckgeliefert.
Ist der optionale Parameter flag != 0, so kann (NICHT muss) auch
eine Float-Zahl zurueckgeliefert werden. Dies ist nur bei Spielern der Fall.

Die Float-Stats duerfen auf keinen Fall fuer Berechnungen irgendwelcher
Art verwendet werden; Ausnahme sind Stat-Shadows, die die Stats des
Spielers veraendern.

Die Float-Stats sind ansonsten nur zur Ausgabe (sp, Spielerausweis, ...)
gedacht; hierzu ist das Makro PRINT_STAT aus stats.h zu verwenden.
Um einen Float-Stat in eine Integer-Zahl umzuwandeln, verwende man
das Makro ROUND_STAT.

ACHTUNG:
    Wenn man query_stat() ueberlagert, muss die Originalfunktion mit
    gesetztem Parameter 'flag' aufgerufen werden, sofern man deren
    Ergebnis zur Berechnung eines neuen Wertes verwenden moechte.

    Man muss also in der Ueberlagerung in jedem Fall mit Float
    rechnen, sofern die Originalfunktion einen Float-Wert liefert.

    Wurde die ueberlagernde Funktion mit 'flag' == 0 aufgerufen,
    muss das Ergebnis mit ROUND_STAT gerundet zurueckgeliefert werden.

    Andernfalls kann es zu schweren Rundungsfehlern kommen.

VERWEISE: set_one_stat, update_stat_dependencies,
          PRINT_STAT, ROUND_STAT
GRUPPEN: spieler, monster, skill
*/
varargs mixed query_stat(int stat, int flag)
{
#ifdef TestMUD
    mapping m_formula = this_object()->query_stat_formular(stat)
                        || STAT_DEFAULT_FORMULA; 
    float my_stat = to_float(stats[stat])*m_formula["factor"]
                  + m_formula["offset"];
    if (member(m_formula,"min") && m_formula["min"] > my_stat)
        my_stat = m_formula["min"];
    if (member(m_formula,"max") && m_formula["max"] < my_stat)
        my_stat = m_formula["max"];
    return (flag ? my_stat : ROUND_STAT(my_stat));
#else
    return (flag ? stats[stat] : ROUND_STAT(stats[stat]));
#endif
}

/*
FUNKTION: query_real_stat
DEKLARATION: nomask varargs mixed query_real_stat(int stat, int flag)
BESCHREIBUNG:
Liefert die Faehigkeit stat zurueck. Als stat sind die in stats.h definierten
Defines zu verwenden.
Kann im Gegensatz zu query_stat nicht ueberlagert/geshadowed werden.
Dient beispielsweise als Bewertungsgrundlage fuer Leo.

Es wird normalerweise eine Integer-Zahl zurueckgeliefert.
Ist der optionale Parameter flag != 0, so kann (NICHT muss) auch
eine Float-Zahl zurueckgeliefert werden. Dies ist nur bei Spielern der Fall.

Die Float-Stats duerfen auf keinen Fall fuer Berechnungen irgendwelcher
Art verwendet werden; Ausnahme sind Stat-Shadows, die die Stats des
Spielers veraendern.

Die Float-Stats sind ansonsten nur zur Ausgabe (sp, Spielerausweis, ...)
gedacht; hierzu ist das Makro PRINT_STAT aus stats.h zu verwenden.
Um einen Float-Stat in eine Integer-Zahl umzuwandeln, verwende man
das Makro ROUND_STAT.
VERWEISE: set_one_stat, query_stat, query_real_stats,
          PRINT_STAT, ROUND_STAT
GRUPPEN: spieler, monster, skill
*/
nomask varargs mixed query_real_stat(int stat, int flag)
{
    return (flag ? stats[stat] : ROUND_STAT(stats[stat]));
}

/*
FUNKTION: query_real_stats
DEKLARATION: nomask varargs mixed * query_real_stats(int flag)
BESCHREIBUNG:
Liefert alle Faehigkeiten als Array zurueck.
Als Index fuer dieses Array sollten die Defines aus stats.h verwendet
werden. Funktion kann nicht ueberlagert/geshadowed werden.

Das zurueckgelieferte Array besteht normalerweise aus Integer-Zahlen.
Ist der optionale Parameter flag != 0, so kann (NICHT muss) das Array
auch Float-Zahlen enthalten. Dies ist nur bei Spielern der Fall.

Die Float-Stats duerfen auf keinen Fall fuer Berechnungen irgendwelcher
Art verwendet werden; Ausnahme sind Stat-Shadows, die die Stats des
Spielers veraendern.

Die Float-Stats sind ansonsten nur zur Ausgabe (sp, Spielerausweis, ...)
gedacht; hierzu ist das Makro PRINT_STAT aus stats.h zu verwenden.
Um einen Float-Stat in eine Integer-Zahl umzuwandeln, verwende man
das Makro ROUND_STAT.
VERWEISE: set_one_stat, query_stat, query_real_stat,
          PRINT_STAT, ROUND_STAT
GRUPPEN: spieler, monster, skill
*/
nomask varargs mixed * query_real_stats(int flag)
{
    return (flag ? copy(stats) : map(stats, (: ROUND_STAT($1) :)));
}

static void raise_stat(int stat, float value)
{
    if(stats[stat] < value)
    {
        stats[stat] = value;
        this_object()->update_points_display();
    }
}

// Nur in der Mudlib zu verwenden.
protected void add_stat(int stat, float diff)
{
    stats[stat] += diff;
    this_object()->update_points_display();
}
