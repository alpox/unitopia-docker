// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/i/monster/accept_objects.c
// Description: Funktionen fuer Monster zum Empfang von Gegenstaenden
// Author:	Garthan	(04.02.94)
//		Freaky (10.03.1998) message auf send_message umgebaut.
//              Parsec (04.01.2000) exec_command an Stellen wo Objekte
//                                  angesprochen werden verwendet

#pragma save_types
#pragma strong_types

// Der Container wird hier inheritet, so
// dass accept_objects::moved_in
//      contain::moved_in aufrufen kann.
inherit "/i/contain";

#include <move.h>
#include <message.h>
#include <invis.h>

int do_command(string arg);
int exec_command( varargs mixed command);


private closure *ao_funcs;         // Array, das alle closures enthaelt.
private mixed *ao_ids;             // Array, das zu jeder closure
				   // eine id enthaelt. (index korreliert)
#ifndef AUTO_COUNTOB
private object *transfer_obs = ({});
#else
nosave private mapping transfer_obs = ([:2]);
nosave private mapping countob_joins = ([:0]);
#endif /* AUTO_COUNTOB */

/*
FUNKTION: set_accept_objects
DEKLARATION: void set_accept_objects(mixed *)
BESCHREIBUNG:
Mit Hilfe von set_accept_objects() kann man einem Monster leicht beibringen,
etwas anzunehmen und mit diesem Objekt dann etwas zu machen.
Auch kann man einfach alle Gegenstaende ablehnen, sprich an den Absender
zurueckgeben.

Beispiel:

   Im create() des Monsters wird set_accept_objects so aufgerufen:

   set_accept_objects(({#'accept_from_void,
                        #'accept_invis,
                        #'bonbon, "bonbon",
                        #'fackel, "fackel", "lampe",
                        #'accept, "taler",
                        #'other,
                        #'refuse}));

   Sobald jetzt in dieses Monster etwas hineingeschoben wird,
   wird diese Liste von Funktionen abgearbeitet:

   Alle Objekte die vorher kein Environment hatten werden reingelassen.
   (accept_from_void)
   Alle Objekte, die unsichtbar sind, werden reingelassen. (accept_invis)

   Dann wird geprueft, ob das uebergebene Objekt die Id "bonbon" hat,
   wenn ja wird im Monster die Funktion bonbon aufgerufen, die z.B.
   so aussehen kann:

      int bonbon(object bonbon)
      {
	 exec_command("sage Hmmm! Danke!");
	 exec_command("lutsche", bonbon);
	 return 1;
      }

   Die Funktion erhält folgende Parameter:
       object ob:         Das erhaltene Objekt.
       object old_room:   Der vorherige Aufenthaltsort
       object monster:    Das Monster, welches das Objekt erhalten hat.
       object tp:         Das Objekt, welches zum Zeitpunkt der Übergabe
                          this_player() war.
       mapping move_info: Alle move-Infos zur Bewegung (siehe move()).

   Liefert die Funktion 1 zurueck, so wird die Abarbeitung der obigen
   Liste beendet.

   Liefert die Funkion dagegen 0, dann wird mit dem naechsten Eintrag
   fortgefahren.

   Hinter jeder Funktion koennen mehrere Ids stehen, die alle
   durchgetestet werden.

   Stehen hinter der Funktion in der obigen Liste KEINE Ids, (wie
   bei #'other und #'refuse) dann wird die Funktion fuer jedes Objekt
   aufgerufen, das in das Monster kommt.

   Die Funktionen #'accept, #'accept_from_void, #'accept_invis und #'refuse
   sind vordefiniert.
   accept liefert einfach nur 1 zurueck, sprich das Objekt wird einfach
   kommentarlos akzeptiert (so wie bisher).
   accept_from_void liefert 1 zurueck, wenn das Objekt vorher keine
   Umgebung hatte, sonst 0.
   accept_invis liefert 1 zurueck, wenn das Objekt unsichtbar war, sonst 0.
   Die Funktion #'refuse gibt dem Uebergeber (this_player) den Gegenstand
   kommentarlos zurueck.

   Gegenstaende, auf die keine Funktion passt, werden einfach vom
   Monster behalten.

   Moechte man das Monster von einem anderen Objekt aus steuern,
   muessen die Closures etwas anders aussehen. In unserem Beispiel
   waere das dann so:

   #include <accept_objects.h>
   ...
   lisa->set_accept_objects(({AO_ACCEPT_FROM_VOID,
                              AO_ACCEPT_INVIS,
			      AO_CALL(bonbon), "bonbon",
			      AO_CALL(fackel), "fackel",
			      AO_ACCEPT,        "taler",
			      AO_CALL(other),
			      AO_REFUSE}));

   Anmerkung: Erwartet man vom Spieler Geld, so kann man dessen
	      (Uebergabe-)Wert in der entsprechenden Funktion *nicht* mit
	      query_money() abfragen, weil zu diesem Augenblick
	      alle vorher schon vorhanden Geldobjekte (derselben
	      Waehrung) zu diesem neuen dazuaddiert wurden.
	      Man erhaelt mit query_money() also die neue Summe an
	      Talern, bzw Dukaten etc, die das Monster jetzt bei sich traegt.
	      Moechte man dagegen den gerade *uebergebenen* Geldbetrag
	      erfahren, so verwendet man statt query_money, die Funktion
	      query_transaction_value().

VERWEISE: accept, accept_from_void, refuse, give_object
GRUPPEN: monster
*/

