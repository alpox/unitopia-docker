// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/room/bsp/bsp_eingang.c
// Description:	Eingang zu den Beispielraeumen.
// Author:	

// Nice try, aber bitte geh erst mal nach Norden, dort
// faengt der Kurs erst richtig an.
// (Merke: 'zmore hier' ist wichtig!)

inherit "/i/room";

#include <config.h>
#include <message.h>
#include <room_types.h>

string aktive_admins() {
   return liste(
             map(
                filter(
                   map(
                      explode(read_file("/static/adm/ADMIN_FINGER"), 
                         "\n") - ({"",}),
                      (: explode($1, " ") - ({"",}) :)),
                (: sizeof($1)>1 && $1[0]=="*" :)),
             (: $1[1] :)) - ({"Sissi",}),
          " oder ");
}

string file_date() {
    return shorttimestr(file_time(program_name()),1,2);
}

void create()
{
   add_type(RT_KUNSTLICHT, 1);
   add_type(RT_TELEPORT_REIN_VERBOTEN, 1);
   set_own_light(1);
   set_long(
     wrap (
      "Du stehst in der Eingangshalle der unterschiedlichsten Kurse. "
      "Noch sinds erst drei, aber das ändert sich sicher bald in ein "
      "paar Jahren.")+
     wrap (
      "Südlich findest Du einen Einstiegskurs für Neugötter "
      "ohne LPC Kenntnisse, nördlich einen für etwas fortgeschrittenere "
      "Junggoetter. Und für ganz Fortgeschrittene gibt es den "
      "Lehrpfad für Töne im Westen.")+
     wrap (
      "In den Kursen kannst du alle Objekte (also Räume, Monster, "
      "Gegenstände, usw.) mit dem Zauberstabbefehl 'zmore <Obj>' "
      "begutachten.")+
     wrap (
      "Die Beispielräume wurden am "+file_date()+
      " zum letzen Mal aktualisiert, "
      "falls dieses Datum deutlich vom heutigen abweicht, solltest Du "+
      aktive_admins()+" mal auf den Füßen rumtreten..."));
   set_short("Einführungskurs - Eingangshalle");
   set_exits(({"bsp1", "/p/Doc/Lehre/Einstieg/room/raum01", "bsp_sound1",
               "../rathaus/forum"}),
             ({"norden", "süden", "westen", "forum"}));
   set_room_domain("Pantheon");
}

int filter_norden(object who)
{
   object ob;
   if(playerp(who) && !present("enzyclopedia",who))
   {
      ob = clone_object("/obj/enzyclopedia");
      ob.move(who);
      call_out("enzy_in_der_hand", 0, who);
   }
   return 0;
}

void enzy_in_der_hand(object who)
{
    if (playerp(who))
       who.send_message_to(this_player(), MT_NOTIFY|MT_LOOK, MA_MOVE,
      "\nPlötzlich hast Du eine Enzyklopedia "+MUD_NAME+" in der Hand.\n");
}
