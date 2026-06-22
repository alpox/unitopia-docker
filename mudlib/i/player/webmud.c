// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /i/player/webmud.c
// Author:      Gnomi (09.09.2009)
// Description: Support fuer den WebMUD-Client

#include <acl.h>
#include <apps.h>
#include <files.h>
#include <telnet.h>
#include <room.h>
#include <move.h>
#include <level.h>
#include <invis.h>
#include <rtlimits.h>
#include <message.h>
#include <colours.h>
#include <simul_efuns.h>
#if __EFUN_DEFINED__(interactive_info)
#include <interactive_info.h>
#endif

#include "player.h"

#define SECURE_RPC if(object_name(previous_object()) != "/secure/rpc/webmud" && object_name(previous_object()) != "/secure/dbus/webmud2") return 0;
#define EDIT_LIMIT 250000
nomask int set_current_path(string pfad);
int query_wiz_level();

private nosave int wmactive, wmbactive;
private string webmud_style;
private nosave int last_inv_update, last_who_update;

private void send_binary_message(string msg)
{
#if __VERSION__ > "3.5.2"
    efun::binary_message(to_bytes(msg, "UTF-8"), 3);
#else
    efun::binary_message(msg, 3);
#endif
}

protected void update_points_display()
{
    if(wmactive && interactive())
    {
        send_binary_message(sprintf(
            "\e_points:AP,%d,%d:%s,%d,%d\e\\",
            this_object()->query_hp(),
            this_object()->query_max_hp(),
            this_object()->query_sp_short_name(),
            this_object()->query_sp(),
            this_object()->query_max_sp()
        ));
    }
}

private void webmud_notify_moved(string controller, mapping mv_infos)
{
    string *exits;
    string msg;

    if(!wmactive || !interactive() || !environment())
        return;

    if(mv_infos[MOVE_FLAGS] & MOVE_ATOM_NOT_NOTIFY)
        return;

    exits = environment()->query_command_list(EXIT_VISIBLE);
    msg = "\e_exits";

    foreach(string ex: exits)
        switch(ex)
        {
            case "norden":       msg += ":n";   break;
            case "nordosten":    msg += ":no";  break;
            case "osten":        msg += ":o";   break;
            case "südosten":     msg += ":so";  break;
            case "suedosten":    msg += ":so";  break;
            case "süden":        msg += ":s";   break;
            case "sueden":       msg += ":s";   break;
            case "südwesten":    msg += ":sw";  break;
            case "suedwesten":   msg += ":sw";  break;
            case "westen":       msg += ":w";   break;
            case "nordwesten":   msg += ":nw";  break;
            case "hoch":         msg += ":h";   break;
            case "runter":       msg += ":r";   break;
            case "geradeaus":    msg += ":xn";  break;
            case "halbrechts":   msg += ":xno"; break;
            case "rechts":       msg += ":xo";  break;
            case "scharfrechts": msg += ":xso"; break;
            case "zurück":       msg += ":xs";  break;
            case "zurueck":      msg += ":xs";  break;
            case "scharflinks":  msg += ":xsw"; break;
            case "links":        msg += ":xw";  break;
            case "halblinks":    msg += ":xnw"; break;
        }
    msg += "\e\\";
    send_binary_message(msg);
}

private void update_inventory()
{
    if(interactive() && last_inv_update < time())
    {
        send_binary_message("\e_update:inv\e\\");
        last_inv_update = time();
    }
}

nomask static void update_directory()
{
    if(interactive() && wmactive)
        send_binary_message("\e_update:dir\e\\");
}

private void webmud_notify_moved_inout(string controller, mapping mv_infos)
{
    // Fuer's Inventar.
    object wer = mv_infos[MOVE_OBJECT];

    if(!wer || (!wizp(this_object()) && (wer->query_invis() & V_ATOM_NOLIST)))
        return;

    update_inventory();
}

private void webmud_notify_invis(string controller, object who, int alt, int neu)
{
    if((alt^neu)&V_ATOM_NOLIST)
        update_inventory();
}

