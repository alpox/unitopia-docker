// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /room/rathaus/npcfinger.c
// Description: Diese Objekt regelt die Ausgabe der Fingertexte von NPCs
// Author:      Jesaia (04.12.1997)
// Modified by: Jesaia (21.01.1998) - Personal_title eingebaut,
//                                    und nen kleinen ED, abgeguckt
//                                    aus Parsecs fehler.c

#include <level.h>
#include <monster.h>
#include <more.h>
#include <editor.h>

#undef NPC_FINGER
#define NPC_FINGER touch("/apps/npcfinger")

inherit "/i/room";

void reset()
{
    "/room/rathaus/div/lager"->get_moebel(this_object());
    "/room/rathaus/div/lager"->get_pflanze(this_object());
}

void create()
{
     set_short("Der Raum der Persönlichkeiten");
     set_long("Das ist der große Raum der Persönlichkeiten. Domainlords "
              "können hier die Bewohner ihrer Domain eintragen, die via "
              "Fingerbefehl zu erreichen sein sollen:\n"
              "       nanu        : Wie ich vorzugehen habe.\n"
              "       name        : rufname domain          \n"
              "       title       : name title              \n"
              "       ptitle      : name ptitle             \n"
              "       default     : name defaulttext        \n"
              "       nurdefault  : name (ja/nein)          \n"
              "       filename    : name filename           \n"
              "       lösche      : name                    \n\n"
              "       übersicht   : (Domain)                \n"
	      "       komplett    : (Domain)                \n"
              "       detail      : name                   ");
     add_type("kunstlicht",1);
     add_type("teleport_rein_verboten",1);
     set_exit("forum","forum");
     set_own_light(1);
     reset();
}

void init()
{
     add_action("nanu","nanu");
     add_action("name","name");
     add_action("ptitle","ptitle");
     add_action("title","title");
     add_action("defa","default");
     add_action("nurdefault","nurdefault");
     add_action("filename","filename");
     add_action("uebers","übersicht");
     add_action("komplett","komplett");
     add_action("einzel","detail");
     add_action("loesche","lösche");
}

