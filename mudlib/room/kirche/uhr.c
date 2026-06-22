// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/room/kirche/uhr.c
// Description:	
// Author:	

inherit "/i/room";

#include <stats.h>
#include <message.h>
#include <move.h>

#define NEUER_SAKROTE "/room/kirche/npc/sakrote"

#ifndef NEUER_SAKROTE
int questioned;
object gloeckner;
#endif

void reset()
{
   object sakrote, kasten, schluessel, key;

   if(!(kasten = present("kasten")))
   {
      kasten = clone_object("/room/kirche/obj/kasten");
      kasten->set_weight(13);
      kasten->set_name("werkzeugkasten");
      kasten->set_gender("maennlich");
      kasten->set_id(({"werkzeugkasten", "kasten"}));
      kasten->set_long("Ein rostiger Werkzeugkasten.");
      kasten->set_feel("Der Werkzeugkasten hat schon die eine oder "
        "andere Delle, und als Du mit Deinen Fingern über ihn streichst, "
	"rieselt etwas Roststaub zu Boden.");
      kasten->move(this_object());
   }
   kasten->open_con();
#ifdef UNItopia
   if(!present("mage#runenstein", kasten))
   {
      object rune = clone_object("/z/Gilden/Magiergilde/obj/rune");
      rune->set_runenstein("tym");
      rune->move(kasten, ([MOVE_FLAGS:MOVE_ERR_REMOVE]));
   }
#endif
   if(!present("schraubenschlüssel", kasten))
   {
      schluessel = clone_object("/obj/nahkampf_waffe");
      schluessel->set_id(({"schlüssel", "schraubenschlüssel","werkzeug"}));
      schluessel->set_value(30);
      schluessel->set_name("schraubenschlüssel");
      schluessel->set_gender("maennlich");
      schluessel->set_material("metall");
      schluessel->set_weight(2);
      schluessel->set_skill_path(({ "skill", "offensiv", "stumpf", "keule" }));
      schluessel->set_used_stats(({ STAT_STR }));
      schluessel->set_damage(3,6);
      schluessel->set_life(120);
      schluessel->set_long(
         "Ein ziemlich großer, wuchtiger Schraubenschlüssel. "
	 "Pass bloß auf, dass er dir nicht auf den Fuß fällt!");
      schluessel->move(kasten, ([MOVE_FLAGS:MOVE_ERR_REMOVE]));
   }
   if(!present("sakrote#key", kasten))
   {
      key = clone_object("/obj/schluessel");
      key->add_id("sakrote#key");
      key->set_long("Ein rostiger, großer Schlüssel.");
      key->move(kasten, ([MOVE_FLAGS:MOVE_ERR_REMOVE]));
   }

   kasten->close_con();

#ifndef NEUER_SAKROTE
   object oben;
   if(!(present("sakrote")) && 
      !((oben = find_object("/room/kirche/glocken"))
	 && present("sakrote", oben)))
   {
      sakrote = clone_object("/obj/monster");
      sakrote->initialize("mensch", 32);
      sakrote->set_name("sakrote");
      sakrote->set_id(({"sakrote"}));
      sakrote->set_npc_name("sakrote");
      sakrote->set_one_stat(STAT_STR, 75);
      sakrote->set_one_stat(STAT_CON, 80);
      sakrote->set_one_stat(STAT_INT, 10);
      sakrote->give_armour_level(12);

      sakrote->set_align(25);
      sakrote->set_gender("maennlich");
      sakrote->set_personal(1);
      sakrote->set_title(", der Glöckner der Kathedrale");
      sakrote->set_long(
         "Sakrote ist hier für das Uhrwerk und den Glockenstuhl "
         "zuständig. Er läutet Tag ein, Tag aus die schweren Glocken, "
         "zieht das Uhrwerk auf und verscheucht das Getier aus den Türmen. "
         "Außerdem kümmert er sich besonders um die Verdombde im Nachbar"
         "turm.");
      sakrote->move(this_object());
      sakrote->load_chat(4, ({"Ticke Tacke, Sakrote hat ne Macke.",
                              "Dong Dong Dong, dicke Berta, ich hör Dich.",
                              "Pang Pang, schwarzer Konrad, das bist Du!",
                              "Wer bist Du, wie bist Du hierher gekommen?"}));
      sakrote->load_a_chat(30,({"$Der() sagt: DOOOOOOOONNNNG!!!!",
                                "$Der() sagt: "
				   "Bald läutet die Verdombde für mich!",
                                "$Der() sagt: "
				   "Mein letztes Stündchen hat geschlagen!",
                                "$Der() sagt: Konrad, Berta, Lutzie,"
				   " so helft!" }));
      sakrote->set_parse_conversation(this_object(),({
	 "hi:  sagt && hi || sagt && hallo || sagt && [tag] || sagt && [moin]",
	 "yes: sagt && ja || sagt && klar || sagt && sicher "
	     " antwortet && ja ",
	 "no:  sagt && nein || sagt && nö || antwortet && nein "}));
   }
#else
    sakrote = find_object(NEUER_SAKROTE) || touch(NEUER_SAKROTE);
    sakrote->heim();
#endif
}

