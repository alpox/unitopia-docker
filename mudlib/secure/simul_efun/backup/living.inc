// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/secure/simul_efun/living.inc
// Description: Living-routinen
// Author:	Freaky (10.09.94) von Amylaar uebernommen und angepasst
// Modified by:	Freaky (28.10.94) start_simul_efun umbenannt in create
// 		Freaky (21.02.96) player_exists weniger Checks
//		Freaky (12.10.1999) global_info eingefuehrt und
//			wizardshellp nach util.inc verschoben

#pragma strict_types
#pragma save_types

/* --- Globale Variablen: --- */
private static mapping living_name_m;   /* object: name                        */
private static mapping name_living_m;   /* name: object oder ({ object, ... }) */

/* --- Prototypen: --- */
string convert_umlaute(string str);

/* --- Initialisierung: --- */
protected void living_create()
{
    living_name_m = global_info["living_name"] ||= m_allocate(0, 1);
    name_living_m = global_info["name_living"] ||= m_allocate(0, 1);
}

protected void living_reset()
{
    /* There might be destructed objects as keys. */
    m_indices(living_name_m);

    if (find_call_out("clean_name_living_m") < 0)
    {
	call_out("clean_name_living_m", 1,
                 m_indices(name_living_m), sizeof(name_living_m));
    }
}

static void clean_name_living_m(string *keys, int left)
{
    /* Entfernen wir mal zerstoerte Objekte aus name_living_m.
     * Wir sind bei Eintrag 'left' in 'keys' und arbeiten
     * uns nach vorn.
     */

    if (left)
    {
        string key = keys[--left];
        mixed a = name_living_m[key];

        if (pointerp(a))
        {
            int size = sizeof(a);

            a -= ({0});

            if (!sizeof(a))
                m_delete(name_living_m, key);
            else if (sizeof(a) == 1)
                name_living_m[key] = a[0];
            else if (sizeof(a) != size)
                name_living_m[key] = a;
        }
        else if(!a)
            m_delete(name_living_m, key);

        call_out("clean_name_living_m", 1, keys, left);
    }
}

/* --- Simul-Efuns: --- */
#if 0
mapping query_living_name_m() { return living_name_m; }
mapping query_name_living_m() { return name_living_m; }
#endif

private object internal_find_living(string name, closure condition)
{
    mixed r;

    if (pointerp(r = name_living_m[name]))
    {
        foreach (int i: sizeof(r))
        {
            object l = r[i];

            if (funcall(condition, l))
            {
                if (i)
                {
                    /* Spieler nach vorn bewegen. */
                    r[1..i] = r[0..i-1];
                    r[0] = l;
                }
                return l;
            }
        }
        return 0;
    }
    return funcall(condition, r) && r;
}

/*
FUNKTION: find_player
DEKLARATION: object find_player(string name)
BESCHREIBUNG:
Findet den Spieler mit dem Namen 'name'
Die Funktion liefert das Player-Objekt des Spielers.
'name' muss dabei der Name sein, der mit set_living_name() gesetzt wurde.
VERWEISE: set_living_name, find_living, find_object
GRUPPEN: simul_efun, player, objekt
*/
object find_player(string name)
{
    return internal_find_living(name, function int(object l)
    {
        return l && ({int})MASTER_OB->playerp(l);
    });
}

private void internal_set_living_name(object ob, string old, string new)
{
    if (old)
    {
        mixed a = name_living_m[old];
        if (pointerp(a))
        {
            a -= ({0, ob });

            if (!sizeof(a))
                m_delete(name_living_m, old);
            else if (sizeof(a) == 1)
                name_living_m[old] = a[0];
            else
                name_living_m[old] = a;
        }
        else
            m_delete(name_living_m, old);
    }

    if (new)
    {
        mixed a = name_living_m[new];
        if (a)
        {
            if (!pointerp(a))
                a = ({ a, ob });
            else
                a = a - ({ 0 }) + ({ob });
            name_living_m[new] = a;
        }
        else
            name_living_m[new] = ob;
    }
}

/*
FUNKTION: set_living_name
DEKLARATION: void set_living_name(string name)
BESCHREIBUNG:
Mit dieser Funktion kann man den Namen eines Lebewesens setzen,
so dass das Lebewesen spaeter mit find_living(name) gefunden werden
kann. Der Name wird in einen Hash-Table eingetragen.
Die Funktion kann nur auf Lebewesen angewandt werden, die mit
enable_commands() als solche gekennzeichnet wurden.
In /i/monster/monster und /obj/monster wird dies bei NPCs automatisch gemacht.

Jedes Lebewesen kann nur einen Namen bekommen, der im Hash-Table
eingetragen wird.
Der Name sollte klein geschrieben sein, da es sonst zu Fehlern kommen kann.

ACHTUNG: Nicht verwenden, sondern stattdessen set_npc_name() verwenden.

VERWEISE: lfun::set_npc_name, find_living, enable_commands, find_player, query_living_name
GRUPPEN: simul_efun, monster
*/
void set_living_name(string name)
{
    string old = living_name_m[previous_object()];
    string old_uml = old && convert_umlaute(old);
    string name_uml = name && convert_umlaute(name);

    internal_set_living_name(previous_object(), old, name);
    internal_set_living_name(previous_object(), (old_uml != old) && old_uml, (name_uml != name) && name_uml);
    living_name_m[previous_object()] = name;
}

/*
FUNKTION: query_living_name
DEKLARATION: string query_living_name(object ob)
BESCHREIBUNG:
Liefert den mit 'set_living_name' gesetzten Namen.

ACHTUNG: Nicht verwenden, sondern stattdessen query_npc_name() verwenden.

VERWEISE: query_npc_name, set_living_name, find_living, find_player, find_object
GRUPPEN: simul_efun, monster
*/
string query_living_name(object ob)
{
    return living_name_m[ob];
}

/*
FUNKTION: find_living
DEKLARATION: object find_living(string name)
BESCHREIBUNG:
Diese Funktion sucht das Lebewesen mit dem Namen 'name' und liefert
dessen Objekt-Pointer. 'name' muss hierbei der Name sein,
der mit set_living_name() gesetzt murde.

Beispiel:
  object ob;
  ob = find_living("detlef");

VERWEISE: set_living_name, find_player, find_object, query_living_name
GRUPPEN: simul_efun, monster
*/
object find_living(string name)
{
    return internal_find_living(name, #'living);
}


/*
FUNKTION: player_exists
DEKLARATION: int player_exists(string name)
BESCHREIBUNG:
Schaut nach, ob es einen Spieler mit diesem Namen gibt.
VERWEISE:
GRUPPEN: simul_efun, player
*/
 
int player_exists(string name)
{
    if (stringp(name) && member(name,'.') == -1 && member(name,' ') == -1)
       	// && strlen(name) >= 3)
    {
	name = lower_case(name);
	return file_size(PLAYER_FILE(name) + ".o") > 0;
    }
}

/*
FUNKTION: query_livings
DEKLARATION: object *query_livings()
BESCHREIBUNG:
Liefert alle Objekte, die einen Living-Namen haben.
VERWEISE: set_living_name
GRUPPEN: simul_efun, monster
*/
object *query_livings()
{
    return m_indices(living_name_m);
}
