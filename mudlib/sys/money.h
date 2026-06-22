// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/sys/money.h
// Description:	Defines fuer Geld

#ifndef MONEY_H
#define MONEY_H 1

#define ZENTRALBANK "/apps/zentralbank"

#define PATH_ROOM_BANK "/room/bank/"
#define ZB_SCHLIESSFAECHER  PATH_ROOM_BANK "obj/schliessfaecher"
#define ZB_SCHLIESSFACH     PATH_ROOM_BANK "obj/schliessfach"
#define ZB_MIETUEBERSICHT   PATH_ROOM_BANK "obj/mietuebersicht"
#define ZB_ARMATESTER       PATH_ROOM_BANK "obj/armatester"
#define ZB_ANLEITUNG_SF     PATH_ROOM_BANK "Anleitung"
#define ZB_KRITERIEN        PATH_ROOM_BANK "Kriterien"
#define ZB_RILIS            PATH_ROOM_BANK "ProgrammierRilis"
#define ZB_BANKENAUFSICHT   PATH_ROOM_BANK "bankenaufsicht"
#define ZB_TEST_FACTORY     PATH_ROOM_BANK "test_factory"

#define ZB_DEBUGLOG(m,s,u,c) (ZENTRALBANK->debuglog(m,s,u,c))
#define ZB_CONSERVATION_CALLER (program_name(previous_object()) \
                                     ==(ZB_SCHLIESSFAECHER ".c"))

#define ZB_SCHLIESSFACH_ID      "zentralbank # schliess # fach"
#define ZB_SCHLIESSFAECHER_ID   "zentralbank # schliess # faecher"
#define ZB_ARMATESTER_ID        "zentralbank # arma # tester"
#define ZB_MIETUEBERSICHT_ID    "zentralbank # miet # uebersicht"

#define ARMA_FACTORY        "Root:Arma:Factory:File"
#define ARMA_FACTORY_ID     "Root:Arma:Factory:Indentifier"
#define ARMA_OBJECT_DATA    "Root:Arma:Data:Object"
#define ARMA_SHADOW_DATA    "Root:Arma:Data:Shadow"
#define ARMA_MUDLIB_DATA    "Root:Arma:Data:Mudlib"
#define ARMA_CONTENT_DATA   "Root:Arma:Data:Content"
#define ARMA_PROPERTIES_DATA    "Root:Arma:Data:Properties"
#define ARMA_LOAD_FILE      "Root:Arma:LoadFile"
#define ARMA_SPECIAL_ID     "Root:Arma:Special:ID"
#define ARMA_ORIGIN_INFO    "Root:Arma:Origin:Info"
#define ARMA_FILE_COUNTER   "Root:Arma:File:Counter"

// Bankenflags
#define ZB_BANK_CLASSIC         0x1000
#define ZB_BANK_ENVIRONMENT     0x2000
#define ZB_BANK_PANTHEON        0x4000
#define ZB_BANK_SAFEROOM        0x8000
#define ZB_F_SAFE_LOCKED        0x0001
#define ZB_F_SAFE_TROPHY        0x0002
#define ZB_F_SAFE_TOUCH_OB      0x0004
#define ZB_F_SAFE_TMARKER       0x0008
#define ZB_BANK_ACTIVE          0x0010
#define ZB_F_WAREHOUSE_STORE    0x0020
#define ZB_F_VALID_CONTAINER    0x0040
#define ZB_F_VALID_IN_CONTAINER 0x0080
#define ZB_F_SPECIAL_CONTAINER  0x0100
#define ZB_F_SHADOWS            0x0200
#define ZB_F_AUTOLOADER         0x0400
#define ZB_F_NPC                0x0800
// Mappingkeys fuer Bankeninfo
#define ZB_BANK_ID              "zentralbank:bank:id"       // String
#define ZB_BANK_TITLE           "zentralbank:bank:title"    // String
#define ZB_BANK_FLAGS           "zentralbank:bank:flags"    // int
#define ZB_BANK_WIZARD          "zentralbank:bank:wizard"   // Object
#define ZB_BANK_WITH_ERROR      "zentralbank:bank:error"    // 0/1
#define ZB_BANK_OWNER           "zentralbank:bank:owner"    // string
#define ZB_BANK_FILE            "zentralbank:bank:file"     // string
#define ZB_BANK_FILE_PATTERN    "zentralbank:bank:file:pattern"// string
#define ZB_BANK_ITEMID          "zentralbank:bank:itemid"   // string
#define ZB_BANK_ITEM            "zentralbank:bank:item"   // mapping
#define ZB_BANK_UNIQUE_ID       "zentralbank:bank:unique:id"// String
#define ZB_BANK_OUTPUT          "zentralbank:bank:output"   // string
#define ZB_BANK_SORTBY          "zentralbank:bank:sortby"   // string
#define ZB_BANK_CURRENCY        "zentralbank:bank:currency" // string
#define ZB_DATA_CACHE           "zentralbank:data:cache"  // internes mapping
#define ZB_BANK_REPLACE_WITH    "zentralbank:bank:replace_with" // string
#define ZB_BANK_HINT            "zentralbank:bank:hint"  // string
#define ZB_BANK_UNIQUE_PATTERN  "zentralbank:bank:unique:pattern"  // string
#define ZB_BANK_CREATOR         "zentralbank:bank:creator"  // string
#define ZB_BANK_CREATED_ON      "zentralbank:bank:created:on"  // int time
#define ZB_DELETED_ON           "zentralbank:safe:deleted:on"//int time
#define ZB_DELETED_GT        "zentralbank:safe:deleted:greater:then"//int time

// Flags fuer Banken-NPCs
#define ZB_NPC_BANKIER              0x0001
#define ZB_NPC_GELD_WECHSLER        0x0002
#define ZB_NPC_BANKIER_WECHSLER     0x0003

// mapping checking_attributes: keys für den (pre)check_conservation, 
// um detailliert prüfen zu können oder objekte zur Generierung von Meldungen.
#define ZB_CHECK_TYPE   "zentralbank:check:type"    // int, s.u. ZB_CT_
#define ZB_CHECK_CON    "zentralbank:check:container" // object Schliessfach/Truhe
#define ZB_CHECKER_OB   "zentralbank:checker:object"  // object/mapping, was prüft
// Identfier für den ZB_CHECK_TYPE bei (pre)check_conservation
#define ZB_CT_UNKNOWN       0
#define ZB_CT_TEST_ONLY     1
#define ZB_CT_SCHLIESSFACH  2
#define ZB_CT_SPEZIALTRUHE  3
#define ZB_CT_AUKTION       4
#define ZB_CT_ALCH_KOFFER   5

// Rueckgabewerte von query_money_info
#define MONEY_VALUTA  0
#define MONEY_VALUTAS 1
#define MONEY_GENDER  2
#define MONEY_KURS    3
//freiwillige Angaben:
#define MONEY_FLAGS   4
#define MONEY_SG_IDS  5
#define MONEY_PL_IDS  6

// Flags:
#define MONEY_NOT_DEKLIN	0x01

// Gruende fuer deliver()
#define VD_NORMAL	0
#define VD_RECP_FULL	1
#define VD_RECP_UNAVAIL	2

#endif // MONEY_H
