// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /i/player/tippse.c
// Description: Tippse (Aliases und Kommandovervollstaendigung)
// Author:        Freaky (03.09.92)
// Modified by:        Freaky (23.12.93)
//                Freaky (18.01.95) tue kann kein 'weiter' mehr
//              Sysop  (10.95)    neue Funktionen der History:
//                                  %<Anfangsbuchstaben>
//                                  ^<suchtext>^<ersatz>
//
//              Sissi (3.1.96)    Skriptspieler werden zu Statuen
//                Freaky (05.01.96) TIME_SLICE usw eingebaut
//                Freaky (19.08.96) aliases ist jetzt ein Mapping
//                Monty (10.10 96)  in modify_command Logging fuer absolute
//                                  Haertefaelle im Pantheon eingebaut
//              Sissi (20.12.96)  Namenskuerzel von Sysop eingebaut
//                Monty (21.03 97)  Logging verschaerft: das Environment wird
//                                  mitgeloggt.
//              Sysop  (8.98)     Verhinderung von Endlos-tue's
//              Sysop  (9.6.99)   Parameter $1*..$9* bei Kuerzeln
//              Sysop  (21.6.99)  Umlaut-Umwandlung
//              Parsec (28.10.99) Namenskuerzel: mehrere name#, 'name#,
//                                Fehlerausgabe, no_wer nur bei entfernten,
//                                name# expandiert auch vorn Trennzeichen
//                Freaky (30.11.1999) \b bei filter_string rausgenommen
//                Freaky (24.02.2000) filter_string nach convert_umlaute
//              Parsec (16.03.2000) kuerzel xx zeigt Kuerzel / neu: unalias
//              Parsec (13.06.2000) \ fuer Kommandos ohne Kuerzelexpansion


#pragma save_types
#pragma strong_types

inherit "/i/player/delay";

#include <commands.h>
#include <more.h>
#include <level.h>
#include <invis.h>
#include <delayed_action.h>
#include <config.h>
#include <configuration.h>
#include <message.h>
#include <event.h>
#include <editor.h>
#include <input_to.h>
#include <files.h>
#include <rtlimits.h>
#include <notify_fail.h>
#include <portal.h>
#include <room.h>
#include "player.h"

#define DEBUGGER "myonara"
#include <debug.h>

#define MAX_HIST 50
#define MAX_ALIASES 1000
#define COMMANDS_PER_TIME_SLICE 30
#define COMMANDS_PER_TIME_SLICE_WARNING 20
#define TIME_SLICE 10
#define FAIL(x) return notify_fail(x)
#define FAILW(x) return write(x), 0

// Protos
string query_real_name();
public string gmcp_edit_get_key(string tempfile,string key);
varargs mapping gmcp_start_edit(closure retfun,string title,
    string tempfile,string* text);

// ({ Uhrzeit, expandierter Befehl, Originalbefehl })
private nosave mixed *hist=({ ({0,0,0}) }) * MAX_HIST;
private nosave mapping std_aliases=([
        "n"  : "norden",        "no" : "nordosten",
        "s"  : "süden",         "so" : "südosten",
        "o"  : "osten",         "nw" : "nordwesten",
        "w"  : "westen",        "sw" : "südwesten",
        "h"  : "hoch",          "r"  : "runter",
        "a"  : "ausgang",
        "sa" : "betrachte",
        "sc" : "schau",
        "b"  : "betrachte",
        "m"  : "mustere",
        "v"  : "verkaufe",
        "f"  : "führe",
        "se" : "senke",
        "t"  : "töte",
        "le" : "lege",
        "ni" : "nimm",
        "na" : "nimm alles",
        "li" : "lies",
        "i"  : "ausr",
        "tm" : "rede",
        ]);
private string tue_trenner;
private nosave string comm;
private nosave int hist_offset = -MAX_HIST;
private nosave int hist_pos, last_time_slice, command_count;
private nosave int command_per_slice_warned;
private mixed aliases = ([]);
private string prompt = "> ";
private string *log_commands = 0;
private nosave int doing;
private nosave int login_command_executed;
// so wird schummeln effektiv ausgeschlossen

void set_tue_trenner (string s)
{
    if (previous_object() != this_object()) return;
    tue_trenner = s;
}

string query_tue_trenner ()
{
    return tue_trenner || ",";
}

private string get_old_command(string cmd)
{
    string rest;
    int nr, pos, i;

    if (strlen(cmd) && cmd[0]=='%')
    {
        if(!hist[hist_pos][1])
            FAILW("Ich kann leider keinen passenden Befehl im Puffer finden!\n");
        if (cmd=="%")
            return hist[hist_pos][1];
        nr=1; i=1;
        while(cmd[nr..nr+1]=="\\b")
            nr+=2,i++;
        return hist[hist_pos][1][0..<i]+cmd[nr..];
    }

    if (!strlen(cmd) || ((cmd[0]<'0' || cmd[0]>'9') && cmd[0]!='-'))
    {
        for (i=hist_pos; i>=0; i--)
            if (hist[i][1] && strstr(hist[i][1], cmd, 0)==0)
                return hist[i][1];
        for (i=MAX_HIST-1; i>hist_pos; i--)
            if (hist[i][1] && strstr(hist[i][1], cmd, 0)==0)
                return hist[i][1];
        FAILW("Ich kann leider keinen passenden Befehl im Puffer finden!\n");
    }

    sscanf(cmd,"%d%s",nr,rest);
    if (nr<0)
        nr+=hist_offset+MAX_HIST;
    if (nr<=0 || nr<=hist_offset || nr>MAX_HIST+hist_offset)
        FAILW("Einen Befehl mit dieser Nummer gibt es im Puffer nicht!\n");

    pos=nr+hist_pos-hist_offset;
    if (pos>=MAX_HIST)
        pos-=MAX_HIST;
    if (rest)
    {
        nr=0;
        i=1;
        while(rest[nr..nr+1]=="\\b")
            nr+=2,i++;
        return hist[pos][1][0..<i]+rest[nr..];
    }
    return hist[pos][1];
}

