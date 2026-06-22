// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/room/rathaus/div/shut.c
// Description: Armageddon faehrt das Mud langsam und gesichert runter.
// Modified by: Freaky (29.09.96)
//		Freaky (06.01.1999) Move geht nur noch in VALID_ROOMS,
//			da die Spieler Armageddon sonst durch Tadmor ziehen.
//              Tmm    (11.06.2000) - an Neue Bank angepasst

/*
 * This is a shut down deamon, that will take care of shutting down
 * the game.
 * Call the function "shut" with a number of minutes as an argument.
 *
 * 12/7/91: Enhanced by Olorin for Deep Trouble to include an optional view
 *          of how much time is before reboot (Wizards) or less than 1/2
 *          hour (players)
 *          Wizard level >= reboot may reboot the game using 'fastshut [mins]'
 */

#include <time.h>
#include <config.h>
#include <move.h>
#include <level.h>
#include <stats.h>
#include <debug_info.h>
#include <driver_info.h>
#include <message.h>

#ifdef UNItopia
#include "/d/Vaniorh/sys/vaniorh_exports.h"
#endif

inherit "/i/monster/monster";
inherit "/i/tools/security";

#include "rollen_weg.inc"

#define CAN_REBOOT(x) lordp(x)

#define SHUT_MASTER  "/apps/shut"
#define INITIAL_ROOM "/room/rathaus/reserv"
#define SHUTDOWN_ROOM "/room/church"
#define VALID_ROOMS ({ INITIAL_ROOM, SHUTDOWN_ROOM, "/room/void" })

#define MONATSERSTER ({ 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334,\
                        365 })

int initial_c_time;
int shut_c_time;
int transport_offer;


private void cont_shutting(int seconds);

int query_prevent_shadow(object sh)
{
    return 1;
}

private int my_shorttimestr2time(string datstring)
{
  int n,tag,mon,jahr,h,m,s;
  if(!stringp(datstring) ||
     (sscanf(datstring,"%d.%d.%d %d:%d:%d",tag,mon,jahr,h,m,s) < 5))
  {
      return 0;
  }
  jahr=jahr % 100;
  jahr += (jahr<70?30:-70);        // Jahre seit 1970
  n +=  jahr*365 + (jahr+2)/4;     // Tage und Schalttage. Das erste Schaltjahr
                                   // ist 1972, also jahr==2
  if (!((jahr+2)%4) && mon<3) n-=1;// dann kommt der Schalttag dieses Jahres
                                   // erst noch
  //gegen RTEs:
  if(mon<1)
      mon=1;
  n += (tag+MONATSERSTER[mon-1]-1);
  n *= 24; n+=h;
  n -= 1;                          // 1 Stunde abziehen! Es wurde am 1.1.70
                                   // um 1 Uhr angefangen zu zaehlen!
  if (mon>3 && mon<11) n-=1;       // Sommerzeit
  n *= 60; n+=m;
  n *= 60; n+=s;
  return n;
}


void move_to_initial_room ()
{
    if (touch (INITIAL_ROOM))
       move(INITIAL_ROOM,([MOVE_FLAGS:MOVE_MAGIC]));
}

void init()
{
    add_action("heart_shut","abschuss");
    add_action("terminate","termin");
}

string query_shut_date()
{
    int next;

    if (shut_c_time)
        return timestr(shut_c_time + initial_c_time);

    next = SHUT_MASTER.query_next_shutdown();
    if (next)
        return timestr(next);
}

string query_noise()
{
    if (!shut_c_time)
    {
        return "Armageddon zählt gerade nicht, also hörst du nichts.\n";
    }
    return wrap("Armageddon zählt leise und du kannst raushören, "
            "wann er damit fertig sein will: "
            +timestr(shut_c_time + initial_c_time));
       
}


