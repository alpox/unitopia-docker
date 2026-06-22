// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/i/tools/move_msg.c
// Description: Bewegungsmeldungen von Objekten
// Author:	Francis, Freaky, Garthan (23.12.93)
// Modified by: Garthan	(16.10.96) das ueberfluessige " " wegbeschissen durch ^A
//		                   statt leerem dir_string (oh weia)
//		Garthan	(03.01.97) Shimmer
//		Freaky  (21.01.98) getrennt von /i/move.c
//
////////////////////////////////////////////////////////////////////////////
//
// Dokumentation siehe /doc/funktionsweisen/move
//
////////////////////////////////////////////////////////////////////////////
  

#pragma save_types
#pragma strong_types

inherit "/i/tools/message_parser";
inherit "/i/tools/move_msg_utils";

#include <move.h>
#include <room.h>
#include <invis.h>
#include <level.h>
#include <error.h>
#include <deklin.h>
#include <message.h>
#include <landschaft.h>

private        int     msg_pflicht;
private        string  msg_out, mmsg_out, msg_in, mmsg_in, msg_invis, msg_vis;
private static closure c_msg_out, c_msg_in, c_mmsg_out, c_mmsg_in,
		       c_msg_invis, c_msg_vis;


/* ================= BEWEGUNGSMELDUNGEN DES OBJEKTS ======== */

/* Standardfunktionen zum Setzen und Abfragen von Bewegungsmeldungen */
 
/*
FUNKTION: set_msg_in
DEKLARATION: void set_msg_in(string msg_in)
BESCHREIBUNG:
Setzt die Bewegungsmeldung, die beim Betreten eines Raumes erscheint. Bei
Spielern und Monstern ist per Default "$Ein() naehert sich $dir()." gesetzt.
Das "\n" wird automatisch angehaengt.
VERWEISE: set_msg_out, set_mmsg_in, set_mmsg_out, query_msg_in ...
GRUPPEN: spieler, monster, move
*/
void set_msg_in(string str)
{
   c_msg_in = str ? message_parser(add_dot_to_msg(str)) : 0;
   msg_in = str;
}

/*
FUNKTION: set_msg_out
DEKLARATION: void set_msg_out(string msg_out)
BESCHREIBUNG:
Setzt die Bewegungsmeldung, die beim Verlassen eines Raumes erscheint. Bei
Spielern und Monstern ist per Default "$Der() entfernt sich $dir()." gesetzt.
Das "\n" wird automatisch angehaengt.
VERWEISE: set_msg_in, set_mmsg_in, set_mmsg_out, query_msg_out ...
GRUPPEN: spieler, monster, move
*/
void set_msg_out(string str)
{
   c_msg_out = str ? message_parser(add_dot_to_msg(str)) : 0;
   msg_out = str;
}

/*
FUNKTION: set_mmsg_in
DEKLARATION: void set_mmsg_in(string mmsg_in)
BESCHREIBUNG:
Setzt die Bewegungsmeldung, die beim magischen Betreten eines Raumes 
erscheint. Bei Spielern und Monstern ist per Default "$Ein() erscheint in 
einer Rauchwolke." gesetzt. Das "\n" wird automatisch angehaengt.
VERWEISE: set_msg_out, set_msg_out, set_mmsg_out, query_mmsg_in ...
GRUPPEN: spieler, monster, move
*/
void set_mmsg_in(string str)
{
   c_mmsg_in = str ? message_parser(add_dot_to_msg(str)) : 0;
   mmsg_in = str;
}

/*
FUNKTION: set_mmsg_out
DEKLARATION: void set_mmsg_out(string str)
BESCHREIBUNG:
Setzt die Bewegungsmeldung, die beim magischen Verlassen eines Raumes
erscheint. Bei Spielern und Monstern ist per Default "$Der() verschwindet in 
einer Rauchwolke." gesetzt. Das "\n" wird automatisch angehaengt.
VERWEISE: set_msg_in, set_msg_out, set_mmsg_out, query_mmsg_out ...
GRUPPEN: spieler, monster, move
*/
void set_mmsg_out(string str)
{
   c_mmsg_out = str ? message_parser(add_dot_to_msg(str)) : 0;
   mmsg_out = str;
}

