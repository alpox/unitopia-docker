// This file is part of UNItopia mudlib.
// -------------------------------------------------------------------
// File:        /room/bank/obj/schliessfach.c
// Description: Schliessfachobjekt fuer die Banken.
// Author:      Myonara (24.Apr.2016)

inherit "/i/object/kiste";

#include <database.h>
#include <invis.h>
#include <level.h>
#include <message.h>
#include <misc.h>
#include <move.h>
#include <notify_fail.h>
#include <object_info.h>
#include <properties.h>

#include <money.h>

#define COUNT_FILE(f,c) if (member(c,f)) c[f]++; else c[f]=1;

private object owner,env;
private string bank_id,valuta,valutas,v_gender;
private closure calc_rent;
private string n_schliessfachmiete;
private closure c_leer_verschwindet,c_long_leer;

public nomask int is_bank_safe()
{
    return 1;
}

int query_no_move()
{
    return 1;
}

string query_no_move_reason()
{
    return wrap(Der(TO)+" lässt sich nicht bewegen.");
}

int forbidden_shadow(object shadow, object victim)
{
    return victim == TO;
}

int remove()
{
    if (!TO) return 1;
    all_inventory()->remove();
    return ::remove();
}

// aus /p/Item/Moebel/sys/p_container.inc kopiert und angepasst:
mapping get_one_detail(object what)
{
    int container_flag = 0,reset_flag = 0;
    mapping detail = ([]);
    container_flag = what->query_container();
    if (   container_flag  && what->query_con_close()
        && !what->query_transparent())
    {
        reset_flag = 1;
        what->set_transparent(1);
    }
    detail = ([
       "environment" : TO,
          "origin"   : what,
        "debug_info" : (what->query_debug_info()||([]))+
                         (["original" : object_name(what) ]),
          "prep"     : "in",
//        "name"     : lower_case(wer(what,ART_BLANK|ART_NO_ADJEKTIV)),
          "name"     : what->query_name(),
          "cap_name" : what->query_cap_name(),
          "gender"   : what->query_gender(),
          "adjektiv" : what->query_adjektiv(),
          "id"       : (what->query_id()||({}))+({"relief"}),
          "short"    : what->query_short(TO),
          "long"     : "Der Inhalt "+des(TO,"")+" ist:\n"
                       +(container_flag ? TP->query_object_description(what)
                         : what->query_long(TO)),
          "look_msg" : "",
          "read_msg" : "",
          "take_msg" : "",
          "feel_msg" : "",
          "smell_msg": "",
          "hear_msg" : "",

          ]);
    if (reset_flag)
    {
        what->set_transparent(0);
    }
    foreach(string key : ({ "seher_info", }))
    {
        string tmp;
        if(tmp=call_other(what,"query_"+key))              
        {
            detail += ([ key : tmp ]);
        }
    }
    foreach(string key : ({ "eigen", "personal", "invis", "plural", }))
    {
        int tmp;
        if(tmp=call_other(what,"query_"+key))              
        {
            detail += ([ key : tmp ]);
        }
    }
    foreach(string key : ({ "menge", }))
    {
        mixed tmp;
        if(tmp=call_other(what,"query_"+key))              
        {
            detail += ([ key : tmp ]);
        }
    }
    return detail;
}

private float get_item_rent_for_container(object con)
{
    object *ai = all_inventory(con),ob;
    float item_rent = ZENTRALBANK->get_conservation_item_rent(con,1);
    foreach (ob : ai)
    {
        item_rent += ZENTRALBANK->get_conservation_item_rent(ob,1);
    }
    return item_rent;
}

private string get_miet_string(object who,object ob)
{
    float item_rent = get_item_rent_for_container(ob);
    float bank_rent = apply(calc_rent,who,ob,
        ZENTRALBANK->get_current_safe_count(RNAME(who),bank_id));
    return
sprintf(
"Gegenstandsmiete ca. %s, %s ca. %s pro RL-Woche.",
ZENTRALBANK->get_value_in_current_valuta(item_rent,valuta,valutas,v_gender),
n_schliessfachmiete||"Schließfachmiete",
ZENTRALBANK->get_value_in_current_valuta(bank_rent,valuta,valutas,v_gender));
}

// eingefuehrt, da program_name replace_program nicht erwischt :(
private string get_program_name(object ob)
{
    string fn = ob->query_file_name();
    if (fn) return fn;
    string *name = explode(object_name(ob),"#");
    return (sizeof(name)==2)?name[0]:0;
}

