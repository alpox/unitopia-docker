// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/i/player/event.c
// Description: Event Client
// Author:	Garthan

#pragma save_types
#pragma strong_types

#include <config.h>
#include <apps.h>
#include <level.h>
#include <event.h>
#include <udp.h>
#include <more.h>
#include <invis.h>
#include <message.h>
#include <colours.h>
#include <error.h>
#include <notify_fail.h>
#include "player.h"

#define MELDUNGSPUFFERGROESSE 100
#define LOGPUFFERGROESSE 200
#define CHANNEL_MASTER "/secure/udp/channel"
#define IS_COMM(x) (member(comm_nrs,x)>=0)
#define FAIL(x) { notify_fail(x); return 0; }
#define LOW(x) lower_case(x)
#define FORWARD(x,y) for(x = 0; x < sizeof(y); x++)
#define DELETEABLE ( !persons && \
       trim(events[EVC_STATUS][index]) == "" && \
       !events[EVC_COLOUR][index])

nomask int query_wiz_level();
string query_gilde();
nomask int query_level();

//Prototypen aus /i/tools/colours:
string colour_to_string(int col);
mixed string_to_colour(string str);
string colour_to_ansi(int colour);
//Prototypen aus /i/player/webmud:
static int uses_webmud();
protected void send_webmud_event_message(int event_nr, mixed origin_ob, string mess);

private mixed *events;
private int onoff, edpuffer, echomode, global_eventmode, global_colourmode=1;
private string* pufferoptionen=({});
private mapping ignored_players;
// Puffer fuer entgangene Kurimeldungen (wegen Editor oder Ruheraum)
private static int puffering;
private static string * puffer = ({});

static nomask void set_events(mixed *str) { events = str; }
static nomask mixed *query_events() { return events; }

// Puffer fuer alle moeglichen Meldungen
// Enthaelt Elemente der Form:
//   Art: Array aus ({Zeit, Verursachername, Text, Flags})
// Art kann sein: "sage", "rede" oder "seele"
// Flags: Flags aus /secure/event (EFV_GRATSFILTER)
//        EVF_NON_INTERACTIVE: Spieler war Statue

private static mapping meldungspuffer = ([]);

private static mixed * commands =
({
   ({ "","spieler","normal","domainlord", "gouverneur","admin",
      "personen", "-----", "monster", "mich", "gratsfilter", "gäste"}),
   ({ "*", "s", "n", "d", "v", "a", "p", "?", "m", "@", "f", "t" }),
   ({ 0, 0, 1, 1, 1, 1,  0, 1, 1, 0, 0, 0 }) // 0 == all, 1 == wizonly
});

private static string * anaus = ({ "aus", "an" });

nomask static void set_online(int i) { onoff = i ? 1 : 0; }
int query_online() { return onoff; }
nomask static void set_global_eventmode(int i) { global_eventmode = i;}
nomask static int query_global_eventmode() { return global_eventmode;}
nomask static void set_global_colourmode(int i) { global_colourmode = i;}
nomask static int query_global_colourmode() { return global_colourmode;}
nomask static void set_edpuffer(int i) {edpuffer = i;}
nomask static int query_edpuffer() {return edpuffer;}

static void setup_player()
{
   int i;

   if(!events)
   {
      // Kanaele, die defaultmaessig an sein sollen.
      // 7=Gildenein/austritt, 9=Gebruell, 10=Pantheon 25=Hilferuf
      events = ({ ({0,7,9,10,25}), ({}), ({}), ({}) });
      edpuffer = 1;
      echomode = 1;
      set_online(1);
      // Oben eingetragene Kanale anschalten und MICH Flag setzen
      FORWARD(i, events[EVC_ID_NR])
      {
	 events[EVC_STATUS]  += ({ set_bit(set_bit("",0), COMM_MICH) });
	 events[EVC_PERSONS] += ({ 0 });
	 events[EVC_COLOUR]  += ({ 0 });
      }
      // Bei Kanal 9=Gebruell Gratsfilter aktivieren
      // int index;
      // if((index = member(events[EVC_ID_NR],9)) >= 0)
      //   events[EVC_STATUS][index] =
      //       set_bit(events[EVC_STATUS][index], COMM_GRATSFILTER);
   }
   else
   {
      if(sizeof(events)==5) // Mit Gilden- und Farbeinstellungen
          events = events[0..1]+events[3..4];
      else if(sizeof(events)==4) // Entweder Gilden- oder Farbeinstellungen
      {
          if(!sizeof(events[2])) {} // Keine Kanaele, dann isses egal...
	  else if(stringp(events[2][0])) // Gildeneinstellungen
	      events = events[0..1]+({events[3],({0})*sizeof(events[0])});
      }
      // Nun noch ueberpruefen, dass genausoviel Farbeinstellungen wie
      // Kanaele da sind. (wegen eines Bugs)
      if(sizeof(events[EVC_COLOUR])<sizeof(events[EVC_ID_NR]))
          events[EVC_COLOUR] += ({0}) * (sizeof(events[EVC_ID_NR])-sizeof(events[EVC_COLOUR]));
   }
    
   if(!onoff)
      this_object()->send_message_to(this_object(),MT_NOTIFY,MA_UNKNOWN,
      	"\aDein Kurier ist ausgeschaltet!\n");
}

static int add_event(int event_nr)
{
   events[EVC_ID_NR]   += ({ event_nr });
   events[EVC_STATUS]  += ({ "" });
   events[EVC_PERSONS] += ({ 0 });
   events[EVC_COLOUR]  += ({ 0 });
   return sizeof(events[EVC_ID_NR])-1;
}


static void delete_event(int satz_nr)
{
   int i;
   if(satz_nr)
      FORWARD(i, events)
         events[i][satz_nr] = events[i][0];
   FORWARD(i, events)
      events[i] = events[i][1..];
}

//Wird von /secure/udp/event aufgerufen, wenn TO einer Gilde
//beigetreten ist, um den Gildenkanal zu aktivieren.
int enter_event(int event_nr)
{
   int i;
   if((i = member(events[EVC_ID_NR], event_nr)) < 0)
      i = add_event(event_nr);
   events[EVC_STATUS][i] = set_bit("",0);
   events[EVC_PERSONS][i] = 0 ;
   events[EVC_COLOUR][i] = 0;
}

//Wird von /secure/udp/event aufgerufen, wenn TO aus einer Gilde
//ausgetreten ist, um den Gildenkanal zu deaktivieren.
int leave_event(int event_nr)
{
   int i;
   if((i = member(events[EVC_ID_NR], event_nr)) >= 0)
      delete_event(i);
}


void flush_puffer()
{
    int i;

    if(sizeof(puffer))
    {
	this_object()->send_message_to(this_object(),MT_NOTIFY|MT_CHANNEL,MA_UNKNOWN,
       		"\n-----\n");
	FORWARD(i, puffer)
	    this_object()->send_message_to(this_object(),MT_NOTIFY|MT_CHANNEL,MA_UNKNOWN,
	    	puffer[i]);    
	this_object()->send_message_to(this_object(),MT_NOTIFY|MT_CHANNEL,MA_UNKNOWN,
		"-----\n");
	puffer = ({});
    }
}

// Wird vom Zauberstab aufgerufen.
void notify_ed_exit(int suspend_client)
{
    puffering = 0;
    if(suspend_client)
	this_object()->restart_mudclient();
    flush_puffer();
}

void notify_ed_enter(int suspend_client)
{
    if(suspend_client)
	this_object()->suspend_mudclient();
    puffering = 1;
}

