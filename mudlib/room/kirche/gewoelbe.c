// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/room/kirche/gewoelbe.c
// Description:	
// Author:	

inherit "/i/room";

#include <message.h>
#include <parse_com.h>

object door;

void reset()
{
   object maus;
   if(!present("fledermaus"))
   {
      maus = clone_object("/obj/monster");
      maus->initialize("fledermaus", 12);
      maus->set_gender("weiblich");
      maus->set_long("Eine kleine ledrige Fledermaus, "
                     "die hier im Gebälk hängt.");
      maus->set_eatable_corpse(1);
      maus->set_weight(2);
      maus->add_id(({"Kirchenmaus","Kirchentier"})); // Versteckte IDs
      maus->move(this_object());
   }
   if(!present("taube"))
   {
      maus = clone_object("/obj/monster");
      maus->initialize("taube", 8);
      maus->add_id(({"vogel","Kirchentaube","Kirchentier"}));
      maus->set_gender("weiblich");
      maus->set_weight(4);
      maus->set_long("Eine kleine, weiße Taube. Manche Leute bezeichnen "
		     "diese Tiere auch als Flugratten...");
      maus->set_eatable_corpse((["braten_set_name":"taubenbraten"]));
      maus->move(this_object());
   }

   if(!door)
   {
      door = clone_object("/obj/tuer");
      door->set_keys(({"sakrote#key"}));
      door->set_pass_cmd("süden");
      door->set_door_exit("/room/kirche/treppe4");
      door->set_long("Die Tür führt in den Südturm.");
      door->move(this_object());
   }
   door->lock_door();
}

void init()
{
   if(player_present() && find_call_out("action") < 0)
      call_out("action", random(5)+5);
   add_action("fange", "fange", -4);
}

int fange(string str)
{
   mixed parsed=parse_com(str,this_object());
   if(parse_com_error(parsed, "Fange was?\n",1))
      return 0;
   parsed = parsed[PARSE_OBS][0];

   if(!parsed->id("Kirchentier"))
      return notify_fail("Was willst Du fangen?\n");

   if(!living(parsed))
      return notify_fail("Du willst "+einen(parsed,({"tot"}))+" fangen?\n");

   this_player()->send_message(MT_LOOK,MA_UNKNOWN,
      wrap(Der(this_player())+" versucht erfolglos " + einen(parsed) +
         " zu fangen."),
      wrap("Du versucht "+den(parsed)+" zu fangen, bist aber zu langsam."),
      this_player());
   return 1;
}

void action()
{
   object ob;
   if(random(2) && (ob = cond_present("Kirchentaube",this_object(),#'living)))
      ob->send_message(MT_NOISE,MA_NOISE,"Die Taube gurrt vor sich hin.\n");
   else if(ob = cond_present("Kirchenmaus",this_object(),#'living))
      ob->send_message(MT_LOOK,MA_MOVE,
	      wrap("Die Fledermaus fliegt erschrocken auf und "
                  "hängt sich an einem anderen Balken auf."));
   else 
      this_object()->send_message(MT_NOISE,MA_UNKNOWN,
	      "Es knirscht im Gebälk.\n");
   if(player_present() && find_call_out("action") < 0)
      call_out("action", 15+random(20));
}

string query_long(object who)
{
   string res;
   res =
   "Dies ist sozusagen der Dachboden der Kathedrale. Über Dir befindet "
   "sich das Dach und unter Dir das Gewölbe des Mittelschiffs. "
   "Der Boden ist bekleckert mit dem Kot von Tauben und Fledermäusen. "
   "Diese Tiere halten sich bevorzugt hier an diesem stillen Ort auf, "
   "an dem kaum ein Mensch vorbeikommt. "
   "Im Norden und Süden setzen sich die Türme der Kathedrale nach oben "
   "hin fort. Ein Durchgang führt nach Norden";

   if(door)
      if(door->query_door_is_open())
	 res += ", eine offene Tür nach Süden in den anderen Turm";
      else 
	 res +=
	 ", während die Passage in den Südturm von einer wuchtigen Tür "
	 "versperrt wird";
   res += ".";
   return wrap(res);
}

void create()
{
   set_own_light(1);
   add_type("kunstlicht", 1);
   add_type("landeplatz","balkon");
   add_type("graben_verboten",1);
   set_short("Unterm Kirchendach");

   add_v_item(([
      "name": "dach",
      "id": ({ "dach", "kirchendach", "unterseite"}),
      "gender": "saechlich",
      "look_msg":"$Der() betrachtet das Kirchendach, zumindest die Unterseite",
      "long": 
   "Über Dir spannt sich das weite mit Ziegeln bedeckte Dach vom Westteil "
   "des Mittelschiffs, über dem Du Dich gerade befindest, bis vor zur "
   "Gewölbedecke über der Apsis. Das Gebälk über den Querschiffen kannst "
   "Du von hier allerdings nicht sehen."]));
   add_v_item(([
      "name": "gewölbe",
      "id":({ "gewölbe", "boden", "fußboden" }),
      "gender": "saechlich",
      "long": 
   "Der Boden besteht hier, am westlichen "
   "Ende des geräumigen Dachbodens, aus Holzplanken, während man "
   "weiter hinten, über der Apsis und den Querschiffen, die Planken "
   "weggelassen hat, so dass dort der Boden nur aus der Oberseite der "
   "von unten so prächtig aussehenden Gewölbekuppeln besteht und keineswegs "
   "eben ist. Er ist bekleckert von Tauben- und Fledermauskot, die hier "
   "oben in der Abgeschiedenheit ihr Dasein führen."]));
   add_v_item(([
      "name": "gebälk",
      "gender": "saechlich",
      "long": "Das Gebälk bildet die Unterseite des Kirchendaches und "
	      "bietet Schutz für Tauben und Fledermäuse, weil nahezu "
	      "unerreichbar für nichtakrobatische Menschen."]));
   add_v_item(([
      "name": "türme",
      "gender": "maennlich",
      "plural": 1,
      "long": "Viel kannst Du von hier aus nicht erkennen. Über den "
              "Nordturm bist Du zumindest hierhergekommen."]));
   set_smell("Es riecht nach ätzendem Taubenmist.\n");
   set_exits(({"/room/kirche/treppe2" }), ({ "norden" }));
   reset();
}
