// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /room/kirche/treppe4.c
// Description: Die Treppe zum Kirchturm
// Author:

inherit "/i/room";

object door;

void reset()
{
   object stroh;

   if(!present("stroh"))
   {
      stroh = clone_object("/obj/schatz");
      stroh->set_name("strohhalme");
      stroh->set_gender("maennlich");
      stroh->set_plural(1);
      stroh->set_value(-10);
      stroh->set_weight(1);
      stroh->set_id(({"stroh","strohhalme","halme"}));
      stroh->set_long("Ein paar Strohhalme. Aus vielen solchen machen sich "
		      "arme Leute ihre Schlafstätte.");
      stroh->move(this_object());
   }
   if(!door)
   {
      door = clone_object("/obj/tuer");
      door->set_pass_cmd("norden");
      door->set_keys(({"sakrote#key"}));
      door->set_door_exit("/room/kirche/gewoelbe");
      door->set_long("Die Tür führt zurück in das Gewölbe über dem "
         "Hauptschiff der Basilika. Neben der Tür an der Wand ist ein Hebel "
         "angebracht, mit dem man sie im Notfall öffnen kann.");
      door->move(this_object());
   }
   door->lock_door();
}

void create()
{
   set_own_light(1);
   add_type("kunstlicht", 1);
   add_type("landeplatz","balkon");
   set_short("Sakrotes Lager");
   set_long("Im dunklen Mittelteil des Südturms der Basilika. Eine "
            "schmale Stiege schlängelt sich in engen Kreisen innen an "
	    "der Außenwand empor zur Heimstatt der 'Verdombden'. "
	    "Hier unten ist ein schlichtes Lager errichtet. Du vermutest, "
	    "dass dies Sakrotes Zuhause darstellt.");
   set_exits(({"treppe5"}),({"hoch"}));
   set_exit_msg("hoch", "$Der(OBJ_TP) klettert die Stiege hinauf",
                        "$Ein(OBJ_TP) kommt von unten hoch geklettert");
   add_v_item(([
      "name": "lager",
      "id" : ({ "lager", "quartier" }),
      "gender": "saechlich",
      "look_msg": "$Der() betrachtet mitleidig Sakrotes Quartier",
      "long": "Sakrotes Lager ist schmutzig und zugig. Du bist froh, dass Du "
	      "hier nicht sein armseeliges Dasein fristen musst. Was der arme "
	      "Kerl schon alles durchgemacht haben muss?"]));
   add_v_item(([
      "name": "stiege",
      "gender": "weiblich",
      "id": ({ "stiege", "treppe" }),
      "long": "Die hölzerne Stiege ist schon etwas in die Jahre gekommen und "
	      "würde einem Steingolem sicher keinen Halt bieten, aber Du als "
	      "vergleichbares Fliegengewicht hast sicher keine Probleme die "
	      "knarzigen Stufen hinaufzuklettern."]));
   add_v_item(([
      "name": "stufen",
      "gender": "weiblich",
      "plural": 1,
      "id": ({ "stufen", "treppenstufen" }),
      "long": "Die morschen Holzbohlen hinterlassen keinen vertrauenerweckenden"
	      " Eindruck bei Dir."]));
   reset();
}

void init()
{
   add_action("ziehe_com", "ziehe", -4);
}

int ziehe_com(string str)
{
   if(!str || str == "")
   {
      notify_fail("Ziehe was?\n");
      return 0;
   }
   if(lower_case(str)!="hebel")
   {
      notify_fail(capitalize(str)+" ist nicht zu sehen.\n");
      return 0;
   }
   door->unlock_door();
   write("Du hörst ein leises Klicken in der Wand in der Nähe der Türe.\n");
   say(Der(this_player())+" zieht an einem Hebel.\n");
   return 1;
}
