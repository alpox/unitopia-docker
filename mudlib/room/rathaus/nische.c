// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/room/rathaus/nische_ost.c
// Description:

inherit "/i/room";

#include <monster.h>

void reset()
{
   object tulomat;

   if(!present("recycl-o-mat"))
       touch(abs_path("div/recycl-o-mat"))->move(this_object());

   if(!present("tul-o-mat"))
   {
      tulomat = clone_object("/obj/dispenser");
      tulomat->set_disp_text(
	 "Der Tul-O-Mat.\n"+
	 "Ein ziemlich unförmiges Gebilde, das in der Lage ist, die im\n"+
	 "folgenden genannten Tuls (auch bekannt als Tools) in dein\n"+
	 "Gepäck zu spucken und dort zu verankern.\n");
      tulomat->set_disp_items(
      ({
	 "Einführung",
	 "Enzyclopaedie",
	 "/obj/newsreader"->query_short(),
	 "/obj/mailreader"->query_short(),
	 "Zauberstab",
#ifdef Orbit
     "OrbitSyncer",
#endif
	 0,
	 "Gewitterwolke",
	 "PAL",
	 "Wicht",
	 "Petze",
	 "Uhr",
      }),
      ({
	 "/room/rathaus/obj/genesis",
	 "enzyclopedia",
	 "newsreader",
	 "mailreader",
	 "zauberstab",
#ifdef Orbit
     "/room/rathaus/obj/orbit_sync",
#endif
	 0,
	 "/p/Item/Toy/WizOnly/obj/wolke",
	 "/p/Tool/obj/pal",
	 "/p/Tool/obj/wicht",
	 "/p/Tool/obj/petze",
	 "/p/Item/obj/uhr",
      }));
      tulomat->set_disp_prices(({0,0,0,0,0,
#ifdef Orbit
        0,
#endif
        0,0,0,0,0,0}));
      tulomat->set_disp_max(({-1,-1,-1,-1,-1,
#ifdef Orbit
        -1,
#endif
        0,-1,-1,-1,-1,-1,-1}));
      tulomat->set_disp_buttons(({"1","2","3","4","5",
#ifdef Orbit
        "6",
#endif
        0,"A","B","C","D","E","F"}));
      tulomat->set_disp_chargeable(0);
      tulomat->set_name("Tul-O-Mat");
      tulomat->set_gender("maennlich");
      tulomat->set_id( ({"automat","tulomat","tul-o-mat" }) );
      tulomat->move(this_object());
   }
}

void create() {
    add_type("kunstlicht",1);
    add_type("teleport_rein_verboten", 1);
    set_own_light(1);
    set_short("Fensternische");
    set_long(
"Du stehst in einer Fensternische. Von hier aus hast du einen schönen Blick\n"+
"über die Abteistraße hinweg auf das Kloster.\n");
    set_exits( ({ "forum" }),
	       ({ "forum" }) );
    add_type("kaempfen_verboten",1);
    set_room_domain("Pantheon");
    reset();
}

object make_object(string str)
{
   object zst;
   if(str == "zauberstab")
   {
      zst = clone_object("/obj/schatz");
      zst->set_name("infostab");
      zst->set_gender("maennlich");
      call_out("instructo", 0, zst);
      return zst;
   }
   if(member(({"enzyclopedia", 
	       "newsreader", 
	       "mailreader" }), str) >= 0)
   {
      zst = clone_object("/obj/"+str);
      /*
      this_player()->do_command("erschaffe /obj/"+str);
      obj = first_inventory(this_player());
      zst = clone_object("/obj/schatz");
      zst->set_name(obj->query_name());
      zst->set_gender(obj->query_gender());
      zst->set_plural(obj->query_plural());
      call_out("destructa",0, zst);
      */
      return zst;
   }
}
void destructa(object obj)
{
   obj->remove();
}

void instructo(object zst)
{
   if(zst && this_player())
   {
      write(
	    "\nDein Infostab flüstert: "+
	    "Einen neuen Zauberstab bekommst du einfach dadurch,\n"+
	    "                         dass du 'zauberstab' eingibst!\n");
   }
   call_out("destructo",0,zst);
}

void destructo(object zst)
{
   if(zst)
   {
      if(this_player())
	 write("Dein Infostab zerfällt zu Staub!\n");
      zst->remove();
   }
}

int key_tools(string str, string verb, object monster, object player,
    int flags)
{
    monster->do_command("sage In der Nische gibt es verschiedene "
        "Werkzeuge/Tools.");
}

mixed *query_keyword_rules()
{
    return ({
"key_tools: werkzeug || werkzeuge || tool || tools", PARSE_SAY|PARSE_CONTINUE,
    });
}
