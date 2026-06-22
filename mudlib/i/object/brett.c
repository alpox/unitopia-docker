// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/i/object/brett.c
// Description: Ein Newsbrett
// Author:	Freaky (23.12.93)

inherit "/i/item";
inherit "/i/install";

#include <error.h>
#include <level.h>
#include <move.h>
#include <news.h>

string brett_name, gruppen_name;
mixed fail_msg;
string *bretter; // Enthaelt die Namen der Bretter, die JEDER
                 // mit diesem Brett lesen kann. Standard: 0

string *set_bretter(string *strs)
{
   if(sizeof(strs))
      brett_name = gruppen_name = 0;
   return bretter = strs;
}

string *query_bretter()
{
    return bretter;
}


varargs string query_read(string rest, string str, object betrachter)
{
    object ob;

    if (!playerp(this_player()))
	return "An "+den()+" sind viele Nachrichten geheftet, die du aber "
		"alle nicht lesen kannst.\n";
    if (query_input_pending (this_player()) || query_editing (this_player()))
        return "Jetzt geht das nicht. Eines nach dem anderen.\n";
    ob = present("newsreader",this_player());
    if (brett_name && !NEWSD->valid_read(this_player(), brett_name, gruppen_name))
        return stringp(fail_msg)?fail_msg
     : closurep(fail_msg)?funcall(fail_msg, this_player(), brett_name, gruppen_name)
     : ("Du darfst das Brett "+(gruppen_name||brett_name)+" nicht lesen.\n");
    if (!ob) 
    {
        ob = clone_object("/obj/newsreader");
        if (ob->move(this_player(),([MOVE_FLAGS:MOVE_FORCE|MOVE_ERR_REMOVE]))
                    !=MOVE_OK)
            return wrap("Die Buchstaben verschwimmen vor Deinen Augen.");
    }
    ob->set_brett_name(brett_name, gruppen_name, fail_msg);
    ob->set_bretter(bretter);
    return ob->query_read(rest, str, betrachter);
}

varargs void set_brett_name(string str, string grp, mixed fail)
{
   brett_name = str;
   gruppen_name = grp;
   fail_msg = fail;
}

string query_long(object viewer)
{
   return query_short(viewer)+".\n"+ ::query_long(viewer);
}

string query_short(object viewer)
{
   return Der()+(brett_name || gruppen_name?
	  " '"+(gruppen_name? gruppen_name: brett_name)+"'":"");
}

void create() {
    seteuid(getuid());
    set_id(({"brett","newsbrett"}));
    set_adjektiv(({"schwarz"}));
    set_name("brett");
    set_gender("saechlich");
    set_brett_name("Spieler");
    set_material("holz");
    set_long(
    "Man kann "+ihn()+" lesen. Viele interessante Nachrichten sind an "+ihm()+"\n"+
    "befestigt und warten darauf, von dir durchstöbert zu werden.\n"+
    "Falls du was an "+den()+" heften möchtest, lies "+ihn()+" erst mal...\n"
    );
}

