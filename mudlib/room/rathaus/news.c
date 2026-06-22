// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/room/rathaus/news.c
// Description:	Ein Raum zur Verwaltung der Brettmoderatoren
// Author:	?? wohl Garthan
// Modified by:	Monty (24.04 1996): list() erweitert.

#include <level.h>
#include <more.h>
#include <config.h>
#include <monster.h>
#include <news.h>

inherit "/i/room";
inherit "/i/tools/security";

#define SECURE	(wizp(this_player()) && check_security())

#define ADD 1
#define DELETE 0

void reset()
{
    "/room/rathaus/div/lager"->get_moebel(this_object());
    "/room/rathaus/div/lager"->get_pflanze(this_object());
}

void create()
{
   init_security_for_actions();
   add_type("kunstlicht", 1);
   add_type("teleport_rein_verboten", 1);
   set_own_light(1);
   set_short("Im Archiv");
   set_long("Im Archiv der Pressagentur "+get_genitiv(MUD_NAME)+".\n"
	    "Hier wird festgesetzt, wer welche Newsbretter moderiert.\n"
	    "   Kommandos: nanu (Was soll das hier?)\n"
	    "              mod[erator] {+|-} <name>[ <Brett>[ <Gruppe>]]\n"
	    "              les[er]     {+|-} <name>[ <Brett>[ <Gruppe>]]\n"
	    "              aut[oren]   {+|-} <name>[ <Brett>[ <Gruppe>]]\n"
	    "              mod[erator] <suchstring>\n"
	    "              les[er]     <suchstring>\n"
	    "              aut[oren]   <suchstring>\n"
	    "              gr[uppe]    + <name> <dateiname> <funktion> <erweitert>\n"
	    "              gr[uppe]    - <name>\n"
	    "              gr[uppe]\n"
	    "              banne <name>    "
			 "(Alle Mod Einträge von <name> löschen.)\n"
	    "              kons[istenzcheck]\n"
	    );
   set_exit("forum", "forum");
   set_room_domain("Pantheon");
   reset();
}

void init()
{
   add_action("moderator", "moderator", -3);
   add_action("reader", "leser", -3);
   add_action("writer", "autoren", -3);
   add_action("gruppe", "gruppe", -2);
   add_action("remove_owner", "banne");
   add_action("nanu", "nanu");
   add_action("konsistenzcheck","konsistenzcheck",-4);
}

int remove_owner(string name)
{
   if(!SECURE)
      return 0;

   if(adminp(this_player()) ||
      NEWSD->is_owner(this_player()))
   {
      string str;
      int fail;
      name = trim(name);
      if(str=NEWSD->delete_from_all(PERM_OWNER,name)) {write(wrap(str)); fail=1;}
      if(str=NEWSD->delete_from_all(PERM_READ,name)) {write(wrap(str)); fail=1;}
      if(str=NEWSD->delete_from_all(PERM_WRITE,name)) {write(wrap(str)); fail=1;}
      if(!fail) write("Ok.\n");
      return 1;
   }
   notify_fail("Du darfst das nicht.\n");
}

private int validate_owner(string brett, string gruppe, int mode)
{
   if(mode == ADD)
   {
      if(gruppe && !NEWSD->query_times(brett, gruppe))
      {
	 notify_fail("Die Gruppe "+gruppe+
		     " gibt es nicht am Brett "+brett+".\n");
	 return 0;
      }
      else
      if(brett && !NEWSD->query_times(brett, ""))
      {
	 notify_fail("Das Brett "+brett+" gibt es nicht.\n");
	 return 0;
      }
   }

   if(adminp(this_player()) || NEWSD->is_owner(this_player(),brett,gruppe))
       return 1;
   if(!brett)
       return notify_fail("Du musst entweder Newsadmin oder Mudadm sein.\n");
   if(!gruppe)
   {
       if(NEWSD->is_owner("Root",brett,0,1))
           return notify_fail(wrap("Du musst Brettmoderator oder Mudadm sein."));
       return notify_fail(wrap("Du musst entweder Brettmoderator, Newsadmin "
           "oder Mudadm sein."));
   }
   if(NEWSD->is_owner("Root",brett,gruppe,1))
       return notify_fail("Du musst entweder Gruppenmoderator oder Mudadm sein.\n");
   else if(NEWSD->is_owner("Root",brett,0,1))
       return notify_fail(wrap("Du musst entweder Gruppenmoderator, "
           "Brettmoderator oder Mudadm sein."));
   return notify_fail(wrap("Du musst entweder Gruppenmoderator, Brettmoderator, "
   "Newsadmin oder Mudadm sein."));	
}

