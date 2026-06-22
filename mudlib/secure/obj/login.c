// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/secure/obj/login.c
// Description: Das Login-Objekt
// Author:	Freaky	(23.12.93)
// Modified by:	Garthan	(18.07.94) queuing
// 		Freaky	(08.07.95) Player-Logging in den Level-Lister
// 		Freaky  (12.01.96) IP-Filter
//		Sissi   (23.09.96) Selbstsperrung
//		Sissi   (06.02.97) Gaeste koennen Geschlecht waehlen
// 		Sissi   (25.06.97) Wizard - Shell
//              Sissi   (27.06.96) Hilfstext, finger - Befehl, wer - Befehl,
//                                 ende, exit, quit usw. - Befehle
//		Freaky  (27.06.96) remove() bei wizardshell eingebaut
//		Sissi   (29.06.96) remove() an richtige Stelle verlegt
//		Freaky  (29.03.1998) set_living_name schon beim Namen setzen
//		Freaky  (12.05.1999) KOELN_ROEMER eingebaut
//				     Das ist fuer 'Koeln zur Roemerzeit'
//              Sissi   (08.06.2000) no_welcome ()

#pragma strong_types
#pragma no_warn_unused_variables

inherit "/secure/i/sperre";
inherit "/i/player/telnet_neg";

#define EXIT(x) do { write(x || ""); try_again(); return; } while(0)
#define EXITV(x,v) do { write(x || ""); try_again(); return (v); } while(0)

#undef SPERRUNG
#define WUNSCH "Wie lautet Dein Name: "
#define HILFSTEXT "/static/adm/LOGIN_HELP"
#define INTRO "/static/adm/WELCOME"
#define NOWELCOME "/static/adm/NOWELCOME/"

#include <error_db.h>
#include <error.h>
#include <message.h>
#include <finger.h>
#include <input_to.h>

#define LINE "-------------------------------------------------------------------------------\n"
#include <config.h>
#include <move.h>
#include <gilden.h>
#include <touch.h>
#include <level.h>
#include <news.h>
#include <mail.h>
#include <apps.h>
#include <invis.h>
#include <passwd.h>
#include <uids.h>
#include <message.h>
#include <commands.h>
#include <rtlimits.h>
#include <shadow.h>
#include <telnet.h>
#if __EFUN_DEFINED__(interactive_info)
#include <interactive_info.h>
#include <driver_info.h>
#endif
#include "/sys/player.h"

#define MOVE_LIMIT 250000

#ifdef UNItopia
#define BANISHED_LOG "/var/adm/BANISHED_LOGINS"
#define IPFILTER_LOG "/var/adm/IP_FILTERED"

#define DEBUG_EVALS
#endif

static int evals=__MAX_EVAL_COST__, di_counter;
static mapping debug_infos=([]);

#ifdef DEBUG_EVALS
#define PRINT_EVALS(str) debug_infos[sprintf("Evals %02d.",++di_counter)]=sprintf("%6d (%6d): %s",evals-(evals=get_eval_cost()),evals,(str))
#else
#define PRINT_EVALS(str)
#endif

#ifdef RELAYD
static string real_ip_name, real_ip_number;
#endif

// Spieler, die nicht reinduerfen und ne Meldung dazu bekommen sollen
int nowelcome_line = 0;

int level;
int version;
int ghost;
int hp;
string mail_address;
mixed artikel;
mixed start_room;
string gender;
string gilde;
string *opfer;
mixed *offline_opfer;
string last_host;
int last_login;
int logout_via_start_or_stadt;
// Oldstyle Bewegungsmeldungen, zur Konvertierung
string msgin, msgout, mmsgin, mmsgout;

static string real_name, new_name, wiz_owner;
static int new;
static int pressed_return;
static int logged_in;
static int ins_passwd;
static int wiz_shell;
static int count_empty_cmd;

// Geradezahlige Indices = weibliche Namen
// Ungeradezahlige Indices = maennliche Namen
static string *guests = GUEST_NAMES;

// Prototypes
static void setup_player();
static void wait_to_continue2();
private void setup_wiz_shell();
void no_welcome ();
private void transfer_gmcp(object player);

#ifdef DEBUG_EVALS
mapping query_debug_info() {return debug_infos;}
#endif

static void try_again()
{
    call_out("remove", 10);
    write("Bitte <Enter>-Taste drücken...\n");
    input_to("remove");
}


void add_offline_opfer(mixed oos)
{
    if(object_name(previous_object())==PLAYER_MODIFIER)
	offline_opfer+=oos;
}

void receive_message(int msg_type, int msg_action, object who, string str)
{
    efun::tell_object(this_object(),str);
}

void receive_message_low(string msg)
{
    efun::tell_object(this_object(), msg);
}

int guest(mixed ob)
{
   string name;

   return objectp(ob) &&
	  (name = ob->query_real_name()) &&
	  member(guests, name) >= 0 ||
	  stringp(ob) &&
	  member(guests, ob) >= 0;
}

string *query_guests() { return ({}) + guests; }

void create()
{
    "*"::create();
}

int remove()
{
    remove_call_out("time_out");
    destruct(this_object());
    return 1;
}

void prepare_renewal() {}
void finish_renewal(object neu) {}
void abort_renewal() {}

nomask int query_level() { return level; }
nomask int query_wiz_level() { return level >= LVL_WIZ ? level : 0; }

nomask string query_real_name() { return real_name; }
nomask string query_current_wiz_owner() { return wiz_owner; }

string query_name() { return new_name; }

nomask mixed query_start_room() { return start_room; }


void log_filtered_player(string name, string msg)
{
#ifdef IPFILTER_LOG
    if (file_size(IPFILTER_LOG)>65000)
        rename(IPFILTER_LOG,IPFILTER_LOG".old");
    write_file(IPFILTER_LOG,
               left(name,11)+
               shorttimestr(time())+" "+
#if __EFUN_DEFINED__(query_ip_name)
               efun::query_ip_name()
#else
               efun::interactive_info(this_object(), II_IP_NAME)
#endif
               +"\t"+msg+"\n");
#endif
}

static void time_out()
{
    write("\nZu lange gewartet.\n");
    remove();
}

static void show_news(string str, int step)
{
    string news, header;

PRINT_EVALS("Start von show_news");
    switch(step)
    {
        case 0:
            if (new == 2) // Gast
                break;

            catch(news = NEWSD->query_news(this_object()));
            header = "------------------- Neuigkeiten (bitte mit 'neues' lesen) ---------------------\n";
            if (sizeof(news))
                break;

            step++;

        case 1:
            catch(news=NEWSD->query_tknews(this_object()));
            header = "------------------ Neuigkeiten (bitte mit 'neues tk' lesen) -------------------\n";
            if (sizeof(news))
                break;

            step++;

        case 2:
            if (level < LVL_HLP)
                break;

            if (level == LVL_HLP)
            {
                catch(news=NEWSD->query_hlpnews(this_object()));
                header = "------------------------- Neuigkeiten für Engel -------------------------------\n";
                if (sizeof(news))
                    break;
            }

            step++;

        case 3:
            if (level < LVL_WIZ)
                break;

            catch(news=NEWSD->query_wiznews(this_object()));
            header = "------------------------ Neuigkeiten für Götter ------------------------------\n";
            if (sizeof(news))
                break;

            step++;

        case 4:
            if (level < LVL_WIZ)
                break;

            catch(news=NEWSD->query_prognews(this_object()));
            header = "--------------------- Neuigkeiten zur Programmierung --------------------------\n";
            if (sizeof(news))
                break;
    }

    if (sizeof(news))
    {
        write(header + news + LINE);
        input_to("show_news", INPUT_PROMPT, "Weiter mit Return. ", step + 1);
    }
    else if(level >= LVL_GESELLE)
        wait_to_continue2();
    else
        setup_player();
}

