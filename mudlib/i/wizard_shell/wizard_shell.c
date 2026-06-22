// This file is part of Avalon Mudlib
// ----------------------------------------------------------------
// File:        /i/wizard_shell/wizard_shell.c
// Author:      Avatar (10/96)
// Description: Eine Wizard-Shell
// Modified:    Jones & Sissi: (18.6.97ff): Anpassung an UniLib
//		Freaky (25.06.97) Bugfix in let_not_in()
//              Sissi  (02.07.97) Newsreader und Mailreader bekommen jetzt
//                                die Autoload - Parameter gesetzt, dafuer
//                                sind die ueberfluessigen "catch"
//                                rausgeflogen.
//              Sissi  (07.08.97) more_chunk des Gottes wird uebernommen,
//                                remove nach MAX_IDLE (3 Stunden) idlezeit
//		Freaky (12.12.1999) set_{path_}aliases() eingebaut.

#pragma save_types
#pragma strong_types

inherit "/i/item";
inherit "/i/contain";
inherit "/i/tools/pipe";
inherit "/i/tools/getopt";
inherit "/i/tools/browser";
inherit "/i/tools/dynamic_browser";
inherit "/i/player/wiz_soul";
inherit "/i/player/files";
inherit "/i/player/gmcp";
inherit "/i/player/more";
inherit "/i/player/tippse";
inherit "/i/player/telnet_neg";
inherit "/i/player/input_to";
inherit "/i/player/vt100client";
inherit "/i/xeditor/xeditor";
inherit "/i/player/webmud";
inherit "/i/player/webmud3";
protected functions inherit "/i/tools/security";
private functions inherit "/i/tools/top";

#define MAX_IDLE 10800
#define NEWS "\nEs gibt Probleme, wenn waehrend des Lesens der Post mit der\n" \
     "Wiz - Shell neue Post eintrifft. Bis diese Probleme behoben\n" \
     "sind, geht daher das Lesen der Post mit der Wiz - Shell nicht.\n" \
     "Sissi.\n\n"

#include <config.h>
#include <quest.h>
#include <gilden.h>
#include <game.h>
#include <error.h>
#include <more.h>
#include <move.h>
#include <level.h>
#include <invis.h>
#include <apps.h>
#include <erq.h>
#include <getopt.h>
#include <parse_com.h>
#include <error_db.h>
#include <browser.h>
#include <dynamic_browser.h>
#include <files.h>
#include <mail.h>
#include <pipe.h>
#include <message.h>
#include <math.h>
#include <simul_efuns.h>
#include <regexp.h>
#include <input_to.h>

#include "/i/player/player.h"

#define WIZARD_SHELL_VERSION "1.1 fuer UNItopia"
#define ME (object_name(this_object()) == MY_NAME)
#define XEDITOR "/obj/xeditor"
#define XEDITOR_SHADOW "/obj/editor_shadow"

#undef LOW

// this_object()
private object owner;
// private object xeditor;

// Objekt-Pointer auf den eingeloggten Programmierer.
// Kann sich aendern, wenn sich derdiedasjenige zwischendurch neu einloggt.
private object master;

// Real-Name des Programmierers
private string master_name;

// Wann wurde die Shell gestartet?
private int login_time;

// Ist != '0', wenn der Container Objekte aufnehmen darf.
// Sollte nur in init_wizard_shell() verwendung finden
private int container_open;

// Wenn dieses Objekt ein Blueprint ist, steht hier der Inhalt der Manpage
private static mixed cmd_manpage;

// Telnet-Pings...
private int telnet_ping;
private nosave int telnet_ping_counter;
private int client_options;

static nomask int query_client_option(int opt)
{
    switch(opt)
    {
	case CLIENT_VT100:
	    return client_options & CLIENT_VT100_WIZSHELL;
	
	case CLIENT_NO_EOR:
	    return client_options & opt;
    }
    
    return 0;
}

void receive_message_low(string str)
{
    if(!sizeof(str))
	return;

    if(query_client_option(CLIENT_VT100))
        vt100client::receive_message_low(str);
    else if(interactive())
        efun::tell_object(this_object(),str);
}