void set_accept_objects(mixed * set)
{
   int anz, i, got_one;
   closure cl;

   ao_funcs = ao_ids = ({});

   for(i = 0, anz = sizeof(set); i < anz; i++)
      if(closurep(set[i]))
      {
	 if(cl && !got_one )
	 {
	    ao_funcs += ({ cl });
	    ao_ids   += ({ 0 });
	 }
	 got_one = 0;
	 cl = set[i];
      }
      else
      {
	 ao_funcs += ({ cl });
	 ao_ids   += set[i..i];
	 got_one++;
      }

   if(anz && closurep(set[anz-1]))
   {
      ao_funcs += ({ set[anz-1] });
      ao_ids   += ({ 0 });
   }
}

#ifndef AUTO_COUNTOB

<int|string> let_not_in(mapping mv_infos)
{
    object what = mv_infos[MOVE_OBJECT];
   int i;
   <int|string> res;
   string v;

   if(!(res = contain::let_not_in(mv_infos)) &&
      sizeof(ao_funcs) &&
      objectp(what) &&
      what->query_count() &&
      (v = what->query_count_type()))
      for(i = sizeof(transfer_obs-=({0})); i--;)
	 if(objectp(transfer_obs[i]) &&
	    transfer_obs[i]->query_count() &&
	    transfer_obs[i]->query_count_type() == v)
	 {
	    if(this_player())
	    {
	       this_object()->send_message(MT_LOOK,MA_PUT,
		  wrap(Der(this_player())+" versucht "+dem()+" "+
		      capitalize(what->query_plural_name())+" zu geben, "+
		      er()+" ist aber noch mit Zählen beschäftigt."),
		  wrap("Du versuchst "+dem()+" "+
		  capitalize(what->query_plural_name())+" zu geben, "+
		  er()+" ist aber noch mit Zählen beschäftigt."),
		  this_player());
	       this_object()->send_message(MT_NOISE,MA_COMM,
                Der()+" sagt: Einer nach dem anderen!\n");
	    }
	    res = 1;
	    break;
	 }
   return res;
}

#endif /* AUTO_COUNTOB */

void moved_in(mapping mv_infos)
{
    object what = mv_infos[MOVE_OBJECT];
    object woher = mv_infos[MOVE_OLD_ROOM];
    int way = mv_infos[MOVE_FLAGS];

   if (!objectp(what))
   {
      // Abkuerzung, solche 0-Objekt-Bewegungen ignorieren.
      ::moved_in(mv_infos);
      return;
   }
#ifndef AUTO_COUNTOB
   object ancestor, origin;
#else
   if(way & MOVE_ATOM_NOT_NOTIFY) // MOVE_SECRET
   {
      // Abkuerzung, solche Bewegungen ignorieren.
      ::moved_in(mv_infos);
      return;
   }
#endif /* AUTO_COUNTOB */

   if(sizeof(ao_funcs) && !what->query_invis() &&
#ifndef AUTO_COUNTOB
   	this_player() != this_object())
#else
   	this_player() != this_object() && !what->undo_split())
   {
      if(what->query_count())
         // Wir schauen mal, ob es Geld/Objekte gleichen Ursprungs gibt.
         foreach(object tob, object torig, object tpl: transfer_obs)
            if(tob->query_count() &&
	       what->query_count_type()==tob->query_count_type() &&
	       tpl == this_player() && torig == woher)
	    	   // Und packen es dann zusammen
	           what->join_object(tob);

      m_add(transfer_obs, what, woher, this_player());

      call_out("got_object", 0, what, woher, this_player(), mv_infos);
   }
   else if(what->query_count())
#endif /* AUTO_COUNTOB */
   {
#ifndef AUTO_COUNTOB
      transfer_obs +=  ({ what });
          // Eigentlicher Ursprung des Geldes ermitteln, falls gesplittet worden
      if(!woher && objectp(what) && what->query_count())
	 origin = objectp(ancestor = what->query_ancestor()) ?
	    environment(ancestor) : 0;
      call_out("got_object", 0, what, origin || woher, this_player(), mv_infos);
#else
	countob_joins += ([ what ]);
	what->join_objects();
#endif /* AUTO_COUNTOB */
   }
   ::moved_in(mv_infos);
}

