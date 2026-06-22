// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/obj/fackel.c
// Description: Eine Fackel.
// Modified:    Wert  (7.8.94): long der Fackel beruecksichtigt deren
//                              Zustand / Verbranntheitsgrad
//              Sissi (3.7.97): Man kann die Fackel befuehlen; brennt sie,
//                              so verbrennt man sich die Pfoten.

#ifdef UNItopia
// in UNItopia mit dem Feuerkonzept aus P

inherit "/p/Item/i/fackel";

#else
// ansonsten eine Standard-Fackel

inherit "/i/object/leuchte";

#include <add_hp.h>
#include <deklin.h>
#include <message.h>
#include <shadow.h>

void create()
{
    ::create();
    set_destruct_at_out_of_fuel(1);
    set_fuel(2000);
    set_id(({"fackel"}));
    set_name("fackel");
    set_gender("weiblich");
    set_smell("Sie riecht harzig.\n");
    set_material( ({"holz"}) );
    set_weight(1);
    set_value(10,0);
    set_power(1);
}

string query_long(object who)
{
    int percent;

    percent=(query_fuel()*100)/query_max_fuel();

    if (query_is_lighted()) {
        switch(percent) {
	  case 99..100:
            return(wrap(Ein(this_object(),"völlig unverbraucht")+". "+Er()+
                        " wurde wohl "
                        "erst vor kurzem entzündet."));
          case 81..98:
            return(wrap(Ein(this_object(),"fröhlich vor sich hinbrennend")+
                        ". "+Er()+" wirkt noch sehr unverbraucht."));
          case 61..80:
            return(wrap(Ein(this_object(),"brennend")+", "+
                        wer(this_object(),ART_NUR_DER)+" noch nicht "
                        "einmal zur Hälfte niedergebrannt ist."));
          case 41..60:
            return(wrap(Ein(this_object())+" mit tanzender Flamme. "+Er()+
                        " ist etwa zur Hälfte niedergebrannt."));
          case 21..40:
            return(wrap(Ein(this_object(),"brennend")+". "+Er()+
                        " ist schon mehr als zur Hälfte niedergebrannt."));
          case 11..20:
            return(wrap(Ein(this_object())+" mit leicht flackernder Flamme. "
                        "Sehr lange wird "+er()+" wohl nicht mehr brennen."));
          case 6..10:
            return(wrap(Ein(this_object())+". "+
                        Sein((["name": "flamme","gender": "weiblich"]),0,
                        this_object())+
                        " flackert, da "+er()+" schon "
                        "nahezu abgebrannt ist."));
          default:
            return(wrap(Ein(this_object(),"stark flackernd")+", "
                    +wer(this_object(),ART_NUR_DER)+
                    " nicht mehr lange brennen wird."));
	}
    }
    else {
        switch(percent) {
	  case 100:
            return(wrap(Ein(this_object(),"völlig unverbraucht")+". "+
                        Er()+" ist aus."));
          case 81..99:
            return(wrap(Ein(this_object())+", "+wer(this_object(),ART_NUR_DER)+
                        " noch einen sehr unverbrauchten "
                        "Eindruck macht, aber schon einmal gebrannt hat."));
          case 61..80:
            return(wrap(Ein(this_object(),"gelöscht")+", "+
                        wer(this_object(),ART_NUR_DER)+
                        " noch recht unverbraucht wirkt."));
          case 41..60:
            return(wrap(Ein(this_object())+", "+wer(this_object(),ART_NUR_DER)+
                        " etwa bis zur Hälfte niedergebrannt "
                        "ist. "+Er()+" ist aus."));
          case 20..40:
            return(wrap(Ein(this_object())+", "+wer(this_object(),ART_NUR_DER)+
                        " schon mehr als zur Hälfte "
                        "niedergebrannt ist. "+Er()+" ist aus."));
          default:
            return(wrap(Ein(this_object(),"fast völlig abgebrannt")+". "+
                    Er()+" ist aus."));
        }
    }
}

void player_message(int percent, object player) {
	string message;

    switch(percent) {
	case 50: message = Dein(0,"")+" ist schon zur Hälfte abgebrannt.";
		 break;
	case 25: message = "Von "+deinem(0,"")+" ist nicht mehr viel übrig.";
		 break;
	case 15: message = Dein(0,"")+" fängt an zu flackern.";
		 break;
	case 12:
	case  8: message = Dein(0,"")+" flackert.";
		 break;
	case  5:
	case  3: message = Dein(0,"")+" flackert stärker.";
		 break;
	case 1:  message = Dein(0,"")+" bäumt sich das letzte Mal auf.";
	}
    if (message)
	this_object()->send_message_to(player,MT_LOOK,MA_UNKNOWN,wrap(message));
}

