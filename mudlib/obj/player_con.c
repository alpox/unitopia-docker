// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/obj/player_con.c
// Description: Container, der beim Ausloggen eines Spielers die
//              Dinge aufnimmt, die der Spieler bei sich traegt.
//		Dieser wird von /secure/player_container/virtual_compiler
//		gecloned und bekommt den Namen des Spielers.
// Author:	Sissi (02.02.1999)
// Modified by:	Freaky (08.02.1999) remove(), query_prevent_shadow() eingebaut.
//		Freaky (22.02.1999) clean_up() eingebaut.

#include <move.h>

mapping states = ([:1]);

void create()
{
}

string query_name() {
  return "nirvana";
}

string query_gender() {
  return "saechlich";
}

int query_container()
{
    return 1;
}

/*
FUNKTION: query_player_container
DEKLARATION: int query_player_container()
BESCHREIBUNG:
Wenn diese Funktion 1 returned, ist das Objekt ein Player-Container, also
der Behaelter, der die Sachen eines Spielers aufbewahrt, wenn sich dieser
ausloggt.
GRUPPEN: spieler
*/
int query_player_container()
{
    return 1;
}

void set_object_state(object obj, mixed state)
{
    if(obj && playerp(previous_object()) && present(obj, this_object()))
	states[obj] = state;
}

mixed query_object_state(object obj)
{
    if(playerp(previous_object()))
	return states[obj];
}

<int|string> let_not_out(mapping mv_infos)
{
    return (!playerp(mv_infos[MOVE_NEW_ROOM])) 
        || (previous_object(1) != mv_infos[MOVE_NEW_ROOM]);
}

<int|string> let_not_in(mapping mv_infos)
{
    if (!mv_infos[MOVE_OLD_ROOM])
        return !__MASTER_OBJECT__->mudlib_privilege_violation(
            "player_con", mv_infos[MOVE_OBJECT]);
    return (!playerp(mv_infos[MOVE_OLD_ROOM])) 
        || (previous_object(1) != mv_infos[MOVE_OLD_ROOM]);
}

int add_encumbrance(object ob, int enc_type, int enc_diff)
{
    return 1;
}

int query_prevent_shadow(object shadow)
{
    return 1;
}

int remove()
{
    destruct(this_object());
    return 1;
}

// Wenn der Container leer ist, kann er auch zerstoert werden.
int clean_up(int ref)
{
    return 0; // Myonara: wird fuer Armageddon-J/N-Erkennung gebraucht.
    if (first_inventory())
	return 1;

    // Wenn es /obj/player_con.c ist, nicht zerstoeren
    if (__FILE__ == (object_name() + ".c"))
	return 0;

    remove();
    return 0;
}
