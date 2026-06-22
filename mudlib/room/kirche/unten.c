// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /room/kirche/unten.c
// Description: Im Kirchturm der Kathedrale von Tadmor
// Author:

inherit "/i/room";

object door;

string look_halter()
{
   if(present("fackel"))
      return wrap("Ein schwarzes, metallenes Behältnis, "
                  "in dem eine Fackel steckt.");
   else
      return wrap("Ein schwarzes, metallenes Behältnis, "
                  "in dem einst eine Fackel steckte.");
}

void reset()
{
   object fackel;
   if(!door)
   {
      door = clone_object("/obj/tuer");
      door->set_pass_cmd("süden");
      door->set_keys(({"kirchturm#key"}));
      door->set_door_exit("/room/church");
      door->set_long("Die Tür führt hinaus in die Basilika.\n"
         "Neben der Tür an der Wand ist ein Hebel, mit dem man diese im "
         "Notfall entriegeln kann.");
      door->move(this_object());
   }
   if(!present("fackel"))
   {
      fackel = clone_object("/obj/fackel");
      fackel->move(this_object());
      fackel->set_hidden_until_next_move();
   }
}

void create()
{
   set_own_light(1);
   add_type("kunstlicht", 1);
   add_type("kaempfen_verboten", 1);
   add_type("landeplatz", "balkon");
   set_short("Im Kirchturm");

   set_long(
      "Im Sockel des Nordturms.\n"+
      "Hier drin ist es ziemlich duster, da der Raum selbst keine "
      "Fenster hat. Nur von oben, über die hier endende Spindeltreppe "
      "fällt ein Lichtschimmer herab. Die Treppe führt hinauf in "
      "den Kirchturm. An der ziemlich dicken Außenmauer ist "
      "ein Fackelhalter befestigt. Eine Tür führt hinaus in "
      "den Westteil des gewaltigen Mittelschiffs der Kathedrale. ");
   add_v_item(([
      "name": "hebel",
      "gender": "maennlich",
      "long": "Ein hölzerner Hebel, direkt neben der Tür."]));
   add_v_item(([
      "name": "treppe",
      "id": ({"treppe", "spindeltreppe"}),
      "gender": "weiblich",
      "long": "Die steinerne Treppe schraubt sich in die Höhe und "
              "führt wohl in den Glockenturm hinauf."]));
   add_v_item(([
      "name": "fackelhalter",
      "gender": "maennlich",
      "id":({"fackelhalter", "halter" }),
      "long": #'look_halter]));
   add_v_item(([
      "name": "außenmauer",
      "gender": "weiblich",
      "id":({"mauer", "außenmauer"}),
      "long": "Die Außenmauer wurde mit Absicht so dick gebaut, um dem "
              "darauf lastenden Glockenturm ein solides Fundament zu geben."]));

   set_exits(({"/room/kirche/treppe1"}), ({"hoch"}));
   reset();
   set_exit_msg("hoch", "$Der(OBJ_TP) steigt die Treppe hinauf",
                        "$Ein(OBJ_TP) kommt von unten hoch gekraxelt");
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

void init()
{
   add_action("ziehe_com", "ziehe", -4);
}