private mapping einlagerungsdaten(object ob,object b,mapping file_counter)
{
    object sh;
    <string|int> str;
    string prefix = (b?(Der(ob)+" lässt sich in "+dem(b)
            +" nicht einlagern. Grund: "):"");
    string factory,identifier;
    mapping * sh_data,m_cons,m_origin,m_constraints,ob_properties;
    mixed ob_data,mud_data;
    int count;

    if (stringp(str = ZENTRALBANK->precheck_conservation(ob,0,([
        ZB_CHECK_TYPE:ZB_CT_SCHLIESSFACH,
        ZB_CHECK_CON:this_object(),
        ZB_CHECKER_OB:this_object()
        ]) )))
    {
        send_message_to(owner, MT_NOTIFY, MA_UNKNOWN,wrap(prefix+str));
        return 0;
    }
    if (!intp(str) || str != 1)
    {
        ob->abort_conservation();
        ZB_DEBUGLOG(sprintf("str=%Q",str),DB_DBGLVL_ERROR,RNAME(owner),
            "schließfach:behaelter_einlagern");
        return 0;
    }
    m_cons = ob->query(P_CONSERVATION)||([]);
    factory = m_cons[P_CONSERVATION_FACTORY];
    identifier = m_cons[P_CONSERVATION_IDENTIFIER];
    m_origin = ob->query(P_ORIGIN); // Orginal verwenden falls vorhanden
    switch (sizeof(m_origin))
    {
        case 0:
            m_origin = ([ 
                P_ORIGIN_ROOM : ob->query_first_room(),
                P_ORIGIN_PLAYER : ob->query_first_player(),
                P_ORIGIN_CREATED_ON : object_time(ob),
                P_ORIGIN_CREATOR: ob->query_creator(),
                ]);
            break;
        case 3:
            m_origin[P_ORIGIN_CREATOR] = ob->query_creator();
        case 4:
        case 5:
            break;
    }
    sh_data = ({});
    for (count=0,sh=ob;sh=object_info(sh, OI_SHADOW_NEXT);count++)
    {
        mixed m = sh->query_conservation_arg_sh(sh);
        string pn = get_program_name(sh);
        if (m && pn)
        {
            sh_data += ({ ([ pn : m ]) });
            COUNT_FILE(pn,file_counter);
        }
    }
    if (sizeof(sh_data)!=count)
    {
        ZB_DEBUGLOG(sprintf("shadows-reject=%Q,%d,%Q",ob,count,sh_data),
            DB_DBGLVL_INFO,RNAME(owner),"schließfach:einlagern");
        ob->abort_conservation();
        send_message_to(owner, MT_NOTIFY, MA_UNKNOWN,wrap(prefix+
            "Eine Art Fluch verhindert aktuell leider noch die Einlagerung "
            +des(ob)+"."));
        return 0;
    }
    if (stringp(factory))
    {
        ob_data = touch(factory)->get_conservation_data(ob);
        COUNT_FILE(factory,file_counter);
    }
    else if (function_exists("query_conservation_arg",ob))
    {
        ob_data = ob->query_conservation_arg();
    }
    else
    {
        ob_data = 1;
    }
    m_constraints = ob->query_conservation_constraints();
    mud_data = ob->query_conservation_data();
    if (sizeof(m_constraints)>0 || !ob_data || !mud_data)
    {
        ZB_DEBUGLOG(sprintf("object-reject=%Q",ob),
            DB_DBGLVL_INFO,RNAME(owner),"schließfach:einlagern");
        ob->abort_conservation();
        send_message_to(owner, MT_NOTIFY, MA_UNKNOWN,wrap(prefix+
            Der(ob)+" lässt sich prinzipiell nicht einlagern."));
        return 0;
    }
    sh_data = filter(sh_data, function int (mapping m)
        {
            string key;
            mixed data;
            foreach (key,data : m)
            {
                if (data==0) return 0;
            }
            return 1;
        } );
    if (ob->query_container())
    {
        if (ob->query_con_close())
        {
            mud_data += ({({ "close_con", ({}) })});
        }
        else
        {
            mud_data += ({({ "open_con", ({}) })});
        }
    }
    ob_properties = ZENTRALBANK->query_saved_properties_for_conservation(ob);
    COUNT_FILE(get_program_name(ob),file_counter);
    return get_one_detail(ob) + ([ 
        ARMA_FACTORY : factory,
        ARMA_FACTORY_ID : identifier,
        ARMA_SHADOW_DATA:sh_data, 
        ARMA_OBJECT_DATA:ob_data,
        ARMA_MUDLIB_DATA:mud_data,
        ARMA_PROPERTIES_DATA:ob_properties,
        ARMA_LOAD_FILE : get_program_name(ob) || object_name(ob),
        ARMA_SPECIAL_ID : ":"+time()+":"+bank_id+":"+RNAME(owner),
        ARMA_ORIGIN_INFO : m_origin,
        ARMA_FILE_COUNTER : file_counter,
        ]);
}

