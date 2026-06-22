// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/i/player/wiz_soul.c
// Description: Markierungen und Spionieren
// Author:	Freaky (23.12.93)
//		Garthan : Trusting (23.12.93)
//		Freaky (19.11.1999) Trusting auf input-to umgestellt.
//		Freaky (26.11.1999) Security-Check in set_mark verbessert.
//				    viele Funktionen nomask gemacht

#pragma save_types
#pragma strong_types

#include <level.h>
#include <message.h>
#include <uids.h>
#include <math.h>
#include <input_to.h>
#include <driver_info.h>
#include <debug_info.h>
#include <more.h>

#define NUM_MARKS 10

#define WSF_ALL_FAILS	1
private int wiz_soul_flags;

private static object *mark  = allocate(NUM_MARKS);
private static string *marks = allocate(NUM_MARKS);
private static object snoop_allow;
private mixed *new_wiz_errors;
private mapping zauberstab_info = ([]);
private mapping editor_options;

int query_level();
string query_real_name();
void call_save();
varargs void notify_message(string msg, int type);

/*
FUNKTION: query_mark
DEKLARATION: nomask mixed query_mark(int nr, int flag)
BESCHREIBUNG:
Liefert die Markierung, wie sie mit dem Zauberstabbefehl zmark gesetzt
werden kann, zurueck.
Ist flag 0, so wird ein Objekt geliefert, andernfalls der Objektname.

Siehe auch:
zm ?
VERWEISE: query_mark, query_marks, set_mark
GRUPPEN: grundlegendes
*/

nomask mixed query_mark(int nr, int flag)
{
    if (!intp(nr) || nr < 0 || nr >= NUM_MARKS)
	return 0;

    if (flag)
	return marks[nr];
    return mark[nr];
}

/*
FUNKTION: query_marks
DEKLARATION: nomask mixed *query_marks(int flag)
BESCHREIBUNG:
Liefert ein Array mit allen Markierungen, wie sie mit dem Zauberstabbefehl
zmark gesetzt werden koennen, zurueck.
Ist flag 0, so werden Objekte geliefert, andernfalls Objektnamen.

Siehe auch:
zm ?
VERWEISE: query_mark, query_marks, set_mark
GRUPPEN: grundlegendes
*/

nomask mixed *query_marks(int flag)
{
    if (flag)
	return copy(marks);
    return copy(mark);
}

/*
FUNKTION: set_mark
DEKLARATION: nomask void set_mark(int nr, object ob)
BESCHREIBUNG:
Speichert das Objekt ob in der Markierung mit der Nummer nr sowie dessen
Objektnamen ab. Diese Methode kann nur von bestimmten privilegierten
Objekten aufgerufen werden.

Siehe auch:
zm ?
VERWEISE: query_mark, query_marks, set_mark
GRUPPEN: grundlegendes
*/

nomask void set_mark(int nr, object ob)
{
    if (this_player() != this_object() || (ob && !objectp(ob)) ||
	    !intp(nr) || nr < 0 || nr >= NUM_MARKS ||
	    (geteuid(previous_object()) != geteuid() &&
	     geteuid(previous_object()) != ROOT_UID))
	return;

    if(ob)
    {
        mark[nr]=ob;
        marks[nr]=object_name(ob);
    }
    else
    {
        mark[nr]=0;
        marks[nr]=0;
    }
}

static nomask int get_zauberstab(string str)
{
    if (str)
	return 0;
    this_object()->open_con();
    clone_object("/obj/zauberstab")->move(this_object());
    notify_message("Ok.\n");
    return 1;
}

static nomask int get_rid_of_zauberstab(string str)
{
    object ob;

    if (str)
	return 0;
    ob = present("zauberstab",this_object());
    if (ob)
	destruct(ob);
    notify_message("Ok.\n");
    return 1;
}

static nomask int snoop_on(string str)
{
    object ob;

    if (this_interactive() != this_object())
    {
	this_object()->send_message_to(this_object(),MT_NOTIFY,MA_UNKNOWN,
	   capitalize(this_interactive()->query_real_name())+
	   " wollte dich zu 'beobachte"+(str?" "+str:"")+"' zwingen.\n");
	return 1;
    }
    if (!str)
    {
	snoop(this_object());
	snoop_allow = 0;
	notify_message("Ok.\n");
        this_object()->restart_mudclient();
	return 1;
    }
    if (str[0] == '+')
    {
	if (str == "+")
	{
	    if (snoop_allow)
		notify_message("Du erlaubst gerade " +
			capitalize(snoop_allow->query_real_name()) +
			", dich zu beobachten.\n");
	    else
		notify_message("Du erlaubst gerade niemanden, dich zu "
			"beobachten.\n");
	    return 1;
	}
	ob = find_player(lower_case(str[1..]));
	if (!ob)
	{
	    notify_message("Person nicht gefunden.\n");
	    return 1;
	}
	snoop_allow = ob;
	notify_message("Ok. Du erlaubst jetzt " +
		capitalize(snoop_allow->query_real_name()) +
		", dich zu beobachten.\n");
	return 1;
    }

    snoop_allow=0;
    ob = find_player(lower_case(str));
    if (!ob)
    {
	notify_message("Person nicht gefunden.\n");
	return 1;
    }
    if (!interactive(ob))
    {
	notify_message("Warum willst Du eine Statue beobachten?\n");
	return 1;
    }
    /*
    if ((query_level()!=LVL_ADMIN) && !ob->query_allow_snoop(this_object())) {
	write(capitalize(str)+" erlaubt dir nicht, "+ihn(ob)+
		" zu beobachten.\n");
	return 1;
	}
    */

    if (snoop(this_object(),ob))
    {
	notify_message("Ok.\n");
	this_object()->suspend_mudclient();
    }
    else
	notify_message(capitalize(str) + " erlaubt dir nicht, " + ihn(ob) +
		" zu beobachten.\n");
    return 1;
}

