// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/room/rathaus/spielerrat/raum.c
// Description: Rathaus, Spielerratsraeume

#ifdef UNItopia
#define SRKONSOLE	"/z/Raetsel/Amsel-Ei/laber/obj/srkonsole"
#endif

inherit "%room";

#include <apps.h>
#include <config.h>
#include <description.h>
#include <invis.h>
#include <level.h>
#include <move.h>

void reset()
{
    int nr;
    object ob;
    
    if(!present("brett"))
    {
	object brett = clone_object("/obj/brett");
	brett->set_bretter(({"Spieler","Ratsmitglieder","Gilden","Projekte","Städte"}));
	brett->set_invis(V_NOLIST);
	brett->move(this_object());
    }
    if(!present("urne"))
    {
	object urne = touch("/room/wahlen/srurne");
	urne->set_invis(V_NOLIST);
	urne->move(this_object());
	
    }
#ifdef SRKONSOLE
    if(!present_clone(SRKONSOLE,this_object()))
    {
	object console = clone_object(SRKONSOLE);
	console->set_invis(V_NOLIST);
	console->move(this_object());
    }
#endif
#ifdef UNItopia
    if(!present("kamin"))
    {
	object kamin = clone_object("/p/Item/Install/obj/kamin");
	kamin->set_invis(V_NOLIST);
	kamin->move(this_object());
    }
    
    nr = sizeof(SPIELERRAT->query_spielerrat());
    for(int i=0;i<nr;i++)
	if(!present("Hänge "+i+" Matte"))
	{
	    object matte = clone_object("/p/Item/Moebel/Sessel/obj/haengematte");
	    matte->set_invis(V_NOLIST);
	    matte->set_sitting_invis(V_VIS);
	    matte->add_id("Hänge "+i+" Matte");
	    matte->set_adjectiv("");
	    matte->set_heal_data(0,0,0);
	    matte->set_enable_cleanup();
	    matte->set_no_move(1);
	    matte->move(this_object());
	}
    for(int i=nr; (ob=present("Hänge "+i+" Matte")); i++)
	ob->remove();
#endif
}

string take_robe(mapping vitem)
{
    if (!spielerratp(this_player()))
        return "Du darfst keine Robe nehmen.";
    if (deep_present("spiellerat # robe",this_player()))
        return "Du hast schon eine Robe.";
    object ob = clone_object("/room/rathaus/spielerrat/obj/robe");
    ob->move(this_player(),([MOVE_FLAGS:MOVE_ERR_REMOVE]));
    if (ob)
        return "Du nimmst eine Robe.";
    else 
        return "Du kannst keine Robe nehmen.";
}

void create()
{
    "*"::create();
    set_short("Arbeitszimmer des Spielerrates");
    set_long(({
      "Du befindest Dich in einem gemütlichen, überaus rustikal eingerichteten "
      "Zimmer des Rathauses. Mehrere mächtige Holzbalken verzieren die "
      "Decke, an welchen ", 
      (: to_string(sizeof(SPIELERRAT->query_spielerrat())) :),
      " Hängematten befestigt sind, die zum erholsamen Arbeiten "
      "einladen. An der östlichen Wand brennt ein Kamin vor sich hin, "
      "daneben sind etliche Bretter angebracht. Auf dem Kamin steht "
      "eine Urne für Abstimmungen bereit. An der Wand hängen ein paar "
      "Roben für Spielerräte. "
#ifdef SRKONSOLE
      "In der südlichen Wand ist eine Konsole in die Holzvertäfelung "
      "eingearbeitet. "
#endif
      "Aber all das wirkt sofort unscheinbar neben einer imposanten "
      "kristallenen Tafel, welche die westliche Wand ausfüllt. "
      "Nach Norden geht es zurück zum Gang."}));
     
    add_v_item(([
	"name":		"kristalltafel",
	"gender":	"weiblich",
	"id":		({ "kristalltafel", "tafel","lords","lord"}),
	"long":		({
	    T_OR(T_WIZ, T_SPIELERRAT),
	    ({
		"Du siehst eine beeindruckende Kristalltafel. Zahlreiche "
		"eingelassene Diamanten bilden folgende Information:\n\n"
		"--------------- *** DIE LORDS "+upper_case(MUD_NAME)+
		"S *** "+("-"*(39-sizeof(MUD_NAME)))+"\n"
		"Kontinente:\n",
		(:
		    implode(map(DOMAIN_INFOS->query_domains(),
		    (:
			sprintf("    %-15s %-55.5#s\n",
			    $1+":", implode(map(sort_array(
			    DOMAIN_INFOS->query_domain_lords($1),
			    #'>),#'capitalize),"\n"))
		    :)),"")
		:),
		"---------------------------------------------------------------------------\n",
		(:
		    implode(map(sort_array(FILED->query_types()-({"schiffe"}),#'>),
		    (:
			mixed * lords = FILED->query_auth($1);
			
			return sizeof(lords) &&
			    sprintf("%-19s %-55.5#s\n",
				capitalize($1)+":",
				implode(map(sort_array(
				lords,
				#'>),#'capitalize),"\n"));
		    :))-({0}),"")
		:),
		"---------------------------------------------------------------------------\n"
		"\nDie Bereichsnamen sind zugleich E-Mail-Adressen "
		"(z.B. Gilden@Unitopia.de). ",
		T_SPIELERRAT,
		    "Die angezeigten Informationen sind vertraulich "
		    "zu behandeln."
	    }),
	    T_ELSE,
		"Die Kristalltafel enthält zahlreiche Diamanten."
	})]));
    add_v_item( ([
        "name" : "roben",
        "gender" : "weiblich",
        "id" : ({ "roben","robe"}),
        "long" : "Ein Satz von Roben nur für Spielerräte.",
        "look_msg": "",
        "take" : #'take_robe,
        "take_msg": "",
    ]) );
    set_exit("gang1","norden");
    reset();
}