private mapping* behaelter_einlagern(object b,mapping file_counter)
{
    object * ai = all_inventory(b),ob;
    mapping* result = ({}),m;
    if (living(b)) return result;
    foreach (ob : ai)
    {
        m = einlagerungsdaten(ob,b,file_counter);
        if (m==0) return 0;
        result += ({m});
    }
    return result;
}

static void einlagern()
{
    object *ai = all_inventory()-({0}),ob;
    mapping m,*content_data,m_cons,vi,file_counter = ([]);
    string str,rentstr;
    int trophey_flag;
    float item_rent,bank_rent;
    if (sizeof(ai)==0)
    {
        // Leeres Schliessfach einfach entfernen
        if (owner && env && owner == ENV_TO && env == ENV(TP))
        {
            send_message_to(owner,MT_NOTIFY,MA_UNKNOWN,wrap(
                c_leer_verschwindet? closure_to_string(c_leer_verschwindet) :
                (Der(TO, ({({"leer"}),({"geschlossen"})}) ) + 
                " entschwindet in der Wand.") ));
                // Das leere, geschlossene Schliessfach 
        }
        remove();
        return;
    }
    ob = ai[0];
    if (!ob || !owner || owner != ENV_TO)
    {
        open_con();
        ZB_DEBUGLOG(sprintf("ai=%Q",ai),DB_DBGLVL_ERROR,RNAME(owner||TP),
            "schließfach:einlagern");
        send_message_to(owner,MT_NOTIFY,MA_UNKNOWN,wrap(
            Der(TO,"")+ " öffnet sich wieder, Du solltest "
            "Deinen Gegenstand daraus entnehmen."));
        return;
    }
    m = einlagerungsdaten(ob,0,file_counter);
    content_data = behaelter_einlagern(ob,file_counter);
    if (m==0 || content_data == 0)
    {
        open_con();
        ob->abort_conservation();
        all_inventory(ob)->abort_conservation();
        return;
    }
    m_cons = ob->query(P_CONSERVATION)||([]);
    trophey_flag = (m_cons[P_CONSERVATION_TROPHEY]!=0);
    item_rent = get_item_rent_for_container(ob);
    bank_rent = apply(calc_rent,owner,ob,
        ZENTRALBANK->get_current_safe_count(RNAME(owner),bank_id));
    vi = m + ([ 
        ARMA_CONTENT_DATA:content_data,
        ]);
    str = Der(TO,"")+" mit "+dem(ob)+" wurde sicher vor Armageddon eingelagert.";
    if (!ZENTRALBANK->enter_bank_safe(bank_id,RNAME(owner),vi[ARMA_SPECIAL_ID],
        ({vi[ARMA_LOAD_FILE]}) + map(vi[ARMA_SHADOW_DATA],(: m_indices($1)[0] :) ), 
        vi,ob->query_value(),vi[ARMA_FACTORY],vi[ARMA_FACTORY_ID],
        file_counter))
    {
        send_message_to(owner, MT_NOTIFY, MA_UNKNOWN,wrap(
            "Die Einlagerung ist gestört, bitte Gegenstand wieder aus dem Fach nehmen."));
        open_con();
        ob->abort_conservation();
        all_inventory(ob)->abort_conservation();
        return;
    }
    all_inventory(ob)->done_conservation();
    ob->done_conservation();
    ai->remove();
    rentstr = ZENTRALBANK->update_bank_safe_initial_rent(vi[ARMA_SPECIAL_ID],
        bank_id,RNAME(owner),bank_rent,item_rent, trophey_flag,
        valuta,valutas,v_gender);
    if (stringp(rentstr))
    {
        str += " "+rentstr;
    }
    else
    {
        str += " Fehler in der Mietberechnung.";
    }
    send_message_to(owner,MT_NOTIFY, MA_UNKNOWN,wrap(str));
    //ZB_DEBUGLOG(sprintf("DONE:=%Q",vi),
    //        DB_DBGLVL_INFO,RNAME(owner),"schliessfach:einlagern");
    remove();
}

