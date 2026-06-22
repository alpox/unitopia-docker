// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/room/hlp/wolke.c
// Description:	Die Engels-Wolke
// Author:	
// Modified by:	Tmm (11.06.00) Anpassung an neue Vaniorh-Pfade,
//                             look_rand repariert

inherit "/i/hlp/room";

#include <hlp.h>
#include <move.h>
#include <deklin.h>
#include <eyes.h>
#include <level.h>
#include <invis.h>
#include <message.h>

#ifdef UNItopia
#include "/d/Vaniorh/sys/vaniorh_exports.h"
#endif

#define WOHER   "/room/kirche/balkon"
#define ILE_DE_LA_CITE "plaza"
#define HLPTOOL "/obj/hlptool"

object *invs = ({ });

string look_loch()
{
      return wrap(
      "Dies scheint der kürzeste Weg zum Kartukultininurta-Platz zu sein. "+
      (hlpp(this_player()) ?
	 "Als Engel sollte der kurze Gleitflug nach unten kein Problem "
	 "darstellen. Unten kannst Du schon die Normalsterblichen auf dem "
	 "Platze umherirren sehen...":
	 "Du wirst ganz bleich im Gesicht als Du die Tiefe starrst. Dieses "
	 "Loch "
	 "scheint hauptsächlich für Engel gedacht zu sein, die mit ihren "
	 "Flügeln ganz leicht in die Tiefe gleiten können, aber für "
	 "Dich scheint dieser Weg ungangbar, obwohl er Dich in einem sehr "
	 "kurzen Sturzflug hinab auf den Kartukultininurta-Platz bringen "
	 "würde, was aber für alle Beteiligten keine Freude wäre."));
}

string look_rand()
{
   object * obs, env;
   int i, found;

   for(i = sizeof(obs = users()); i--;)
      if((env = environment(obs[i])) && 
          strstr(object_name(env),ILE_DE_LA_CITE)!=-1)
      {
	 found = 1;
	 break;
      }
   if(found)
      return "Weit unten siehst Du:\n"+ this_player()->query_room_description(
	 env, EYE_FORCE_LONG|EYE_NO_EXITS);
   else
      return wrap(
	     "In schwindelnder Tiefe siehst Du das kleine Städtchen Tadmor "
	     "liegen. Man erkennt das Rathaus, die Basilika, den "
	     "Kartukultininurta-Platz und die drei großen Ausfallstraßen. "
	     "Außenrum die übliche Agrikulturlandschaft mittelgroßer "
	     "Kleinstädte und die dazugehörige Ruderalflora. "
	     "Im Norden erkennst Du den Berg der Götter und um all das "
	     "schließt sich in ihrer kalten Umarmung die Tadmorer See an.");

}

