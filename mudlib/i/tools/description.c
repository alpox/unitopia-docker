// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/i/tools/description.c
// Description: Definiert einige Bedingungen und Funktionen
// Author:	Gnomi

/*
FUNKTION: TI_LISTE
DEKLARATION: Liste der Beschreibungs-TI-Defines
BESCHREIBUNG:

Um die Funktionen dieses Inherits  zu nutzen, muss der Parameter fuer
die von compile_desc generierte Closure ein Mapping mit folgenden Elementen
sein:

   TI_VIEWER: Der Betrachter (ein Objekt)
   TI_ROOM:   Der Raum (ein Objekt)
   TI_ITEM:   0, falls ein Raum betrachtet wird, ansonsten
              der betrachtete Gegenstand (Objekt) oder V-Item (Mapping)
   TI_OBJECT: 0, falls TI_ITEM kein V-Item ist,
              sonst das Objekt, an welchem des V-Item haengt.
   TI_DARK:   0, wenn der Betracher sehen kann,
              1, wenn es fuer den Betrachter zu dunkel ist.
              (Kann auch eine Closure sein.)

Die Funktion mapping get_desc_info_mapping(object viewer) erstellt ein
solches Mapping fuer this_object().

VERWEISE: get_desc_info_mapping
GRUPPEN: beschreibung
*/

#pragma save_types
#pragma strong_types

inherit "/i/tools/description_core";

#include <description.h>
#include <eyes.h>
#include <invis.h>
#include <landschaft.h>
#include <level.h>
#include <message.h>
#include <room.h>
#include <stats.h>
#include <term.h>
#include <time.h>

// Nur fuer interne Zwecke!
mixed * get_light_descriptions()
{
    return ({
	({      0, "Es ist Geisterstunde."			}),
	({  10000, "Es ist Nacht."				}),
	({  40000, "Der Horizont hellt sich langsam auf."	}),
	({  42000, "Es dämmert."				}),
	({  45000, "Die Sonne geht auf."			}),
	({  51000, "Es ist kurz nach Sonnenaufgang."		}),
	({  54000, "Es ist früh am Morgen."			}),
	({  62000, "Es ist früher Vormittag."			}),
	({  73000, "Es ist vormittags."				}),
	({  90000, "Es ist spät am Vormittag."			}),
	({ 104000, "Es ist kurz vor Mittag."			}),
	({ 113000, "Die Sonne steht im Zenit."			}),
	({ 130000, "Es ist früher Nachmittag."			}),
	({ 143000, "Es ist nachmittags."			}),
	({ 161000, "Es ist spät am Nachmittag."		}),
	({ 174000, "Es ist früh am Abend."			}),
	({ 182000, "Es ist abends."				}),
	({ 190000, "Es ist spät abends."			}),
	({ 193000, "Es ist kurz vor Sonnenuntergang."		}),
	({ 195000, "Die Sonne geht unter."			}),
	({ 201000, "Es dämmert."				}),
	({ 205000, "Es wird dunkel."				}),
	({ 212000, "Es ist Nacht."				}),
	({ 240001, "Es ist Mitternacht."			})
    });
}

/*
FUNKTION: query_light_description
DEKLARATION: string query_light_description(int time)
BESCHREIBUNG:
Liefert die Tageszeitbeschreibung (z.B: "Es ist abends.")
fuer die in 'time' angegebene Zeit. 'time' ist dabei eine Zahl hhmmss
der Spielerzeit (wie von vclock() zurueckgeliefert).
VERWEISE: query_light, query_daylight
GRUPPEN: raum, licht
*/
string query_light_description(int time)
{
    int a;
    mixed *light_desc = get_light_descriptions();

    for (a=1; a<sizeof(light_desc); a++)
	if (time < light_desc[a][0])
	    return light_desc[a-1][1];
    return "Es ist irgendwann tagsüber.";
}

