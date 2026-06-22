// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/room/death/obj/death_shadow.c
// Description:	Shadow, um einige Spielerfunktionen zu blockieren.

inherit "/i/shadow";

object shadowing;

varargs int init_shadow(object victim, int arg)
{
   shadowing = victim;
   ::init_shadow(victim, arg);
}

// Ueberlagerte Funktionen:

int revive()
{
   write("Du spürst wieder Boden unter den Füßen.\n");
   shadowing->revive();
   destruct_shadow();
   return 1;
}

int id(string str)
{
   return str == "nebel" || str == "geist" || shadowing->id(str);
}

string query_short()
{
    string desc = Ihr((["gender":"maennlich", "name":"geist"]), 0, shadowing);
    if(shadowing->query_moerder() && !shadowing->query_wiz_level())
	desc += " (M)";

    return desc;
}

string query_long(object who)
{
   object qso = query_shadow_owner();
   string lstr = qso ? qso->query_ghost_long_description() : 0;
   return lstr||"Du siehst einen dünnen Nebel.\n";
}

/*
int score()
{
   write("In Deinem vergeistigten Zustand ist das nicht moeglich.\n");
   return 1;
}

int skill()
{
   write("In Deinem vergeistigten Zustand ist das nicht moeglich.\n");
   return 1;
}
*/

int attackiere_command(string str)
{
   write("In Deinem vergeistigten Zustand ist das nicht möglich.\n");
   return 1;
}

int werfe_command(string str)
{
   write("In Deinem vergeistigten Zustand ist das nicht möglich.\n");
   return 1;
}

int schiesse_command(string str)
{
   write("In Deinem vergeistigten Zustand ist das nicht möglich.\n");
   return 1;
}

int wield_command(string str)
{
   write("In Deinem vergeistigten Zustand ist das nicht möglich.\n");
   return 1;
}

int add_hp(int i, mapping infos)
{
   return -1;
}
