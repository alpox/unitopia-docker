// This file is part of UNItopia mudlib.
// -------------------------------------------------------------------
// File:        /room/bank/touch/warenkorb.c
// Description: Warenkorb zur Freigabe zu einlagerbaren Objekten 
// Author:      Myonara (21.Jan.2020)

inherit "/i/tools/security";
inherit "/i/object/kiste";

#include <apps.h>
#include <config.h>
#include <level.h>
#include <message.h>
#include <misc.h>
#include <more.h>
#include <move.h>
#include <notify_fail.h>
#include <properties.h>

#include <money.h>

private object last_ob;
private string last_msg,last_data;

private string get_program_name(object ob)
{
    string fn = ob->query_file_name();
    if (fn) return fn;
    string *name = explode(object_name(ob),"#");
    return (sizeof(name)==2)?name[0]:0;
}

private string get_full_check(object ob,object tester)
{
    string rstr = wrap("Geprüft wird: "+der(ob));
    string *bstr = ({});
    object who = TI || TP;
    string unique_clone = get_program_name(ob) || program_name(ob)[..<3];
    int flags = 0;
    <string|int> istr = ZENTRALBANK->all_check_conservation(ob,([
        ZB_CHECK_TYPE: ZB_CT_TEST_ONLY,
        ZB_CHECK_CON:this_object(),
        ZB_CHECKER_OB:who,
    ]));
    if (stringp(istr))
    {
        last_ob = ob;
        last_msg = istr;
        mixed mud_data,m_constraints;
        m_constraints = ob->query_conservation_constraints();
        mud_data = ob->query_conservation_data();
        last_data = "contraints: "+mixed2str(m_constraints)
                  + "muddata: "+mixed2str(mud_data);
        set(P_DEBUG_GROUP,MASTER_OB->query_debugger(
                get_program_name(ob)+".c"));
        rstr += wrap("Test fehlgeschlagen:\n"+istr);
    } else {
        last_ob = 0;
        last_msg = last_data = "";
        set(P_DEBUG_GROUP,({}));
        rstr += wrap("Test erfolgreich, "+der(ob)+plural(" ist"," sind",ob)
                    +" einlagerbar.");
        return rstr;
    }
    flags = ZENTRALBANK->get_flags_for_bank_safe_file(
        unique_clone||get_program_name(ob));
    if (last_ob && ob->query_auto_load() && (flags&ZB_F_AUTOLOADER)==0)
    {
        rstr += wrap("- 'Freigabe Autoloader' fehlt.");
        bstr += ({"autoloader"});
    }
    if (last_ob && ob->query_no_store() && (flags&ZB_F_WAREHOUSE_STORE)==0)
    {
        rstr += wrap("- 'Freigabe no_store' fehlt.");
        bstr += ({"no_store"});
    }
    mapping fil = ZENTRALBANK->get_one_bank_safe_filter(
        unique_clone||get_program_name(ob));
    if (last_ob && (!mappingp(fil) || (fil[ZB_BANK_FLAGS]&ZB_BANK_ACTIVE)==0))
    {
        rstr += wrap("- 'Freigabe allgemein' fehlt.");
        bstr += ({"allgemein"});
    }
    if (sizeof(bstr) > 1) 
    {
        rstr += wrap("Beim Befehl freigabe können mehrere Typen "
            "auf einmal angegeben werden. "+
            "In diesem Fall:\nfreigabe "+implode(bstr," "));
    } else if (sizeof(bstr) == 0) 
    {
        rstr += wrap("Freigaben erteilt, aber ein programtechnischer "
            "Grund liegt noch vor, wie z.B. ein fehlendes "
            "clear_initial_conservation_data(); am Ende vom Create oder "
            "ein Shadow (=Wandlung) verhindert das Einlagern. Shadows "
            "können nicht durch den Warenkorb freigegeben werden. "
            "=> lies rilis");
    }
    return rstr;
}

string precheck_conservation(mapping check_attributes)
{
    return 
    "Das ist nur ein Warenkorb, das nicht aufbewahrt werden kann.";
} 

<int|string> forbidden_move_in(mapping mv_infos)
{
    if (sizeof(all_inventory(TO))>0)
    {
        return "Es ist schon ein Objekt im Warenkorb.";
    }
    return 0;
}

void notify_moved_in(mapping mv_infos)
{
    object ob = mv_infos[MOVE_OBJECT];
    object who = mv_infos[MOVE_OLD_ROOM] || TI;
    if (who) {
        send_message_to(who,MT_UNKNOWN,MA_UNKNOWN,get_full_check(ob,who));
    }
}

string query_long(object who)
{
    string txt = ::query_long(who);
    object *obs = all_inventory(TO);
    if (sizeof(obs)>0)
    {
        txt += get_full_check(obs[0],who);
    }
    return txt;
}