void my_notify_move(string cntr, mapping mv_infos)
{
    if (owner == mv_infos[MOVE_OBJECT] && env != mv_infos[MOVE_NEW_ROOM])
    {
        einlagern();
        if (TO) 
        {
            object *ai = all_inventory(TO);
            if (sizeof(ai))
                ai->move(ENVR(TO));
            remove();
        }
        return;
    }
}

private string get_container_content(object ob)
{
    if (!ob || !ob->query_container()) 
    {
        return mixed2str(ob);
    }
    return mixed2str(({ob})+all_inventory(ob));
}

int forbidden_put_into(object who, object ob, object where)
{
    if (!ob || who != owner)
        return 1;
    if (where != TO)
        return 0;
    object *ai = all_inventory(TO);
    if (sizeof(ai)>0)
    {
        send_message_to(who,MT_NOTIFY, MA_UNKNOWN,wrap(
            "Nur ein Gegenstand pro "+einem(TO,"")+" ist zulässig."));
        return 1;
    }
    if (ob->query_money())
    {
        send_message_to(who,MT_NOTIFY, MA_UNKNOWN,wrap(
            "Für die Aufbewahrung von Geld ist das Bankkonto da."));
        return 1;
    }
    <int|string> istr = ZENTRALBANK->all_check_conservation(ob,([
        ZB_CHECK_TYPE:ZB_CT_SCHLIESSFACH,
        ZB_CHECK_CON:this_object(),
        ZB_CHECKER_OB:who,
        ]));
    if (stringp(istr))
    {
        send_message_to(who,MT_NOTIFY, MA_UNKNOWN,wrap(
            Der(TO,"")+" verweigert die Aufbewahrung "+des(ob)+".\n"
            "Begründung: "+istr));
        set(P_DEBUG_INFO, ([
            "fach:last_ob": get_program_name(ob)||mixed2str(ob),
            "fach:last_msg": istr,
            "fach:last_content": get_container_content(ob),
            "fach:constraints": ob->query_conservation_constraints(),
            ]) );
        set(P_DEBUG_GROUP,MASTER_OB->query_debugger(
                get_program_name(ob)+".c"));
        return 1;
    }
    set(P_DEBUG_INFO, ([]) );
    set(P_DEBUG_GROUP,({}) );
    return 0; // OK.
}

void notify_put_into(object who, object ob, object where)
{
    if (where != TO || !playerp(who) || !ob)
        return;
    send_message_to(who,MT_NOTIFY, MA_UNKNOWN,wrap(
            "Sobald "+der(TO,"")+" geschlossen ist, "
            "wird "+der(ob)+" vor Armageddon sicher sein. "
            +get_miet_string(who,ob)));
}

void notify_close(object container, object who)
{
    if (container == TO)
        call_out("einlagern",0);
}

void init_fach(string bid,object own,closure crent,
    string val,string vals,string vgen)
{
    if (own && !owner) owner = own;
    bank_id = bid;
    calc_rent = crent;
    valuta = val;
    valutas = vals;
    v_gender = vgen;
}

private string Der_ist()
{
    return Der() + ist(this_object(),1);
}

private varargs void notify_msg(string str, object who)
{
    who = who || this_player();
    who->send_message_to(who,MT_NOTIFY,MA_UNKNOWN,str);
}

// Ueberlagerung: ein notify_fail anders, kein forbidden und keine msg_other
int open_command(string str)
{
    if (!str)
	return notify_fail("Was willst Du öffnen?\n",FAIL_NOT_OBJ);
    if (!me(str))
	return notify_fail(wrap(capitalize(str) + " kannst du nicht öffnen."),FAIL_NOT_OBJ);
    if (query_no_door())
	return notify_fail(wrap(Den() + " kann man nicht öffnen."),FAIL_INTERNAL,1);
    if (query_locked())
	return notify_fail(wrap(Der_ist() + " verschlossen."),FAIL_INTERNAL,1);
    if (!query_con_close())
        return 0; // uebernimmt schliessfaecher
        // notify_fail(wrap(Der_ist() + " gar nicht zu."),FAIL_INTERNAL,1);
    if (this_player()->free_hand() < 0)
	return notify_fail(wrap("Du hast keine Hand frei, um " + deinen() 
        + " zu öffnen."), FAIL_INTERNAL);

    // if (this_object()->forbidden("open",this_object(),this_player()))
    //    return 1;
    notify_msg(wrap("Du öffnest " + deinen() + "."));
    // send_msg(wrap(Der(this_player()) + " oeffnet " + seinen() + "."));
    open_con();
    return 1;
}

