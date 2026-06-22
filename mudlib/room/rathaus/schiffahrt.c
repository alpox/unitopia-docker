// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /room/rathaus/schiffahrt.c
// Description: Startet und verwaltet alle Faehren
// Author:        Francis
// Modified by:        Freaky (18.10.95) Faehren bekommen ein set_preven_cleanup()

#pragma strong_types

inherit "/i/room";

#include <misc.h>
#include <move.h>
#include <monster.h>
#include <apps.h>
#include <filed.h>

/*
 * Beim Laden ALLER Schiffe im Preloading gibts Probleme,
 * wenn zu viele call-outs aufeinanderlaufen:
 * Der Driver schmiert einfach ab.
 *
 * Mit den folgenden Parametern kann man den Ladevorgang zeitlich
 * etwas dehnen. 
 */
#define FAEHRE_P_FLAGS 0
#define FAEHRE_F_TEST  1

#define OFFSET 20              // Bis zum ersten Schiff
#define DIST   3               // Zwischen den einzelnen Schiffen

string *faehren;

static object starte_faehre(string name) {
    object schiff;
    string err, room;
    int ret;

    if (name && name != "") {
        if ((schiff = find_object(name)) && environment(schiff))
            return schiff;
        if (!schiff) err = catch(schiff = touch(name));
        if (!schiff) {
            write("Fehler beim Laden des Schiffes "+name+":\n"+err+"\n");
            return 0;
            }
        room = schiff->query_default_room();
        if (!room || room == "")
            room = "/map/m8_0";

        if ((ret = schiff->move(room)) != MOVE_OK) {
            write("Fehler beim Moven des Schiffes "+name+" in den Raum "+room+":"+ret+".\n");
            return 0;
            }
        schiff->set_prevent_cleanup();
        return schiff;
        }
    write("Kein Schiffs-Name angegeben.\n");
    return 0;
}



private int faehren_flag(mixed *entry, int flag)
{
   return sizeof(entry[FD_PARAMETERS]) > FAEHRE_P_FLAGS &&
      entry[FD_PARAMETERS][FAEHRE_P_FLAGS] & flag;
}



varargs void starte_alle_faehren(int dist) {
    mixed *entries;
    int i;

    entries = FILED->query_entries("faehren");
    for(faehren = ({}), i = 0; i < sizeof(entries); i++)
       if(!faehren_flag(entries[i], FAEHRE_F_TEST))
          faehren += ({ entries[i][FD_FILE] });

    call_out("starte_naechste_faehre",dist,0);
}

static void starte_naechste_faehre(int num) {
    if (num<sizeof(faehren)) {
        call_out("starte_naechste_faehre",DIST,num+1);
        starte_faehre(faehren[num]);
        }
}

// Wird von /room/rathaus/filed aufgerufen, wenn die Faehreneintraege 
// geaendert wurden.
void reload(string type)
{
   if(type == "faehren")
   {
      remove_call_out("starte_naechste_faehre");
      starte_alle_faehren();
   }
}

void starte_alles() {
    starte_alle_faehren(OFFSET);
}

void reset() {
    starte_alles();
    "/room/rathaus/div/lager"->get_moebel(this_object());
    "/room/rathaus/div/lager"->get_pflanze(this_object());
}

void create() {
    set_short("Büro für Schiffahrts-Angelegenheiten");
    set_long("Dies ist das Büro für Schiffahrts-Angelegenheiten. "
        "Hier werden alle Dinge geregelt, die in irgendeinem "
        "Zusammenhang mit Schiffbau und Schiffahrt in Magyra "
        "zusammenhängen. Insbesondere werden hier die "
        "Haupt-Verbindungen zwischen den Inseln und Kontinenten "
        "verwaltet. "
        "Sir Francis hat hier seinen Arbeitsplatz, ist jedoch "
        "selten anzutreffen, da er ständig unterwegs ist, um "
        "liegengebliebene Schiffe wieder flottzumachen, neue "
        "Schiffslinien zu eröffnen oder um neue Inseln zu "
        "entdecken.\n"
"   Kommandos: liste [-l]                    (alle Faehren)\n"+
"              starte alle                   (alle Faehren)\n"+
"              starte <nummer>               (spezielle Faehre)\n"+
"              passagiere <nummer>           (einer Faehre)\n"+
"              versenke <nummer>             (eine Faehre)\n"
"              test <schiffs-file>           (Testflag einer Fähre aendern)");
    
    set_own_light(1);
    add_type("kunstlicht",1);
    add_type("teleport_rein_verboten", 1);
    set_exits(({"forum"}),({"forum"}));
    set_room_domain("Pantheon");
    reset();
}