protected mixed desc_condition(string name, mixed info, mixed* par)
{
    mixed tmp;
    
    switch(name)
    {
	case T_ATOM_DAY:
	case T_ATOM_NIGHT:
	case T_ATOM_VCLOCK:
	    tmp = info[TI_ROOM];
	    tmp = (tmp && tmp->local_vclock()) || vclock();
	    switch(name)
	    {
		case T_ATOM_DAY:
		    return tmp >= BEGIN_DAYLIGHT && tmp < BEGIN_DARKNESS;
		case T_ATOM_NIGHT:
		    return tmp >= BEGIN_DARKNESS || tmp < BEGIN_DAYLIGHT;
		case T_ATOM_VCLOCK:
		    return tmp >= par[0] && tmp <= par[1];
	    }
	    break; // Shouldn't happen.

	case T_ATOM_CLOCK:
	    tmp = clock();
	    return tmp >= par[0] && tmp <= par[1];

	case T_ATOM_DARK:
	    return funcall(info[TI_DARK]);
	
	case T_ATOM_LIGHT:
	    return !funcall(info[TI_DARK]);
	
	case T_ATOM_BLIND:
	    return info[TI_VIEWER] && info[TI_VIEWER]->is_blind();

	case T_ATOM_EMPTY:
	    tmp = ({ 1, 0 });
	    // Fall through.
	case T_ATOM_VIS_EMPTY:
	    if(!tmp)
		tmp = ({ (: !($1->query_invis() & $2) :),
		    (sizeof(par)>1)?par[1]:V_INVIS });
	    
	    if(!info[TI_OBJECT] && !info[TI_ITEM] && !info[TI_ROOM])
		return 1;
	    
	    for(object ob=first_inventory(info[TI_OBJECT] || info[TI_ITEM] || info[TI_ROOM]);
		ob;
		ob = next_inventory(ob))
		    if(funcall(tmp[0], ob, tmp[1]))
			return 0;
	
	    return 1;

	case T_ATOM_INVIS_PRESENT:
	case T_ATOM_INVIS_PRESENT_CONT:
	case T_ATOM_INVIS_PRESENT_ROOM:
	    tmp = ({ (: $1->query_invis() & $2 :), par[1] });
	    // Fall through.
	case T_ATOM_VIS_PRESENT:
	case T_ATOM_VIS_PRESENT_CONT:
	case T_ATOM_VIS_PRESENT_ROOM:
	    if(!tmp)
		tmp = ({ (: !($1->query_invis() & $2) :),
		    (sizeof(par)>1)?par[1]:V_INVIS });
	    // Fall through.
	case T_ATOM_PRESENT:
	case T_ATOM_PRESENT_CONT:
	case T_ATOM_PRESENT_ROOM:
	    if(!tmp)
		tmp = ({ 0, 0 });
		
	    switch(name)
	    {
		case T_ATOM_PRESENT:
		case T_ATOM_INVIS_PRESENT:
		case T_ATOM_VIS_PRESENT:
		case T_ATOM_PRESENT_CONT:
		case T_ATOM_INVIS_PRESENT_CONT:
		case T_ATOM_VIS_PRESENT_CONT:
		    if(info[TI_OBJECT] || info[TI_ITEM])
		    {
			mixed ob = info[TI_OBJECT] || info[TI_ITEM];
			while(mappingp(ob))
			    ob = ob["environment"];
			if(ob)
			{
			    ob = cond_present(par[0], ob, tmp[0], tmp[1]);
			    if(ob)
				return 1;
			    if( name==T_ATOM_PRESENT_CONT || 
				name == T_ATOM_INVIS_PRESENT_CONT ||
				name == T_ATOM_VIS_PRESENT_CONT)
				return 0;
			}
		    }
		    // Fall through.
		case T_ATOM_PRESENT_ROOM:
		case T_ATOM_INVIS_PRESENT_ROOM:
		case T_ATOM_VIS_PRESENT_ROOM:
		    return info[TI_ROOM] &&
			cond_present(par[0], info[TI_ROOM], tmp[0], tmp[1]);
	    }
	    break;
	
	case T_ATOM_COND_PRESENT:
	{
	    // par = ({ TP_ALL/ROOM/CONTAINER/VIEWER, CDP-Flag, Id [, Fun [, Args]] })
	    object* where = ({});
	    switch(par[0])
	    {
		case TP_ALL:
		case TP_CONTAINER:
		    if(info[TI_OBJECT] || info[TI_ITEM])
			where = ({info[TI_OBJECT] || info[TI_ITEM]}) - ({0});
		    if(par[0] != TP_ALL)
			break;
		    // Fall through.
		case TP_ROOM:
		    where += ({info[TI_ROOM]});
		    break;
		case TP_VIEWER:
		    where = ({ info[TI_VIEWER] });
		    break;
	    }
	    
	    foreach(object ob: where - ({0}))
		if(apply(#'cond_deep_present, par[2], ob, par[1],
		    sizeof(par)>3?par[3]:0, sizeof(par)>4?par[4]:({})))
		    return 1;
	
	    return 0;
	}
	
	case T_ATOM_EXIT_FLAG:
	    return info[TI_ROOM] && info[TI_ROOM]->query_exit_flag(par[0], par[1]);

	case T_ATOM_GILDE:
	    return info[TI_VIEWER] && info[TI_VIEWER]->query_gilde()==par[0];
	    
	case T_ATOM_GENDER:
	    return info[TI_VIEWER] && info[TI_VIEWER]->query_gender()[0]==par[0][0];
	
	case T_ATOM_ROOMTYPE:
	    return info[TI_ROOM] && info[TI_ROOM]->query_type(par[0]);
	
	case T_ATOM_NO_ASCII_ART:
	    return info[TI_VIEWER] && info[TI_VIEWER]->query_no_ascii_art();
	
	case T_ATOM_UNICODE:
	    return info[TI_VIEWER] && info[TI_VIEWER]->has_unicode();
	
	case T_ATOM_ONLY_ASCII:
	    return info[TI_VIEWER] && info[TI_VIEWER]->has_only_ascii();
	
	case T_ATOM_FAR:
	    return info[TI_VIEWER] && info[TI_ROOM] &&
		!present(info[TI_VIEWER], info[TI_ROOM]);
	
	case T_ATOM_OWNER:
	    return info[TI_VIEWER] && (info[TI_OBJECT]||info[TI_ITEM]) &&
		(filter(all_environment(info[TI_OBJECT]||info[TI_ITEM])||({}), #'living)+
		    ({0}))[0] == info[TI_VIEWER];

	case T_ATOM_ENTRANCE:
	    if(!info[TI_ROOM] || !info[TI_VIEWER])
		return 0;
		
	    tmp = info[TI_ROOM]->query_temp_traces();
	    if(!member(tmp,info[TI_VIEWER]))
		return 0;

	    tmp = tmp[info[TI_VIEWER], TRTT_ORIGIN_FN];
	    return tmp==par[0] || tmp==domain2map(par[0]) ||
		   tmp==info[TI_ROOM]->query_one_exit(par[0]);
	
	case T_ATOM_GUEST:
	    return guestp(info[TI_VIEWER]);
	
	case T_ATOM_NEWBIE:
	    return newbiep(info[TI_VIEWER]);
	
	case T_ATOM_WIZ:
	    return wizp(info[TI_VIEWER]);
	
	case T_ATOM_SPIELERRAT:
	    return spielerratp(info[TI_VIEWER]);
	
	case T_ATOM_EYE_OPTION:
	    if(!wizp(tmp=info[TI_VIEWER]))
		return 0;
	
	    return tmp->query_eye_option(par[0]);
	
	default:
	    return ::desc_condition(name, info, par);
    }
}

protected mixed desc_text(string name, mixed info, mixed* par)
{
    mixed tmp;
    
    switch(name)
    {
	case T_ATOM_DAYTIME:
	    tmp = info[TI_ROOM];
	    tmp = (tmp && tmp->local_vclock()) || vclock();
	    return (strlen(tmp=query_light_description(tmp))?tmp:"");
	    
	case T_ATOM_ROOMTYPE:
	    return info[TI_ROOM] && info[TI_ROOM]->query_type(par[0]);

	case T_ATOM_DARKNESS:
	    tmp = (info[TI_ROOM] && info[TI_ROOM]->query_type (LANDSCHAFT));
    	    if (tmp &&
                (tmp & (L_WASSER | L_FLIESSEND | L_UNTERWASSER)) == tmp)
		return
                ({ "Du schwimmst in einer finsteren Brühe.",
                   "Mutig schwimmst Du deines Weges, bei totaler Finsternis.",
                   "Etwas ratlos paddelst Du in der Dunkelheit herum." })
                   [random(3)];
	    else
		return
                ({ "Ein schaurig dunkles Plätzchen.",
                   "Ein grausig finsterer Ort.",
                   "Eine bei Helligkeit viel angenehmere Stelle." })
                   [random(3)];
	case T_ATOM_SHORT_DESC:
	    return (info[TI_OBJECT]||info[TI_ITEM]||info[TI_ROOM]||this_object())->query_short()||"";

	default:
	    return ::desc_text(name, info, par);
    }
}

protected mixed desc_filter(string name, mixed info, mixed orig, mixed* par)
{
    switch(name)
    {
	case T_ATOM_CAPITALIZE:
	    return capitalize(orig);
	
	case T_ATOM_UPPERCASE:
	    return upper_case(orig);
	    
	case T_ATOM_LOWERCASE:
	    return lower_case(orig);

	case T_ATOM_DEBUG:
	    if(!wizp(info[TI_VIEWER]))
		return "";
	
	    orig = info[TI_VIEWER]->colour_msg(MT_DEBUG, MA_UNKNOWN, orig);
	    return sprintf(VT_SAVE_COL "%s" VT_RESTORE_COL "%s",
		orig[<1] == '\n' ? orig[..<2] : orig,
                orig[<1] == '\n' ? "\n" : "");
		
	default:
	    return ::desc_filter(name, info, orig, par);
    }
}

protected int desc_number(string name, mixed info, mixed* par)
{
    mixed tmp;
    
    switch(name)
    {
	case TN_VCLOCK:
	    tmp = info[TI_ROOM];
	    return (tmp && tmp->local_vclock()) || vclock();
	    
	case TN_CLOCK:
	    return clock();
	    
	case TN_STAT_STR:
	    return info[TI_VIEWER] && info[TI_VIEWER]->query_stat(STAT_STR);
	    
	case TN_STAT_INT:
	    return info[TI_VIEWER] && info[TI_VIEWER]->query_stat(STAT_INT);

	case TN_STAT_CON:
	    return info[TI_VIEWER] && info[TI_VIEWER]->query_stat(STAT_CON);

	case TN_STAT_DEX:
	    return info[TI_VIEWER] && info[TI_VIEWER]->query_stat(STAT_DEX);
	
	case TN_LEVEL:
	    return info[TI_VIEWER] && info[TI_VIEWER]->query_level();
	
	case TN_KOERPERGROESSE:
	    return info[TI_VIEWER] && info[TI_VIEWER]->query_koerpergroesse();
	    
	case TN_ALIGN:
	    return info[TI_VIEWER] && info[TI_VIEWER]->query_align();
	
	case TN_STAYDUR:
	    if(!info[TI_ROOM] || !info[TI_VIEWER])
		return 0;
	    tmp = info[TI_ROOM]->query_temp_traces();
	    if(!member(tmp,info[TI_VIEWER]))
		return 0;
	    return time()-tmp[info[TI_VIEWER], TRTT_TIME];
	
	default:
	    return ::desc_number(name, info, par);
    }
}

protected int desc_compile(string name, mixed res, mixed* par)
{
    switch(name)
    {
	case T_ATOM_PSEUDO:
	    if(!stringp(par[0]))
		raise_error("Kein String bei T_PSEUDO angegeben.\n");
	    res = string_parser(par[0], 0, 1);
	    if(pointerp(res))
		res = ({#',,
		    ({#'||=, 'tp,   ({#'[, 'info, TI_VIEWER }) }),
		    ({#'||=, 'item, ({#'[, 'info, TI_ITEM   }) }),
		    ({#'||=, 'room, ({#'[, 'info, TI_ROOM   }) }),
		    res});
	    return T_ATOM_TEXT;
	    
	default:
	    return ::desc_compile(name, &res, par);
    }
}

    
mapping get_desc_info_mapping(object viewer)
{
    mapping info = ([
	TI_VIEWER: viewer,
	//TI_DARK: viewer && !viewer->can_see(this_object()),
#if  __VERSION__ > "3.3.500"
	// Closure, welche das Ergebnis in einem Cache (i) speichert.
	TI_DARK: viewer && function int () : int i
		{
		    return viewer &&
			(i ||= viewer->can_see(this_object())?1:-1)==-1;
		},
#else
	// Das gleiche nur als Lambda fuer vorsintflutliche Driver:
	TI_DARK: viewer && lambda(0,
		({#'&&, viewer, ({#'==, ({#'||=,({#'[,'({0}),0}),
		({#'?, ({#'call_other, viewer, "can_see", this_object()}),
		1, -1})}), 1})})),
#endif
    ]);
    
    if(!this_object()->query_room())
    {
	m_add(info, TI_ITEM, this_object());
	if(environment())
	    m_add(info, TI_ROOM, all_environment(this_object())[<1]);
    }
    else
        m_add(info, TI_ROOM, this_object());

    return info;
}
