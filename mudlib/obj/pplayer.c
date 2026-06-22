// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/secure/obj/pplayer.c
// Description:	Pseudo-Player-Objekt
// Author:	Gnomi

inherit "/i/contain";
inherit "/i/player/more";
inherit "/i/player/editor";

#include <move.h>

void notify_ed_enter(int suspend_client) {}
void notify_ed_exit(int suspend_client) {}

string real_name;

string query_real_name() { return real_name; }

int remove()
{
    (all_inventory()||({}))->quit_reader();
    destruct(this_object());
    return 1;
}

void net_dead()
{
    remove();
}

void receive_message(int mt, int ma, object wer, string msg)
{
    if(msg && interactive(this_object()))
	efun::tell_object(this_object(), msg);
}


void setup_player()
{
    object ob;
    
    if(previous_object() &&
	!strstr(object_name(previous_object()),"/secure/obj/login#"))
	    real_name = previous_object()->query_real_name();
    else
    {
	remove();
	return;
    }

    enable_commands();
    set_living_name("MAIL "+real_name);

    add_action("remove", "ende");
    add_action("remove", "quit");
    
    ob = clone_object("/obj/mailreader");
    ob->move(this_object(), ([MOVE_FLAGS:MOVE_ERR_REMOVE]));
    write(wrap(ob->query_read()));
}

void mailreader_exitted()
{
    remove();
}
