// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /i/player/gmcp.c
// Description: Unterstuetzung fuer GMCP

#include <acl.h>
#include <config.h>
#include <eyes.h>
#include <files.h>
#include <invis.h>
#include <level.h>
#include <message.h>
#include <move.h>
#include <room.h>
#include <simul_efuns.h>
#include <stats.h>
#include <tls.h>
#include <telnet.h>
#include "player.h"

#define DEBUGGER "myonara"
#include <debug.h>

#define TPRN (this_object()->query_real_name())
#define GMCP_LOG        50

protected functions inherit "/i/tools/room_types";

// save numpad from gmcp client.
mapping numpad = ([:1]);

nosave mapping packages = ([:1]);
nosave mapping editfiles = ([:3]);
nosave mapping editkeys = ([:1]);
#ifdef GMCP_LOG
nosave string* gmcp_log = ({});
#endif

private void char_items_list();
private void char_items_add(string ctrl, mapping mv_infos) ;
private void char_items_remove(string ctrl, mapping mv_infos);
private void playermap_info(string ctrl, mapping mv_infos);
private varargs void gmcp_send_files_url(string file,string title, int flag);
private mapping gmcp_edit_saved(string tempfile);
private int gmcp_edit_drop_tempfile(string tempfile);
private void numpad_send_level(string prefix);
private void numpad_send_all();
private void numpad_update(string prefix,string key,string value);

static void gmcp_send_dir(string path);
static void gmcp_chdir_dir(string path);
static int has_gmcp();

protected string|string* complete_command(string cmd); // Aus tippse.c



static string dump_gmcp_data()
{
    return sprintf("packages %Q\nnumpad %Q\n",packages,numpad);
}
private void log_gmcp(string msg)
{
#ifdef GMCP_LOG
    gmcp_log = gmcp_log[>-GMCP_LOG..<1] + ({ msg });
#endif
}

private void send_gmcp(string package, string message, varargs mixed* data)
{
    if (!has_gmcp()) return; // nicht dass wir Clients verwirren.
#if __EFUN_DEFINED__(json_serialize)
    string pkg = lower_case(package);
    if (pkg != "core" && !member(packages, pkg))
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
    log_gmcp("Sending: " + msg);
    efun::binary_message( ({ IAC, SE }), 1 );
#endif
}

