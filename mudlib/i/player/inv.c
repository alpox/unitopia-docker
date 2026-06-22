// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/i/player/inv.c
// Description: Inventory
// Modified by: Garthan (18.01.1994) eye_option: "v_items"
//		Freaky  (29.03.1998) inv: Waffen vor Behaelter anzeigen
//		Sissi   (23.04.1998) inv: Gildentools werden auch als
//			                  Autoloader betrachtet.

#pragma save_types
#pragma strong_types

private functions inherit "/i/tools/kleidung";

#include <control.h>
#include <invis.h>
#include <eyes.h>
#include <more.h>
#include <strings.h>
#include <term.h>
#include <time.h>
#include <message.h>

nomask int query_wiz_level();

#define PREFIX "--= "
#define SUFFIX " =--"
#define REPLACE "        "
#define INDENT "  "
#define EXTERN(x) lambda(({'ob}),({#'call_other,'ob, x}))

// #define VI_ENV(x) (mappingp(x) ? x["environment"] : environment(x))

private mapping eye_option =
([
   // == ALL ==

   // "kurz"     : 0,

   // == WIZ == 

      "file"     : 1,
      "nolist"   : 1,
      "hidden"   : 1,
      "locked"   : 1,
      "roominv"  : 1,
      "myinv"    : 1,
      "otherinv" : 1,
   // "invstyle" : 0,
      "v_items"  : 1,
      "dirinfo"  : 1,
]);

private static mapping eye_option_text =
// "name": "Beschreibung"; 1 wenn String/0 wenn Zahl
([
   "kurz"  :    "Langbeschreibung (0) oder kurz (1) oder beides (2) bei Bewegung"; 0, 
   "file"  :    "Anzeige des Pfades der Räume"; 0,
   "nolist":    "Anzeigen von NoList Ausgängen"; 0,
   "hidden":    "Anzeigen von versteckten Ausgängen"; 0,
   "locked":    "Anzeigen von gesperrten Ausgängen"; 0,
   "roominv":   "Anzeigen von unsichtbaren Gegenständen in Räumen"; 0,
   "myinv" :    "Anzeigen von versteckten Gegenständen "
		"in Ausrüstung"; 0,
   "otherinv":  "Anzeigen von versteckten Gegenständen bei 'schau an'"; 0,
   "invstyle":  "Voreinstellungen des 'ausruestung' Befehls"; 1,
   "v_items":   "Anzeigen von v_items"; 0,
   "dirinfo":   "Anzeigen der .info - Datei beim Wechseln in ein Verzeichnis (cd)"; 0,
   "shadows":	"Anzeigen aller auf dem Objekt liegenden Shadows"; 0,
   "debug":	"Debugmeldungen in Raumbeschreibungen anzeigen"; 0,
   "roomorder":	"Rauminhalt vor/nach Beschreibung anzeigen"; 0,
]);

/*
FUNKTION: set_eye_option
DEKLARATION: void set_eye_option(string option, mixed value)
BESCHREIBUNG:
Setzt die Augenoption 'option' auf den Wert 'value'.
Folgende Optionen (Defines aus eyes.h) gibt es:
(Die Ja/Nein-Optionen koennen den Wert 0 (nein) oder 1 (ja) erhalten.)

    EYE_DIRINFO		Anzeige der .info-Datei bei 'cd'.
    EYE_FILE		Anzeige des Objektnamens beim Betrachten
    EYE_KURZ		Lang- (=0), Kurz- (=0) oder beide (2) Beschreibungen
    EYE_HIDDEN_EXITS	Anzeige versteckter Ausgaenge
    EYE_NOLIST_EXITS	Anzeige von NO_LIST-Ausgaengen
    EYE_LOCKED_EXITS	Anzeige gesperrter Ausgaenge
    EYE_ROOM_INVIS	Anzeige von unsichtbaren Gegenstaenden im Raum
    EYE_MY_INVIS	Anzeige eigener unsichtbarer Gegenstaende
    EYE_OTHER_INVIS	Anzeige fremder unsichtbarer Gegenstaende
    EYE_INVSTYLE	Optionen des invis-Befehls
    EYE_V_ITEMS		Anzeige der V-Item-Liste
    EYE_SHADOWS		Anzeige von Shadows
    EYE_DEBUG		Anzeige von Debugtexten in der Beschreibung
    EYE_ROOM_ORDER	Anzeige des Rauminventars vor der Beschreibung
VERWEISE: query_eye_option, set_eye_level
GRUPPEN: spieler, augen
*/
void set_eye_option(string option, mixed value)
{
   if(!eye_option)
      eye_option = ([]);
   if(value)
      eye_option[option] = value;
   else
      m_delete(eye_option, option);
}

