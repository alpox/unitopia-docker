// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/apps/banishd.c
// Description: Der banish daemon verwaltet die gesperrren Namen
// Author:	Garthan	(29.07.94)
// Modified by:	Freaky (28.04.2000) BANISH_STRSTR, parse_com-Worte eingebaut

// Userid fuer  /apps/banishd setzen:
// UID: Apps


// Zur verwaltung der Namen wird ein geschachteltes Mapping der Form
// 
// ([ level : ([ name : vonwem; wann; begruedung,
//		 name2 : vonwem; wann; begruendung, ... ])
//    level : ... ])
// verwendet.

#include <level.h>
#include <uids.h>

#undef CONVERT
#define SAVE_FILE "/var/banishd"

#define BANISH_CONTROL "/room/rathaus/reserv"

// Das sind Wortteile, die NICHT im Namen vorkommen duerfen
#define BANISH_STRSTR ({ "fuck", "fick", "schwein", "sex" })

mapping banish = ([]);
mapping special_finger = ([]);

void create()
{
   mixed conv;
   restore_object(SAVE_FILE);
   if(sizeof(conv=m_values(banish)) && widthof(conv[0])==3)
     banish=map(banish,(:m_reallocate($2,4):));
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

int banish(string name, int level, string vonwem, string grundwiz, string grundplayer)
{
   if(object_name(previous_object()) != BANISH_CONTROL &&
      geteuid(previous_object()) != ROOT_UID)
      return -1;
   if(!stringp(name) || strlen(name) > 10)
      return 1;
   if(!stringp(vonwem))
      return 2;
   if(!banish[level])
      banish[level] = m_allocate(1,4);
   else if(banish[level][name])
   {
      if (banish[level][name] == vonwem || adminp(this_interactive())) {
         if(grundwiz)
            banish[level][name,2] = grundwiz;
         if(grundplayer)
            banish[level][name,3] = grundplayer;
         if(grundwiz||grundplayer)
	    save();
	 return 0;
      }
      return 3;
   }
   banish[level][name] = vonwem;
   banish[level][name,1] = time();
   banish[level][name,2] = grundwiz;
   banish[level][name,3] = grundplayer;
   save();
}

int free(string name, int level)
{
   if(object_name(previous_object()) != BANISH_CONTROL)
      return -1;
   if(!stringp(name) || strlen(name) > 10)
      return 1;
   if(!member(banish, level) || !member(banish[level], name))
      return 3;
   m_delete(banish[level], name);
   if(!sizeof(banish[level]))
      m_delete(banish, level);
   save();
}

mapping query_banish()
{
   return copy(banish);
}

varargs int query_banished(string name, string grund)
{
   int *levels, i;

   if(!stringp(name))
      return LVL_MAX_LEVEL+1;

   // Alle Teilworte checken
   for (i = sizeof(BANISH_STRSTR); i--; )
       if (strstr(name,BANISH_STRSTR[i]) >= 0)
	   return 0;

   // Alle durch parse_com() reservierten Worte checken
   if (member(query_pc_meinesgleichen(),name) >= 0)
       return 0;
   if (member(query_pc_artikel(),name) >= 0)
       return 0;
   if (member(query_pc_dinge(),name) >= 0)
       return 0;
   if (member(query_pc_alles(),name) >= 0)
       return 0;

   // Jetzt die 'wirklich' reservierten Namen checken
   levels = sort_array(m_indices(banish), #'>);
   for(i = 0; i < sizeof(levels); i++)
      if(member(banish[levels[i]], name)) {
         grund = banish[levels[i]][name,3];
	 return levels[i];
      }

   return LVL_MAX_LEVEL+1;
}

string query_special_finger (string name)
{
    return special_finger [name];
}

int set_special_finger (string name, string text)
{
   if (object_name(previous_object()) != BANISH_CONTROL &&
      geteuid(previous_object()) != ROOT_UID)
      return -1;
   if (!adminp (this_interactive ()) || !name)
      return -1;
   if (!text) text = name + " bewohnte einst UNItopia.";
   special_finger [name] = text;
   return 1;
}


#ifdef CONVERT

static string *names;
string banished_by;

void do_one(int i)
{
   int ft;
   string name;

   banished_by = "leo";
   for(; i < sizeof(names); i++)
   {
      restore_object("/banish/"+names[i][0..<3]);
      if(banished_by) 
	 banished_by = lower_case(banished_by);
	 
      ft = file_time("/banish/"+names[i]);
      name = names[i][0..<3];
      if(!banish[0])
	 banish[0] = m_allocate(1,2);
      banish[0][name] = banished_by;
      banish[0][name,1] = ft;
      if(get_eval_cost() < 100000)
      {
	 call_out("do_one", 3, i+1);
	 return;
      }
   }
   save();
}

void convert()
{
   names = get_dir("/banish/.");
   do_one(0);
}

#endif