private void webmud_notify_remove_inventory(string controller, object who)
{
    remove_call_out(#'update_inventory);
    call_out(#'update_inventory, 0);
}

nomask void webmud_notify_whoupdate()
{
    if(!wmactive || !interactive() || object_name(previous_object()) != CONTROL)
        return;

    if (last_who_update < time())
    {
        send_binary_message("\e_update:who\e\\");
        last_who_update = time();
    }
}

protected void send_webmud_event_message(int event_nr, mixed origin_ob, string mess)
{
    mapping info;

    if(!wmactive || !interactive())
        return;

    info = EVENT_MASTER->query_events()[event_nr];
    if(!event_nr)
        return;

    send_binary_message("\e_event:"+info["name"]+"\e\\");
    catch(this_object()->send_message_to(this_object(), MT_CHANNEL, MA_UNKNOWN, mess); publish, reserve 16384);
    send_binary_message("\e_event\e\\");
}

protected void init_webmud()
{
    mixed client;
    string ip;

    if(wmactive)
    {
        this_object()->delete_controller("notify_moved", #'webmud_notify_moved);
        this_object()->delete_controller(({"notify_moved_in","notify_moved_out"}), #'webmud_notify_moved_inout);
        this_object()->delete_controller("notify_invis", #'webmud_notify_invis);
        this_object()->delete_controller("notify_remove_inventory", #'webmud_notify_remove_inventory);
        CONTROL->delete_controller(({"notify_event_login","notify_event_logout","notify_event_statue"}), #'webmud_notify_whoupdate);
    }

#if __EFUN_DEFINED__(query_ip_number)
    ip = efun::query_ip_number(this_object());
#else
    ip = efun::interactive(this_object()) && efun::interactive_info(this_object(), II_IP_NUMBER);
#endif
    if(member(({"127.0.0.1", "::1", "::ffff:127.0.0.1"}), ip) >= 0
            && this_object()->query_telnet(TELOPT_TTYPE, &client)
            && sizeof(client)>1 && sizeof(client[1]))
    {
        wmbactive = client[1][0] == "webmudbeta";
        wmactive = wmbactive || client[1][0] == "webmud";
    }
    else
        wmbactive = wmactive = 0;

    if(wmactive)
    {
         mapping colors = ({mapping}) this_object()->query_colours();

        this_object()->add_controller("notify_moved", #'webmud_notify_moved);
        this_object()->add_controller(({"notify_moved_in","notify_moved_out"}), #'webmud_notify_moved_inout);
        if(!wizp(this_object()))
            this_object()->add_controller("notify_invis", #'webmud_notify_invis);
        this_object()->add_controller("notify_remove_inventory", #'webmud_notify_remove_inventory);

        update_points_display();
        webmud_notify_moved("startup", ([
            MOVE_OBJECT:this_object(),
            MOVE_NEW_ROOM:environment()]) );
        send_binary_message("\e_update:inv\e\\\e_update:who\e\\");
        if(webmud_style)
            send_binary_message("\e_style:"+webmud_style+"\e\\");
        if(colors[ACT_CONFIG,0]&CO_CONFIG_MUTE)
            send_binary_message("\e_mutebeep\e\\");
        else
            send_binary_message("\e_unmutebeep\e\\");
        if(colors[ACT_CONFIG,COL_BEEP_TYPE])
            send_binary_message("\e_defaultbeep:"+colors[ACT_CONFIG,COL_BEEP_TYPE]+"\e\\");
        CONTROL->add_controller(({"notify_event_login","notify_event_logout","notify_event_statue"}), #'webmud_notify_whoupdate);
        if(query_wiz_level())
            send_binary_message("\e_update:dir\e\\");
        send_binary_message("\e_commands:Befehle:3:gib,gib ,an ,\n:töte,töte ,\n:nimm,nimm :öffne,öffne ,\n:schließe,schließe ,\n:rede,rede :betrachte,betrachte ,\n:drücke,drücke ,\n:ziehe,ziehe ,\n\e\\");
    }
}

static int uses_webmud()
{
    return wmactive;
}

static int uses_webmud_beta()
{
    return wmbactive;
}

static string query_webmud_style()
{
    return webmud_style;
}

static void set_webmud_style(string st)
{
    webmud_style = st;
    if(wmactive && interactive() && st)
        send_binary_message("\e_style:"+st+"\e\\");
}

void add_actions()
{
    if (query_wiz_level())
    {
        add_action("w_ed", "wed");
    }
}

nosave mapping editfiles = ([:5]);
nosave string lasteditfile;
#define EF_TYPE   0
#define EF_TEXT   1
#define EF_FILE   1
#define EF_RETFUN 2
#define EF_ALIVE  3

#define EFT_TEXT  0
#define EFT_FILE  1

nosave int editnr = 0;

varargs int webmud_edit(string name, closure retfun, string txt)
{
    string editid;
    int notempty;

    if(!uses_webmud())
        return 0;

    editid = sprintf("%s#%d", explode(object_name(),"#")[1], ++editnr);
    notempty = sizeof(txt) && 1;

    m_add(editfiles, editid, EFT_TEXT, txt, retfun, 0);
    this_object()->send_message_to(this_object(), MT_NOTIFY, MA_UNKNOWN,
        "WebMUD-Editor gestartet.\n");

    send_binary_message(sprintf("\e_edit:%s:%d:%s\e\\",editid, notempty, name));
    if(notempty)
        call_out(function void()
        {
            if(member(editfiles, editid) && !editfiles[editid, EF_ALIVE])
            {
                this_object()->send_message_to(this_object(), MT_NOTIFY, MA_UNKNOWN,
                    "Editieren fehlgeschlagen.\n");
                call_with_this_player(retfun, 0);
            }
        }, 10);
    return 1;
}

varargs int webmud_editfile(string file, closure retfun)
{
    string editid;
    int notempty;

    if(!uses_webmud() || !query_wiz_level())
        return 0;

    if(extern_call() && geteuid(previous_object()) != geteuid())
        return 0;

    editid = sprintf("%s#%d", explode(object_name(),"#")[1], ++editnr);
    notempty = read_bytes(file, 0, 1) && 1;

    m_add(editfiles, editid, EFT_FILE, file, retfun, 0);
    lasteditfile = file;
    send_binary_message(sprintf("\e_edit:%s:%d:%s\e\\",editid, notempty, explode(file,"/")[<1]));

    if(notempty)
        call_out(function void()
        {
            if(member(editfiles, editid) && !editfiles[editid, EF_ALIVE])
            {
                this_object()->send_message_to(this_object(), MT_NOTIFY, MA_UNKNOWN,
                    "Editieren fehlgeschlagen.\n");
                call_with_this_player(retfun, 0);
            }
        }, 10, editid);

    return 1;
}

protected void start_webmud3_edit(string file);

static int w_ed(string str)
{
    int flag_ignore = 0;
    if(!str)
        return notify_fail("wed <Dateiname>[ -w]\nwed -o <objekt>[ -w]\n");

    if(!uses_webmud() && !this_object()->query_uses_webmud3())
        return notify_fail("WebMUD ist nicht aktiv.\n");

    if(strstr(space(str)," -w")!=-1)
    {
        flag_ignore = 1;
        str = regreplace(str,"[ ]+-w","",1);
    }
    if(strstr(space(str),"-o")==0)
    {
        str = space(space(str)[2..]);
        object ob = search_object(str,SO_DONT_LOAD);
        if (!objectp(ob))
            return notify_fail("Kein Objekt gefunden.\n");
        str = program_name(ob);
    }
    else if(space(str) == "-")
    {
        if(!sizeof(lasteditfile))
            return notify_fail("Keine Datei zuletzt geöffnet.\n");
        str = lasteditfile;
    }
    else
        str = this_object()->add_path(str);

    if (file_size(str)==-2)
        write("Ein Verzeichnis darf man nicht Editieren!\n");
    else if (!flag_ignore && !MAY_WRITE(str, this_object()))
        write("Keine Schreibrechte auf die Datei.\n");
    else if(file_size(str) > EDIT_LIMIT)
        write("Die Datei ist zu groß.\n");
    else if (this_object()->query_uses_webmud3())
    {
        start_webmud3_edit(str);
        lasteditfile = str;
    }
    else
        webmud_editfile(str, 0);
    return 1;
}

string webmud_edit_loadfile(string editid)
{
    int maxbytes, pos;
#if __VERSION__ > "3.5.2"
    bytes buf, tmp;
#else
    string txt, tmp;
#endif
    string filename;
    SECURE_RPC;

    if(!member(editfiles, editid))
        return 0;

    editfiles[editid, EF_ALIVE] = 1;

    if(editfiles[editid, EF_TYPE] == EFT_TEXT)
         return editfiles[editid, EF_TEXT];

    filename = editfiles[editid, EF_FILE];
    if(!filename)
        return "";

    maxbytes = query_limits()[LIMIT_BYTE] || EDIT_LIMIT;
#if __VERSION__ > "3.5.2"
    buf = to_bytes(({}));

    do
    {
        tmp = read_bytes(filename, pos, maxbytes);
        if(tmp)
        {
            pos += maxbytes;
            buf += tmp;
        }
    } while(tmp && sizeof(buf) < EDIT_LIMIT);

#if __VERSION__ > "3.6.5"
    return to_text(buf, "UTF-8//REPLACE");
#else
    return to_text(buf, "UTF-8");
#endif
#else
    txt = "";

    do
    {
        tmp = read_bytes(filename, pos, maxbytes);
        if(tmp)
        {
            pos += maxbytes;
            txt += tmp;
        }
    } while(tmp && sizeof(txt) < EDIT_LIMIT);

    return convert_umlaute(txt);
#endif
}

int webmud_edit_savefile(string editid, string txt)
{
    int res;

    SECURE_RPC;

    if(!member(editfiles, editid))
        return 0;

    if(editfiles[editid, EF_TYPE] == EFT_FILE)
    {
        if(txt)
        {
            res = write_file(editfiles[editid, EF_FILE], txt, 1);
            this_object()->send_message_to(this_object(), MT_NOTIFY, MA_UNKNOWN,
                res ? wrap("Datei '"+editfiles[editid,EF_FILE]+"' gespeichert.")
                    : wrap("Fehler beim Schreiben von '"+editfiles[editid,EF_FILE]+"'."));
        }
        else
        {
            this_object()->send_message_to(this_object(), MT_NOTIFY, MA_UNKNOWN,
                wrap("Datei '"+editfiles[editid,EF_FILE]+"' wurde nicht gespeichert."));
            res = 1;
        }
        lasteditfile = editfiles[editid, EF_FILE];
    }
    else
        res = 1;

    call_with_this_player(editfiles[editid, EF_RETFUN], txt);
    m_delete(editfiles, editid);

    return res;
}

string webmud_dir()
{
    string path, ret, fret;
    int omitcvs;
    mixed *files;

    SECURE_RPC;

    path = this_object()->query_current_path();
    if(!sizeof(path) || path[<1] != '/')
        path += "/";
    omitcvs = member(({"/d/","/p/","/z/", "/w/","/lo", "/sa"}), path[0..2]) < 0;
    files = get_dir(path, GETDIR_NAMES | GETDIR_SIZES | GETDIR_DATES);

    ret = "<div id=\"dir\"><h2>"+path+"</h2><table>";
    fret = "";
    
    if(path != "/")
    {
        ret += sprintf("<tr class=\"folder\"><td class=\"filename\"><a href=\"#\" class=\"folder\">../</a></td>"
                                            "<td class=\"filedate\">%s</td>"
                                            "<td class=\"filetime\">%s</td>"
                                            "<td class=\"filesize\">Verzeichnis</td></tr>",
                shorttimestr(file_time(path[..<2]), 1, TIMESTR_ONLY_DATE),
                shorttimestr(file_time(path[..<2]), 1, TIMESTR_ONLY_TIME));
    }

    for(int i=2; i < sizeof(files); i+=3)
    {
        if(omitcvs && files[i-2] == "CVS")
        {
            omitcvs = 0;
            continue;
        }

        if(files[i-1] == FSIZE_DIR)
            ret += sprintf("<tr class=\"folder\"><td class=\"filename\"><a href=\"#\" class=\"folder\">%s/</a></td>"
                                                "<td class=\"filedate\">%s</td>"
                                                "<td class=\"filetime\">%s</td>"
                                                "<td class=\"filesize\">Verzeichnis</td></tr>",
                    files[i-2],
                    shorttimestr(files[i], 1, TIMESTR_ONLY_DATE),
                    shorttimestr(files[i], 1, TIMESTR_ONLY_TIME));
        else
            fret += sprintf("<tr class=\"file\"><td class=\"filename\"><a href=\"#\" class=\"file\">%s</a></td>"
                                              "<td class=\"filedate\">%s</td>"
                                              "<td class=\"filetime\">%s</td>"
                                              "<td class=\"filesize\">%d</td></tr>",
                    files[i-2],
                    shorttimestr(files[i], 1, TIMESTR_ONLY_DATE),
                    shorttimestr(files[i], 1, TIMESTR_ONLY_TIME),
                    files[i-1]);
    }

    ret += fret + "</table></div>";
    return ret;
}

void webmud_chdir(string dir)
{
    SECURE_RPC;
    set_current_path(dir);
}

string webmud_start_edit(string editid, string fname)
{
    SECURE_RPC;

    m_add(editfiles, editid, EFT_FILE, fname, 0, 0);
    lasteditfile = fname;
    return webmud_edit_loadfile(editid);
}

// Experimentell, kann sich evntl. noch aendern.
int query_uses_webmud()
{
    return uses_webmud();
}