varargs int event(int event_nr, mixed origin_ob, string mess, int flag, string wizmess, string bracketmess)
{
   int level, show, idx;
   mixed *colour; 
   string rname, stat, *persons;

   if(!onoff)
   {
      if(flag & EVF_FEEDBACK)
         this_object()->receive_message(MT_NOTIFY, MA_UNKNOWN, this_object(),
	    "Ok.\n");
      return 0;
   }
      
   if((idx = member(events[EVC_ID_NR], event_nr)) >= 0)
   {
      stat = events[EVC_STATUS][idx];
      persons = events[EVC_PERSONS][idx];
      if(sizeof(events)>EVC_COLOUR)  //Falls vorm setup_player ein Event
                                     //ausgeloest wird. (z.B. durch Hexengilde)
         colour = events[EVC_COLOUR][idx];
   }
   else
   {
      stat = "";
      persons = 0;
      colour = 0;
   }

   if (test_bit(stat, COMM_PERSONS))  /* Ereignis aus? */
   {
      if(flag & EVF_FEEDBACK)
         this_object()->receive_message(MT_NOTIFY, MA_UNKNOWN, this_object(),
	    "Ok.\n");
      return 0;
   }
   
   if(origin_ob)
   {
      int fit;

      if(objectp(origin_ob))
      {
	 if(origin_ob == this_object() && !test_bit(stat,COMM_MICH))
         {
            if(flag & EVF_FEEDBACK)
               this_object()->receive_message(MT_NOTIFY, MA_UNKNOWN,
	          this_object(), "Ok.\n");
	    return 0;
	 }

	 if(!(rname = origin_ob->query_real_name()))
	    rname = lower_case(origin_ob->query_name() || "");

	 level = query_real_player_level(origin_ob);
      }
      else if(stringp(origin_ob))
      {
	 rname = origin_ob;
	 level = 0;
      }

      fit = rname && persons && member(persons, rname) >= 0 ||
            test_bit(stat,COMM_S) && playerp(origin_ob) && level < LVL_WIZ ||
            test_bit(stat,COMM_N) && wizp(origin_ob) && !lordp(origin_ob)  ||
            test_bit(stat,COMM_D) && lordp(origin_ob) && !adminp(origin_ob) ||
            test_bit(stat,COMM_A) && adminp(origin_ob) ||
            test_bit(stat,COMM_M) && !level ||
            test_bit(stat, 0) && test_bit(stat,COMM_GRATSFILTER)
                && flag & EVF_GRATSFILTER ||
            // beim Gratsfilter kein Invertieren
            test_bit(stat,COMM_GAESTE) && guestp(origin_ob);
      if(fit)
         fit = !rname || !persons || member(persons, "!"+rname) < 0;

      show = test_bit(stat, 0) ? !fit : fit;
   }
   else if(!mess) // Nur ein Test
   {
      show = test_bit(stat,0) || test_bit(stat, COMM_S) ||
             test_bit(stat,COMM_N) || test_bit(stat, COMM_D) ||
             test_bit(stat, COMM_A) || test_bit(stat,COMM_M) || 
	     test_bit(stat, COMM_GAESTE) || test_bit(stat,COMM_GRATSFILTER) ||
	     (persons && sizeof(persons));
   }
   else
      show = test_bit(stat,0);

   if (wizp (this_object())) {
       if (bracketmess && (global_eventmode & EVF_G_MODE_BRACKETS))
           mess = bracketmess;
       else
           if (wizmess) mess = wizmess;   
   }
    
   if(show)
   {
      mixed ruhe;
      
      if(stringp(mess) && colour && global_colourmode)
          mess = sprintf("%s%s%s%s%s",
            ((colour[EVCC_COLOR] & CO_BEEP) && interactive() &&
              ( sizeof(colour) <= EVCC_ECOLOR || query_idle(this_object()) / 60 >= COE_TO_IDLEBEEP(colour[EVCC_ECOLOR]) ))
            ? (uses_webmud() && sizeof(colour) > EVCC_BEEP_TYPE && colour[EVCC_BEEP_TYPE])
               ? "\e_beep:"+colour[EVCC_BEEP_TYPE]+"\e\\"
               : "\a"
            : "",
	    (colour[EVCC_ANSI]||"") - "\a",
	    (sizeof(mess) && mess[<1] == '\n') ? mess[0..<2] : mess,
	    VT_NORM,
	    (sizeof(mess) && mess[<1] == '\n') ? "\n" : "");

      if(mess && this_object()->uses_webmud())
      {
          int wm = read_bits(stat, COMM_WEBMUD, COMM_WEBMUD_WIDTH);
          if(wm == CW_OWN_WINDOW || wm == CW_BOTH_WINDOWS)
          {
              send_webmud_event_message(event_nr, origin_ob, mess);

              if(wm == CW_OWN_WINDOW)
                  return 1;
          }
      }

      if(environment())
        catch(ruhe = environment()->query_type ("ruhe"); publish, reserve 16384);

      if((ruhe ||
         (edpuffer &&
	 (query_editing(this_object()) || puffering))) &&
	 sizeof(puffer) < 20)
      {
	 if(mess)
	    puffer+=({ mess });
         if(flag & EVF_FEEDBACK)
            this_object()->receive_message(MT_NOTIFY, MA_UNKNOWN,
	       this_object(), "Ok.\n");
	 return 2;
      }
      else
      {
	 // Avatar xeditor patch start
	 object editor_shadow;
	 
	 if( editor_shadow=this_object()->has_editor_shadow() )
	 {
	    if( mess )
	       editor_shadow->receive_message(
	           ((origin_ob==this_object())?MT_NOTIFY:0)|MT_CHANNEL,
		   MA_UNKNOWN,objectp(origin_ob)?origin_ob:this_player(),mess);
	    return 2;
	 }
	 // Avatar xeditor patch stop

	 if(mess)
	 {
	    if(sizeof(puffer))
	    {
	        if(mess) puffer+=({mess});
		flush_puffer();
	    }
	    else
		this_object()->receive_message(
	    	    ((origin_ob==this_object())?MT_NOTIFY:0)|MT_CHANNEL,
		    MA_UNKNOWN,objectp(origin_ob)?origin_ob:this_player(),mess);
	 }
	 return 1;
      }
   }
   if(flag & EVF_FEEDBACK)
      this_object()->receive_message(MT_NOTIFY, MA_UNKNOWN,
         this_object(), "Ok.\n");
   return 0;
}

static string query_status_string(int index)
{
   string global, out;
   int i;

   out = global = "";
   for(i = 1; i < sizeof(commands[EVC_COMM]); i++)
      if(test_bit(events[EVC_STATUS][index], i) &&
         i != COMM_PERSONS &&
         i != COMM_MICH)
         out += commands[EVC_COMM_AN][i];
   if(events[EVC_PERSONS][index])
      out += commands[EVC_COMM_AN][COMM_PERSONS];
   if(test_bit(events[EVC_STATUS][index],0))
   {
      global = commands[EVC_COMM_AN][0];
      if(test_bit(events[EVC_STATUS][index],COMM_MICH))
         global += commands[EVC_COMM_AN][COMM_MICH];
      if(out != "")
         global += "-";
   }
   i = test_bit(events[EVC_STATUS][index],COMM_PERSONS);
   return (i?"(":"") + global + out + (i?")":"");
}

static string query_one_status(mapping event_liste, int i)
{
   int index;
   string status_display;

   if((index = member(events[EVC_ID_NR],i))>=0)
      status_display = query_status_string(index);
   else
      status_display = "";
   return right(i, 5)+" "+
          left(event_liste[i]["name"], 10)+" "+
          left(status_display,10)+" "+
          left(event_liste[i]["desc"]+".", 51)+"\n";
}


static string query_one_status_detailed(mapping event_liste, int i)
{
   int index;
   string status_display, *persons;

   if((index = member(events[EVC_ID_NR],i))>=0)
   {
      status_display = query_status_string(index);
      persons = events[EVC_PERSONS][index];
   }
   else
      status_display = "";
   return right(i, 5)+" "+
          left(event_liste[i]["name"], 10)+" "+
          left(status_display, 10)+" "+
          (persons ? implode(persons,", ")+" " : " ")+"\n" ;
}

