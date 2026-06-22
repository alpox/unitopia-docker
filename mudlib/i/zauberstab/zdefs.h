// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/i/zauberstab/zdefs.h
// Description: Headerfile fuer Zauberstab
// Author:	Freaky (23.12.93)

#pragma strong_types

#undef DEBUG

#define OWN owner
#define HILFE_PFAD (HELP_PATH+"/goetter/zauberstab/")
#ifdef Orbit
#define LPC_FILE     "/w/"+(OWN->get_intermud_src_mud() == "UNItopia" ? lower_case(OWN->get_intermud_src_name()) : OWN->query_real_name())+"/LPC_zst.c"
#define INCLUDE_FILE "/w/"+(OWN->get_intermud_src_mud() == "UNItopia" ? lower_case(OWN->get_intermud_src_name()) : OWN->query_real_name())+"/zlpc.h"
#else
#define LPC_FILE     "/w/"+OWN->query_real_name()+"/LPC_zst.c"
#define INCLUDE_FILE "/w/"+OWN->query_real_name()+"/zlpc.h"
#endif
#define MARK_ICON "@"
#define ENV_ICON "umgebung"
#define LINE "-------------------------------------------------------------------------------\n"
#define LINE2(x) (sprintf("%'-':-*s",79,"--- "+(x)+" ")+"\n")
#define DIR 1
#define LAST 2
#define MSG 4
#define C_ARG 0
#define C_OBJ 1

#define CAP(x)	capitalize(x)
#define LOW(x)	lower_case(x)
#define ENV(x)	environment(x)
#define ENVTO	environment()
#define TO	this_object()
#define TP	this_player()
#define TI	this_interactive()
#define NAME(x) (x->query_name())
#define SHORT(x) (x->query_short(TP))
#define FN(x)	object_name(x)
#define ON(x)	object_name(x)
#define OFN(x)	"OBJ("+FN(x)+")"
#define OON(x)	"OBJ("+ON(x)+")"
#define FOR_ALL(x,y) for(x=first_inventory(y);x;x=next_inventory(x))
#define SO(x)	search_object(x,0)
#define VSO(x)	search_object(x,1)
#define SOT(x)	search_object(x,2)
#define FAIL(x) return (notify_fail(x) && 0)
#define TELEPORT(x) TELE_MASTER->get_tele(x)
#define TELEPORT_LISTE TELE_MASTER->all_teles()
#define TELEPORT_LISTE_FILTER(x) TELE_MASTER->filter_teles(x)
#define TOWN(x) (x->query_last_town())
#define INST(x) TOWN_MASTER->description(x)

#define HELP(x) if (strstr(x,query_verb_ascii())) \
		   return 0; \
		if (str=="?") { \
		   OWN->more(HILFE_PFAD+x,0,0,M_AUTO_END); \
		   return 1; \
		   } \
		BEGIN_PIPE(str);

// Nicht auf query_verb achten
#define HELP_NO_VERB(x) \
		if (str=="?") { \
		   OWN->more(HILFE_PFAD+x,0,0,M_AUTO_END); \
		   return 1; \
		   } \
		BEGIN_PIPE(str);

// Hilfe bei !str || str=="?"
#define HELP_NO_STR(x) if (strstr(x,query_verb_ascii())) \
		   return 0; \
		if (!str || str=="?") { \
		   OWN->more(HILFE_PFAD+x,0,0,M_AUTO_END); \
		   return 1; \
		   } \
		BEGIN_PIPE(str);

#define HELP_NO_PIPE(x) if (strstr(x,query_verb_ascii())) \
		   return 0; \
		if (str=="?") { \
		   OWN->more(HILFE_PFAD+x,0,0,M_AUTO_END); \
		   return 1; \
		   } \
		CLEAR_BUFFER;

#define SECURE	if (secure(query_verb_ascii(),str)) return 1
#define SECURE_LORD   if (!playerp(OWN) || !lordp(OWN)) \
			FAIL("Das darfst du nicht.\n");
#define SECURE_ADMIN  if (!playerp(OWN) || !adminp(OWN)) \
			FAIL("Das darfst du nicht.\n");
#define ADD_PATH OWN->add_path

#define LIV(x) (objectp(x) && living(x))
#define VENV(x) (mappingp(x)?x["environment"]:ENV(x))

string null_or_string(int i)
{
    return i?to_string(i):"";
}
#define CX(x) lambda(({'w}),({#'call_other,'w,"query_"x}))
#define CXS(x) lambda(({'w}),({#'null_or_string,({#'call_other,'w,"query_"x})}))