void create()
{
   add_type("kunstlicht",1);
   add_type("landeplatz","balkon");
   add_type("graben_verboten",1);
   set_own_light(1);
   set_short("Oben im Kirchturm");
   set_long(
   "Hier befindet sich das mächtige Schalt- und Räderwerk der "
   "Tadmorer Kirchenuhr. Zahlreiche Zahnräder verhaken sich in "
   "grazilen Ankern und großen Schwungrädern. Leises Ticken "
   "wird von lautem Klackern und mächtigem Quietschen übertönt, "
   "wenn sich beispielsweise außen die Minutenzeiger auf allen "
   "vier Seiten des Glockenturms um eine Einheit weiterbewegen. "
   "Kurzerhand ein faszinierendes Kunstwerk Tadmorer Feinmechanik.\n"
   "Über Dir hängen bestimmt mehrere Tonnen Schwermetall; Du "
   "fühlst Dich hier unter den schweren bronzenen Glocken "
   "etwas unwohl. An einer Seite führt eine wacklige, hölzerne "+
   "Stiege hinauf auf die Glockengalerie. Im steinernen Boden ver"
   "schwindet eine Spindeltreppe.");
   set_exits( ({ "/room/kirche/glocken",
                 "/room/kirche/treppe3" }),
	      ({ "hoch", "runter" }) );
   set_exit_msg("hoch", "$Der(OBJ_TP) klettert mutig die Stiege empor",
                        "$Ein(OBJ_TP) kommt von unten hoch gekraxelt");
   set_exit_msg("runter", "$Der(OBJ_TP) steigt die Treppe runter",
                          "$Ein(OBJ_TP) kommt von oben heruntergelaufen");

   add_v_item(([
      "name": "räderwerk",
      "id": ({"räderwerk", "werk", "schaltwerk" }),
      "gender": "saechlich",
      "long": "Viele Zahnräder greifen ineinander und bilden ein "
              "komplexes mechanisches System, das Räderwerk der Uhr."])),
   add_v_item(([
      "name": "anker",
      "gender": "maennlich",
      "long": "Ein weiteres sicher wichtiges mechanisches Detail, "
              "das das Uhrwerk am laufen hält."]));
   add_v_item(([
      "name": "schwungräder",
      "id" : ({ "räder", "schwungräder"}),
      "gender": "saechlich",
      "plural": 1,
      "long": "Die Schwungräder halten das ganze System am laufen, "
              "doch dann und wann muss es von einem Fachmann aufgezogen "
              "werden."]));
   add_v_item(([
      "name": "minutenzeiger",
      "gender": "maennlich",
      "id" : ({ "zeiger", "minutenzeiger"}),
      "plural": 1,
      "long" : "Die Zeiger der Uhr sind nur von außen zu sehen!",
      "look_msg" : "$Der() versucht die Zeiger der Uhr zu finden"]));
   add_v_item(([
      "name": "glocken",
      "gender": "weiblich",
      "id": ({ "glocken", "tonnen", "schwermetall", "glockenstuhl" }),
      "plural": 1,
      "long": "Von hier erkennst Du mehrere große und kleine Glocken, an "
              "denen jeweils ein Seil zum Läuten angebracht ist.",
      "look_msg": "$Der() schaut nach oben in den Glockenstuhl"]));
   add_v_item(([
      "name": "stiege",
      "gender": "weiblich",
      "id":({"stiege", "holzstiege"}),
      "look_msg": "$Der() betrachtet misstrauisch die Holzstiege",
      "long": "Sie könnte Dich tragen, anderseits könnte sie auch jeden "
              "Moment zusammenstürzen."]));
   add_v_item(([
      "name": "glockengalerie",
      "gender": "weiblich",
      "id":({"glockengalerie", "galerie"}),
      "long": "Die Galerie dient hauptsächlich dazu, um an die "
              "Glockenstraenge heranzukommen, mit denen die schweren "
              "Kirchenglocken in Bewegung gesetzt werden. Eine "
              "morsch aussehende Holzstiege führt von hier hinauf."]));
   add_v_item(([
      "name": "seile",
      "id":({"seile", "seil"}),
      "gender": "saechlich",
      "plural":1, 
      "look_msg": "$Der() schaut die armdicken Stränge an den Glocken an",
      "long": "Am besten läutet man die Glocken, in dem man auf die "
      "Glockengalerie hinaufsteigt, sich an einem der armbreiten Seile "
      "festhält und sich dann mit denn Füßen von der Galerie abstößt."]));
   reset();
}

