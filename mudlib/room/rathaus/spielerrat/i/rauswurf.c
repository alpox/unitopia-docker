// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/room/rathaus/spielerrat/i/rauswurf.c
// Description: Rathaus, Spielerratsraeume, Rauswurfsinherit
// Author:      Goldie (14.10.2004)

inherit "%rauswurf_tuer";
inherit "/i/tools/security";

#include <apps.h>
#include <message.h>
#include <move.h>
#include <level.h>
#include <parse_com.h>
#include <notify_fail.h>

#define SECURE (check_security() && \
	(spielerratp(this_player()) || adminp(this_player())))

void init()
{   
    if(SPIELERRAT->is_spielerrat(this_player()) || adminp(this_player()))
    {
        add_action ("rausschmiss","verweise", -7);
        add_action ("rausschmiss","schmeiße", -7);
        add_action ("rausschmiss","prügele", -6);
        add_action ("rausschmiss","prügle");
        add_action ("rausschmiss","zeige", -4);
        add_action ("rausschmiss","schnipse", - 7);
    }
}

private void real_move(object raus, string msg)
{
// Direkt den force_move nehmen...
//    if(raus->move(query_tuer_richtung(),MOVE_NORMAL,"$", msg)!=MOVE_OK)
    {
	string dir = query_tuer_richtung();
	object dest = touch(this_object()->query_one_exit(dir));
	
	raus->move(dest, ([MOVE_FLAGS:MOVE_FORCE|MOVE_NORMAL,
        MOVE_MSG_ENTER:msg]));
    }
}

private int rausfunktion(object raus, string msg_wen_mit_tuer, 
        string msg_draussen_mit_tuer, string msg_alle_mit_tuer,
        string msg_tp_mit_tuer,
        string msg_wen_ohne_tuer, string msg_draussen_ohne_tuer, 
        string msg_alle_ohne_tuer, string msg_tp_ohne_tuer)
{
    object tuer = present("SRTuer");
    
    if(!living(raus))
	return notify_fail(wrap(Der(raus)+" ist kein Lebewesen."),
		FAIL_WRONG_ARG);
    
    if(raus==this_player())
	return notify_fail("Nö.\n");
	
    sys_log("SR_RAUSWURF", sprintf("%s | %10-s rauswurf %#Q\n",
	shorttimestr(time()),
	this_player()->query_real_name(), 
	raus->query_real_name() || 
	    ({raus, raus->query_name(), raus->query_short()})));
    
    if(tuer->query_door_is_open()==0)
    {
        this_player()->send_message_to(raus, MT_NOTIFY, MA_MOVE_OUT,
            wrap(msg_wen_mit_tuer));
    	tuer->open_door(0,1);
	real_move(raus, msg_draussen_mit_tuer);
        tuer->close_door(0,1);
        this_player()->send_message(MT_NOTIFY, MA_MOVE_OUT,
            wrap(msg_alle_mit_tuer),wrap(msg_tp_mit_tuer),this_player());
    }
    else
    {
        this_player()->send_message_to(raus, MT_NOTIFY, MA_MOVE_OUT,
            wrap(msg_wen_ohne_tuer));
        real_move(raus, msg_draussen_ohne_tuer);
        this_player()->send_message(MT_NOTIFY, MA_MOVE_OUT,
            wrap(msg_alle_ohne_tuer),wrap(msg_tp_ohne_tuer),this_player());
    }
    
    return 1;
}