string query_long(object who)
{
    int how_c_long;
    string desc;

    desc = "Das Terminieren von Spielen macht ihm besonderen Spaß.\n";

    if (!shut_c_time) 
    {
        if (wizp(who))
            desc += "Einleitung eines Shutdowns mit\n"
            	"'abschuss <minuten>' oder 'termin <datum uhrzeit>'.\n";
        return desc;
    }

    how_c_long = (shut_c_time + initial_c_time) - time();

    if (wizp(who))
    {
        desc += "Derzeitiger Shutdown : "+format_seconds(how_c_long)+".\n";
    }
    else
    {
	desc += "Er ist wohl gerade mit Zählen beschäftigt.\n";

	/* Inform lower-level creatures roughly about time until reboot */

        switch (how_c_long) 
        {
            case 0..3599 : // 0 - 1h
	        desc += "Du bist Dir fast sicher, dass er in weniger als "
                        "einer Stunde aufwacht und das\n"
                        "Spiel terminiert.\n";
                break;
            case 3600..43199 : // 1h - 12h
                desc += "Du hast das Gefühl, dass er innerhalb des "
                        "nächsten halben Tages erwachen\n"
                        "und das Spiel terminieren wird.\n";
                break;
            case 43200..172799 : // 12h - 2d 
                desc += "Es wird wohl keine zwei Tage mehr dauern, bis er "
                        "mit dem Zählen fertig ist\n"
                        "und das Spiel terminiert.\n";
                break;
            case 172800..604799 : // 2d - 7d
                desc += "Einige Tage wird er mit dem Zählen sicher noch "
                        "beschäftigt sein, bevor er\n"
                        "das Spiel dann terminieren wird.\n";
                break;
            case 604800..1209599 : // 7d - 14d
                desc += "Bis es soweit ist und er das Spiel terminieren "
                        "wird, zählt er sicher noch\n"
                        "viele Tage vor sich hin.\n";
                break;
            default : // > 2 Wochen
                desc += "So entspannt, wie er zählt, ist er sicher "
                        "noch einige Wochen beschäftigt,\n"
                        "bevor er erwacht, um das Spiel zu terminieren.\n";
        }
    }

    return desc;
}