#ifndef NEUER_SAKROTE
void init()
{
   if(present("sakrote") && player_present() && 
      find_call_out("question") < 0 && find_call_out("answer") < 0 &&
      find_call_out("dont_init") < 0)
      call_out("question", 12);
}

void question()
{
   object sakrote;
  if(this_player() && (sakrote = present("sakrote")) && present(this_player()))
  {
     sakrote->do_command("sage Soll ich Dir die Glocken zeigen?\n");
     questioned = 1;
     call_out("answer", 60);
  }
}

void answer()
{
   object sakrote;
   if(this_player() && (sakrote = present("sakrote")) && present(this_player()))
      sakrote->do_command("sage Dann halt nicht.\n");
   questioned = 0;
}

void hi()
{
   string anrede;
   object sakrote;

   if(sakrote = present("sakrote"))
   {
      if (this_player()->query_invis())
      {
          sakrote->send_message(MT_LOOK,MA_EMOTE,
		  Der(sakrote)+" schaut sich verwirrt um.\n");
	  sakrote->do_command("sage Wer ist da ?\n");
	  return;
      }
      switch(this_player()->query_gender())
      {
	 case "maennlich":
	    anrede = "junger Mann"; break;
	 case "weiblich":
	    anrede = "schöne Frau"; break;
	 default:
	    anrede = "edles Wesen"; break;
      }
      sakrote->do_command("sage Einen schönen Tag, "+anrede+"!");
      sakrote->send_message(MT_NOISE,MA_EMOTE,Der(sakrote)+" kichert irre.\n");

      if(present("sakrote") && player_present() &&
	 find_call_out("question") < 0 && find_call_out("answer") < 0)
	 call_out("question", 12);
   }
}


void no()
{
   object sakrote;
   if(questioned)
   {
      if(sakrote = present("sakrote"))
	 sakrote->do_command("sage Dann halt nicht.\n");
      remove_call_out("answer");
      questioned = 0;
   }
}

void yes()
{
   object sakrote;
   if(questioned)
   {
      if(sakrote = present("sakrote"))
      {
	 sakrote->do_command("sage Gut dann komm mit!");
	 call_out("leave_uhr", 5);
      }
      remove_call_out("answer");
      questioned = 0;
   }
}