int rausschmiss(string str)
{
    mixed parsed;
    
    if(!SECURE)
	return 0;

    parsed = parse_com(str, this_object(), 0, PARSE_NO_V_ITEMS);
    
    if(query_verb()=="verweis" || query_verb()=="verweise")
    {
	object raus;

        if(parse_com_error(parsed,"Wen willst du aus dem Raum verweisen?\n",1))
	    return 0;

	raus = parsed[PARSE_OBS][0];

        return rausfunktion(raus,
	    "\n" + Der(this_player()) + " öffnet die Tür, verweist dich "
	    "des Raumes und schließt sie wieder hinter dir.\n\n",
		    
	    "Eine Tür öffnet sich und $der() nähert sich mit gesenktem "
	    "Kopf $dir(), daraufhin schließt sich die Tür wieder.",
		    
	    Der(this_player()) + " öffnet die Tür, verweist "
             +den(raus)+" des Raumes und schließt die Tür wieder.",
		     
	    "Du öffnest die Tür und verweist " + den(raus) + " des Raumes.",
		    
	    "\n"+Der(this_player())+" verweist dich des Raumes.\n\n",
		    
	    "$Der() nähert sich mit gesenktem Kopf $dir().",
		    
	    Der(raus) + " wird von " + dem(this_player()) +
	    " des Raumes verwiesen.",
		    
	    "Du verweisst "+den(raus)+" des Raumes.");
    }
    
    if(query_verb_ascii() == "schmeiss" || query_verb_ascii()=="schmeisse")
    {
	object raus;

        if(parse_com_error(parsed,"Wen willst du aus dem Raum schmeißen?\n",1))
	    return 0;

	raus = parsed[PARSE_OBS][0];
		
        if(parsed[PARSE_REST]!="raus" && parsed[PARSE_REST]!="hinaus")
            return notify_fail("Wohin willst du jemanden schmeißen?\n");
		
        return rausfunktion(raus,
		    
            "\n"+Der(this_player())+" öffnet die Tür, schmeißt dich im "
	    "hohen Bogen aus dem Spielerratsraum und schließt die Tür "
	    "anschließend wieder.\n\n",
			
	    "Eine Tür geht auf und $der() kommt im hohen Bogen $dir() "
	    "angeflogen. Daraufhin schließt sich die Tür wieder.",
			
	    Der(this_player())+" öffnet die Tür und schmeißt "+den(raus)+
	    " im hohen Bogen aus dem Raum. Danach schließt "+
	    er(this_player())+" wieder die Tür.",
			
            "Du öffnest die Tür und schmeißt "+den(raus)+" im hohen "
	    "Bogen aus dem Raum. Danach schließt du die Tür wieder.",
			
	    "\n"+Der(this_player())+" schmeißt dich im hohen Bogen aus "
	    "dem Spielerratsraum.\n\n",
			
	    "$Der() kommt im hohen Bogen $dir() angeflogen.",
			
	    Der(this_player())+" schmeißt "+den(raus)+" im hohen Bogen aus "
	    "dem Raum.",
			
	    "Du schmeißt "+den(raus)+" im hohen Bogen aus dem Raum.");
    }
    
    if(query_verb_ascii()=="pruegel" || query_verb()=="pruegele" || 
       query_verb_ascii()=="pruegle")
    {
	object raus;
        
        if(parse_com_error(parsed,"Wen willst du aus dem Raum prügeln?\n",1))
	    return 0;

	raus = parsed[PARSE_OBS][0];

        if(parsed[PARSE_REST]!="raus" && parsed[PARSE_REST]!="hinaus")
            return notify_fail("Wohin willst du jemanden prügeln?\n");
	
	return rausfunktion(raus,
		    
	    "\n"+Der(this_player())+" öffnet die Tür und verpasst dir "
	    "einen schmerzhaften Tritt in den Hintern, welcher dich vor die "
	    "selbige befördert. Danach schließt "+er(this_player())+
	    " die Tür wieder.\n\n",
	    
	    "Eine Tür geht auf und $der() nähert sich mit schmerzverzogenem"
	    " Gesicht $dir(). Danach wird die Tür wieder geschlossen.",
	    
	    Der(this_player())+" öffnet die Tür und verpasst "+dem(raus)+
	    " einen schmerzhaften Tritt in den Hintern, welcher "+ihn(raus)+
	    " vor die selbige befördert. Anschließend schließt "+
	    er(this_player())+" die Tür wieder.",
			
	    "Du öffnest die Tür und verpasst "+dem(raus)+" einen "
	    "schmerzhaften Tritt in den Hintern, welcher "+ihn(raus)+" vor "
	    "die selbige befördert. Danach schließt du die Tür wieder.",
	    
	    "\n"+Der(this_player())+" verpasst dir einen schmerzhaften Tritt "
	    "in den Hintern, welcher dich vor die Tür befördert.\n\n",
			
	    "$Der() nähert sich mit schmerzverzogenem Gesicht $dir().",
			
	    Der(this_player())+" verpasst "+dem(raus)+" einen schmerzhaften "
	    "Tritt in den Hintern, welcher "+ihn(raus)+" vor die Tür "
	    "befördert.",
			
	    "Du verpasst "+dem(raus)+" einen schmerzhaften Tritt in den "
	    "Hintern, welcher "+ihn(raus)+" vor die Tür befördert.");
    }
    
    if(query_verb()=="zeig" || query_verb()=="zeige")
    {
	object raus;
	object tuer;
        
        if(parse_com_error(parsed,"Zeige wem was?\n",1))
	    return 0;

	raus = parsed[PARSE_OBS][0];

	if((tuer=present("SRTuer")) && !tuer->me(parsed[PARSE_REST]))
	    return 0;
	
	return rausfunktion(raus,

	    "\n"+Der(this_player())+" zeigt dir die Tür. "+Er(this_player())+
	    " möchte wohl, dass du gehst.\nAlso öffnest du die Tür und "
	    "verlässt diesen Raum Richtung "+
	    capitalize(query_tuer_richtung())+". \nHinter dir schließt du "
	    "natürlich wieder die Tür.\n\n",
	    
	    "Eine Tür öffnet sich. $Der() nähert sich $dir() und "
	    "schließt hinter sich wieder die Tür.",
	    
	    Der(this_player())+" zeigt "+dem(raus)+" die Tür.\n"+Der(raus)+
	    " öffnet daraufhin die Tür, verlässt den Raum und schließt "
	    "die Tür wieder hinter sich.",
	    
	    "Du zeigst "+dem(raus)+" die Tür. "+Der(raus)+" öffnet "
	    "daraufhin die Tür und verlässt den Raum. Hinter sich "
	    "schließt "+er(raus)+" sie wieder.",
	    
	    "\n"+Der(this_player())+" zeigt dir die Tür. "+Er(this_player())+
	    " möchte wohl, dass du gehst.\nAlso verlässt du diesen Raum "
	    "Richtung "+capitalize(query_tuer_richtung())+".\n\n",
	    
	    "$Der() nähert sich $dir().",
	    
	    Der(this_player())+" zeigt "+dem(raus)+" die Tür.\n"+Der(raus)+
	    " verlässt daraufhin den Raum.",
	    
	    "Du zeigst "+dem(raus)+" die Tür. "+Der(raus)+" verlässt "
	    "daraufhin den Raum.");
    }
    
    if(query_verb()=="schnips" || query_verb()=="schnipse")
    {
	object raus;
	
        if(parse_com_error(parsed,"Wen willst du rausschmeißen lassen?\n",1))
	    return 0;

	raus = parsed[PARSE_OBS][0];

	if(!living(raus))
	    return notify_fail(wrap(Der(raus)+" ist kein Lebewesen."),
		    FAIL_WRONG_ARG);
	
        if(raus==this_player())
	    return notify_fail("Nö.\n");

	
        this_player()->send_message(MT_NOISE, MA_EMOTE,
	    wrap(Der(this_player())+" schnipst kurz mit den Fingern."),
	    wrap("Du schnipst kurz mit den Fingern."), this_player());
	    
	if(find_call_out("schnips1") == -1 &&
	   find_call_out("schnips2") == -1 &&
	   find_call_out("schnips3") == -1 &&
	   find_call_out("schon_weg")== -1)
	    call_out("schnips1", 3, raus);
        return 1;
    }
}