void reset()
{
   object ob, *obs;
   int i, anz;

   invs -= ({ 0 });

   if(ob = find_living("lyrion"))
      ob->move(this_object());
   if(!present("lyrion"))
   {
      ob = clone_object("/obj/monster");
      ob->initialize("engel", 85);
      ob->set_npc_name("lyrion");
      ob->set_name("lyrion");
      ob->set_gender("maennlich");
      ob->set_personal(1);
      ob->set_id( ({"lyrion", "engel"}) );
      ob->set_title(", der Schutzengel aller Lebenden");
      ob->set_long("So die Götter wollen, errettet Lyrion die Sterblichen "
		   "vor dem Tod. Etwas profaner sind seine Alltagsarbeiten: "
		   "Er beaufsichtigt die Wolke und sorgt dafür, dass sie "
		   "vom Gott der Stürme und Winde nicht abgetrieben wird, "
		   "außerdem bringt er verloren gegangene Sterbliche wieder "
		   "hinab auf die Erde.");
      ob->set_align(250);
      ob->move(this_object());
      ob->set_parse_conversation(this_object(),
      ({ "hi: sagt && hi || sagt && hallo || sagt && tag",
	 "erde: sagt && erde || sagt && [bring] ",
	 "gibkugel: sagt && bescheid || sagt && lyrion && [kugel] ||"
		  " sagt && [kugel] && lyrion "}));
      invs += ({ ob });
   }
   if(!present("gedenktafel"))
   {
      ob = clone_object("obj/gedenktafel");
      ob->move(this_object());
      invs += ({ ob });
   }
   if(!present("brett"))
   {
      ob = clone_object("/obj/brett");
      ob->set_brett_name("Engel");
      ob->move(this_object());
      invs += ({ ob });
   }
/*
   if(!present("liste"))
   {
      ob = clone_object("/obj/hlplist");
      ob->move(this_object());
      invs += ({ ob });
   }
*/
   if(!present("urne"))
   {
      (ob = touch("/room/wahlen/eurne"))->move(this_object());
      invs += ({ ob });
   }

   if(sizeof(filter(all_inventory(),
     (: playerp($1) && !IS_HIDDEN($1) &&
        interactive($1) && query_idle($1)<1800 :))))
	  return;
   for(i = sizeof(obs = filter(all_inventory(),
       lambda(({'a}),({#'!,({#'living,'a})}))) - invs); i--;)
   {
      if (obs[i])
	  obs[i]->remove();
      anz++;
   }
   if(anz)
      tell_room(this_object(),"Ein niederer Gott kommt bruddelnd vorbei "
                "und kehrt deinen Dreck weg!\n");
}

void create()
{
   set_own_light(1);
   add_type("flugstart_meldung","Du lässt Dich durch die Wolke hindurchsinken "
      "und fliegst davon.");
   add_type("lande_meldung","Bei der Landung auf der Wolke hast Du Mühe, nicht "
      "durch die weiche Wolke hindurchzusinken, doch es gelingt Dir schließlich, "
      "sicher zu landen.");
   add_type("kaempfen_verboten","Beim Kämpfen würdest Du grausam "
      "von der Wolke hinabstürzen.\n");
   add_type("graben_verboten","Auf einer Wolke?");
   add_type("tempel",1);
   set_short("Auf einer Wolke hoch oben am Himmel");
   set_long("Auf einer Wolke hoch oben am Himmel. Du stellst fest, "
	    "dass die wabernde Substanz der Wolke recht trittfest ist "
	    "und fühlst Dich auch sofort wohl. Da die Wolke nicht "
	    "sonderlich groß ist, kannst Du über die Ränder hinabsehen, "
	    "und das Fußvolk in den Straßen Tadmors beobachten. Im "
	    "südlichen Teil der Wolke ist ein Loch, das Dir einen leichten "
	    "Absprung nach unten ermöglicht.");
   add_v_item(([
      "name": "loch",
      "gender": "saechlich",
      "look_msg": "$Der() schaut neugierig durch das Loch in der Wolke",
      "long": #'look_loch]));
   add_v_item(([
      "name": "ränder",
      "id": ({"rand", "ränder", "abgrund", "tiefe"}),
      "plural": 1, 
      "gender": "maennlich",
      "look_msg": "$Der() schaut vorsichtig über den Rand der Wolke",
      "long": #'look_rand]));
#ifdef UNItopia      
   set_exits(({ K_PLATZ_SO }), ({ "runter" }));
   set_exit_msg("runter",
		"$Der(OBJ_TP) stürzt sich durch das Loch in die Tiefe",
		"$Ein(OBJ_TP) gleitet vom Himmel herab und landet neben Dir");
#endif   
   reset();
}

#ifdef UNItopia

static void move_with_lyrion(object lyrion,object who)
{
    if (!lyrion || !who) return;
      lyrion->send_message(MT_LOOK,MA_MOVE,
      wrap("Gesagt, getan. Lyrion nimmt "+den(who)+" bei der Hand, "
	   "entfaltet seine mächtigen Schwingen und gleitet "
	   "durch das Loch, das nach unten führt."),
      wrap("Gesagt, getan. Lyrion nimmt Dich bei der Hand, entfaltet seine "
	   "mächtigen Schwingen und gleitet durch das Loch in der Wolke "
	   "hinab auf den Kartukultininurta-Platz. Der Wind rauscht Dir "
	   "dabei kräftig um die Ohren und Du bist heilfroh als Du "
	   "gesunder Dinge unten angelangst.\n "), who);
      who->move(K_PLATZ_SO, ([ MOVE_FLAGS: MOVE_MAGIC, 
	 MOVE_MSG_LEAVE: "$Der() wird von Lyrion mitgerissen",
	 MOVE_MSG_ENTER: "Ein mächtiger Engel gleitet vom Himmel herab und "
	 "setzt $den() sanft am Boden ab"]) );
}

<int|string> filter_runter(object who)
{
   object lyrion;

   if(who && living(who) && !hlpp(who) && !wizp(who) && who->query_personal() &&
      !who->query_animal() && !(who->query_invis()&V_ATOM_NOSHIMMER) &&
      (lyrion = present("lyrion")) &&
      who->query_name() != "lyrion")
   {
      lyrion->do_command("sage Haltet ein, der Sturz in die Tiefe würde "
			 "Euch töten, flügelloses Wesen! Ich werde Euch "
			 "hinabgeleiten.");
        call_out("move_with_lyrion",0,lyrion,who);
      return 1;
   }
}
#endif

void hi()
{
   if(this_player() && !random(3))
   {
      if(wizp(this_player()))
	 previous_object()->do_command("sage Seid gegrüßt, "+
	    Wer(this_player(), ART_KEINS|ART_NO_NOMEN, "allmächtig")+"!");
      else if(hlpp(this_player()))
	 previous_object()->do_command("sage Seid gegrüßt, "+
	    "Engel "+this_player()->query_cap_name()+"!");
      else 
	 previous_object()->do_command("sage Seid gegrüßt, "+
	    Wer(this_player(), ART_KEINS|ART_NO_NOMEN, "normalsterblich")+"!");
   }
}

void erde()
{
    if(this_player() && !hlpp(this_player()))
       previous_object()->do_command("sage Geh voraus, "+
	  this_player()->query_cap_name()+". Ich geleite Dich heil zur Erde "+
	  "zurück.");
}

void moved_in(mapping mv_infos)
{
    object who = mv_infos[MOVE_OBJECT];
    object woher = mv_infos[MOVE_OLD_ROOM];
    object ly, ob;

    ::moved_in(mv_infos);

   if(woher && 
      object_name(woher) == WOHER &&
      who && !(who->query_invis()&V_ATOM_NOSHIMMER) &&
      (hlpp(who) || wizp(who)) &&
      !present("hlp#tool", this_player()) && 
      (ly = present("lyrion")))
   {
      ly->do_command("sage Willkommen in der Schar der Engel, "+
		     who->query_cap_name()+"!\n");
      if((ob = clone_object(HLPTOOL))->move(who) != MOVE_OK)
      {
	 ly->do_command("sage Du trägst zu viel mit dir rum, "+
		      who->query_cap_name()+". Ich kann Dir "
		     "keine Kristallkugel geben, leg etwas ab und sag "
		     "Bescheid. "
		     "Vergiss nicht meinen Namen zu nennen, sonst weiß ich "
		     "nicht, ob Du mit mir sprichst oder mit den anderen "
		     "Engeln.");
	 if(ob)
	    ob->remove();
      }
      else
      {
	 ly->send_message(MT_LOOK|MT_FEEL,MA_PUT,
		 wrap(Der(ly)+" gibt "+dem(who)+" "+einen(ob)+".\n"),
		     wrap(Der(ly)+" gibt Dir "+einen(ob)+".\n"), who);
	 ly->do_command("sag Nimm diese Kugel. Sie ist das Verbindungsglied "
	 "zwischen uns Engeln und den allmächtigen Göttern. Durch diese "
	 "Kugel erhältst Du die Gaben der Götter, sobald Du sie Dir "
	 "verdient hast. Nütze diesen wertvollen Gegenstand weise und "
	 "allzeit überlegt, wenn Du Dir nicht die Verbindung mit den Göttern "
	 "verderben willst. Mit 'hilfe engel' erfährst Du mehr.");
      }
   }
}

string gibkugel(string str)
{
   object ly, ob;

   if(this_player() && (ly = present("lyrion")))
   {
      if(!hlpp(this_player()) && !wizp(this_player()))
	 ly->do_command("sage Dir darf ich keine Kristallkugel aushändigen!");
      else if(present("hlp#tool", this_player()))
	 ly->do_command("sage Du hast doch bereits eine Kristallkugel, "+
			this_player()->query_cap_name());
      else if((ob = clone_object(HLPTOOL))->move(this_player()) != MOVE_OK)
      {
	 ly->do_command("sage Du trägst zu viel mit dir rum, "+
		      this_player()->query_cap_name()+". Ich kann Dir "
		     "keine Kristallkugel geben, leg etwas ab und sag "
		     "Bescheid. "
		     "Vergiss nicht meinen Namen zu nennen, sonst weiß ich "
		     "nicht, ob Du mit mir sprichst oder mit den anderen "
		     "Engeln.");
	 if(ob)
	    ob->remove();
      }
      else
      {
	 ly->send_message(MT_LOOK|MT_FEEL,MA_PUT,
	    wrap(Der(ly)+" gibt "+dem(this_player())+" "+ einen(ob)+".\n"),
	    wrap(Der(ly)+" gibt Dir "+einen(ob)+".\n"), this_player());
	 ly->do_command("sag Nimm diese Kugel. Sie ist das Verbindungsglied "
	 "zwischen uns Engeln und den allmächtigen Göttern. Durch diese "
	 "Kugel erhältst Du die Gaben der Götter, sobald Du sie Dir "
	 "verdient hast. Nütze diesen wertvollen Gegenstand weise und "
	 "allzeit überlegt, wenn Du Dir nicht die Verbindung mit den Göttern "
	 "verderben willst. Mit 'hilfe engel' erfährst Du mehr.");
      }
   }
}