static string query_one_colour(mapping event_liste, int i)
{
   int index;
   int *colour;

   if((index = member(events[EVC_ID_NR],i))>=0)
      colour = events[EVC_COLOUR][index];
   else
      colour = 0;
   return right(i, 5)+" "+
          left(event_liste[i]["name"], 10)+" "+
	  colour_to_string(colour?colour[0]:0)+"\n";
}

string toggle_bit(string what, int n)
{
   if(test_bit(what, n))
      return clear_bit(what, n);
   else
      return set_bit(what, n);
}

/* Diese Funktion existiert genauso in event.c */
int validate_event(mapping event_liste, int i, int level, string gilde)
{
   return ((level >= event_liste[i]["level"] || adminp(this_object())) &&
          (!event_liste[i]["gilde"] || query_wiz_level() ||
           event_liste[i]["gilde"] == gilde) &&
          (!event_liste[i]["musthave"] || query_wiz_level() ||
           present(event_liste[i]["musthave"]))) && 1;
}


/* Diese Funktion existiert genauso in event.c */
int get_event_index(mapping event_liste, mixed event_nr)
{
   int i, *idxs;

   if(intp(event_nr) && member(event_liste, event_nr))
      return event_nr;
   else if(stringp(event_nr))
   {
      string name = convert_umlaute(lower_case(event_nr));
      for(i = sizeof(idxs = m_indices(event_liste)); i--;)
         if(event_liste[idxs[i]]["lcname"] == name)
            return idxs[i];
   }
   return -1;
}

#define IS_WIZARD (level >= LVL_WIZ)