/*
FUNKTION: set_msg_invis
DEKLARATION: void set_msg_invis(string msg_invis)
BESCHREIBUNG:
Setzt die Meldung, die beim Unsichtbarwerden erscheint.
Ist keine gesetzt, so soll die mmsg_out - Meldung erscheinen.
Das "\n" wird automatisch angehaengt.
VERWEISE: set_msg_out, set_mmsg_out, set_msg_vis, query_msg_invis ...
GRUPPEN: spieler, monster, move
*/
void set_msg_invis(string str)
{
   c_msg_invis = str ? message_parser(add_dot_to_msg(str)) : 0;
   msg_invis = str;
}

/*
FUNKTION: set_msg_vis
DEKLARATION: void set_msg_vis(string msg_vis)
BESCHREIBUNG:
Setzt die Meldung, die beim Sichtbarwerden erscheint.
Ist keine gesetzt, so soll die mmsg_in - Meldung erscheinen.
Das "\n" wird automatisch angehaengt.
VERWEISE: set_msg_in, set_msg_invis, set_mmsg_in, query_msg_vis ...
GRUPPEN: spieler, monster, move
*/
void set_msg_vis(string str)
{
   c_msg_vis = str ? message_parser(add_dot_to_msg(str)) : 0;
   msg_vis = str;
}

/*
FUNKTION: query_msg_in
DEKLARATION: string query_msg_in()
BESCHREIBUNG:
Liefert die Bewegungsmeldung, die mit set_msg_in() gesetzt wurde.  
VERWEISE: query_msg_out, query_mmsg_in, query_mmsg_out, set_msg_in ...
GRUPPEN: spieler, monster, move
*/
string query_msg_in() { return msg_in; } 

/*
FUNKTION: query_msg_out
DEKLARATION: string query_msg_out()
BESCHREIBUNG:
Liefert die Bewegungsmeldung, die mit set_msg_out() gesetzt wurde.
VERWEISE: query_msg_in, query_mmsg_in, query_mmsg_out, set_msg_out ...
GRUPPEN: spieler, monster, move
*/
string query_msg_out() { return msg_out; }                        

/*
FUNKTION: query_mmsg_in
DEKLARATION: string query_mmsg_in()
BESCHREIBUNG:
Liefert die Bewegungsmeldung, die mit set_mmsg_in() gesetzt wurde.
VERWEISE: query_msg_in, query_msg_out, query_mmsg_out, set_mmsg_in ...
GRUPPEN: spieler, monster, move
*/
string query_mmsg_in() { return mmsg_in; }

/*
FUNKTION: query_mmsg_out
DEKLARATION: string query_mmsg_out()
BESCHREIBUNG:
Liefert die Bewegungsmeldung, die mit set_mmsg_out() gesetzt wurde.
VERWEISE: query_msg_in, query_msg_out, query_mmsg_in, set_mmsg_out ...
GRUPPEN: spieler, monster, move
*/
string query_mmsg_out() { return mmsg_out; }                        

/*
FUNKTION: query_msg_invis
DEKLARATION: string query_msg_invis()
BESCHREIBUNG:
Liefert die Meldung, die beim Unsichtbarwerden kommt.
VERWEISE: query_msg_vis, query_msg_in, query_msg_out, set_msg_invis ...
GRUPPEN: spieler, monster, move
*/
string query_msg_invis() { return msg_invis; } 

/*
FUNKTION: query_msg_vis
DEKLARATION: string query_msg_vis()
BESCHREIBUNG:
Liefert die Meldung, die beim Sichtbarwerden erscheint.
VERWEISE: query_msg_invis, query_msg_in, query_msg_out, set_msg_vis ...
GRUPPEN: spieler, monster, move
*/
string query_msg_vis() { return msg_vis; }                        

