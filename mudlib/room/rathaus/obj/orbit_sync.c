// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/room/rathaus/obj/object_sync.c
// Description: temporäares Tool zum Synchroniseren von
//              autoloader und zauberstab Daten inkl Plugins.
// Author:	Myonara

inherit "/i/tools/trust";
inherit "/i/tools/security";
inherit "/i/item";
inherit "/i/install";
inherit "/i/tools/getopt";

#include <level.h>
#include <message.h>
#include <misc.h>
#include <move.h>
#include <notify_fail.h>
#include <room_types.h>

#define CALC_EVALS 500000
#define ID_ORBIT_SYNCER "orbit # syncer"

private nosave mapping data_storage = 0;

static void loading_autoloader(mixed *al, mapping param, object tp)
{
    string err;
    object ob;
    if (!playerp(tp)) return;
    while (get_eval_cost() > CALC_EVALS && sizeof(al))
    {
        // printf("al2=%d: %Q",sizeof(al),al[0]);
        if (!find_object(al[0][0]))
        {
            if (err = catch(ob = touch(al[0][0])) || !ob)
            {
                send_message_to(tp,MT_NOTIFY,MA_UNKNOWN,
                    "Fehler "+err+" beim Laden von "+al[0][0]);
                al = al[1..];
                continue; // pruefen auf eval und Schleifenbedingung...
            }
        }
        if (ob = present_clone(al[0][0],tp))
        {
            if(member(param,"o"))
            {
                ob->init_arg(al[0][1]);
                send_message_to(tp,MT_NOTIFY,MA_UNKNOWN,
                    "Reinitialisiert: "+al[0][0]);
            }
            else
            {
                send_message_to(tp,MT_NOTIFY,MA_UNKNOWN,
                    "Unveraendert: "+al[0][0]);
            }
        }
        else
        {
            if (err = catch(ob = clone_object(al[0][0])) || !ob)
            {
                send_message_to(tp,MT_NOTIFY,MA_UNKNOWN,
                    "Fehler "+err+" beim Clonen von "+al[0][0]);
                al = al[1..];
                continue; // pruefen auf eval und Schleifenbedingung...
            }
            ob->move_or_remove(tp);
            if (ob)
            {
                ob->init_arg(al[0][1]);
                send_message_to(tp,MT_NOTIFY,MA_UNKNOWN,
                    "Geladen und Initialisiert: "+al[0][0]);
            }
            else
            {
                send_message_to(tp,MT_NOTIFY,MA_UNKNOWN,
                    "Fehler beim Move von "+al[0][0]);
            }
        }
        al = al[1..];
    }
    if (sizeof(al)) call_out("loading_autoloader",1,al,param,tp);
}

mapping get_data(object sender)
{
    if (load_name(PO) != load_name(TO)) return ([]);
    if (sender == ENV_TO)
    {
        return data_storage||([]);
    }
    sender ||= TP;
    mapping data = ([]);
    mixed *al = filter(map(all_inventory(sender), 
                (: ({ load_name($1),$1->query_auto_load() }) :) ),
                    (: $1[1] != 0 :) );
    if (sizeof(al)) data["autoload"]= al;
    mapping pl = sender->query_zauberstab_info();
    // printf("get_data-pl: %Q\n",pl);
    if (sizeof(pl)) data["plugins"]=pl;
    return data;
}