int set_event(string str)
{
   int i, j, level, index, size;
   int *comm_nrs, *idxs;
   string comm, *args, *persons;
   string gilde;
   mixed event_nr, *listeners;
   mapping event_liste;

   event_liste = EVENT_MASTER->query_events();
   level = query_level();
   gilde = query_gilde();


   /* Auflistung der Ereignisse mit Settings */

   if(!str || (str=space(str)) == "")
   {
      this_object()->send_message_to(this_object(),MT_NOTIFY,MA_UNKNOWN,
      	"Mögliche Ereignisse:\n");
      idxs = sort_array(m_indices(event_liste),#'>);
      FORWARD(i, idxs)
	 if(validate_event(event_liste, idxs[i], level, gilde))
            this_object()->send_message_to(this_object(),MT_NOTIFY,MA_UNKNOWN,
	    	query_one_status(event_liste, idxs[i]));
      this_object()->send_message_to(this_object(),MT_NOTIFY,MA_UNKNOWN,
      	"Dein Kurier ist "+anaus[onoff]);
      this_object()->send_message_to(this_object(),MT_NOTIFY,MA_UNKNOWN,
      	", Editor-Puffer ist "+anaus[edpuffer]);
      this_object()->send_message_to(this_object(),MT_NOTIFY,MA_UNKNOWN,
      	", Echo - Modus ist "+anaus[echomode]+".\n");
      if (wizp (this_object()))
          this_object()->send_message_to(this_object(),MT_NOTIFY,MA_UNKNOWN,
            "Klammer - Modus ist "+anaus[
            (global_eventmode & EVF_G_MODE_BRACKETS) == EVF_G_MODE_BRACKETS]
            +".\n");
      return 1;
   }
   if(str == "detail" || str == "details")
   {
      this_object()->send_message_to(this_object(),MT_NOTIFY,MA_UNKNOWN,
      	"Mögliche Ereignisse (Details):\n");
      idxs = m_indices(event_liste);
      FORWARD(i, idxs)
	 if(validate_event(event_liste, idxs[i], level, gilde))
            this_object()->send_message_to(this_object(),MT_NOTIFY,MA_UNKNOWN,
	    	query_one_status_detailed(event_liste, idxs[i]));
      this_object()->send_message_to(this_object(),MT_NOTIFY,MA_UNKNOWN,
      	"Dein Kurier ist "+anaus[onoff]);
      this_object()->send_message_to(this_object(),MT_NOTIFY,MA_UNKNOWN,
      	", Editor-Puffer ist "+anaus[edpuffer]);
      this_object()->send_message_to(this_object(),MT_NOTIFY,MA_UNKNOWN,
      	", Echo - Modus ist "+anaus[echomode]+".\n");
      if (wizp (this_object()))
          this_object()->send_message_to(this_object(),MT_NOTIFY,MA_UNKNOWN,
            "Klammer - Modus ist "+anaus[
            (global_eventmode & EVF_G_MODE_BRACKETS) == EVF_G_MODE_BRACKETS]
            +".\n");
      return 1;
   }
   if(str == "befehl" || str == "befehle" || str == "?")
   {
      this_object()->send_message_to(this_object(),MT_NOTIFY,MA_UNKNOWN,
      "Syntax: {kuri[er]|erei[gnis]} <ereignis>[ <befehl>[<befehl>...]]\n"+
      "        <ereignis> ist ein mögliches Ereignis aus der Ereignisliste\n"+
      "        <befehl> ist ein Befehl aus der folgenden Liste:\n"+
      "  * schaltet das Ereignis an oder aus (wie ohne Befehl)\n");
      for(i = 1; i < sizeof(commands[EVC_COMM]); i++)
	 if((IS_WIZARD || !commands[EVC_COMM_ACCESS][i]) &&
	     commands[EVC_COMM_AN][i] != "?")
	 {
	    this_object()->send_message_to(this_object(),MT_NOTIFY,MA_UNKNOWN,
	    	  right(commands[EVC_COMM_AN][i], 3)+" oder "+
		  commands[EVC_COMM][i]);
	    if(i == COMM_PERSONS)
	       this_object()->send_message_to(this_object(),MT_NOTIFY,MA_UNKNOWN,
	       		" <name1>, <name2>, !<name3>,...");
	    this_object()->send_message_to(this_object(),MT_NOTIFY,MA_UNKNOWN,
	    	"\n");
	 }
      if(IS_WIZARD)
	 this_object()->send_message_to(this_object(),MT_NOTIFY,MA_UNKNOWN,
	 	"  g oder götter    (entspricht ndva)\n");
      this_object()->send_message_to(this_object(),MT_NOTIFY,MA_UNKNOWN,
      	"Ist kein Befehl angegeben, so wird das Ereignis "+
            "an- bzw. ausgeschaltet!!\n");
      return 1;
   }
   if(str == "flags")
   {
      cat(FLAGS);
      return 1;
   }
   if(str == "beispiel")
   {
      cat(BEISPIEL);
      return 1;
   }
   if(str == "puffer")
   {
      if(edpuffer = !edpuffer)
	 this_object()->send_message_to(this_object(),MT_NOTIFY,MA_UNKNOWN,
	 	"Dein Kurier puffert nun, wenn Du editierst.\n");
      else
	 this_object()->send_message_to(this_object(),MT_NOTIFY,MA_UNKNOWN,
	 	"Dein Kurier puffert nun nicht, wenn Du editierst.\n");
      return 1;
   }
   if ((str == "puffer an") || (str == "puffer ein"))
   {
      edpuffer = 1;
      this_object()->send_message_to(this_object(),MT_NOTIFY,MA_UNKNOWN,
      	"Dein Kurier puffert nun, wenn Du editierst.\n");
      return 1;
   }
   if(str == "puffer aus")
   {
      edpuffer = 0;
      this_object()->send_message_to(this_object(),MT_NOTIFY,MA_UNKNOWN,
      	"Dein Kurier puffert nun nicht, wenn Du editierst.\n");
      return 1;
   }

   if(str == "echo")
   {
      if(echomode = !echomode)
	 this_object()->send_message_to(this_object(),MT_NOTIFY,MA_UNKNOWN,
	 	"Du erhältst nun ein Echo beim sagen und reden.\n");
      else
	 this_object()->send_message_to(this_object(),MT_NOTIFY,MA_UNKNOWN,
	 	"Du erhältst nun kein Echo beim sagen und reden.\n");
      return 1;
   }
   if ((str == "echo an") || (str == "echo ein"))
   {
      echomode = 1;
      this_object()->send_message_to(this_object(),MT_NOTIFY,MA_UNKNOWN,
      	"Du erhältst nun ein Echo beim sagen und reden.\n");
      return 1;
   }
   if(str == "echo aus")
   {
      echomode = 0;
      this_object()->send_message_to(this_object(),MT_NOTIFY,MA_UNKNOWN,
      	"Du erhältst nun kein Echo beim sagen und reden.\n");
      return 1;
   }
   if(str == "farben" || str == "farbe")
   {
      this_object()->send_message_to(this_object(),MT_NOTIFY,MA_UNKNOWN,
      	"Mögliche Ereignisse (Farben):\n");
      idxs = m_indices(event_liste);
      FORWARD(i, idxs)
	 if(validate_event(event_liste, idxs[i], level, gilde))
            this_object()->send_message_to(this_object(),MT_NOTIFY,MA_UNKNOWN,
	    	query_one_colour(event_liste, idxs[i]));
      this_object()->send_message_to(this_object(),MT_NOTIFY,MA_UNKNOWN,
      	"Farben sind "+({"AUSGESCHALTET","angeschaltet"})[global_colourmode]+
	".\n");
      return 1;
   }
   if(str == "farben an" || str == "farbe an")
   {
      if(global_colourmode)
         this_object()->send_message_to(this_object(),MT_NOTIFY,MA_UNKNOWN,
	   "Dein Kurier zeigt Dir Deine Meldungen bereits farbig.\n");
      else
      {
         global_colourmode = 1;
         this_object()->send_message_to(this_object(),MT_NOTIFY,MA_UNKNOWN,
	   "Dein Kurier zeigt Dir Deine Meldungen jetzt farbig.\n");
      }
      return 1;
   }
   if(str == "farben aus" || str == "farbe aus")
   {
      if(!global_colourmode)
         this_object()->send_message_to(this_object(),MT_NOTIFY,MA_UNKNOWN,
	   "Dein Kurier zeigt Dir Deine Meldungen bereits ohne Farben.\n");
      else
      {
         global_colourmode = 0;
         this_object()->send_message_to(this_object(),MT_NOTIFY,MA_UNKNOWN,
	   "Dein Kurier zeigt Dir Deine Meldungen jetzt nicht mehr farbig.\n");
      }
      return 1;
   }
   
   if (wizp (this_object())) {
      if((str == "klammer") || (str == "klammern"))
      {
         if(((global_eventmode ^= EVF_G_MODE_BRACKETS) & EVF_G_MODE_BRACKETS) == EVF_G_MODE_BRACKETS)
	    this_object()->send_message_to(this_object(),MT_NOTIFY,MA_UNKNOWN,
	    	"Es werden jetzt Klammern mit Kanalnamen und Initiator vorangestellt.\n");
         else
	    this_object()->send_message_to(this_object(),MT_NOTIFY,MA_UNKNOWN,
	 	"Du erhaetlst jetzt keine Klammern mehr mit Kanalnamen und Initiator.\n");
         return 1;
      }
      if ((str == "klammer an") || (str == "klammer ein") ||
          (str == "klammern an") || (str == "klammern ein"))
      {
         global_eventmode |= EVF_G_MODE_BRACKETS;
         this_object()->send_message_to(this_object(),MT_NOTIFY,MA_UNKNOWN,
   	    "Es werden Klammern mit Kanalnamen und Initiator vorangestellt.\n");
         return 1;
      }
      if ((str == "klammer aus") || (str == "klammern aus"))
      {
         global_eventmode |= EVF_G_MODE_BRACKETS;
         global_eventmode ^= EVF_G_MODE_BRACKETS;
         this_object()->send_message_to(this_object(),MT_NOTIFY,MA_UNKNOWN,
	     "Du erhältst keine Klammern mit Kanalnamen und Initiator.\n");
         return 1;
      }
   }

   /* Argumente testen */

   if(sizeof(args = explode(str, " ")) < 1)
      FAIL("Kein Ereignis angegeben.\n");


   /* Global an/ausschalten? */

   if((i = member(anaus, args[0])) >= 0)
   {
      set_online(i);
      this_object()->send_message_to(this_object(),MT_NOTIFY,MA_UNKNOWN,
      	"Dein Kurier ist jetzt "+anaus[onoff]+".\n");
      return 1;
   }

   /* Bestimmung des Events */

   if(sscanf(args[0],"%d", event_nr) != 1)
      event_nr = capitalize(args[0]);
   if((event_nr = get_event_index(event_liste, event_nr)) < 0 ||
      !validate_event(event_liste, event_nr, level, gilde))
      FAIL("Ereignis '"+args[0]+"' nicht gefunden.\n");

   index = member(events[EVC_ID_NR],event_nr);

   /* Bestimmung des Befehls */

   comm_nrs = ({ -1 });
   if(sizeof(args) < 2)
      comm_nrs[0] = 0;
   else
   {
      comm = args[1];
      if(convert_umlaute(LOW(comm)) == "goetter")
         comm_nrs = ({ COMM_N, COMM_D, COMM_A });
      else
      if(LOW(comm) == "prevent")
      {
	 if(sizeof(args) >= 3 && member(({"aus","off","0"}), args[2]) >= 0)
	 {
	    EVENT_MASTER->delete_prevent(this_player(), event_nr);
	    this_object()->send_message_to(this_object(),MT_NOTIFY,MA_UNKNOWN,
	    	"Your events (class: "+ 
		event_liste[event_nr]["name"]+") now enabled.\n");
	 }
	 else
	 {
	    EVENT_MASTER->add_prevent(this_player(), event_nr);
	    this_object()->send_message_to(this_object(),MT_NOTIFY,MA_UNKNOWN,
	    	"Your events (class: "+ 
		event_liste[event_nr]["name"]+") are prevented, now.\n");
	 }
	 return 1;
      }
      else
      if(LOW(comm) == "farbe" || LOW(comm) == "farben")
      {
         mixed col;
         if(sizeof(args) <= 2)
	 {
	    if(index>0)
	    {
	       col = events[EVC_COLOUR][index];
	       if(col) col=col[0];
	    }
	    else
	       col = 0;
            this_object()->send_message_to(this_object(),MT_NOTIFY,MA_UNKNOWN,
	       wrap("Deine Farbeinstellungen für Kanal '" +
	       event_liste[event_nr]["name"]+"': " +
	       colour_to_string(col) +"." +
	       (global_colourmode?"":
	       " Dein Kurier zeigt aber nichts farbig an.")));
	    return 1;
	 }
	 col = string_to_colour(implode(args[2..<1]," "));
	 if(stringp(col)) FAIL(col);
	 if(index < 0)
	    index = add_event(event_nr);
	 if(col)
	   events[EVC_COLOUR][index] = ({col,colour_to_ansi(col)});
	 else
	 {
	    events[EVC_COLOUR][index] = 0;
	    persons=events[EVC_PERSONS][index];
            if(DELETEABLE)
               delete_event(index);
	 }
         this_object()->send_message_to(this_object(),MT_NOTIFY,MA_UNKNOWN,
	    wrap("Deine Farbeinstellungen für Kanal '" +
	    event_liste[event_nr]["name"]+"': " +
	    colour_to_string(col) +"." +
	    (global_colourmode?"":
	    " Dein Kurier zeigt aber nichts farbig an.")));
         return 1;
      }
      else
      if(LOW(comm) == "puffer" || comm == "%")
      {
         mixed *puffer;
	 string *outbuf;
	 int is_wiz;

	 is_wiz = wizp(this_object());
	 outbuf = ({});
	 puffer = EVENT_MASTER->query_history(event_nr);
	 size = sizeof(puffer);
	 // j wieder benutzt ...
         if (sizeof(args) > 2 && (j = to_int(args[2])) &&
		 size && j <= size)
            j = size - j;
         else
	     j = 0;
	 for (i = j; i < size; i++)
	 {
	    if(is_wiz && puffer[i][1]!="OBJ(/secure/udp/channel)")
	       outbuf += explode(sprintf("%8s %=-10s %=-1.59s",
	                    shorttimestr(puffer[i][0])[9..],
			    puffer[i][1] || "Unbekannt",
			    puffer[i][2]),"\n");
	    else
	       outbuf += explode(sprintf("%8s %=-1.70s", 
	                    shorttimestr(puffer[i][0])[9..],
			    puffer[i][2]),"\n");
	 }
	 if (sizeof(outbuf))
	    this_object()->more(outbuf, 0, 0, M_AUTO_END);
	 else
	    this_object()->send_message_to(this_object(),MT_NOTIFY,MA_UNKNOWN,
	    	"Keine Meldungen auf diesem Kanal seit "+
	          shorttimestr(program_time(touch(EVENT_MASTER)))+".\n");
         return 1;
      }
      else
      if((i = member(anaus, LOW(comm))) >= 0)
      {
         if(index < 0)
            index = add_event(event_nr);
         if(i)
            events[EVC_STATUS][index] =
               clear_bit(events[EVC_STATUS][index],COMM_PERSONS);
         else
            events[EVC_STATUS][index] =
               set_bit(events[EVC_STATUS][index],COMM_PERSONS);
         if(DELETEABLE)
            delete_event(index);
         this_object()->send_message_to(this_object(),MT_NOTIFY,MA_UNKNOWN,
	 	query_one_status_detailed(event_liste, event_nr));
         return 1;
      }
      else
      if(LOW(comm) == "wer")
      {
         listeners = EVENT_MASTER->query_listeners(event_nr);
         if (!wizp (this_object()))
            listeners = filter (listeners, 
               "kurier_wer_filter",this_object ());
         listeners = map_objects(listeners, "query_real_name") - ({ 0 });
         listeners = map(listeners, #'capitalize);
         listeners = sort_array(listeners, #'>);

         if(sizeof(listeners))
	    this_object()->send_message_to(this_object(),MT_NOTIFY,MA_UNKNOWN,
	    	"Auf Kanal '"+event_liste[event_nr]["name"]+"' "
		  "hören folgende Leute zu:\n"+
		  sprintf("%-75#s\n", implode(listeners,"\n")));
         else 
            this_object()->send_message_to(this_object(),MT_NOTIFY,MA_UNKNOWN,
	    	"Niemand hört auf Kanal '"+
		  event_liste[event_nr]["name"]+"'.\n");
         return 1;
      }
      else
      if((comm_nrs[0] = member(commands[EVC_COMM], LOW(comm))) < 0)
         for(i = 0; i < strlen(comm); i++)
            if((j = member(commands[EVC_COMM_AN], comm[i..i])) >=0 )
               comm_nrs += ({ j });
            else
            if(comm[i] == 'g')
               comm_nrs += ({ COMM_N, COMM_D, COMM_A });
   }
   comm_nrs -= ({ -1 });
   if(!IS_WIZARD)
      FORWARD(i, commands[EVC_COMM_ACCESS])
	 if(commands[EVC_COMM_ACCESS][i])
	    comm_nrs -= ({ i });
      
   if(!sizeof(comm_nrs))
      FAIL("Unbekannter Befehl '"+comm+"'.\n");


   /* Eventuell alte Persons loeschen */

   if(index >= 0)
   {
      if(member(comm_nrs, COMM_PERSONS) >= 0)
         events[EVC_PERSONS][index] = 0;
      persons = events[EVC_PERSONS][index];
   }


   /* Eventuell neue Persons? */

   if( sizeof(args) > 2 &&
       (args = explode(implode(args[2..]," "),",")) &&
       sizeof(args) )
      if(member(comm_nrs, COMM_PERSONS) >= 0)
      {
         persons = ({});
         FORWARD(i, args)
            persons += ({ lower_case(trim(args[i])) });
      }

   /* Ist es ein Neueintrag? */
   if(index < 0)
      index = add_event(event_nr);

   /* Daten in den Datensatz events[][index] schreiben */
   FORWARD(i, comm_nrs)
      if(comm_nrs[i] != COMM_PERSONS)
         events[EVC_STATUS][index] =
            toggle_bit(events[EVC_STATUS][index] ,comm_nrs[i]);
   if(persons)
      events[EVC_PERSONS][index] = persons;

   /* Ist der Datensatz loeschbar? */
   if(DELETEABLE)
      delete_event(index);

   /* Neuen Status ausgeben */
   this_object()->send_message_to(this_object(),MT_NOTIFY,MA_UNKNOWN,
   	query_one_status_detailed(event_liste, event_nr));
   return 1;
}

int listener_cmd(string str)
{
   mixed event_nr, *listeners;
   mapping event_liste;
   string gilde;
   int level;
    
   event_liste = EVENT_MASTER->query_events();
   level = query_level();
   gilde = query_gilde();

   if(!str || trim(str)=="")
   {
      int *idxs;
      string *text;
      text = ({"Folgende Ereignisse bekommst Du mit:"});
      idxs = sort_array(m_indices(event_liste),#'>);
      foreach(int i: idxs)
	 if(validate_event(event_liste, i, level, gilde) && event(i,0,0,0))
	     text += ({
		sprintf(" %3d %-20.20s %1.50s", i,
		    event_liste[i]["name"], event_liste[i]["desc"]) });
      this_player()->more(text, "Ereignisse des Kuriers [q,u,d,<,>] ", 0,
          M_AUTO_END, "Kurierliste");
      return 1;
   }
   
   if(sscanf(str,"%d", event_nr) != 1 && !sscanf(str,"k%!s%D",event_nr))
      event_nr = capitalize(str);

   if((event_nr = get_event_index(event_liste, event_nr)) < 0 ||
      !validate_event(event_liste, event_nr, level, gilde))
      FAIL("Ereignis '"+str+"' nicht gefunden.\n");

   if(!wizp(this_object()) && event_liste[event_nr]["undisclosed"])
   {
      this_object()->send_message_to(this_object(),MT_NOTIFY,MA_UNKNOWN,
     	 wrap("Dein Kurier weiß leider nicht, wer auf dem Kanal '"+
	  event_liste[event_nr]["name"]+"' mithört."));
      return 1;
   }
   listeners = EVENT_MASTER->query_listeners(event_nr);
   if (!wizp (this_object()))
      listeners = filter (listeners, "kurier_wer_filter",this_object ());
   listeners = map_objects(listeners, "query_real_name") - ({ 0 });
   listeners = map(listeners, #'capitalize);
   listeners = sort_array(listeners, #'>);

   if(sizeof(listeners))
      this_object()->send_message_to(this_object(),MT_NOTIFY,MA_UNKNOWN,
    	 "Auf Kanal '"+event_liste[event_nr]["name"]+"' "
	 "hören folgende Leute zu:\n"+
	  sprintf("%-75#s\n", implode(listeners,"\n")));
   else 
      this_object()->send_message_to(this_object(),MT_NOTIFY,MA_UNKNOWN,
     	 "Niemand hört auf Kanal '"+
	  event_liste[event_nr]["name"]+"'.\n");
   return 1;
}

int kurier_wer_filter (object ob)
{
   if (!playerp(ob) || ob->query_invis() & V_ATOM_INVIS || ob->query_no_wer ()) return 0;
   return 1;
}

int channel(string str)
{
   string channel, text, cmd;
   int i;
   mixed *remotes;
   int *prevents;

   if(guestp(this_object()))
   {
      notify_fail("Intermud-Kanäle stehen Gästen leider nicht zur "
                  "Verfügung.\n");
      return 0;
   }
   if(!str || sscanf(str, "%s %s", channel, text) != 2)
   {
      notify_fail("+<Kurier Kanal>[:] <text>\n");
      return 0;
   }
   str = str[1..];
   if(channel == "")
   {
      notify_fail("+<Kurier Kanal>[:] <text>\n");
      return 0;
   }
   if(channel[<1] == ':')
   {
      cmd = "emote";
      channel = channel[0..<2];
   }
   remotes = EVENT_MASTER->query_remote_channels();
   prevents = EVENT_MASTER->query_prevents()[this_object()->query_real_name()];
   for(i = 0; i < sizeof(remotes[EVR_ID_STRING]); i++)
      if(!strstr(remotes[EVR_ID_STRING][i], lower_case(channel)) ||
         ""+remotes[EVR_ID_NR][i] == channel)
      {
	 if(!adminp(this_player()) &&
	    this_player()->query_level() < remotes[EVR_LEVEL][i])
	    this_object()->send_message_to(this_object(),MT_NOTIFY,MA_UNKNOWN,
	    	"Du darfst auf Kanal '"+channel+
		  "' keine Meldungen ausgeben!\n");
	 else if(prevents && (member(prevents, remotes[EVR_ID_NR][i])>=0 ||
		member(prevents,"*")>=0))
	    this_object()->send_message_to(this_object(),MT_NOTIFY,MA_UNKNOWN,
	    	"Dein Kurier verweigert Dir seinen Dienst.\n");
	 else
	    CHANNEL_MASTER->send(remotes[EVR_ID_STRING][i],
				 remotes[EVR_DISTRIB][i],
				 text, cmd);
	 return 1;
      }
   this_object()->send_message_to(this_object(),MT_NOTIFY,MA_UNKNOWN,
	wrap("Kurier Kanal '"+channel+"' ist entweder kein Intermud-Kanal "
	    "oder nur lesbar!"));
   return 1;
}

/*
FUNKTION: query_echomode
DEKLARATION: int query_echomode()
BESCHREIBUNG:
Liefert 1, falls der Spieler mit dem Kurier das Echo fuer
sage und rede eingeschaltet hat, sonst 0.
Es gibt kein set_echomode.
GRUPPEN: spieler
*/

int query_echomode()
{
    return echomode;
}

nomask static int set_echomode(int i)
{
    echomode = i;
}

protected string add_commander(string orig, object wer)
{
    return (wer && stringp(wer->query_commander()))
	?sprintf("%s <- %s",orig,wer->query_commander())
	:orig;
}

static nomask void add_to_meldungspuffer(string art, string verursacher, string text)
{
    int i;
    if(!strlen(text)) return;
    if(!meldungspuffer[art])
        meldungspuffer[art]=({});
    if(!interactive(this_object()))
    {
	if(sizeof(meldungspuffer[art]) &&
	    !(meldungspuffer[art][<1][3]&PUFFER_NON_INTERACTIVE))
		meldungspuffer[art]+=({
		    ({0,"","Während Du versteinert warst:\n",
                      PUFFER_NON_INTERACTIVE
		    })
		});
        meldungspuffer[art]+=({ 
	    ({
              (art=="log")?utime():time(), verursacher,
	      text,
	      PUFFER_NON_INTERACTIVE
             })
         });
    }
    else
	meldungspuffer[art]+=({ ({ (art=="log")?utime():time(), verursacher, text, 0 }) });

    if(art!="log" && (i=sizeof(meldungspuffer[art])-MELDUNGSPUFFERGROESSE)>0)
	meldungspuffer[art]=meldungspuffer[art][i..];
    else if(art=="log" && (i=sizeof(meldungspuffer[art])-LOGPUFFERGROESSE)>0)
        meldungspuffer["log"]=meldungspuffer["log"][i..];
}

// Um gespeicherte Meldungen zu korrigieren.
// (Fuer "Ok." -> "Du sagst: ...")
static nomask void korrigiere_meldungspuffer(string art, string old, string new)
{
    if(!sizeof(meldungspuffer[art]))
	return;
    if(meldungspuffer[art][<1][0]==time() &&
       meldungspuffer[art][<1][2]==old)
    {
	if(!strlen(new))
	    meldungspuffer[art][<1..<1]=({}); //Loeschen
	else
	    meldungspuffer[art][<1][2]=new;
    }
}

/*
FUNKTION: add_to_rede_puffer
DEKLARATION: void add_to_rede_puffer(string redetext)
BESCHREIBUNG:
Spieler haben einen rede - Puffer, in dem sie die zuletzt von ihnen oder
an sie geredeten Sachen abfragen koennen.
Befehle von rede - Charakter wie beispielsweise das "hallo" des Schnullers
oder die Befehle zur Gildenkommunikation koennen mit dieser Funktion ihren
Text ebenfalls an diesen rede - Puffer anhaengen.
Diese Funktion sollte sowohl im Spieler, der redet, aufgerufen werden
(mit einem Text der Art: Du redest zu blablabla: blubber) sowie in dem
Ziel - Spieler (mit einem Text der Art: Xyz redet zu Dir: blubber).
GRUPPEN: spieler
*/
nomask void add_to_rede_puffer (string s)
{
    mixed sa;
    
    if(playerp(this_player()) &&
	(this_object()->query_ignored_player(this_player()->query_real_name())&IGN_TELL))
	    return;

    s = regreplace(s," +\\[[0-2][0-9]:[0-5][0-9]:[0-5][0-9]\\]\n$","",1);
    
    add_to_meldungspuffer("rede",
	this_player()
	    ?(add_commander(Name(this_player()), this_player()))
	    :((sizeof(sa=explode(s," redet "))>1)?sa[0]:""), s);
}

private string format_save_puffer(mixed ev)
{
    return sprintf("%8s.%02i %=-10s %=-80s\n",
	shorttimestr(pointerp(ev[0])?ev[0][0]:ev[0],0,1),
	pointerp(ev[0])?(ev[0][1]/10000):0,ev[1]||"",ev[2]);
}

private void save_puffer()
// Acthung, Ausgabe an this_player(), nicht this_object()
{
    string *log;
    log =    map(meldungspuffer["log"]||({}), #'format_save_puffer) +

        ({"\n      Nun kommen die Rede/Sage/Seele-Puffer, da die Spieler bestimmt\n"
          "      davon ausgehen, dass diese gesichert werden (sicherheitshalber).\n",
          "\n=========================== Der Rede-Puffer ===============================\n\n"
        }) + map(meldungspuffer["rede"]||({}), #'format_save_puffer) +

        ({"\n=========================== Der Sage-Puffer ===============================\n\n"
        }) + map(meldungspuffer["sage"]||({}), #'format_save_puffer) +

	({"\n=========================== Der Seele-Puffer ==============================\n\n"
	}) + map(meldungspuffer["seele"]||({}), #'format_save_puffer);

    if(MASTER_OB->log_puffer(log))
        this_player()->send_message_to(this_player(),MT_NOTIFY,MA_UNKNOWN,
		"Puffer gespeichert.\n"
                "Schreib bitte einen Brief an mudadm, um es zu erklären.\n"
		"Vergiss bitte dabei nicht zu erwähnen, dass Du den Puffer gespeichert hast.\n");
    else
        this_player()->send_message_to(this_player(),MT_NOTIFY,MA_UNKNOWN,
		"Puffer konnte nicht gespeichert werden.\n"
                "Wahrscheinlich hast Du schon zu oft diese Möglichkeit genutzt.\n"
		"Bitte setz Dich mit einem Admin in Verbindung.\n");
}

nomask void log_puffer()
{
    if (!this_player() || this_player()!=this_interactive() ||
        !previous_object() || !adminp(this_player()) ||
        (geteuid(previous_object())!=geteuid(this_player())) ||
        (object_name(previous_object())[0..14] != "/obj/zauberstab"))
        return;
    save_puffer();
}

int speicher_command(string str)
{
    if(previous_object(0) || previous_object()!=this_object() ||
	!this_player() || !this_interactive() ||
	this_player()!=this_interactive())
	    return 0;
    str = lower_case(trim(str||""));
    if((!strstr(str,"erinn") && !strstr("erinnerungen",str)) ||
	str=="puffer")
    {
	save_puffer();
	return 1;
    }
    return notify_fail(capitalize(query_verb())+" was?\n",FAIL_NOT_OBJ,1);
}

// add action des Spielers
int puffer_command (string str)
{
    mixed pufferarrays = ({});
    mixed puffernamen = ({});
    string * optionen = pufferoptionen; //Angegebene Optionen
    mixed tmp; mixed i, anzahl;
    string *ausgabe = ({});
    mixed *optaenderung=({({}),({})}); //Hier werden Optionsaenderungen gespeichert
    mapping event_liste;

    // Sicherheitscheck
    if ((this_player() != this_interactive())
          || (this_player() != this_object()))
        return 0;

    if(str && strip(str)=="?" && this_object()->help("puffer"))
	return 1;
	
    event_liste = EVENT_MASTER->query_events();
    // Die Kommandozeile teilen ('kurier <zahl>' gehoert dabei zusammen.)
    tmp = regexplode(str||"","[ ,;.:]|[kK](anal|uri(|er)|uri(|er)[kK]anal|)( |)[0-9]+")-({""," ",",",";",".",":"});

    // Die Parameter auswerten
    foreach(string what:tmp)
	switch(lower_case(what))
	{
	    case "speicher":
            case "speichere":
            case "speichern":
            case "sicher":
            case "sichere":
            case "sichern":
                save_puffer();
	        return 1;
	    case "seele":
	    case "emote":
		puffernamen = puffernamen - ({"seele"}) + ({"seele"});
		break;
	    case "rede":
	    case "red":
		puffernamen = puffernamen - ({"rede"}) + ({"rede"});
		break;
	    case "sage":
	    case "sag":
		puffernamen = puffernamen - ({"sage"}) + ({"sage"});
		break;
	    case "lokal":
		puffernamen = puffernamen - ({"sage","seele"}) + ({"sage","seele"});
		break;
	    case "befehl":
	    case "befehle":
	    case "kommando":
	    case "kommandos":
		puffernamen = puffernamen - ({"befehle"}) + ({"befehle"});
		break;
	    case "gilde": //Den Gildenkanal finden
		{
		    int *idxs;
		    string gilde=query_gilde();
		    if(!gilde)
			return notify_fail("Du gehörst keiner Gilde an.\n");
		    idxs = m_indices(event_liste);
                    // Sonderbehandlung fuer Seher, Druiden und Barden
                    if(sizeof(({"Sehergilde","Bardengilde","Druidengilde"})&
                                                                   ({gilde}))) 
                    {
                        for(i = sizeof(idxs); i--;)
                            if(event_liste[idxs[i]]["name"] == 
                                                          "Geschwistergilden")
                                puffernamen = puffernamen - ({idxs[i]}) + 
                                                                  ({idxs[i]});
                    }
                    else
                    {
		        for(i = sizeof(idxs); i--;)
			    if(event_liste[idxs[i]]["gilde"] == gilde)
			        puffernamen = puffernamen - ({idxs[i]}) + 
                                                                  ({idxs[i]});
                    }
		}
		break;
	    case "deb":
	    case "debug":
		if(wizp(this_object()))
		{
		    puffernamen = puffernamen - ({"debug"}) + ({"debug"});
		    break;
		}
		// Fall through!
	    default:
		if(what[0]=='-') // Eine (oder mehrere) Option
		{
		    foreach(string opt:explode(what[1..<1],""))
			if(!wizp(this_object()) &&
			   member((["n","N"]),opt))
			    return notify_fail("Unbekannte Option '"+opt+"'.\n");
			else if(member((["n","z","k","y","d","c"]),opt))
			    if(member(optionen,opt)>=0)
				optionen-=({opt});
			    else
				optionen+=({opt});
			else if(member((["N","Z","K","Y","D","C"]),opt))
			{
			    if(member(pufferoptionen,lower_case(opt))>=0)
			    {
				optionen-=({lower_case(opt)});
				pufferoptionen-=({lower_case(opt)});
				optaenderung[1]+=({opt});
			    }
			    else
			    {
				optionen+=({lower_case(opt)});
				pufferoptionen+=({lower_case(opt)});
				optaenderung[0]+=({opt});
			    }
			}
			else
			    return notify_fail("Unbekannte Option '"+opt+"'.\n");
		}
		else if(str2int(what,&i)!=1)	//Es ist die Anzahl an Ereignissen
		    anzahl = i;
		else
		{
		    mixed event_nr;
		    
		    // Oki, schauen wir mal, ob es ein Kurierkanal ist.
		    if(!sscanf(what,"k%!s%D",event_nr) &&
		       str2int(what,&event_nr)==1)
			    event_nr=capitalize(what);
		    if((event_nr = get_event_index(event_liste, event_nr)) < 0 ||
		       !validate_event(event_liste, event_nr, query_level(),
		           query_gilde()))
			return notify_fail("Ereignis '"+what+"' nicht gefunden.\n");
		    puffernamen = puffernamen - ({event_nr}) + ({event_nr});
		}
	}

    // Falls Optionsaenderung, dann das ausgeben.
    if(sizeof(optaenderung[0]) || sizeof(optaenderung[1]))
        this_object()->send_message_to(this_object(),MT_NOTIFY,MA_UNKNOWN,
	    wrap("Optionen " +
		(sizeof(optaenderung[0])?(liste(optaenderung[0])+" eingeschaltet"):"") +
		((sizeof(optaenderung[0]) && sizeof(optaenderung[1]))? " und ":"") +
		(sizeof(optaenderung[1])?(liste(optaenderung[1])+" ausgeschaltet"):"") +
		"."));
    else
	optaenderung = 0;
	
    // Keine Ereignisse ausgewaehlt.
    if(!sizeof(puffernamen))
    {
	if(optaenderung) // Optionen wurden geaendert, das reicht.
	    return 1;
	puffernamen += ({"befehle"}); //Standard
	if(anzahl<=0) anzahl=20;
    }
    
    if(!wizp(this_object()))
	optionen -= ({"n","N"});

    // So, nun zu den Ereignissen die Puffer besorgen
    foreach(mixed what:puffernamen)
	if(intp(what)) // Ein Kurierkanal
	{
	    pufferarrays+=({EVENT_MASTER->query_history(what)});
	    if(!sizeof(pufferarrays[<1]))
	    {
		pufferarrays=pufferarrays[0..<2];
		puffernamen-=({what});
	    }
	    else
		puffernamen[sizeof(pufferarrays)-1]=event_liste[what]["name"];
	}
	else			// Andere Kanaele
	    switch(what)
	    {
		case "seele":
		case "rede":
		case "sage":
		    if(meldungspuffer[what])
		    {
			pufferarrays+=({meldungspuffer[what]});
			puffernamen+=({what});
		    }
		    break;
		case "befehle":
		    pufferarrays+=({this_object()->query_history()});
		    if(!sizeof(pufferarrays[<1]))
		    {
			pufferarrays=pufferarrays[0..<2];
			puffernamen-=({what});
		    }
		    break;
		case "debug":
		    if(meldungspuffer[what])
		    {
		        pufferarrays+=({meldungspuffer[what]});
		        puffernamen+=({what});
		    }
		    break;
		default:
		    do_error(sprintf("Unbekannter Puffer %O!\n",what));
	    }

    if(anzahl<=0) anzahl=50;
    
    //Ok. Nun haben wir alle benoetigten Puffer zusammen und muessen
    //sie nun entsprechend der Zeit mischen...
    while(anzahl && sizeof(pufferarrays))
    {
	int maximum=pufferarrays[0][<1][0]; //Die maximale Zeit
	int j=0;
	string tstr;
	for(i=1;i<sizeof(pufferarrays);i++) 	    //Das naechste Ereignis aus
	    if(maximum<pufferarrays[i][<1][0]) j=i; //allen Kanaelen suchen

	//Ok, im Puffer j ist das naechste Ereignis
	tmp=pufferarrays[j][<1];
	tstr=tmp[2];
	if(tstr[<1]=='\n')
	    tstr=tstr[0..<2];

	if(member(optionen,"c")>=0 || member(optionen,"y")>=0)
	// Datum/ Uhrzeit hinten anhaengen
	    tstr = wrap_say(regreplace(tstr,"\n *"," ",1)+
	        " ["+shorttimestr(tmp[0],1,3-((member(optionen,"c")<0)?0:1)
		-((member(optionen,"y")<0)?0:2))+"]","");
        else if(member(tstr,'\n')<0)
	// die Kanaltexte sind unformatiert, umbrechen...
	    tstr = wrap_say(tstr,"");
	
	if(tstr[<1]=='\n') tstr=tstr[0..<2];
	if(tstr[<1]==' ') tstr=tstr[0..<2];

        // Keine besonderen Infos -> Originalformatierung
	if(tmp[3]&PUFFER_NOT_INDENT ||
	   !sizeof(optionen&((tmp[1]=="OBJ(/secure/udp/channel)")
	    ?({"z","k","d"}):({"n","k","z","d"}))))
		ausgabe = explode(tstr,"\n") + ausgabe;
	// Zusaetzliche Infos wie Zeit, Verursacher, Kanalname
	else 
	{
	    string formatstr="";
	    mixed args=({}); // Was soll ausgegeben werden?
	    i=79;
	    tstr = space(regreplace(tstr,"\n"," ",1));
	    
	    foreach(string opt:optionen)
		switch(opt)
		{
		    case "z":			// Die Zeit
			formatstr+="%8s ";
			args+=({shorttimestr(tmp[0],0,1)});
			i-=9;
			break;
		    case "n":			// Verursachername
		        if(tmp[1]=="OBJ(/secure/udp/channel)")
			    break;
			formatstr+="%=-10s ";
			args+=({tmp[1] || "Unbekannt"});
			i-=11;
			break;
		    case "k":			// Kanalname
			formatstr+="%-10s ";
			args+=({capitalize(puffernamen[j])});
			i-=11;
			break;
		    case "d":
			formatstr+="%10s ";
			args+=({shorttimestr(tmp[0],1,2)});
			i-=11;
			break;
		}
	    ausgabe = explode(apply(#'sprintf,formatstr+"%=-1."+i+"s",
		      args+({tstr})),"\n") + ausgabe;
	}
	
	pufferarrays[j][<1..<1]=({}); //Letztes Element loeschen.
	if(!sizeof(pufferarrays[j]))
	{
	    pufferarrays[j..j]=({});
	    puffernamen[j..j]=({});
	}
	anzahl--;
    }
    if (!sizeof(ausgabe))
        this_object()->send_message_to(this_object(),MT_NOTIFY,MA_UNKNOWN,
		"Der Puffer ist noch leer.\n");
    this_player()->more(ausgabe,0,0,M_AUTO_END);
    return 1;
}


// Fuer /secure/cshsh, damit der Puffer nicht floeten geht.
mapping query_meldungspuffer()
{
    if(object_name(previous_object())=="/secure/chsh")
        return meldungspuffer;
}
void set_meldungspuffer(mapping p)
{
    if(p && object_name(previous_object())=="/secure/chsh")
        meldungspuffer=p;
}

nomask static string query_pufferoptionen() {return implode(pufferoptionen,"");}
nomask static void set_pufferoptionen(string str) {pufferoptionen = explode(str,"");}

nomask static string ignore_player_allowed(string name)
{
    if(ignored_players && member(ignored_players,name))
	return 0;
    if(adminp(this_object()))
	return 0;
    if(GOETTER_REGISTER->is_wiz(name))
	return "Du kannst keine Götter ignorieren.";
    if(spielerratp(name))
	return "Du darfst Spielerräte nicht ignorieren.";
    if(sizeof(ignored_players)>=MAX_IGNORED)
	return "Du kannst maximal "+MAX_IGNORED+" Spieler ignorieren.";
    return 0;
}

nomask static void set_ignored_player(string who, int how)
{
    if(!how)
    {
	if(ignored_players)
	    m_delete(ignored_players, who);
	if(!sizeof(ignored_players))
	    ignored_players = 0;
    }
    else if(!ignored_players)
	ignored_players = ([who: how]);
    else
	m_add(ignored_players, who, how);
}


nomask static int query_ignored_player(string who)
{
    if(!ignored_players)
	return 0;
    return ignored_players[who];
}

nomask static mapping query_ignored_players()
{
    return ignored_players||([]);
}

int ooc_shout_command(string str)
{
    if(!str)
	return notify_fail("Was willst Du kundtun?\n", FAIL_WRONG_ARG);

    EVENT_MASTER->event(23, this_object(), str);
    this_object()->send_message_to(this_object(), MT_NOTIFY, MA_UNKNOWN,
	"Ok.\n");
    return 1;
}

int notruf_command(string str)
{
    string name, text;
    object ob;
    if(!str)
	return notify_fail(wizp(this_object())
	    ?"Was willst Du den anderen Göttern mitteilen?\n"
	    :"Welchen Notruf willst Du an die Götter richten?\n",
	    FAIL_WRONG_ARG);

    if(wizp(this_object()) && sscanf(str, "%s:%s",name,text))
    {
	text = trim(text);
	name = lower_case(trim(name));
	if(!(ob=find_player(name)) && strstr(name," ")<0)
	    return notify_fail(wrap(
		"Spieler '"+capitalize(name)+"' nicht gefunden."),
		FAIL_WRONG_ARG);
	
	if(ob)
	    this_object()->send_message_to(ob, MT_SENSE|MT_FAR, MA_COMM,
		wrap_say(this_object()->query_real_cap_name()+
		    " antwortet auf Deinen Hilferuf:", text));
	else
	    text = str;
    }
    else
	text = str;
    EVENT_MASTER->event(24, this_object(), text, ob && ob->query_real_cap_name());
    if(wizp(this_object()))
	this_object()->send_message_to(this_object(), MT_NOTIFY, MA_UNKNOWN,
	    "Ok.\n");
    else
        this_object()->send_message_to(this_object(), MT_NOTIFY, MA_UNKNOWN,
	    wrap_say("Du rufst zu den Göttern:", str));

    return 1;
}

int hilferuf_command(string str)
{
    int flag_antwort = strstr(query_verb(1),"antwort")!=-1;
    int kosten;

    if (!str)
        FAIL("hilferuf/hilfeantwort was?\n");

    kosten = strlen (str) / SHOUT_CHARS_PER_SP;
    if (kosten < MIN_SHOUT_COST) kosten = MIN_SHOUT_COST;
    kosten = kosten / 2;
    if (wizp(this_object()) || newbiep(this_object()))
        kosten = 0;
    if (this_object()->query_sp() < kosten)
        FAIL(wrap("Du hast nicht genug "+this_object()->query_sp_name()+"."));
    if (kosten)
        this_object()->add_sp(-kosten);
    
    EVENT_MASTER->event(25, this_object(), str, flag_antwort);   
    this_object()->send_message_to(this_object(), MT_NOTIFY, MA_UNKNOWN,
        "Ok.\n");
    return 1;
}

void add_actions()
{
    add_action("ooc_shout_command", "oocbruelle", -3);
    add_action("ooc_shout_command", "#bruelle", -3);
    add_action("notruf_command", "notrufe", -3);
    add_action("notruf_command", "#notrufe", -4);
    add_action("hilferuf_command", "hilferufe", -6);
    add_action("hilferuf_command", "#hilferufe", -7);
    add_action("hilferuf_command", "hilfeantworte", -6);
    add_action("hilferuf_command", "#hilfeantworte", -7);
}
