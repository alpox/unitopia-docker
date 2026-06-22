// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/room/rathaus/reserv.c
// Description:

inherit "/i/room";
inherit "/i/tools/security";

#include <level.h>
#include <apps.h>
#include <monster.h>
#include <more.h>
#include <config.h>

int query_prevent_shadow(object ob) { return 1; }

void reset()
{
    if (!present("bann-fibel",this_object()))
    {
        object ob;

        ob = clone_object("/obj/zeitschrift");
        ob->add_id(({"fibel","bann-fibel", "bannfibel", "buch"}));
        ob->set_name("Bann-Fibel");
        ob->set_gender("weiblich");
        ob->set_long("Ein Buch mit dem Titel: \"Bann-Fibel\"\n");
        ob->set_page_name("/doc/funktionsweisen/banish");
        ob->move(this_object());
   }
   if(!present("liste"))
	clone_object(abs_path("obj/minus_list"))->move(this_object());
    "/room/rathaus/div/lager"->get_moebel(this_object());
    "/room/rathaus/div/lager"->get_pflanze(this_object());
}

void create() {
    set_short("Raum der Beschwörung");
    set_long(
"Dies ist der Raum der Beschwörung. Hier vollziehen manche Götter ihre\n"+
"geheimnisvollen Riten oder gründen Geheimbünde. Am meisten wird dieser\n"+
"Raum jedoch zum Reservieren von Monsternamen verwendet.\n"+
"   Kommandos: reserviere <name> <Grund für Goetter> | <Grund für Spieler>\n"
"              (senkrechte Strich als Trennungszeichen)\n"
"              teste <wen>\n"
"              liste [<level> [<wer>]]\n"
"              banne <wen> <level> <grund>\n"
"              löse <wen> <level>\n"
"              finger <wer> <fingertext>\n");
    set_own_light(1);
    add_type("kunstlicht",1);
    add_type("teleport_rein_verboten", 1);
    set_exits(({"forum"}),({"forum"}));
    add_type("kaempfen_verboten",1);
    set_room_domain("Pantheon");
    init_security_for_actions();
    reset();
}

void init() {
    add_action("reserviere", "reserviere", -6);
    add_action("banne", "banne", -4);
    add_action("loese", "löse", -3);
    add_action("liste", "liste", -4);
    add_action("teste", "teste", -4);
    add_action("finger","finger");
}

int banne(string str)
{
   string name, grund;
   int level;

   if(!str || sscanf(str, "%s %d %s", name, level, grund) < 2)
   {
      notify_fail("banne <wen> <level>\n");
      return 0;
   }
   if(!check_security())
      return notify_fail("Banne wen?\n");
   if(!this_player() || this_player() != this_interactive() ||
      !present(this_player()) || !adminp(this_player()))
   {
      write("Du darfst diesen Befehl nicht benutzen.\n");
      return 1;
   }
   if(grund && !strlen(grund)) grund=0;
   if(BANISHD->banish(lower_case(name),level,this_player()->query_real_name(),grund))
      write("Das hat nicht geklappt.\n");
   else
      write("Ok.\n");
   return 1;
}

int loese(string str)
{
   string name;
   int level;

   if(!str || sscanf(str, "%s %d", name, level) != 2)
   {
      notify_fail("löse <wen> <level>\n");
      return 0;
   }
   if(!check_security())
      return notify_fail("Löse was?\n");
   if(!this_player() || this_player() != this_interactive() ||
      !present(this_player()) || !adminp(this_player()))
   {
      write("Du darfst diesen Befehl nicht benutzen.\n");
      return 1;
   }
   if(BANISHD->free(lower_case(name),level,this_player()->query_real_name()))
      write("Das hat nicht geklappt.\n");
   else
      write("Ok.\n");
   return 1;
}

