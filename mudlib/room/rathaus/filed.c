// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /room/rathaus/filed.c
// Description: Userinterface zu /apps/filed.c
// Author:      Garthan (17.08.94)

#include <apps.h>
#include <monster.h>
#include <more.h>
#include <level.h>
#include <touch.h>
#include <filed.h>
#include <deklin.h>
#include <more.h>

#define GROUPSCHANGED GROUP_MASTER->changed()

inherit "/i/room";
inherit "/room/rathaus/i/wizinfo";
inherit "/i/tools/security";

#define SECURE	(wizp(this_player()) && check_security())

int query_prevent_shadow(object ob) { return 1; }

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
   set_short("Im Gouverneursbüro");
   set_long("Im Gouverneursbüro. Hier werden neue Spiele, Rätsel oder "
            "Gilden von den\ndafür zuständigen Lords eingetragen.\n\n"
            "   Kommandos: nanu  (Was soll das hier?)\n"
	    "              liste {<typ>|auth}\n"
            "              auth <typ> {+|-}<name> (nur Admins)\n"
//            "              export (nur Admins)   \\  fuer groessere\n" 
//            "              import (nur Admins)   /  Aenderungen\n" 
            "              neu <typ> <file> <prgrmer1>[, <prgrmer2>, ...]\n"
            "              lösche <typ> {<file>|<nr>}\n"
            "              rep[osition] <typ> {<file>|<nr>} <pos>\n"
	    "              file <typ> {<file>|<nr>} <new_file>\n"
            "              coder <typ> {<file>|<nr>} "
			   "<prgrmer1>[, <prgrmer2>, ...]\n"
            "              komm[entar] <typ> {<file>|<nr>} <text>\n"
            "              ers[atz] [alte-AL-Pfad [neue-AL-Pfad]]         (siehe ersatz ?)\n"
            "              pers[atz] [alter-PL-Pfad [neuer-PL-Pfad]]      (siehe persatz ?)\n"
            "              wt[ext] <prgrmer> {liste|neu} <gebiet> <text>\n"
            "              wt[ext] <prgrmer> {aender} <gebiet> <nr> <text>\n"
	    "                                                             (siehe wtext ?)\n"
            "   mit <typ> =  {"+implode(FILED->query_types(), "|")+"}\n"
            "                (abkuerzbar)\n"
            "       <file> =  Filename des einzutragenden Objekts ohne .c\n");
   set_exit("forum", "forum");
   set_room_domain("Pantheon");
   reset();
}

int nanu()
{
   write(
"Nanu, was soll denn das?\n"
"------------------------\n"
"\n"
"Dieser Raum ermöglicht, speziell dazu priviligierten Göttern, verschiedene\n"
"Filenamelisten zu verwalten, die von anderen Objekten benötigt werden.\n"
"Wer wofür zuständig ist, sieht man mit 'liste auth'.\n"
"Ändern können MudAdmins diese 'Authority'-Liste mit dem 'auth' Befehl.\n"
"Mit 'liste <typ>' kann man sich die Listen ansehen. (Beispiel: liste r)\n"
"Einen neuen Eintrag in die Listen macht man mit dem Befehl 'neu'.\n\n"
"Beispiel: 'neu rätsel /z/Raetsel/irgendwo/super_quest garthan'\n\n"
"Die dazugehörigen Testflags kann man dann im Gilden- oder Rätselraum\n"
"setzen.\n\n"
"Die restlichen Befehle dienen dazu einen bestehenden Eintrag zu löschen\n"
"oder zu ändern. Mit 'file' kann man den Filenamen eines Eintrags nach-\n"
"traeglich ändern, mit 'coder' die Programmierer und mit 'komm' die\n"
"Kommentarzeile des Eintrags. Mit 'rep' kann man einen Eintrag in der Liste\n"
"verschieben. Der Eintrag wird nach dem angegebenen eingeschoben (<pos>=0..x).\n"
"Einträge werden immer mit der zugehörigen Liste und dem Filenamen oder\n"
"der Nummer aus dem 'liste' Befehl referenziert.\n\n"
"Beispiel: 'coder r 5 garthan' setzt den Programmierer des fünften Rätsels\n"
"          auf Garthan.\n\n"
"In diesen Raum kommt man am schnellsten mit 'zg go[uverneursbuero]'.\n"
   );
   return 1;

}