void init() {
    add_action("starte_cmd","starte",-5);
    add_action("liste_cmd","liste",-4);
    add_action("versenke","versenke",-7);
    add_action("passagiere","passagiere",-5);
    add_action("test","test");
    if (!present("nauta"))
        clone_object(abs_path("obj/nauta"))->move(this_object());
}

int starte_cmd(string str) {
        int num;

    if (sizeof(faehren) <= 0) {
        write("Keine Fähren eingetragen.\n");
        return 1;
        }
    if (!str || str == "") {
        write("Starte welche Fähre ?\n");
        return 1;
        }
    if (lower_case(str) == "alle") {
        starte_alle_faehren();
        write("Ok.\n");
        say(Der(this_player())+" startet alle Fähren.\n");
        return 1;
        }
    if (sscanf(str,"%d",num)) {
        num -= 1;
        if (num > sizeof(faehren) || num < 0) {
            write("Diese Fähre gibt es nicht.\n");
            return 1;
            }
        starte_faehre(faehren[num]);
        write("Ok.\n");
        say(Der(this_player())+" startet die Fähre "+(num+1)+".\n");
        return 1;
        }
    write("Starte welche Fähre ?\n");
    return 1;
}

int liste_cmd(string str) {
    int a,i, flag_l;
    object schiff;
    string room,*line;
    mixed *entries;

    flag_l = (str=="-l");
    say(Der(this_player())+" schaut sich die Liste der Fähren an.\n");
    if (sizeof(faehren) <= 0) 
    {
        write("Es sind keine Fähren hier eingetragen.\n");
        return 1;
    }

    if (!flag_l)
    {
        write("               Fähre                   Idle/gestoppt       Ort\n");
        write(copies("-",79)+"\n");
    }
    for (a=0;a<sizeof(faehren); a++) 
    {
        string schiff_objname = faehren[a];
        int schiff_nr = a + 1;

        schiff = find_object(faehren[a]);
        if (!schiff)
        {
            printf("%-4d%-=39s nicht geladen\n", 
                schiff_nr, schiff_objname);
            continue;
        }

        int last_time = schiff->query_last_time();
        int idle_time = time()-last_time;
        int stopped = schiff->query_stopped();

        object env = environment(schiff);
        string schiff_env = env ? object_name(env) : "Nirwana";

        if (flag_l)
        {
            object kapi = schiff->query_commander();
            string schiff_kapi = "Kein Kapitän.";
            string schiff_route = "";
            if (kapi)
            {
                schiff_kapi = Name(kapi);
                schiff_route = implode(
                    transpose_array(kapi->query_route())[0],", ") 
                    + " ("+schiff->query_speed() + ")";
            }

            printf("%d: %s\n"
                "Objekt:        %-=63s\n"
                "Umgebung:      %-=63s\n"
                "Kapitän:       %-=63s\n"
                "Idle/gestoppt: %d %s\n"
                "Route:         %-=63s\n\n",
                schiff_nr, Name(schiff),
                schiff_objname,
                schiff_env,
                schiff_kapi,
                idle_time, stopped ? "S" : "",
                schiff_route);
        }
        else
        {
            printf("%-4d%-=39s%-5d%-2s%-=29s\n", 
                schiff_nr,
                schiff_objname,
                idle_time,
                stopped ? "S" : "",
                schiff_env);
        }
    }
    write("\nFähren im Test (nicht in Betrieb):\n");
    entries = FILED->query_entries("faehren");
    int test_count = 0;
    {
        for(i = 0; i < sizeof(entries); i++)
        {
           if(faehren_flag(entries[i], FAEHRE_F_TEST))
           {
              write("    "+
                  left(entries[i][FD_FILE], 39)+
                  " "+entries[i][FD_REMARKS]+"\n");
              test_count++;
           }
        }
    }
    if (!test_count)
        write("    Keine.\n");

    return 1;
}

