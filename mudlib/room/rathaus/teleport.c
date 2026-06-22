// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /room/rathaus/teleport.c
// Description: Verwaltung aller Teleportziele
// Author:      

#include <config.h>
#include <level.h>
#include <apps.h>
#include <more.h>
#include <monster.h>
#include <touch.h>

inherit "/i/room";
inherit "/i/tools/security";

#define FAIL(x) { notify_fail(x); return 0; }

#define INHERITS ({ "/i/hlp/room.c", "/i/hlp/lander.c" })

int query_prevent_shadow(object ob) { return 1; }

int query_ort(string filename)
{
   return HLP_ORTE->query_ort(filename);
}

void reset()
{
   if(!present("roomautoload"))
      ROOM_AUTOLOAD->move(this_object());
    "/room/rathaus/div/lager"->get_moebel(this_object());
    "/room/rathaus/div/lager"->get_pflanze(this_object());
}

void create()
{
   init_security_for_actions();
   set_own_light(1);
   add_type("kunstlicht", 1);
   add_type("kaempfen_verboten", 1);
   add_type("teleport_rein_verboten", 1);
   set_exit("forum","forum");
   set_room_domain("Pantheon");
   set_short("Teleportzentrale");
   set_long(
   "Die Teleportzentrale. Ein Raum mit schrecklich vielen Röhren, Kabeln "
   "und sonstigen technischen Einrichtungen, Dir gänzlich unbekannt. "
   "Hierher kommen die Domainlords, um ihre Teleportanflugziele zu warten.\n"
   "   Kommandos: nanu        Die Hilfe. LIES DAS ZUERST!!!\n"
   "              tele [my]   Zeigt alle Teleportziele.\n"
   "              hlp [my]    Zeigt alle Engelteleportziele.\n"
   "              addtele     Legt ein neues Teleportziel an.\n"
   "              chgtele     Ändert Filenamen in der Teleliste.\n"
   "              deltele     Löscht ein Teleportziel.\n"
   "              delhlp      Löscht nur das Engelteleportziel.\n"
   "              chktele     Welche Teleportzielfiles sind nicht existent?"
   );
   reset();
}

void init()
{
   if(wizp(this_player()))
   {
      add_action("help",      "nanu");
      add_action("list_tele", "tele");
      add_action("add_tele",  "addtele");
      add_action("delete_tele", "deltele");
      add_action("change_tele", "chgtele");
      add_action("list_hlp",  "hlp");
      add_action("delete_hlp", "delhlp");

      add_action("check_tele", "chktele");
      add_action("check_hlp", "chkhlp");
      add_action("clean_hlp", "clnhlp");
   }
   else
      call_out("raus", 3);
}

int help()
{
   this_player()->more("/doc/funktionsweisen/hlp_teleport");
   return 1;
}

void raus()
{
   if(this_player() && present(this_player()))
      write("Du fragst Dich, was Du hier suchst.\n");
}

private mixed secure()
{
   string *domains =({});

   if(this_player() && check_security())
   {
      if(adminp(this_player()))
         return 1;
      if(!(lordp(this_player()) ||
           sizeof((domains=DOMAIN_INFOS->query_domainhelfer_of( 
                             this_player()->query_real_name())))))
         return 0;
      domains = map(
          DOMAIN_INFOS->query_domains_of(this_player()->query_real_name())+
          domains,
	  (: "/d/"+$1+"/" :) );
      domains += map(
          FILED->query_auth_of(this_player()->query_real_name()),
	  (: ($1=="p") ? "/p/" : ("/z/"+capitalize($1)+"/") :));
      return domains;
   }
}

private int responsible(mixed domains, string name)
{
   if(!pointerp(domains)) return domains;
   return sizeof(filter(domains, (: !strstr($2, $1) :), name));
}

private void clean_hlp_orte()
{
   HLP_ORTE->clean_orte();
}