void receive_message(int msg_type, int msg_action, object who, string msg)
{
    if (stringp(msg))
	receive_message_low(msg);
}

void print_prompt(mixed prompt)
{
    if(!query_client_option(CLIENT_VT100) ||
       !vt100client::print_prompt(prompt))
	telnet_neg::print_prompt(prompt);
}

nomask int start_input_to(closure callback, mixed prompt, int flags)
{
    if(this_player()!=this_object())
        return 0;

    if(query_client_option(CLIENT_VT100) && uses_vt100client())
        return input_to::start_input_to(callback, prompt, flags);
}

protected string calc_trenner_info()
{
    return sprintf("_____%s___(Wiz-Shell)___",
        this_object()->query_real_cap_name() ||
	capitalize(this_object()->query_real_name()));
}

void receive_notify_fail(string msg, object msgobj, object orig_cmd_giver)
{
    this_object()->receive_message(MT_NOTIFY,MA_UNKNOWN,
        msgobj || this_object(),msg);
}
		
int query_prevent_shadow(object shadow)
{
   return !strstr(object_name(shadow),XEDITOR_SHADOW);
}


// Eine Abfrage nach "wizardshellp()" ist sicherer
int query_wizard_shell()               { return 1; }

// Wann hat sich der User eingeloggt?
int query_login_time()                 { return login_time; }

// Objekt-Pointer auf den Master (Vorsicht: wenn er sich zwischendurch
// ausgeloggt hat, ist 'master' solange Null, bis update_master()
// aufgerufen wurde
object query_master()                  { return master; }

// Name des Masters
string query_master_name()             { return master_name; }


// Predefines
private void define_basic_commands(int flag);

#include "/i/wizard_shell/shell.inc"
#include "/i/wizard_shell/vplayer.inc"
#include "/i/wizard_shell/commands.inc"

// Liefert im Blueprint die Manpage zurueck
string query_cmd_manpage() { return cmd_manpage; }
 
private void begin_cmd_manpage()
{
   cmd_manpage=([]);
}
 