string edit_default_text(string def_name)
{
    this_player()->mini_ed(lambda(({'str}),({
	(:
	    if(!$1)
	    {
    		write("Der default text wurde nicht gespeichert.");
    		return;
	    }
	    
	    if(!sizeof($1))
	    {
        	write(
            	    "Du hast keinen Text eingegeben. "
            	    "Versuch's nochmal.\n");
        	edit_default_text($2);
		return;
	    }
	    
            write(NPC_FINGER->set_npc_default($2,
		implode($1,"\\n")+"\\n")+"\n");
	:), 'str, def_name})), 0, 0,
	([
	    MINI_ED_START_TEXT:
        	"Gib nun den Defaulttext zu "+capitalize(def_name)+" ein\n"
		"Speichern: '**' oder '.', Abbrechen: '~q'.",
        MINI_ED_TITLE: "NPC Defaulttext",
	]));
}

int nanu()
{
  write(wrap("Was man hier so tun und lassen kann:\n"
        "------------------------------------\n"
        "Dieser Raum ist dafür konzipiert, das wichtige NPCs durch den Finger-"
        "Befehle zu erreichen sind. Wichtig ist dabei, das der Name reserviert ist, "
        "da die NPC Abfrage im Fingerbefehl die letzte ist. Mit dem Befehl name "
        "kann ein DL den Namen eines NPC hier eintragen, damit der Fingerbefehl "
        "dann aber auch anspricht müssen sämtliche anderen einträge bis auf "
        "filename gemacht sein.\nMit title und ptitle legt man die Kurzbezeichnung "
        "des NPC fest. Ist ein Filename angegeben und der NPC geladen, wird die " 
        "short des geladenen NPCs aber verwendet. Mit default gibt man den "
        "default fingertext ein, der kommt wenn ein NPC nicht geladen ist. "
        "Zeilenumbrüche werden mit \\n gesetzt, der fingerbefehl analysiert "
        "selbstständig den String. Ist nurdefault auf ja "
        "gesetzt wird, nur dieser text genommen. Bei NPCs wie Achronos und "
        "Hotzenplotz ist es ja nicht sehr sinnvoll, wenn der Spieler erkennt ob "
        "ein NPC nicht geladen ist. :)\nMit filename gibt man den Filename des "
        "NPCs an. Wenn nurdefault auf nein gesetzt ist, wird in diesem File die "
        "Funktion <query_finger_info_text()> gesucht und bei erfolg ausgeben. "
        "So ist es möglich das der Fingertext sich bei geladenen NPC dynamisch "
        "ändern kann.\nübersicht und einzel sind wohl klar.\nEine Domain sollte "
        "wirklich nur die prominenten NPC eingetragen haben, NPCs die für die "
        "Domain stimmungsmäßig wichtig sind."));
  return 1;
}
int name(string str)
{
     mixed *temp;
     if (!str || str =="")
       {
          notify_fail("Bitte Namen und Domain angeben.\n");
          return 0;
        }
     temp = explode(str," ");
     if (sizeof(temp) !=2)
         {
          notify_fail("Bitte Namen und Domain angeben.\n");
          return 0;
        }
      write(NPC_FINGER->add_npc_name(temp[0],
                          capitalize(lower_case(temp[1]) ))+"\n" );
    return 1;
}

int loesche(string str)
{
     mixed *temp;
     if (!str || str =="")
       {
          notify_fail("Bitte Namen angeben.\n");
          return 0;
        }
     temp = explode(str," ");
     if (sizeof(temp) !=1)
         {
          notify_fail("Bitte nur den Rufnamen angeben.\n");
          return 0;
        }
      write(NPC_FINGER->del_npc_name(temp[0])+"\n" );
    return 1;
}

int title(string str)
{
     mixed *temp;
     if (!str || str =="")
       {
          notify_fail("Bitte Namen und Titel angeben.\n");
          return 0;
        }
     temp = explode(str, " ");
     if (sizeof(temp) < 2)
        {
          notify_fail("Bitte Namen und Titel angeben.\n");
          return 0;
        }
      write(NPC_FINGER->set_npc_short(temp[0],implode(temp[1..]," "))+"\n" );
      return 1;
}
int ptitle(string str)
{
     mixed *temp;
     if (!str || str =="")
       {
          notify_fail("Bitte Namen und Personaltitel angeben.\n");
          return 0;
        }
     temp = explode(str, " ");
     if (sizeof(temp) < 2)
	  temp += ({""});
      write(NPC_FINGER->set_npc_ptitle(temp[0],implode(temp[1..]," "))+"\n" );
      return 1;
}
int defa(string str)
{
     mixed *temp;
     if (!str || str =="")
       {
          notify_fail("Bitte Namen und Defaulttext angeben.\n");
          return 0;
        }
     temp = explode(str, " ");
     if (sizeof(temp) < 2)
       {
          if(NPC_FINGER->name_check(temp[0]))
            {
              edit_default_text(temp[0]);
              return 1;
            }
          else
            {
              notify_fail("Diesen Namen gibt es noch nicht.\n");
              return 0;
            }
        }
      write(NPC_FINGER->set_npc_default(temp[0],implode(temp[1..]," "))+"\n" );
      return 1;
}

int nurdefault(string str)
{
     mixed *temp;
     if (!str || str =="")
       {
          notify_fail("Bitte Namen und ja oder nein angeben.\n");
          return 0;
        }
     temp = explode(str, " ");
     if (sizeof(temp) !=2)
        {
          notify_fail("Bitte Namen und ja oder nein angeben.\n");
          return 0;
        }
      if ((lower_case(temp[1]) != "ja") && (lower_case(temp[1]) != "nein"))
          {
             notify_fail("Bitte ja oder nein angeben.\n");
             return 0;
          }
      
      write(NPC_FINGER->set_npc_only_default(temp[0],
                          (lower_case(temp[1])== "ja"?1:0))+"\n" );
      return 1;
}
int filename(string str)
{
     mixed *temp;
     if (!str || str =="")
       {
          notify_fail("Bitte Namen und Filename angeben.\n");
          return 0;
        }
     temp = explode(str, " ");
     if (sizeof(temp) !=2)
       {
          notify_fail("Bitte Namen und Filename angeben.\n");
          return 0;
        }
      write(NPC_FINGER->set_npc_filename(temp[0],temp[1])+"\n" );
      return 1;
}

int uebers(string str)
{
      mixed *temp=({}),*names, *doms=({});
      mapping npcs;
      int x;
      names = NPC_FINGER->query_npc_names();
      npcs = NPC_FINGER->query_finger_npc();
      names=sort_array(names,(:$3[$1]["domain"]<$3[$2]["domain"] ||
          ($3[$1]["domain"]==$3[$2]["domain"] && $1<$2):),
          npcs);
                               
      for(x=sizeof(names);x--;)
        {
            if (str && (str != "") &&
               (strstr(lower_case(npcs[names[x]]["domain"]),lower_case(str)) ==-1))
                continue;
	    if(member(doms,npcs[names[x]]["domain"])<0)
	       doms+=({npcs[names[x]]["domain"]});
	    temp+=explode(sprintf(" %12-=s %14-=s %50-=s\n",
	       npcs[names[x]]["domain"], names[x],
	       npcs[names[x]]["short"]||" - "),"\n")[0..<2];
	}
       if (!sizeof(doms))
          return notify_fail("Domain "+str+" hat keine Einträge.\n");
       temp=({" Domain       Name           Titel",
              " ------------ -------------- --------------------------------------------------"})+
	    temp;  
       if (str)
         this_player()->more(({"Wichtige NPC der Domain"+
	 ((sizeof(doms)>1)?"s ":" ")+liste(doms)+":",""})+
                              temp,0,0,M_AUTO_END);
       else
         this_player()->more( ({"Übersicht über alle NPCs:",""})+temp,
	    0,0,M_AUTO_END);
       return 1;
}

int komplett(string str)
{
      mixed *temp,*temp2, *names, *doms=({});
      mapping npcs;
      int x;
      object ob;
      temp =({});
      names = NPC_FINGER->query_npc_names();
      npcs = NPC_FINGER->query_finger_npc();
      names=sort_array(names,(:$3[$1]["domain"]<$3[$2]["domain"] ||
          ($3[$1]["domain"]==$3[$2]["domain"] && $1<$2):),
          npcs);
      for(x=sizeof(names);x--;)
        {
            if (str && (str != "") &&
               (strstr(lower_case(npcs[names[x]]["domain"]),lower_case(str)) ==-1))
                continue;
	    if(member(doms,npcs[names[x]]["domain"])<0)
	       doms+=({npcs[names[x]]["domain"]});
            temp += ({ "- Name        : "+names[x]+"    Domain: "+
                       npcs[names[x]]["domain"] });
            temp += ({ "- Short ges.  : "+npcs[names[x]]["short"] });
            if (member(npcs[names[x]],"ptitle") && !(npcs[names[x]]["ptitle"]==""))
              temp += ({"- Ptitle ges.  : "+npcs[names[x]]["ptitle"]});
            temp += ({ "- default ges.:"});
            temp2 = explode(npcs[names[x]]["default"],"\\n");
            if(temp2 != ({}) )
                 temp += temp2;
            temp += ({ "- nur default : "+(npcs[names[x]]["only_default"]?
                                           "ja":"nein") });
            temp += ({ "- Filename    : "+npcs[names[x]]["file"] });
            if(ob = find_object(npcs[names[x]]["file"]) )
              {
               temp += ({ "- FILE ist gerade geladen." });
               if (living(ob))
                  {
                     temp += ({"- FILE ist auch NPC."});
                     temp += ({"- Short des NPC: "+ob->query_short()});
                  }
                else
                  {
                     temp += ({"- FILE ist nicht NPC."});
                  }
                if(ob->query_finger_info_text())
                  {
                     temp += ({ "- FILE_FINGERTEXT:\n"});
                     temp2=explode(ob->query_finger_info_text(),"\n");
                     temp += temp2;
                  }
                else
                   temp += ({ "- Im File ist kein Fingertext definiert."});
             }
            else
              temp+= ({ "- FILE ist nicht geladen"});
            temp += ({ "----------------------------------->><<-------------------"
                       "------------" });
       }
       if (!sizeof(doms))
          return notify_fail("Domain "+str+" hat keine Einträge.\n");
       if (str)
         this_player()->more(({"Wichtige NPC der Domain"+
	 ((sizeof(doms)>1)?"s ":" ")+liste(doms)})+
                              ({"---------------------------------------"})+
                              temp,0,0,M_AUTO_END);
       else
         this_player()->more( ({"Übersicht über alle NPCs"})+
                              ({"--------------------------"})+temp,0,0,
			      M_AUTO_END);
       return 1;
}

int einzel(string name)
{
     mapping npc;
     mixed *temp,temp2;
     object ob;
     if (!name && name ="")
       {
         notify_fail("Bitte einen Rufnamen angeben.\n");
         return 0;
       }
     if (!mappingp(npc=NPC_FINGER->query_info_of_npc(name)) )
        {
           notify_fail(name+" ist unbekannt.\n");
           return 0;
        }
     temp2 ="";
     if (member(npc,"ptitle") && !(npc["ptitle"]==""))
         temp2 = "- Ptitle ges.  : "+npc["ptitle"]+" \n";
      write("Infos über "+name+":\n"+
            "- Name        : "+capitalize(lower_case(name))+"    Domain: "+
                       npc["domain"] +"\n"+
            "- Short ges.  : "+npc["short"]+"\n"
            +temp2+
            "- default ges.:\n");
      temp = explode(npc["default"],"\\n");
      if(temp != ({}) )
         write(implode(temp,"\n")+"\n");
      write("- nur default : "+(npc["only_default"]?"ja\n":"nein\n")+
            "- Filename    : "+npc["file"]+"\n");
      if(ob = find_object(npc["file"]) )
         {
          write( "- FILE ist gerade geladen.\n" );
          if (living(ob))
             {
               write("- FILE ist auch NPC.\n"+
                     "- Short des NPC: "+ob->query_short()+"\n");
                  }
                else
                  {
                     write("- FILE ist nicht NPC.\n");
                  }
                if(ob->query_finger_info_text())
                  {
                     write("- FILE_FINGERTEXT:\n"+
                     ob->query_finger_info_text()+"\n");
                  }
                else
                   write("- Im File ist kein Fingertext definiert.\n");
             }
         else
             write("- FILE ist nicht geladen.\n");
 
       return 1;
}

int key_npcfinger(string str, string verb, object monster, object player,
    int flags)
{
    monster->do_command("sage Der Fingertext für NPCs können von DLs "
        "in 'npcfinger' angepasst werden.");
}

mixed *query_keyword_rules()
{
    return ({
"key_npcfinger: [finger] || [npc] || [name]",
        PARSE_SAY|PARSE_CONTINUE,
    });
}