static void do_continue()
{
PRINT_EVALS("Start von do_continue");
    logged_in = 1;
    pressed_return = 1;

    show_news(0, 0);
}

static string query_gesperrt();
static int valid_name(string name);

#ifdef SPERRUNG
static void sperr_pass(string pass)
{
    object throw_out_ob, dummy, ob;
    string tmp;

    write("\n");

    if (pass == "") {
        EXIT("Kein Passwort eingegeben.\n");
    }
    if (time()-FAILD->query_time_of_last_but_two_fail(new_name) < 300)
    {
        EXIT(wrap("In den letzten 5 Minuten waren mindestens 3 Versuche "
            "mit falschem Passwort, sich mit Deinem Namen einzuloggen. "
            "Daher ist der Login mit Deinem Namen jetzt 5 Minuten "
            "gesperrt."));
    }
    if (!PASSWD_CHECK(pass, password))
    {
#if __EFUN_DEFINED__(query_ip_name)
	FAILD->add_fail(new_name, efun::query_ip_name());
#else
	FAILD->add_fail(new_name, efun::interactive_info(this_object(), II_IP_NAME));
#endif
        EXIT("Falsches Passwort.\n");
    }
    real_name = new_name;

    if(LVL_PLAYER >= BANISHD->query_banished(real_name))
    {
       log_banished_player(real_name);
       no_welcome ();
       EXIT(0);
    }

    if (sperre_bis > time()) {
        write(wrap("Du hast Deinen Charakter bis zum " +
            timestr(sperre_bis) + " leider selbst gesperrt."));
        EXIT(0);
    }

    if (tmp = query_gesperrt()) {
        write("Du hast Deinen Charakter für diesen Wochentag zu dieser "
               "Uhrzeit\nleider selbst gesperrt.\n");
        EXIT(0);
    }
                                           
    remove_call_out("time_out");
    set_living_name("Login:"+real_name);
    throw_out_ob = find_player(real_name) || find_player("STATUE "+real_name)
    	|| find_player("MAIL "+real_name) || find_player("login:"+real_name);
    set_living_name("login:"+real_name);

    if (throw_out_ob)
    {
	throw_out_ob->remove();
	if (throw_out_ob)
	    destruct(throw_out_ob);
    }
    
    ob=clone_object("/obj/pplayer");
    exec(ob,this_object());
    ob->setup_player();
    transfer_gmcp(ob);
    remove();
}

static void sperr_name(string str)
{
    string name;
    int res;
    
    if (!str || str == "")
    {
        if(count_empty_cmd)
	{
           EXIT("Tschüss!\n");
	}
	else
	    count_empty_cmd++;
	input_to("sperr_name", INPUT_PROMPT|INPUT_IGNORE_BANG,
	    "Name: ");
        return;
    }

    name = lower_case(str);
    if (name == "ende" || name == "nd" || name == "exit" || name == "quit"
        || name == "bye" || name == "q") {
        EXIT("Tschüss!\n");
    }

    if (name[0] == '*') {
        name = name[1..];
        wiz_shell = 1;
        write ("Wiz-Shell gewünscht.\n");
    }

    if (!valid_name(name))
    {
        write("Der Name ist ungültig.\n");
	input_to("sperr_name", INPUT_PROMPT|INPUT_IGNORE_BANG,
	    "Name: ");
	return;
    }
    
    new_name = name;

    res = restore_object(PLAYER_FILE(new_name));
    set_living_name("login:"+new_name);

    if (res == 0)
    {
	EXIT("Derzeit dürfen keine neuen Spielercharaktere angelegt werden.\n");
    }
    else
    {
	if (level < LVL_WIZ) {
            if(wiz_shell) {
                EXIT("Sorry, Du darfst die Wizard-Shell nicht benutzen!\n");
            }
	    seteuid(PLAYER_UID);
	} else if (level == LVL_LEARNER) {
            if(wiz_shell) {
                EXIT("Sorry, Du darfst die Wizard-Shell nicht benutzen!\n");
            }
	    seteuid(LEARNER_UID);
	} else
	    seteuid(new_name);
	new=0;
	if(level < LVL_WIZ && !testplayerp(new_name))
	    input_to("sperr_pass", INPUT_PROMPT|INPUT_NOECHO|INPUT_IGNORE_BANG, "Passwort: ");
	else
	    input_to("get_password", INPUT_PROMPT|INPUT_NOECHO|INPUT_IGNORE_BANG, "Passwort: ");
	return;
    }
}

static void sperr_info(string str, string *text)
{
    string name;
    str = lower_case(str);
    switch(str)
    {
	case "r":
	    write(text[0]);
	    input_to("sperr_info", INPUT_PROMPT|INPUT_IGNORE_BANG,
		"[Um Weiterzulesen bitte die <Enter>-Taste druecken] ",
		text);
	    return;
	case "q":
	    EXIT(0);
	default:
	    if(str && str[0]=='*')
		name = str[1..<1];
	    else
		name = str;
	    if(str && player_exists(name) &&
		(testplayerp(name) || GOETTER_REGISTER->is_wiz(name)))
		sperr_name(str);
	    else
	    {
		write(text[1]);
		if(sizeof(text)==2)
		    input_to("sperr_name", INPUT_PROMPT|INPUT_IGNORE_BANG,
			"Name: ");
		else
		    input_to("sperr_info", INPUT_PROMPT|INPUT_IGNORE_BANG,
			"[Um Weiterzulesen bitte die <Enter>-Taste druecken] ",
		    text[1..<1]);
	    }
	    return;
    }
}

int sperrung()
{
    string *intro;

    seteuid(ROOT_UID);    
    call_out("time_out",300);
    add_action("remove","ende");
    add_action("remove","quit");

    intro = explode(read_file("/static/adm/SPERRUNG"),"\n-\n");
    sperr_info("r", intro);
    return 1;
}
#endif // SPERRUNG

static void get_name(string str);

void print_more(string str, string* text)
{
    str = lower_case(str||"");
    switch(str)
    {
	case "r": 
	    write(text[0]);
	    input_to("print_more", INPUT_PROMPT|INPUT_IGNORE_BANG,
		"[Um Weiterzulesen bitte die <Enter>-Taste druecken] ",
		text);
	    return;
	case "q":
	    input_to("get_name", INPUT_PROMPT|INPUT_IGNORE_BANG, WUNSCH);
	    return;
	case "":
	    write(text[1]);
	    if(sizeof(text)==2)
		input_to("get_name", INPUT_PROMPT|INPUT_IGNORE_BANG, WUNSCH);
	    else
		input_to("print_more", INPUT_PROMPT|INPUT_IGNORE_BANG,
		    "[Um Weiterzulesen bitte die <Enter>-Taste druecken] ",
		    text[1..<1]);
	    return;
	default:
	    get_name(str);
	    return;
    }
}

