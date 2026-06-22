// This file is part of UNItopia mudlib.
// -------------------------------------------------------------------
// File:        /room/bank/bankenaufsicht.c
// Description: zentrale Kontrolle der Banken.
// Author:      Myonara (30.12.2015)
// Modified by:
//  Myonara 24.Apr.2016  Rewrite fuer Schliessfaecher

/* --- Inherits: --- */
protected functions nosave variables inherit "/i/tools/security";
nosave variables inherit "/i/room";
protected functions nosave variables inherit "/i/tools/build_table";
nosave variables inherit "/i/tools/dynamic_browser";
//protected functions nosave variables inherit "/p/Misc/i/array";
//protected functions nosave variables inherit "/p/Misc/i/queue";

/* --- Includes: --- */
#include <acl.h>
#include <config.h>
#include <database.h>
#include <dynamic_browser.h>
#include <level.h>
#include <message.h>
#include <misc.h>
#include <more.h>
#include <move.h>
#include <notify_fail.h>
#include <parse_com.h>
#include <security.h>

#include <money.h>

#define DEBUGGER "myonara"
#include <debug.h>

#define LINE      "------------------------------------------------------" \
                  "-------------------------"
#define MORE_LINE "...---------------------------------------------------" \
                  "------------------ (MORE)"

                  
nosave object *to_rescue = ({});

static int bank_files_menu_total(mapping menue);

private int check_access_for_bank(string bank_id, object wiz)
{
    if (adminp(wiz)) return 1;
    if (!wizp(wiz)) return 0;
    string * banks = ZENTRALBANK->get_bank_ids_by_access(
        ([ ZB_BANK_ID: bank_id,ZB_BANK_WIZARD: wiz ]), 0);
    return 0 && sizeof(banks)==1; // TODO freigeben?
}

private string* get_all_accessible_banks(object wiz)
{
    return ZENTRALBANK->get_bank_ids_by_access(([ ZB_BANK_WIZARD: wiz ]), 0);
}

private int check_v_item_entry(mapping entry, string ids, string *adj)
{
    if (mappingp(entry) && HAS_ID(entry,ids)) {
        if (adj) {
            foreach(string str : adj) {
                if (member(entry["adjektiv"]||({}),str) == -1) {
                    return 0;
                }
            }
        }
        return 1;
    }
    return 0;
}