void init()
{
   int i;
   string *types;
   add_action("nanu", "nanu");
   add_action("liste", "liste", -4);
   add_action("liste", "info");
   add_action("auth", "auth");
   add_action("add_coder", "neu");
   add_action("delete_coder", "lösche", -5);
   add_action("reposition", "repositioniere", -3);
   add_action("remarks", "kommentar", -4);
   add_action("coder", "coder");
   add_action("file", "file");
   add_action("ersatz","ersatz",-3);
   add_action("persatz","persatz",-4);
   add_action("fingertext","wtext",-2);
   // disabled weil import mit parameter nicht klappt
   //add_action("import", "import");
   add_action("export", "export");
   for(i = sizeof(types = FILED->query_types()); i--;)
      add_action("shorties", types[i]);
}

private void reload_request(string type)
{
   string admin_ob;
   object ob;

   if((admin_ob = FILED->query_admin_ob(type)) && 
      (ob = touch(admin_ob, NO_LOG|NO_WRITE)))
      ob->reload(type);
}

private string error(int err)
{
   switch(err)
   {
      case FDR_OK: return "Hat geklappt.\n";
      case FDR_ILLEGAL_TYPE: return "Unbekannter Typ.\n";
      case FDR_NO_AUTH: return "Du darfst das leider nicht.\n";
      case FDR_NO_FILE: return "Übergebener Filename ist ungültig.\n";
      case FDR_NO_CODERS: return "Keine Programmierer angegeben.\n";
      case FDR_ENTRY_EXISTS: return "Eintrag existiert bereits.\n";
      case FDR_ENTRY_NOT_EXISTANT: return "Eintrag existiert nicht.\n";
      case FDR_NO_TESTER: return "Keine Tester eingetragen.\n";
      case FDR_FILE_NOT_FOUND: return "Importfile nicht gefunden.\n";
      default: return "Unbekannter Fehler von "+FILED+".\n";
   }
}


private int xmember(string *arr, string mem)
{
   int i;
   for(i = 0; i < sizeof(arr); i++)
      if(!strstr(arr[i], mem))
         return i;
   return -1;
}

