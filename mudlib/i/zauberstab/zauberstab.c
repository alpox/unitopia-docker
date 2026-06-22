// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/i/zauberstab/zauberstab.c
// Description: Zauberstab
// Author:	Freaky (23.12.93)
// Modified by:	Freaky (07.03.96) Hilfe checkt auch uppercase
//		Freaky (10.03.1998) message auf send_message umgebaut.

/* Zauberstab created by Freaky
 * Der Zauberstab ist dazu da, Objekte zu durchleuchten und zu manipulieren
 *
 * Wer Vorschlaege hat kann sich an Freaky wenden (z.B. per Mail im Spiel)
 */

#pragma save_types
#pragma strong_types

inherit "/i/item";
inherit "/i/install";
inherit "/i/tools/coroutine";
inherit "/i/tools/pipe";
inherit "/i/tools/getopt";
inherit "/i/tools/browser";
inherit "/i/tools/dynamic_browser";
inherit "/i/xeditor/xeditor";
inherit "/i/tools/build_table";
protected functions inherit "/i/tools/security";
private functions inherit "/i/tools/top";

#include <acl.h>
#include <add_hp.h>
#include <apps.h>
#include <browser.h>
#include <config.h>
#include <configuration.h>
#include <database.h>
#include <debug_info.h>
#include <driver_info.h>
#include <deklin.h>
#include <dynamic_browser.h>
#include <editor.h>
#include <error_db.h>
#include <error.h>
#include <files.h>
#include <functionlist.h>
#include <game.h>
#include <getopt.h>
#include <gilden.h>
#include <input_to.h>
#include <invis.h>
#include <landschaft.h>
#include <level.h>
#include <mail.h>
#include <math.h>
#include <message.h>
#include <more.h>
#include <move.h>
#include <notifier.h>
#include <objectinfo.h>
#include <object_info.h>
#include <parse_com.h>
#include <pipe.h>
#include <properties.h>
#include <quest.h>
#include <regexp.h>
#include <room.h>
#include <sent.h>
#include <simul_efuns.h>
#include <stats.h>
#include <term.h>
#include <touch.h>
#include "/secure/simul_efun.h"
#if __EFUN_DEFINED__(interactive_info)
#include <interactive_info.h>
#endif

private object owner;
private string ed_func;
private mapping params, vars;
private closure c_clone_msg, c_destruct_msg;

#include "/i/zauberstab/zdefs.h"
#include "/i/zauberstab/zlib.inc"
#include "/i/zauberstab/zskill.inc"
#include "/i/zauberstab/zinfos.inc"
#include "/i/zauberstab/zprof.inc"
#include "/i/zauberstab/ztrace.inc"
#include "/i/zauberstab/zmarker.inc"
#include "/i/zauberstab/zsoul.inc"
#include "/i/zauberstab/files.inc"
#include "/i/zauberstab/zacl.inc"

#include "/i/zauberstab/find.inc"
#include "/i/zauberstab/zvolk.inc"
#include "/i/zauberstab/zinhalt.inc"
#include "/i/zauberstab/zbrowse.inc"
#include "/i/zauberstab/zfehler.inc"
#include "/i/zauberstab/zpost.inc"
#include "/i/zauberstab/zplugin.inc"
#include "/i/zauberstab/zdump.inc"
#include "/i/zauberstab/zsysinfo.inc"
#include "/i/zauberstab/zgit.inc"

#ifdef __SQLITE__
#include "/i/zauberstab/zsqlite.inc"
#endif

// Dieses Define ist dafuer da, die Kommandouebersicht zu erzeugen
// Erzeugt wird sie dann, indem man /i/zauberstab/zauberstab erneuert und
// die Funktion make_doc() aufruft.
// Danach sind die neuen Files unter /save/kommandos.* zu finden.
// NICHT VERGESSEN: Danach wieder undefinieren und erneuern !!!!!
// #undef MAKE_DOC
// #define MAKE_DOC

mapping public_groups;