int reserviere(string str)
{
    int i;
    string who, grund, *gruende;
    if(!wizp(this_player()))
    {
       write("DU???? Was suchst denn Du hier?\n");
       return 1;
    }
    if(!vogtp(this_player()))
    {
	write("Erst als Vogt kannst du hier Namen reservieren.\n");
	return 1;
    }
    if(!check_security())
        return notify_fail("Reserviere was?\n");
    if(!str || (!sscanf(str,"%s %s",who,grund) && !sizeof(who=str)))
    {
	write("Welchen Namen willst Du reservieren?\n");
	return 1;
    }
    if(strlen(who) > 10)
    {
	write("Einen so langen Namen kann sowieso kein Spieler haben.\n");
	return 1;
    }
    if(strlen(who) < 3)
    {
	write("Einen so kurzen Namen kann sowieso kein Spieler haben.\n");
	return 1;
    }
    for(i = strlen(who = lower_case(who)); i--;)
	if(who[i] < 'a' || who[i] > 'z')
	{
	   write("Sorry, ein Name kann nur Kleinbuchstaben enthalten. (a-z)\n");
	   return 1;
	}
    if(LOGIN_OB->guest(who))
    {
       write("Dieser Name ist für Gäste vorbehalten.\n");
       return 1;
    }
    if(player_exists(who) && !adminp(this_player()))
    {
	write("Dieser Name wird bereits von einem Spieler benutzt.\n");
	return 1;
    }
    if(grund && !strlen(grund)) grund=0;
    if (grund) {
	gruende = explode (grund," | ");
	if (sizeof (gruende) < 2)
	    gruende = explode (grund,"|");
	if (sizeof(gruende) > 2) {
	    write ("Verwende zum Trennen der Gründe für Götter und Spieler bitte *ein* \"|\".\n");
	    return 1;
	}
	gruende[0] = space(gruende[0]);
	if (sizeof (gruende) > 1) gruende[1] = space(gruende[1]);
    }
    else gruende = ({0,0});
    if (BANISHD->query_banished(who) <= 0) 
    {
        mixed b;
        if((b=BANISHD->query_banish()[0]) &&
	  ((b[who]==this_player()->query_real_name()) || adminp(this_player())) 
	  && grund)
	{
          if(BANISHD->banish(who, 0, this_player()->query_real_name(),
            gruende[0], sizeof(gruende) > 1 ? gruende[1] : 0))
	    write("Fehler beim Ändern des Kommentares zu "+capitalize(who)+".\n");
	  else
            write("Kommentar zu "+capitalize(who)+" geändert.\n");
          return 1;
	}
	write("Dieser Name ist bereits reserviert.\n");
	return 1;
    }
    BANISHD->banish(who, 0, this_player()->query_real_name(),
       gruende[0], sizeof(gruende) > 1 ? gruende[1] : 0);
    write("Der Name "+capitalize(who)+" ist nun reserviert.\n");
    return 1;
}