private string edit_old_command(string cmd)
{
    string *kommando, *teile;
    string ergebnis;
    int i;

    kommando=explode(cmd,"^");
    if (sizeof(kommando)==1)
        kommando += ({""});

    cmd=kommando[0];
    if (cmd=="")
        return 0;

    i=hist_pos;
    ergebnis=0;
    while( i>=0 && !ergebnis )
    {
        if (hist[i--][1] && strstr(hist[i+1][1], cmd, 0) >= 0)
            ergebnis=hist[i+1][1];
    }
    i=MAX_HIST-1;
    while( i>hist_pos && !ergebnis )
    {
        if (hist[i--][1] && strstr(hist[i+1][1], cmd, 0) >= 0)
            ergebnis=hist[i+1][1];
    }

    if (!ergebnis)
        FAILW("Ich kann leider keinen passenden Befehl im Puffer finden!\n");

    teile = explode(ergebnis,cmd);

    ergebnis=teile[0]+kommando[1]+implode(teile[1..<1],cmd);
    if (sizeof(kommando)>2)
        ergebnis+=kommando[2];

    if (ergebnis=="")
        FAILW("Ich kann leider keinen passenden Befehl im Puffer finden!\n");
    return ergebnis;
}

/*

private string insert_cmd(string cmd, string str)
{
    string st, *stra, *strs, ret;
    int max, nr, i;

    if (sscanf(cmd,"%s$%d%s",ret,nr,st) < 2)
        return cmd+" "+str;
    if (nr==0)
        return ret+str+st;
    stra=explode(str," ");
    max=sizeof(stra);
    strs=regexplode(cmd,"\\$[1-9]");
    ret="";
    for (i=1; i<sizeof(strs); i+=2)
    {
        nr=strs[i][1]-49;
        if (nr>=0 && nr<max)
            ret+=strs[i-1]+stra[nr];
        else
            ret+=strs[i-1][0]==' '?strs[i-1][1..]:strs[i-1];
    }
    return ret+strs[i-1];
}
*/

private string insert_cmd(string cmd, string str) {
    string *stra,*strs,ret,ersetz;
    int max,nr,i;

    if (!cmd) return 0;
    strs=regexplode(cmd,"[&$][0-9]\\*|[&$][0-9*]");
    
    for(i=0;i<sizeof(strs)-2;i+=2)
        if(strlen(strs[i]) && strs[i][<1]=='\\')
        {
            strs=strs[0..i-1]+({strs[i][0..<2]+strs[i+1]+strs[i+2]})+strs[i+3..<1];
            i-=2;
        }
        
    if (sizeof(strs)<=1)
        return strs[0]+(str=="" ? "":" ")+str;

    if (str=="")
        stra=({});
    else
        stra=explode(str," ");
    max=sizeof(stra);
    ret="";
    for (i=1; i<sizeof(strs); i+=2) {
        nr=strs[i][1]-49;
        ret+=strs[i-1];
        if (nr>=0 && nr<max)
            ersetz=((strlen(strs[i])>2 && strs[i][2]=='*') ?
                        implode(stra[nr..]," ") : stra[nr]);
        else if (nr<0)
            ersetz=str;
        else
            ersetz="";
        if (ersetz=="" && ret!="" && ret[<1]==' ')
            ret=ret[0..<2];
        ret+=ersetz;
    }
    return ret+strs[i-1];
}

protected void add_history(string str, string orig)
{
    // no duplicates in history
    if (str==hist[hist_pos][1])
        return;
    hist_offset++;
    if (++hist_pos==MAX_HIST)
        hist_pos=0;
    hist[hist_pos]=({time(),str,orig});
}

private int check_delayed_action(string cmd)
{
    mixed ret;

    if (in_delayed_action() && 
        ((!(query_da_flag() & DA_OK_COMMAND) && cmd != DA_STOP_ACTION) ||
         ((query_da_flag() & DA_NO_HALT) && cmd == DA_STOP_ACTION)))
    {
            ret = funcall(query_da_handler(),cmd);
        if (stringp(ret))
        {
            write(ret);
            return 1;
        }
        if (ret != DA_COMMAND_ALLOWED)
            return 1;
    }
}

private int is_log_command(string str)
{
    int i;

    if (!sizeof(log_commands) || !str)
        return 0;
    for (i=0; i<sizeof(log_commands); i++)
        if (left(str, strlen(log_commands[i])) == log_commands[i])
            return 1;
    return 0;
}

private int compare_strings(string str1, string str2)
{

    if (!str1) return 0;
    return !strstr(str1,str2);
    // return (sscanf(str1, str2+"%~s"));
}

int namens_kuerzel_filter(object who, object env)
{
// if (who == this_object()) return 0;
   return ( !(wizp(who) && who->query_no_wer()) ||         // nicht auf Wer-Liste aber
            (!IS_INVIS(who) && env == environment(who)) ); // sichtbare neben einem -> geht doch
}

private string expand_aliases(string str, int was_std_alias)
{
    string verb, rest;

    // Direkter Treffer ohne Argumente
    string cmd = aliases[str];
    if (!cmd)
    {
        cmd = std_aliases[str];
        if (cmd)
            was_std_alias = 1;
    }
    if (cmd)
        return regreplace(cmd, "\\\\([$&])", "\\1", 1);

    // Nur Verb nachschlagen.
    if (sscanf(str, "%s %s", verb, rest)!=2)
        verb=str;
    if (sizeof(verb) > 1 && verb[0] == '\\' )
        return str[1..] ;
    else if (!(cmd = aliases[verb]) && !(cmd = std_aliases[verb]))
        return str;
    else if (rest)
        return insert_cmd(cmd,rest);
    else
        return regreplace(cmd,"\\\\([$&])","\\1",1);
}

