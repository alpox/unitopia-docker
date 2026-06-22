// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/room/rathaus/spielerrat/i/rauswurf_tuer.c
// Description: Rathaus, Spielerratsraeume, Rauswurfsinherit
// Author:      Goldie (14.10.2004)

#include <apps.h>
#include <level.h>
#include <message.h>
#include <misc.h>
#include <move.h>

string t_richtung, r_richtung;

void set_tuer_richtung(string str)
{
    t_richtung = str;
    r_richtung = opposite_direction(str) || r_richtung;
}

string query_tuer_richtung()
{
    return t_richtung;
}

void set_raum_richtung(string str)
{
    r_richtung = str;
}

string query_raum_richtung()
{
    return r_richtung;
}

void reset()
{
    object tuer;
    
    if (inheritp(TO)) return;

    if (!(tuer=present("SRTuer"))) 
    {
        tuer=clone_object("/obj/tuer");
        tuer->init_door(this_object()->query_one_exit(query_tuer_richtung()),
	    query_tuer_richtung());
	tuer->set_door_height(8);
        tuer->move(this_object());
	if(!strstr(object_name(), "/room/rathaus/spielerrat/raum"))
    	    tuer->add_controller("forbidden_close_door");
	else
    	    tuer->add_controller("forbidden_open_door");
        tuer->add_id(({"SRTuer", "tür"}));
    }
    if(tuer->query_door_is_open())
    {
        tuer->close_door(0,1);
        this_object()->send_message(MT_NOTIFY,MA_LOOK,
            wrap("Ein Windhauch schließt leise die Tür."));
        tuer->other_door()->send_message(MT_NOTIFY, MA_LOOK,
            wrap("Ein Windhauch schließt leise die " + 
		([ "norden":	"nördliche",
		   "nordosten": "nordöstliche",
		   "osten":	"östliche",
    		   "südosten": "südöstliche",
		   "süden":	"südliche",
		   "südwesten":"südwestliche",
		   "westen":	"westliche",
		   "nordwesten":"nordwestliche"
		])[query_raum_richtung()] + " Tür."));
    }
}

int forbidden_close_door(object wer, object tuer, object raum, int second)
{
    if(second == 1) return 0;
    
    if(spielerratp(wer) || adminp(wer)) 
	return 0;
    else
    {
	wer->send_message_to(wer, MT_NOTIFY, MA_USE,
	    wrap("Diese Tür kann nur von Spielerratsmitgliedern "
            "geschlossen werden."));
        return 1;
    }
}

int forbidden_open_door(object wer, object tuer, object raum, int second)
{
    if(second == 1) return 0;
    
    if(spielerratp(wer) || adminp(wer))
	return 0;
    else
    {
	wer->send_message_to(wer, MT_NOTIFY, MA_USE,
	    wrap("Diese Tür kann nur von Spielerratsmitgliedern "
            "geöffnet werden."));
        return 1;
    }
}
