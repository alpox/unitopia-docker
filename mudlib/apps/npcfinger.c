// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /apps/npcfinger.c
// Description: Diese Objekt regelt die Ausgabe der Fingertexte von NPCs
// Author:      Jesaia (04.12.97)

// UID: Apps

#include <level.h>
#include <apps.h>
#include <gilden.h>

#define SAVE_FILE "/var/npcfinger"
#define CAP(x) capitalize(x)

mapping npc_finger =([]);

void create()
{
    restore_object(SAVE_FILE);
}

void save()
{
    save_object(SAVE_FILE);
}

void remove()
{
     save();
     destruct(this_object());
}

void abort_renewal() {}
void finish_renewal(object neu) {}
void prepare_renewal() { save(); }

int secure_check()
{
    // Lords, Gildengoetter, DHs und Admins.
      return playerp(this_player()) && this_player() == this_interactive() &&
    	    (((lordp(this_player()) || 
               GILDEN_OB->query_programmed_gilden(this_player()->query_real_name()) ||
               sizeof(DOMAIN_INFOS->query_domainhelfer_of(this_player()->query_real_name()))) &&
		(object_name(previous_object()) == "/room/rathaus/npcfinger")) || 
	      adminp(this_player()));
}

int auth_check(string domain)
{
     return ( DOMAIN_INFOS->domain_lord(domain,
                this_player()->query_real_name()) 
             || DOMAIN_INFOS->domain_helfer(domain,
                this_player()->query_real_name())
	     || member(FILED->query_auth(lower_case(domain)),
	        this_player()->query_real_name())>=0
             || adminp (this_player()) );
}

int name_check(string name)
{
      return member(npc_finger,name);
}

string add_npc_name(string name, string domain)
{
       name = lower_case(name);
       if (!secure_check())
           return "Dazu hast du nicht die Berechtigung.";
       if(name_check(name) > 0)
          return "Dieser Name ist schon belegt.";
       if (!auth_check(domain))
           return "In dieser Domain bist du nicht zuständig.";
       if (sizeof(explode(name," "))>1)
          return "Der Name darf keine Leerzeichen (SPACE) haben";
       npc_finger += ([ lower_case(name): 
                     ([ 
                       "domain":domain,
                       "file":"",
                       "short":"",
                       "only_default":1,
                       "default":"",
                       "ptitle":""]) 
                     ]);
       save();
       return "Name: " + CAP(lower_case(name))+" gesetzt.";
}
string del_npc_name(string name)
{
       name = lower_case(name);
       if (!secure_check())
           return "Da zu hast du nicht die Berechtigung.";
       if(name_check(name) == 0)
          return "Diesen Namen gibts gar nicht.";
       if (!auth_check(npc_finger[name]["domain"]))
           return "In dieser Domain bist du nicht zuständig.";
       if (sizeof(explode(name," "))>1)
          return "Der Name darf keine Leerzeichen (SPACE) haben";
       m_delete(npc_finger,name);
       save();
       return "Name: " + CAP(lower_case(name))+" gelöscht.";
}


string set_npc_short(string name, string short)
{
       name = lower_case(name);
       if (!secure_check())
           return "Da zu hast du nicht die Berechtigung";
       if(name_check(name) == 0)
          return "Diesen Namen gibts nicht.";
       if (!auth_check(npc_finger[name]["domain"]))
           return "In dieser Domain bist du nicht zuständig.";
       npc_finger[name]["short"]=short;
       save();
       return "Titel: "+short+" wurde für Name: " +name+" gesetzt.";
}

string set_npc_ptitle(string name, string ptitle)
{
       name = lower_case(name);
       if (!secure_check())
           return "Da zu hast du nicht die Berechtigung";
       if(name_check(name) == 0)
          return "Diesen Namen gibts nicht.";
       if (!auth_check(npc_finger[name]["domain"]))
           return "In dieser Domain bist du nicht zuständig.";
       if (member(npc_finger[name],"ptitle"))
              npc_finger[name]["ptitle"]=ptitle;
          else
	    npc_finger[name]+= (["ptitle":ptitle]);
       save();
       return strlen(ptitle)
           ?("Ptitle: "+ptitle+" wurde für Name: " +name+" gesetzt.")
	   :("Ptitle wurde für Name: "+name+" gelöscht.");
}

string set_npc_default(string name, string defa)
{
       name = lower_case(name);
       if (!secure_check())
           return "Da zu hast du nicht die Berechtigung";
       if(name_check(name) == 0)
          return "Diesen Namen gibts nicht.";
       if (!auth_check(npc_finger[name]["domain"]))
           return "In dieser Domain bist du nicht zuständig.";
       npc_finger[name]["default"]=defa;
       save();
       return "default: "+defa+" wurde für Name: " +name+" gesetzt.";
}

string set_npc_only_default(string name, int odefa)
{
       name = lower_case(name);
       if (!secure_check())
           return "Da zu hast du nicht die Berechtigung";
       if(name_check(name) == 0)
          return "Diesen Namen gibts nicht.";
       if (!auth_check(npc_finger[name]["domain"]))
           return "In dieser Domain bist du nicht zuständig.";
       npc_finger[name]["only_default"]=odefa;
       save();
       return "Only_default: "+odefa+" wurde für Name: " +name+" gesetzt.";
}
string set_npc_filename(string name, string file)
{
       name = lower_case(name);
       if (!secure_check())
           return "Da zu hast du nicht die Berechtigung";
       if(name_check(name) == 0)
          return "Diesen Namen gibts nicht.";
       if (!auth_check(npc_finger[name]["domain"]))
           return "In dieser Domain bist du nicht zuständig.";
       if (npc_finger[name]["domain"]!="Pantheon" &&
           strstr(file,"/"+npc_finger[name]["domain"]+"/") == -1)
           return "Das File liegt nicht in der Domain des Namens";
       npc_finger[name]["file"]=file;
       if (file_size(file) < 1)
           return file +" nicht gefunden.";
       save();
       return "File: "+file+" wurde für Name: " +name+" gesetzt.";
}

mapping query_finger_npc()
{
    return deep_copy(npc_finger);
}
mixed *query_npc_names()
{
     return m_indices(npc_finger);
}

mixed query_info_of_npc(string name)
{
       name = lower_case(name);
       if(name_check(name) == 0)
          return -1;
       return copy(npc_finger[name]);
}