#define CATCH_ALL_ACTIONS ({ "go_command", "ride_command" })

private int command_exists(string cmd)
{
    mixed **cmds = match_command(cmd, this_object());
    if (!cmds)
        return 0;
    foreach(mixed* cmd: cmds)
        if (!(cmd[CMDM_FUN] in CATCH_ALL_ACTIONS))
            return 1;
    return 0;
}

nomask mixed modify_command(string str)
{
    string cmd, orig;
    string *words, *names, *matching_names;
    int i,j,k, was_std_alias;

#if 0
    if(guestp(this_object()) && str)
        write_file("/save/GUEST-"+(this_object()->query_real_name()),
            shorttimestr(time())+" > "+str+"\n");
#endif

    this_object()->set_active_prompt(0);

    command_count++;
    if (command_count > COMMANDS_PER_TIME_SLICE)
    {
        command_per_slice_warned = 0;
        command_count = 0;
        if ((time() - last_time_slice < TIME_SLICE) && !lordp(this_object()))
        {
            this_object()->send_message_to(this_object(),
                MT_NOTIFY, MA_UNKNOWN, 
                "Deine viel zu schnellen Handlungen haben dich völlig "
                "durcheinander gebracht. Das haut dich glatt um...");
            remove_interactive(this_object());
            return 1;
        }
        last_time_slice = time();
    }
    else if (command_count > COMMANDS_PER_TIME_SLICE_WARNING)
    {
        if (!command_per_slice_warned && 
            (time() - last_time_slice < TIME_SLICE) && !lordp(this_object()))
        {
            command_per_slice_warned = 1;
            this_object()->send_message_to(this_object(),
                MT_NOTIFY, MA_UNKNOWN, 
                "\nVORSICHT!\nDeine schnellen Handlungen bringen dich "
                "noch völlig durcheinander. Mach doch mal Pause!");
        }
    }

#if __VERSION__ > "3.5.2"
    str = trim(str);
#else
    str = trim(convert_umlaute(str));
#endif

    if (!sizeof(str))
        return 1;

    if (str[0]=='^')
    {
        str=edit_old_command(str[1..]);
        if (!str)
            return 1;
        write(str+"\n");
    }
    else if (str[0]=='%')
    {
        str=get_old_command(str[1..]);
        if (!str)
            return 1;
        write(str+"\n");
    }

    orig = str;
    
// ' muss drinnen sein wegen sage-Kuerzel ' !!!
// #define TRENNER_REGEXP   "[ .,;:!?()\"']"
#define TRENNER_REGEXP   "[^a-zA-Z0-9#]"

    if ( !wizardshellp(this_object()) && str && sizeof(
        regexp( ({ " "+str+" " }),
                TRENNER_REGEXP "[a-zA-Z][a-zA-Z]*#" TRENNER_REGEXP)) > 0 )
    {
        if(this_object()->query_wiz_level())
            names = users()->query_real_name();
        else
            names =
            map_objects( filter( users(), #'namens_kuerzel_filter,
                                       environment( this_object())),
                         "query_real_name");

        words = regexplode( str, TRENNER_REGEXP);
        for ( i = 0 ; i < sizeof( words) ; i += 2 )
            // if ( words[i][<1] == '#'  &&  words[i] != "#" )
            if ( sizeof( regexp( ({ words[i] }), "^[a-zA-Z][a-zA-Z]*#$")) )
            {
                matching_names =
                    filter( names, #'compare_strings,
                                  lower_case( words[i][0..<2]));
                switch ( sizeof( matching_names) )
                {
                    case 0 :
                        write("\""+words[i]+"\" nicht gefunden.\n");
                        break ;
                    case 1 :
                        words[i] = capitalize(matching_names[0]);
                        break ;
                    case 2 :
                        // Nur 2 Woerter, aber ich bin selbst dabei
                        if ((k=member(matching_names, this_object()->query_real_name()))>=0)
                        {
                            words[i] = matching_names[1-k];
                            break;
                        }
                        // Fall through!
                    default :
                        matching_names -= ({this_object()->query_real_name()});
                        for ( k = j = 0; j < sizeof( matching_names) ; j++ )
                            if ( present( matching_names[j], environment()) )
                                if ( k )
                                    k = -1;
                                else
                                    k = j+1;
                        if ( k > 0 )
                            words[i] = capitalize(matching_names[k-1]);
                        else
                            write("\""+words[i]+"\" ist nicht eindeutig.\n");
                }
            }
        str = implode( words, "");
    }

    cmd = expand_aliases(str, &was_std_alias);
    if (!command_exists(cmd))
    {
        string verb, lcverb, rest;
        if (!sscanf(str, "%s %s", verb, rest))
            verb = str;
        lcverb = lower_case(verb);
        if (lcverb != verb)
        {
            cmd = lcverb;
            if (rest)
                cmd += " " + rest;
            cmd = expand_aliases(cmd, &was_std_alias);
        }
    }

    if (check_delayed_action(cmd))
            return 1;

    if (is_log_command(cmd))
        MASTER_OB->log_command("z.Zt in "+object_name(environment())+
            " ("+environment()->query_short(this_object())+")\n"+cmd);
    this_object()->add_to_meldungspuffer("log",">",cmd);
    if (!was_std_alias &&
        (!previous_object() ||
         (doing!=time() && previous_object()==this_object() && (!previous_object(1) ||
         load_name(previous_object(1)) == PORTAL_SERVER))))
        add_history(cmd, orig);
    command_start();
    return comm=cmd;
}