static void got_object(object what, object woher, object ti, mapping mv_infos)
{
   int anz, i;

#ifndef AUTO_COUNTOB
   transfer_obs -= ({ what, 0 });
#else
   m_delete(transfer_obs, what);
#endif /* AUTO_COUNTOB */

   if(what && present(what, this_object()) && (anz = sizeof(ao_funcs)))
      for(i = 0; what && (i < anz); i++)
	 if((!ao_ids[i] || what->id(ao_ids[i])) &&
	    funcall(ao_funcs[i], what, woher, this_object(), ti, mv_infos))
	    break;

#ifdef AUTO_COUNTOB
    if(what && present(what, this_object()) && what->query_count())
    {
	countob_joins += ([ what ]);
	what->join_objects();
    }
#endif /* AUTO_COUNTOB */
}

/*
FUNKTION: give_object
DEKLARATION: void give_object(object ob, object player)
BESCHREIBUNG:
Die Funktion give_object() uebergibt dem angeben Spieler <player>
den Gegenstand <ob> (mit Hilfe des Monsterkommandos "gebe").
Spieler <player> muss beim Monster sein, und das Monster muss den
Gegenstand bei sich haben.
Kann der <player> den Gegenstand nicht annehmen, so legt ihn
das Monster in seine Umgebung.
Diese Funktion wird auch von refuse benutzt, das wiederum
in set_accept_objects verwendet werden kann.
VERWEISE: set_accept_objects, refuse, accept, accept_from_void, accept_invis
GRUPPEN: monster
*/

void give_object(object got, object who)
{
    if(who && who->query_id())
	exec_command( "gebe", got, "an", who);
    if(got && !present(got, who))
	exec_command("lege", got);
}

/*
FUNKTION: accept
DEKLARATION: int accept(object was, object woher)
BESCHREIBUNG:
siehe set_accept_objects
VERWEISE: refuse, accept_from_void, accept_invis, set_accept_objects,
          give_object
GRUPPEN: monster
*/

int accept(object got, object woher)
{
   return 1;
}

/*
FUNKTION: accept_from_void
DEKLARATION: int accept_from_void(object was, object woher)
BESCHREIBUNG:
siehe set_accept_objects
VERWEISE: accept, refuse, set_accept_objects, accept_invis, give_object
GRUPPEN: monster
*/
int accept_from_void(object was, object woher)
{
   if(!woher)
      return 1;
}

/*
FUNKTION: accept_invis
DEKLARATION: int accept_invis(object was, object woher)
BESCHREIBUNG:
siehe set_accept_objects
VERWEISE: accept, refuse, accept_from_void, set_accept_objects, give_object
GRUPPEN: monster
*/
int accept_invis(object was, object woher)
{
   if(objectp(was) && was->query_invis())
      return 1;
}


/*
FUNKTION: refuse
DEKLARATION: int refuse(object was, object woher, object acceptor)
BESCHREIBUNG:
siehe set_accept_objects
VERWEISE: accept, accept_from_void, accept_invis, set_accept_objects,
          give_object
GRUPPEN: monster
*/

int refuse(object got, object woher, object acceptor)
{
   if(got && acceptor && woher)
   {
      string einen_got = einen(got);
      
      if(acceptor->query_in_fight() && living(woher))
      {
         if(got->id("wurfwaffe"))
         {
             //acceptor->do_command("fuehre "+got->query_id()[0]);
            acceptor->exec_command("führe", got);
            // acceptor->do_command("werfe "+got->query_id()[0]+" nach "+
            //                      woher->query_name());
            got->do_wurf(woher);
         }
      }
      else
	 if(present(woher, environment(acceptor)) && !IS_INVIS(woher))
	    give_object(got, woher);
	 else if(got->move(environment(acceptor)) == MOVE_OK)
	    acceptor->send_message(MT_LOOK,MA_PUT,Der(acceptor)+" legt "+einen_got+" hin.\n");
	 else
	    got->remove();
	       
      if(!got || !acceptor || !present(got,acceptor)) return 1; // Wir sind's losgeworden.
      
      if(got->move(environment(acceptor))==MOVE_OK) // Er hats immer noch
         acceptor->send_message(MT_LOOK,MA_PUT,wrap(Der(acceptor)+" lässt "+einen_got+" fallen."));
      else
         got->remove();
   }
   return 1;
}

#ifdef AUTO_COUNTOB
private void late_join_countob(object alt, object neu)
{
    if(!alt || !neu)
	return;

    if(!member(transfer_obs, alt))
	countob_joins += ([ alt ]);
    if(!member(transfer_obs, neu))
    {
	countob_joins += ([ neu ]);
	neu->join_objects();
    }
}

private int f_join_countob(string controller, object alt, object neu)
{
    if(!sizeof(ao_funcs))
	return 0;

    if(member(countob_joins, alt) && member(countob_joins, neu))
	return 0;
    
    call_out(#'late_join_countob, 1, alt, neu);
    return 1;
}

void create()
{
    this_object()->add_controller("forbidden_join_countob", #'f_join_countob);
}
#endif /* AUTO_COUNTOB */
