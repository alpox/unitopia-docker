// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/room/rathaus/forum.c
// Description:	Zentraler Verteiler fuer die 'Serviceraeume'
// Author:	Garthan	(18.08.94)

inherit "/i/room";

#include <config.h>
#include <level.h>
#include <message.h>
#include <move.h>
#include <player.h>
#include <portal.h>
#ifdef UNItopia
#include "/d/Vaniorh/sys/vaniorh_exports.h"
#include "/d/Midgard/sys/export.h"
#define P_WOFINDEICH "/p/Doc/Lehre/Einstieg/obj/wofindeich"
#endif

void reset()
{
   object regal;
   object brett;

   if(!(regal = present("regal")))
   {
      regal = clone_object("/obj/kiste");
      regal->set_name("bücherregal");
      regal->set_id(({"regal", "bücherregal"}));
      regal->set_gender("saechlich");
      regal->set_no_move(1);
      regal->set_long("Ein gut gefülltes Bücherregal.");
      regal->set_content_message("\tDarin stehen:");
      regal->set_no_door(1);
      regal->move(this_object());
   }
   if(!present("who-is-who-buch",regal))
      clone_object(abs_path("obj/whois"))->move(regal,
            ([MOVE_FLAGS:MOVE_ERR_REMOVE]));
   if(!present("ausbildungsschriftrolle",regal))
      clone_object(abs_path("obj/ausbildung"))->move(regal,
            ([MOVE_FLAGS:MOVE_ERR_REMOVE]));
   if(!present("exodus",regal))
      clone_object(abs_path("obj/exodus"))->move(regal,
            ([MOVE_FLAGS:MOVE_ERR_REMOVE]));
   if(!present("genesis",regal))
      clone_object(abs_path("obj/genesis"))->move(regal,
            ([MOVE_FLAGS:MOVE_ERR_REMOVE]));
   if(!present("götterbuch",regal))
      clone_object(abs_path("obj/goetterbuch"))->move(regal,
            ([MOVE_FLAGS:MOVE_ERR_REMOVE]));
   if(!present("wiedereinstieg",regal))
      clone_object(abs_path("obj/wiedereinstieg"))->move(regal,
            ([MOVE_FLAGS:MOVE_ERR_REMOVE]));
   if(!present("götterkodex",regal))
      clone_object(abs_path("obj/kodex"))->move(regal,
            ([MOVE_FLAGS:MOVE_ERR_REMOVE]));
#ifdef UNItopia
   if(!present("zeitung",regal))
      clone_object(abs_path(ZEITUNG_ZUM_RUMTRAGEN))->move(regal,
            ([MOVE_FLAGS:MOVE_ERR_REMOVE]));
   if(!present("buch # erbauer",regal))
      clone_object(abs_path(ERBAUER))->move(regal,
            ([MOVE_FLAGS:MOVE_ERR_REMOVE]));
   if(!present("wo # finde # ich", regal))
      clone_object(P_WOFINDEICH)->move(regal,
            ([MOVE_FLAGS:MOVE_ERR_REMOVE]));
#endif

/*
   if(!present("top-20"))
      clone_object(abs_path("obj/wiz_list"))->move(this_object());
*/
   if(!present("gurne"))
      touch("/room/wahlen/gurne")->move(this_object());
   if(!present("brett"))
   {
      brett = clone_object("/obj/brett");
      brett->set_brett_name("Goetter");
      brett->move(this_object());
   }
   
   if (!present("nessaja"))
   {
       object npc = touch("/room/rathaus/div/wegweiser");
       npc->move(this_object());
   }

#ifdef UNItopia
   if(!present(ID_PORTAL_ALL))
   {
      object portal=clone_object("/obj/portal");
#ifdef TestMUD
      portal->set_portal_name("UNItopia");
      portal->set_long("Ein rundes Portal, welches nach UNItopia führt.");
#else
      portal->set_portal_name("Orbit");
      portal->set_long("Ein rundes Portal, welches nach Orbit führt.");
#endif
      portal->move(this_object());
   }
#endif
}

void create()
{
   set_own_light(1);
   add_type("kunstlicht",1);
   add_type("teleport_rein_verboten", 1);
   set_short("Das Forum");
   set_long(
   "Du stehst im kreisrunden Forum der Götter, zu allen Seiten erstrecken "
   "sich Ausgänge, die in die diversen Verwaltungsräume des Rathauses "
   "führen. Dies ist das Herz "+MUD_NAME+"s. Sämtliche Informationen "
   "werden hier erfasst und verarbeitet. Geschäftige Lakaien wandeln "
   "von einem Büro zum nächsten und sorgen für rege Betriebsamkeit "
   "im Forum. Die gold-blau getünchte Kuppel mit ihren seitlichen "
   "Oberlichtern gibt dem großen Raum eine angenehme Atmosphäre. "
   "Eine Treppe führt nach unten zu den Sterblichen.");
   add_type("kaempfen_verboten",1);
   set_exits( ({
	   "reinkarnation", "nische", "../bsp/bsp_eingang", "domain", "register",
	   "reserv", "filed", "raetsel", "gilden", "schiffahrt", "news",
	   "teleport", "kontor", "projekt","npcfinger", "reisebuero",
       "../bank/bankenaufsicht",
#ifdef UNItopia
           "/p/Doc/room/halle", "/d/Levitanis/public/Zentrum/center"
#endif
	   }), ({
	   "runter", "nische",  "kurs", "grundbuch", "register", "bann",
	   "gouverneur", "rätsel", "gilden", "schiff", "archiv", "fziele",
	   "kontor", "projekt","npcfinger", "reisebüro", "bank",
#ifdef UNItopia
	   "halle", "levitanis"
#endif	   
	   }) );
   set_exit_msg("runter",
      "$Der(OBJ_TP) geht die Treppe hinunter", 
      "$Ein(OBJ_TP) kommt die Treppe herunter");
   set_room_domain("Pantheon");
   reset();
}

<int|string> let_not_out(mapping mv_infos)
{
    object who  = mv_infos[MOVE_OBJECT];
    string dir = mv_infos[MOVE_DIRECTION];
   if(stringp(dir) && !wizp(who) && !testplayerp(who) &&
      lower_case(dir) != "runter")
   {
     return "Ein Lakai bemerkt Dein unbefugtes Hiersein und hält Dich auf.\n";
   }
   return ::let_not_out(mv_infos);
}

void notify_login(object who, int flag, int mode)
{
    if (!playerp(who)) return;
    switch (mode)
    {
        case NL_PORTAL_ENTER:
            send_message_to(who, MT_NOTIFY, MA_MOVE_IN,     
                            "Willkommen in "+MUD_NAME+".");
            return;
        case NL_PORTAL_REENTER:
            send_message_to(who, MT_NOTIFY, MA_MOVE_IN,     
                            "Willkommen zurück in "+MUD_NAME+".");
            return;
    }
}