private varargs string type_check(string str, string mess)
{
   string *types, typstr;
   int i;

   types = sort_array(FILED->query_types(),#'>);
   notify_fail(query_verb()+" {"+implode(types, "|")+"}"+
       (mess ? " " + mess : "")+"\n");
   typstr = convert_umlaute(explode(lower_case(str||"")," ")[0]);
   if(str && (i = xmember(types, typstr)) >= 0)
      return types[i];
}

private string get_file_by_nr(string type, int i)
{
   mixed *entries;

   entries = FILED->query_entries(type);
   if(pointerp(entries) && i >= 0 && i < sizeof(entries))
      return entries[i][FD_FILE];
}

int liste(string str)
{
   mixed *entries;
   int i;
   string ret, type, *types;

   if(!SECURE)
      return 0;

   if(str && lower_case(str) == "auth")
   {
      types = sort_array(FILED->query_types(),#'>);
      write("Eingetragene Lords:\n");
      for(i = 0; i < sizeof(types); i++)
         write(sprintf("%-=12s %-=66s\n", capitalize(types[i])+":",
                 implode(map(FILED->query_auth(types[i]),#'capitalize),
                        ", ")));
      return 1;
   }
   if(!(type = type_check(str)))
      return 0;
   entries = FILED->query_entries(type);
   ret = "Eingetragene "+capitalize(type)+":\n";
   for(i = 0; i < sizeof(entries); i++)
   {
      ret += sprintf("%2d. %-=51s %=-20s\n",
         i+1,
         entries[i][FD_FILE],
         implode(map(entries[i][FD_CODERS]||({}),#'capitalize),", "));

      if(entries[i][FD_REMARKS])
	 ret += "       ("+entries[i][FD_REMARKS]+")\n";
   }
   this_player()->more(explode(ret,"\n")[0..<2],0,0,M_AUTO_END);
   return 1;
}

int shorties()
{
   return liste(query_verb());
}

int auth(string str)
{
   string type, name;
   int err;

   if(!SECURE)
      return 0;

   if(!(type = type_check(str, "{+|-}<name>")))
      return 0;
   if(sscanf(lower_case(str),"%~s +%s", name) == 2)
   {
      write(error(err=FILED->add_auth(type, name)));
      GROUPSCHANGED;
      if(err==FDR_OK)
      {
         type=capitalize(type);
         if(strlen(type)==1) type+="-Lord";
         else type+="lord";
         wizinfo (capitalize(name)+" ist jetzt "+type+"!");
      }
      return 1;
   }
   else
   if(sscanf(lower_case(str),"%~s -%s", name) == 2)
   {
      write(error(err=FILED->delete_auth(type, name)));
      GROUPSCHANGED;
      if(err==FDR_OK)
      {
         type=capitalize(type);
         if(strlen(type)==1) type+="-Lord";
         else type+="lord";
         wizinfo (capitalize(name)+" ist kein "+type+" mehr!");
      }
      return 1;
   }
}

static int import()
{
   write("Das Importfile bitte Löschen nach Gebrauch!\n");
   write(error(FILED->import()));
   return 1;
}

int export()
{
   if(!SECURE)
      return 0;

   write(error(FILED->export()));
   return 1;
}

string expand_path(string file)
{
   file = "/"+implode(this_player()->compose_path(file),"/");
   sscanf(file, "%s.c", file);
   return file;
}

int add_coder(string str)
{
   string type, file, rest, *coders;
   int res;

   if(!SECURE)
      return 0;

   if(!(type = type_check(str, "<file> <coder1>[, <coder2>, ...]")))
      return 0;
   if(sscanf(str, "%~s %s %s", file, rest) == 3)
   {
      string err;

      file = expand_path(file);
      rest = lower_case(rest);
      coders = map(explode(rest, ","),#'trim)-({""});

      if (err = catch(touch(file,NO_LOG|NO_WRITE)))
         write("Fehler beim Laden des Files: "+err);
      else
      {
	 write(error(res = FILED->add(type, file, coders)));
	 if(res == FDR_OK)
	    reload_request(type);
      }
      return 1;
   }
}

int delete_coder(string str)
{
   string type, file;
   int nr, res;

   if(!SECURE)
      return 0;

   if(!(type = type_check(str, "{<file>|<nr>}")))
      return 0;
   if(sscanf(str, "%~s %s", file) == 2)
   {
      if(sscanf(file, "%d", nr))
         file = get_file_by_nr(type, nr-1);
      else
	 file = expand_path(file);
      write(error(res = FILED->delete(type, file)));
      if(res == FDR_OK)
	 reload_request(type);
      return 1;
   }
}

int reposition(string str)
{
   string type, file;
   int nr, newpos, res;

   if(!SECURE)
      return 0;

   if(!(type = type_check(str, "{<file>|<nr>} <neupos>")))
      return 0;
   if(sscanf(str, "%~s %s %d", file, newpos) == 3)
   {
      if(sscanf(file, "%d", nr))
         file = get_file_by_nr(type, nr-1);
      else
	 file = expand_path(file);
      write(error(res = FILED->reposition(type, file, newpos-1)));
      if(res == FDR_OK)
	 reload_request(type);
      return 1;
   }
}

int remarks(string str)
{
   string type, file, text;
   int nr, res;

   if(!SECURE)
      return 0;

   if(!(type = type_check(str, "{<file>|<nr>} [<text>]")))
      return 0;
   if(sscanf(str, "%~s %s %s", file, text) == 3 ||
      sscanf(str, "%~s %s", file) == 2)
   {
      if(sscanf(file, "%d", nr))
         file = get_file_by_nr(type, nr-1);
      else
	 file = expand_path(file);
      if(text)
         text = implode(explode(text,":"),"");
      write(error(res = FILED->set(type, file, FD_REMARKS, text)));
      if(res == FDR_OK)
	 reload_request(type);
      return 1;
   }
}

int coder(string str)
{
   string type, file, rest, *coders;
   int res, nr;

   if(!SECURE)
      return 0;

   if(!(type = type_check(str, "{<file>|<nr>} <coder1>[, <coder2>, ...]")))
      return 0;
   if(sscanf(str, "%~s %s %s", file, rest) == 3)
   {
      rest = lower_case(rest);
      if(sscanf(file, "%d", nr))
         file = get_file_by_nr(type, nr-1);
      else
	 file = expand_path(file);
      coders = map(explode(rest, ","),#'trim)-({""});
      write(error(res = FILED->set(type, file, FD_CODERS, coders)));
      if(res == FDR_OK)
	 reload_request(type);
      return 1;
   }
}

int file(string str)
{
   string type, file, new_file;
   int res, nr;

   if(!SECURE)
      return 0;

   if(!(type = type_check(str, "{<file>|<nr>} <new_file>")))
      return 0;
   if(sscanf(str, "%~s %s %s", file, new_file) == 3)
   {
      if(sscanf(file, "%d", nr))
         file = get_file_by_nr(type, nr-1);
      else
	 file = expand_path(file);
      if(catch(touch(new_file,NO_LOG|NO_WRITE)))
         write("Fehler beim Laden des Files.\n");
      else
      {
	 write(error(res = FILED->set(type, file, FD_FILE, new_file)));
	 if(res == FDR_OK)
	    reload_request(type);
      }
      return 1;
   }
}

int ersatz(string str)
{
   string *arr; int res;

   if(!SECURE)
      return 0;

   if (!str) {
       write (FILED->query_autoloader_replacement_list());
       return 1;
   }
   arr = explode (str," ");
   if ((str == "?") || (sizeof (arr) != 1) && (sizeof (arr) != 2)) {
       notify_fail (
       "ersatz              liefert Liste aller Autoloader-Ersaetze\n"
       "ersatz alt          der Autoloader wird aus der Ersatzliste entfernt\n"
       "ersatz alt neu      Beim Einloggen werden bei Autoloadern mit dem Pfad alt\n"
       "                    statt dem Pfad alt der Pfad neu verwendet.\n\n"
       "                    Pfade stets ohne .c!\n");
       return 0;
   }
   write(error(res = FILED->set_autoloader_replacement(
       arr[0], sizeof (arr) == 2 ? arr[1] : 0)));
   if (res == FDR_OK) {
       write (wrap ("Versuche, Autoloader in Liste der genehmigten Autoloader "
           "ebenfalls zu ändern. Das wird schiefgehen, wenn der Autoloader "
           "nicht genehmigt ist."));
       file ("a "+str);
   }
   return 1;
}

int persatz(string str)
{
    string *arr;

    if(!SECURE)
        return 0;

    if (!str) {
        write (FILED->query_plugin_replacement_list());
        return 1;
    }
    arr = explode (str," ");
    if ((str == "?") || (sizeof (arr) != 1) && (sizeof (arr) != 2)) {
        notify_fail (
        "persatz             liefert Liste aller Plugin-Ersaetze\n"
        "persatz alt         das Plugin wird aus der Ersatzliste entfernt\n"
        "persatz alt neu     Beim Einloggen werden bei Plugins mit dem Pfad alt\n"
        "                    statt des Pfades alt der Pfad neu verwendet.\n\n"
        "                    Pfade stets ohne .c!\n");
        return 0;
    }
    write(error(FILED->set_plugin_replacement(
        arr[0], sizeof (arr) == 2 ? arr[1] : 0)));
    return 1;
}

private void fingerausgabe(mixed *liste, string fehler)
{
    string *msg;
    if(!sizeof(liste))
    {
	write(fehler);
	return;
    }
    msg=({
        "Datum      Gebiet       Lord       Meldung",
        "---------- ------------ ---------- --------------------------------------------"
    });
    foreach(mixed *app:liste)
        msg += explode(sprintf(
            "%=10s %-=12s %-=10s %-=44s\n",
		shorttimestr(app[1],1,2), capitalize(app[0]), capitalize(app[2]),
		app[3]),"\n")[0..<2];
    this_player()->more(msg,0,0,M_AUTO_END);
}

int fingertext(string str)
{
    string *args, gebiet, text, wizname;
    mixed back;
    object who;
    int nr,i;

    if(!SECURE)
        return 0;

    str=strip(str||"");
    if(str=="" || str=="?") {
	write(
	    "wt <Gott>                                     Listet alle Würdigungen auf\n"
	    "wt <Gott> liste <Gebiet>                      Zeigt die Texte dieses Gebiets\n"
	    "wt <Gott> neu <Gebiet> \"Text\"                 Fügt die Würdigung hinzu\n"
	    "wt <Gott> ändere <Gebiet> <Nr> \"Neuer Text\"  Ändert die Würdigung\n"
	    "Gebiet ist entweder eine Domain oder Ratsel, Spiele, Gilden, Admin usw.\n"
	    "Nr die Nummer des Eintrages beginnend bei 1.\n"
            "(Eintraege unter ggf. alten Gebietsnamen nicht mitzählen!)\n");
        return 1;
    }
    args=explode(str," ")-({""});
    wizname = lower_case(args[0]);
    who=find_player(wizname);
    if(!who)
    {
        mixed res;
        printf("'%s' ist nicht eingeloggt, verwende offline Zugriff.\n",
            capitalize(wizname));
        who = touch (PLAYER_MODIFIER);
        res = who->load_wiz_appreciations(wizname);
        if (res != 1)
        {
            write (res);
            return 1;
        }
    }
    if(sizeof(args)==1)
    {
        fingerausgabe(who->query_wiz_appreciations(0),
	    sprintf("%s hat noch keine Würdigungen.\n",
	    args[0]));
	return 1;
    }
    if(sizeof(args)==2)
	return notify_fail("Es wurde kein Gebiet angegeben.\n");

    // Sonderbehandlung Midgard/Mittelerde
    if(lower_case(args[2]) == "mittelerde") args[2]="midgard";

    foreach(string g:FILED->query_types()+DOMAIN_INFOS->query_domains())
    {
	if(!strstr(lower_case(g),lower_case(args[2])))
	{
	    gebiet=g;
	    break;
	}
    }
    
    if (!gebiet && lower_case(args[2])=="admin") gebiet="admin";
    
    if (!gebiet)
	return notify_fail("Das Gebiet '" + args[2] + "' gibt es nicht.\n");

    gebiet=lower_case(gebiet);
    if(!strstr("liste",args[1]))
    {
        fingerausgabe(
            (gebiet=="midgard"?
                who->query_wiz_appreciations("mittelerde"):({}))+
                who->query_wiz_appreciations(gebiet),
	    sprintf("%s hat noch keine Würdigungen in '%s'.\n",
	    capitalize(wizname),capitalize(gebiet)));
	return 1;
    }

    if(!lordp(this_interactive()))
        return notify_fail("Würdigungen können nur Lords setzen.\n");
    if(gebiet=="admin" && !adminp(this_interactive()))
        return notify_fail("Für Admin können nur Admins würdigen.\n");

    if(!strstr("neu",args[1]))
    {
	text = implode(args[3..<1]," ");
	if(text[0]=='\"' && text[<1]=='\"')
	    text = text[1..<2];
	if(!strlen(text))
	    return notify_fail("Es wurde kein Text angegeben.\n");
	back = who->add_wiz_appreciation(gebiet,text);
	if(stringp(back))
	    write(back);
	else {
	    if (playerp (who))
	        write("Würdigung erfolgreich eingetragen.\n");
	    else {
	        mixed res;
	        res = who->save_wiz_appreciations(wizname);
	        if (res != 1)
	            write (res);
	        else write ("Würdigung erfolgreich offline eingetragen.\n");
	    }
	}
	return 1;
    }
    if(!strstr("aendere",convert_umlaute(args[1])))
    {
	if(sizeof(args)==3)
	    return notify_fail("Es wurde keine Nummer angegeben.\n");
	if(str2int(args[3],&nr) || !nr)
	    return notify_fail("Fehlerhaftes Format der Nummer.\n");
	back=who->query_wiz_appreciations(0);
	for(i=0;nr>0 && i<sizeof(back);i++)
	    if(back[i][0]==gebiet)
	    {
		nr--;
		if(!nr) break;
	    }
	if(nr)
	{
	    write("Es gibt keine Würdigung mit dieser Nummer.\n");
	    return 1;
	}
	text = implode(args[4..<1]," ");
	if(strlen(text) && text[0]=='\"' && text[<1]=='\"')
	    text = text[1..<2];
	if(!strlen(text))
	    return notify_fail("Es wurde kein Text angegeben.\n");
	back=who->change_wiz_appreciation(i,text);
	if(stringp(back))
	    write(back);
	else {
	    mixed res;
	    res = who->save_wiz_appreciations(wizname);
	    if (res != 1)
	        write (res);
	    else write ("Würdigung erfolgreich offline geändert.\n");
	}
	return 1;
    }
    return notify_fail(sprintf("Unbekannter Befehl '%s'\n",args[1]));
}

int key_gouverneur(string str, string verb, object monster, object player,
    int flags)
{
    monster->do_command("sage Das Anlegen von Autoloader, Gilden, Spiele "
        "und Rätsel findet hinter Ausgang 'gouverneur' statt.");
}

int key_faehre_neu(string str, string verb, object monster, object player,
    int flags)
{
    monster->do_command("sage Eine neue Schiffsfähre muss hinter "
        "dem Ausgang 'gouverneur' als Fähre registriert sein, "
        "bevor es hinter Ausgang 'schiff' verwaltet werden kann.");
}

int key_wuerdigung(string str, string verb, object monster, object player,
    int flags)
{
    monster->do_command("sage Die Würdigungen, die im Fingertext von "
        "Göttern auftauchen, werden im Ausgang 'gouverneur' behandelt.");
}

mixed *query_keyword_rules()
{
    return ({
"key_gouverneur: [spiel] || [gild] || [raetsel] || [autoload]", 
        PARSE_SAY|PARSE_CONTINUE,
"key_faehre_neu: [faehr] || [schiff]", 
        PARSE_SAY|PARSE_CONTINUE,
"key_wuerdigung: [wuerd] || [finger]", 
        PARSE_SAY|PARSE_CONTINUE,
    });
}
