// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /room/kirche/verdombde.c
// Description: Im Glockenturm
// Author:

inherit "/i/room";

void create()
{
   set_own_light(1);
   add_type("kunstlicht", 1);
   add_type("landeplatz", "balkon");
   set_short("Im südlichen Glockenturm");
   set_long("Der obere Teil des Südturms wird komplett von der größten "
	    "Glocke aller Zeiten eingenommen. Riesige Balken ächzen unter "
	    "der unglaublichen Last. Diese Glocke muss sehr schwer sein und "
	    "einen unglaublich tiefen Klang haben. Das muss die Verdombde sein,"
	    " die Totenglocke der Tadmorer Basilika!\n"+
	    "Ein schmaler Durchgang führt hinaus auf ein kleinen Balkon. "
	    "Eine Stiege führt in den Turm hinab.");
   set_exits(({"treppe5", "balkon"}), ({"runter", "westen"}));
   add_v_item(([
      "name": "stiege",
      "gender": "weiblich",
      "id": ({ "stiege", "treppe" }),
      "long": "Die hölzerne Stiege ist schon etwas in die Jahre gekommen und "
              "würde einem Steingolem sicher keinen Halt bieten, aber Du als "
              "vergleichbares Fliegengewicht hast sicher keine Probleme die "
              "knarzigen Stufen runterzuklettern.",
      "noise": "Bei jedem Deiner Schritte knarzen die Stufen "
               "besorgniserregend.",
   ]));
   add_v_item(([
      "name": "stufen",
      "gender": "weiblich",
      "plural": 1,
      "id": ({ "stufen", "treppenstufen" }),
      "long": "Die morschen Holzbohlen hinterlassen keinen vertrauenerweckenden"
              " Eindruck bei Dir."]));
   add_v_item(([
      "name": "verdombde",
      "eigen": 1,
      "id":({"verdombde","glocke"}),
      "gender": "weiblich",
      "look_msg": "$Der() bewundert die gigantische Glocke",
      "long": "Eine Glocke so edel und schön in Form hast Du noch nie zuvor "
	      "gesehen. Völlig unklar ist Dir, wie ein Mensch dieses "
	      "Ungetüm aus Metall auch nur in Bewegung setzen kann. "
	      "Dass es aber manchmal doch geschieht, ist dir vom "
	      "Totenlaeuten her bekannt. Dies muss die bekannte 'Verdombde' "
	      "sein, die den Abschied eines liebgewonnenen Einwohners von "
	      "Tadmor aus dem Leben verkündet."]));
   add_v_item(([
      "name": "balkon",
      "gender": "maennlich",
      "id":({"balkon","sims"}),
      "long": "Ein ganz schmaler Sims mit Geländer schmiegt sich "
	      "westlich an den Turm an."]));
}

void init()
{
   add_action("laeute", "läute",-4);
}

int laeute(string str)
{
   if(here(str, "glocke"))
   {
      say(wrap(Der(this_player())+
	       " versucht vergeblich die Verdombde zu läuten."));
      write("Die Glocke ist viel zu schwer, als dass Du sie in Bewegung "
	    "versetzen könntest.\n");
      return 1;
   }
   notify_fail("Läute was?\n");
}