varargs void set_msg(string raus, string rein, int noetig)
{
   if(raus != "")
      set_msg_out(raus);
   if(rein != "")
      set_msg_in(rein);
   msg_pflicht = noetig ? 1 : 0;
}

void set_mmsg(string raus, string rein)
{
   if(raus != "")
      set_mmsg_out(raus);
   if(rein != "")
      set_mmsg_in(rein);
}


   /* Liefern expandierte Bewegungsmeldungen */

varargs string query_exp_msg_in(string dir)
{
   return message_expansion(c_msg_in,this_object(), dir ? dir :"");
}
varargs string query_exp_msg_out(string dir)
{
   return message_expansion(c_msg_out,this_object(), dir ? dir :"");
}
varargs string query_exp_mmsg_in(string dir)
{
   return message_expansion(c_mmsg_in,this_object(), dir ? dir :"");
}
varargs string query_exp_mmsg_out(string dir)
{
   return message_expansion(c_mmsg_out,this_object(), dir ? dir :"");
}
varargs string query_exp_msg_invis(string dir)
{
   return message_expansion(c_msg_invis,this_object(), dir ? dir :"");
}
varargs string query_exp_msg_vis(string dir)
{
   return message_expansion(c_msg_vis,this_object(), dir ? dir :"");
}

   /* Closures der Bewegungsmeldungen */

closure query_c_msg_in()    { return c_msg_in; }
closure query_c_msg_out()   { return c_msg_out; }
closure query_c_mmsg_in()   { return c_mmsg_in; }
closure query_c_mmsg_out()  { return c_mmsg_out; }
closure query_c_msg_invis() { return c_msg_invis; }
closure query_c_msg_vis()   { return c_msg_vis; }


/* ================ MOVE UTILITY FUNCTIONS ============= */

static int count_shimmer(object room)
{
   int i, invis, res;
   object *obs;

   for(i = sizeof(obs = all_inventory(room)); i--;)
      if((invis = obs[i]->query_invis()) & V_ATOM_INVIS &&
	 !(invis & V_ATOM_NOSHIMMER))
	 res++;
   return res;
}

//---------------------------- neue move_msg --------------------------------
closure move_msg_parser(mapping mv_infos, string msg, int adddot = 0)
{
    mapping args = mv_infos[MOVE_MSG_ARGS] || ([]);
    if (!msg)
        return 0;

    if (adddot)
        msg = add_dot_to_msg(msg);
    return lambda(({'tp, 'richtung}) + m_indices(args), string_parser(msg, 0, 1));
}

string move_msg_expansion(mapping mv_infos, closure cl, string dir)
{
    mapping args = mv_infos[MOVE_MSG_ARGS] || ([]);
    return wrap(message_expansion(cl, this_object(), dir, m_values(args)...));
}

string query_move_msg_self(mapping mv_infos)
{
    string dir_str, tmp, dir = mv_infos[MOVE_DIRECTION];
    mixed* exit_info = mv_infos[MOVE_EXIT_INFO];
    closure rule;

    // Wir machen das nur, wenn explizit eine Meldung gesetzt wurde.
    if (member(mv_infos,MOVE_MSG_SELF))
    {
        rule = move_msg_parser(mv_infos, mv_infos[MOVE_MSG_SELF]);
    }
    else if (sizeof(exit_info)<= EI_C_EXIT_MSG_SELF 
        || !exit_info[EI_C_EXIT_MSG_SELF] )
    {
        return 0;
    }
    else
    {
        rule = exit_info[EI_C_EXIT_MSG_SELF];
    }

    /* DIRECTION PARSER */

    if (exit_info[EI_DIR_MSG_OUT])
        dir_str = exit_info[EI_DIR_MSG_OUT];
    else if (dir && exit_info[EI_DIR_PREP_OUT] &&
            (tmp = direction_string(exit_info[EI_DIR_PREP_OUT],dir,0)))
        dir_str = tmp;
    else if (dir && (tmp = direction_string("nach", dir, 0)))
        dir_str = tmp;
    else
        dir_str = "\1";

    /* EXEC */

    return move_msg_expansion(mv_infos, rule, dir_str);
}