// ohne auf/zu, ohne msg_other, andere notify_fails.
int close_command(string str)
{
    if (!me(str))
        return notify_fail("Schließe was?\n",FAIL_NOT_OBJ);
    if (!strstr(query_verb(),"sperr"))
        return notify_fail(Der(TO)+" kann man nicht absperren.\n",
            FAIL_WRONG_ARG);


    if (query_no_door())
        return notify_fail(wrap(Den() + " kann man nicht schließen.\n"),
            FAIL_INTERNAL);
    if (query_con_close())
        return notify_fail(wrap(Der_ist() + " doch schon zu!"),FAIL_INTERNAL);
    if (this_player()->free_hand() < 0)
        return notify_fail(wrap("Du hast keine Hand frei, um " + deinen() +
            " zu schließen."), FAIL_INTERNAL);
    //if (this_object()->forbidden("close",this_object(),this_player()))
    //    return 1;
    notify_msg(wrap("Du schließt " + deinen() + "."));
    // send_msg(wrap(Der(this_player()) + " schliesst " + seinen() + "."));
    close_con();
    return 1;
}

// othermeldung unterdruecken,ownmsg default
string *query_put_messages(object wer, object was)
{
    return ({0 ,""});
}

// othermeldung unterdruecken,ownmsg default
string *query_take_messages (object wer, object was)
{
    return ({0,""});
}

void init()
{
    "*"::init();
    if (TP && TP == ENV_TO && ENV(TP))
    {
        if (owner)
        {
            if (owner != TP)
            {
                call_out("remove",1);
                return;
            }
        }
        else
        {
            owner = TP;
        }
        env = ENV(TP);
        TP->add_controller("notify_move",#'my_notify_move);//'
        return;
    }
}

string query_long(object beobachter)
{
    object *ai = all_inventory(TO);
    if (sizeof(ai))
        return wrap(Ein(TO)+", Aktuelle "
        +get_miet_string(ENV_TO,ai[0]));
    else if (c_long_leer)
        return closure_to_string(c_long_leer);
    else
        return Ein(TO,"leer")+".\n";
}

public void set_schliessfach_design(mapping design)
{
    if(!mappingp(design)) design = ([]);
    set_name(design["name"]||"schließfach");
    set_gender(design["gender"]||"saechlich");
    if (stringp(design["id"])) 
        design["id"] = ({ design["id"] });
    if (pointerp(design["id"]))
        set_id( design["id"]-({ZB_SCHLIESSFACH_ID})+({ZB_SCHLIESSFACH_ID}) );
    else
        set_id( ({"schließfach","fach",ZB_SCHLIESSFACH_ID}) );
    if (member(design,"adjektiv"))
    {
        set_adjektiv(design["adjektiv"]);
    }    
    if (stringp(design["n_schliessfachmiete"]))
        n_schliessfachmiete = design["n_schliessfachmiete"];
    if (stringp(design["c_long_leer"]))
        c_long_leer = mixed_to_closure(design["c_long_leer"]);
    else
        c_long_leer = 0;
    if (stringp(design["c_leer_verschwindet"]))
        c_leer_verschwindet = mixed_to_closure(design["c_leer_verschwindet"]);
    else
        c_leer_verschwindet = 0;
}

void create() 
{
    "*"::create();
    set_schliessfach_design( ([
        "name": "schließfach",
        "gender": "saechlich",
        "id": ({ "schließfach","fach" }),
    ]) );
    set_invis(V_NOLIST);
    set_weight(0);
    set_max_internal_encumbrance(0); // Ein Gegenstand ueber Controller!
    set_value(12);
    set_no_lock(1);
    set_collapsible(1);
    add_controller( ({"forbidden_put_into","notify_close","notify_put_into",
        }),TO);
    add_controller("forbidden_shadow",TO);
    set(P_LOOK_MSG,"");
    set(P_READ_MSG,"");
    set(P_SMELL_MSG,"");
    set(P_HEAR_MSG,"");
    set(P_FEEL_MSG,"");
    set(P_TAKE_MSG,"");

}

void abort_renewal()
{
}

void finish_renewal(object neu)
{
    neu->init_fach(bank_id,owner,calc_rent,valuta,valutas,v_gender);
}

void prepare_renewal()
{
}