int list_tele(string str)
{
   this_player()->more(explode(
      TELE_MASTER->all_teles(0, str ? this_player() :0)[0..<2],"\n"));
   return 1;
}

int list_hlp(string str)
{
   this_player()->more(explode( 
      TELE_MASTER->all_teles(this_object(), str?this_player():0)[0..<2],"\n"));
   return 1;
}

int add_tele(string str)
{
   int res;
   string name, filename, telefile, *parts;
   mixed domains;

   if(!(domains = secure()))
      FAIL("Du darfst das noch nicht.\n");
   if(!str ||
      sizeof(parts = explode(trim(str), " ") - ({})) < 2 ||
      parts[<1][0..0] != "/")
      FAIL(query_verb()+" <anflugname> <filename>\n");
   filename = parts[<1];
   if(name=map2domain(filename,1))
      filename = name[0..<3];
   name = lower_case(implode(parts[0..<2], " "));

   if(pointerp(domains) &&
      (telefile = TELE_MASTER->get_tele(name)) &&
      !responsible(domains,telefile))
      FAIL(wrap("Du kannst das neue Ziel nicht anlegen, "
                "da es bereits existiert und "
                "dessen File nicht in Deinem Gebiet liegt."));
   if(!responsible(domains, filename))
      FAIL(wrap("Du kannst das neue Ziel nicht anlegen, "
                "da dessen File nicht in Deinem Gebiet liegt."));
   switch(res = TELE_MASTER->add_tele(name, filename))
   {
      case -1 : FAIL("Ungültiger Ziel- und/oder Filename.\n");
      case -2 : FAIL("Angegebener Raum existiert nicht.\n");
      case -3 : FAIL("Ungültiger Zielname.\n");
      case -4 : FAIL("Ein Teil deines Zielnamens exitiert bereits als Ziel.\n");
      case -5 : FAIL("Dein Zielname ist bereits Teil eines anderen Ziels.\n");
      case 0  : write("Ziel angelegt.\n"); break;
      default : FAIL("Error: "+res+"\n");

   }
   return 1;
}

int delete_tele(string str)
{
   int res;
   mixed domains;
   string telefile;

   if(!(domains = secure()))
      FAIL("Du darfst das noch nicht.\n");
   if(!str)
      FAIL(query_verb()+" <anflugname|filename>\n");
   if(str[0..0] == "/" && strstr(str, " ") < 0)
   {
      string mapr = map2domain(str,1);
      if(mapr)
         str = mapr[0..<3];
      if(!responsible(domains, str))
         FAIL(wrap("Du kannst die via Filename angegebenen Ziele nicht "
                   "löschen, weil das File nicht in Deinem Gebiet liegt."));
   }
   else
   {
      if(pointerp(domains) &&
         (telefile = TELE_MASTER->get_tele(str)) &&
	 !responsible(domains, telefile))
        FAIL(wrap("Du kannst das via Zielkürzel angegebene Ziel nicht "
                  "löschen, weil dessen Zielraum nicht in Deinem Gebiet "
                  "liegt."));
   }
   switch(res = TELE_MASTER->delete_tele(str))
   {
      case -1: FAIL("Ungültiger Ziel-/Filename.\n");
      case -3: FAIL("Ungültiger Zielname.\n");
      case -4: FAIL("Ein Teil Deines Zielnamens ist selbst Ziel.\n");
      case -5: FAIL("Dein Zielname ist Teilname eines Ziels.\n");
      case -6: FAIL("Dein zu löschendes Ziel existiert gar nicht.\n");
      case 0: write("Ziel(e) gelöscht.\n"); break;
      default : FAIL("Error: "+res+"\n");
   }
   clean_hlp_orte();
   return 1;
}