private int validate_admin(string brett, string gruppe, int mode)
{
   if(mode == ADD)
   {
      if(gruppe && !NEWSD->query_times(brett, gruppe))
      {
	 notify_fail("Die Gruppe "+gruppe+
		     " gibt es nicht am Brett "+brett+".\n");
	 return 0;
      }
      else
      if(brett && !NEWSD->query_times(brett, ""))
      {
	 notify_fail("Das Brett "+brett+" gibt es nicht.\n");
	 return 0;
      }
   }

   if(adminp(this_player()))
       return 1;
      
   if(NEWSD->is_owner("Root", gruppe, brett, 1))
       return notify_fail(wrap("Du musst Mudadm sein, um den Moderator für"
           " diese Gruppe zu ändern."));

   if(brett && NEWSD->is_owner(this_player(), gruppe?brett:0))
       return 1;
       
   if(brett && NEWSD->is_owner("Root", gruppe?brett:0))
         notify_fail(wrap("Du darfst den "+(gruppe?"Gruppen":"Brett")+
	     "moderator nicht ändern."));
   else if(brett && gruppe)
	 notify_fail(wrap("Du bist weder Brettmoderator, noch Newsadmin, noch "
		     "Mudadmin und darfst den Gruppenmoderator nicht "
		     "ändern."));
   else if(brett)
	 notify_fail(wrap(
		     "Du bist weder Newsadmin noch Mudadmin und darfst den "
		     "Brettmoderator nicht ändern."));
   else
	 notify_fail("Du bist kein Mudadmin und darfst den Newsadmin nicht "
	             "ändern.\n");
   return 0;
}


private string get_brett_name(string index, int perm)
{
   if(index == "/")
      return (perm==PERM_OWNER)?"Newsadmin":"Alle";
   else
      return implode(explode(index, "/")-({""})," ");
}

