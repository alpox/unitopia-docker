// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/apps/hlp_orte.c
// Description:
// Author:	Garthan

// UID: Apps

#include <apps.h>
#include <level.h>
#define ORTE "/var/HLP_ORTE"
#define TELEPORT "/room/rathaus/teleport"

string *orte = ({});

void create()
{
   string file;

   if(stringp(file = read_file(ORTE)))
      orte = regexp(explode(file, "\n"),"^[^#]");
}

void save()
{
    rm(ORTE);
    write_file(ORTE,
    "# ORTE -  Filenamen von Orten, an die Engel fliegen dürfen, \n"
    "#         wenn sie schon einmal dort waren. Dieses File wird \n"
    "#         automatisch von "+object_name()+" erzeugt.\n"
    "# Last Update: "+shorttimestr(time())+"\n"
    "#\n"
    "# Die Reihenfolge der Einträge ist wesentlich!!!\n"
    "# Keine Zeilen einfügen oder löschen!!!\n"
    "# Eintrage mit einem - werden wiederverwendet.\n"
    "#\n"+
    implode(orte,"\n")+"\n");
}

int query_ort(string ort)
{
   return member(orte, ort)+1;
}

// Testfunktion, dont use it!
string *query_orte()
{
   return ({})+orte;
}

int add_ort(string ort)
{
   int wo;

   if(!ort || strstr(ort,"#") >= 0)
      return 0;
   if((wo = member(orte, ort)) >= 0)
      return wo+1;
   if(!TELE_MASTER->is_tele(ort))
   {
      if(this_player() && wizp(this_player()))
	 write("Dieser Ort ist nicht in der Teleportliste eingetragen.\n"
	       "'inherit \"/i/hlp/room\";' hat so keinen Sinn.\n");
      return 0;
   }
   if((wo = member(orte, "-")) >= 0)
   {
      orte[wo] = ort;
      save();
      return wo+1;
   }
   orte += ({ ort });
   save();
   return sizeof(orte);
}

private int secure()
{
   return object_name(previous_object()) == TELEPORT;
}


varargs int delete_ort(string ort, int noreuse)
{
   int wo;

   if(!secure())
      return -10;

   if(ort && (wo = member(orte, ort)) >= 0)
   {
      orte[wo] = noreuse ? "0" : "-";
      save();
      return wo+1;
   }
}

int change_filename(string old, string new)
{
   int i;

   if(!secure())
      return -10;
   if(!stringp(old) || !stringp(new))
      return -1;
   if(!TELE_MASTER->is_tele(new))
      return -2;
   for(i = sizeof(orte); i--;)
      if(orte[i] == old)
         orte[i] = new;
   save();
}

string *check_orte()
{
   string *res;
   int i;

   for(res = ({}), i = sizeof(orte); i--;)
      if(orte[i][0..0] == "/" && !TELE_MASTER->is_tele(orte[i]))
         res += orte[i..i];
   return res;
}

int clean_orte()
{
   int i;

   if(!secure())
      return -10;

   for(i = sizeof(orte); i--;)
      if(orte[i][0..0] == "/" && !TELE_MASTER->is_tele(orte[i]))
         delete_ort(orte[i]);
   save();
}