void more(string filename)
{
    string* text = explode(read_file(filename),"\n-\n");
    print_more("r", text);
}

int check_existing_name()
{
	if (level < LVL_WIZ)
	{
#ifdef Orbit
	    if(!testplayerp(new_name))
    		EXITV("Sorry, Orbit ist den Göttern vorbehalten.\n", 0);
#endif
            if(wiz_shell)
                EXITV("Sorry, Du darfst die Wizard-Shell nicht benutzen!\n", 0);
	    seteuid(PLAYER_UID);
	}
	else if (level == LVL_LEARNER)
	{
            if(wiz_shell)
                EXITV("Sorry, Du darfst die Wizard-Shell nicht benutzen!\n", 0);
	    seteuid(LEARNER_UID);
	}
	else
	    seteuid(new_name);

	new=0;
	
	return 1;
}

int check_throw_out()
{
    object throw_out_ob, dummy;

    remove_call_out("time_out");
    set_living_name("Login:"+real_name);
    throw_out_ob = find_player(real_name) || find_player("STATUE "+real_name)
    	|| find_player("MAIL "+real_name) || find_player("login:"+real_name);
    set_living_name("login:"+real_name);

    if(wiz_shell)
    {
        if (!throw_out_ob)
        {
            EXITV("\nSolange Dein Character noch nicht eingeloggt ist, kannst "+
                  "Du die\nWizard-Shell leider nicht benutzen!\n\n", 0);
        }
        setup_wiz_shell();
        remove();
        return 0;
    }

    // Player ist noch drin und ist interaktiv gewesen!
    if (throw_out_ob)
    {
	if (interactive(throw_out_ob))
	{
	    if (playerp(throw_out_ob))
	    {
#if __EFUN_DEFINED__(query_ip_name)
		write(wrap("Du bist bereits am Spielen von " +
		    efun::query_ip_name(throw_out_ob)+" aus. Übernehme Körper..."));
#else
		write(wrap("Du bist bereits am Spielen von " +
		    efun::interactive_info(throw_out_ob, II_IP_NAME)+" aus. Übernehme Körper..."));
#endif
		dummy=clone_object("/obj/schatz");
		throw_out_ob->suspend_mudclient();
		exec(dummy,throw_out_ob);
		exec(throw_out_ob,this_object());
		touch(LEVEL_LISTER)->list_me(throw_out_ob, real_name);
		destruct(dummy);
		throw_out_ob->wieder_belebung(0);
		transfer_gmcp(throw_out_ob);
#ifdef RELAYD
		throw_out_ob->set_real_ip(real_ip_number,real_ip_name);
#endif
		remove();
		return 0;
	    }
	    else
	    {
		throw_out_ob->remove();
		if (throw_out_ob)
		    destruct(throw_out_ob);
	    }
	}
	else if (!playerp(throw_out_ob))
	{
	    throw_out_ob->remove();
	    if (throw_out_ob)
		destruct(throw_out_ob);
	}
    }
    
    return 1;
}

#ifdef __TLS__
// Liefert: -1: Fehler
//           0: Kein Zertifikat
//           1: Erfolgreicher Login
private int handle_tls_certificate(closure cont_cb)
{
    mixed cert = tls_check_certificate(this_object());
    string name;
    string msg, tmp;
    int res;

    if(cert && !cert[0])
    {
        for(int i=0;i<sizeof(cert[1]);i+=3)
            if(cert[1][i]=="2.5.4.3") // CommonName
            {
                name = cert[1][i+2];
                break;
            }
    }

    if (name)
        name = lower_case(name);

    if (!name || !valid_name(name))
        return 0;

#if __EFUN_DEFINED__(query_ip_name)
    if (msg = IP_FILTER->ip_filter(efun::query_ip_number(this_object()),name))
#else
    if (msg = IP_FILTER->ip_filter(efun::interactive_info(this_object(), II_IP_NUMBER),name))
#endif
    {
        log_filtered_player(name, msg);

        if(sizeof(msg) && msg[0]!='*')
            EXITV(wrap(msg), -1);
    }

#ifdef UNItopia
#  if __EFUN_DEFINED__(query_mud_port)
#    ifdef Orbit
    if (efun::query_mud_port()==9987)
#    else
    if (efun::query_mud_port()==3332)
#    endif
#  else
#    ifdef Orbit
    if (efun::interactive_info(this_object(), II_MUD_PORT)==9987)
#    else
    if (efun::interactive_info(this_object(), II_MUD_PORT)==3332)
#    endif
#  endif
    {
        wiz_shell = 1;
        write ("Wiz-Shell gewünscht.\n");
#ifdef PLAYER_DAY
        if (PLAYER_DAY)
        {
            EXITV("Das geht in diesem Monat nicht.\n", -1);
        }
#endif
    }
#endif

    res = restore_object(PLAYER_FILE(name));
    if (res == 0)
    {
        do_warning("Gültiges Zertifikat für neuen Spieler '"+name+"'!\n");
        return 0;
    }

    new_name = name;
    set_living_name("login:"+new_name);

    if (!check_existing_name())
        return -1;

    tmp = check_banished(new_name);
    if(stringp(tmp))
    {
        if(sizeof(tmp))
        {
            EXITV(tmp, -1);
        }
        else
        {
            no_welcome();
            return -1;
        }
    }

    real_name = new_name;

    if(check_throw_out())
        funcall(cont_cb);
    return 1;
}

protected void tls_finished()
{
    // Wird auch von Telnet aufgerufen.
    handle_tls_certificate(function void()
    {
        remove_input_to(this_object());
        do_continue();
    });
}
#endif