private string get_dir_in(string dir, mixed * exit_info)
{
    string tmp;
    if (exit_info[EI_EXIT_FLAG] & EXIT_ATOM_HIDDEN)
	    return "\1";
	else
	{
	    if (exit_info[EI_DIR_MSG_IN])
		    return exit_info[EI_DIR_MSG_IN];
	    else if (dir && exit_info[EI_DIR_PREP_IN] &&
		       (tmp = direction_string(exit_info[EI_DIR_PREP_IN], dir, 1)))
            return tmp;
	    else if (dir && (tmp = direction_string("von", dir, 1)))
            return tmp;
	    else
            return "\1";
	}
}

private string get_dir_out(string dir, mixed * exit_info)
{
    string tmp;
	if (exit_info[EI_EXIT_FLAG] & EXIT_ATOM_HIDDEN)
	    return "\1";
	else
	{
	    if (exit_info[EI_DIR_MSG_OUT])
            return exit_info[EI_DIR_MSG_OUT];
	    else if (dir && exit_info[EI_DIR_PREP_OUT] &&
		      (tmp = direction_string(exit_info[EI_DIR_PREP_OUT],dir,0)))
            return tmp;
	    else if (dir && (tmp = direction_string("nach", dir, 0)))
            return tmp;
	    else
            return "\1";
	}
}

string query_move_out_msg(mapping mv_infos)
{
    string dir_str;
    int invis,way;
    closure rule;
    object old_room = mv_infos[MOVE_OLD_ROOM];

    way = mv_infos[MOVE_FLAGS];
    invis = this_object()->query_invis();
    if (invis & V_ATOM_INVIS)
        if (invis & V_ATOM_NOSHIMMER)
            return 0; // no_msg
        else // shimmer
        {
            if (count_shimmer(old_room) == 1)
                return (old_room->query_type(LANDSCHAFT)&L_UNTERWASSER)?
                       "Das Wasser um dich herum hört auf zu schimmern.\n":
                       "Die Luft um dich herum hört auf zu flimmern.\n";
            return 0; // no_msg
        }
    else // normal msg
    {
        /* DIRECTION PARSER */
        dir_str = get_dir_out(
            mv_infos[MOVE_DIRECTION],
            mv_infos[MOVE_EXIT_INFO]);

        /* RULE PARSER */

        if (mv_infos[MOVE_MSG_LEAVE])
            rule = move_msg_parser(mv_infos, mv_infos[MOVE_MSG_LEAVE], 1);
        else if ((way & MOVE_ATOM_MAGIC) && c_mmsg_out)
            rule = c_mmsg_out;
        else if (msg_pflicht && c_msg_out)
            rule = c_msg_out;
        else if (mv_infos[MOVE_EXIT_INFO][EI_C_EXIT_MSG_OUT])
            rule = mv_infos[MOVE_EXIT_INFO][EI_C_EXIT_MSG_OUT];
        else if (c_msg_out)
            rule = c_msg_out;
        else
            rule = message_parser("$Der() entfernt sich $dir().");


        /* EXEC */

        return move_msg_expansion(mv_infos, rule, dir_str);
    }
}