void init_groups()
{
    public_groups = ([:1]);
    
    string f = read_file("/static/adm/PUBLIC_GROUPS");
    if(!f)
	return;
	
    foreach(string line: explode(f,"\n"))
    {
	string* parts;
	
	line = trim(line);
	if(!strlen(line) || line[0]=='#')
	    continue;
	    
	parts = regexplode(line," ")-({" ",""});
	if(sizeof(parts)!=2)
	    continue;
	
	public_groups[parts[1]]=to_int(parts[0]);
    }
}

void create()
{
    seteuid(getuid());
    set_id( ({ "stab", "zauberstab" }) );
    set_name("zauberstab");
    set_gender("maennlich");
    set_long("Ein blau schimmernder Stab. Versuch mal 'hilfe stab'.\n");
    vars = ([]);
    params = ([]);
    set_max_output(24);
    set_weight(0);
    params["plugins"] = ([]);
    set_no_move_reason("Der Stab darf nicht aus der Hand gegeben werden!");
    init_security_for_actions();
    if(!clonep())
	init_groups();
}

#ifdef MAKE_DOC
string *comms;
int doc_level;

varargs void add_action(string fun, string command, int flag)
{
    int len;
    string tmp;

    if (!flag)
    {
	if (fun[0..1]=="z_" && command[0]=='z')
	    tmp=command[1..];
	else
	    tmp=command;
	if (strstr(fun,convert_umlaute(tmp),2)!=2)
	{
	    printf("Falsche Funktion: %s, %s\n",fun,command);
	    return;
	}
	if (!symbol_function(fun,TO))
	{
	    printf("Funktion existiert nicht: %s, %s\n",fun,command);
	    return;
	}
	if (command!="hilfe")
	    comms+=({command});
    }
    else
    {
	if (fun[0..1]=="z_")
	    tmp="z"+fun[2..];
	else if (fun[0..1]=="f_")
	    tmp=fun[2..];
	else
	    tmp=fun;
	if (tmp != convert_umlaute(command))
	{
	    printf("Falsche Funktion: %s, %s\n",fun,command);
	    return;
	}
	if (!symbol_function(fun,TO))
	{
	    printf("Funktion existiert nicht: %s, %s\n",fun,command);
	    return;
	}
	if (flag > 0)
	{
	    printf("Flag falsch: %s, %s\n",fun,command);
	    return;
	}
	else
	    len = -flag;
	comms+=({tmp[0..len-1]+"["+tmp[len..]+"]"});
    }
}

#undef wizp
#define wizp(x) doc_level>=LVL_WIZ
#undef gesellep
#define gesellep(x) doc_level>=LVL_GESELLE
#undef lordp
#define lordp(x) doc_level>=LVL_LORD+1
#undef adminp
#define adminp(x) doc_level==LVL_ADMIN+2

#endif // MAKE_DOC