static int alias(string str)
{
    int i;
    string kurz, lang;

    if (this_player() != this_interactive())
        return 1;
    if (!str)
    {
        string *tmp, *out;
        out = ({ "Deine Kürzel"+
                 (adminp(this_object())?sprintf("(%d):",sizeof(aliases)):
                  sprintf(" (%d von maximal %d):",
                          sizeof(aliases),
                          (wizp(this_object())?2*MAX_ALIASES:MAX_ALIASES)))
              });
        tmp=sort_array(m_indices(aliases),#'<);
        for (i=sizeof(tmp); i--; )
            out += explode ( wrap_say (
            strlen(tmp[i])>14?tmp[i]+":":left (tmp[i]+":",15),
            aliases[tmp[i]],0,17)[0..<2],"\n");
        this_object()->more(out,"--Mehr--",0,M_AUTO_END);
        return 1;
    }

    if (sscanf(trim(str),"%s %s",kurz,lang)==2)
    {
        if (convert_umlaute(kurz)=="kuerzel")
        {
            write("Auf 'kuerzel' kann man kein Kürzel setzen.\n");
            return 1;
        }
        if (kurz=="alias")
        {
            write("Auf 'alias' kann man kein Kürzel setzen.\n");
            return 1;
        }
        for(i = 0; i < strlen(kurz); i++)
            if(kurz[i] < 32 || (kurz[i] >= 127 && member("ÄÖÜẞäöüß", kurz[i]) < 0))
            {
                write("Nur Asciizeichen sind im Kürzel erlaubt.\n");
                return 1;
            }
        if ( adminp(this_object()) ||
             sizeof(aliases)<MAX_ALIASES || 
             (wizp(this_object()) && (sizeof(aliases)<2*MAX_ALIASES)) ||
             member(aliases,kurz))
        {
            aliases[kurz] = lang;
            write("Ok.\n");
            return 1;
        }
        write("Sorry, mehr Kürzel gehen nicht!\n");
        return 1;
    }

    if ( member( aliases, str) )
        write( sprintf( "Ku%-=73s\n", "erzel "+str+":  "+ aliases[str])) ;
    else if (member (str, '*')>=0)
    {
        string reg = regreplace(str, "([\\[\\]\\(\\).+])","\\\\\\1", 1);
        string *allist;
        reg = regreplace(reg, "\\*", ".*", 1);
        
        allist = regexp(m_indices(aliases), "^"+reg+"$");
        if(sizeof(allist))
        {
            string *out;
            out = ({ "Deine Kürzel:" });
            allist=sort_array(allist,#'>);
            foreach(string al: allist)
                out += explode ( wrap_say (
                strlen(al)>14?al+":":left (al+":",15),
                aliases[al],0,17)[0..<2],"\n");
            this_object()->more(out,"--Mehr--",0,M_AUTO_END);
            return 1;
        }
        else
            write("Keine Kürzel gefunden.\n");
    }
    else
        write(str+" ist kein Kürzel!\n");
    return 1;
}


static int unalias(string str)
{
    string * words;

    if (this_player() != this_interactive())
        return 1;
    else if ( !str )
        write( "Welches Kürzel soll entfernt werden?\n");
    else if ( member(aliases,str) )
    {
        write(wrap("Kürzel "+str+" für \""+aliases[str]+"\" entfernt."));
        m_delete(aliases,str);
    }
    else
    {
        words = explode(str, " ");
        if ((sizeof(words) > 1) && member(aliases, words[0]))
        {
            write(str+" ist kein Kürzel!\n"
                "Meintest du vielleicht: entkürzel "+words[0]+"\n");

        }
        else
        {
            write(str+" ist kein Kürzel!\n");
        }
    }
    return 1;
}


static int history()
{
    int i;

    if (hist_offset<0)
    {
        for (i=1; i<=hist_pos; i++)
            write("%"+i+"\t"+hist[i][1]+"\n");
        return 1;
    }
    for (i=hist_pos+1; i<MAX_HIST; i++)
        write("%"+(i+hist_offset-hist_pos)+"\t"+hist[i][1]+"\n");
    for (i=0; i<=hist_pos; i++)
        write("%"+(i+hist_offset-hist_pos+MAX_HIST)+"\t"+hist[i][1]+"\n");
    return 1;
}

// Liefert den Befehlspuffer in einem Format, wie
// der Puffer von /secure/event und /i/player/event.
// Also ({Zeit, Verursacher, Text, Flag})
static nomask mixed *query_history()
{
    int i;
    mixed p = ({});

    if (hist_offset<0)
    {
        for (i=1; i<=hist_pos; i++)
            p+=({({hist[i][0],this_object(),"%"+i+"\t"+hist[i][1]+"\n",
                 PUFFER_NOT_INDENT})});
        return p;
    }
    for (i=hist_pos+1; i<MAX_HIST; i++)
        p+=({({hist[i][0],this_object(),"%"+(i+hist_offset-hist_pos)+"\t"+hist[i][1]+"\n",
             PUFFER_NOT_INDENT})});
    for (i=0; i<=hist_pos; i++)
        p+=({({hist[i][0],this_object(),"%"+(i+hist_offset-hist_pos+MAX_HIST)+"\t"+hist[i][1]+"\n",
             PUFFER_NOT_INDENT})});
    return p;
}

protected string *query_history_commands()
{
    if (hist_offset<0)
        return map(hist[1..hist_pos],#'[,2);

    return map(hist[hist_pos+1..MAX_HIST-1]+hist[0..hist_pos],#'[,2);
}

nomask void show_history ()
{
    if (!this_player() || this_player()!=this_interactive() ||
        !previous_object() || !adminp(this_player()) ||
        (geteuid(previous_object())!=geteuid(this_player())) ||
        (object_name(previous_object())[0..14] != "/obj/zauberstab"))
        return;
    history();
}                                                            

static void do_it(string *cmds, int show_act)
{
    if (show_act)
        write("Tue: "+cmds[0]+"\n");
    doing=time(); // gegen Endlos-tue's
    efun::command(strip(cmds[0]));
    doing=0;
    
    if(!this_object())        // ende via tue...
        return;
        
    if (sizeof(cmds)>1)
        call_out("do_it",2,cmds[1..],show_act);
    else
        write("Tue beendet.\n");
}

static int do_cmd(string str)
{
    string *cmds;
    int show_act;

    if (!str)
    {
        if (remove_call_out("do_it")!=-1)
        {
            write("Tue gestoppt.\n");
            return 1;
        }
        if (!tue_trenner)
            FAIL("Syntax: tue [-l] cmd1,cmd2,cmd3,...\n");
        FAIL("Dein Tue-Trenner: "+tue_trenner+"\n"
             "Syntax: tue [-l] cmd1"+tue_trenner+"cmd2"+tue_trenner+"cmd3"+tue_trenner+"...\n");
    }
    if (doing==time() || find_call_out("do_it")!=-1)
    {
        write("Bin noch mit einem anderen Tue beschäftigt.\n");
        return 1;
    }
    cmds=explode(str,tue_trenner || ",")-({""});
    if (sizeof(cmds))
    {
        if (cmds[0][0..2]=="-l ")
        {
            cmds[0]=cmds[0][3..];
            show_act=1;
        }
        do_it(cmds,show_act);
        return 1;
    }
    if (!tue_trenner)
        FAIL("Syntax: tue [-l] cmd1,cmd2,cmd3,...\n");
    FAIL("Dein Tue-Trenner: "+tue_trenner+"\n"
         "Syntax: tue [-l] cmd1"+tue_trenner+"cmd2"+tue_trenner+"cmd3"+tue_trenner+"...\n");
}

int stop_do_cmd(string str)
{
    if(!str || lower_case(space(str))!="tue")
        return notify_fail("Stoppe was?\n", FAIL_NOT_CMD);
    if (remove_call_out("do_it")!=-1)
        write("Tue gestoppt.\n");
    else
        write("Du tust derzeit nix.\n");
    doing = 0; // Falls der Befehl einen RTE ausloeste, doing zuruecksetzen.
    return 1;
}

/*
FUNKTION: query_doing
DEKLARATION: int query_doing()
BESCHREIBUNG:
Liefert 1 zurueck, wenn der Spieler gerade einen Befehl
innerhalb eines Tue ausfuehrt, 0 sonst.
GRUPPEN: spieler
*/
int query_doing()
{
    return doing == time();
}

/*
FUNKTION: query_aliases
DEKLARATION: mapping query_aliases()
BESCHREIBUNG:
Damit bekommt man ein mapping, in dem alle kuerzel eines Spielers stehen.
Wird von der Wizard - Shell benoetigt.
VERWEISE:
GRUPPEN: spieler
*/
mapping query_aliases()
{
    if (!extern_call() || wizardshellp(previous_object()))
        return aliases;
}

int query_aliases_count()
{
    return sizeof(aliases);
}

static void set_aliases(mapping al)
{
    if (mappingp(al))
        aliases = al;
}

int alias_exists(string str)
{
    return member(aliases, str);
}

void set_prompt_string(string str);
static void init_aliases()
{
    if (efun::interactive(this_object()))
#if __EFUN_DEFINED__(set_modify_command)
        set_modify_command(this_object());
#else
        configure_interactive(this_object(), IC_MODIFY_COMMAND, this_object());
#endif
    set_prompt_string(prompt);
    if (pointerp(aliases))                // Konvertierung
        aliases = mkmapping(aliases[0],aliases[1]);
}

/*
FUNKTION: set_prompt_string
DEKLARATION: void set_prompt_string(string prompt_string)
BESCHREIBUNG:
Mit dieser Funktion kann man einen neuen Prompt setzen.
VERWEISE: query_prompt_string
GRUPPEN: spieler
*/
void set_prompt_string(string str)
{
    if (!stringp(str))
        prompt="> ";
    else
        prompt=str;
    /* Garthan: naechste Zeile wegen Ende_statue mit Wicht Fehler. */
    if (interactive(this_object()))
        set_prompt(prompt, this_object());
}

/*
FUNKTION: query_prompt_string
DEKLARATION: string query_prompt_string()
BESCHREIBUNG:
Liefert den momentan gesetzten Prompt eines Spielers zurueck.
VERWEISE: set_prompt_string
GRUPPEN: spieler
*/
string query_prompt_string()
{
    return prompt;
}

nomask string query_command()
{
    if (object_name(previous_object()) == MASTER_OB)
        return comm;
}


void execute_login_command ()
{
    if (login_command_executed) return;
    // steckt der Spieler in der Todessequenz?
    if (this_object()->query_ghost() > 0) return;
    login_command_executed = 1;
    if (aliases["login"])
        efun::command ("login");
}

/*****************************************************************************
 *     Der Mini-Editor                                                       *
 *****************************************************************************/
private nosave mapping editor_status;
private nosave mapping gmcp_status;

/*
FUNKTION: in_mini_ed
DEKLARATION: int in_mini_ed()
BESCHREIBUNG:
Liefert einen Wert !=0, wenn dieser Spieler sich im Mini-ed befindet.
VERWEISE: is_editing, mini_ed
GRUPPEN: Dateien
*/
int in_mini_ed()
{
    return (editor_status && 
            (editor_status["webmud"] 
            // || editor_status["gmcp"]
            || find_input_to(this_object(), this_object())>=0));
}

/*
FUNKTION: is_editing
DEKLARATION: int is_editing()
BESCHREIBUNG:
Liefert einen Wert !=0, wenn dieser Spieler sich in irgendeinem Editor
befindet.
VERWEISE: in_edit, query_editing
GRUPPEN: Dateien
*/
int is_editing()
{
    if(this_object()->in_edit()            // Player-Ed
    || query_editing(this_object())        // Driver-Ed
    || this_object()->has_editor_shadow()  // Xed
    || in_mini_ed())                       // Mini-Ed
        return 1;
}

            
/*
FUNKTION: mini_ed
DEKLARATION: varargs int mini_ed(closure retfun, int which_ed, string tempfile, mapping msgs, mapping flags, string* text)
BESCHREIBUNG:
Ein Mini-Editor wird gestartet. Dort kann man einfach nacheinander Zeile
fuer Zeile eingeben. Mit einem einzelnen '.' oder '**' wird der Editor beendet.

Man hat folgende Befehle zur Verfuegung:
  ~e  In den ed wechseln. (Goetter erhalten den Driver-ed mit der Datei
      edfile, Spieler den ed aus dem /i/player/editor.c)
  ~x  In den xed wechseln (Nur fuer Goetter, die einen Zauberstab dabei
      haben bzw. in der Wizard-Shell)
  ~p  Nur fuer Goetter: In den Player-Ed wechseln (nicht in der Wiz-Shell)
  ~q  Ed abbrechen

Die Parameter:
  closure retfun      Diese Funktion nach Beendigung des Editors aufgerufen.
                      Sie erhaelt als Parameter den Text als Array aus den
                      einzelnen Zeilen oder 0, falls der Ed abgebrochen wurde.
  int which_ed        Damit kann man explizit angeben, mit welchem Editor
                      gestartet werden soll:
                        0  Mini-Ed
                        1  Ed (Goetter: Driver-Ed, Spieler: Player-ed)
                        2  Player-Ed
                        3  Xed (Nur fuer Goetter)
  string tempfile     Eine temporaere Datei, die vom Driver-Ed editiert wird.
                      Ist tempfile 0, so wird /w/<gott>/priv/ed.tmp genutzt.
  mapping msgs        Meldungen fuer den Player-ed (siehe edit())
                      Folgende Meldungen daraus nutzt der Mini-ed:

     MINI_ED_ALREADY_EDITING: "Du schreibst schon einen Text.\n"
     MINI_ED_START_TEXT:      "Gib nun den Text ein. Mit '**' oder '.'
                               beenden, mit '~q' abbrechen.\n"
     MINI_ED_PLAYER_INFO:     "Mit '~e' kommt man in den ED.\n"
     MINI_ED_WIZ_INFO:        "Mit '~e' kommt man in den ED, mit ~x in den XED
                               und mit ~p in den Player-ED.\n"
     MINI_ED_MAX_SIZE_OVERRUN "Mehr als %d Zeilen darfst Du nicht schreiben.\n"
     MINI_ED_TITLE:           Fenstertitel fuer Editorfenster, z.B. im WebMUD.

  mapping flags       Zusaetzliche Flags:

     MINI_ED_WRAP_LEN:        Bei wieviel Zeichen umgebrochen werden soll.
                              (-1: Ueberhaupt nicht. Standard ist 79)
     MINI_ED_FORCE_WRAP:      Bricht die Zeilen, welche in den anderen
                              Editoren (player-ed, driver-ed, xed)
                              eingegeben wurden, nachtraeglich um.
                              
  string* text        Bereits eingegebene Zeilen.

Alle Angaben sind optional. Die Funktion liefert 1 bei Erfolg, 0 bei einem
Fehler zurueck (Fehlermeldung wird per write ausgegeben.)
VERWEISE: edit, ed, more, cat, tail, set_more_chunk, query_more_chunk,
GRUPPEN: Dateien
*/

static void end_mini_ed(string *buffer)
{
    mapping old_status;
    
    // Text umbrechen
    if(buffer && editor_status["flags"][MINI_ED_FORCE_WRAP] && 
        editor_status["flags"][MINI_ED_WRAP_LEN]>=0)
    {
        int len = editor_status["flags"][MINI_ED_WRAP_LEN] || 79;
        int maxlen = query_limits()[LIMIT_ARRAY];
        string *ntext = ({});

        foreach(string str:buffer)
        {
                string *toadd = explode(wrap(str,len)[0..<2],"\n");
            if(sizeof(ntext)+sizeof(toadd)>maxlen)
            {
                    printf(funcall(editor_status["msgs"][MINI_ED_MAX_SIZE_OVERRUN])
                    || MINI_ED_STD_MSGS[MINI_ED_MAX_SIZE_OVERRUN],maxlen);
                ntext = 0;
                break;
            }
                ntext += toadd;
        }
        buffer = ntext;
    }
    old_status = editor_status;
    editor_status = 0;

    funcall(old_status["retfun"], buffer);
}

static void end_player_ed(string *buffer, int changed)
{
    end_mini_ed((changed || sizeof(buffer))?buffer:0);
}

static void driver_ed_ends()
{
    string file=editor_status["tempfile"];
    string text=read_file(file);
    string *buffer;
    this_object()->notify_ed_exit(1);
    if(!text)
    {
        if(file_size(file)>0)
            write("Die Datei konnte nicht eingelesen werden.\n");
    }
    else
    {
        buffer=explode(text,"\n");
        if(buffer[<1]=="") buffer=buffer[0..<2];
    }
    rm(file);
    end_mini_ed(buffer);
}

void xed_ends(string *buffer, int esc_key)
{
    end_mini_ed(esc_key?0:buffer);
}

void gmcp_edit_ends(string *buffer,string key)
{
    if (mappingp(gmcp_status))
    {
        if (member(gmcp_status,key))
        {
            mapping old_status = editor_status;
            editor_status = gmcp_status[key];
            end_mini_ed(buffer);
            editor_status = old_status;
            m_delete(gmcp_status,key);
            if (!sizeof(gmcp_status))
                gmcp_status=0;
        }
    }
}

private void add_to_gmcp_status(string key,mapping estatus)
{
    if (!mappingp(gmcp_status))
    {
        gmcp_status = ([key:estatus]);
    }
    else
    {
        gmcp_status[key] = estatus;
    }
}

private int change_to_ed(int which_ed, string* buffer)
{
    switch(which_ed)
    {
        case 1: // ED
        case 2:  //Player-ED
            if(wizardshellp(this_object()) || (which_ed==1 && wizp(this_object())))
            {
                string file;
                if(!(file=editor_status["tempfile"]))
                    editor_status["tempfile"]=file=
                        "/w/"+this_object()->query_real_name()+
                        ((file_size("/w/"+this_object()->query_real_name()+"/priv")==FSIZE_DIR)?"/priv/ed.tmp":"/ed.tmp");
                if(file_size(file)!=FSIZE_NOFILE)
                {
                    write("Datei '"+file+"' existiert bereits!\n");
                    return 0;
                }
                if(!write_file(file, buffer?implode(buffer,"\n")+"\n":""))
                {
                    write("Konnte die Datei '"+file+"' nicht schreiben!\n");
                    return 0;
                }
                this_object()->notify_ed_enter(1);
                ed(file, "driver_ed_ends");
                rm(file); // Jetzt hat er's hoffentlich in den Speicher eingelesen.
                return 1;
            }
            return this_object()->edit(buffer, "end_player_ed", this_object(),
                0, editor_status["msgs"]);
        case 3: // Xed
            if(wizp(this_object()))
            {
                object xed;
                if(wizardshellp(this_object()))
                    xed = this_object();
                else
                    xed = present_clone("/obj/zauberstab",this_object());
                if(!xed)
                {
                    write("Keinen XED gefunden!\n");
                    return 0;
                }
                if(xed->edit_buffer(buffer||({}),#'xed_ends)<=0)//'))
                {
                    write("Fehler beim Start des XED.\n");
                    return 0;
                }
                return 1;
            }
    }
}

void mini_input_line(string str, string *buffer)
{
    int n;
    mixed text;
    
    this_object()->set_active_prompt(0);
    
    if(!str) str="";
    switch(str)
    {
        case "~q":
            this_object()->notify_ed_exit(0);
            end_mini_ed(0);
            return;
        case ".":
        case "**":
            this_object()->notify_ed_exit(0);
            end_mini_ed(buffer);
            return;
        case "~x":
        case "~p":
            if(!wizp(this_object()))
                break;
        case "~e":
            this_object()->notify_ed_exit(0);
            if(change_to_ed(member(({"~e","~p","~x"}),str)+1, buffer))
                return;
            input_to("mini_input_line", INPUT_PROMPT,
                (sizeof(buffer)+1)+((sizeof(buffer)<9)?">>":">"),
                buffer);
            return;
    }
    n=query_limits()[LIMIT_ARRAY];
    if(editor_status["flags"][MINI_ED_WRAP_LEN]<0 || !sizeof(str))
        text = ({str});
    else
        text=explode(wrap(str,editor_status["flags"][MINI_ED_WRAP_LEN]||79),"\n")[0..<2];
    if(sizeof(buffer)+sizeof(text)>n)
        printf(funcall(editor_status["msgs"][MINI_ED_MAX_SIZE_OVERRUN])
            || MINI_ED_STD_MSGS[MINI_ED_MAX_SIZE_OVERRUN],n);
    else
        buffer+=text;
    input_to("mini_input_line", INPUT_PROMPT,
        (sizeof(buffer)+1)+((sizeof(buffer)<9)?">>":">"),
        buffer);
}

#define GET_MSG(x) (funcall(msgs[x])||MINI_ED_STD_MSGS[x])
varargs int mini_ed(closure retfun, int which_ed, string tempfile,
    mapping msgs, mapping flags, string *buffer)
{
    string gmcp_key;
    if(this_object()!=this_player()) // Was soll man da als Fehler ausgeben?
        return 0;

    if(!msgs) msgs=([]);
    if(!flags) flags=([]);

    if(is_editing())
    {
        write(GET_MSG(MINI_ED_ALREADY_EDITING));
        return 0;
    }

    // Parameter speichern
    editor_status=(["retfun":retfun,
                    "tempfile":tempfile,
                    "msgs":msgs,
                    "flags":flags]);
    
    gmcp_key = this_object()->gmcp_edit_get_key(tempfile,msgs[MINI_ED_TITLE]);
    if( stringp(gmcp_key) &&
        this_object()->query_client_option(CLIENT_WEBMUD_NO_EDIT))
    {
        mapping result =        
            this_object()->gmcp_start_edit(#'gmcp_edit_ends,//')(
                msgs[MINI_ED_TITLE],tempfile,buffer);
        // TODO Fehlermeldungen
        DEBUG(sprintf("gmcp_start_edit=>%Q",result));
        if (result["ok"])
        {
            add_to_gmcp_status(gmcp_key,editor_status);
        }
        editor_status = 0; 
        // beim gmcp edit koennen mehr als 1 Datei edittiert werden
        return 1;
    }

    if(this_object()->uses_webmud() &&
      !this_object()->query_client_option(CLIENT_WEBMUD_NO_EDIT) &&
       this_object()->webmud_edit(GET_MSG(MINI_ED_TITLE),
           function void(string str)
           {
               string *restext;
               if(str)
               {
                   if(sizeof(str) && str[<1]=='\n')
                       str = str[0..<2];
                   restext = explode(str, "\n");
               }
               end_mini_ed(restext);
           }, sizeof(buffer) ? implode(buffer,"\n")+"\n" : ""))
    {
        editor_status["webmud"] = 1;
        return 1;
    }

    if(!which_ed)
    {
        this_object()->notify_ed_enter(0);
        write(GET_MSG(MINI_ED_START_TEXT));
        if(wizp(this_object()))
            write(GET_MSG(MINI_ED_WIZ_INFO));
        else
            write(GET_MSG(MINI_ED_PLAYER_INFO));
        input_to("mini_input_line", INPUT_PROMPT,
            (sizeof(buffer)+1)+((sizeof(buffer)<9)?">>":">"),
            buffer||({}));
        return 1;
    }
    else
        return change_to_ed(which_ed, buffer);
}

// begin: Das zu vervollstaendigende Wort
// words: Ein Mapping aus den Woertern,
//        die optionalen Werte dienen dazu, gleichartige Worte zu definieren
//        (zwei Worte gelten als gleich, wenn sie die gleichen Werte haben)
// casesensitive: Vergleiche ohne Beachtung von Gross-/Kleinschreibung durchfuehren
// Ergebnis:
//  0:       Kein Treffer gefunden.
//  string:  Der neue Ersatz fuer 'begin'.
//  string*: Mehrere Optionen zur Auswahl.
private string|string* make_completion(string begin, mapping words, int casesensitive)
{
    string res;

    words = filter(words,
        casesensitive?(: !strstr($1,$3) :):(: !strstr(lower_case($1),$3) :),
        begin);

    if(!sizeof(words))
        return 0;

    if(sizeof(words)>1 && widthof(words))
    {
        int equal = 1;
        // Wir schauen, ob die nicht alle das gleiche behandeln.
        // (Gleiche Werte im Mapping)
        for(int col=0; col<widthof(words); col++)
            if(sizeof(mkmapping(m_values(words, col)))>1)
            {
                equal = 0;
                break;
            }

        if(equal)
            // Wir suchen das laengste raus.
            words = ([
                sort_array(m_indices(words), (: (sizeof($1) < sizeof($2)) ||
                    (sizeof($1) == sizeof($2) && $1>$2) :))[0] ]);
    }

    foreach(string v: words)
    {
        if(!res)
            res = v;
        else
        {
            int start = sizeof(begin);
            int stop = min(sizeof(res), sizeof(v));
            string cv, cres;

            if(casesensitive)
            {
                cv = v;
                cres = res;
            }
            else
            {
                cv = lower_case(v);
                cres = lower_case(res);
            }

            for(int i=start;i<stop;i++)
                if(cres[i] != cv[i])
                {
                    stop = i;
                    break;
                }

            res = v[0..stop-1];
        }
        if(sizeof(res) <= sizeof(begin))
            return sort_array(map(m_indices(words),(: trim($1) :)),#'>);
    }

    return res;
}

protected string|string* complete_command(string cmd)
{
    int spacepos;

    if(!sizeof(cmd))
        return "";

    spacepos = strrstr(cmd, " ");
    if(spacepos<0)
    {
        string* kuerzel = m_indices(query_aliases());

        // Wir vervollstaendigen Verben.
        string|string* result = make_completion(cmd[(cmd[0]=='!')?1:0..],
            mkmapping(
                map(efun::query_actions(this_object(), QA_VERB),#'+, " "),
                efun::query_actions(this_object(), QA_OBJECT),
                efun::query_actions(this_object(), QA_FUNCTION)) +
            mkmapping(
                map(kuerzel, #'+, " "),
                ({0})*sizeof(kuerzel),
                kuerzel), 1);
        if (stringp(result))
            return cmd[0]=='!' ? ("!" + result) : result;
        return result;
    }
    else if(spacepos+1 == sizeof(cmd))
        return cmd;
    else
    {
        string* words;
        int slashpos = strrstr(cmd, "/");
        int casesensitive;
        string|string* result;

        if(this_object()->query_wiz_level() && slashpos > spacepos)
        {
            // Ein Dateiname
            string path = this_object()->add_path(
                cmd[spacepos+1..slashpos]);

            if(path[<1] != '/')
                path += "/";

            words = map(transpose_array(({
                         get_dir(path, GETDIR_NAMES|GETDIR_UNSORTED) || ({}),
                         get_dir(path, GETDIR_SIZES) || ({})})),
                    (: $1[0] + (($1[1]==FSIZE_DIR)?"/":" ") :));
            spacepos = slashpos;
            casesensitive = 1;
        }
        else
        {
            // Gegenstaende im Env und Inv.
            if(!wizardshellp(this_object()))
                words = map(all_inventory(this_object())+
                            ((environment() && this_object()->can_see(environment()))?
                                all_inventory(environment()):({})),
                    function string(object ob)
                    {
                        if(ob->query_invis() & V_ATOM_NOLIST)
                            return 0;
                        if(strstr(ob->query_cap_name()||""," ")>-1)
                            return 0;
                        return ob->query_cap_name() + " ";
                    }) - ({0});
            else
                words = ({});

            // Ausgaenge
            if (environment())
                words += environment()->query_command_list(this_object()->query_wiz_level()?0:EXIT_VISIBLE) || ({});

            if(this_object()->query_wiz_level())
            {
                // Dateien im aktuellen Verzeichnis
                string path = this_object()->query_current_path();

                if(path[<1] != '/')
                    path += "/";

                words +=
                    map(transpose_array(({
                         get_dir(path, GETDIR_NAMES|GETDIR_UNSORTED) || ({}),
                         get_dir(path, GETDIR_SIZES) || ({})})),
                    (: $1[0] + (($1[1]==FSIZE_DIR)?"/":" ") :));

                // Spieler
                if(!wizardshellp(this_object()))
                    words += map(users()->query_real_cap_name(), #'+, " ");
            }
            else
                // Spieler
                words += map(filter(users(),
                    (: !IS_INVIS($1) && !$1->query_no_wer() :)
                    )->query_real_cap_name(), #'+, " ");
        }

        result = make_completion(casesensitive
            ?cmd[spacepos+1..]
            :lower_case(cmd[spacepos+1..]),
            mkmapping(words), casesensitive);

        if (stringp(result))
            return cmd[..spacepos] + result;
        return result;
    }
}