private varargs string liste_part(mapping banish, string wer)
{
   string *names, res, r, tmp;
   int i;

   names = m_indices(banish||([]));
   if(wer && strlen(wer))
       names=filter(names,(:$2[$1]==$3:),banish,lower_case(wer));
   names = sort_array(names,#'<);
   for(res = "", i = sizeof(names);
		 i--;)
   {
       r = capitalize(names[i])+" ("+capitalize(banish[names[i]])+")";
       if ((tmp=banish[names[i],2]) || banish[names[i],3])
           res+=sprintf("%s: %"+(74-strlen(r))+"-=s\n",r,
               (tmp?tmp:"")+((tmp=banish[names[i],3] )?" | "+tmp:""));
       else
           res+=r+"\n";
   }
   return res; // sprintf("%-79#s\n", res);
}

int liste(string str)
{
   mapping banish;
   int i, *lvls;

   string ret, res, wer;

   if(!check_security())
      return notify_fail("Liste was?\n");
      
   ret = "";
   banish = BANISHD->query_banish();

   if(str && sscanf(str, "%d %s", i, wer))
      ret += "---Level "+i+"---\n" + liste_part(banish[i],wer);
   else
      for(i = sizeof(lvls = sort_array(m_indices(banish), #'>)); i--;)
	 if((res = liste_part(banish[lvls[i]])) != "")
	    ret += "---Level "+lvls[i]+"---\n" + res;
   this_player()->more(explode(ret,"\n")-({""}),0,0,M_AUTO_END);
   return 1;
}

int teste(string str)
{
   int i;
   mapping banish;

   if(!check_security())
      return notify_fail("Teste was?\n");
      
   if(!str)
      return notify_fail("Teste wen?\n");

   str = lower_case(str);
   banish = BANISHD->query_banish();

   i = BANISHD->query_banished(str);
   switch(i)
   {
      case 0:
         if(!banish[i][str])
	   write("Der Name '"+capitalize(str)+"' ist verboten.\n");
	 else
	   write(wrap("Der Name '"+capitalize(str)+"' ist reserviert\nDurch: "
	   +capitalize(banish[i][str])+"\nGrund: "
	   +(banish[i][str,2]?banish[i][str,2]:"kein Grund angegeben")+"\nGrund für Spieler: "
	   +(banish[i][str,3]?banish[i][str,3]:"kein Grund angegeben")+"\n"));
	 break;
      case 1..LVL_MAX_LEVEL-1:
	 write(wrap(capitalize(str)+" ist für alle Level >= "+i+
	 " gesperrt. ("+capitalize(banish[i][str])+
	 (banish[i][str,2]?": "+banish[i][str,2]:"")+")"));
	 break;
      default:
	 write(capitalize(str)+" ist nicht gebannt.\n");
   }
   str = lower_case(str);
   if(player_exists(str))
      write(capitalize(str)+" ist ein existierender Spielercharakter.\n");
   else
      write(capitalize(str)+" ist KEIN Spielercharakter.\n");
   return 1;
}

int finger(string str)
{
   string name;
   string text;

   if(!str || (sscanf(str, "%s %s", name, text) != 2))
   {
      notify_fail("finger <wer> <text>\n");
      return 0;
   }

   if(!check_security())
      return notify_fail("Finger wen?\n");
      
   if(!this_player() || this_player() != this_interactive() ||
      !present(this_player()) || !adminp(this_player()))
   {
      write("Du darfst diesen Befehl nicht benutzen.\n");
      return 1;
   }
   if(BANISHD->set_special_finger(lower_case(name),text) != 1)
      write("Das hat nicht geklappt.\n");
   else
      write("Ok.\n");
   return 1;

}

int key_reserv(string str, string verb, object monster, object player,
    int flags)
{
    monster->do_command("sage Die Reservierung von Monster-Namen sowie "
        "die Admin-Anpassung des Fingertextes geschieht im Ausgang 'Bann'.");
}

int key_bann(string str, string verb, object monster, object player,
    int flags)
{
    monster->do_command("sage Das Bannen/Daemmern von Spielern und Göttern "
        "kann im Ausgang 'Bann' vorgenommenn werden.");
}

int key_reserv_teleport(string str, string verb, object monster, object player,
    int flags)
{
    monster->do_command("sage Die Teleportliste, welche Götter "
        "teleportiert haben, liegt im Ausgang 'Bann'.");
}

int key_arma(string str, string verb, object monster, object player,
    int flags)
{
    monster->do_command("sage Wenn inaktiv ist Armageddon hinter Ausgang "
        "'bann', wenn aktiv im Raum /room/church (Auch per 'zg armageddon' "
        "zu erreichen).");
}

mixed *query_keyword_rules()
{
    return ({
"key_reserv: reserviere || npcname || monstername || [finger] ", 
        PARSE_SAY|PARSE_CONTINUE,
"key_bann: bann || [daemm]", 
        PARSE_SAY|PARSE_CONTINUE,
"key_reserv_teleport: [teleport]", 
        PARSE_SAY|PARSE_CONTINUE,
"key_arma: [arma] || neustart", 
        PARSE_SAY|PARSE_CONTINUE,
    });
}