void add_commands()
{

#ifndef MAKE_DOC
    if (this_player()!=OWN)
	return;
#endif // MAKE_DOC

    if (wizp(OWN))
    {
        add_action("f_ebruelle",       "ebrülle",         -6 );
        add_action("f_eich",           "eich"                );
	add_action("f_cat",            "cat"                 );
	add_action("f_echo",           "echo"                );
	add_action("f_echoan",         "echoan"              );
	add_action("f_echoanalle",     "echoanalle"          );
	add_action("f_grep",           "grep"                );
	add_action("f_heim",           "heim"                );
	add_action("f_hilfe",          "hilfe"               );
	add_action("f_in",             "in"                  );
	add_action("f_ls",             "ls"                  );
	add_action("f_more",           "more"                );
	add_action("f_sichtbar",       "sichtbar",        -6 );
	add_action("f_tail",           "tail"                );
	add_action("f_unsichtbar",     "unsichtbar",      -8 );
	add_action("f_volk",           "volk"                );
	add_action("z_HB",             "zHB"                 );
        add_action("z_acl",            "zacl"                );
	add_action("z_alter",          "zalter",          -4 );
	add_action("z_ausgaenge",      "zausgänge",       -5 );
	add_action("z_ausruestung",    "zausrüstung",     -2 );
	add_action("z_bruelle",        "zbrülle",         -2 );
	add_action("z_cd",             "zcd"                 );
	add_action("z_cout",           "zcout",           -3 );
	add_action("z_dbg",            "zdbg"                );
	add_action("z_dbg_hands",      "zdbg_hands"          );
	add_action("z_dbg_komm",       "zdbg_komm"           );
	add_action("z_echo",           "zecho"               );
	add_action("z_etup",           "zetup",           -3 );
	add_action("z_funktion",       "zfunktion",       -3 );
	add_action("z_fl",             "zfl"                 );
	add_action("z_gehzu",          "zgehzu",          -2 );
#ifdef UNItopia
#ifdef TestMUD
	add_action("z_gehzu_unitopia", "zgunitopia",      -3 );
#else
	add_action("z_gehzu_orbit",    "zgorbit",         -3 );
#endif
#endif
	add_action("z_gilde",          "zgilde",          -3 );
	add_action("z_grad",           "zgrad",           -3 );
        add_action("z_gruppe",         "zgruppe"             );
	add_action("z_heile",          "zheile"              );
	add_action("z_idle",           "zidle",           -3 );
	add_action("z_information",    "zinformation",    -2 );
	add_action("z_inhalt",         "zinhalt",         -5 );
	add_action("z_inherit",        "zinherit",        -5 );
	add_action("z_kommandos",      "zkommandos",      -2 );
	add_action("z_licht",          "zlicht",          -2 );
	add_action("z_map",            "zmap"                );
	add_action("z_markiere",       "zmarkiere",       -2 );
	add_action("z_more",           "zmore",           -3 );
	add_action("z_grep",           "zgrep",           -4 );
	add_action("z_msg",            "zmsg"	             );
	add_action("z_plugin",         "zplugin"             );
	add_action("z_qspion",         "zqspion"             );
	add_action("z_raetsel",        "zrätsel",         -2 );
	add_action("z_shadows",        "zshadows",        -3 );
	add_action("z_skill",          "zskill"              );
	add_action("z_skillalles",     "zskillalles",     -8 );
	add_action("z_skillsumme",     "zskillsumme",     -8 );
	add_action("z_spiele",         "zspiele",         -3 );
	add_action("z_statistik",      "zstatistik",      -5 );
	add_action("z_stop",           "zstop"               );
	add_action("z_suche",          "zsuche",          -3 );
	add_action("z_sysinfo",        "zsysinfo",        -3 );
	add_action("z_obst",           "zobst"               );
	add_action("z_teleport",       "zteleport"           );
	add_action("z_top",            "ztop"                );
#ifdef CREATE_TRACES
	add_action("z_trace",          "ztrace",          -3 );
#endif
	add_action("f_xed",            "xed"                 );
#if __EFUN_DEFINED__(git_status)
        add_action("z_log",            "zlog"                );
#endif
#if __EFUN_DEFINED__(spell_check)
	add_action("z_spell",          "zspell",          -4 );
#endif
	this_player()->add_options_menue("Zauberstab",
	    #'print_zauberstab_options);
	this_player()->add_options_menue("Plugins",
	    #'print_plugin_options);
	    
	this_player()->add_options_action("Einstellungen des Zauberstabs",
	    "zauberstab", -1, (: z_etup(strlen($1)&&$1) :));
	this_player()->add_options_action("Zauberstab-Plugins",
	    "plugin", -4, (: z_plugin(strlen($1)&&$1) || write(funcall(query_notify_fail())); :));
    }

    if (gesellep(OWN))
    {
	add_action("f_blitze",         "blitze"              );
	add_action("f_cp",             "cp"                  );
	add_action("f_ed",             "ed"                  );
	add_action("f_hole",           "hole"                );
	add_action("f_kneble",         "kneble"              );
	add_action("f_kneble",         "knebele",         -6 );
	add_action("f_lade",           "lade"                );
	add_action("f_mkdir",          "mkdir"               );
	add_action("f_mv",             "mv"                  );
	add_action("z_putze",          "zputze"              );
	add_action("f_rm",             "rm"                  );
	add_action("f_rmdir",          "rmdir"               );
	add_action("f_find",           "find"                );
	add_action("f_sperre",         "sperre"              );
	add_action("f_entsperre",      "entsperre"           );
	add_action("f_zwinge",         "zwinge"              );
	add_action("z_call",           "zcall",           -2 );
	add_action("z_ed",             "zed"                 );
	add_action("z_erneuere",       "zerneuere",       -4 );
	add_action("z_erschaffe",      "zerschaffe",      -4 );
	add_action("z_fehler",         "zfehler",         -3 );
	add_action("z_kuerzel",	       "zkürzel",         -3 );
	add_action("z_lpc",            "zlpc"                );
	add_action("z_skilladdieren",  "zskilladdieren",  -8 );
	add_action("z_skilloeschen",   "zskillöschen",    -7 );
        add_action("z_sort",           "zsort"               );
#if __EFUN_DEFINED__(git_status)
        add_action("z_commit",         "zcommit"             );
#endif
#ifdef __SQLITE__
        add_action("z_sqlite",         "zsqlite",         -4 );
#endif
	add_action("z_variablen",      "zvariablen",      -2 );
	add_action("z_zerstoere",      "zzerstöre",       -2 );
    }
    
    if (vogtp(OWN))
    {
	add_action("z_post",           "zpost"	             );
    }

    if (lordp(OWN))
    {
	add_action("f_entferne",       "entferne"            );
	add_action("f_installiere",    "installiere"         );
    }

    if (adminp(OWN))
    {
	add_action("f_chsh",           "chsh"                );
	add_action("z_dump",           "zdump"               );
	add_action("z_vl",             "zvl"                 );
    add_action("z_client",         "zclient"             );
    }
}

void init()
{
    if (OWN && TP != OWN)
	return;

    if (TP && TP == ENV(TO))
    {
	mapping new_groups;
    
	if (!wizp(TP))
	{
	    write("Du darfst mit so einem mächtigen Spielzeug noch nicht spielen.\n");
	    destruct(TO);
	    return;
	}
	
	if(sizeof(filter(all_inventory(TP),
	    (:
		load_name($1)=="/obj/zauberstab" &&
		!function_exists("make_doc",$1)
	    :) ))>1)
	{
	    write("Dein Zauberstab löst sich in Luft auf, da du bereits einen besitzt.\n");
	    destruct(TO);
	    return;
	}

	OWN=TP;
	set_owner(OWN);

	if (geteuid()!=geteuid(OWN))
	{
	    write("Das ist nicht Dein Zauberstab!\n");
	    destruct(TO);
	    return;
	}

	add_commands();

	xeditor::init_xeditor(OWN);

	/* c_clone_msg, c_destruct_msg also updated in z_etup */
	c_clone_msg = mixed_to_closure(add_dot_to_msg(({string})OWN->query_clone_msg()), ({'ob}), 1);
	c_destruct_msg = mixed_to_closure(add_dot_to_msg(({string})OWN->query_destruct_msg()), ({'ob}), 1);

	walk_mapping(({mapping})OWN->query_zauberstab_info(),"set_param");
	
	new_groups = ({mapping})blueprint()->get_public_groups(params["last-init"]);
	
	if(sizeof(new_groups) || !params["zfe-groups"])
	    set_param("zfe-groups", (params["zfe-groups"]||([]))+new_groups);
	
	set_param("last-init", time());
    }
}

mapping get_public_groups(int last_time)
{
    mapping ret = ([:0]);
    
    foreach(string grp, int t: public_groups)
	if(last_time<t)
	    m_add(ret, grp);

    return ret;
}

string query_short(object viewer)
{
    string tmp, short_string;

    if (query_own_light()>0)
	tmp = "leuchtend";
    else if (query_own_light()<0)
	tmp = "schwarz";
    else
	tmp = 0;
    if(short_string = query_short_string())
       return short_string + (tmp?" ("+tmp+")":"");
    return Ihr(0, tmp, 0, 0, 0, ART_DER | ART_VIS);
}

int query_prevent_shadow(object dummy) { return 1; }

int f_hilfe(string str)
{
    string rest, low_rest, *pl_help;

    SECURE;
    HELP("hilfe");
    if (!(rest = me(str)))
	rest = str;

    if (!rest)
	return 0;

    low_rest = convert_umlaute(LOW(rest));
    if (low_rest == "")
	low_rest = "stab";
    if (low_rest == "kommandos")
    {
	if (adminp(OWN))
	    low_rest = "admin";
	else if (lordp(OWN))
	    low_rest = "lord";
	else if (vogtp(OWN))
	    low_rest = "vogt";
	else if (gesellep(OWN))
	    low_rest = "geselle";
	else
	    low_rest = "lehrling";
	if (({string})OWN->more(HILFE_PFAD+"kommandos."+low_rest,0,0,M_AUTO_END))
	    FAIL("Zu " + str + " gibt's keine Hilfe.\n");
    }
    else
    {
	if (pl_help = get_plugin_command_help(rest))
	    OWN->more(pl_help,0,0,M_AUTO_END);
	else if (({string})OWN->more(HILFE_PFAD+rest,0,0,M_AUTO_END) &&
		 ({string})OWN->more(HILFE_PFAD+low_rest,0,0,M_AUTO_END))
	    FAIL("Zu " + str + " gibt's keine Hilfe.\n");
    }
    PRINT;
    return 1;
}

#ifdef MAKE_DOC
private void do_make_doc(string str, int level)
{
    comms=({});
    doc_level=level;
    add_commands();
    comms=sort_array(comms,#'>);
    rm("/save/kommandos."+str);
    write_file("/save/kommandos."+str,sprintf(LINE "%-79#s\n" LINE
	"Hilfe zum Kommando gibt es immer mit '<Befehl> ?'\n" LINE,
	implode(comms,"\n")));
}

int make_doc()
{
    do_make_doc("admin",LVL_ADMIN+2);
    do_make_doc("geselle",LVL_GESELLE);
    do_make_doc("lehrling",LVL_LEARNER);
    do_make_doc("lord",LVL_LORD+1);
    do_make_doc("vogt",LVL_VOGT);
    return 1;
}
#endif // MAKE_DOC

int remove()
{
    plugins_remove();
    if (OWN)
	OWN->set_zauberstab_info(params);
    return ::remove();
}

int more_action(string str, int line, int max_line, mixed more_id)
{
    switch(stringp(more_id)?more_id:mappingp(more_id)?more_id["id"]:"")
    {
	case "Options":
	    return handle_zauberstab_options(str);
	case "Plugins":
	    return handle_plugin_options(str);
	case "Plugins:List":
	    return handle_plugin_list_options(str);
	case "Plugins:Active":
	    return handle_active_plugin_options(str, more_id["plugin"]);
	case "Plugins:Info":
	    return handle_plugin_info_options(str, more_id["callback"]);
	case "Plugins:Inactive":
	    return handle_inactive_plugin_options(str, more_id["plugin"]);
	case "Plugins:Proxy":
	    return handle_plugin_proxy(str, line, max_line, more_id);
	default:
	    return zfe_more_action(str, line, max_line, more_id);
    }
}

void more_end(string str, int line, int max_line, mixed more_id)
{
    switch(stringp(more_id)?more_id:mappingp(more_id)?more_id["id"]:"")
    {
	case "Plugins:Proxy":
	    handle_plugin_proxy_end(str, line, max_line, more_id);
	    return;

	case "Plugins:Info":
	    return handle_plugin_info_end(str, more_id["callback"]);

#if __EFUN_DEFINED__(trace_call)
        case "Trace:Help":
            return handle_trace_help_end(more_id);
#endif

	default:
	    browser::more_end(str, line, max_line, more_id);
	    return;
    }
}
