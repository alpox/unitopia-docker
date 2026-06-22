// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/sys/parse_com.h
// Description: Defines fuer parse_com()
// Author:	Francis

#ifndef PARSE_COM_H
#define PARSE_COM_H 1

/*
 * Von parse_com zurueckgegebene Return-Codes.
 */

#define PARSE_OK           0	// Alles klar
#define PARSE_NO_ARG       1	// Es wurde nichts uebergeben
#define PARSE_NO_OB        2	// Objekt nicht gefunden
#define PARSE_NO_ALL_OB    3	// Wie 2, aber "alles" war angegeben
#define PARSE_NO_MY_OB     4	// Wie 2, aber "mein" war angegeben
#define PARSE_NO_ALL_MY_OB 5	// Wie 2, aber "mein" und "alles" war angegeben
#define PARSE_WRONG_ID     6	// Unvollstaendige/falsche Objekt-Id angegeben
#define PARSE_NOT_SEARCHED 7	// Nicht gesucht
#define PARSE_NOT_ENOUGH   8    // Countob: Nicht genuegend Elemente da.
#define PARSE_CANT_SEE     9	// "alle" wurde verwendet, kann aber nix sehen.

/*
 * Die einzelnen Felder des Return-Wertes:
 */
#define PARSE_NUM_OBS		0
#define PARSE_OBS		1
#define PARSE_REST		2
#define PARSE_RET_CODE		3
#define PARSE_ID		4
#define PARSE_TRENNER		5
#define PARSE_BEYOND_TRENNER	6

/*
 * Steuer-Flags
 */

// Ist im Kommando kein Zahlwort angegeben
// (1. 2. ..), sollen ALLE passenden Objekte
// zurueckgegeben werden, nicht nur das erste.
#define PARSE_ALL_MATCHES		0x01

// Keine Virtuellen Objekte suchen
#define PARSE_NO_V_ITEMS		0x02

#define PARSE_AFTER_TRENNER		0x04

#define PARSE_NOT_WITHOUT_TRENNER	0x08

#define PARSE_NOSHIMMER_INVIS		0x10

#define PARSE_DONT_SPLIT		0x20

/*
 * Fuer virtuelle Objekte
 */

#if __VERSION__ < "3.6.0-U043"
// Variablenclosures moegen keine Parameter... :-(
#define QUERY(x,ob) (mappingp(ob)								\
	?((closurep((ob)[x])&&get_type_info((ob)[x],1)==2)					\
	    ?funcall((ob)[x]):((member((x),':')>=0&&((ob)["v_item_master"]))						\
	        ?(ob)["v_item_master"]->query_v_item_property((ob),(x)):funcall((ob)[x],ob)))	\
	:(call_other(ob,"query_"+(x)) || call_other(ob, "query", (x))))

// Variablenclosures moegen keine Parameter... :-(
#define QUERY_PARS(x,ob,pars) (mappingp(ob)									\
	?((closurep((ob)[x])&&get_type_info((ob)[x],1)==2)							\
	    ?funcall((ob)[x]):member((x),':')>=0									\
	        ?(ob)["v_item_master"]->query_v_item_property((ob),(x),(pars)):apply((ob)[x],(ob),(pars)))	\
	:(apply(#'call_other,ob,"query_"+(x),(pars)) || apply(#'call_other,ob,"query",(x),(pars))))

#endif

#if __VERSION__ < "3.6.1-U048"
#define HAS_ID(ob,x) (mappingp(ob)?(member((ob)["id"]||({(ob)["name"]}),x)>=0):(ob)->id(x))
#endif

/*
 * Synonyme fuer "alle"
 */
#define ALLES ({"alle","alles","allem","allen","jeder","jede","jedes","jedem","jeden"})

#endif // PARSE_COM_H