nomask int query_allow_snoop(object who)
{
    return (query_level() < LVL_WIZ) || (who && (who == snoop_allow));
}

private void write_trusted_file(mapping trustees)
{
    string file, *indices, name;
    int i;

    name = query_real_name();
    rm("/w/" + name + "/TRUSTED");
    file = "";
    indices = m_indices(trustees);
    for(i = 0; i < sizeof(indices); i++)
	file += indices[i] + ":" + trustees[indices[i]] + "\n";
    if (file != "")
	write_file("/w/"+name+"/TRUSTED",file);
}

private mapping read_trusted_file()
{
    string file, *lines, *fields;
    mapping table;
    int dosave, i;

    table = ([]);
    if(file = read_file("/w/"+query_real_name()+"/TRUSTED"))
    {
	lines = explode(file,"\n")-({""});
	for(i = 0; i < sizeof(lines); i++)
	{
	    fields = explode(lines[i], ":");
	    table[fields[0]] =
		(dosave += (sizeof(fields) < 2)) ? time() : to_int(fields[1]);
	}
    }
    if (dosave)
	write_trusted_file(table);
    return table;
}

static int trust_command(string str)
{
    mapping map;
    string *trustees;
    int i;
    if(!str)
    {
	notify_message("Auf Dauer vertraust du folgenden Objekten:\n");
	map = read_trusted_file();
	trustees = m_indices(map);
	for(i = 0; i < sizeof(trustees); i++)
	    notify_message("   " + left(trustees[i],40) + " " +
		    shorttimestr(map[trustees[i]]) + "\n");
	notify_message("\nWeitere Objekte bekommst Du in diese Liste, wenn Du "
		"statt 'vertraue ....'\nden Befehl 'vertraue .... immer' "
		"verwendest.\nObjekte kannst Du aus dieser Liste mit "
		"'vertraue ... nicht mehr' wieder\nlöschen. Diese Liste "
		"ist unter ~/TRUSTED zu finden.\n");
	return 1;
    }
    notify_fail(capitalize(str)+" hast du wohl nicht bei Dir!\n");
    return 0;
}

nomask int trust(int mode)
{
    object ob;
    string res;

    ob = previous_object();
    if (!ob || !clonep())
	return 0;
    if (!this_player() || this_player() != this_interactive() ||
         this_player() != this_object())
	return 0;
    if (!wizp(this_player()))
	return 0;
    if (strstr(query_verb(),"vertrau") && strstr(query_verb(),"trust"))
	return 0;
    
    switch(mode)
    {
	case 2:
	    res = "ewig ";
	    break;
	case 1:
	    res = "immer ";
	    break;
	case -1:
	    res = "nicht mehr ";
	    break;
	default:
	    res = "";
	    break;
    }
    notify_message(sprintf("%-1.75=s","Möchtest du " + Dem(ob) + " (" +
	    object_name(ob) + ") wirklich " + res + "vertrauen?\n"));
    input_to("input_trust", INPUT_PROMPT, "(Ja/Nein) ",ob,mode);
    return 1;
}