int verbindung(string str)
{
    string source,target;
    int receiver_flag;
    if (!trusted())
        FAILWP(Dem()+" bitte vertrauen.", FAIL_INTERNAL);
    if (!playerp(TP) || TP != TI) 
        FAILWP("Selbst ausfuehren ist die Devise.",FAIL_INTERNAL);
#ifdef Orbit
    if (TP->is_intermud_guest())
    {
        source = explode(TP_RN,"@")[0];
        target = TP_RN;
        receiver_flag = 1;
    }
    else
    {
        source = TP_RN;
        target = TP_RN +"@unitopia";
        receiver_flag = 0;
    }
    object sender = find_player(source);
    if (!wizp(sender)) 
        FAILWP("Kein Sender eingeloggt.",FAIL_INTERNAL);
    object tool_sender = present_clone(load_name(TO),sender);
    if (!objectp(tool_sender))
        FAILWP("Der Sender hat kein Werkzeug",FAIL_INTERNAL);
    object receiver = find_player(target);
    if (!wizp(receiver)) 
        FAILWP("Kein Empfänger eingeloggt.",FAIL_INTERNAL);
    object tool_receiver = present_clone(load_name(TO),receiver);
    if (!objectp(tool_receiver))
        FAILWP("Der Empfänger hat kein Werkzeug",FAIL_INTERNAL);
    object z_receiver = present_clone("/obj/zauberstab",receiver);
    if (receiver_flag)
    {
        mapping data = tool_sender.get_data(sender);
        if (!mappingp(data))
            FAILWP("Bitte im Sender 'zverb' aufrufen "
                    "zur Bereitstellung der Daten.",FAIL_INTERNAL);
        //printf("zverb-data:%Q\n",data);
        mapping opt = getopt(str, ([ "p":0,"a":0,"o":0 ]) );
        if (member(opt,"errors"))
            FAILWP(opt["errors"][0], FAIL_INTERNAL);
        if (member(opt,"p"))
        {
            mapping pl = data["plugins"];
            printf("zverb-p: %Q\n",pl);
            if (mappingp(pl) && sizeof(pl))
            {
                if (z_receiver) z_receiver.remove();
                receiver.set_zauberstab_info(pl);
                z_receiver = clone_object("/obj/zauberstab");
                z_receiver.move_or_remove(receiver);
                send_message_to(receiver,MT_NOTIFY, MA_UNKNOWN, "Plugin-Daten gesetzt.");
                return 1;
            }
            else
            {
                FAILWP("Keine Plugin-Daten gesetzt!",FAIL_INTERNAL);
            }
        }
        else if (member(opt,"a"))
        {
            mixed *al = data["autoload"];
            send_message_to(receiver,MT_NOTIFY, MA_UNKNOWN, "SyncLoad gestartet: "+sizeof(al));
            loading_autoloader(al,opt,receiver);
            return 1;
        }
        FAILWP("zverbindung [-a|-p]\nEine der Optionen -a für Autoloader oder "
            "-p fuer Plugins muss angegeben sein.",FAIL_INTERNAL);
    }
    else
    {
        data_storage = get_data(0);
        send_message_to(sender,MT_NOTIFY, MA_UNKNOWN, 
            "Daten für Orbittbesucher gesetzt.");
        return 1;
    }
#else
    FAILWP("Funktuniert nur in Orbit mit "+TP_RN+"@unitopia und "
        +TP_RN+" zusammen eingeloggt und dieses Tool.",
        FAIL_INTERNAL);
#endif
}

void init()
{
    "*"::init();
    if (TP == ENV_TO && gesellep(ENV_TO))
    {
        add_action("verbindung","zverbindung",-5);
    }
    else if (!gesellep(ENV_TO))
    {
        call_out("remove",0);
    }
}

void create() 
{
    
    "*"::create();
    set_name("orbitsyncer");
    set_gender("maennlich");
    set_id( ({ "sync-tool","syncer","sync","orbitsyncer",ID_ORBIT_SYNCER}) );
    set_long("Dies ist das Synchronisierungstool fuer Autoloader und "
             "Plugins innerhalb von Orbit. Eine Anleitung zum lesen ist dabei.");
     
    add_v_item( ([
        "name" : "anleitung",
        "gender" : "weiblich",
        "id" : ({ "anleitung"}),
        "long" : "Eine Anleitung zum Lesen.",
        "read":
"Der Sender loggt sich direkt in Orbit mit seinem GottChar ein, "
"der Empfänger kommt durch das Portal im Forum oder per zgorbit. "
"Das Tool ist nur zum einmaligen Verwenden gedacht. Folgende Anweisungen "
"sind von dem Sender bzw Empfänger in dieser Reihenfolge durchzuführen:\n"
"Sender> zg forum\nSender> nische\nSender> drücke 6\n"
"Sender> vertraue sync\nSender> zverbindung\n"
"Empfänger> zgorbit forum\nEmpfänger> nische\nEmpfänger> drücke 6\n"
"Empfänger> vertraue sync\n"
"Empfänger> zverbindung -p\nEmpfänger> zverbindung -a\n"
"Wobei -p die plugins und andere Zauberstabdaten lädt, -a die Autoloader\n"
"Sender> zz sync\nEmpfänger> zz sync\n",
    ]) );
    init_security_for_actions();
}
void prepare_renewal()
{
}

void abort_renewal()
{
}

void finish_renewal(object neu)
{
}