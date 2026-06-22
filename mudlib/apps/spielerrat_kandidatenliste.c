// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/apps/spielerrat_kandidatenliste.c
// Description: Liste, auf der sich Spieler, die in den Spielerrat gewaehlt
//              werden wollen, eintragen koennen.
//
// UID: Apps

static variables inherit "/i/install";
static variables inherit "/i/item";

#define SAVEFILE "/var/spielerrat_kandidatenliste"
#define HINWEISSCHILD "/room/rathaus/obj/hinweis_srwahl"
#define PO_TREPPE "/room/rathaus/treppe.c"

#include <time.h>
#include <level.h>
#include <more.h>
#include <uids.h>

private int jahr;
private string *kandidaten;
static string name;

// ([string realname:string text])
private mapping wahlplakate = ([]);

int get_phase()
{
    int* now;
    
    now = timearray(time());
    if (now[TM_MON]==11 && now[TM_MDAY]>=15)
        return 1;
    if (now[TM_MON]==12)
        return 2;
    if (now[TM_MON]==1)
        return 3;
    return 0;
}

object get_hinweisschild()
{
    if (get_phase()==0)
        return 0;
    return clone_object(HINWEISSCHILD);
}

int darf_waehlen(int msg)
{
    if (!playerp (this_player())) {
        if (msg) write ("Nur Spieler können gewählt werden.\n");
        return 0;
    }
    if (this_player() != this_interactive()) {
        if (msg) write ("Na, so geht das aber nicht.\n");
        return 0;
    }
    name = this_player()->query_real_name();
    if (wizp (this_player())) {
        if (msg) write ("Götter haben im Spielerrat nix verlorn.\n");
        return 0;
    }
    if (testplayerp (this_player())) {
        if (msg) write ("Zweities im Spielerrat? Ach nö.\n");
        return 0;
    }
    if (guestp (this_player())) {
        if (msg) write ("Gäste im Spielerrat? Witzbold.\n");
        return 0;
    }
/*
    if (this_player()->query_age() < 432000) {
        if (msg) write ("Du bist zu jung.\n");
        return 0;
    }
    if (this_player()->query_quest_count() < 3) {
        if (msg) write ("Du hast ja noch nicht mal 3 Raetsel geloest!\n");
        return 0;
    }
*/
    return 1;
}

void checkliste()
{
    int currentyear;
    int* currenttime;
    
    currenttime = timearray(time());
    currentyear = currenttime[TM_YEAR];
    if(currenttime[TM_MON]<7)
	currentyear--;

    if(jahr!=currentyear)
    {
	kandidaten = ({});
	wahlplakate = ([]);
	jahr = currentyear;
	
	save_object (SAVEFILE);
    }
}

string query_long (object who)
{
    checkliste();

    who->more (explode (
     "Hier können sich alle Spieler eintragen, die im Falle, dass sie für "
     "den \nSpielerrat gewählt werden würden, die Wahl annehmen würden.\n"
     "Wenn Du Dich ebenfalls in dieser Liste eintragen willst, so tu dies\n"
     "mit \"trage mich in liste ein\".\n" +
     (kandidaten && sizeof (kandidaten) ?
     ("Es haben sich bereits "+sizeof(kandidaten)+" Spieler eingetragen:\n\n"
     +wrap (implode(kandidaten,", ")+"."))
     :"Es hat sich noch niemand eingetragen, du könntest derdiedas Erste "
     "sein."),"\n"),
     0, 0, M_AUTO_END);
    return "";
}

varargs string query_read (string parse_rest, string str)
{
    return query_long(this_player());
}

     
void create ()
{
    if (clonep (this_object())) {
        remove (); return;
    }
    restore_object (SAVEFILE);
    if (!kandidaten) kandidaten = ({});
    if (!wahlplakate) wahlplakate = ([]);
    set_name ("liste");
    set_gender ("weiblich");
    set_id (({"liste","kandidaten"}));
    set_short ("Eine Liste der Spieler, die bereit sind, im Spielerrat mitzuwirken");
}

void init ()
{
    add_action("trag","trag",1);
}


int trag(string s)
{
    notify_fail ("trage mich in liste ein?\n");
    if (!s || s == "") return 0;
    s = lower_case (s);
    if (s != "mich in liste ein") return 0;
    checkliste();
    if (!darf_waehlen(1)) return 1;
    name = capitalize (name);
    if (member (kandidaten,name) != -1)
    {
        write ("Du stehst doch bereits in der Liste.\n");
        return 1;
    }
    kandidaten += ({name});
    save_object (SAVEFILE);
    write ("Du hast Dich in die Liste eingetragen.\n");
    say (wrap (Der(this_player())+" hat sich in die Liste eingetragen."));
    return 1;
}

// Fuer die Urne
string* query_kandidaten()
{
    if(strstr(object_name(previous_object()), "/room/wahlen/"))
	return 0;

    checkliste();
	
    return kandidaten;
}
int ist_kandidat(string name)
{
    string kname = capitalize(lower_case(name||""));
    return (get_phase()>0) && (member(kandidaten,kname)>-1);
}

// fuer die Wahlplakate:

mapping query_wahlplakate()
{
    // TODO Absichern.
    if (program_name(previous_object())!=PO_TREPPE) return ([]);
    checkliste();
	
    return copy(wahlplakate);
}

string *query_wahlplakat_namen()
{
    // TODO absichern
    if (program_name(previous_object())!=PO_TREPPE) return ({});
    if (get_phase()==0) return ({});
    return sort_array(m_indices(wahlplakate),#'>);
}

string *query_one_plakat(string name)
{
    // TODO absichern
    if (program_name(previous_object())!=PO_TREPPE) return ({});
    if (!member(wahlplakate,name))
        return ({});
    string * lines = ({"*** Plakat von "+name+" ***"});
    lines += explode(wahlplakate[name],"\n");
    return lines;
}

string *query_my_plakat(string name)
{
    // TODO absichern
    if (program_name(previous_object())!=PO_TREPPE) return 0;
    if (!member(wahlplakate,name))
        return 0;
    return explode(wahlplakate[name],"\n");
}

string update_my_plakat(string cname,string *lines)
{
    // TODO absichern
    if (program_name(previous_object())!=PO_TREPPE) return "Kein Zugriff.";
    string kname = capitalize(lower_case(cname||""));
    if (member(kandidaten,kname)==-1)
    {
        return "Nur Kandidaten auf der Kandidatenliste "
               "dürfen Wahlplakate erstellen.";
    }
    if (get_phase()<1)
    {
        return "Die Wahl ist derzeit nicht aktiv.";
    }
    if (get_phase()>=3)
    {
        return "Die Wahl ist abgeschlossen, "
               "das Plakat kann nicht mehr geändert werden.";
    }
    if (cname && sizeof(lines))
    {
        // TODO 20 Zeilen begrenzung.
        wahlplakate = filter(wahlplakate, 
            (: lower_case($1)!= $3 :), lower_case(cname));
        wahlplakate[cname] = implode(lines,"\n");
        save_object (SAVEFILE);
        return "Wahlplakat aktualisiert.";
    }
    else if (cname && pointerp(lines))
    {
        m_delete(wahlplakate,cname);
        save_object (SAVEFILE);
        return "Wahlplakat gelöscht.";
    }
    return "Wahlplakat bleibt unverändert.";
}