static int input_trust(string str, object ob, int mode)
{
    mapping trustees;
    string fname;

    if (!str || lower_case(str) != "ja")
    {
	notify_message("Ok, dann halt nicht.\n");
	return 1;
    }

    if (sscanf(object_name(ob),"%s#%~d",fname) == 2)
	if (mode == 1 || mode == 2)
	{
	    if (!(trustees = read_trusted_file())[fname])
	    {
		trustees[fname] = mode == 2 ? -1 : time();
		write_trusted_file(trustees);   
	    }
	}
	else if (mode == -1)
	{
	    if ((trustees = read_trusted_file())[fname])
	    {
		m_delete(trustees, fname);
		write_trusted_file(trustees);
	    }
	    notify_message(Dem(ob) + " (" + object_name(ob) +
		    ") traust Du nun gar nicht mehr.\n");
	    ob->callback_trust(ob, mode);
	    return 0;
	}

    if (this_object()->query_wiz_level())
    {
	if (geteuid(ob) == getuid())
	{
	    notify_message("Du vertraust " + dem(ob) + " (" + object_name(ob) +
		    ") schon, da " + er(ob) +
		    " schon Deine effektive User ID besitzt.\n");
	    return 0;
	}
        ob->setup_trust();
	if (geteuid(ob))
	{
	    notify_message("Du kannst " + dem(ob) + " (" + object_name(ob) +
	       ") nicht vertrauen, da " + er(ob) +
	       " schon eine effektive User ID besitzt.\n");
	    return 0;
	}
#if __EFUN_DEFINED__(export_uid)
	if (getuid(ob) == geteuid())
#else
	if (geteuid(ob) == geteuid())
#endif
	{
	    notify_message("Du vertraust " + dem(ob) + " (" + object_name(ob) +
		    "), " + er(ob) + " besitzt eh schon Deine UserID.\n");
	    ob->callback_trust(ob, mode);
	    return 1;
	}
	notify_message("Du vertraust " + dem(ob) + " (" + object_name(ob) +
		").\n");
#if __EFUN_DEFINED__(export_uid)
    	export_uid(ob);
#else
	configure_object(ob, OC_EUID, geteuid());
#endif
	ob->callback_trust(ob, mode);
	return 1;
    }
}

nomask int is_trusted_object()
{
    int trust_time, *times;
    mapping trustees;
    string fname;
    object ob;

    if ((ob=previous_object()) && clonep() &&
	    sscanf(object_name(ob),"%s#%~d",fname) == 2 &&
	    (trust_time = (trustees = read_trusted_file())[fname]))
    {
	if ((!sizeof(times = get_dir(fname+".c", 4)) || times[0] > trust_time)
		&& trust_time != -1)
	{
	    m_delete(trustees, fname);
	    write_trusted_file(trustees);
	    return -1;
	}
        ob->setup_trust();
#if __EFUN_DEFINED__(export_uid)
	export_uid(ob);
#else
        configure_object(ob, OC_EUID, geteuid());
#endif
        ob->callback_trust(ob, (trust_time==-1)?2:1);
	return 1;
    }
    return 0;
}

// Wird vom Zauberstab und von der wizard-shell aufgerufen
void set_new_wiz_errors(mixed *errs)
{
    new_wiz_errors = errs;
    call_save();
}
mixed *query_new_wiz_errors() { return new_wiz_errors || ({ MAX_INT, ({}) }); }

// Wird vom Zauberstab gesetzt
void set_zauberstab_info(mapping info)
{
    if (geteuid(previous_object()) == geteuid() && mappingp(info))
	zauberstab_info = info;
}
mapping query_zauberstab_info()
{
    if (geteuid(previous_object()) == geteuid())
	return zauberstab_info;
}

// Fuer den Xed
mapping query_editor_options()
{
    if (geteuid(previous_object()) == geteuid())
	return editor_options ||
	    ([
		"max_x":	80,	// Bildschirmbreite
		"max_y":	23,	// Bildschirmhoehe
		"split_y":	0,	// Wieviel davon fuers MUD
		"tab_width":	8,	// Tabulatorweite
		"insert":	1,	// Einfuegemodus
		"show_pos": 	1,	// Position anzeigen
		"backup":	0,	// Backups anfertigen
	    ]);
}
void set_editor_options(mapping new_opts)
{
    if (geteuid(previous_object()) == geteuid())
	editor_options = new_opts;
}


static void set_show_all_fails(int i)
{
    if(i)
	wiz_soul_flags |= WSF_ALL_FAILS;
    else
	wiz_soul_flags &= ~WSF_ALL_FAILS;
}

int query_show_all_fails()
{
    return (wiz_soul_flags & WSF_ALL_FAILS)?1:0;
}

int driver_status(string str)
{
#if __EFUN_DEFINED__(driver_info)
    int opt;

    switch(space(str||""))
    {
        case "":
            opt = DI_STATUS_TEXT_MEMORY;
            break;

        case "tables":
            opt = DI_STATUS_TEXT_TABLES;
            break;

        case "swap":
            opt = DI_STATUS_TEXT_SWAP;
            break;

        case "malloc":
            opt = DI_STATUS_TEXT_MALLOC;
            break;

        case "malloc extstats":
            opt = DI_STATUS_TEXT_MALLOC_EXTENDED;
            break;

        default:
            return notify_fail("status [tables|swap|malloc|malloc extstats]\n");
    }

    this_object()->more(explode(driver_info(opt), "\n")[..<2], 0, 0, M_AUTO_END);
#else
    string old_opt = space(str||"");

    switch(old_opt)
    {
        case "":
            old_opt = 0;
            break;

        case "tables":
        case "swap":
        case "malloc":
        case "malloc extstats":
            break;

        default:
            return notify_fail("status [tables|swap|malloc|malloc extstats]\n");
    }

    this_object()->more(explode(efun::debug_info(DINFO_STATUS, old_opt), "\n")[..<2], 0, 0, M_AUTO_END);
#endif
    return 1;
}
