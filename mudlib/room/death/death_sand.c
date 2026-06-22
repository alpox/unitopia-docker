// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/room/death/death_sand.c
// Description: Das Sandmaennchen, das den TOD vertritt
// Author:      Wluut, Mirza

inherit "/i/monster/monster";

string take_it(string str)
{
   return wrap(
      "Du greifst beherzt nach "+str+" und versuchst, sie an Dich zu ziehen. "
      "Das Sandmännchen runzelt missbilligend die Stirn, klopft Dir mit der "
      "Hand auf die Finger und sagt:\n"+
      (this_player()->query_gender()=="weiblich" ?"DUMME STERBLICHE! " 
      :this_player()->query_gender()=="saechlich"?"DUMMES STERBLICHES! "
                                                 :"DUMMER STERBLICHER! ")+
      "DU HAST DIR GERADE "+(random(90)+10)+" ZUSÄTZLICHE JAHRE IM FEGEFEUER "
      "EINGEHANDELT!");
}

string take_robe() { return take_it("der Robe"); }
string take_sense() { return take_it("der Sense"); }

void create()
{
   monster::create();
   initialize("sandmännchen",100);
   give_hp(999);
   give_sp(999);
   give_armour_level(50);
   give_weapon_level(50);
   set_personal(1);
   set_gender("saechlich");
   set_name("sandmännchen");
   set_id(({"sandmännchen", "sandmann", "der#tod"}));
   set_long("Das Sandmännchen. Es begleitet kleine Kinder in den Schlaf. "+
           "Außerdem ist es der Geliebte des Rotkäppchens. "+
           "Und außerdem ist es manchmal ziemlich ganz schön doof.");

   add_v_item(([
      "name": "sense",
      "gender": "weiblich",
      "take": #'take_sense,
      "take_msg": "$Der() versucht sich die Sense zu schnappen",
      "long":
	 "Eine extrem scharfe Sense. Sie ist so scharf, das selbst der Wind "
	 "versucht, nicht über die scharfe Schneide zu wehen, um nicht von "
	 "der grausig aussehenden Sense entzweigeschnitten zu werden. "
	 "Sie veranstaltet seltsame Dinge mit dem Licht, wenn unglückliche "
	 "Photonen auf die Schneide treffen und dabei in ihre Bestandteile "
	 "zerlegt werden." ]));

   add_v_item(([
      "name": "robe", 
      "gender": "weiblich",
      "take": #'take_robe,
      "take_msg": "$Der() versucht sich die Robe zu schnappen",
      "long":
	 "Eine schwarze Robe mit zahllosen Taschen. Sie würde Dir wohl nicht "
	 "sehr gut passen. Sie scheint für einen sehr mageren Kunden genäht "
	 "worden zu sein. SEHR mager sogar..." ]));
}