/* Wird vom Driver aufgerufen, wenn der Hauptspeicher knapp wird. */
void shut(int minutes)
{
    if (object_name(previous_object())!=MASTER_OB &&
        object_name(previous_object()) != "/apps/shut" &&
	previous_object()!=this_object())
	return;

    if (minutes < 0)
	minutes = 0;

#ifdef LOG_SHUTDOWN
    sys_log("SHUT","Shutdown per Armageddon gestartet von "+
        object_name(previous_object()) + " in "+minutes+" Minuten am "+
        timestr(time())+".\n");
#endif

    shut_c_time = minutes * 60;
    initial_c_time = time();

    move(SHUTDOWN_ROOM,([MOVE_FLAGS:MOVE_MAGIC]));

    call_out(#'cont_shutting, 0, shut_c_time);
}

private void bruelle(string str)
{
    filter(efun::users(),#'tell_object,str);
    sys_log("SHOUTS",str);
}

private void cont_shutting(int seconds)
{
    int new_delay;

    if (seconds <= 0)
    {
	bruelle("\nArmageddon brüllt: Und Tschüss!\n");
#ifdef LOG_SHUTDOWN
	sys_log("SHUT","Shutdown: "+timestr(time())+"   "+__VERSION__+"\n"+
			"   "+mixed2str(rusage())+"\n"+
			"   Uptime: "+format_seconds(query_up_time())+"\n");
#endif
#if __EFUN_DEFINED__(dump_driver_info)
	efun::dump_driver_info(DDI_OBJECTS);
#else
	efun::debug_info(DINFO_DUMP,"objects");
#endif
	efun::garbage_collection();
	call_out(#'efun::shutdown, __ALARM_TIME__); // Ein Zyklus spaeter wg. GC
	return;
    }

    new_delay = seconds * 3 / 4 - 10;
    call_out(#'cont_shutting, seconds - new_delay, new_delay);

    bruelle("\n"+wrap("Armageddon brüllt: Noch " +
    	format_seconds(seconds)+" bis zum Neustart.")+"\n");

    if (seconds <= 300)
    {
	bruelle(wrap("Armageddon brüllt: 'Redet' zu mir, wenn ihr zum Laden "
		"oder zur Bank wollt!")+"\n");
	transport_offer = 1;
    }
}

void receive_message(int msg_type, int msg_action, object wer, string msg)
{
    string what;

    if (!transport_offer)
	return;
    if (sscanf(msg, "%~s redet zu dir: %s", what) != 2)
	return;
#ifdef UNItopia
    if (strstr(what,"bank") >= 0)
	wer->move(TADMOR_BANK_SCHALTERHALLE, ([MOVE_FLAGS: MOVE_MAGIC]) );
    else
	wer->move(TADMOR_ANKAUFSLADEN, ([MOVE_FLAGS: MOVE_MAGIC]));
#else	
    wer->move("/room/laden/verkaufsraum", ([MOVE_FLAGS: MOVE_MAGIC]));
#endif
}



int terminate(string str)
{
    int newtime;
    if (!check_security() || !CAN_REBOOT(this_player()) )
        return 0;

    newtime = my_shorttimestr2time(space(str));
    if (!newtime)
    {
	notify_fail("Benötigtes Zeitformat DD.MM.YYYY HH:MI:SS\n");
	return 0;
    }
    if (newtime < (time()+600))
    {
        notify_fail("Bisschen knapp die Zeit!\n");
	return 0;
    }
#ifdef LOG_SHUTDOWN
    sys_log("SHUT","Shutdown per Armageddon gestartet von "+
	    this_interactive()->query_real_name()+" für Termin "+str+" am "+
	    timestr(time())+".\n");
#endif
    shut_c_time = newtime - time();
    initial_c_time = time();
       move(SHUTDOWN_ROOM,([MOVE_FLAGS:MOVE_MAGIC]));
    call_out(#'cont_shutting, 0, shut_c_time);

    write("Ok.\n");
    return 1;
}

int heart_shut(string str)
{
    int minutes;
    if (!check_security() || !CAN_REBOOT(this_player()) )
        return 0;

    if (!str)
    {
	notify_fail("In wieviel Minuten ?\n");
	return 0;
    }
    if (!sscanf(str, "%d", minutes))
    {
	notify_fail("Das soll ne Zahl sein ???\n");
	return 0;
    }
    if (minutes < 1)
    {
	if (adminp(this_player()) && this_player()==this_interactive())
	    cont_shutting(0);
	notify_fail("Bisschen knapp die Zeit!\n");
	return 0;
    }

    shut_c_time = minutes * 60;
    initial_c_time = time();
       move(SHUTDOWN_ROOM,([MOVE_FLAGS:MOVE_MAGIC]));
    call_out(#'cont_shutting, 0, shut_c_time);

    write("Ok.\n");
    return 1;
}

int remove()
{
    if (shut_c_time)
    {
        sys_log("SHUT",timestr(time())+" Armageddon gestoppt "+
             (this_interactive()?this_interactive()->query_real_name():"-")
             +"\n");
    }
    remove_call_out("shut");
    remove_call_out(#'cont_shutting);
    return monster::remove();
}

varargs int add_hp(int i, mapping infos)
{
    if (i < 0)
	return -1;
    return ::add_hp(i,infos);
}

<int|string> forbidden_move(mapping mv_infos)
{
    string room = object_name(mv_infos[MOVE_NEW_ROOM]);
    return (member(VALID_ROOMS,room) < 0);
}

string query_finger_info_text()
{
    string t = query_shut_date();
    t = t?
        ("Armageddon plant einen Weltuntergang für "+t+".\n")
        :"Momentan ist kein Weltuntergang geplant.\n";
    return "Armageddon macht es Spaß, UNItopia zu terminieren\n"
           "und danach wieder neu zu erschaffen.\n"
	   "Zuletzt war er am "+timestr(time()-query_up_time())+" aktiv.\n"+t;
}


void create()
{
    int i;

    monster::create();

    initialize("mensch",100);
    set_name("armageddon");
    set_npc_name("armageddon");
    give_hands(4);
    give_sp(666);
    give_hp(666);
    give_weapon_level(666);
    give_armour_level(666);
    for (i = 0; i < STAT_NUMBER; i++)
	set_one_stat(i,1272);
    set_id(({"armageddon","shut"}));
    set_short("Armageddon");
    set_gender("maennlich");
    set_smell("Er riecht nach Pech und Schwefel.\n");
    set_personal(1);
    set_prevent_cleanup();
    
    init_security_for_actions();
    
    call_out ("move_to_initial_room",2);
    add_controller("forbidden_move");

    initial_c_time = 0;
    shut_c_time = 0;
}