private string error(int err)
{
   switch(err)
   {
      case FDR_OK: return "Ok.\n";
      case FDR_ILLEGAL_TYPE: return "Unbekannter Typ.\n";
      case FDR_NO_AUTH: return "Du darfst das leider nicht.\n";
      case FDR_NO_FILE: return "Übergebener Filename ist ungültig.\n";
      case FDR_NO_CODERS: return "Keine Programmierer angegeben.\n";
      case FDR_ENTRY_EXISTS: return "Eintrag existiert bereits.\n";
      case FDR_ENTRY_NOT_EXISTANT: return "Fähre existiert nicht.\n";
      case FDR_NO_TESTER: return "Keine Tester eingetragen.\n";
      case FDR_FILE_NOT_FOUND: return "Importfile nicht gefunden.\n";
      default: return "Unbekannter Fehler von "+FILED+".\n";
   }
}

private int toggle_faehren_flag(string file, int flag)
{
   int res;
   mixed *parms;

   parms = ({}) + (FILED->query_parameters("faehren", file) || ({}));
   if(sizeof(parms) <= FAEHRE_P_FLAGS)
      parms = ({0});
   parms[FAEHRE_P_FLAGS] ^= flag;
   if(!(res = FILED->set_parameters("faehren",file,parms)))
      reload("faehren");
   return res;
}

int test(string str)
{
   if(!str)
   {
      notify_fail(query_verb()+" <schiffs-file>\n");
      return 0;
   }
   write(error(toggle_faehren_flag(str, FAEHRE_F_TEST)));
   return 1;
}

int versenke(string str)
{
    int num;
    object schiff, *passengers;
    string default_room, name;

    if (!str || str == "")
        return notify_fail("Versenke welches Schiff ?\n");

    if (!sscanf(str,"%d",num))
        return notify_fail("Bitte die Schiffsnummer angeben.\n");

    if (sizeof(faehren) <= 0)
        return notify_fail("Es sind keine Fähren eingetragen.\n");

    num -= 1;
    if (num < 0 || num > sizeof(faehren)-1)
        return notify_fail("Diese Fähre gibt es nicht.\n");

    schiff = find_object(faehren[num]);
    if (!schiff)
        return notify_fail("Das Schiff ist nicht geladen.\n");

    default_room = schiff->query_default_room();
    if (!default_room || default_room == "")
        default_room = "/map/m8_0";
    name = schiff->query_cap_name();
    passengers = schiff->query_passagiere();

    schiff->remove();
    foreach (object passenger: passengers)
        if (passenger &&
            interactive(passenger) &&
            passenger->move(default_room) != MOVE_OK &&
            passenger)
                passenger->move("/room/void");

    write("Das Schiff "+name+" wurde versenkt.\n");
    say(Der(this_player())+" hat soeben die Fähre "+(num+1)+" versenkt.\n");
    return 1;
}

string *query_faehren() { return ({})+faehren; }

int passagiere(string str) {
        int num, a;
        object *passengers;

    if (sizeof(faehren) <= 0) {
        write("Keine Fähren eingetragen.\n");
        return 1;
        }
    if (!str || str == "") {
        write("Von welcher Fähre ?\n");
        return 1;
        }
    if (sscanf(str,"%d",num)) {
        num -= 1;
        if (num >= sizeof(faehren) || num < 0) {
            write("Diese Fähre gibt es nicht.\n");
            return 1;
            }
        passengers = faehren[num]->query_passagiere();
        if (sizeof(passengers) <= 0) {
            write("Es ist niemand an Bord.\n");
            return 1;
            }
        write("An Bord "+des(touch(faehren[num]))+" befinden sich:\n");
        for (a=0; a<sizeof(passengers); a++)
            if (playerp (passengers[a]))
                write(Der(passengers[a])+" (Spieler).\n");
            else
                write(Der(passengers[a])+" (NPC).\n");
        say(Der(this_player())+" schaut sich die Passagierliste der Fähre "+(num+1)+" an.\n");
        return 1;
        }
    write("Von welcher Fähre ?\n");
    return 1;
}

int key_schiffsfaehren(string str, string verb, object monster, object player,
    int flags)
{
    monster->do_command("sage Die Verwaltung von Schiffsfähren "
        "findet im Ausgang 'schiff' statt "
        "(starten,passagiere,versenken,Routen).");
}

mixed *query_keyword_rules()
{
    return ({
"key_schiffsfaehren: [schiff] || [faehr]",
        PARSE_SAY|PARSE_CONTINUE,
    });
}