string query_move_in_msg(mapping mv_infos)
{
    string dir_str;
    int invis,way;
    closure rule;
    object new_room = mv_infos[MOVE_NEW_ROOM];

    way = mv_infos[MOVE_FLAGS];
    invis = this_object()->query_invis();
    if (invis & V_ATOM_INVIS)
        if (invis & V_ATOM_NOSHIMMER)
            return 0; // no_msg
        else // shimmer
        {
            if (count_shimmer(new_room) == 1)
                return (new_room->query_type(LANDSCHAFT)&L_UNTERWASSER)?
                           "Das Wasser um dich herum beginnt zu schimmern.\n":
                           "Die Luft um dich herum beginnt zu flimmern.\n";
        }
    else // normal msg
    {
        /* DIRECTION PARSER */
        dir_str = get_dir_in(
            mv_infos[MOVE_DIRECTION],
            mv_infos[MOVE_EXIT_INFO]);

        /* RULE PARSER */

        if (mv_infos[MOVE_MSG_ENTER])
            rule = move_msg_parser(mv_infos, mv_infos[MOVE_MSG_ENTER], 1);
        else if ((way & MOVE_ATOM_MAGIC) && c_mmsg_in)
            rule = c_mmsg_in;
        else if (msg_pflicht && c_msg_in)
            rule = c_msg_in;
        else if (mv_infos[MOVE_EXIT_INFO][EI_C_EXIT_MSG_IN])
            rule = mv_infos[MOVE_EXIT_INFO][EI_C_EXIT_MSG_IN];
        else if (c_msg_in)
            rule = c_msg_in;
        else
            rule = message_parser("$Ein() nähert sich $dir().");

        /* EXEC */

        return move_msg_expansion(mv_infos, rule, dir_str);
    }
}

void send_move_out_msg(mapping mv_infos)
{
    string msg,dir_str;
    object * all_whom = ({}),who;
    <object|object*> whom;
    closure rule;
    int ix,size;
    mixed * excl = mv_infos[MOVE_MSG_LEAVE_OTHERS];
    if (size=sizeof(excl))
    {
        dir_str = get_dir_out(
            mv_infos[MOVE_DIRECTION],
            mv_infos[MOVE_EXIT_INFO]);
        for (ix=0;ix<size;ix+=2)
        {
            whom  = excl[ix];
            if (ix+1 >= size) 
            {
                rule = message_parser("$");
            }
            else
            {
                rule = move_msg_parser(mv_infos, excl[ix+1]);
            }
            if (objectp(whom))
                whom = ({ whom });
            else if (!pointerp(whom))
                continue;
            foreach (who : whom)
            {
                msg = move_msg_expansion(mv_infos, rule, dir_str);
                who->send_message_to(who,MT_LOOK|MT_NOISE,MA_MOVE_OUT,msg);
            }
            all_whom += whom;
        }
    }

    msg = this_object()->query_move_out_msg(mv_infos);
    if (msg)
        this_object()->send_message(MT_LOOK|MT_NOISE,MA_MOVE_OUT,
            msg,"",all_whom);
}

void send_move_in_msg(mapping mv_infos)
{
    string msg,dir_str;
    object * all_whom = ({}),who;
    <object|object*> whom;
    closure rule;
    int ix,size;
    mixed * excl = mv_infos[MOVE_MSG_ENTER_OTHERS];
    if (size=sizeof(excl))
    {
        dir_str = get_dir_in(
            mv_infos[MOVE_DIRECTION],
            mv_infos[MOVE_EXIT_INFO]);
        for (ix=0;ix<size;ix+=2)
        {
            whom  = excl[ix];
            if (ix+1 >= size) 
            {
                rule = message_parser("$");
            }
            else
            {
                rule = move_msg_parser(mv_infos, excl[ix+1]);
            }
            if (objectp(whom))
                whom = ({ whom });
            else if (!pointerp(whom))
                continue;
            foreach (who : whom)
            {
                msg = move_msg_expansion(mv_infos, rule, dir_str);
                who->send_message_to(who,MT_LOOK|MT_NOISE,MA_MOVE_IN,msg);
            }
            all_whom += whom;
        }
    }

    msg = this_object()->query_move_in_msg(mv_infos);
    if (msg)
        this_object()->send_message(MT_LOOK|MT_NOISE,MA_MOVE_IN,
            msg,"",all_whom);
}

void send_move_msg_self(mapping mv_infos)
{
    string msg;
    
    msg = this_object()->query_move_msg_self(mv_infos);
    if (msg)
        this_object()->send_message_to(this_object(),
            MT_LOOK | MT_FEEL | MT_NOTIFY, MA_MOVE,
            msg);
}
