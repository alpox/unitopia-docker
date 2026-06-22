// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/room/rathaus/foyer.c
// Description: Die grosse Eingangshalle des Rathauses
//              Tmm         11.06.00 - Anpassung an neues Vaniorh

inherit "/i/room";

#include <quest.h>
#include <level.h>
/*
#ifdef UNItopia
#include "/d/Vaniorh/sys/vaniorh_exports.h"
#endif
*/
void reset()
{
   object buch, brett, zettel;
   string *quests, *colours, desc;

   if(!present("abenteurer-buch"))
   {
      buch = clone_object(abs_path("obj/adv_book"));
      buch->move(this_object());
   }
   if(!present("urne"))
      touch("/room/wahlen/urne")->move(this_object());
   if(!present("brett"))
   {
      brett = clone_object("/obj/brett");
      brett->set_bretter(({"Computer", "Flohmarkt", "Medien", "Smalltalk",
			   "Schiffe", "Spieler", "Traegerkreis", "InterMUD"}));
      // brett->set_brett_name("Spieler");
      brett->move(this_object());
   }
   if(!present("quest_zettel") &&
      sizeof(quests = QUEST_ROOM->query_quest_objects(Q_NOT_NECESSARY)))
   {
      colours = ({ "rot", "gelb", "grün", "blau",
	           "schwarz", "braun", "grau", "violett" });
      desc = touch(quests[random(sizeof(quests))])->query_hint();
      desc = implode(explode(desc,"\n")," ");
      desc = sprintf("%-=50s",desc);
      desc = implode(explode("\n"+desc,"\n"),"\n          ");
      desc = "\n               !!!!!!!!!    HILFE GESUCHT   !!!!!!!!!\n"+
	     desc+ "\n\n";
      zettel = clone_object("/obj/schatz");
      zettel->set_name("zettel");
      zettel->set_gender("maennlich");
      zettel->set_id(({"zettel","quest_zettel"}));
      zettel->set_adjektiv(colours[random(sizeof(colours))]);
      zettel->set_material("papier");
      zettel->set_long(desc);
      zettel->set_read(desc);
      zettel->set_value(0);
      zettel->move(this_object());
   }
}

void create()
{
   set_own_light(1);
   add_type("kunstlicht", 1);
   add_type("kaempfen_verboten", 1);
   add_type("landeplatz","treppe");
   set_short("Das Rathausfoyer");
   set_long(
      "Du stehst im Foyer des Rathauses von Tadmor. Hier im Rathaus kannst "
      "Du Dir Informationen über Gilden sowie Rätsel und Spiele "
      "beschaffen, dem schwarzen Brett Informationen entnehmen und auf "
      "diesem auch Deine Meinungen kund tun. Auch findest Du hier die "
      "Räumlichkeiten des Trägerkreises sowie die des Spielerrates. "
      "Schließlich kannst Du in der Halle der Reinkarnation den Schritt "
      "vollziehen, der Dich in die Schar der Engel erhebt.\n"
      "Zwei breite Gänge führen nach Westen und Osten, nördlich liegt "
      "hinter einem Torbogen eine große Halle. Eine breite Treppe führt "
      "nach oben und unten. Der Ausgang des Rathauses liegt südlich von Dir.\n"
      "Direkt vor Dir steht ein großer Wegweiser.");


   add_v_item(([
      "name": "treppe",
      "gender": "weiblich",
      "id": ({"treppe", "steintreppe"}),
      "long": "Eine massive Steintreppe führt entlang einer der Wände "
              "hinauf in den ersten Stock des Gebäudes "
              "und hinunter in den Keller."]));

   add_v_item (([
      "name":"wegweiser",
      "gender":"maennlich",
      "id":({"wegweiser","schilder","schildern"}),
      "feel":"Der Wegweiser fühlt sich sehr rauh an, wie festes, "
             "stabiles Holz.",
      "smell":"Der Wegweiser riecht äußerst angenehm nach Holz.",
      "noise":"Du hörst nichts. Nichtmal das Knabbern von "
              "Holzwürmern. Oder doch?",
      "long":"Ein großer, stabiler Wegweiser steht hier direkt "
             "vor Deiner Nase und zeigt nach Norden, Süden, Osten, Westen "
             "und auch nach oben. Du kannst ja mal lesen, was auf den "
             "einzelnen Schildern steht.",
      "read":"Auf dem Schild des Wegweisers, das nach Norden zeigt, steht "
             "\"Halle der Reinkarnation\", auf dem Schild, dessen Spitze "
             "nach Westen zeigt, steht \"Gildeninformationsraeume\". "
             "Auf dem Schild, welches in den Gang nach Osten zeigt, steht "
             "\"Spielerrat\", auf dem Schild nach Süden steht \"Magyra\" "
	     "und auf dem Schild, das die Treppe hinauf "
	     "zeigt, steht schließlich \"Trägerkreis UNItopia e.V.\" "
	     "geschrieben.",
      "take":"Dann würden sich bestimmt viele Leute hier gnadenlos "
             "verlaufen, und das willst Du doch nicht? Oder?"
      ]));

   add_v_item(([
    "name":	"torbogen",
    "gender":	"maennlich",
    "id":	({"tor","bogen","torbogen"}),
    "long":	"Durch den Torbogen gelangt man in die Halle der "
		"Reinkarnation. Er besteht aus weißem Marmor und "
		"lässt erahnen, wie wichtig diese Halle sein muss. "
		"Ein Tor oder eine Tür suchst du allerdings vergebens, "
		"es existiert nur der Bogen.",
    "feel":	"Der Torbogen sieht nicht nur so aus, als wäre er aus "
		"Marmor, er fühlt sich auch so an! Er ist schön glatt "
		"und ein wenig kühl.",
    "noise":	"Ab und zu hörst du von Norden her, wie sich Leo leise "
	        "mit jemandem unterhält. Vielleicht sind ja sogar Götter "
		"anwesend.",
    "smell":	"Der Marmor ist völlig geruchsneutral.",
    ]));

   set_exits(({ "treppe", "reinkarnation", "gilden1", "spielerrat/gang1",
#ifdef UNItopia
                "traegerkreis/room/verein",
#endif
                "keller/keller1"
   }),
	     ({ "süden", "norden", "westen", "osten",
#ifdef UNItopia
	        "hoch",
#endif
                "runter"
	     }));
   reset();
}