int change_tele(string str)
{
   int res;
   string *parts, old, new, mapr;
   <int|string*> domains;

   if(!(domains = secure()))
      FAIL("Du darfst das noch nicht.\n");
   if(!str || sizeof(parts = explode(str, " ")-({""})) != 2)
      FAIL(query_verb()+" <old_filename> <new_filename>\n");
   old = parts[0];
   new = parts[1];
   if(old[0..0] != "/" || new[0..0] != "/")
      FAIL("Einer der beiden Filenamen ist keiner.\n");

   mapr = map2domain(old,1);
   if(mapr)
      old = mapr[0..<3];
   mapr = map2domain(new,1);
   if(mapr)
      new = mapr[0..<3];

   if(!responsible(domains, old))
      FAIL("Der alte Filename liegt nicht in Deinem Gebiet.\n");
   if(!responsible(domains, new))
      FAIL("Der neue Filename liegt nicht in Deinem Gebiet.\n");
   switch(res = TELE_MASTER->change_filename(old, new))
   {
      case -1: FAIL("Ungültige Filenamen.\n");
      case -2: FAIL("Der neue Raum existiert nicht.\n");
      case -3: FAIL("Der alte Filename ist kein Teleportzielraum.\n");
      case 0:  write("Zielfilename geändert in allen Zielen.\n"); break;
      default : FAIL("Error: "+res+"\n");
   }
   HLP_ORTE->change_filename(old, new);
   return 1;
}

int check_tele()
{
   if(!wizp(this_player()) || !check_security())
      FAIL("Du darfst das nicht.\n");

   this_player()->more(TELE_MASTER->check_teles(),0,0,M_AUTO_END);
   return 1;
}

int check_hlp()
{
   if(!wizp(this_player()) || !check_security())
      FAIL("Du darfst das nicht.\n");

   this_player()->more(HLP_ORTE->check_orte(),0,0,M_AUTO_END);
   return 1;
}

int clean_hlp()
{
   if(!this_player() || this_player() != this_interactive() ||
      !adminp(this_player()) || !check_security())
      FAIL("Du darfst das nicht.\n");
   clean_hlp_orte();
   return 1;
}

int delete_hlp(string str)
{
   string *domains, file, *parts, *inheritance;
   int i;
   object ob;

   if(!(domains = secure()))
      FAIL("Du darfst das noch nicht.\n");
   if(!str)
      FAIL(query_verb()+" <file_name|zielname>\n")
   if(str[0..0] != "/")
   {
      if(!(file = TELE_MASTER->get_tele(str)))
	 FAIL("Das angegebene Ziel existiert nicht in der Teleliste.\n");
   }
   else
   {
      file = map2domain(str,1);
      if(file)
        file = file[0..<3];
      else
        file = str;
   }
   if(!responsible(domains, file))
      FAIL("Das angegebene Ziel liegt nicht in Deinem Gebiet.\n");
   if(ob = touch(file, NO_LOG | NO_WRITE))
   {
      inheritance = inherit_list(ob);
      for(i = sizeof(parts = INHERITS); i--;)
	 if(member(inheritance, parts[i]) != -1)
	    FAIL(file + " darf nicht mehr " + parts[i] + " inheriten!\n");
   }
      
   if(!HLP_ORTE->delete_ort(file))
      FAIL("Das war wohl gar kein HLP Teleportziel?\n");
   write("Engelanflugort entfernt.\n");
   return 1;
}

int key_teleport(string str, string verb, object monster, object player,
    int flags)
{
    monster->do_command("sage Engelsziele und Teleportziele werden "
        "im Ausgang 'fziele' verwaltet.");
}

int key_schranke(string str, string verb, object monster, object player,
    int flags)
{
    monster->do_command("sage Baustellen-Schranken und andere "
        "Raum-Autoloader werden als Buch hinter Ausgang 'fziele' "
        "verwaltet.");
}

mixed *query_keyword_rules()
{
    return ({
"key_teleport: [ziel] || zg || landeplatz || teleport",
        PARSE_SAY|PARSE_CONTINUE,
"key_schranke: [schrank] || [baustell] || [autoload]",
        PARSE_SAY|PARSE_CONTINUE,
    });
}