void other_message(int percent, object player) {
	string mess_str;

    switch (percent) {
	case 25: if ((!player->query_invis()) && 
	         ((!query_once_interactive(player)) || interactive(player)))
		    mess_str = Der(player)+" wirft einen besorgten Blick auf "+seinen()+".";
		 break;
	case 15: mess_str = Ihr(0,"")+" fängt an zu flackern.";
		 break;
	case 12:
	case  8: mess_str = Ihr(0,"")+" flackert.";
		 break;
	case  5:
	case  3: mess_str = Ihr(0,"")+" flackert stärker.";
		 break;
	case  1: mess_str = Ihr(0,"")+" bäumt sich das letzte Mal auf.";
	}
    if (mess_str)
	this_object()->send_message(MT_LOOK,MA_UNKNOWN,wrap(mess_str),0,player);
}

void room_message(int percent) {
	string message;

    switch(percent) {
	case 15: message = Wer(0,ART_AAA,"")+" fängt an zu flackern.";
		 break;
	case 12:
	case  8: message = Wer(0,ART_AAA,"")+" flackert.";
		 break;
	case  5:
	case  3: message = Wer(0,ART_AAA,"")+" flackert stärker.";
		 break;
	case 1:  message = Wer(0,ART_AAA,"")+" bäumt sich das letzte Mal auf.";
	}
    if (message)
	this_object()->send_message(MT_LOOK,MA_UNKNOWN,wrap(message));
}

string fmsg;

string query_feel ()
{
    if (query_is_lighted())
    {
	int aps;
	string me;
	object wer = previous_object();
	object *obs;
	
        if (!playerp (wer) || this_player() != wer)
    	    return wrap ("Fühlt sich äußerst rau und - Autsch! Du hast Deine "
        	"Finger verbrannt! Das war ganz schön heiß und tut tierisch "
        	"weh.");
	
	switch(random(100))
	{
	    // Fackel fallen lassen:
	    case 0..19:
		if(environment()==wer)
		{
		    aps = -5-random(10);
		    me = Er()+" fühlt sich äußerst rau und - Autsch! "
			 "Das war verdammt heiß, da hast du glatt "+deinen()+" fallen lassen.";
		    fmsg = Der(wer)+" befühlt "+seinen(this_object(),0,wer)
			 + ". Nachdem "+er(wer)+" bemerkt, dass "+er()+" doch "
		         "etwas heiß ist, lässt "+er(wer)+" "+ihn()+" schnell fallen.";
		    move(environment(wer));
		    break;
		}
		// Fall through
	    case 20..79:
		aps = -8-random(7);
		if(environment()==wer)
		    obs = filter(all_inventory(wer)-({this_object()}),
			(: "/obj/streichhoelzer"->brennbar($1) &&
			    !$1->query_fire_shadow() &&
			    !$1->query_is_lighted()
			:));
		if(sizeof(obs))
		{
		    object ob = obs[random(sizeof(obs))];
		    string denob = den(ob);
		    string seinenob = seinen(ob);
		    
		    if(clone_object("/obj/feuer_shadow")->init_shadow(ob)==SHADOWING_OK)
		    {
			me = "Fühlt sich äußerst rau und - Autsch! "
		             "Du hast Deine Finger verbrannt! Das war ganz schön heiß "
			     "und tut tierisch weh. Mist, jetzt hast Du auch noch "
			   + denob+" in Brand gesteckt.";
			fmsg = Der(wer)+" befühlt "+seinen(this_object(),0,wer)
		    	   + ". Dabei verbrennt sich "+der(wer)+" die Finger und "
			     "zündet gleichzeitig "+seinenob+" an!";
			break;
		    }
		}
		// Fall through.
	    default:
    		me = "Fühlt sich äußerst rau und - Autsch! Du hast Deine "
        	     "Finger verbrannt! Das war ganz schön heiß und tut tierisch "
        	     "weh.";
		fmsg = Der(wer)+" befühlt "+seinen(this_object(),0,wer)
		     + ". Dabei verbrennt sich "+der(wer)+" die Finger!";
		aps = -5-random(20);
		break;
	}
	call_out ("verbrenne_finger",0,previous_object(), aps);
	return wrap(me);
    }
    return wrap ("Fühlt sich sehr rau, hart und stabil an.");
}

string query_feel_msg ()
{
    if(find_call_out("verbrenne_finger") && fmsg)
	return fmsg;

    if (query_is_lighted() && playerp (previous_object())
        && this_player() == previous_object())
        return Der(previous_object())+" befühlt "
            +seinen(this_object(),0,previous_object())+". Dabei verbrennt "
            "sich "+der(previous_object())+" die Finger!";
    return 0;
}

void verbrenne_finger (object opfer, int aps)
{
    if (opfer && !opfer->query_ghost())
        opfer->add_hp(aps, ([
	    AH_ERF_TOD: "Du hast Dir Deine Finger verbrannt.",
	    AH_DAMAGE_TYPE: ({ "feuer" }),
	]));
}

#endif // UNItopia
