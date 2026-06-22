// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/i/room/death_room.c
// Description: Inherit fuer Todesraeume.

#pragma save_types

virtual inherit "/i/room";

#include <move.h>
#include <commands.h>
#include <config.h>
#include <shadow.h>
#include <description.h>

private object *players = ({});
private string nicht_tot = "Der Tod sagt: WAS TUST DU HIER? "
                   "DEINE ZEIT IST NOCH NICHT REIF!\n\n";
private string kirche = 0;
private string ende_meldung = "Der Tod sagt: DU KANNST DEM TOD NICHT ENTRINNEN!";

// --------------- Anfang Interface nach "Aussen" ---------------

static void set_nicht_tot_meldung(string s) { if (s) nicht_tot = s; }
// Spieler landet beim Tod, obwohl er gar nicht tot ist

static void set_ende_meldung(string s) { if (s) ende_meldung = s; }
// Spieler gibt 'ende' ein.

/*
FUNKTION: set_kirche
DEKLARATION: static void set_kirche(string kirche)
BESCHREIBUNG:
Damit wird der generelle Auferstehungsort gesetzt.
VERWEISE: query_kirche
GRUPPEN: spieler
*/
static void set_kirche(string s) { if (s && touch(s)) kirche = s; }
// Dateiname der Kirche, hier landet der Spieler nach dem Tod.

void get_tod() {} // zum Ueberladen da
// Erschaffen eines "Todesmonsters", falls erwuenscht

void reset() {} // zum Ueberladen da
// Clonen von Runen oder so

/*
FUNKTION: query_kirche
DEKLARATION: string query_kirche(object player)
BESCHREIBUNG:
Liefert den Auferstehungsort fuer den Spieler.
Wird 0 geliefert, so wird dieser aufgrund des Sterbeortes ermittelt.
VERWEISE: set_kirche
GRUPPEN: spieler
*/
string query_kirche(object player)
{
    return kirche;
}

// --------------- Ende Interface nach "Aussen" ---------------

int query_prevent_shadow (object shadow) { return 1; }

void init()
{
   "*"::init();
   add_action("filter", "", AA_SHORT);
   get_tod();
   if(playerp(this_player()))
   {
      if(this_player()->query_ghost() < 1)
      {
         write(nicht_tot);
	 // this_player()->query_kirche() alleine entscheidet, wo die
	 // Kirche liegt (es fragt ggf. unser query_kirche ab.).
	 // Das '|| DEFAULT_ROOM_AFTER_DEATH' ist nur zur Sicherheit.
         this_player()->move(
		this_player()->query_kirche() || DEFAULT_ROOM_AFTER_DEATH,
		([MOVE_FLAGS:MOVE_MAGIC]));
         return;
      }
      players += ({ this_player() });
      this_player()->set_ghost(1);
      set_heart_beat(1);
   }
}

<int|string> _i_death_room_forbidden_move_in(string ctrl,mapping mv_infos)
{
    return mv_infos[MOVE_OLD_ROOM] && !playerp(mv_infos[MOVE_OBJECT]);
}
<int|string> _i_death_room_forbidden_move_out(string ctrl,mapping mv_infos)
{
    return playerp(mv_infos[MOVE_OBJECT]) 
        && mv_infos[MOVE_OBJECT]->query_ghost() > 0;
}


void create()
{
   "*"::create();
   set_own_light(1);
   add_type("kunstlicht", 1);
   add_type("kein_startraum", 1);
   add_type("teleport_raus_verboten",1);
   add_type("teleport_rein_verboten",1);
   set_short("Arbeitszimmer des Todes");
    add_controller("forbidden_move_in",#'_i_death_room_forbidden_move_in);
    add_controller("forbidden_move_out",#'_i_death_room_forbidden_move_out);
   reset();
}

int * query_map_pos()
{
    return ({0,0}); // Naehe der Kirche...
}

int do_sequence (object wer, int nr)
{
    tell_object (wer,"Nicht ins Inherit springen.\n");
    return -1;
}

protected mixed desc_number(string name, mixed info, mixed* par)
{
    switch(name)
    {
	case "sequence":
	    return info[TI_VIEWER] &&
		info[TI_VIEWER]->query_ghost();
    }
    
    return ::desc_number(name, info, par);
}

void heart_beat()
{
   int i;

   players = filter(players - ({ 0 }), 
      lambda(({'pl}), ({#'== , this_object(), ({ #'environment, 'pl }) })));
   if(sizeof(players))
      for(i = sizeof(players); i--;)
      {
	 players[i]->add_ghost(1);
         if (do_sequence (players[i], players[i]->query_ghost()) == -1)
	 {
	    // players[i]->query_kirche() alleine entscheidet, wo die
	    // Kirche liegt (es fragt ggf. unser query_kirche ab).
	    // Alles andere bringt Inkonsistenzen...
	    // Das '|| DEFAULT_ROOM_AFTER_DEATH' ist nur zur Sicherheit.
	    //
	    // Es muss vor set_ghost abgefragt werden, weil
	    // set_ghost die Kirche loescht.
	    string my_church = players[i]->query_kirche() ||
			       DEFAULT_ROOM_AFTER_DEATH;
	    players[i]->set_ghost(-1);
	    clone_object("/room/death/obj/death_shadow")
	       ->init_shadow(players[i], NO_MULTI_SHADOW);
	    players[i]->close_con();
	    
	    if(!touch(my_church))
		my_church = DEFAULT_ROOM_AFTER_DEATH;
	    
	    players[i]->move(my_church, ([
            MOVE_FLAGS: MOVE_NORMAL,
	        MOVE_MSG_LEAVE: "$Der() verschwindet plötzlich.",
	        MOVE_MSG_ENTER: "$Ein(OBJ_TP,vergeistigt) erscheint plötzlich!",
            ]) );
	 }
      }
   else
      set_heart_beat(0);
}

int filter(string str)
{
   switch(query_verb_ascii())
   {
      case "ende":
      case "#ende":
	 write(ende_meldung);
      case "schaue":
      case "schau":
      case "betrachte":
      case "nehme":
      case "nehm":
      case "nimm":
      case "fehler":
      case "idee":
      case "typo":
      case "toll":
      case "wuerdige":
      case "wuerdigung":
      case "kuri":
      case "erei":
      case "hilfe":
      case "suizid":
      case "einst":
         return 0;
      default:
         if(query_verb() && query_verb()[0]=='#')
	    return 0;
	
	 write("Das ist unmöglich in Deinem nichtmateriellen Zustand.\n");
	 if(this_player()->query_wiz_level())
	 {
	    write("... als Gott andererseits, kein Problem.\n");
	    return 0;
	 }
	 return 1;
   }
}

int query_death_room() { return 1; }