int cmd_freigabe(string str)
{
    if (!check_security()) return 0;
    if (!wizp(TP)) return 0;
    
    object *obs = all_inventory(TO);
    string pn,*dir,*dom,tmp;
    int flags = 0;
    if (sizeof(obs)==0)
    {
        FAILWP("Objekt muss im Warenkorb zur Prüfung liegen.",FAIL_NOT_OBJ);
    }
    pn = program_name(obs[0]);
    
    if (!adminp(TP))
    {
        dir = explode(pn,"/");
        switch(dir[1])
        {
            case "d":
                if (DOMAIN_INFOS->domain_lord(dir[2],TP_RN))
                    break;
                FAILWP("Bist kein Domain-Lord von "+dir[2]+".",FAIL_INTERNAL);
            case "z":
                dom = FILED->query_auth_of(TP_RN);
                if (member(dom,lower_case(dir[2]))!=-1)
                    break;
                FAILWP("Bist kein "+dir[2]+"-Lord.",FAIL_INTERNAL);
            case "p":
                dom = FILED->query_auth_of(TP_RN);
                if (member(dom,"p")!=-1)
                    break;
                FAILWP("Bist kein P-Lord.",FAIL_INTERNAL);
            default:
                FAILWP("Bist kein zuständiger Lord/Admin.",FAIL_INTERNAL);
        }
    }
    str = space(str);
    if (str == "") 
    {
        send_message_to(TP,MT_UNKNOWN,MA_UNKNOWN,get_full_check(obs[0],TP));
        return 1;
    }
    dom = explode(str," ");
    foreach (tmp:dom)
    {
        switch(lower_case(tmp))
        {
            case "allgemein":
                flags |= ZB_BANK_ACTIVE;
                continue;
            case "no_store":
            case "nostore":
                flags |= ZB_F_WAREHOUSE_STORE;
                continue;
            case "autoloader":
                flags |= ZB_F_AUTOLOADER;
                continue;
            default:
                FAILWP("freigabe nimmt nur [allgemein |no_store | autoloader]+ "
                    "entgegen. Sprich einzeln oder "
                    "mit Leerzeichen aneinandergereiht.",FAIL_INTERNAL);
        }
    }
    // release_bank_safe_file
    pn = pn[..<3];
    mapping fil = ZENTRALBANK->get_one_bank_safe_filter(pn);
    if (fil == 0)
    {
        if (!ZENTRALBANK->release_bank_safe_file(pn))
        {
            FAILWP("Allgemeine Freigabe fehlgeschlagen.",FAIL_INTERNAL);
        }
    } else {
        flags |= fil[ZB_BANK_FLAGS];
    }
    if (!ZENTRALBANK->set_one_bank_safe_filter(pn,([ZB_BANK_FLAGS:flags ]) ) )
    {
        FAILWP("Fehler beim setzen der Sonder-Freigaben",FAIL_INTERNAL);
    }
    sys_log("armatester",shorttimestr(time())+" "
                +TP_RN+":Freigabe: "+pn+"("+obs[0]->query_short()+")="
                +flags+"\n");
    send_message_to(TP,MT_UNKNOWN,MA_UNKNOWN,"Ok.\n"+get_full_check(obs[0],TP));
    return 1;
}

string read_rilis(string parse_rest, string str, 
        mapping was, object leser)
{
    leser->more(ZB_RILIS,0,0,M_AUTO_END);
    return "";
}


void init()
{
    "*"::init();
    add_action("cmd_freigabe","freigabe");
}

void create() 
{
    set_name("warenkorb");
    set_id(({"warenkorb","korb",}));
    set_gender("maennlich");
    set_weight(0);
    set_long("Ein Warenkorb zum Prüfen und Setzen von Freigaben "
             "durch das Bankenkonsortitum. "
             "Für Programmierrichtlinien 'lese rilis'.");
    set(P_LOOK_MSG,"");
    set_no_door(1);
    add_v_item( ([ // Kann gefuellt/Geaendert werden
        "name" : "programmierrichtlinien",
        "gender" : "weiblich",
        "id" : ({ "rilis","richtlinien","programmierrichtlinien"}),
        "plural" : 1,
        "long" : "Rilis zum Lesen.",
        "look_msg" : "",
        "read" : #'read_rilis,
        "read_msg" : "",
    ]) );
    add_controller("forbidden_move_in");
    add_controller("notify_moved_in");
    init_security_for_actions();
}

mapping query_debug_info()
{
    mapping dbg = "/i/object/kiste"::query_debug_info();
    if (objectp(last_ob))
    {
        dbg["armatester:first_room"] = last_ob->query_first_room();
        dbg["armatester:object_info"] = mixed2str(last_ob->query_debug_info());
        dbg["armatester:last_ob"] = mixed2str(last_ob);
        dbg["armatester:last_msg"] = last_msg;
        dbg["armatester:last_data"] = last_data;
    }
    return dbg;
}

void prepare_renewal() {}
void abort_renewal() {}
void finish_renewal(object neu) {}
