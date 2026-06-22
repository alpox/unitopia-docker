// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/i/money/schliessfachaddon.c
// Description: Fuer Bankraeume mit Schliessfaechern ein AddOn
//              (geht nicht ueber /i/room.c o.ae.)
// Author:	Myonara

#include <error.h>
#include <invis.h>
#include <level.h>
#include <misc.h>
#include <move.h>
#include <properties.h>
#include <room_types.h>

#include <money.h>

private string bankid,valuta,valutas,v_gender;
private closure calc_rent,calc_max_count;
private mapping faecher_design,fach_design;

// Interne Routine als Basis fuer die Faecher-und Fachmietenprogression.
float get_skill_age_progression(int age, int skill)
{
    // 1) Bereich 100T=86,400.0, 100%EP=132,000
    // return (age+(skill*0.1))/100000.0;
    // 2) age weniger stark=>1% und skill mal 30.
    return ((age*0.01)+(skill*3.0))/100000.0;
}


/*
FUNKTION: get_schliessfach_max_faecher
DEKLARATION: public int get_schliessfach_max_faecher(object player)
BESCHREIBUNG:
Prueft in der aktuellen Bank, wieviel Faecher dem Spieler zur Verfuegung steht.
Standardmaessig wird das Spieleralter und die Erfahrungspunkte einbezogen.
Dient als Angabe als Closure in init_schliessfaecher.
VERWEISE: get_schliessfach_miete, init_schliessfaecher
GRUPPEN: armageddon
*/
public int get_schliessfach_max_faecher(object player)
{
    return  min(9,to_int(floor(get_skill_age_progression(
        player->query_age(),player->query_sum_skill()))))+3;
    // auf 3..12 begrenzen
}

/*
FUNKTION: get_schliessfach_miete
DEKLARATION: public float get_schliessfach_miete(object player, object ob, int current_safe_count)
BESCHREIBUNG:
Liefert die Schliessfachmiete für den Spieler und ein Objekt. 
In current_safe_count wird die Anzahl der aktuellen belegten Schliessfaecher
uebergeben. Für das Objekt ob wird ausserhalb get_conservation_item_rent
aufgerufen, kann aber optional hier angegeben werden. Die Fachmiete gilt
nicht nur fuer das aktuelle Objekt sondern fuer alle Faecher in dieser Bank.
Dient als Angabe als Closure in init_schliessfaecher.
Der Rueckgabewert ist in Taler, so genau wie moeglich.
VERWEISE: get_schliessfach_max_faecher, init_schliessfaecher
GRUPPEN: armageddon
*/
public float get_schliessfach_miete(object player, object ob,
        int current_safe_count)
{
    if (objectp(ob)&&ob->query(P_CONSERVATION,P_CONSERVATION_TROPHEY)>0)
        return 0.1;
    return max(2.11,get_skill_age_progression(
        player->query_age(),player->query_sum_skill()))*4.9;
}


/*
FUNKTION: init_schliessfaecher
DEKLARATION: protected varargs int init_schliessfaecher(string my_bankid,string my_valuta,closure my_calc_rent,closure my_calc_max)
BESCHREIBUNG:
Diese Funktion uebernimmt die Initialisierung des Schliessfachraums mit
eine korrekten Bank-ID und der aktuellen Waehrung in der Einzahl.
Rueckgabe 1 bei Erfolg, 0 und eine do_warning bei Misserfolg.
Die Angabe von 
- my_calc_rent (Parameter und Funktion siehe get_schliessfach_miete) und
- my_calc_max (Parameter und Funktion siehe get_schliessfach_max_faecher) 
sind optional, die Standards sind in dieser Sourcedatei definiert.
wenn nicht ang
VERWEISE: liefere_schliessfaecher, get_schliessfach_miete, 
GRUPPEN: armageddon
*/
protected varargs int init_schliessfaecher(string my_bankid,string my_valuta,
    closure my_calc_rent,closure my_calc_max)
{
    mixed * minfo;
    if (!sizeof(ZENTRALBANK->get_bank(my_bankid)))
    {
        do_warning(wrap("Unbekannte Bank-ID: "+my_bankid));
        return 0;
    }
    if (!my_valuta || !(minfo=ZENTRALBANK->query_money_info(my_valuta)))
    {
        do_warning(wrap("Ungültige Währung "+my_valuta));
        return 0;
    }
    bankid = my_bankid;
    valuta = my_valuta;
    valutas = minfo[1];
    v_gender = minfo[2];
    calc_max_count = my_calc_max || (: TO->get_schliessfach_max_faecher($1) :);
    calc_rent = my_calc_rent || (: TO->get_schliessfach_miete($1,$2,$3) :);
    return 1;
}