private void init_gmcp_package(string package)
{
    string path;
    log_gmcp("init: " + package);
    switch (package)
    {
        case "sound":
#ifdef UNItopia
            send_gmcp("Sound", "Url", ([ "url": GMCP_SOUND_URL ]));
#endif
            break;
        case "char":
            if (this_object()==0) return;
            send_gmcp("Char","Name",([ 
                "name":this_object()->query_real_cap_name(),
                "fullname":this_object()->query_short(this_object()),
                "gender":this_object()->query_real_gender(),
                "wizard":wizp(this_object()),
                ]));
            break;
        case "char.items":
            if (this_object()==0) return;
            char_items_list();
            this_object()->add_controller(
                "notify_moved_in", #'char_items_add);
            this_object()->add_controller(
                "notify_moved_out", #'char_items_remove);
            break;
        case "playermap":
            if (this_object()==0) return;
            this_object()->add_controller(
                "notify_moved", #'playermap_info);//');
            playermap_info(0,0);
            break;
        case "files":
            if (this_object() && wizp(this_object()))
            {
                path = "/"+implode(this_object()->compose_path(".",1),"/");
                gmcp_send_dir(path);
            }
            break;
        case "numpad":
            numpad_send_all();
            break;
    }
}

static void receive_gmcp(string data)
{
    log_gmcp("Received: " + data);

#if __EFUN_DEFINED__(json_parse)
    string* words = explode(data, " ");
    mixed args;
    mapping tmpargs=([]);

    if (sizeof(words) > 1)
        args = json_parse(implode(words[1..], " "));

    switch(lower_case(words[0]))
    {
        // Modul Core
        case "core.hello":
            foreach (string fkey,mixed fval: args)
            {
                switch(lower_case(fkey))
                {
                    case "real_ip":
                        log_gmcp("real_ip: "+fval);
                    case "client":
                    case "version":
                        if (this_object())
                        {
                            this_object()->set_webmud3_info(fkey,fval);
                        }
                        break;
                    default:
                        log_gmcp("Unknown BrowserInfo:"+fkey);
                        if (this_object())
                        {
                            this_object()->set_webmud3_info(fkey,fval);
                        }
                        break;
                }
            }
            break;

        case "core.supports.set":
            packages = ([:1]);
            // Fallthrough

        case "core.supports.add":
            foreach (string pkg: args)
            {
                mixed *values = explode(pkg, " ");
                string name;
                int ver;

                if (sizeof(values) != 2)
                    continue;

                ver = to_int(values[1]);
                if (!ver)
                    continue;
                name = lower_case(values[0]);

                m_add(packages, name, ver);
                init_gmcp_package(name);
            }

            break;

        case "core.supports.remove":
            foreach (string pkg: args)
                m_delete(packages, lower_case(pkg));
            break;
        case "core.browserinfo":
            foreach (string fkey,mixed fval: args)
            {
                switch(lower_case(fkey))
                {
                    case "real_ip":
                    case "proxies":
                    case "browser":
                    case "browser_version":
                    case "os":
                    case "os_version":
                    case "useragent":
                    case "ismobile":
                    case "istablet":
                    case "isdesktop":
                    case "client":
                    case "version":
                    case "clientID":
                    case "clientType":
                        if (this_object())
                        {
                            this_object()->set_webmud3_info(fkey,fval);
                        }
                        break;
                    default:
                        log_gmcp("Unknown BrowserInfo:"+fkey);
                        if (this_object())
                        {
                            this_object()->set_webmud3_info(fkey,fval);
                        }
                        break;
                }
            }
            break;
        case "core.ping":
            send_gmcp("Core", "Ping");
            break;
        case "input.complete":
            if (stringp(args))
            {
                string|string* result = complete_command(args);
                if (stringp(result))
                    send_gmcp("Input", "CompleteText", result);
                else if (pointerp(result) && this_object()->query_wiz_level())
                    send_gmcp("Input", "CompleteChoice", result);
                else
                    send_gmcp("Input", "CompleteNone");
            }
            break;

        case "char.items.inv":
            char_items_list();
            break;
        case "files.openfile":
            foreach (string fkey,<string|int> fval: args)
            {
                string name = lower_case(fkey);
                tmpargs[name] = fval;
            }
            
            gmcp_send_files_url(
                tmpargs["file"],
                tmpargs["title"],
                tmpargs["flag"]);
            break;
        case "files.filesaved":
            foreach (string fkey,<string|int> fval: args)
            {
                string name = lower_case(fkey);
                if (name != "file")
                    continue;
                gmcp_edit_saved(fval);
            }
            break;
        case "files.filecanceled":
            foreach (string fkey,<string|int> fval: args)
            {
                string name = lower_case(fkey);
                if (name != "file")
                    continue;
                gmcp_edit_drop_tempfile(fval);
            }
            break;
        case "files.chdir":
            foreach (string fkey,<string|int> fval: args)
            {
                string name = lower_case(fkey);
                if (name != "dir")
                    continue;
                gmcp_chdir_dir(fval);
            }
            break;
        case "numpad.getall":
            numpad_send_all();
            break;
        case "numpad.getlevel":
            foreach (string fkey,<string|int> fval: args)
            {
                string name = lower_case(fkey);
                if (name != "prefix")
                    continue;
                numpad_send_level(fval);
            }
            break;
        case "numpad.update":
            foreach (string fkey,<string|int> fval: args)
            {
                string name = lower_case(fkey);
                tmpargs[name] = fval;
            }
            numpad_update(
                tmpargs["prefix"],
                tmpargs["key"],
                tmpargs["value"]);
            break;
    }
#endif
}

static int has_gmcp()
{
    return this_object() && this_object()->has_telnet_option(TELOPT_GMCP, 0);
}

static void init_gmcp()
{
    if (!has_gmcp())
        return;

    send_gmcp("Core", "Hello", ([ "name": MUD_NAME ]));
}

void transfer_gmcp(string *messages)
{
    if (!previous_object() || program_name(previous_object()) != LOGIN_OB ".c")
        return 0;
    log_gmcp("Reset due to transfer from " + program_name(previous_object()));

    packages = ([:1]);
    foreach (string msg: messages)
        receive_gmcp(msg);
}

#ifdef GMCP_LOG
static void dump_gmcp_log()
{
    write(implode(gmcp_log, "\n") + "\n");
}
#endif

private mapping check_content(mapping attributes, string* keys)
{
    mapping result=([]);
    foreach (string key : keys)
    {
        if (!member(attributes||([]),key))
            return 0;
        result[key] = attributes[key];
    }
    return result;
}

#define MY_LIST_GMCP_CHAR_NAME ({"name","fullname","gender","wizard"})
#define MY_LIST_GMCP_STATUS ({"race","guild","rank"})
#define MY_LIST_GMCP_VITALS ({"hp","maxhp","sp","maxsp","string"})
#define MY_LIST_GMCP_STATS ({"str","int","con","dex"})
#define MY_LIST_GMCP_ITEMS_HEADER ({"location","items"})
#define MY_LIST_GMCP_ITEM_DETAILS ({"name","category"})
#define MY_LIST_GMCP_ITEM_HEADER ({"location","item"})
#define MY_LIST_GMCP_COMM_TELL ({"player","text"})
#define MY_LIST_GMCP_FILES_URL ({"url"})
#define MY_LIST_GMCP_DIR_PWD   ({"path"})
#define MY_LIST_GMCP_DIR_CONTENT_HEADER ({"path","entries"})
#define MY_LIST_GMCP_DIR_CONTENT_ENTRIES ({"name","size",\
                                           "filedate","filetime","isdir"})

static varargs void process_gmcp(mapping attributes,string package, string message)
{
    // Sound.Event implizt.
    string sound = attributes && attributes[MSG_SOUND];
    if (sound)
        send_gmcp("Sound", "Event", ([ "file": sound ]));
    if (!package || !message)
        return;
    mapping data,header,detail;
    int ix;
    log_gmcp("Process: "+package+"."+message);
    switch (lower_case(package+"."+message))
    {
        // Modul core:------------------------------------------
        case "core.goodbye":
            data = check_content(attributes,({"msg"}));
            if (mappingp(data) && stringp(data["msg"]))
                send_gmcp("Core", "Goodbye", data["msg"]);
            return;
        // Modul char:------------------------------------------
        case "char.name":
                    data = check_content(attributes,MY_LIST_GMCP_CHAR_NAME);
                    if (mappingp(data))
                        send_gmcp("Char", "Name", data);
                    return;
        case "char.statusvars":
                    data = ([
                        "race": "Rasse", 
                        "guild": "Gilde", 
                        "rank": "Gildenrang"]);
                    send_gmcp("Char", "StatusVars", data);
                    return;
        case "char.status": // 
                    data = check_content(attributes,MY_LIST_GMCP_STATUS);
                    if (mappingp(data))
                        send_gmcp("Char", "Status", data);
                    return;
        case "char.vitals": // 
                    data = check_content(attributes,MY_LIST_GMCP_VITALS);
                    if (mappingp(data))
                        send_gmcp("Char", "Vitals", data);
                    return;
        case "char.stats": // 
                    data = check_content(attributes,MY_LIST_GMCP_STATS);
                    if (mappingp(data))
                        send_gmcp("Char", "Stats", data);
                    return;
        // Modul char.items:------------------------------------------
        case "char.items.list":
                    header = check_content(attributes,
                        MY_LIST_GMCP_ITEMS_HEADER);
                    if (!mappingp(header) || header["location"] != "inv"
                            || !pointerp(header["items"]) )
                        return;
                    for (ix=0;ix<sizeof(header["items"]);ix++)
                    {
                        detail = check_content(header["items"][ix],
                                MY_LIST_GMCP_ITEM_DETAILS);
                        header["items"][ix] = detail;
                    }
                    send_gmcp("Char.Items","List",header);
                    return;
        case "char.items.add":
                    header = check_content(attributes,
                        MY_LIST_GMCP_ITEM_HEADER);
                    if (!mappingp(header) || header["location"] != "inv"
                            || !mappingp(header["item"]) )
                        return;
                    header["item"] = check_content(header["item"],
                                MY_LIST_GMCP_ITEM_DETAILS);
                    send_gmcp("Char.Items","Add",header);
                    return;
        case "char.items.remove":
                    header = check_content(attributes,
                        MY_LIST_GMCP_ITEM_HEADER);
                    if (!mappingp(header) || header["location"] != "inv"
                            || !mappingp(header["item"]) )
                        return;
                    header["item"] = check_content(header["item"],
                                MY_LIST_GMCP_ITEM_DETAILS);
                    send_gmcp("Char.Items","Remove",header);
                    return;
        // Modul comm:------------------------------------------
        case "comm.say":
            data = check_content(attributes,MY_LIST_GMCP_COMM_TELL);
            if (mappingp(data))
                send_gmcp("Comm", "Say", data);
            return;
        case "comm.soul":
            data = check_content(attributes,MY_LIST_GMCP_COMM_TELL);
            if (mappingp(data))
                send_gmcp("Comm", "Soul", data);
            return;
        case "comm.tell":
            data = check_content(attributes,MY_LIST_GMCP_COMM_TELL);
            if (mappingp(data))
                send_gmcp("Comm", "Tell", data);
            return;
        // Modul files:------------------------------------------
        case "files.url":
            data = check_content(attributes,MY_LIST_GMCP_FILES_URL);
            if (mappingp(data))
                send_gmcp("Files","URL", data );
            return;
        case "files.currentpath":
            data = check_content(attributes,MY_LIST_GMCP_DIR_PWD);
            if (mappingp(data))
                send_gmcp("Files","CurrentPath",data);
            return;
        case "files.directorylist":
            header = check_content(attributes,MY_LIST_GMCP_DIR_CONTENT_HEADER);
            if (!mappingp(header) || !pointerp(header["entries"]) )
                return;
            for (ix=0;ix<sizeof(header["items"]);ix++)
            {
                detail = check_content(header["items"][ix],
                        MY_LIST_GMCP_DIR_CONTENT_ENTRIES);
                if (!mappingp(detail)) 
                    return;
                header["items"][ix] = detail;
            }
            send_gmcp("Files","DirectoryList",header);
            return;
        default:
            return;
    }    
}
private nosave string last_hpsp_str = "";
private nosave string lstats = "";
protected void update_points_display()
{
    if (!interactive() || !has_gmcp() || !member(packages, "char"))
        return;
    int my_sp = this_object()->query_sp();
    int my_hp = this_object()->query_hp();
    int my_maxsp = this_object()->query_max_sp();
    int my_maxhp = this_object()->query_max_hp();
    string hpsp_str = my_hp+" AP("+my_maxhp
            +") und "+my_sp+" "+this_object()->query_sp_short_name()
            +"("+my_maxsp+").";
    if (last_hpsp_str!=hpsp_str)
    {
        last_hpsp_str=hpsp_str;
        process_gmcp(([
            "hp":       my_hp,
            "maxhp":    my_maxhp,
            "sp":       my_sp,
            "maxsp":    my_maxsp,
            "string":   hpsp_str,
            ]),"Char","Vitals");
    }
    string lstr = PRINT_STAT(this_object()->query_stat(STAT_STR,1));
    string lint = PRINT_STAT(this_object()->query_stat(STAT_INT,1));
    string lcon = PRINT_STAT(this_object()->query_stat(STAT_CON,1));
    string ldex = PRINT_STAT(this_object()->query_stat(STAT_DEX,1));
    string lneu = lstr+"/"+lint+"/"+lcon+"/"+ldex;
    if (lneu != lstats) {
        lstats = lneu;
        process_gmcp(([
            "str": lstr,
            "int": lint,
            "con": lcon,
            "dex": ldex,
            ]),"Char","Stats");
    }
}

//---------------------------------------------------------------------
// Inventarfunktionen:
// redundant zu einem Teil von /i/player/inv.c
string object_category(object ob) {
  if (living(ob)) return IC_LIVING;
  if (ob->query_weapon()) return IC_WEAPON;
  if (ob->query_armour()) return IC_ARMOUR;
  if (ob->query_container()) return IC_CONTAINER;
  if (ob->query_cloth()) return IC_CLOTHES;
  if (ob->material("wasser") || ob->material("nahrung")) return IC_FOOD;
  if (ob->query_money()) return IC_MONEY;
  if (ob->query_value()) return IC_VALUEABLES;
  return IC_OTHER;
}

private void char_items_list() {
  object *obs;
  obs = all_inventory(this_object());
  if (!wizp(this_object()))
    obs = filter(obs, (: $1->query_invis() == V_VIS :));
  // TODO: Autoloaderanzeigeeinstellung abfragen und weiter filtern
  //ob->query_inventory_flags() & IF_HIDE_INVENTORY
  process_gmcp( ([
    "location": "inv",
    "items": map(obs, (:
      ([
         "name": $1->query_short(this_object()),
         "category": object_category($1)
      ]) 
    :)
  )]),"Char.Items", "List");
}

private void char_items_add(string ctrl, mapping mv_infos) 
{
  object ob;

  ob = mv_infos[MOVE_OBJECT];
  if (!wizp(this_object()) && ob->query_invis() != V_VIS)
    return;
  process_gmcp( ([
    "location": "inv",
    "item": ([
      "name": ob->query_short(this_object()),
      "category": object_category(ob)
    ])
  ]),"Char.Items", "Add");
}

private void char_items_remove(string ctrl, mapping mv_infos) 
{
  object ob;

  ob = mv_infos[MOVE_OBJECT];
  if (!wizp(this_object()) && ob->query_invis() != V_VIS)
    return;
  process_gmcp( ([
    "location": "inv",
    "item": ([
      "name": ob->query_short(this_object()),
      "category": object_category(ob)
    ])
  ]),"Char.Items", "Remove");
}

// tell,say,soul...
static void handle_gmcp_communication(int msg_type,int msg_action,
    string msg,string verursacher)
{
    string cmd;
    if (msg_action == MA_EMOTE)
    {
        cmd = "Soul";
    }
    else if (msg_action == MA_COMM)
    {
        if (msg_type & MT_FAR)
        {
            cmd = "Tell";
        }
        else
        {
            cmd = "Say";
        }
    }
    else
    {
        return;
    }
    process_gmcp( ([
        "player": verursacher || "-",
        "text":msg,
    ]),"Comm", cmd);
}

// Playermap...
private nosave int flag_data = 0;
private void playermap_info(string ctrl, mapping mv_infos) 
{
#ifdef UNItopia
    // Daten aus der Playermap
    object pmap = present("kokos # playermap", this_object());
    object envtp = environment(this_object());
    if (!pmap || !envtp)
        return;
    mapping data = pmap->query_playermap(this_object());
    if (!data)
    {
        if (flag_data) // nur einmal loeschen, danach ignorieren.
            return;
        flag_data = 1;
    }
    else
    {
        flag_data = 0;
    }
    send_gmcp("Playermap", "Info", ([
        "data": data,
    ]));
#endif
}

protected void gmcp_send_room_info()
{
    object env;

    if (!member(packages, "room"))
        return;

    env = environment();
    if (!env)
        return;

    send_gmcp("Room", "Info", ([
        "name":   env->query_short(),
        "domain": get_displayed_domain(env),
        "exits":  env->query_command_list(EXIT_VISIBLE),
    ]));
}

static string gmcp_get_jwt(string file)
{
#if __EFUN_DEFINED__(get_jwt)
    return get_jwt(MUD_NAME,file||"/UNItopia.debug.log");
#else
    return "Invalid-Token";
#endif
}

public string* get_file_and_dir(string file)
{
    string* xpath = explode(file||"/","/");
    string parent_dir = sizeof(xpath)<=1 ? "/" : implode(xpath[..<2],"/");
    string ftype = "."+explode(xpath[<1],".")[<1];
    if (strstr(xpath[<1],".")<0)
        ftype = "";
    switch (file_size(file)) 
    {
        case FSIZE_DIR:
            return ({ file,xpath[<1], ftype,file });
        case FSIZE_NOFILE:
            return ({ parent_dir,xpath[<1], ftype,0});
        default:
            return ({ parent_dir,xpath[<1], ftype,file});
    }
}

private varargs void gmcp_send_files_url(string file,string title,int flag)
{
    DEBUG(sprintf("%11s:gmcp_send_files_url(%s)",TPRN,file));
    string url = lower_case(GMCP_FILES_BASE_URL) + gmcp_get_jwt(file);
    // DEBUG("url: "+url);
    int flag_newfile = 0,flag_write_access = 0, fsize = 0;
    string* filedir = get_file_and_dir(file);
    switch (fsize=file_size(file)) 
    {
        case FSIZE_DIR:
            log_gmcp("send_files_url.DIR: "+file);
            return;
        case FSIZE_NOFILE:
            flag_newfile = 1;
            break;
        default:
            break;
    }
    flag_write_access = MAY_WRITE(file,this_object());
    send_gmcp("Files", "URL", ([
        "url": url,
        "newfile": flag_newfile,
        "writeacl": flag_write_access,
        "saveactive": (flag&1)?1:0,
        "temporary":(strstr(file,"/var/spool/edit/"+TPRN+"_")==0),
        "filesize": fsize,
        "title":title||"",
        "file":file,
        "path":filedir[0],
        "filename":filedir[1],
        "filetype":filedir[2],
    ]));
}

public varargs void gmcp_edit_file(string file,string title)
{
    gmcp_send_files_url(file,title);
}

/*
FUNKTION: uses_gmcp_edit
DEKLARATION: public int uses_gmcp_edit()
BESCHREIBUNG:
Gibt 1 zurueck, wenn der Editor verfuegbar ist.
VERWEISE: mini_ed,gmcp_start_edit
GRUPPEN: editor
*/
public int uses_gmcp_edit()
{
    return member(packages, "files")
         && this_object()->query_client_option(CLIENT_WEBMUD_NO_EDIT);
}

public string gmcp_edit_get_key(string tempfile,string key)
{
    if (!uses_gmcp_edit())
        return 0;
    if (stringp(tempfile))
    {
        if (MAY_WRITE(tempfile,this_object()))
            return tempfile;
        else
            return 0;
    }
    if (stringp(key))
        return key;
    else
        return 0;
}
/*
FUNKTION: gmcp_start_edit
DEKLARATION: varargs mapping gmcp_start_edit(closure retfun,string title,string tempfile,string* text)
BESCHREIBUNG:
Der gmcp_sart_edit wird vom mini_ed aufgerufen.
retfun ist eine function(string *buffer), buffer ist 0, wenn abgebrochen wurde,
ansonsten enthaelt es das Ergebnis des Editierens.
title sollte eindeutig im Mud sein und gibt den sprechenden Titel des Fensters
z.B. im Webmud3. Ist tempfile ungleich 0, wird das verwendet, 
aber nicht geloescht. Bei gleich 0 wird eine systemseitige temporäre Datei
erstellt, verwendet und nach dem Schliessen wieder geloescht.
Mit dem Stringarray 'text' kann ein Text zur Initialisierung ergaenzt werden.
Das Mapping gibt enweder (["file":tempfile,"key":key,"ok":1]) bei Erfolg
zurueck und (["errortype":"xxx"]); zurueck, bei Fehlschlag, mi xxx als:
GMCPnotAvailable, TempfileOrTitleFailed, CreateTempFile
VERWEISE: mini_ed
GRUPPEN: editor
*/
varargs mapping gmcp_start_edit(closure retfun,string title,
    string tempfile,string* text)
{
    int ok = 0;
    if (!uses_gmcp_edit())
    {
        return (["errortype":"GMCPnotAvailable"]);
    }
    string key = gmcp_edit_get_key(tempfile,title);
    if (!stringp(key))
    {
        return (["errortype":"TempfileOrTitleFailed"]);
    }
    if (!stringp(tempfile) )
    {
        tempfile = "/var/spool/edit/"+TPRN+"_"+
            hash(TLS_HASH_MD5,title+shorttimestr(time())+TPRN,1);
    }
    if (!pointerp(text) || !sizeof(text))
    {
        if (file_size(tempfile)>FSIZE_NOFILE)
            ok = rm(tempfile);
        else
            ok = 1;
    }
    else
    {
        ok = write_file(tempfile,implode(text,"\n"),1);
    }
    if (!ok)
    {
        return (["errortype":"CreateTempFile"]);
    }
    gmcp_send_files_url(tempfile,title);
    editfiles[tempfile,0] = retfun;
    editfiles[tempfile,1] = title;
    editfiles[tempfile,2] = key;
    editfiles[key] = tempfile;
    return (["file":tempfile,"key":key,"ok":1]);
}

private mapping gmcp_edit_saved(string tempfile)
{
    if (!member(editfiles,tempfile))
    {
        return (["errortype":"FileUnknown"]);
    }
    closure cl = editfiles[tempfile,0];
    string key = editfiles[tempfile,2];
    if (!closurep(cl))
    {
        m_delete(editfiles,tempfile);
        m_delete(editkeys,key);
        return (["errortype":"ReturnFunctionUnknown"]);
    }
    if (file_size(tempfile)==FSIZE_NOFILE)
    {
        apply(cl,0,key);
        m_delete(editfiles,tempfile);
        m_delete(editkeys,key);
        return (["errortype":"FileDoesNotExist"]);
    }
    string txt = read_file(tempfile);
    if (!stringp(txt))
    {
        apply(cl,0,key);
        m_delete(editfiles,tempfile);
        m_delete(editkeys,key);
        return (["errortype":"ErrorReadingFile"]);
    }
    apply(cl,explode(txt,"\n"),key);
    m_delete(editfiles,tempfile);
    m_delete(editkeys,key);
    return (["ok":1]);
}

private int gmcp_edit_drop_tempfile(string tempfile)
{
    if (!member(editfiles,tempfile))
    {
        return 0;
    }
    if (!rm(tempfile))
        return 0;
    string key = editfiles[tempfile,2];
    m_delete(editfiles,tempfile);
    m_delete(editkeys,key);
    return 1;
}

protected void gmcp_edit_drop_all_tempfiles()
{
    string tempfile,title,key;
    closure cl;
    foreach(tempfile,cl,title,key : editfiles)
    {
        if (closurep(cl))
            apply(cl,0,tempfile,key);
    }
    editfiles = ([:3]);
    editkeys = ([:1]);
}

static void gmcp_send_path(string path)
{
    send_gmcp("Files", "CurrentPath", ([
        "path": path||"/",
    ]));
}

static void gmcp_chdir_dir(string path)
{
    DEBUG(sprintf("%11s:gmcp_chdir_dir(%s)",
        TPRN,path));
    this_object()->cd(path);
}

static void gmcp_send_dir(string path) 
{
    if (!member(packages, "files") && wizp(this_object()))  {
        // DEBUG("files switched off fur gmcp_send_dir");
        return; // short cut.
    }
    DEBUG(sprintf("%11s:gmcp_send_dir(%s)",TPRN,path));
    path ||= "/";
    if (path[<1..<1]!="/")
        path += "/";
    if (path[0..0]!="/")
        path = "/" + path;
    int tmptime;
    int omitcvs = member(({"/d/","/p/","/z/","/w/","/lo","/sa"}),path[0..2])<0;
    
    mapping data = (["path":path]);
    mapping *entries = ({});
    mixed* files;
    if (path != "/") {
        tmptime = file_time(path[..<2]);
        entries += ({([
            "name":"../",
            "size":-1,
            "filedate": shorttimestr(tmptime, 1, TIMESTR_ONLY_DATE),
            "filetime": shorttimestr(tmptime, 1, TIMESTR_ONLY_TIME),
            "isdir":1,
        ])});
    }
    files = get_dir(path, GETDIR_NAMES | GETDIR_SIZES | GETDIR_DATES);
    for(int i=2; i < sizeof(files); i+=3)
    {
        if(omitcvs && files[i-2] == "CVS")
        {
            omitcvs = 0;
            continue;
        }

        if(files[i-1] == FSIZE_DIR)
            entries += ({([
                "name" : files[i-2],
                "filedate" : shorttimestr(files[i], 1,TIMESTR_ONLY_DATE),
                "filetime" : shorttimestr(files[i], 1,TIMESTR_ONLY_TIME),
                "size" : -1,
                "isdir" : 1,
            ])});
        else
            entries += ({([
                "name" : files[i-2],
                "filedate" : shorttimestr(files[i], 1,TIMESTR_ONLY_DATE),
                "filetime" : shorttimestr(files[i], 1,TIMESTR_ONLY_TIME),
                "size" : files[i-1],
                "isdir" : 0,
            ])});
    }
    data["entries"] = entries;
    send_gmcp("Files", "DirectoryList",data);
}

private void numpad_send_level(string prefix)
{
    if (!member(packages, "numpad"))  {
        return; // short cut.
    }
    mapping data = ([ "prefix" : prefix ]);
    if (member(numpad,prefix)) {
        data["keys"] = numpad[prefix];
    } else {
        data["keys"] = ([]);
    }
    send_gmcp("Numpad","SendLevel", data);
}

private void numpad_send_all()
{
    if (!member(packages, "numpad"))  {
        return; // short cut.
    }
    string* keys = m_indices(numpad);
    foreach (string prefix: keys) {
        numpad_send_level(prefix);
    }
}

private void numpad_update(string prefix,string key,string value)
{
    if (!member(packages, "numpad"))  {
        return; // short cut.
    }
    if (!member(numpad,prefix)) {
        numpad[prefix] = ([ key:value ]);
    } else {
        mapping kv = numpad[prefix];
        kv[key] = value;
        numpad[prefix] = kv;
    }
}