varargs mapping query_v_item(mixed *pfad, int flag)
{
    mapping ret, what, *visa;
    string id, *adj;
    int nummer;

    visa = ZENTRALBANK->get_all_bank_safe_room_v_items();
    if(!(ret = ::query_v_item(pfad,flag)) &&
       sizeof(pfad)==1)
    {
        if(stringp(pfad[0]))
        {
            id = lower_case(pfad[0]);
        }
        else if(mappingp(pfad[0]))
        {
            id = lower_case(pfad[0]["name"]);
            adj = pfad[0]["adjektiv"];
            nummer=pfad[0]["nummer"];
        }
        visa=filter(visa,#'check_v_item_entry,id,adj);
        if(sizeof(visa))
        {
            if(nummer > 1)
            {
                if(sizeof(visa)>=nummer)
                    what = visa[nummer-1];
                else
                    return 0;
            } else {
                what = visa[0];
            }
            ret = what + ([
              "environment"   : TO,
              "v_item_master" : TO,
                              ]);
        }
    }
    return ret;
}

mixed *query_all_v_items()
{
    mapping *visa;
    visa = ZENTRALBANK->get_all_bank_safe_room_v_items();
    return (::query_all_v_items()||({}))
            +map(visa||({}),
                 function(mapping entry)
                 {
                     return entry + ([
                 "environment"   : TO,
                 "v_item_master" : TO,
                                 ]);
                     });
}


/*
    DL/DH versus query_member_of?: ==> Listen nach "meiner" Domain...
*/

//-------------------------------------------------------------------------
/*
static mixed <type>_init(mapping old)
{
}

static int <type>_total(mapping menue)
{
}

static string * <type>_display(mapping menue)
{
}

static string <type>_prompt(mapping menue)
{
}

static mixed <type>_action(string str, mapping * menues)
{
}

*/


//-------------------------------------------------------------------------

private string* get_search_mask(mapping menue)
{
    string * mask = ({});
    mask += ({"(D) Dateifilter: "+menue[ZB_BANK_FILE_PATTERN]||"%" });
    mask += ({"(A) Datenanzeige."});
    return mask;
}

static mixed search_mask_init(mapping old)
{
    old[B_START_LINE]=0;
    return old;
}

static int search_mask_total(mapping menue)
{
    return sizeof(get_search_mask(menue));
}

static string * search_mask_display(mapping menue)
{
    menue[B_DATA] = get_search_mask(menue);
    return staticmore_display(menue);
}

static string search_mask_prompt(mapping menue)
{
    return "Suchmaske [d,a,q]";
}

static mixed search_mask_action(string str, mapping * menues)
{
    string * split = explode(strip(str)," ");
    string param = implode(split[1..]," ");
    mapping menue = menues[<1];
    switch (lower_case(split[0]))
    {
        case "d":
            if (param == "")
            {
                m_delete(menue,ZB_BANK_FILE_PATTERN);
                browse_write_line("Dateimaske gelöscht.");
            }
            else
            {
                menue[ZB_BANK_FILE_PATTERN] = param;                
                browse_write_line("Dateimaske gesetzt.");
            }
            return B_REBUILT;
        case "a":
            return menues+({([
                B_TYPE : "sub_menu",
                ZB_BANK_FILE_PATTERN: menue[ZB_BANK_FILE_PATTERN],
            ])});
    }
    return B_NOTHING;
}

//-------------------------------------------------------------------------

static mixed sub_menu_init(mapping old)
{
    old[B_START_LINE]=0;

    old[B_DATA] = ({
        "- 1- Alle Banken",
        "- 2- Banken mit ausgelagerten Dateien",
        "- 3- Ausgelagerte Dateien",
        "- 4- Ausgelagerte Dateien mit Fehlerflag",
        "- 5- Eigentümer, die Dateien ausgelagert haben",
        "- 6- Alle Anträge (DL/Admin)",
        "- 7- Freigegebene Einzelobjekte und Shadows",
        //"- 8- Freigegebene Factory-Objekte",
        "-20- Gesamtstatistik",
        "-21- Statistik eingelagerte Objekte und Shadows",
        "-22- Statistik nach Eigentümer",
        "-23- Statistik nach LoadFile/Creator",
        "-24- Statistik je Datei und sortiert nach Alter",
        "-d- Dateifilter setzen."
    });
}

static int sub_menu_total(mapping menue)
{
    return sizeof(menue[B_DATA]);
}

static string * sub_menu_display(mapping menue)
{
    return staticmore_display(menue);
}

static string sub_menu_prompt(mapping menue)
{
    string filter = menue[ZB_BANK_FILE_PATTERN];
    if (filter)
        return sprintf("Menü gefiltert (%s) (%d/%d) [<nr>,z,q]",
            filter, menue[B_CURRENT_LINE],sub_menu_total(menue));
    return sprintf("Menü ungefiltert (%d/%d) [<nr>,z,q]",
            menue[B_CURRENT_LINE],sub_menu_total(menue));
}

static mixed sub_menu_action(string str, mapping * menues)
{
    int i;
    mapping menue = menues[<1];
    if (!str2int(space(str),&i) && i>0)
    {
        switch(i)
        {
            case 1:
                return menues + ({([ B_TYPE:"banks",
                      ZB_BANK_FILE_PATTERN: menue[ZB_BANK_FILE_PATTERN] ])});
            case 2:
                return menues + ({([ B_TYPE:"banks_with_safe_files",
                      ZB_BANK_FILE_PATTERN: menue[ZB_BANK_FILE_PATTERN],
                ]) });
            case 3:
                return menues + ({([ B_TYPE:"bank_safe_by_files",
                      ZB_BANK_FILE_PATTERN: menue[ZB_BANK_FILE_PATTERN],
                ]) });
            case 4:
                return menues + ({([ B_TYPE:"bank_safe_by_files",
                                ZB_BANK_WITH_ERROR: 1,
                      ZB_BANK_FILE_PATTERN: menue[ZB_BANK_FILE_PATTERN],
                ]) });
            case 5:
                return menues + ({([ B_TYPE:"bank_safes_by_owner",
                      ZB_BANK_FILE_PATTERN: menue[ZB_BANK_FILE_PATTERN],
                ]) });
            case 6:
                return menues + ({([ B_TYPE:"bank_request_files",
                      ZB_BANK_FILE_PATTERN: menue[ZB_BANK_FILE_PATTERN] ])});
            case 7:
                return menues + ({([ B_TYPE:"bank_safe_filters",
                      ZB_BANK_FILE_PATTERN: menue[ZB_BANK_FILE_PATTERN],
                ]) });
            // case 8:
                // return menues + ({([ B_TYPE:"bank_factory_filters",
                      // ZB_BANK_FILE_PATTERN: menue[ZB_BANK_FILE_PATTERN],
                // ]) });
            case 20:
                return menues + ({([ B_TYPE:"overview_statistics",
                ]) });
            case 21:
                return menues + ({([ B_TYPE:"file_statistics",
                      ZB_BANK_FILE_PATTERN: menue[ZB_BANK_FILE_PATTERN],
                ]) });
            case 22:
                return menues + ({([ B_TYPE:"owner_statistics",
                ]) });
            case 23:
                return menues + ({([ B_TYPE:"loadfile_creator_statistics",
                      ZB_BANK_FILE_PATTERN: menue[ZB_BANK_FILE_PATTERN],
                ]) });
            case 24:
                return menues + ({([ B_TYPE:"oldest_statistics",
                      ZB_BANK_FILE_PATTERN: menue[ZB_BANK_FILE_PATTERN],
                ]) });
        }
    }
    string * split = explode(strip(str)," ");
    string param = implode(split[1..]," ");
    switch (lower_case(split[0]))
    {
        case "d":
            if (param == "")
            {
                m_delete(menue,ZB_BANK_FILE_PATTERN);
                browse_write_line("Dateimaske gelöscht.");
            }
            else
            {
                menue[ZB_BANK_FILE_PATTERN] = param;                
                browse_write_line("Dateimaske gesetzt.");
            }
            return B_REBUILT;
    }
    return B_NOTHING;
}

//-------------------------------------------------------------------------

static mixed banks_init(mapping old)
{
    old[B_START_LINE]=0;
    return old;
}

static int banks_total(mapping menue)
{
    return ZENTRALBANK->get_bank_list(menue,1);
}

static string * banks_display(mapping menue)
{
    string * lines = ZENTRALBANK->get_bank_list(
        menue+([
            DB_DBG_LIMIT:menue[B_NUM_LINES],
            DB_DBG_OFFSET:menue[B_CURRENT_LINE]
            ]),0);
    if (lines == 0 || !pointerp(lines))
        return ({ "interner Zugriffsfehler." });
    for (int i = 0;i<sizeof(lines);i++)
    {
        lines[i] = sprintf("%2d %s",i+1+menue[B_CURRENT_LINE],lines[i]);
    }
    return lines;
}

static string banks_prompt(mapping menue)
{
    return sprintf("Banken (%d/%d) [<nr>,z,q]",
        menue[B_CURRENT_LINE],banks_total(menue));
}

static mixed banks_action(string str, mapping * menues)
{
    int i;
    string * lines ;
    mapping menue = menues[<1];
    if (!str2int(space(str),&i) && i>0 && i <= banks_total(menue))
    {
        lines = ZENTRALBANK->get_bank_list(
            menue+([ DB_DBG_LIMIT:1, DB_DBG_OFFSET:i-1, ]),0);
        if (sizeof(lines))
        {
            return menues + ({([ B_TYPE:"bank_menu",ZB_BANK_ID:lines[0] ])});
        }
    }
    return B_NOTHING;
}

//-------------------------------------------------------------------------

static mixed banks_with_access_init(mapping old)
{
    old[B_START_LINE]=0;
    return old;
}

static int banks_with_access_total(mapping menue)
{
    menue[ZB_BANK_WIZARD] = TP;
    return ZENTRALBANK->get_bank_ids_by_access(menue,1);
}

static string * banks_with_access_display(mapping menue)
{
    menue[ZB_BANK_WIZARD] = TP;
    string * lines = ZENTRALBANK->get_bank_ids_by_access(
        menue+([
            DB_DBG_LIMIT:menue[B_NUM_LINES],
            DB_DBG_OFFSET:menue[B_CURRENT_LINE]
            ]),0);
    if (lines == 0 || !pointerp(lines))
        return ({ "interner Zugriffsfehler." });
    for (int i = 0;i<sizeof(lines);i++)
    {
        lines[i] = sprintf("%2d %s",i+1+menue[B_CURRENT_LINE],lines[i]);
    }
    return lines;
}

static string banks_with_access_prompt(mapping menue)
{
    return sprintf("Banken (%d/%d) [<nr>,z,q]",
        menue[B_CURRENT_LINE],banks_with_access_total(menue));
}

static mixed banks_with_access_action(string str, mapping * menues)
{
    int i;
    string * lines ;
    mapping menue = menues[<1];
    menue[ZB_BANK_WIZARD] = TP;
    if (!str2int(space(str),&i) && i>0 && i <= banks_with_access_total(menue))
    {
        lines = ZENTRALBANK->get_bank_ids_by_access(
            menue+([ DB_DBG_LIMIT:1, DB_DBG_OFFSET:i-1, ]),0);
        if (sizeof(lines))
        {
            return menues + ({([ B_TYPE:"bank_menu",ZB_BANK_ID:lines[0] ])});
        }
    }
    return B_NOTHING;
}

//-------------------------------------------------------------------------
private string * get_bank_menu(mapping menue)
{
    mapping bankmap = ZENTRALBANK->get_bank(menue[ZB_BANK_ID]);
    string * lines = ({}),line="";
    int flags;
    if (sizeof(bankmap)==0)
    {
        return ({ "Es liegen keine Daten vor."});
    }
    lines += ({ "Bank-ID: "+bankmap[ZB_BANK_ID] });
    flags = bankmap[ZB_BANK_FLAGS];
    if (flags & ZB_BANK_CLASSIC)
    {
        line += "Klassische Bank";
    }
    else
    {
        line += "Bank mit NPCs";
    }
    if (flags & ZB_BANK_ACTIVE)
    {
        line += " (Aktiv)";
    }
    lines += ({ line });
    lines += ({ "D: "+bank_files_menu_total(menue)+" Dateien." });
    return lines;
}

static mixed bank_menu_init(mapping old)
{
    old[B_START_LINE]=0;
    return old;
}

static int bank_menu_total(mapping menue)
{
    return sizeof(get_bank_menu(menue));
}

static string * bank_menu_display(mapping menue)
{
    menue[B_DATA] = get_bank_menu(menue);
    return staticmore_display(menue);
}

static string bank_menu_prompt(mapping menue)
{
    return sprintf("Bank \"%s\" (%d/%d) [+,-,d,t,z,q]",
        menue[ZB_BANK_ID],menue[B_CURRENT_LINE], bank_menu_total(menue));
}

static mixed bank_menu_action(string str, mapping * menues)
{
    mapping menue = menues[<1];
    string bank_id = menue[ZB_BANK_ID];
    switch (lower_case(space(str)))
    {
        case "d":
            return menues + ({([ 
                B_TYPE:"bank_files_menu",  
                ZB_BANK_ID:bank_id ]) });
        case "+":
            bank_id = ZENTRALBANK->get_next_bank(bank_id,1);
            if (bank_id==0) 
            {
                browse_write_line("Ende der Daten erreicht.");
                return B_DONE;
            }
            else
            {
                return menues[..<2]+({
                    ([ B_TYPE:"bank_menu",ZB_BANK_ID:bank_id ]) 
                    });
            }
        case "-":
            bank_id = ZENTRALBANK->get_next_bank(bank_id,-1);
            if (bank_id==0) 
            {
                browse_write_line("Anfang der Daten erreicht.");
                return B_DONE;
            }
            else
            {
                return menues[..<2]+({
                    ([ B_TYPE:"bank_menu",ZB_BANK_ID:bank_id ]) 
                    });
            }
    }
    return B_NOTHING;
}

//-------------------------------------------------------------------------

static mixed bank_files_menu_init(mapping old)
{
    old[B_START_LINE] = 0;
    return old;
}

static int bank_files_menu_total(mapping menue)
{
    return ZENTRALBANK->get_bank_files(menue,1);
}

static string * bank_files_menu_display(mapping menue)
{
    string * lines = ZENTRALBANK->get_bank_files(
        menue+([
            DB_DBG_LIMIT:menue[B_NUM_LINES],
            DB_DBG_OFFSET:menue[B_CURRENT_LINE]
            ]),0);
    if (lines == 0 || !pointerp(lines))
        return ({ "Keine Anzeige." });
    for (int i = 0;i<sizeof(lines);i++)
    {
        lines[i] = sprintf("%2d %s",i+1+menue[B_CURRENT_LINE],lines[i]);
    }
    return lines;
}

static string bank_files_menu_prompt(mapping menue)
{
    return sprintf("Dateien der Bank \"%s\" (%d/%d) [g <nr>,z,q]",
        menue[ZB_BANK_ID],menue[B_CURRENT_LINE], bank_files_menu_total(menue));
}

static mixed bank_files_menu_action(string str, mapping * menues)
{
    string * split = explode(space(str)," ");
    int i;
    mapping menue = menues[<1];
    object ob;
    switch (lower_case(split[0]))
    {
        case "g":
            if (sizeof(split)<2)
            {
                browse_write_line("g <nr> zum Anspringen von Bank/Raum oder "
                    "geladenem NPC");
                return B_DONE;
            }
            if (!str2int(split[1],&i) && i > 0 
                    && i <= bank_files_menu_total(menue))
            {
                split = ZENTRALBANK->get_bank_files(
                    menue+([ DB_DBG_LIMIT:1, DB_DBG_OFFSET:(i-1) ]),0);
                split = explode(split[0]," ");
                if (split[1] == "(Bank)" || split[1] == "(Raum)")
                {
                    ob = find_object(split[0]) || touch(split[0]);
                }
                else
                {
                    ob = find_object(split[0]);
                    if (ob) 
                    {
                        ob = ENVR(ob);
                    }
                }
                if (ob && TP->move(ob) == MOVE_OK)
                {
                    browse_write_line("Transport durchgeführt.");
                    return B_QUIT;
                }
            }
            browse_write_line("Transport nicht durchgeführt.");
            return B_DONE;
    }
    return B_NOTHING;
}
//-------------------------------------------------------------------------
static mixed bank_safe_files_init(mapping old)
{
    old[B_START_LINE] = 0;
    return old;
}

static int bank_safe_files_total(mapping menue)
{
    return ZENTRALBANK->retrieve_bank_safe_files(menue,1);
}

static string * bank_safe_files_display(mapping menue)
{
    string * lines = ZENTRALBANK->retrieve_bank_safe_files(
        menue+([
            DB_DBG_LIMIT:menue[B_NUM_LINES],
            DB_DBG_OFFSET:menue[B_CURRENT_LINE]
            ]),0);
    if (lines == 0 || !pointerp(lines))
        return ({ "Keine Anzeige." });
    for (int i = 0;i<sizeof(lines);i++)
    {
        lines[i] = sprintf("%3d %s",i+1+menue[B_CURRENT_LINE],lines[i]);
    }
    return lines;
}

static string bank_safe_files_prompt(mapping menue)
{
    string title = "Unbekannt";
    switch (menue[ZB_BANK_OUTPUT])
    {
        case ZB_BANK_FILE:   title = "Ausgelagerte Dateien";break;
        case ZB_BANK_ITEMID: raise_error("nicht mehr verwendet-itemid.\n");
        case ZB_BANK_ID:     raise_error("nicht mehr verwendet-bankid.\n");
        case ZB_BANK_OWNER:  
            title = "Eigentümer mit ausgelagerten Dateien";break;
    }
    return sprintf("%s (%d/%d) [<nr>,z,q]",
        title,menue[B_CURRENT_LINE], bank_safe_files_total(menue));
}

static mixed bank_safe_files_action(string str, mapping * menues)
{
    mapping menue = menues[<1];
    mixed lines;
    int i;
    if (!str2int(space(str),&i) && i>0 && i <= bank_safe_files_total(menue))
    {
        lines = ZENTRALBANK->retrieve_bank_safe_files(
            menue+([ ZB_BANK_OUTPUT:0, DB_DBG_LIMIT:1, DB_DBG_OFFSET:i-1, ]),0);
        //DEBUG(sprintf("bank_safe_files_action: %d %Q %Q",i,lines,menue));
        if (sizeof(lines))
        {
            switch (menue[ZB_BANK_OUTPUT])
            {
                case ZB_BANK_FILE:   
                    return menues + ({ menue + ([
                        B_TYPE: "bank_safe_item_ids",
                        ZB_BANK_OUTPUT: ZB_BANK_ITEMID,
                        ZB_BANK_SORTBY: ZB_BANK_ITEMID,
                        ZB_BANK_FILE: lines[0][0]
                    ]) });
                case ZB_BANK_ITEMID: 
                    return menues + ({ ([
                        B_TYPE: "bank_safe_item",
                        ZB_BANK_ITEMID: lines[0][1],
                    ]) });
                case ZB_BANK_ID:
                    return menues + ({ menue + ([
                        B_TYPE: "bank_safe_item_ids",
                        ZB_BANK_OUTPUT: ZB_BANK_ITEMID,
                        ZB_BANK_SORTBY: ZB_BANK_ITEMID,
                        ZB_BANK_ID: lines[0][2],
                    ]) });
                case ZB_BANK_OWNER:
                    return menues + ({ menue + ([
                        B_TYPE: "bank_safe_item_ids",
                        ZB_BANK_OUTPUT: ZB_BANK_ITEMID,
                        ZB_BANK_SORTBY: ZB_BANK_ITEMID,
                        ZB_BANK_OWNER: lines[0][3],
                    ]) });
            }
       }
    }

    return B_NOTHING;
}

//-------------------------------------------------------------------------
static mixed bank_safes_by_owner_init(mapping old)
{
    old[B_START_LINE] = 0;
    return old;
}

static int bank_safes_by_owner_total(mapping menue)
{
    return ZENTRALBANK->retrieve_bank_safes_by_owner(menue,1);
}

static string * bank_safes_by_owner_display(mapping menue)
{
    string * lines = ZENTRALBANK->retrieve_bank_safes_by_owner(
        menue+([
            DB_DBG_LIMIT:menue[B_NUM_LINES],
            DB_DBG_OFFSET:menue[B_CURRENT_LINE]
            ]),0);
    if (lines == 0 || !pointerp(lines))
        return ({ "Keine Anzeige." });
    for (int i = 0;i<sizeof(lines);i++)
    {
        lines[i] = sprintf("%3d %s",i+1+menue[B_CURRENT_LINE],lines[i]);
    }
    return lines;
}

static string bank_safes_by_owner_prompt(mapping menue)
{
    string title = "Eigentümer mit ausgelagerten Dateien";
    
    return sprintf("%s (%d/%d) [<nr>,z,q]",
        title,menue[B_CURRENT_LINE], menue[B_END_LINE]+1);
}

static mixed bank_safes_by_owner_action(string str, mapping * menues)
{
    mapping menue = menues[<1];
    mixed lines;
    int i;
    if (!str2int(space(str),&i) && i>0 && i <= (menue[B_END_LINE]+1))
    {
        lines = ZENTRALBANK->retrieve_bank_safes_by_owner(
            menue+([ ZB_BANK_OUTPUT:0, DB_DBG_LIMIT:1, DB_DBG_OFFSET:i-1, ]),0);
        //DEBUG(sprintf("bank_safes_by_owner_action: %d %Q %Q",i,lines,menue));
        if (sizeof(lines))
        {
            return menues + ({ menue + ([
                        B_TYPE: "bank_safe_item_ids",
                        ZB_BANK_OUTPUT: ZB_BANK_ITEMID,
                        ZB_BANK_SORTBY: ZB_BANK_ITEMID,
                        ZB_BANK_OWNER: lines[0],
                    ]) });
       }
    }

    return B_NOTHING;
}

//-------------------------------------------------------------------------
static mixed bank_safe_by_files_init(mapping old)
{
    old[B_START_LINE] = 0;
    return old;
}

static int bank_safe_by_files_total(mapping menue)
{
    return ZENTRALBANK->retrieve_bank_safe_by_files(menue,1);
}

static string * bank_safe_by_files_display(mapping menue)
{
    string * lines = ZENTRALBANK->retrieve_bank_safe_by_files(
        menue+([
            DB_DBG_LIMIT:menue[B_NUM_LINES],
            DB_DBG_OFFSET:menue[B_CURRENT_LINE]
            ]),0);
    if (lines == 0 || !pointerp(lines))
        return ({ "Keine Anzeige." });
    for (int i = 0;i<sizeof(lines);i++)
    {
        lines[i] = sprintf("%3d %s",i+1+menue[B_CURRENT_LINE],lines[i]);
    }
    return lines;
}

static string bank_safe_by_files_prompt(mapping menue)
{
    string title = "Ausgelagerte Dateien";
    return sprintf("%s (%d/%d) [<nr>,z,q]",
        title,menue[B_CURRENT_LINE], bank_safe_by_files_total(menue));
}

static mixed bank_safe_by_files_action(string str, mapping * menues)
{
    mapping menue = menues[<1];
    mixed lines;
    int i;
    if (!str2int(space(str),&i) && i>0 && i <= bank_safe_by_files_total(menue))
    {
        lines = ZENTRALBANK->retrieve_bank_safe_by_files(
            menue+([ ZB_BANK_OUTPUT:0, DB_DBG_LIMIT:1, DB_DBG_OFFSET:i-1, ]),0);
        //DEBUG(sprintf("bank_safe_by_files_action: %d %Q %Q",i,lines,menue));
        if (sizeof(lines))
        {
            return menues + ({ menue + ([
                        B_TYPE: "bank_safe_item_ids",
                        ZB_BANK_OUTPUT: ZB_BANK_ITEMID,
                        ZB_BANK_SORTBY: ZB_BANK_ITEMID,
                        ZB_BANK_FILE: lines[0]
                    ]) });
       }
    }

    return B_NOTHING;
}

//-------------------------------------------------------------------------
static mixed bank_safe_item_ids_init(mapping old)
{
    old[B_START_LINE] = 0;
    return old;
}

static int bank_safe_item_ids_total(mapping menue)
{
    return ZENTRALBANK->retrieve_bank_safe_item_ids(menue,1);
}

static string * bank_safe_item_ids_display(mapping menue)
{
    string * lines = ZENTRALBANK->retrieve_bank_safe_item_ids(
        menue+([
            DB_DBG_LIMIT:menue[B_NUM_LINES],
            DB_DBG_OFFSET:menue[B_CURRENT_LINE]
            ]),0);
    mapping cache = ([]);
    string* oneline;
    if (lines == 0 || !pointerp(lines))
        return ({ "Keine Anzeige." });
    for (int i = 0;i<sizeof(lines);i++)
    {
        oneline = explode(lines[i]," ");
        cache[i+menue[B_CURRENT_LINE]+1] = oneline[0];
        lines[i] = sprintf("%3d %s",i+1+menue[B_CURRENT_LINE],
            implode(oneline[1..]," "));
    }
    menue[ZB_DATA_CACHE] = cache;
    return lines;
}

static string bank_safe_item_ids_prompt(mapping menue)
{
    string title = "Ausgelagerte Gegenstände";
    return sprintf("%s (%d/%d) [<nr>,z,q]",
        title,menue[B_CURRENT_LINE], bank_safe_item_ids_total(menue));
}

static mixed bank_safe_item_ids_action(string str, mapping * menues)
{
    mapping menue = menues[<1],cache = menue[ZB_DATA_CACHE];
    int i;
    if (!str2int(space(str),&i) && i>0 && member(cache,i))
    {
        return menues + ({ ([
                        B_TYPE: "bank_safe_item",
                        ZB_BANK_ITEMID: cache[i],
                    ]) });
    }

    return B_NOTHING;
}
//-------------------------------------------------------------------------
static mixed banks_with_safe_files_init(mapping old)
{
    old[B_START_LINE] = 0;
    return old;
}

static int banks_with_safe_files_total(mapping menue)
{
    return ZENTRALBANK->retrieve_banks_with_safe_files(menue,1);
}

static string * banks_with_safe_files_display(mapping menue)
{
    string * lines = ZENTRALBANK->retrieve_banks_with_safe_files(
        menue+([
            ZB_BANK_OUTPUT:1,
            DB_DBG_LIMIT:menue[B_NUM_LINES],
            DB_DBG_OFFSET:menue[B_CURRENT_LINE]
            ]),0);
    if (lines == 0 || !pointerp(lines))
        return ({ "Keine Anzeige." });
    for (int i = 0;i<sizeof(lines);i++)
    {
        lines[i] = sprintf("%3d %s",i+1+menue[B_CURRENT_LINE],lines[i]);
    }
    return lines;
}

static string banks_with_safe_files_prompt(mapping menue)
{
    string title = "Banken mit ausgelagerten Dateien";
    
    return sprintf("%s (%d/%d) [<nr>,z,q]",
        title,menue[B_CURRENT_LINE], banks_with_safe_files_total(menue));
}

static mixed banks_with_safe_files_action(string str, mapping * menues)
{
    mapping menue = menues[<1];
    mixed lines;
    int i;
    if (!str2int(space(str),&i) && i>0 && i <= banks_with_safe_files_total(menue))
    {
        lines = ZENTRALBANK->retrieve_banks_with_safe_files(
            menue+([ ZB_BANK_OUTPUT:0, DB_DBG_LIMIT:1, DB_DBG_OFFSET:i-1, ]),0);
        //DEBUG(sprintf("banks_with_safe_files_action: %d %Q %Q",i,lines,menue));
        if (sizeof(lines))
        {
            return menues + ({ menue + ([
                        B_TYPE: "bank_safe_item_ids",
                        ZB_BANK_OUTPUT: ZB_BANK_ITEMID,
                        ZB_BANK_SORTBY: ZB_BANK_ITEMID,
                        ZB_BANK_ID: lines[0],
                    ]) });
       }
    }

    return B_NOTHING;
}

//-------------------------------------------------------------------------

private string * get_bank_safe_item(mapping menue)
{
    string *lines=({});
    int ix=0;
    mixed result = ZENTRALBANK->retrieve_bank_safe_files(
            ([ ZB_BANK_ITEMID:menue[ZB_BANK_ITEMID] ]),0);
    string oneline,*split,*bs_lfc = ZENTRALBANK->get_timed_lfc(
            ([ ZB_BANK_ITEMID:menue[ZB_BANK_ITEMID] ]),0);
    //bwf.file,bwf.itemid, bw.bank_id, bw.owner, bw.item, bw.error_count
    mapping vitem,cache;
    if (!sizeof(result))
    {
        return ({ "Ungültiger Gegenstand, Datenfehler!" });
    }
    vitem = restore_value(result[0][4]);
    lines += ({ "Itemid: "+result[0][1] + " Errors: "+result[0][5] });
    lines += ({ "Bank: "+result[0][2]+" Eigentümer: "+result[0][3] });
    lines += ({ "Name: "+vitem["name"] });
    lines += explode(wrap("IDs: "+liste(vitem["id"],", ")),"\n");
    lines += explode(wrap("LONG:\n"+vitem["long"]),"\n");
    cache = ([]);
    lines += ({ "Objekte mit Erschaffungszeitpunkt:"});
    foreach(oneline : bs_lfc)
    {
        split = explode(oneline," ");
        cache[++ix] = split[0];
        lines+= ({ sprintf("%3d ",ix)+left(implode(split[1..]," "),75) });
    }
    lines += ({ "Dateien:" });
    lines += map(result,(: $1[0] :));
    menue[ZB_DATA_CACHE] = cache;
    return lines;
}

static mixed bank_safe_item_init(mapping old)
{
    old[B_START_LINE] =0;
    return old;
}

static int bank_safe_item_total(mapping menue)
{
    return sizeof(get_bank_safe_item(menue));
}

static string * bank_safe_item_display(mapping menue)
{
    menue[B_DATA] = get_bank_safe_item(menue);
    return staticmore_display(menue);
}

static string bank_safe_item_prompt(mapping menue)
{
    return sprintf("Gegenstandsanzeige %d,%d [<nr>,z,q]",menue[B_CURRENT_LINE],
        sizeof(get_bank_safe_item(menue)));
}

static mixed bank_safe_item_action(string str, mapping * menues)
{
    mapping menue = menues[<1],cache = menue[ZB_DATA_CACHE];
    int i;
    if (!str2int(space(str),&i) && i>0 && member(cache,i))
    {
        return menues+({([
            B_TYPE: "bank_safe_unique_item",
            ZB_BANK_UNIQUE_ID: cache[i],
            ZB_BANK_ITEMID:menue[ZB_BANK_ITEMID],
        ])});
    }
    return B_NOTHING;
}

//-------------------------------------------------------------------------

private string * get_bank_safe_unique_item(mapping menue)
{
    string *lines=({});
    mapping result = ZENTRALBANK->retrieve_bank_unique_item(
            menue[ZB_BANK_UNIQUE_ID], menue[ZB_BANK_ITEMID]);
// ZB_BANK_ITEMID,ZB_BANK_FILE,ZB_BANK_CREATOR,ZB_BANK_UNIQUE_ID,ZB_BANK_ITEM
// ZB_BANK_CREATED_ON
    mapping vitem;
    if (!sizeof(result))
    {
        return ({ "Ungültiger Gegenstand, Datenfehler!" });
    }
    vitem = result[ZB_BANK_ITEM];
    lines += ({ "Unique_Id: "+result[ZB_BANK_UNIQUE_ID] });
    lines += ({ "Itemid: "+result[ZB_BANK_ITEMID] });
    lines += ({ "Datei: "+result[ZB_BANK_FILE] });
    lines += ({ "Creator: "+result[ZB_BANK_CREATOR] });
    lines += ({ "Created On: "+shorttimestr(result[ZB_BANK_CREATED_ON]) });
    lines += ({ "Creator: "+result[ZB_BANK_CREATOR] });
    lines += ({ "Short: "+vitem["short"] });
    lines += explode(wrap("IDs: "+liste(vitem["id"],", ")),"\n");
    lines += explode(wrap("LONG:\n"+vitem["long"]),"\n");
    return lines;
}

static mixed bank_safe_unique_item_init(mapping old)
{
    old[B_START_LINE] =0;
    return old;
}

static int bank_safe_unique_item_total(mapping menue)
{
    return sizeof(get_bank_safe_unique_item(menue));
}

static string * bank_safe_unique_item_display(mapping menue)
{
    menue[B_DATA] = get_bank_safe_unique_item(menue);
    return staticmore_display(menue);
}

static string bank_safe_unique_item_prompt(mapping menue)
{
    return sprintf("Inhaltsanzeige %d,%d [z,q]",menue[B_CURRENT_LINE],
        sizeof(get_bank_safe_unique_item(menue)));
}

static mixed bank_safe_unique_item_action(string str, mapping * menues)
{
    return B_NOTHING;
}

//-------------------------------------------------------------------------
static mixed bank_request_files_init(mapping old)
{
    old[B_START_LINE] =0;
    return old;
}

static int bank_request_files_total(mapping menue)
{
    return ZENTRALBANK->get_file_requests(menue,1);
}

static string * bank_request_files_display(mapping menue)
{
    string * lines = ZENTRALBANK->get_file_requests(
        menue+([
            DB_DBG_LIMIT:menue[B_NUM_LINES],
            DB_DBG_OFFSET:menue[B_CURRENT_LINE]
            ]),0);
    if (lines == 0 || !pointerp(lines))
        return ({ "Keine Anzeige." });
    for (int i = 0;i<sizeof(lines);i++)
    {
        lines[i] = left(sprintf("%3d %s",
                        i+1+menue[B_CURRENT_LINE],lines[i]),78);
    }
    return lines;
}

static string bank_request_files_prompt(mapping menue)
{
    return sprintf("Anträge nach Dateiname %d,%d [<nr>,f <nr>,z,q]",
        menue[B_CURRENT_LINE], bank_request_files_total(menue));
}

static mixed bank_request_files_action(string str, mapping * menues)
{
    mapping menue = menues[<1];
    string *split;
    mixed lines;
    int i;
    split = explode(space(str)," ");
    switch (lower_case(split[0]))
    {
        case "f":
            if (sizeof(split)==2)
            {
                if (!str2int(space(split[1]),&i) && i>0 
                    && i <= bank_request_files_total(menue))
                {
                    lines = ZENTRALBANK->get_file_requests(
                        menue+([ ZB_BANK_OUTPUT:0, 
                                 DB_DBG_LIMIT:1, 
                                 DB_DBG_OFFSET:i-1, ]),0);
                    if (!sizeof(lines))
                    {
                        browse_write_line("Fehler bei der Auswahl der Datei!");
                        return B_DONE;
                    }
                    lines = explode(lines[0]," ");
                    if (!sizeof(lines))
                    {
                        browse_write_line("Fehler bei der Auswahl der Datei.");
                        return B_DONE;
                    }
                    if (!adminp(TP))
                    {
                        browse_write_line("Keine Freigaberecht!");
                        return B_DONE;
                    }
                    if (sizeof(lines)>2)
                    {
                        if (!ZENTRALBANK->release_bank_safe_factory_file(
                            lines[0],lines[1],lines[2][1..<2]))
                        {
                            browse_write_line("Interner Fehler!");
                            return B_DONE;
                        }
                    }
                    else if (!ZENTRALBANK->release_bank_safe_file(lines[0]))
                    {
                        browse_write_line("Interner Fehler.");
                        return B_DONE;
                    }
                    browse_write_line("Freigabe erteilt, Anträge gelöscht.");
                    return menues+({([
                        B_TYPE: "bank_safe_filter",
                        ZB_BANK_FILE: lines[0],
                    ])});
                }
            }
            browse_write_line("f <nr>   für Freigabe der Datei.");
            return B_DONE;
        default:
            if (!str2int(space(str),&i) && i>0 
                    && i <= bank_request_files_total(menue))
            {
                lines = ZENTRALBANK->get_file_requests(
                    menue+([ ZB_BANK_OUTPUT:0, 
                             DB_DBG_LIMIT:1, 
                             DB_DBG_OFFSET:i-1, ]),0);
                if (!sizeof(lines))
                {
                    browse_write_line("Fehler bei der Auswahl der Datei!");
                    return B_DONE;
                }
                lines = explode(lines[0]," ");
                if (!sizeof(lines))
                {
                    browse_write_line("Fehler bei der Auswahl der Datei.");
                    return B_DONE;
                }
                return menues + ({([
                    B_TYPE : "bank_requests_per_file",
                    ZB_BANK_FILE : lines[0],
                    ])});
            }
    }
    return B_NOTHING;
}

//-------------------------------------------------------------------------
static mixed bank_requests_per_file_init(mapping old)
{
    old[B_START_LINE] =0;
    return old;
}

static int bank_requests_per_file_total(mapping menue)
{
    return ZENTRALBANK->get_requests_per_file(menue,1);
}

static string * bank_requests_per_file_display(mapping menue)
{
    string * lines = ZENTRALBANK->get_requests_per_file(
        menue+([
            DB_DBG_LIMIT:menue[B_NUM_LINES],
            DB_DBG_OFFSET:menue[B_CURRENT_LINE]
            ]),0);
    if (lines == 0 || !pointerp(lines))
        return ({ "Keine Anzeige." });
    return lines;
}

static string bank_requests_per_file_prompt(mapping menue)
{
    return sprintf("Anträge für Datei %s %d,%d [<nr>,z,q]",
        space(right(menue[ZB_BANK_FILE],50)),menue[B_CURRENT_LINE],
        menue[B_END_LINE]);
}

static mixed bank_requests_per_file_action(string str, mapping * menues)
{
    return B_NOTHING;
}

//-------------------------------------------------------------------------

static mixed bank_safe_filters_init(mapping old)
{
    old[B_START_LINE] =0;
    return old;
}

static int bank_safe_filters_total(mapping menue)
{
    return ZENTRALBANK->get_bank_safe_filters(menue,1);
}

static string * bank_safe_filters_display(mapping menue)
{
    int i;
    mapping cache = ([]);
    string * lines = ZENTRALBANK->get_bank_safe_filters(menue+([
            DB_DBG_LIMIT:menue[B_NUM_LINES],
            DB_DBG_OFFSET:menue[B_CURRENT_LINE]
            ]),0);
    if (lines == 0 || !pointerp(lines))
        return ({ "Keine Anzeige." });
    for (i=0;i<sizeof(lines);i++)
    {
        cache[i+menue[B_CURRENT_LINE]+1] = lines[i];
        lines[i] = sprintf("%3d %s",1+i+menue[B_CURRENT_LINE],lines[i]);
    }
    menue[ZB_DATA_CACHE] = cache;
    return lines;
}

static string bank_safe_filters_prompt(mapping menue)
{
    string hint="";
    switch(menue[ZB_BANK_HINT])
    {
        default:
            hint = "";break;
        case 1:
            hint = "mit Hinweistexten";break;
        case 2:
            hint = "ohne Hinweistexte";break;
    }
    return sprintf("Freigegebene Objekte und Shadows %s(%d,%d) [<nr>,h,z,q]",
        hint,menue[B_CURRENT_LINE]+1,menue[B_END_LINE]+1);
}

static mixed bank_safe_filters_action(string str, mapping * menues)
{
    int i;
    mapping menue = menues[<1],cache = menue[ZB_DATA_CACHE];
    string *split = explode(str||""," ");
    switch (lower_case(space(split[0])))
    {
        case "h":
            menue[ZB_BANK_HINT]++;
            if (menue[ZB_BANK_HINT]>2)
                m_delete(menue,ZB_BANK_HINT);
            browse_write_line("Hinweisoption weitergeschaltet.");
            menues[<1]= menue;
            return menues;
            
    }
    if (!str2int(space(str),&i) && i>0 && member(cache,i))
    {
        return menues+({([
            B_TYPE: "bank_safe_filter",
            ZB_BANK_FILE: cache[i],
        ])});
    }
    return B_NOTHING;
}
//-------------------------------------------------------------------------

private string* list_flags(int flags)
{
    string * tmp = ({});
    if (flags & ZB_BANK_ACTIVE)
    {
        tmp += ({"Aktiv"});
    }
    if (flags & ZB_F_WAREHOUSE_STORE)
    {
        tmp += ({"Anti-No-Store"});
    }
    if (flags & ZB_F_VALID_CONTAINER)
    {
        tmp += ({"Valid-Container"});
    }
    if (flags & ZB_F_VALID_IN_CONTAINER)
    {
        tmp += ({"Valid-in-Container"});
    }
    if (flags & ZB_F_SPECIAL_CONTAINER)
    {
        tmp += ({"Special-Container"});
    }
    if (flags & ZB_F_SHADOWS)
    {
        tmp += ({"Shadow"});
    }
    if (flags & ZB_F_AUTOLOADER)
    {
        tmp += ({"Autoloader"});
    }
    if (flags & ZB_F_NPC)
    {
        tmp += ({"NPC"});
    }
    return tmp;
}

private string* get_one_filter(mapping fil)
{
    string* lines = ({});
    int flags;
    if (!mappingp(fil)) return ({"Keine Daten."});
    if (member(fil,ARMA_FACTORY))
    {
        lines += ({ "(Y) Factory: "+fil[ARMA_FACTORY]});
        lines += ({ "(I) Factory-ID:"+fil[ARMA_FACTORY_ID]});
    }
    lines += ({"(D) Dateiname: "+fil[ZB_BANK_FILE]});
    flags = fil[ZB_BANK_FLAGS];
    lines += explode(wrap_say("(F) Flags: ",liste(
                list_flags(flags),", ")),"\n")[..<2];
    if (member(fil,ZB_BANK_REPLACE_WITH))
    {
        lines += ({ "(R) Replace-With: "+fil[ZB_BANK_REPLACE_WITH] });
    }
    if (member(fil,ZB_BANK_HINT))
    {
        lines += ({ "(H) Hinweis: "+fil[ZB_BANK_HINT]||"" });
    }
    return lines;
}

static mixed bank_safe_filter_init(mapping old)
{
    old[B_START_LINE] =0;
    return old;
}

static int bank_safe_filter_total(mapping menue)
{
    return sizeof(get_one_filter(
            ZENTRALBANK->get_one_bank_safe_filter(menue[ZB_BANK_FILE])
        ));
}

static string * bank_safe_filter_display(mapping menue)
{
    menue[B_DATA] = get_one_filter(
            ZENTRALBANK->get_one_bank_safe_filter(menue[ZB_BANK_FILE])
        );
    return staticmore_display(menue);
}

static string bank_safe_filter_prompt(mapping menue)
{
    return "Bank-Safe-Filter [d <pfad>,f,r <pfad>,z,q]";
}

static mixed bank_safe_filter_action(string str, mapping * menues)
{
    mapping menue = menues[<1],fil;
    string * split = explode(space(str)," ");
    int ix,flags;
    switch (lower_case(split[0]))
    {
        case "d": // Dateinamen aendern
            if (!adminp(TP))
            {
                browse_write_line("Kein Zugriff.");
                return B_DONE;
            }
            if (sizeof(split)<=1)
            {
                browse_write_line("d <vollst.dateipfad>");
                return B_DONE;
            }
            if (!ZENTRALBANK->set_one_bank_safe_filter(menue[ZB_BANK_FILE],
                    ([ZB_BANK_FILE:split[1] ]) ) )
            {
                browse_write_line("Keine Änderung.");
                return B_DONE;
            }
            menue[ZB_BANK_FILE] = split[1];
            browse_write_line("Dateinamen geändert.");
            return B_REBUILT;
        case "r": // replace with aendern
            if (!adminp(TP))
            {
                browse_write_line("Kein Zugriff.");
                return B_DONE;
            }
            if (sizeof(split)<=1)
            {
                split += ({""});
            }
            if (!ZENTRALBANK->set_one_bank_safe_filter(menue[ZB_BANK_FILE],
                    ([ZB_BANK_REPLACE_WITH:split[1] ]) ) )
            {
                browse_write_line("Keine Änderung.");
                return B_DONE;
            }
            browse_write_line("Replace-With geändert.");
            return B_REBUILT;
        case "h": // Hinweis
            if (!adminp(TP) && !MAY_WRITE(menue[ZB_BANK_FILE]+".c",TP))
            {
                browse_write_line("Kein Zugriff.");
                return B_DONE;
            }
            if (sizeof(split)<=1)
            {
                split += ({""});
            }
            if (!ZENTRALBANK->set_one_bank_safe_filter(menue[ZB_BANK_FILE],
                    ([ZB_BANK_HINT:implode(split[1..]," ") ]) ) )
            {
                browse_write_line("Keine Änderung.");
                return B_DONE;
            }
            browse_write_line("Hinweis geändert.");
            return B_REBUILT;
        case "f": // Flags aendern.
            if (sizeof(split)< 2)
            {
                browse_write_line(
"f 1: Aktiv; f 2:Anti-No-Store; f 3:Valid-Container; f 4:Valid-in-Container");
                browse_write_line(
"f 5: Shadow; f 6:Special Container; f 7:Autoloader; f 8:Valid NPC");
                return B_DONE;
            }
            ix = to_int(split[1]);
            if (ix < 1 || ix > 8)
            {
                browse_write_line(
"f 1: Aktiv; f 2:Anti-No-Store; f 3:Valid-Container; f 4:Valid-in-Container");
                browse_write_line(
"f 5: Shadow; f 6:Special Container; f 7:Autoloader; f 8:Valid NPC");
                return B_DONE;
            }
            if (!adminp(TP))
            {
                browse_write_line("Kein Zugriff.");
                return B_DONE;
            }
            fil = ZENTRALBANK->get_one_bank_safe_filter(menue[ZB_BANK_FILE]);
            flags = fil[ZB_BANK_FLAGS];
            switch (ix)
            {
                case 1: flags ^= ZB_BANK_ACTIVE; break;
                case 2: flags ^= ZB_F_WAREHOUSE_STORE; break;
                case 3: flags ^= ZB_F_VALID_CONTAINER; break;
                case 4: flags ^= ZB_F_VALID_IN_CONTAINER; break;
                case 5: flags ^= ZB_F_SHADOWS; break;
                case 6: flags ^= ZB_F_SPECIAL_CONTAINER; break;
                case 7: flags ^= ZB_F_AUTOLOADER; break;
                case 8: flags ^= ZB_F_NPC; break;
            }
            if (!ZENTRALBANK->set_one_bank_safe_filter(menue[ZB_BANK_FILE],
                    ([ZB_BANK_FLAGS:flags ]) ) )
            {
                browse_write_line("Keine Änderung.");
                return B_DONE;
            }
            browse_write_line("Flags geändert.");
            return B_REBUILT;
    }
    return B_NOTHING;
}

//-------------------------------------------------------------------------
static mixed file_statistics_init(mapping old)
{
    old[B_START_LINE] = 0;
    return old;
}

static int file_statistics_total(mapping menue)
{
    return ZENTRALBANK->get_file_statistik(menue,1);
}

static string * file_statistics_display(mapping menue)
{
    string *lines = ZENTRALBANK->get_file_statistik(menue+
        ([  DB_DBG_LIMIT:menue[B_NUM_LINES],
            DB_DBG_OFFSET:menue[B_CURRENT_LINE]
        ]),0);
    int ix;
    mapping cache = ([]);
    for (ix = 0;ix < sizeof(lines); ix++)
    {
        cache[ix+menue[B_CURRENT_LINE]+1] = explode(lines[ix]," ")[<1];
        lines[ix] = sprintf("%3d: %s",ix+menue[B_CURRENT_LINE]+1,lines[ix]);
    }
    menue[ZB_DATA_CACHE] = cache;
    return lines;
}

static string file_statistics_prompt(mapping menue)
{
    return sprintf("Statistik eingelagerte Dateien (%d,%d) [<nr>,z,q]",
        menue[B_CURRENT_LINE]+1,menue[B_END_LINE]+1);
}

static mixed file_statistics_action(string str, mapping * menues)
{
    int ix = to_int(space(str));
    mapping cache = menues[<1][ZB_DATA_CACHE];
    if (member(cache,ix))
    {
        return menues + ({([ B_TYPE:"bank_safe_item_ids",
                      ZB_BANK_FILE: cache[ix],
                ]) });
    }
    return B_NOTHING;
}

//-------------------------------------------------------------------------
static mixed owner_statistics_init(mapping old)
{
    old[B_START_LINE] = 0;
    return old;
}

static int owner_statistics_total(mapping menue)
{
    return ZENTRALBANK->get_owner_statistik(menue,1);
}

static string * owner_statistics_display(mapping menue)
{
    string *lines = ZENTRALBANK->get_owner_statistik(menue+
        ([  DB_DBG_LIMIT:menue[B_NUM_LINES],
            DB_DBG_OFFSET:menue[B_CURRENT_LINE]
        ]),0);
    int ix;
    mapping cache = ([]);
    for (ix = 0;ix < sizeof(lines); ix++)
    {
        cache[ix+menue[B_CURRENT_LINE]+1] = explode(lines[ix]," ")[0];
        lines[ix] = sprintf("%3d: %s",ix+menue[B_CURRENT_LINE]+1,lines[ix]);
    }
    menue[ZB_DATA_CACHE] = cache;
    return lines;
}

static string owner_statistics_prompt(mapping menue)
{
    return sprintf("Statistik Eigentümer (%d,%d) [<nr>,z,q]",
        menue[B_CURRENT_LINE]+1,menue[B_END_LINE]+1);
}

static mixed owner_statistics_action(string str, mapping * menues)
{
    int ix = to_int(space(str));
    mapping cache = menues[<1][ZB_DATA_CACHE];
    if (member(cache,ix))
    {
        return menues + ({([ B_TYPE:"bank_safe_item_ids",
                      ZB_BANK_OWNER: cache[ix],
                ]) });
    }
    return B_NOTHING; // TODO verzweigen je Datei.
}

//-------------------------------------------------------------------------
static mixed overview_statistics_init(mapping old)
{
    old[B_START_LINE] = 0;
    old[B_DATA] = ZENTRALBANK->get_schliessfach_uebersicht();
    return old;
}

static int overview_statistics_total(mapping menue)
{
    return sizeof(menue[B_DATA]);
}

static string * overview_statistics_display(mapping menue)
{
    return menue[B_DATA];
}

static string overview_statistics_prompt(mapping menue)
{
    return sprintf("Gesamtstatistik (%d,%d) [z,q]",
        menue[B_CURRENT_LINE]+1,menue[B_END_LINE]+1);
}

static mixed overview_statistics_action(string str, mapping * menues)
{
    return B_NOTHING;
}

//-------------------------------------------------------------------------
static mixed loadfile_creator_statistics_init(mapping old)
{
    old[B_START_LINE] = 0;
    return old;
}

static int loadfile_creator_statistics_total(mapping menue)
{
    return ZENTRALBANK->get_load_file_statistik(menue,1);
}

static string * loadfile_creator_statistics_display(mapping menue)
{
    string *lines = ZENTRALBANK->get_load_file_statistik(menue+
        ([  DB_DBG_LIMIT:menue[B_NUM_LINES],
            DB_DBG_OFFSET:menue[B_CURRENT_LINE]
        ]),0);
    // TODO nummer und cache
    return map(lines,(: left($1,79) :));
}

static string loadfile_creator_statistics_prompt(mapping menue)
{
    return sprintf("Loadfile/Creator-Statistik (%d,%d) [z,q]",
        menue[B_CURRENT_LINE]+1,menue[B_END_LINE]+1);
}

static mixed loadfile_creator_statistics_action(string str, mapping * menues)
{
    return B_NOTHING;
}

//-------------------------------------------------------------------------

static mixed oldest_statistics_init(mapping old)
{
    old[B_START_LINE] = 0;
    return old;
}

static int oldest_statistics_total(mapping menue)
{
    return ZENTRALBANK->get_oldest_statistik(menue,1);
}

static string * oldest_statistics_display(mapping menue)
{
    string *lines = ZENTRALBANK->get_oldest_statistik(menue+
        ([  DB_DBG_LIMIT:menue[B_NUM_LINES],
            DB_DBG_OFFSET:menue[B_CURRENT_LINE]
        ]),0);
    int ix;
    string *expline;
    mapping cache = ([]);
    for (ix = 0;ix < sizeof(lines); ix++)
    {
        expline = explode(lines[ix]," ");
        cache[ix+menue[B_CURRENT_LINE]+1] = expline[0];
        lines[ix] = left(sprintf("%3d: %s",ix+menue[B_CURRENT_LINE]+1,
            implode(expline[1..]," ")),79);
    }
    menue[ZB_DATA_CACHE] = cache;
    return lines;
}

static string oldest_statistics_prompt(mapping menue)
{
    return sprintf("Statistik nach Alter (%d,%d) [<nr>,z,q]",
        menue[B_CURRENT_LINE]+1,menue[B_END_LINE]+1);
}

static mixed oldest_statistics_action(string str, mapping * menues)
{
    int ix = to_int(space(str));
    mapping cache = menues[<1][ZB_DATA_CACHE];
    if (member(cache,ix))
    {
        return menues + ({([ B_TYPE:"bank_safe_item",
                        ZB_BANK_ITEMID: cache[ix],
                ]) });
    }
    return B_NOTHING;
}

//-------------------------------------------------------------------------

int cmd_menue(string str)
{
    if (!check_security())
    {
        FAILWP("Zugriffsproblem.", FAIL_INTERNAL);
    }
    dynamic_browse( ([ B_TYPE: "sub_menu" ]) );
    return 1;
}

void init()
{
    "*"::init();
    add_action("cmd_menue","menü",-3);
}

void create()
{
  "*"::create();

  set_own_light(1);
  add_type("kunstlicht",1);
  add_type("nocleanup",1);
  set_room_domain("Pantheon");
  set_short("In den Räumen des Bankenkonsortiums");
  set_long("Dies ist die Bankenaufsicht. Befehl: menü");
        
    init_security_for_actions();
    
  add_exit("test_schliessfaecher","runter",0,"Testbank");
  add_exit("../rathaus/forum","forum",0,"Zurück");
  touch("/room/bank/touch/warenkorb").move(TO);
  touch("/room/bank/touch/warenkorb").set_no_move(1);
}

static void clear_rescue()
{
    to_rescue = ({});
}

varargs void moved_out(mapping mv_infos)
{
    if (!mv_infos[MOVE_NEW_ROOM])
    {
        ::moved_out(mv_infos);
        return;
    }
    if ( (mv_infos[MOVE_FLAGS]&MOVE_ATOM_NOT_NOTIFY) &&
            (object_name(mv_infos[MOVE_NEW_ROOM]) == "/room/void"))
        to_rescue += ({ mv_infos[MOVE_OBJECT] });
    if (find_call_out("clear_rescue")==-1) 
        call_out("clear_rescue",0);
    ::moved_out(mv_infos);
}

void abort_renewal() 
{
    to_rescue->move(this_object());
    clear_rescue();
}
void finish_renewal(object neu) {}
void prepare_renewal() {}

/* --- End of file. --- */
