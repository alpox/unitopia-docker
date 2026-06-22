// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/room/rathaus/div/wegweiser.c
// Description: Wegweiser fuer Goetter.

inherit "/i/monster/monster";

#include <monster.h>

#include "rollen_weg.inc"

int query_prevent_shadow(object shadow) { return 1; }

int nein_danke(object ob)
{
    if (!present(ob,this_object()) || ob->id("seele"))
        return 1;
    do_command("sage Nein danke. Das will ich nicht.");
    return 0;
}

static void sammel_stichworte(string * commands)
{
    while (sizeof(commands) && get_eval_cost() > 200000)
    {
        string path = commands[0];
        commands = commands[1..];
        if (strstr(path,"/room/rathaus/")!=0)
            continue; // Das keiner Raeume dazu erfindet...
        object room = find_object(path) || touch(path);
        if (!objectp(room))
            continue; // wenn er nicht laedt, schade.
        mixed * regeln = room->query_keyword_rules();
        if (!regeln)
            continue; // hier fehlen noch Stichworte...
        add_parse_conversation(room,regeln);
    }
    if (sizeof(commands))
        call_out("sammel_stichworte",0,commands);
    else
        add_parse_conversation(this_object(),({
"bank: bank || schließfach || schließfächer || [armafest] "
    "|| [einlager]", PARSE_SAY,
"p_halle: p || halle", PARSE_SAY,
"kurs: [kurs] || [beispiel]", PARSE_SAY,
"levitanis: levitanis", PARSE_SAY,
"stichwort_neu: stichwort", PARSE_SAY|PARSE_RE_TRADITIONAL //sonst
        }));
}

void create()
{
    monster::create();
    clone_object("/obj/soul")->move(this_object());
    initialize("mensch",100);
    set_name("nessaja");
    set_npc_name("nessaja");
    set_id(({"nessaja","wegweiser","wegweisende"}));
    set_short("Nessaja, die Wegweisende");
    set_long("Nessaja lernt momentan die Raumstruktur hier im Forum "
        "und sammelt Stichworte, nach denen man sie dann fragen kann.");
    set_personal(1);
    set_gender("weiblich");
    set_align(1000);
    
    // reset();
    set_accept_objects(({
             #'accept_from_void,
             #'accept_invis,
             #'nein_danke,
             #'refuse}));
    set_parse_conversation(this_object(),({
"wegweiser: wegweiser", PARSE_SAY,
"gruss: hallo && nessaja", PARSE_SAY,
    }));
    call_out("sammel_stichworte",0,
        "/room/rathaus/forum"->query_exit_list());
}

mapping gegruesst = ([:0]);

int gruss(string str, string verb, object monster, object player,
    int flags)
{
    gegruesst = filter(gegruesst, (: present($1, environment()) :));
    if(member(gegruesst, this_player()))
        return 0;
    else
        m_add(gegruesst, this_player());
    do_command("sage Willkommen, "+this_player()->query_cap_name()+".");
}

int p_halle(string str, string verb, object monster, object player,
    int flags)
{
    do_command("sage Das 'P' enthält Standardobjekte als Erweiterung "
        "zur Mudlib. Zur P-Halle geht es mit dem Ausgang 'halle'.");
}

int wegweiser(string str, string verb, object monster, object player,
    int flags)
{
    do_command("sage Ich soll den Weg zu den verschiedenen "
        "Räumen hier weisen, lerne aber noch die Stichworte.");
}

int stichwort_neu(string str, string verb, object monster, object player,
    int flags)
{
    do_command("sage Bitte neue Stichworte im entsprechenden Raum "
        "als Idee absetzen.");
}

int kurs(string str, string verb, object monster, object player,
    int flags)
{
    do_command("sage Der Kurs beinhaltet Beispiele zum Programmieren "
        "für Anfänger, Fortgeschrittene und für Soundprogrammierer.");
}

int levitanis(string str, string verb, object monster, object player,
    int flags)
{
    do_command("sage Levitanis ist ein Bereich, wo Götter gestalten "
        "und Spieler und Götter sich treffen können.");
}

int bank(string str, string verb, object monster, object player,
    int flags)
{
    do_command("sage In der Bank kannst du Objekte zum armafesten "
        "Einlagern registrieren oder Informationen über solche "
        "Objekte erfahren.");
}