private void end_cmd_manpage()
{
   int i;
   string mp, *ind;
   
   for(mp="",i=sizeof(ind=sort_array(m_indices(cmd_manpage),#'<));i--;)
      mp+="\n"+ind[i]+"\n"+copies("~",strlen(ind[i]))+"\n"+
         implode(sort_array(cmd_manpage[ind[i]],#'>),"\n")+"\n";
 
   cmd_manpage=LINE+"\n"+
      "Eine Liste aller Kommandos der Wizard-Shell.\n\n"+
      mp+"\n"+
      "Hilfe zum Kommando gibt es immer mit '<Befehl> ?'.\n"+LINE[0..<2];
}

// zetup plugins-Befehl
private string add_plugin(string file, mixed args)
{
    return "Plugins sind auf der Wiz-Shell nicht verfügbar.\n";
}


private varargs void add_cmd(string fun, string action, int flag,
   string key, string cmd, string desc)
{
   if( ME ) // Avatar: Manpage liegt lediglich im Blueprint!
   {
      if( key && cmd && desc )
         cmd_manpage[key]=(cmd_manpage[key]||({}))+
            ({ sprintf("%:-15s %s",cmd,desc) });
   }
   else
      if (fun != "dummy")
         add_action(fun,action,flag);
}
 

// Kommandos definieren
private void define_basic_commands(int flag) 
{
   if( !flag )
      begin_cmd_manpage();

   add_cmd("who_command",       "wer", 0, "Basisbefehle",  "wer",
      "wie \"volk\"");
   add_cmd("who_command",       "volk", 0, "Basisbefehle", "volk",
      "Zeigt an, wer gerade eine Wizard-Shell benutzt");
   add_cmd("kill_command",      "kill",0, "Basisbefehle",  "kill",
      "Hiermit 'killt' man eigene Shells");
   add_cmd("quit_wizard_shell", "exit",0, "Basisbefehle",  "exit",
         "Verlässt die Wizard-Shell");
   add_cmd("quit_wizard_shell", "quit");
   add_cmd("alias",       "alias",     1, "Basisbefehle",  "kürzel",
      "Kuerzelelei (Aliase)");
   add_cmd("history",	  "puffer",    1, "Basisbefehle",  "puffer",
      "Historyfunktion");
#if 0
   add_cmd("trust_command","vertrau",  1, "Basisbefehle",  "vertrau[e]",
      "Einem Tool vertrauen schenken");
#endif
   add_cmd("pk",          "pk",        0, "Basisbefehle", "Files", "pk",
      "Pfadkürzel");
   add_cmd("cd",          "cd",        0, "Files",         "cd",
      "Verzeichnis wechseln");
   add_cmd("pwd",         "pwd",       0, "Files",         "pwd",
      "Aktuelles Verzeichnis anzeigen");
   add_cmd("f_tail",      "tail",      0, "Files",         "tail",
      "Liefert die letzten Zeilen eines Files.");
   add_cmd("f_cat",        "cat",       0, "Files",         "cat",
      "Liefert dem kompletten Inhalt eines Files.");
   add_cmd("f_more",       "more",      0, "Files",         "more",
      "Gibt den Inhalt eines Files aus.");
   add_cmd("edit",         "ed",        0, "Files",         "ed",
      "Editieren eines Files.");
   add_cmd("f_mkdir",      "mkdir",     0, "Files",
"mkdir",
      "Verzeichnis erstellen.");
   add_cmd("f_rmdir",      "rmdir",     0, "Files",         "rmdir",
      "Verzeichnis löschen.");
   add_cmd("f_ls",         "ls",        0, "Files",         "ls",
      "Liefert den Inhalt des akt. Verz.");
   add_cmd("f_rm",         "rm",        0, "Files",         "rm",
      "Datei löschen.");
   add_cmd("f_cp",          "cp",        0, "Files",         "cp",
      "Datei kopieren.");
   add_cmd("f_mv",          "mv",        0, "Files",         "mv",
      "Datei verschieben.");
   add_cmd("f_grep",        "grep",      0, "Files",         "grep",
      "Fileinhalt untersuchen.");
   add_cmd("f_find",        "find",      0, "Files",         "find",
      "Sucht nach Dateien in einem Verzeichnis.");
   //add_cmd("dummy",      "xed",         1, "Files",        "xed[it]",
    //  "Komfortabler Bildschirmeditor.");
   add_cmd("f_xed",      "xed",         1, "Files",        "xed[it]",
      "Komfortabler Bildschirmeditor.");
   add_cmd("w_ed",      "wed", 1, "Files", "wed[it]",
      "Komforatbler Editor im Webmud, falls genutzt.");
   add_cmd("z_fehler",    "zfe",       1, "Sonstiges",     "zfe[hler]",
      "Fehlermeldungen ansehen.");
   add_cmd("z_acl",	  "zacl",      0, "Sonstiges",	   "zacl",
      "ACLs verwalten.");
   add_cmd("z_gruppe",	  "zgruppe",   0, "Sonstiges",	   "zgruppe",
      "Gruppen verwalten.");
   add_cmd("z_top",	  "ztop",      0, "Sonstiges",	   "ztop",
      "UID-Aktivitäten anzeigen.");
   add_cmd("dummy",       "news",      0, "Sonstiges",     "lies news",
      "Blubberblaettle lesen.");
   add_cmd("dummy",       "mail",      0, "Sonstiges",     "lies mail",
      "Post lesen.");
   add_cmd("snoop_on",    "beobachte", 0, "Sonstiges",     "beobachte",
      "Spieler beobachten.");
   add_cmd("dummy",       "menü",     0, "Dokumentation", "menü",
      "Enzyclopedia - Menü.");
   add_cmd("dummy",       "?",         0, "Dokumentation", "? <funktion>",
      "Infos über eine Funktion abfragen.");
   add_cmd("dummy",       "dek",       1,   "Dokumentation", "dek[laration]",
      "Deklaration abfragen, nach Fun. suchen.");


   if( !flag )
      end_cmd_manpage();
   else
   {
      // "aliases"
      add_action("manpage", "hilfe");
      add_action("manpage", "help");
      add_action("who_command", "who");
      add_action("quit_wizard_shell", "ende");
      add_action("quit_wizard_shell", "ende");
      add_action("alias", "kürzel",-3);
#if 0 // Wozu auch?
      add_action("trust_command", "vertraue",-7);
      add_action("trust_command", "trust");
#endif
 
      // fuer "lies news" und "lies mail"... jaja, diese Gewohnheitstiere
      add_action("reading","lies");
      add_action("reading","lese",-3);
   }
}

void create() 
{
   if( owner )
      return;
   owner = this_object();
   set_name("wizard-shell");
   set_cap_name("Wizard-Shell");
   set_id( ({"shell", "wizard", "wizardshell", "wizard-shell" }) );
   set_gender("weiblich");
   seteuid(getuid());
   if( ME ) {
        define_basic_commands(0);
   }
   init_security_for_actions();
   "*"::create();
}

int remove()
{
   if(query_master_name())
       PLAYER_SECOND->logout(query_master_name(), this_object());

   if(interactive() && query_client_option(CLIENT_VT100))
       stop_mudclient();

   destruct(this_object());
   return 1;
}

void setup_wizard_shell(object master)
{
   object eigener, goettlicher;
   if( explode(object_name(previous_object()),"#")[0] != LOGIN_OB )
   {
      write("Illegal Call.\n");
      remove();
      return;
   }

   if( !playerp(master) || 
       !gesellep(master) ||
       !(master_name=master->query_real_name()) )
   {
      write("Eigentümer nicht (mehr) eingeloggt, oder kein Geselle.\n");
      remove();
      return;
   }

   update_master();
   set_name(get_genitiv(master_name)+" wizard-shell");
   set_cap_name(get_genitiv(master_name)+" Wizard-Shell");
   set_personal(1);
   enable_commands();
   init_current_path();
   init_aliases();
   set_prompt(lambda(({}), ({ #'+,
        ({symbol_function("query_current_path",this_object())}), "> "})),
        this_object());

   define_basic_commands(1);
   init_command_shell();

   set_aliases(master->query_aliases());
   set_path_aliases(master->query_path_aliases());
   login_time=time();
   set_more_chunk(master->query_more_chunk());

   // Hier werden zusaetzliche Tools eingefuegt
   container_open=1;
   goettlicher = present_clone("/obj/newsreader",master);
   if(goettlicher && strstr(load_name(goettlicher),"/obj/newsreader"))
       goettlicher = 0;
   (eigener=clone_object(goettlicher?load_name(goettlicher):"/obj/newsreader"))->move(this_object());
   if (eigener && goettlicher)
      eigener->init_arg(goettlicher->query_auto_load());
/*
   (eigener=clone_object("/obj/mailreader"))->move(this_object());
   if (eigener && goettlicher=present("mailreader",master))
      eigener->init_arg(goettlicher->query_auto_load());
*/
   clone_object("/obj/enzyclopedia")->move(this_object());
   /*
   catch( xeditor = clone_object(XEDITOR) );
   if (xeditor)
   {
       xeditor->move(this_object());
       xeditor->init_xeditor(owner);
   }
   */
   xeditor::init_xeditor(owner);
   container_open=0;
   params=master->query_zauberstab_info()||([]);


   telnet_ping = master->query_telnet_ping();
   if(telnet_ping)
      set_heart_beat(1);
   client_options = master->query_client_options();
   if(query_client_option(CLIENT_VT100))
      start_mudclient();                                                                                                                                                   
   telnet_neg::set_eor_protokoll(!query_client_option(CLIENT_NO_EOR));
   
    init_webmud3();
	init_gmcp();
    call_out( function void () {
        process_gmcp(([ // Char.Name initially. (wizard)
            "name":query_real_cap_name(),
            "fullname":query_short(this_object()),
            "gender":query_real_gender(),
            "wizard":wizp(this_object()),
            ]),"Char","Name"); // Explain fields
        },2);
}

void heart_beat()
{
    if(query_client_option(CLIENT_VT100) && interactive())
        vt100client::heart_beat();

    telnet_ping_counter+=2;
    
    if(telnet_ping_counter>=telnet_ping)
    {
	telnet_ping_counter=0;
	send_telopt_tm();
    }
}

void start_wizard_shell()
{
   int i, all_users, my_users;
   object *usrs;
   
   if( explode(object_name(previous_object()),"#")[0] != LOGIN_OB )
   {
      write("Illegal Call.\n");
      remove();
      return;
   }

   write("\n\nWizard Shell Version "+WIZARD_SHELL_VERSION+"\n"+
      MUD_NAME+"#"+__VERSION__+"\n");
   write (NEWS);

   for(all_users=my_users=0,i=sizeof(usrs=efun::users());i--;)
      if( !strstr(object_name(usrs[i]),WIZARD_SHELL_OB) )
      {
         ++all_users;
         if( usrs[i]->query_master_name() == master_name )
            ++my_users;
      }

   write("Es "+(all_users == 1 ? "ist" : "sind")+" insgesamt "+
      all_users+" Benutzer eingeloggt.\n");
   if( my_users > 1 )
      write("Dies ist Deine "+my_users+". Wizard-Shell.\n");
}

int quit_wizard_shell()
{
   write("Wizard-Shell verlassen.\n");
   return remove();
}

void reset ()
{
   if(clonep(this_object()) && 
      (!interactive(this_object()) || (query_idle(this_object()) > MAX_IDLE)))
   {
       remove();
   }
}

// Security... verhindert, dass die Adeligen ihre Shells mit "illegalen"
// Tools ausstatten
<int|string> let_not_in(mapping mv_infos)
{
   if( mv_infos[MOVE_OBJECT] 
            && !strstr(object_name(mv_infos[MOVE_OBJECT]),XEDITOR "#") )
      return 0;
   if( !container_open )
      return 1;
}
<int|string> let_not_out(mapping mv_infos)
{
   if( !container_open )
      return 1;
}

int manpage(string str)
{
   BEGIN_PIPE(str);

   if( !strlen(str=lower_case(str||"")) )
   {
      if( !(str=({string})MY_NAME->query_cmd_manpage()) &&
          !MY_NAME->define_basic_commands(0) &&
          !(str=({string})MY_NAME->query_cmd_manpage()) )
      {
         write("Seite nicht verfügbar.\n");
         return 1;
      }
      W(str);
   }
   else
      if( OWN->more(HILFE_PFAD+str,0,0,M_AUTO_END) ) {
         notify_fail("Zu "+str+" gibt's keine Hilfe.\n");
         return 0;
      }
   PRINT;
   return 1;
}

// Avatar: Damit "lies news" und "lies mails" funktioniert
int reading(string str)
{
   string text;
   mixed *parsed, ob;

   parsed = parse_com(str);
   if( parse_com_error(parsed,"Lies was?\n",1) )
      return 0;

   ob=parsed[PARSE_OBS][0];

   if( mappingp(ob) )
   {
      if( closurep(ob["read"]) )
         text=funcall(ob["read"],parsed[PARSE_REST],str);
      else
         text=ob["read"];
   }
   else
      text=ob->query_read(parsed[PARSE_REST],str,this_object());

   if( !text )
   {
      write("Da gibt es nichts zu lesen.\n");
      return 1;
   }

   if( strlen(text) )
       receive_message(MT_UNKNOWN,MA_UNKNOWN,this_object(),text);

   return 1;
}

static int edit(string str)
{
    SECURE;
    HELP("ed");
    if (!str) {
        /* request for file with last error */
        mixed *error;
        error=get_error_file(({string})OWN->query_real_name());
        if (!error || error[3]) {
            write("Kein Fehler.\n");
            return 1;
            }
        str=error[0];
        write(str+" Zeile "+error[1]+": "+error[2]+"\n");
        }
    else
        str=ADD_PATH(str);

    if (file_size(str)!=-2)
        own_ed(str);
    else
        write("Ein Verzeichnis darf man nicht Editieren!\n");
    return 1;
}