static void schnips1(object raus)
{
    if(!raus || !this_player())
	return;

    this_player()->send_message_in(
	touch(this_object()->query_one_exit(query_tuer_richtung())),
        MT_NOTIFY,MA_MOVE_OUT,
        wrap("Zwei Rathausangestellte nähern sich und verschwinden "
         "durch die Tür im "+capitalize(query_raum_richtung())+"."));
	 
    this_player()->send_message(MT_NOTIFY, MA_LOOK, 
        wrap("Plötzlich kommen zwei muskulöse Rathausangestellte durch die "
        "Tür und sehen "+den(this_player())+" fragend an."),
	
        wrap("Plötzlich kommen zwei muskulöse Rathausangestellte durch die "
         "Tür und sehen dich fragend an."), this_player());
	 
    call_out("schnips2", 2, raus);
}

static void schnips2(object raus)
{
    if(!this_player())
	call_out("schon_weg", 2, raus);
    else if(!raus || !present(raus))
    {
        this_player()->send_message(MT_NOISE, MA_COMM,
            wrap(Der(this_player())+" sagt: Oh, ich seh gerade, es hat sich "
            "erledigt."),
	    wrap("Du sagst: Oh, ich seh gerade, es hat sich erledigt."),
	    this_player());		
	
        call_out("schon_weg",2,raus);
    }
    else
    {
        this_player()->send_message(MT_NOISE, MA_COMM,
            wrap_say(Der(this_player())+" sagt:", Der(raus)+" möchte gehen."),
            wrap_say("Du sagst:",Der(raus)+" möchte gehen."), this_player());

        call_out("schnips3",3,raus);
    }
}