/*
FUNKTION: query_eye_option
DEKLARATION: varargs mixed query_eye_option(string option)
BESCHREIBUNG:
Liefert den Wert einer bestimmten Option oder aller Optionen in einem
Mapping (falls option nicht angegeben wurde). Eine Liste aller
Augenoptionen gibt es bei 'set_eye_option'.
VERWEISE: set_eye_option, query_eye_level
GRUPPEN: spieler, augen
*/
varargs mixed query_eye_option(string option)
{
   if(!eye_option)
      eye_option = ([]);
   if(!option)
      return copy(eye_option);
   return eye_option[option];
}

private int is_visible(object ob)
{
   return !(ob->query_invis() & V_ATOM_NOLIST);
}

private string ltrim(string str)
{
   return str[0..37];
}

/*
FUNKTION: query_inventory_category
DEKLARATION: string query_inventory_category()
BESCHREIBUNG:
Der Rueckgabewert liefert die Kategorie, bei der einem Gegenstand eingeordnet 
wird, wenn man den 'ausr'-Befehl benutzt.

Die Funktion muss im Gegenstand implementiert werden.
Alles was unter keine uebliche Kategorie faellt landet unter Sonstiges.

Folgende Kategorien (Defines in eyes.h) stehen zur Verfuegung:

    IC_LIVING		Alles, was lebt.
    IC_WEAPON		Waffen
    IC_ARMOUR		Ruestungen
    IC_CONTAINER	Behaelter
    IC_CLOTHES		Kleidung
    IC_FOOD		Nahrung (Essen, Getraenke)
    IC_MONEY		Geld
    IC_VALUEABLES	Sachen, die wertvoll sind
    IC_OTHER		Sonstiges

VERWEISE: set_eye_option, query_inventory_flags
GRUPPEN: spieler, augen
*/

private int is_inventory_category(object ob, string title, closure query)
{
    string category;
 
    // Alles was unter keine Kategorie passt, landet unter Sonstiges   
    if(!query) return 1;

    // wenn eine Kategorie gesetzt ist beim Objekt bekommt diese Vorrang
    category=ob->query_inventory_category();
    if(category) return category==title;

    return funcall(query, ob);
    // ansonsten werden die Kategorien in der vorgegebenen Reihenfolge
    // mit der angegebenen query getestet
}

/*
FUNKTION: query_inventory_flags
DEKLARATION: int query_inventory_flags()
BESCHREIBUNG:
Der Rueckgabewert liefert Informationen dazu, wie ein Gegenstand
bei der Ausruestungsanzeige ('ausr'-Befehl) dargestellt werden soll.
Die Funktion muss im Gegenstand implementiert werden.

Folgende Flags (welche via binaeres Oder verbunden werden koennen)
sind in eyes.h definiert:

    IF_HIDE_INVENTORY	Der Gegenstand will nicht, dass man seinen Inhalt
			anzeigt.

VERWEISE: set_eye_option, query_inventory_category
GRUPPEN: spieler, augen
*/