#define WIDTH 30
#define COMMANDS ({"mod[erator]", "les[er]", "aut[oren]"})
#define DESC ({"", " Leser", " Autor"})
#define NAME ({"Moderatoren", "Leser", "Autoren"})
private int list(int perm, string what)
{
   mapping owners;
   string *idxs, out, brett_str;
   int i;

   what = what ? lower_case(what) : "";
   out = left("Brett",WIDTH)+" "+NAME[perm]+"\n"+
         copies("-",WIDTH)+" "+("-"*strlen(NAME[perm]))+"\n";
   for(i = sizeof(
       idxs = sort_array(m_indices(owners = NEWSD->query_owners()), #'<)); i--;)
   {
       int maxlen=0;
       if(!sizeof(owners[idxs[i],perm]))
           continue;
       foreach(string str:owners[idxs[i],perm])
           if(maxlen<strlen(str))
	       maxlen=strlen(str);
       brett_str=sprintf("%-*s %-*"+
            ((maxlen<=((74-WIDTH)/4))?".4"
	   :(maxlen<=((74-WIDTH)/2))?".2"
	   :"")+"#s\n", WIDTH, get_brett_name(idxs[i],perm),
         78-WIDTH,
	 implode(sort_array(owners[idxs[i],perm],#'>),"\n"));
       if(sizeof(regexp(({lower_case(brett_str)}),what)))
         out+=brett_str;
   }
   this_player()->more(explode(out, "\n")-({""}),0,0,M_AUTO_END);
   return 1;
}

private int change_perm(int perm, string str)
{
   string brett, gruppe, name, junk, err;
   int action;

   if(!SECURE)
       return 0;

   if(!str)
       return list(perm,str);
   str = trim(str);
   action = str[0];
   if(action!='+' && action!='-')
       return list(perm, str);
   str = trim(str[1..<1]);
   if((sscanf(str+" ", "%s %s %s %s", name, brett, gruppe, junk) == 4 && junk!="") ||
      !strlen(name))
   {
      notify_fail(COMMANDS[perm] + " {+|-} <name>[ <Brett>[ <Gruppe>]]\n");
      return 0;
   }
   if(!this_player() || this_interactive() != this_player())
   {
      notify_fail("So geht's nicht.\n");
      return 0;
   }

   if(perm==PERM_OWNER)
   {
      if(!validate_admin(brett, gruppe, (action=='+')?ADD:DELETE))
         return 0;
   }
   else if(!validate_owner(brett, gruppe, (action=='+')?ADD:DELETE))
       return 0;

    
   if(action=='+')
       err=NEWSD->add_perm(perm, name, brett, gruppe);
   else
       err=NEWSD->delete_perm(perm, name, brett, gruppe);
   if(err)
       write(wrap(err));
   else
       printf("%s%s: %c %s %s, ok.\n", capitalize(name), DESC[perm],action,brett||"",gruppe||"");
   return 1;
}

int moderator(string str) {return change_perm(PERM_OWNER, str);}
int reader(string str) {return change_perm(PERM_READ, str);}
int writer(string str) {return change_perm(PERM_WRITE, str);}

int gruppe(string str)
{
   if(!SECURE)
      return 0;

    str = trim(str||"");
    if(str=="")
    {
	mapping masters = NEWSD->query_masters();
	string out, *idxs;
	out = "Gruppe               Funktion                                                Fl\n"
	      "-------------------- ------------------------------------------------------- --\n";
	idxs = sort_array(m_indices(masters), #'>);
	foreach(string grp:idxs)
	    out+=sprintf("%-20s %-55=s %s\n", grp,
		masters[grp,MASTER_FILE]+"->"+masters[grp,MASTER_FUN],
		((masters[grp,MASTER_FLAGS]&GRP_ALLOW_NAME)?"N":" ")+
		((masters[grp,MASTER_FLAGS]&GRP_ALL_BOARDS)?"A":" "));
	this_player()->more(explode(out, "\n")-({""}),0,0,M_AUTO_END);
	return 1;
    }
    if(!NEWSD->is_owner(this_player()) && !adminp(this_player()))
	return notify_fail("Du musst entweder Newsmoderator oder Admin sein.\n");
    if(str[0]=='+')
    {
	string name, datei, funktion, result, flags;
	if(sscanf(trim(str[1..<1])+" ","%s %s %s %s",name,datei,funktion,flags)<3)
	    return notify_fail("gr[uppe] + <name> <datei> <funktion> <flags>\n");
	flags ||="";
	result = NEWSD->add_master(name, datei, funktion,
	    (member(lower_case(flags),'a')>=0 ? GRP_ALL_BOARDS : 0) |
	    (member(lower_case(flags),'n')>=0 ? GRP_ALLOW_NAME : 0));
	if(stringp(result))
	    write(wrap(result));
	else
	    printf("%s: %s->%s, Okay.\n", name, datei, funktion);
	return 1;
    }
    else if(str[0]=='-')
    {
	string name, result;
	name = trim(str[1..<1]);
	if(!strlen(name))
	    return notify_fail("gr[uppe] + <name> <datei> <funktion>\n");
	result = NEWSD->delete_master(name);
	if(stringp(result))
	    write(wrap(result));
	else
	    printf("%s entfernt.\n", name);
	return 1;
    }
    else
	return notify_fail("gr[uppe] [{+ <name> <datei> <funktion>|- <name>}]\n");
}

int nanu()
{
    this_player()->more(
	({
	    "In diesem Raum kann man die Eigenschaften der Newsbretter einstellen.",
	    "Dazu gehört, wer die Moderatoren der Newsgruppen sind, wer sie lesen und wer",
	    "in ihnen schreiben darf. Diese Einstellungen kann man für spezielle Gruppen",
	    "ganze Bretter oder alle Bretter vornehmen. Einen neuen Gruppenmoderator",
	    "dürfen nur der Brettmoderator und Newsmoderator ernennen, einen neuen",
	    "Brettmoderator darf nur der Newsmoderator ernennen. Die Admins dürfen alles.",
	    "Lese- und Schreibberechtigte dürfen die entsprechenden Moderatoren des",
	    "betreffenden Bretters bzw. der betreffenden Gruppe benennen.",
	    "",
	    "Übergeordnete Einstellungen gelten automatisch für die untergeordneten",
	    "Bereiche. (D.h. Einstellungen ohne ein speziell angegebenes Brett gilt für",
	    "alle Bretter, Einstellungen für ein Brett gilt für seine Gruppen.) Von",
	    "dieser Regel gibt es eine Ausnahme: Wurde 'Root' als Moderator/Lese-/Schreib-",
	    "berechtigter angegeben, so gelten die übergeordneten Einstellungen nicht.",
	    "Die Standardeinstellung ist, dass niemand Moderator und alle Lese- bzw.",
	    "Schreibberechtigt sind. Es ist also notwendig, bei Lese-/Schreib-",
	    "einschränkungen 'Root' anzugeben.",
	    "",
	    "Um einen Moderator hinzuzufügen gibt man:",
	    "    moderator +<name> <Brett> <Gruppe>",
	    "ein. 'moderator' kann mit 'mod' abgekürzt werden. Gruppe kann weggelassen",
	    "werden, wenn dies für das gesamte Brett gelten soll. Darüberhinaus kann",
	    "das Brett weggelassen werden, wenn dies für alle Bretter gelten soll.",
	    "Entsprechend verhält es sich bei Lese- oder Schreibberechtigten, wo man",
	    "'leser' oder 'autor' statt Moderator angeben muss. Um einen Moderator/",
	    "Lese-/Schreibberechtigten zu entfernen muss einfach ein '-' statt einem '+'",
	    "angegeben werden. Man muss ihn dabei immer aus dem Bereich austragen, für",
	    "den er auch eingetragen war. D.h. es ist nicht möglich, jemanden für ein",
	    "ganzes Brett einzutragen und dann nur für eine Gruppe wieder auszutragen.",
	    "",
	    "Eingetragen werden kann:",
	    " - ein einzelner Spieler (in Kleinbuchstaben)",
	    " - eine Gruppe nach /apps/groups",
	    " - eine Gruppe nach /apps/newsd",
	    "",
	    "Das Archiv kann eigene Gruppen anlegen. Dazu muss man ein Master-Objekt",
	    "bereitstellen, welches für jeden Spieler angibt, ob er zur Gruppe gehört",
	    "oder nicht. Dies geschieht über den Befehl:",
	    "    gruppe + <gruppenname> <dateiname> <funktion> <flags>",
	    "<gruppenname> kann dann als Moderator, Lese- oder Schreibberechtigter",
	    "eingetragen werden. Muss nun überprüft werden, ob ein Spieler Mitglied",
	    "ist, wird",
	    "    <dateiname>-><funktion>(object spieler, int art,",
	    "                            string brett, string gruppe)",
	    "aufgerufen, wobei art entweder PERM_OWNER (Moderator), PERM_READ",
	    "(Leseberechtigter) oder PERM_WRITE(Schreibberechtigter) ist. (Diese",
	    "Konstanten sind in news.h definiert.) spieler ist nicht zwangsweise ein",
	    "Spieler, sondern kann auch ein NPC oder Gegenstand sein. Daher sollte man",
	    "auf playerp(spieler) überprüfen, falls man den Zugang auf Spieler",
	    "begrenzen will. (Dies ist dazu gedacht, um bestimmten Tools Zugriff auf",
	    "bestimmte Bretter zu ermöglichen.) Falls <flags> ein 'N' enthält, so kann",
	    "als Spieler auch ein String, der Real-Name, angegeben werden. Dies ist",
	    "für Zugriff z.B. via Usenet gedacht. Falls <flags> ein 'A' enthält, so",
	    "erhält die Funktion die Parameter art, brett und gruppe nicht (die Funktion",
	    "wird dann auch deutlich seltener aufgerufen).",
	    "",
	    "Der Name der Gruppe sollte so gewählt werden, dass auch Spieler ihn",
	    "verstehen, denn sie sehen ihn bei der Auflistung der Moderatoren eines",
	    "Brettes.",
	    "",
	    "Es gibt folgende vordefinierte Gruppen, welche unveränderbar sind:",
	    "    Gildenmitglieder: Das Brett ist Gildenbrett des Spielers",
	    "    Spielerrat:       Der Spielerrat",
	    "    Engel:            Alle Engel und Götter",
	    "    Goetter:          Alle Götter",
	    "    Lords:            Alle Lords",
	    "    Vorstand:         Der Vorstand des Trägerkreises",
	    "",
	    "Mit den Befehlen 'moderator', 'leser' und 'autoren' kann man sich die",
	    "aktuellen Einstellungen anschauen, wobei man als Parameter einen Suchstring",
	    "angeben kann. Mit 'gruppe' kann man sich alle Gruppendefinitionen anschauen."
	}), 0, 0, M_AUTO_END);
    return 1;
}

int konsistenzcheck(string str)
{
    string *err;
    err = NEWSD->consistency_check();
    if(sizeof(err))
	write(implode(map(err,#'wrap),""));
    else
	write("Alles Okay.\n");
    return 1;
}

int key_news(string str, string verb, object monster, object player,
    int flags)
{
    monster->do_command("sage Die Verwaltung der Bretter geschieht "
        "im 'Archiv'.");
}

mixed *query_keyword_rules()
{
    return ({
"key_news: news || neuigkeiten || [brett] || [moderator]",
        PARSE_SAY|PARSE_CONTINUE,
    });
}