static void schon_weg(object raus)
{
    if(!raus || !present(raus) || !this_player())
    {
        this_object()->send_message(MT_NOTIFY, MA_LOOK,
            wrap("Die Angestellten nicken knapp und verlassen wieder "
             "den Raum."));
	
	(this_object()->query_one_exit(query_tuer_richtung()))->send_message(
            MT_NOTIFY,MA_MOVE_OUT,
            wrap("Kurz darauf kommen die Angestellten wieder heraus und "
             "entfernen sich."));
    }
    else
    {
        call_out("schnips3",3, raus);
    }
}

static void schnips3(object raus)
{
    if(!raus || !present(raus))
    {
        this_object()->send_message(MT_NOTIFY, MA_LOOK,
            wrap("Die Angestellten schauen verblüfft drein, zucken dann "
             "aber mit ihren Schultern und gehen wieder."));
	
	(this_object()->query_one_exit(query_tuer_richtung()))->send_message(
            MT_NOTIFY,MA_MOVE_OUT,
	    wrap("Kurz darauf kommen die Angestellten wieder heraus und "
             "entfernen sich."));
    }
    else
    {
        object tuer = present("SRTuer");

	sys_log("SR_RAUSWURF", sprintf("%s | %10-s rauswurf %#Q\n",
	    shorttimestr(time()),
	    this_player() && this_player()->query_real_name(), 
	    raus->query_real_name() || 
		({raus, raus->query_name(), raus->query_short()})));

        this_object()->send_message_to(raus, MT_NOTIFY, MA_MOVE_OUT,
            wrap("Die Angestellten nicken knapp, packen dich an den Armen "
             "und verlassen mit dir den Raum.\n\n"));
	
        if(tuer->query_door_is_open()==0)
        {
            tuer->open_door(0,1);
            real_move(raus,
                "Kurz darauf wird $der() von ihnen $dir() herangeschleift und "
                "fallen gelassen. Daraufhin verschwinden beide wieder.");
            tuer->close_door(0,1);
        }
        else
        {
	    real_move(raus,
                "Kurz darauf wird $der() von ihnen $dir() herangeschleift und "
                "fallen gelassen. Daraufhin verschwinden beide wieder.");
        }
	
        this_object()->send_message(MT_NOTIFY, MA_MOVE_OUT,
            wrap("Die Angestellten nicken knapp, packen "+den(raus)+" an den "
            "Armen und verlassen mit "+ihm(raus)+" den Raum."));
    }
}

string sr_tafel(mapping vitem, object betrachter)
{
    if(spielerratp(betrachter) || adminp(betrachter))
        return wrap("Auf dieser unscheinbaren Tafel scheinen Hinweise für dich "
            "zu stehen, wie du hier kreativ Gäste hinaus befoerden "
            "kannst, welche sich unbeliebt gemacht haben.");
    else
        return "Hmmm... eine Tafel. Mit irgendeinem albernen Gekritzel "
            "drauf.\n";
}

string sr_text(string parse_rest, string str, mapping vitem, object betrachter)
{
    if(spielerratp(betrachter) || adminp(betrachter))
        return "Folgende Aktionen sind hier möglich: \n"
            "    - verweis[e] <jemanden>\n"
            "    - schmeiss[e] <jemanden> raus|hinaus\n"
            "    - prügel <jemanden> raus|hinaus\n"
            "    - zeig[e] <jemanden> die tür\n"
            "    - schnips[e] <jemanden>.\n";
    else 
        return "Dieses Gekrakel kannst du leider nicht entziffern.\n" ;
}

void create()
{
    this_object()->add_v_item(([
        "name":     "tafel",
        "gender":   "weiblich",
        "id":     ({"tafel","hinweis","gekritzel","gekrakel"}),
        "long":     #'sr_tafel,
        "read":     #'sr_text ]));
	
     this_object()->add_controller("forbidden_spy_here",
        (: 
	    object tuer = present("SRTuer");
	    return tuer && !tuer->query_door_is_open();
	:));
	
    init_security_for_actions();
}