private varargs string pinv(object * obs, int si, string options,
			    string title, closure query)
{
    string ret = "";
   
    for(int i=0; i<sizeof(obs); i++)
    {
	object ob = obs[i];
	string obshort;
	
	if(ob && is_inventory_category(ob, title, query) &&
	    stringp(obshort = ob->query_short(this_object())))
        {
	    int invis = ob->query_invis() & V_ATOM_NOLIST;
	    int anz = 0;
	    string jret;
	    object *jobs;
	    
	    if(!invis)
		ret += obshort;
	    else if(si)
		ret += "("+obshort+")";
		
	    jret = "";
	    jobs = (!ob->query_con_close() || ob->query_transparent()) &&
	        filter(all_inventory(ob), #'is_visible);
		
    	    if(!(ob->query_inventory_flags() & IF_HIDE_INVENTORY))
	    {
		foreach(object job: jobs)
		    if(job && stringp(obshort = job->query_short(this_object())))
		    {
			anz++;
			
			if(member(options, 'r') >= 0)
	        	    jret += INDENT+obshort+"\n";
		    }
            }
	    
	    if(!invis || si)
	    {
		ret += anz ? " ("+anz+")" : "";
		ret += jret == "" ? (title?"\n":".\n") : " :\n"+jret;
	    }
	    
    	    obs[i] = 0;
	}
    }
	
    if(member(options, 't') >= 0)
        ret = implode(map(explode(ret, "\n"), #'ltrim) , "\n");

    if(strlen(ret) && title)
        ret = PREFIX+title+SUFFIX+"\n" + ret;

    return ret;
}

static int nahrung(object ob) 
{
   return ob->material("wasser") || ob->material("nahrung");
}

static int lebendig(object ob)
{
   return living(ob);
}

static int sort_by_name(object a, object b)
{
   return objectp(a) && objectp(b) && a->query_name() > b->query_name();
}

int no_autoloader(object x)
{
    return (!x->query_auto_load()) && (!x->query_gilden_tool());
}

private int filter_inv_living(object ob)
{
    if (ob->query_wield())
        return 1;

    if (ob->query_obvious())
        return 1;

    return 0;
}

int inventory(string str)
{
   object *obs, container;
   string table, ret, options;
   int si;

   if (sizeof(str))
   {
      string* words = explode(str, " ") - ({""});
      string rest;
      mixed parsed;

      if (sizeof(words) && words[0][0] == '-')
      {
         options = words[0];
         rest = implode(words[1..], " ");
      }
      else
      {
         options = "";
         rest = str;
      }

      if (sizeof(rest))
      {
         parsed = parse_com(rest, 0, 0, PARSE_NO_V_ITEMS|PARSE_DONT_SPLIT);
         if (parse_com_error(parsed, "Inhalt wovon anzeigen?\n", 1))
            return 0;

         container = parsed[PARSE_OBS][0];
      }
      else
         container = this_object();
   }
   else
   {
      options = "";
      container = this_object();
   }

   if(member(options,'?')>=0)
   {
      string tmp="";
      foreach(int opt:eye_option["invstyle"]||"")
	switch(opt)
	{
	   case 'k': tmp+="   k    Einteilung der Gegenstände nach Klassen\n"; break;
	   case 'm': tmp+="   m    Verwendung eines 'more' zur Ausgabe\n"; break;
	   case 'r': tmp+="   r    Tascheninhalte auch anzeigen\n"; break;
	   case 's': tmp+="   s    Sortierung der Gegenstände nach Namen\n"; break;
	   case 't': tmp+="   t    Zweispaltige Ausgabe\n"; break;
	   case 'v': tmp+="   v    VT100 Support\n"; break;
	   case 'z': tmp+="   z    Gegenstände, die Du eh immer bei Dir hast wie Kaffeeklatsche,\n"
	                  "        Kuscheli oder Schnuller nicht auflisten\n"; break;
	}
     this_object()->send_message_to(this_object(),MT_NOTIFY,MA_LOOK,
        strlen(tmp)?"Du hast folgende Optionen als Voreinstellung gespeichert:\n"+tmp
	           :"Du hast keine Optionen als Voreinstellung gespeichert.\n");
     options-="-? ";
     if(!strlen(options))
        return 1;
   }
   if(!sizeof(options) && !(options = eye_option["invstyle"]))
      options = "";
   if(member(options,'+')>=0) 
      options += ""+eye_option["invstyle"];
   else if(member(options,'w')>=0)
      eye_option["invstyle"] = implode(explode(options, "w"), "");

   si = query_wiz_level() && (eye_option["myinv"] || member(options,'a')>=0);

   obs = all_inventory(container);
   if (container != this_object())
   {
      string content;

      /* Wir machen hier alle Schritte analog zum Betrachten... */
      if (!present(container, this_object()) &&
          !QUERY("visible_in_the_dark",container) &&
          !this_object()->can_see(container))
      {
         this_object()->send_message_to(this_object(),MT_NOTIFY,MA_LOOK,"Es ist zu dunkel.");
         return 1;
      }

      if(this_object()->do_forbiddens(C_RESORT, "look", ({"", "_me"}), ({this_object(), container})))
         return 1;

      if (!IS_HIDDEN(this_object()))
      {
         string msg = (IS_NIGHT && QUERY("look_msg_night", container)) || QUERY("look_msg", container);
         object owner;
         if (msg)
         {
            if (sizeof(msg))
               this_object()->send_message(MT_LOOK,MA_LOOK,msg);
         }
         else if (IS_HIDDEN(container))
            this_object()->send_message(MT_LOOK,MA_LOOK,
               Der()+" schaut etwas an.", Der()+" schaut dich an.", container);
         else if ((owner = auto_owner_search(container)) && owner && owner != this_object())
            this_object()->send_message(MT_LOOK,MA_LOOK,
               Der()+" schaut "+ihren(container)+" an.",
               Der()+" schaut "+deinen(container)+" an.", owner);
         else
            this_object()->send_message(MT_LOOK,MA_LOOK,
               Der() +" schaut " + seinen(container) + " an.",
               Der() +" schaut dich an.", container);
      }

      content = container->query_contents(0, this_object());
      if (content)
      {
         content = (container->query_short(this_object()) || (Ein(container)+"."))
                 + (sizeof(content)? "\n" + content : "");

         if(member(options, 'm') < 0)
            this_object()->send_message_to(this_object(),MT_LOOK,MA_LOOK,content);
         else if(ret=this_player()->more(explode(content,"\n")[0..<2], "--Mehr--",0,M_AUTO_END))
            return notify_fail(M_ERR(ret));

         this_object()->do_notifies(C_RESORT, "look", ({"", "_me"}), ({this_object(), container}));
         return 1;
      }
      else if (container->query_con_close() && !container->query_transparent())
      {
         this_object()->send_message_to(this_object(),MT_LOOK,MA_LOOK,
            Der(container) + " " + ist(container) + " geschlossen.");
         return 1;
      }
      else if (container->query_worn() && present(container, this_object()) && !container->query_content_visible_when_worn())
      {
         mixed adj = container->query_worn_adjektiv() || "angezogen";
         this_object()->send_message_to(this_object(),MT_LOOK,MA_LOOK,
            "Du hast " + den(container) + " " + (pointerp(adj) ? adj[0] : adj) + ".");
         return 1;
      }

      if (living(container))
      {
         object* klamotten = determine_obvious_clothes(filter_objects(obs, "query_worn"));
         obs = klamotten + (filter(obs, #'filter_inv_living) - klamotten);
      }

      options -= "rz";
   }
   else if(member(options, 'z') >= 0) /* Autoloader mit anzeigen? */
      obs = filter(obs,#'no_autoloader);

#ifdef AUTO_COUNTOB
   obs->undo_split(); /* countobs in einen ordentlichen Zustand bringen. */
   obs -= ({0});
#endif /* AUTO_COUNTOB */

   /* Sort by Name? */
   if(member(options, 's') >= 0)
      obs = sort_array(obs, "sort_by_name");

   /* by class? */
   if(member(options, 'k') >= 0)
   {
      table =  pinv(obs, si, options, IC_LIVING, #'lebendig);
      table += pinv(obs, si, options, IC_WEAPON, EXTERN("query_weapon"));
      table += pinv(obs, si, options, IC_ARMOUR, EXTERN("query_armour"));
      // Rucksack als Container erkennen, statt Kleidung.
      table += pinv(obs, si, options, IC_CONTAINER, EXTERN("query_container"));
      table += pinv(obs, si, options, IC_CLOTHES, EXTERN("query_cloth"));
      table += pinv(obs, si, options, IC_FOOD, #'nahrung);
      table += pinv(obs, si, options, IC_MONEY, EXTERN("query_money"));
      table += pinv(obs, si, options, IC_VALUEABLES, EXTERN("query_value"));
      table += pinv(obs, si, options, IC_OTHER);
   }
   else
      table =  pinv(obs, si, options);

   /* Mehrspaltige Tabelle? */
   if(member(options, 't') >= 0)
      table = sprintf("%-79.2#s", table)+"\n";

   /* VT100 Support? */
   if(member(options, 'v') >= 0)
   {
      table = implode(explode(table,PREFIX),VT_UNDERLINE);
      table = implode(explode(table,SUFFIX),VT_NORM + REPLACE);
   }

   /* Header/Footer */
   if (container == this_object())
   {
      if(!sizeof(table - " \n"))
         table = "Du hast nichts bei dir.";
      else
         table = "Du hast bei dir:\n" + table;
   }
   else
   {
      if(!sizeof(table - " \n"))
      {
         if (living(container))
            table = Der(container) + plural(" trägt nichts bei sich."," tragen nichts bei sich.", container);
         else
            table = Der(container) + plural(" enthält nichts."," enthalten nichts.", container);
      }
      else
      {
         string intro = container->query_content_message();
         if (sizeof(intro))
            table = wrap(intro) + table;
         else if (living(container))
            table = wrap(Er(container)+plural(" trägt bei sich:"," tragen bei sich:",container)) + table;
         else
            table = wrap(Er(container)+plural(" enthält:"," enthalten:",container)) + table;

         table = wrap(container->query_short(this_object()) || (Ein(container)+".")) + table;
      }
   }

   /* More output? */
   if(member(options, 'm') < 0)
      this_object()->send_message_to(this_object(),MT_LOOK,MA_LOOK,
      	table);
   else if(ret=this_player()->
	   more( explode(trim(table,TRIM_RIGHT,"\n"),"\n"), "--Mehr--",0,M_AUTO_END)) {
      notify_fail(M_ERR(ret));
      return 0;
      }

   if (container != this_object())
      this_object()->do_notifies(C_RESORT, "look", ({"", "_me"}), ({this_object(), container}));

   return 1;
}

int augen(string str)
{
   int i;
   string *indices;
   string option, value;

   if(!query_wiz_level())
      return 0;
   indices = sort_array(m_indices(eye_option_text),#'>);
   if(!str)
   {
      write("Option     Wert       Erklärung\n");
      write("---------- ---------- ------------------"+
	    "--------------------------------------\n");
      for(i = 0; i < sizeof(indices); i++)
	 write(sprintf("%-10s %-10s %=-58s\n",
		       indices[i], to_string(eye_option[indices[i]]), 
		       eye_option_text[indices[i]]));
      write("---------- ---------- ------------------"+
	    "--------------------------------------\n");
      return 1;
   }
   if(sscanf(str, "%s %s", option, value)!=2)
   {
      notify_fail("augen [<option> <wert>]\n");
      return 0;
   }
   if(member(indices,option) < 0)
   {
      notify_fail("Unbekannte Option für 'augen': "+option+"\n");
      return 0;
   }
   if(eye_option_text[option,1])
      eye_option[option] = value;
   else
      eye_option[option] = to_int(value);
   write("Ok.\n");
   return 1;
}
