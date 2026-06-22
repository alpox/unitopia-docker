// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/apps/spielerrat.c
// Description: Verwaltung des Spielerrates
// Author:	Sissi

// UID: Apps

#include <level.h>
#include <more.h>
#include <uids.h>

private string *members;

#define SAVE_FILE "/var/spielerrat"
#define HISTORY_FILE "/var/spielerrat_history"

#define SECURE if (!secure()) return;
                                                            
int secure()
{
    return this_player() &&
        this_player()==this_interactive() &&
            previous_object() &&
            adminp(this_player()) &&
            (geteuid(previous_object())==geteuid(this_player())
            || (object_name(previous_object()) == "/room/rathaus/register"));
}

void new_member (string s)
{
    SECURE;
    if (!player_exists (s)) {
        write ("Einen Spieler dieses Namens gibts nicht.\n");
        return;
    }
    s = lower_case (s);
    if (!members) members = ({s});
    else if (member(members,s) != -1) {
        write (capitalize(s)+" ist bereits Mitglied des Spielerrates.\n");
        return;
    }
    members += ({s});    
    save_object (SAVE_FILE);
    write_file (HISTORY_FILE,
        "Am "+shorttimestr(time())+" wurde "+capitalize(s)+
        " in den Spielerrat aufgenommen.\n");
    write ("Du hast "+capitalize(s)+" in den Spielerrat aufgenommen.\n");
}

void remove_member (string s)
{
    SECURE;
    if (!stringp (s)) {
        write ("Bitte den Namen eines Spielers angeben.\n");
        return;
    }
    s = lower_case (s);
    if (!members || (member(members,s) == -1)) {
        write (capitalize(s)+" ist gar nicht Mitglied des Spielerrates.\n");
        return;
    }
    members -= ({s});    
    save_object (SAVE_FILE);
    write_file (HISTORY_FILE,
        "Am "+shorttimestr(time())+" schied "+capitalize(s)+
        " aus dem Spielerrat aus.\n");
    write ("Du hast "+capitalize(s)+" aus dem Spielerrat entfernt.\n");
}

int is_spielerrat (mixed s)
{
    if (playerp (s))
        s = s->query_real_name();
    else if (stringp (s))
        s = lower_case (s);
    else return 0;
    return members && (member(members,s) != -1);
}

string *query_spielerrat ()
// fuer Finger, Mail - Alias, ...
{
    return members;
}

void create()
{
   restore_object(SAVE_FILE);
}