void leave_uhr()
{
   if(this_player() && (gloeckner = present("sakrote")))
   {
      gloeckner->do_command("hoch");
      call_out("await_follower", 10);
   }
}

void await_follower()
{
   if(this_player() && gloeckner)
   {
      if(present(this_player(), environment(gloeckner)))
	 call_out("zeige_glocken",5);
      else
      {
	 gloeckner->do_command(
	    "rede "+this_player()->query_real_name()+
	    " Kommst Du? Ich dachte Du wolltest meine schönen Drei bewundern.");
	 call_out("zurueck_fail", 5);
      }
   }
}

void zurueck_fail()
{
   if(gloeckner && this_player())
   {
      if(present(this_player(), environment(gloeckner)))
      {
	 gloeckner->do_command("sage Schön, dass Du doch noch hochgekommen "
			       "bist!");
	 call_out("zeige_glocken",5);
      }
      else
      {
	 call_out("dont_init", 10);
	 gloeckner->do_command("runter");
	 gloeckner->do_command("sage Lässt mich hier glatt umsonst "
				 "raufklettern. Mist!");
	 call_out("check_besitz", 5);
      }
   }
}

void zurueck(int flag)
{
   if(gloeckner)
   {
      call_out("dont_init", 10);
      gloeckner->do_command("runter");
      if(flag)
	 gloeckner->do_command("sage Ich war doch noch gar nicht fertig!");
      call_out("check_besitz", 5);
   }

}

void check_besitz()
{
   object kasten;
   if(!gloeckner)
      return;
   if(kasten = present("kasten"))
   {
      gloeckner->send_message(MT_LOOK,MA_LOOK,
	      Der(gloeckner)+" untersucht den Werkzeugkasten.\n");
      if(sizeof(all_inventory(kasten))!=3)
      {
	 gloeckner->send_message(MT_NOISE,MA_EMOTE,Der(gloeckner)+
	    " stampft wütend mit dem Fuß auf!\n");
	 gloeckner->do_command("sage Hast Du mir meine Sachen geklaut?");
	 gloeckner->send_message(MT_NOISE,MA_EMOTE,
		 Der(gloeckner)+" jammert herum.\n");
      }
   }
   else
   {
      gloeckner->do_command("sage Wo ist mein Kasten hin?");
      gloeckner->send_message(MT_NOISE,MA_EMOTE,
	      Der(gloeckner)+" jammert herum.\n");
   }
}

void zeige_glocken()
{
   if(gloeckner && this_player())
      if(present(this_player(), environment(gloeckner)))
      {
	 gloeckner->do_command("sage Das ist die dicke Berta!");
	 gloeckner->do_command("zeige "+
			       this_player()->query_real_name()+" berta");
	 call_out("zeige_g2", 15);
      }
      else
	 call_out("zurueck", 5, 1);
}
void zeige_g2()
{
   if(gloeckner && this_player())
      if(present(this_player(), environment(gloeckner)))
      {
	 // gloeckner->send_message(MT_UNKNOWN,MA_UNKNOWN,"\n");
	 gloeckner->do_command("sage Und das ist der schwarze Konrad!");
	 gloeckner->do_command("zeige "+
			       this_player()->query_real_name()+" konrad");
	 call_out("zeige_g3", 15);
      }
      else
	 call_out("zurueck", 5, 1);
}

void zeige_g3()
{
   if(gloeckner && this_player())
      if(present(this_player(), environment(gloeckner)))
      {
	 // gloeckner->send_message(MT_UNKNOWN,MA_UNKNOWN,"\n");
	 gloeckner->do_command("sage Und mein Liebling, die kleine Lutzie.");
	 gloeckner->do_command("zeige "+
			       this_player()->query_real_name()+" lutzie");
	 call_out("gehen_wir", 5);
      }
      else
	 call_out("zurueck", 5, 1);
}

void gehen_wir()
{
   if(gloeckner)
   {
       gloeckner->do_command("sage Die anderen sind nicht so interessant. Gehen wir!");
       call_out("zurueck", 5);
   }
}

#endif