int logon()
{
    int admin;
    string wunsch, intro;

#ifdef SPERRUNG
    return sperrung();
#endif

    call_out("time_out",180);
    add_action("remove","ende");
    add_action("remove","quit");
    intro = INTRO;
    wunsch = WUNSCH;

#ifdef UNItopia
#if __EFUN_DEFINED__(query_mud_port)
    if (query_mud_port(-1) > 1 && query_mud_port(this_object()) == 13579)
#else
    if (sizeof(driver_info(DI_MUD_PORTS))>1 &&
        efun::interactive_info(this_object(), II_MUD_PORT) == 13579)
#endif
    {
#if __EFUN_DEFINED__(query_ip_number)
        string ip = efun::query_ip_number(this_object());
#else
        string ip = efun::interactive_info(this_object(), II_IP_NUMBER);
#endif
        // Nur Leute von localhost duerfen den letzten Port verwenden.
        if(member(({"127.0.0.1", "::1", "::ffff:127.0.0.1"}), ip) < 0)
	{
	    remove();
	    return 0;
	}

        admin = 1;
    }
#endif

#if 0
    cat("/InternetStreik");
    if (! admin)
    {
	call_out("time_out",10);
	return 0;
    }
#endif

    set_telnet(WILL, TELOPT_ECHO); // Telnet-Maschine starten.
    set_telnet(WONT, TELOPT_ECHO);
    set_telnet(WILL, TELOPT_EOR);  // Manche Clients antworten nicht auf
				   // TELOPT_ECHO, daher noch ein EOR.
    cat(intro);

#ifdef __TLS__
    if (tls_available() && tls_query_connection_info(this_object()) && handle_tls_certificate(#'do_continue))
        return 1;
#endif

    input_to("get_name", INPUT_PROMPT, wunsch);
    return 1;
}

#define ROOT_LOG "/var/adm/ROOT_COMMANDS"
private void entertain_root(string input)
{
    if (file_size(ROOT_LOG)>65000)
        rename(ROOT_LOG,ROOT_LOG".old");
    write_file(ROOT_LOG, sprintf("%s %-24s %s\n",
        shorttimestr(time()),
#if __EFUN_DEFINED__(query_ip_name)
       efun::query_ip_name(),
#else
       (efun::interactive(this_object()) ? efun::interactive_info(this_object(), II_IP_NAME) : "Non-Interactive"),
#endif
        input || ""));

    if (!sizeof(input))
        input_to(#'entertain_root, INPUT_PROMPT, "$ ");
    else
    {
        input_to(#'entertain_root);
        if (find_call_out(#'write) < 0)
            call_out(#'write, 5, "Device or resource busy.\n$ ");
    }
}

static int valid_name(string name)
{
    int i;
    string banished_reason;

    if (strlen(name) > 10 )
    {
	write("Sorry, Dein Name kann maximal 10 Buchstaben lang sein.\n");
	return 0;
    }
    for (i = 0; i < strlen(name); i++)
    {
	if (name[i] < 'a' || name[i] > 'z')
       	{
	   if(name[i]==' ')
	      write("Sorry, Dein Name darf keine Leerzeichen beinhalten.\n");
	   write("Sorry, Dein Name darf nur aus Kleinbuchstaben ohne Umlaute bestehen. (a-z)\n");
	   return 0;
       }
    }
    if (strlen(name) < 3 && !player_exists(name))
    {
	write("Sorry, Dein Name muss mindestens 3 Buchstaben lang sein.\n");
	return 0;
    }
    if (member(guests,name) >= 0)
    {
       write("Dieser Name ist für Gäste reserviert.\n");
       return 0;
    }
    if (BANISHD->query_banished(name,&banished_reason) <= 0)
    {
        log_banished_player(name);
        if (banished_reason)
	    write("Dieser Name ist reserviert:\n"+wrap(banished_reason));
        else
	    write("Dieser Name ist reserviert.\n");
	return 0;
    }
    return 1;
}

static void get_name(string str)
{
    string wiz_of_testplayer;
    string name, msg, s;
    int res;
    object *x;
    
    if (!str || str == "" || str == "?" || str == "hilfe")
    {
	if(!str || str == "")
	   count_empty_cmd++;
        else
	   count_empty_cmd = 0;
        if(count_empty_cmd > 1)
	{
	   EXIT("Bis bald in "+MUD_NAME+"!\n");
	}
        cat (HILFSTEXT);
        input_to ("get_name", INPUT_PROMPT, WUNSCH);
        return;
    }
#ifdef RELAYD
    if ((sscanf(str,"remoteip %s %s",real_ip_number,real_ip_name)==2) &&
		efun::query_ip_number(this_object())==RELAYD) {
	input_to("get_name");
	return;
	}
#endif

    name = lower_case(str);
    if (name == "ende" || name == "nd" || name == "exit" || name == "quit"
        || name == "bye" || name == "q") {
	EXIT("Bis bald in " + MUD_NAME + "!\n");
    }
    if (name == "wer" || name == "who") {
	count_empty_cmd = 0;
        x=users();
        res=sizeof(x);
        if (res) {
            x=filter(x, (: !IS_INVIS($1) &&
                           !($1->query_no_wer() &&
                             (wizp($1) || testplayerp($1))) :));
            res-=sizeof(x);
            s=liste(map(sort_array(x, (: to_string($1->query_name()) >
                                         to_string($2->query_name()) :)),
                    (: $1->query_cap_name() :)));
            if (res) {
                if (s=="")
                    s=to_string(res)+(res==1?" Unsichtbarer":" Unsichtbare");
                else
                    s+=" sowie "+to_string(res)+
                               (res==1?" Unsichtbarer":" Unsichtbare");
            }
            write ("\n"+wrap (s+"."));
        }
        else
            write ("\nLeider ist gerade niemand in "+MUD_NAME+".\n");
        write ("\n");
        input_to ("get_name", INPUT_PROMPT, WUNSCH);
        return;
    }

    if ((strstr (name,"f ") == 0) || (strstr (name, "finger ") == 0)) {
	count_empty_cmd = 0;
        if (name[0..1] == "f ")
	   name = name [2..];
        else if (name [0..6] == "finger ")
	   name = name [7..];
        write ("\n"+FINGER_OB->do_local_finger(name,0,
            FINGER_FLAG_OTHER_LOGIN)+"\n");
        input_to ("get_name", INPUT_PROMPT, WUNSCH);
        return;
    }
    if (strlen(name)>=10 && !strstr("datenschutzerklaerung", name))
    {
	more("/static/adm/DATENSCHUTZERKLAERUNG");
	return;
    }
    if (strlen(name)>=10 && !strstr("spielregeln", name))
    {
	more("/doc/hilfe/spielregeln.login");
	return;
    }

    if (name == "mssp-request")
    {
        write("\nMSSP-REPLY-START\n");
        send_mssp_vars(function void(string name, <string|string*> values)
        {
            write(name);
            foreach (string value: stringp(values) ? ({ values }) : values)
                write("\t" + value);
            write("\n");
        });
        write ("MSSP-REPLY-END\n");

        input_to ("get_name", INPUT_PROMPT, WUNSCH);
        return;
    }

    if (name[0] == '*') {
        name = name[1..];
        wiz_shell = 1;
        write ("Wiz-Shell gewünscht.\n");
#ifdef PLAYER_DAY
        if (PLAYER_DAY) {
            EXIT("Das geht in diesem Monat nicht.\n");
        }
#endif
    }

#ifdef PLAYER_DAY
    if ((PLAYER_DAY) && (testplayerp(name))) {
        EXIT("Zur Zeit sind keine Testspieler zugelassen.\n");
    }
#endif

    if (name == "root" || name == "admin")
    {
        input_to(function void(string str)
        {
            write("\nYou have unread mail in /var/mail/" + name + ".\n");
            input_to(#'entertain_root, INPUT_PROMPT, "$ ");
        }, INPUT_NOECHO|INPUT_PROMPT|INPUT_IGNORE_BANG, "Passwort: ");
        return;
    }

    if (!valid_name(name))
    {
        EXIT("Probiers doch mit einem anderen Namen nochmal.\n");
    }
#if __EFUN_DEFINED__(query_ip_name)
    if (msg = IP_FILTER->ip_filter(efun::query_ip_number(this_object()),name))
#else
    if (msg = IP_FILTER->ip_filter(efun::interactive_info(this_object(), II_IP_NUMBER),name))
#endif
    {
	log_filtered_player(name, msg);
	
	if(sizeof(msg) && msg[0]!='*')
	    EXIT(wrap(msg));
    }
    new_name = name;
    if (new_name == "gast")
    {
#ifdef Orbit
	EXIT("Sorry, Orbit ist den Göttern vorbehalten.\n");
#else
	seteuid(PLAYER_UID);
	new=2;
	input_to("guest_password", INPUT_PROMPT,
	    "Bist Du weiblich oder männlich (w/m)? ");
	return;
#endif
    }
    res = restore_object(PLAYER_FILE(new_name));
    set_living_name("login:"+new_name);
    if (res == 0)
    {
	// Wenn es das Playerfile nicht gibt,
	// sofort die Banish-Meldung ausgeben,
	// sonst erst nach dem Passwort fragen.
	if (BANISHD->query_banished(name) == 1)
	{
	   log_banished_player(name);
           no_welcome ();
           return; // no_welcome macht das remove, wenns soweit ist
	}

#ifdef Orbit
	if(!(wiz_of_testplayer=testplayerp(new_name)))
    	    EXIT("Sorry, Orbit ist den Göttern vorbehalten.\n");
	else // Nach PW des Gottes fragen.
	{
	    string init = save_object();
	    restore_object(PLAYER_FILE(wiz_of_testplayer));
	    input_to("check_wiz_password",
		INPUT_NOECHO|INPUT_PROMPT|INPUT_IGNORE_BANG,
		"Goetterpasswort: ",
		wiz_of_testplayer, name, init);
	    return;
	}
#else
	seteuid(PLAYER_UID);
	real_name = new_name;
	write("Ein neuer Spieler! Herzlich Willkommen!\n"+
		"Bitte jetzt ein beliebiges Passwort eingeben.\n");
	new=1;
	input_to("get_new_password",
	    INPUT_NOECHO|INPUT_PROMPT|INPUT_IGNORE_BANG,
	    "Passwort: ");
#endif
    }
    else if(SECOND->is_special(name))
    {
	input_to("get_special_wiz", INPUT_PROMPT, "Gott: ");
    }
    else if((wiz_of_testplayer=testplayerp(name)))
    {
	restore_object(PLAYER_FILE(wiz_of_testplayer));
	input_to("check_wiz_password",
	    INPUT_NOECHO|INPUT_PROMPT|INPUT_IGNORE_BANG,
	    "Goetterpasswort: ", wiz_of_testplayer, name, 0);
    }
    else
    {
	if(!check_existing_name())
	    return;
	
	input_to("get_password", INPUT_NOECHO|INPUT_PROMPT|INPUT_IGNORE_BANG,
	    "Passwort: ");
    }
}

static void get_special_wiz(string str)
{
    string group;
    
    if(!sizeof(str))
    {
        EXIT("Kein Name eingegeben.\nProbiers doch nochmal.\n");
    }
    
    str = lower_case(str);
    
    if(!GOETTER_REGISTER->is_wiz(str))
    {
        EXIT("Kein Gott.\n");
    }
    
    group = SECOND->is_special(new_name);
    if(!GROUP_MASTER->is_group_member(str, group))
    {
	EXIT("Falscher Name.\n");
    }

    restore_object(PLAYER_FILE(str));

    input_to("check_wiz_password",
	INPUT_NOECHO|INPUT_PROMPT|INPUT_IGNORE_BANG,
	"Goetterpasswort: ",
	str, new_name, 0);
}

static void check_wiz_password(string pass, string wizname, string name, string init)
{
    write("\n");
    if (pass == "")
    {
        EXIT("Kein Passwort eingegeben.\nProbiers doch nochmal.\n");
    }
    if (time()-FAILD->query_time_of_last_but_two_fail(wizname) < 300)
    {
        EXIT(wrap("In den letzten 5 Minuten waren mindestens 3 Versuche "
            "mit falschem Passwort, sich mit Deinem Namen einzuloggen. "
            "Daher ist der Login mit Deinem Namen jetzt 5 Minuten "
            "gesperrt. Wende Dich in dringenden Fällen an die Admins."));
    }
    if (!PASSWD_CHECK(pass, password))
    {
#if __EFUN_DEFINED__(query_ip_name)
	FAILD->add_fail(wizname, efun::query_ip_name());
#else
	FAILD->add_fail(wizname, efun::interactive_info(this_object(), II_IP_NAME));
#endif
        EXIT("Falsches Passwort.\n");
    }
    
    wiz_owner = wizname;

#ifdef Orbit
    if(init)
    {
        /* Neuer Spieler */
	restore_object(init);
        seteuid(PLAYER_UID);
        real_name = new_name;
        write("Dieser Testspieler wird in Orbit neu angelegt.\n");
        new=1;

	password = "*";
	remove_call_out("time_out");
	input_to("get_gender", INPUT_PROMPT, 
	    "Bist Du männlich, weiblich oder sonstiges (m/w/s): ");
    }
    else
#endif
    {
        restore_object(PLAYER_FILE(new_name));
        real_name = new_name;

	if(!check_existing_name())
	    return;

        if(check_throw_out())
	    do_continue();
    }
}

static void guest_password(string str)
{
    int i;

    string *choose;

    if (!str || ((str != "m") && (str != "w"))) {
       write ("\n");
       input_to ("guest_password", INPUT_PROMPT, 
    	    "Gib bitte \"w\" für weiblich oder \"m\" für männlich ein: ");
       return;
    } 
    remove_call_out("time_out");
    for(i = (str == "w") ? 0 : 1, choose = ({}); i < sizeof(guests); i+=2)
       if(!find_player(guests[i]) && !find_player("login:"+guests[i]))
	  choose += ({ guests[i] });
    if(sizeof(choose))
       real_name = choose[random(sizeof(choose))];
    else
    {
       EXIT("Alle Gästeplätze sind leider belegt!\n"
           "Probiers nachher nochmal.\n");
    }
    set_living_name("login:"+real_name);
    do_continue();
}

static void get_password(string pass)
{
    string tmp;

PRINT_EVALS("Start von get_password");

    write("\n");
    
    tmp = check_password(pass, new_name, &ins_passwd);
    
    if(stringp(tmp))
    {
	if(sizeof(tmp))
	{
	    EXIT(tmp);
	}
	else
	{
	    no_welcome();
	    return;
	}
    }
    real_name = new_name;

PRINT_EVALS("Nach Password und Banish-Check");

    if(check_throw_out())
	do_continue();
}

static int valid_password(string pass)
{
    string msg;
    int res;

    if(msg = passwd_msg(res = insecure_passwd(pass, real_name), PASSWD_ALL))
       write(msg+"\n");
    return !(res & PASSWD_ERRORS);
}	

static void get_new_password(string pass)
{
    write("\n");
    if (!valid_password(pass))
    {
        EXIT(0);
    }
    password = pass;
    input_to("verify_new_password",INPUT_NOECHO|INPUT_PROMPT|INPUT_IGNORE_BANG,
	"Tippe das Passwort zur Bestätigung nochmal ein: ");
}

static void verify_new_password(string pass)
{
    if (password != pass)
    {
	EXIT("\nFehler bei der Passwortbestätigung.\nProbiers doch nochmal.\n");
    }
    else
    {
	password = PASSWD_CRYPT(pass);
	remove_call_out("time_out");
	write("\n");
	input_to("get_gender", INPUT_PROMPT, 
	    "Bist Du männlich, weiblich oder sonstiges (m/w/s): ");
    }
}

static void get_gender(string g)
{
    g = lower_case(g);
    if (g == "m" || g == "maennlich" || g == "männlich")
    {
	gender="maennlich";
    }
    else if (g == "w" || g == "weiblich")
    {
	gender="weiblich";
    }
    else if (g == "s" || g == "sonstiges")
    {
	gender="saechlich";
    }
    else
    {
	write("\nEntschuldigung, aber sowas kommt mir nicht ins Spiel!\n");
	input_to("get_gender", INPUT_PROMPT,
		"Bist Du männlich, weiblich oder sonstiges (m/w/s): ");
	return;
    }
    write("\n");
    input_to("get_email", INPUT_PROMPT, 
	"Bitte gib Deine E-Mail-Addresse an (oder 'keine'): ");
}

static void get_email(string str)
{
    if (str && trim(lower_case(str))!="keine")
	mail_address=str;
    do_continue();
}

static void check_return()
{
   if(!pressed_return)
   {
      write("Weiter mit Return.\n");
      input_to("setup_player");
   }
   else
      setup_player();
}

static void wait_to_continue2()
{
PRINT_EVALS("Start von wait_to_continue2");
    ERROR_DB->check_errors(real_name);
    check_return();
}

private mixed limited_eval(closure cl, int evals, varargs mixed* arg)
{
    // Sprung ueber Unlimited machen, damit die verbrauchten
    // Evals nicht fuer den normalen Ablauf mitzaehlen.
    return limited( (: apply(#'limited, $1, ({$2}), $3) :),
            ({ LIMIT_UNLIMITED }), cl, evals, arg);
}

private int do_limited_move(object who, mixed wohin, int way, string in_msg)
{
    int res;
    string err;
    err = catch(res=who->move(wohin,([
        MOVE_FLAGS:way,
        MOVE_MSG_ENTER:in_msg,
        ]))

#if __VERSION__ > "3.3.560"
	; publish, reserve 10000
#endif
	);

    if(err)
    {
	do_error2("Fehler bei der Bewegung in diesen Raum:\n"+err[1..<1],wohin,wohin,0);
	return MOVE_NO_DEST;
    }
    else
	return res;
}

private int do_move(object who, mixed wohin, int way, string in_msg)
{
    return limited_eval(#'do_limited_move, MOVE_LIMIT, who, wohin, way, in_msg);
}

static void setup_player()
{
    mixed todesraum;
    object ob;
    string fails, ins_passwd_msg;
    object throw_out_ob, dummy;

PRINT_EVALS("Start von setup_player");

    if (new==1)
    {
	seteuid(ROOT_UID);
	save_object(PLAYER_FILE(real_name));
	seteuid(PLAYER_UID);
	ERROR_DB->delete_feedback(real_name,-1);
    }
    else if (new==0)
    {
	if(ins_passwd_msg = passwd_msg(ins_passwd, PASSWD_ERRORS))
	   write("\nDein Passwort ist UNSICHER! Grund:\n"+ins_passwd_msg+
		 "Ändere es mit dem 'passwort' Befehl.\n");
	if(fails = FAILD->query_fail_and_update(real_name))
	   write("\n"+fails);
	MAILD->neue_post(1);
    }

PRINT_EVALS("Nach PW- und Mailcheck");

    if (level < LVL_WIZ)
	seteuid(PLAYER_UID);
    else if (level == LVL_LEARNER)
	seteuid(LEARNER_UID);
    else
	seteuid(new_name);

    set_living_name("Login:"+real_name);
    throw_out_ob = find_player(real_name) || find_player("STATUE "+real_name)
    	|| find_player("login:"+real_name);
    set_living_name("login:"+real_name);
    if (throw_out_ob)
    {
	if (playerp(throw_out_ob))
	{
	    // Folgendes sollte nicht passieren,
	    // da ja schon oben bei get_passwd abgefangen!
	    if (interactive(throw_out_ob))
	    {
#if __EFUN_DEFINED__(query_ip_name)
		write(wrap("Du bist bereits am Spielen von " +
		    efun::query_ip_name(throw_out_ob)+" aus! Übernehme Körper..."));
#else
		write(wrap("Du bist bereits am Spielen von " +
		    efun::interactive_info(throw_out_ob, II_IP_NAME)+" aus! Übernehme Körper..."));
#endif
		sys_log("2INTER_THROW", sprintf("%s %O\n", shorttimestr(time()), 
					 throw_out_ob));
		dummy=clone_object("/obj/schatz");
		exec(dummy,throw_out_ob);
		touch(LEVEL_LISTER)->list_me(throw_out_ob, real_name);
		transfer_gmcp(throw_out_ob);
		destruct(dummy);
	    }
	    else if (query_living_name(throw_out_ob) == real_name)
            {
                write(wrap("Du warst zur Statue erstarrt"+
                ((last_host && "/apps/second"->is_special(real_name)==0)?
                    " (eingeloggt von "+last_host+" aus).":"")));
                write("Übernehme Körper...\n");
            }
	    exec(throw_out_ob,this_object());
	    touch(LEVEL_LISTER)->list_me(throw_out_ob, real_name);

	    // Kann sich seit dem letzten Abspeichern geaendert haben:
	    ghost = throw_out_ob->query_ghost();
	    hp = throw_out_ob->query_hp();

	    if (ghost > 0)
	    {
		throw_out_ob->open_con();
#ifdef MASSENMOERDER
                if (sizeof (opfer) >= MASSENMOERDER)
                    todesraum = "/room/death/death_room_massenmoerder";
                else
#endif
            
		if (todesraum = throw_out_ob->query_gilden_info(GILDEN_TOD))
		    catch(todesraum = touch(todesraum));
		if (!todesraum)
		    todesraum = "/room/death/death_room";
		throw_out_ob->set_ghost (ghost);
		do_move(throw_out_ob,todesraum,MOVE_SECRET, 0);
	    }
	    else if (ghost < 0 || !ghost && hp < 0) // letzteres sollte nicht passieren!
	    {
		if(!ghost)
		   throw_out_ob->set_ghost(-1);
		throw_out_ob->close_con();
		clone_object("/room/death/obj/death_shadow")->init_shadow(throw_out_ob, NO_MULTI_SHADOW);
	    }
	    if (throw_out_ob->query_invis() & V_ATOM_INVIS)
		tell_object(throw_out_ob,"\nVorsicht, Du bist noch unsichtbar.\n\n");
	    throw_out_ob->wieder_belebung(1);
#ifdef RELAYD
	    throw_out_ob->set_real_ip(real_ip_number,real_ip_name);
#endif
	    transfer_gmcp(throw_out_ob);
	    remove();
	    return;
	}
	else
	{
	    throw_out_ob->remove();
	    if (throw_out_ob)
		destruct(throw_out_ob);
	}
    }
    if(!new && last_host && "/apps/second"->is_special(real_name)==0)
        write("Du warst zuletzt eingeloggt:\n"
        "    von "+last_host+"\n"
        "     am "+timestr(last_login)+".\n\n");
#ifdef UNItopia
    if (member((["sissilein","gnomilein","testgott"]),real_name))
        ob=clone_object("/obj/newplayer");
    else
        ob=clone_object("/obj/player");
#else
    if (member((["testgott"]),real_name))//TODO myonara wieder rausschmeissen
        ob=clone_object("/obj/newplayer");
    else
        ob=clone_object("/obj/player");
#endif

PRINT_EVALS("player object cloned");
    exec(ob,this_object());
#ifdef RELAYD
    ob->set_real_ip(real_ip_number,real_ip_name);
#endif
    
    ob->setup_player();
    transfer_gmcp(ob);
    if (!ob)
    {
        remove();
        return;
    }

PRINT_EVALS("Nach ob->setup_player()");

    touch(LEVEL_LISTER)->new_enter(ob);

PRINT_EVALS("Nach dem Level-Lister");

    if(get_eval_cost() >= 100000)
       EVENT_MASTER->event("Login", ob);
    /*
    else
       sys_log("EVENT_LOGIN", shorttimestr(time())+": "+
	  ob->query_real_name()+" event login omitted.\n");
    */

PRINT_EVALS("Login-Events");

    if (new>0)
    {
	ob->set_level(LVL_PLAYER);
	ob->update_stats();
	ob->update_max_encumbrance();
	ob->update_max_hp();
	ob->set_hp(ob->query_max_hp());
	ob->update_max_sp();
	ob->set_sp(ob->query_max_sp());
	ob->set_material(({"biologisch"}));
	if(new == 2)
	{
	   gender = member(guests, real_name) % 2 ? "maennlich" : "weiblich";
	   // Damit die nicht gleich rumbruellen koennen.
	   ob->set_sp(0);
	}
	ob->set_gender(gender);
	if (gender == "weiblich")
	    ob->set_title(new==1?"die Neue":"die "+MUD_NAME+" Besucherin");
	else if (gender == "saechlich")
	    ob->set_title(new==1?"das Neue":"das "+MUD_NAME+" Besuchende");
	else
	    ob->set_title(new==1?"der Neue":"der "+MUD_NAME+" Besucher");
	ob->set_align(0);
	ob->set_weight(30);
	ob->set_whimpy(15);
	ob->set_msg_in("$Ein() nähert sich $dir().");
	ob->set_msg_out("$Der() entfernt sich $dir().");
	ob->set_mmsg_in("$Ein() erscheint in einer Rauchwolke.");
	ob->set_mmsg_out("$Der() verschwindet in einer Rauchwolke.");
	ob->add_skill_points(({"skill","offensiv","haende"}),0);
	ob->add_skill_points(({"skill","defensiv","schild","klein"}),0);
	ob->add_skill_points(({"skill","spiel"}),0);
	ob->add_skill_points(({"skill","raetsel"}),0);
	ob->set_fp(100);
	ob->set_wp(100);
	write("\nDu bist nun bekannt als: "+ob->query_short()+"\n\n");
    }
    else
    {
	// Gewicht zuruecksetzen, hilft gegen verbloedetes Pantheon
	//if(ob->query_weight() <= 1)
	ob->set_weight(30);

	ob->set_short(0);
	ob->set_long(0);

        if(sizeof(offline_opfer))
        {
	    ob->add_opfer_namen(map(offline_opfer,(:lower_case($1[0]):)));
            ob->send_message_to(ob,MT_NOTIFY,MA_UNKNOWN,
                wrap("Du hast " +
                liste(map(offline_opfer, // $1[0]: Cap-Name, $1[1]: Ursache im Akkusativ
            	    (: $1[0]+($1[1]?" durch "+$1[1]:"") :)))+
	        " umgebracht."));
        }

	if (ghost > 0)
	{
	    ob->open_con();
#ifdef MASSENMOERDER
            if (sizeof (opfer) >= MASSENMOERDER)
                todesraum = "/room/death/death_room_massenmoerder";
            else
#endif
	    if (todesraum = ob->query_gilden_info(GILDEN_TOD))
		catch(todesraum = touch(todesraum));
	    if (!todesraum)
	       	todesraum = "/room/death/death_room";
	
	    // Die Bewegung machen wir spaeter beim Startraum-Handling
#if 0
	    do_move(ob,todesraum,MOVE_SECRET,0);
	    remove();
	    return;
#endif
	}
	else if (ghost < 0 || !ghost && hp < 0) // letzteres sollte nicht passieren!
	{
	    if(!ghost)
	       ob->set_ghost(-1);
	    ob->close_con();
	    clone_object("/room/death/obj/death_shadow")->init_shadow(ob, NO_MULTI_SHADOW);
	}
        if (ob->query_invis() & V_ATOM_INVIS)
            tell_object (ob,"\nVorsicht, Du bist noch unsichtbar.\n\n");
    }

PRINT_EVALS("Update des Players");

/* --- Ermittlung des Startraums und Bewegung: --- */

#define START_DOMAIN     1
#define START_DEFAULT    2

    mixed start, room, tmp;
    string domain;
    int i;

    // Gegen mehrfaches Laden, mehrfachen Move, und Endlosrekursion bei
    // aufeinander gegenseitig oder sich selbst verweisenden Startraum-Typ.
    mapping done = ([:1]);
#define DONE_TOUCH     0b001
#define DONE_MOVE      0b010
#define DONE_STARTRAUM 0b100

    if(pointerp(start_room))
        start = start_room;
    else
        start = ({start_room});

    if(todesraum)
	start += ({todesraum});
    else
	start += ({START_DOMAIN});

    for(i = 0; i < sizeof(start); i++)
    {
        room = start[i];

        // Spezialfall 1: Domainstartraum
        if(room == START_DOMAIN)
        {
            start += ({START_DEFAULT});
            room = STARTRAUM_SERVER->query_startraum_of_domain(domain);
        }

        // Spezialfall 2: Defaultstartraum.
        else if(room == START_DEFAULT)
        {
            // Defaultstartraum verwenden.
            room = DEFAULT_START_ROOM;
        }

        // Raum/Objektname? Laden/Finden:
        if(stringp(room))
        {
            // Room ist "[prefix#]/pfad/zum/raum[#clonenummer]"
            // prefix ist notouch oder object_time()
            int t;
            string prefix;

            prefix = room[..(member(room, '/') - 2)];
            room = room[member(room, '/')..];
            t = to_int(prefix);

            // Vermeiden von mehrfachem Laden desselben Raumes:
            if(prefix == "notouch" || t || (done[room] & DONE_TOUCH))
            {
                room = find_object(room);

                // Gibt es ueberhaupt einen Raum?
                if(!room)
                    continue;

                // Ist es noch derselbe Blueprint?
                if(t && object_time(room) != t)
                    continue;
            }

            else
            {
                done[room] |= DONE_TOUCH;
PRINT_EVALS("Lade "+(room)+"...");
                if(catch(room = touch(room, NO_WRITE|NO_LOG); publish))
                {
PRINT_EVALS("Laden fehlgeschlagen");
                    continue;
                }
            }
        }

        // Haben wir ein Raumobjekt?
        if(objectp(room) && room->query_room())
        {
	    // Sind wir tot?
	    if(todesraum && room!=todesraum && !room->query_death_room())
	    {
PRINT_EVALS("Kein Todesraum.");
		continue;
	    }
	
            // Domain setzen.
            domain = room->query_room_domain();

            // Nicht in Pantheonsraeumen einloggen.
            if(domain == "Pantheon"
#ifndef PLAYER_DAY
               && level < LVL_WIZ && !testplayerp(ob)
#endif
              )
            {
PRINT_EVALS("Spieler im Pantheon");
                domain = "Vaniorh";
                continue;
            }

            // Vermeidung von mehrfachen moves.
            if(done[object_name(room)] & DONE_MOVE)
            {
PRINT_EVALS("Move in diesen Raum bereits bei vorherigem Versuch fehlgeschlagen.");
                continue;
            }

            // Vermeiden von Endlosrekursion bei Beachtung des Startraums
            if(!(done[object_name(room)] & DONE_STARTRAUM))
            {
                done[object_name(room)] |= DONE_STARTRAUM;

                // Definiert der Raum einen anderen Startraum?
                tmp = room->query_type("startraum");

                if(tmp)
                {
                    if(!pointerp(tmp))
                        tmp = ({tmp});

                    start[i+1..] = tmp + ({START_DOMAIN});
PRINT_EVALS("Raumtyp startraum gesetzt");
                    continue;
                }
            }

            // Darf man in dem Raum nicht starten?
            if(!todesraum && room->query_type("kein_startraum"))
            {
PRINT_EVALS("Raumtyp kein_startraum gesetzt");
                continue;
            }

            // Raumumgebung anhaengen als zusaetzliche Alternative.
            tmp = room->query_room_environment();

            if(sizeof(tmp))
            {
                tmp = filter(tmp, #'objectp);

                if(sizeof(tmp))
                {
                    start[i..i] = ({start[i]})+tmp;
                }
            }

            // Bewegung in den Raum versuchen.
            tmp = do_move(ob, room, todesraum?MOVE_SECRET:MOVE_MAGIC,
                          "$Der() betritt "+MUD_NAME+".");

            if(tmp == MOVE_OK)
            {
                // Fertig.
PRINT_EVALS("Spielerobjekt erfolgreich in Startraum bewegt");
                break;
            }

            // Vermeidung von mehrfachen Moves:
            done[object_name(room)] |= DONE_MOVE;

PRINT_EVALS("Move in "+object_name(room)+" fehlgeschlagen (Grund: "+tmp+")");
        }
    } /* for */

    // Wurden wir etwa nicht bewegt?
    if(!environment(ob))
    {
PRINT_EVALS("Das Spielerobjekt ließ sich nicht bewegen!");

        do_error("Schwerer Fehler beim Betreten von "+MUD_NAME+".\n");
        write("Schwerer Fehler beim Betreten von "+MUD_NAME+".\n");

        if(level < LVL_WIZ)
	{
            destruct(ob);
	    return;
	}
        else
            write("Du bist Gott ohne Environment.\n");
    }

/* --- Ende Bewegung in den Startraum. --- */

	/* Oldstyle Konvertierung */
        if(msgin) 
        {
            ob->set_msg_in("$Ein() "+msgin+" $dir().");
            ob->set_msg_out("$Der() "+msgout+" $dir().");
            ob->set_mmsg_in("$Ein() "+mmsgin+".");
            ob->set_mmsg_out("$Der() "+mmsgout+".");
        }
#ifdef PLAYER_NOTIFY_MODES
        ob->set_logout_via_start_or_stadt(logout_via_start_or_stadt);
        ob->do_normal_notify("login",NL_LOGON,ob->get_logon_mode());
#else
        ob->do_normal_notify("login",0);
#endif
#ifdef PLAYER_DAY
        if ((PLAYER_DAY) && (level >= LVL_WIZ)) {
           for (int b=0; b<4; b++)
               ob->set_one_stat(b,1);
           ob->update_stats();
           ob->update_max_hp();
           ob->update_max_sp();                       
        }
        if (PLAYER_DAY)
           ob->set_kein_verbrauch(0);
#endif

PRINT_EVALS("Login fertig.");
    remove();
}

#define WARN_INTERVAL 120  // mahnung alle zwei Minuten
#define WARNINGS 5         // Rauswurf nach WARN_INTERVAL*WARNINGS Sekunden

void warning(int warn)
{
   if(warn)
   {
      write("\aBitte <return>-Taste drücken! (Timeout in "+(WARN_INTERVAL*warn/60)+
	    " Minuten.)\n");
      call_out("warning", WARN_INTERVAL, warn-1);
   }
   else
      time_out();
}

int query_logged_in()
{
   return logged_in;
}

mixed query_read_artikel()
{
    return artikel;
}

void net_dead()
{
    remove();
}

#ifdef RELAYD
string query_real_ip_name() { return real_ip_name; }
string query_real_ip_number() { return real_ip_number; }
#endif


private void setup_wiz_shell()
{
   object ob;
   
   ob=clone_object("/obj/wizard_shell");

   exec(ob,this_object());
   ob->setup_wizard_shell(find_player(real_name));
   transfer_gmcp(ob);
   
   // sys_log("wiz_shell",
   //   left(ob->query_real_name(),15)+
   //   shorttimestr(time())+" "+efun::query_ip_name(ob)+"\n");
   ob->start_wizard_shell();
   return;
}

    
void no_welcome ()
{
    write("\n");
    if (!cat (NOWELCOME+new_name, nowelcome_line,22)) {
        if (nowelcome_line == 0) {
            write("Dein Charakter wurde verbannt.\n");
            nowelcome_line = 1;
            input_to ("no_welcome", INPUT_PROMPT, 
                   "Bitte <return> drücken: ");
            return;
        }
        remove ();
        return;
    }
    nowelcome_line += 22;
    write ("\n");
    input_to ("no_welcome", INPUT_PROMPT, 
	"Bitte <return> drücken: ");
}

private void send_gmcp(string package, string message, varargs mixed* data)
{
#if __EFUN_DEFINED__(json_serialize)
    string pkg = lower_case(package);
    if (pkg != "core") // && !member(packages, pkg)
        return;

    efun::binary_message( ({ IAC, SB, TELOPT_GMCP }), 1 );
    string msg = package + "." + message;
    if (sizeof(data))
        msg += " " + json_serialize(data[0]);
#if __VERSION__ > "3.5.2"
    efun::binary_message(to_bytes(msg, "UTF-8"), 1);
#else
    efun::binary_message(msg, 1);
#endif
    // log_gmcp("Sending: " + msg);
    efun::binary_message( ({ IAC, SE }), 1 );
#endif
}

nosave string* gmcp_messages = ({});
static void receive_gmcp(string data)
{
#if __EFUN_DEFINED__(json_parse)
    string* words = explode(data, " ");
    mixed args;
    mixed temp;

    if (sizeof(words) > 1)
        args = json_parse(implode(words[1..], " "));

    switch(lower_case(words[0]))
    {
        // Modul Core
        case "core.ping":
            send_gmcp("Core", "Ping");
            return; // no buffering.
        case "char.login": // TODO implement gmcp login
            // name, password, token, wizname
            return; // no buffering
        default:
            break;
    }
#endif
    gmcp_messages += ({ data });
}

private void transfer_gmcp(object player)
{
    player->transfer_gmcp(gmcp_messages);
}