/*
NOENZY: liefere_schliessfaecher
DEKLARATION: protected int liefere_schliessfaecher(object who)
BESCHREIBUNG:
Diese Funktion uebernimmt die Versorgung des Spielers who
mit einem Satz Schliessfaecher. Sollte in moved_in aufgerufen werden.
Rueckgabe 1 bei Erfolg, 0 sonst. Wird automatisch im schliessfachaddon 
ueber ein controller im create aufgerufen.
VERWEISE: init_schliessfaecher
GRUPPEN: armageddon
*/
protected int liefere_schliessfaecher(object who)
{
    if (playerp(who) && !guestp(who) && !present(ZB_SCHLIESSFAECHER_ID,who)
            && !who->query_con_close())
    {
        object faecher = clone_object(ZB_SCHLIESSFAECHER);
        faecher->set_designs(faecher_design,fach_design);
        faecher->init_faecher(bankid,who,valuta,valutas,v_gender,
                calc_rent,calc_max_count);
        faecher->move(who);
        return 1;
    }
    return 0;
}

public nomask void schliessfach_my_moved_in(string ctrl, mapping mv_infos)
{
    liefere_schliessfaecher(mv_infos[MOVE_OBJECT]);
}

/*
FUNKTION: set_designs
DEKLARATION: public nomask void set_designs(mapping plural_design,mapping single_design)
BESCHREIBUNG:
Mit Hilfe dieser Funktion laesst sich das Design der Schliessfaecher 
beeinflussen. Im Folgenden sind die Parameter fuer das Standarddesign
zusammen mit Kommentaren fuer die Sonderfaelle gekennzeichnet:
plural_design = ([
            "name" : "schliessfaecher",
            "cap_name": "Schliessfaecher",
            "gender" : "saechlich",
            "plural" : 1, // Pflicht bei Plural_design
            "id" : ({"schliessfaecher","faecher"}),
            "prefix_long" : "", // Text fuer den Anfang der Long
            "c_item_take" : 0, // pseudoclosure fuer Gegenstand nehmen
            "c_fach_take" : 0, // pseudoclosure fuer Fach nehmen.
    ]);
single_design = ([
            "name" : "schliessfach",
            "cap_name" : "Schliessfach",
            "gender" : "saechlich",
            "id" : ({"schliessfach","fach"}),
            "n_schliessfachmiete": 0, // Ersatz fuer Schliessfachmiete
            "c_long_leer":0, // Closure fuer ein leeres Schliessfach.
            "c_leer_verschwindet":0, // Closure fuer 
      // "Das leere, geschlossene Schliessfach entschwindet in der Wand"
    ]);

GRUPPEN: armageddon
*/
public nomask void set_designs(mapping plural_design,mapping single_design)
{
    faecher_design = plural_design;
    fach_design = single_design;
}

void create()
{
    TO->set_own_light(1);
    TO->add_type(({RT_KUNSTLICHT, RT_KEIN_CLEANUP, RT_KAEMPFEN_VERBOTEN,
        RT_STEHLEN_VERBOTEN, RT_GRABEN_VERBOTEN, RT_MAGIE_VERBOTEN, 
        RT_HANDWERK_VERBOTEN }), 1);
    TO->add_v_item( ([ // Default wenn man keine Faecher hat...
        "name": "schließfächer",
        "id" : ({ "schließfächer","schließfach","fächer","fach" }),
        "gender": "saechlich",
        "plural" : 1,
        "long": "Diese Schließfächer sind nicht für dich.",
        "look_msg": "",
        "invis": (: present(ZB_SCHLIESSFAECHER_ID,TP) ? V_INVIS : V_VIS :),
    ]) );
    TO->add_controller("notify_moved_in",#'schliessfach_my_moved_in);
    set_designs(0,0);
}
