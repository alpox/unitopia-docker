// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/i/player/colours.c
// Description:	Zum Einfaerben und Hervorhebungen von bestimmten Meldungen
// Author:	Freaky (21.07.1999)
// Modified by:	Freaky (30.03.2000) vieles nach /i/tools/colours ausgelagert

#pragma save_types
#pragma strong_types

#include <message.h>
#include <colours.h>
#include <level.h>
#include <term.h>
#include <strings.h>
#include <simul_efuns.h>

#include "player.h"

protected virtual inherit "/i/tools/colours";

// Prototypes
varargs void notify_message(string msg, int type);
static int uses_webmud();

// Das Mapping mit dem Bitfeldern
private mapping colours;

// Das Mapping mit den Ansi-Steuercodes. Dieses Mapping muss jedesmal neu
// aus dem Mapping colours erzeugt werden.
private static mapping ansi_colours;

private int show_colour_definitions()
{
    string *cols;
    int i;
    mapping act_names=ACT_NAMES;
    if(wizp(this_object())) act_names+=ACT_WIZ_NAMES;
    
    if (colours[ACT_CONFIG,0] & CO_CONFIG_OFF)
	notify_message("Farbeinstellungen: (AUSGESCHALTET)\n");
    else
	notify_message("Farbeinstellungen:\n");
    cols = sort_array(m_indices(act_names),#'<);
    for (i = sizeof(cols); i--; )
	notify_message(sprintf("%-18s  %s\n",cols[i] + ":",
		colour_to_string(colours[act_names[cols[i]],COL_ON_COLOR], colours[act_names[cols[i]],COL_ON_ECOLOR], colours[act_names[cols[i]],COL_BEEP_TYPE])));
    return 1;
}

#if 0
void debug_colour(int colour)
{
    int val, i;

    printf("Colour: %10d\n",colour);
    printf("Colour: ");
    val = 1;
    for (i = 0; i < 32; i++)
    {
	if (i == 3 || i == 12 || i == 15 || i == 16 || i == 19 || i == 20)
	    printf(" ");
	if (colour & val)
	    printf("1");
	else
	    printf("0");
	val *= 2;
    }
    printf("\n");

    printf("FG: %d\n",CO_TO_FG(colour));
    printf("BG: %d\n",CO_TO_BG(colour));
    printf("Indent: %d\n",CO_TO_INDENT(colour));
}
#endif

protected void setup_colours()
{
    if (!colours)
	colours = m_allocate(0, COL_SIZE);
    else if(widthof(colours) < COL_SIZE)
	colours = m_reallocate(colours, COL_SIZE);

    ansi_colours = colours_to_ansi_colours(colours);
}

string colour_msg(int msg_type, int msg_action, string msg)
{
    int typ;
    string indent;

    if (!strlen(msg))
        return msg;

    if (!ansi_colours)
	setup_colours();

    if (msg_type & MT_DEBUG)
        typ = ACT_DEBUG;
#ifdef MT_FAIL
    else if (msg_type & MT_FAIL)
        typ = ACT_FAILMSG;
#endif
    else if (msg_type == MT_NOTIFY)
        typ = ACT_NOTIFY;
    else if((msg_type & MT_FAR) && msg_action == MA_COMM)
        typ = ACT_FARCOMM;
    else
        typ = msg_action;

    if (indent = ansi_colours[typ,ANSI_INDENT])
    {
	string *lines;
	lines = explode(msg,"\n");
	if (lines[<1] == "")
	    msg = indent + implode(lines[0..<2],"\n" + indent) + "\n";
	else
	    msg = indent + implode(lines,"\n" + indent);
    }
    if (ansi_colours[typ,ANSI_ON])
        msg = sprintf("%s%s%s%s%s",
                      (ansi_colours[typ, ANSI_IDLEBEEP] && interactive() && query_idle(this_object()) / 60 >= ansi_colours[typ, ANSI_IDLEBEEP])
                      ? (ansi_colours[typ, ANSI_BEEP_TYPE] && uses_webmud())
                          ? "\e_beep:"+ansi_colours[typ, ANSI_BEEP_TYPE]+"\e\\"
                          : "\a"
                      : "",
                      ansi_colours[typ,ANSI_ON] - "\a",
                      msg[<1] == '\n' ? msg[..<2] : msg,
                      (ansi_colours[typ,ANSI_OFF] || VT_NORM),
                      msg[<1] == '\n' ? "\n" : "");
    return msg;
}

mapping query_colours()
{
    return copy(colours);
}

nomask static void set_colour(int typ, int colour, int ecolour, string beep)
{
    if (colour || ecolour || beep)
    {
	colours[typ,COL_ON_COLOR] = colour;
	colours[typ,COL_ON_ECOLOR] = ecolour;
	colours[typ,COL_BEEP_TYPE] = beep;
    }
    else
	m_delete(colours,typ);
    ansi_colours = colours_to_ansi_colours(colours);
}

private mixed* trigger;

protected mixed* query_color_trigger()
{
    if(trigger)
	return trigger;
	
    return ({0});
}

protected void set_color_trigger(mixed* t)
{
    trigger = t;
}

private int strchr(string str, string chrs, int pos)
{
    int p = min(map(explode(chrs,""),
	(: strstr($2,$1,$3)+1 || __INT_MAX__:), str, pos));

    if(p == __INT_MAX__)
	return strlen(str);
    else
	return p-1;
}

private string convert_word(string str, int pos, int start, string trenner, int casesensitive, int wholeword)
{
    string result = "";
    int len = strlen(str);
    mapping umschreibungen = ([ "ae": "ä", "oe": "ö", "ue": "ü", "ss": "ß", "Ae": "Ä", "AE": "Ä", "Oe": "Ö", "OE": "Ö", "Ue": "Ü", "UE": "Ü", "SS": "ẞ" ]);
    mapping umlaute =    ([ "ä": "([Ää]|AE|Ae|ae)", "ö": "([Öö]|OE|Oe|oe)", "ü": "([Üü]|UE|Ue|ue)", "ß": "([ẞß]|SS|Ss|ss)",
                            "Ä": "([Ää]|AE|Ae|ae)", "Ö": "([Öö]|OE|Oe|oe)", "Ü": "([Üü]|UE|Ue|ue)", "ẞ": "([ẞß]|SS|Ss|ss)", ]);
    mapping umlaute_cs = ([ "ä": "(ä|ae)",    "ö": "(ö|oe)",    "ü": "(ü|ue)",    "ß": "(ß|ss)",
                            "Ä": "(Ä|A[Ee])", "Ö": "(Ö|O[Ee])", "Ü": "(Ü|U[Ee])", "ẞ": "(ẞ|SS)", ]);
    string re_umschreibungen = implode(m_indices(umschreibungen), "|");

    while(pos < sizeof(str))
    {
	int p = strchr(str, "[]{}()"+trenner, pos);
	int escapes = p - sizeof(trim(str[0..p-1], TRIM_RIGHT, "\\"));
	string part;
	
	if(escapes&1)
	    p--;

	if(start && str[pos]=='*')
	    result += "(\n|^)";
	
	part = escape_string(str[pos..p-1], ESCAPE_WILDCARD);
	part = regreplace(part, re_umschreibungen, function string(string sub) { return umschreibungen[sub]; }, 1);
	if(!casesensitive)
	{
	    part = regreplace(part, "[A-Za-z]+",
		function string(string sub)
		{
		    return "["+implode(map(transpose_array(({explode(lower_case(sub),""),explode(upper_case(sub),"")})),#'implode,""),"][")+"]";
		},1);
	    part = regreplace(part, "[ÄÖÜẞäöüß]",
		function string(string sub)
		{
		    return umlaute[sub];
		},1);
	}
	else
	    part = regreplace(part, "[ÄÖÜẞäöüß]",
		function string(string sub)
		{
		    return umlaute_cs[sub];
		},1);
	
	result += regreplace(part, "\\.\\*", wholeword?"[A-Za-z0-9_-]*":"[^\n]*", 1),
	
	pos = p;
	
	if(p >= len || member(trenner, str[p])>=0)
	    return result;
	
	switch(str[p])
	{
	    case '\\':
		if(p+1 == len)
		{
		    pos++;
		    return result + "\\";
		}
		
		if(member("[]",str[p+1])>=0)
		    result += str[p..p+1];
		else
		    result += str[p+1..p+1];
		pos+=2;
		break;
	    
	    case ']':
	    case '{':
	    case '}':
	    case '(':
	    case ')':
		pos = -1;
		return 0;
	    
	    case '[':
		while(1)
		{
		    p = strstr(str, "]", p+1);
		    if(p<0)
		    {
			pos = -1;
		        return 0;
		    }
		
		    escapes = p - sizeof(trim(str[0..p-1], TRIM_RIGHT, "\\"));
		    if(!(escapes&1))
			break;
		}
		result += str[pos..p];
		pos = p+1;
		break;
	}
    }
    
    return result;
}

protected string convert_glob(string str, int casesensitive)
{
    int pos, len;
    string result = "";
    
    len = sizeof(str);
    
    if(!len)
	return "";
    
    while(1)
    {
	result += convert_word(str, &pos, 1, "{(", casesensitive, 0);
	if(pos < 0)
	    return 0;
	    
	if(pos < len)
	{
	    int start = !sizeof(result);
	    int wholeword = str[pos] == '(';
	    
	    if(wholeword)
		result += "\\<";
	    result += "(";
	    pos++;
	    
	    if(pos >= len)
		return 0;
	    
	    while(1)
	    {
	    	result += convert_word(str, &pos, start, wholeword?",)":",}", casesensitive, wholeword);
	    
		if(pos < 0 || pos >= len)
		    return 0;
		
		if(str[pos] == (wholeword?')':'}'))
		    break;
		
		result += "|";
		pos++;
	    }
	
	    result += ")";
	    if(wholeword)
		result += "\\>";
	    pos++;
	}
	else
	    break;
    }
    
    if(catch(regmatch("", result);reserve 1024))
	return 0;
    
    return result;
}

protected string process_trigger(string msg)
{
#if __VERSION__ >= "3.3.271"
    int num;
    
    if(!sizeof(msg))
	return msg;

    if(!trigger || !trigger[0])
	return msg;

    
    foreach(mixed t: reverse(trigger))
	if(pointerp(t) && t[TRIGGER_REGEXP])
	{
	    num++;
	    msg = regreplace(msg, t[TRIGGER_REGEXP],
		function string(string sub)
		{
		    string beg;
		    if(!sizeof(sub))
			return sub;
			
		    if(sub[0]=='\n')
		    {
			beg = "\n";
			sub = sub[1..<1];
		    }
		    else
			beg = "";
		    
		    return sprintf("%s%s%s%s%s%s%s",
		        beg,
			VT_SAVE_COL_ID(num),
			(((t[TRIGGER_OPTIONS]&TO_BELL) || (t[TRIGGER_COLOR]&CO_BEEP)) && interactive() && query_idle(this_object()) / 60 >= COE_TO_IDLEBEEP(t[TRIGGER_ECOLOR]))
			    ? (uses_webmud() && sizeof(t)>TRIGGER_BEEP_TYPE && t[TRIGGER_BEEP_TYPE])
			      ? "\e_beep:"+t[TRIGGER_BEEP_TYPE]+"\e\\"
			      : "\a"
			    : "",
			((t[TRIGGER_OPTIONS]&TO_KEEPCOLOR)?"":VT_NORM),
			((t[TRIGGER_ANSI]||"")-"\a"),
			sub,
			VT_RESTORE_COL_ID(num));
		}, 1);
	}

#endif
    return msg;
}

protected string process_color_savings(string msg)
{
#if __VERSION__ >= "3.3.271"
    // VT_SAVE/RESTORE_COL verarbeiten:
    msg = regreplace(msg, VT_ESC "\\[(![RS]|![0-9]*;[RS]|[^!A-Za-z]*m)",
	function string(string sub) : string* stack = ({ VT_NORM });
	                              string* stack_ids = ({ "" })
	{
	
	    if(!strstr(sub, VT_ESC "[!"))
	    {
		if(sub[<1]=='S')
		{
		    stack += ({ stack[<1] });
		    stack_ids += ({ sub[3..<2] });
		    return "";
		}
		else
		{
		    string cid = sub[3..<2];
		    int cidpos = rmember(stack_ids, cid);
		    
		    if(cidpos<0)
			return "";
		    else if(cidpos == sizeof(stack_ids)-1)
		    {
			stack = stack[0..<2];
			stack_ids = stack_ids[0..<2];
			return stack[<1];
		    }
		    else
		    {
			stack = arr_delete(stack, cidpos+1);
			stack_ids = arr_delete(stack_ids, cidpos);
			return "";
		    }
		}
	    }
	    else if(sub == VT_NORM)
	        stack[<1] = VT_NORM;
	    else
		stack[<1] += sub;
	
	    return sub;
	}, 1);
#else
    msg = regreplace(msg, VT_ESC "\\[(![RS]|![0-9]*;[RS])", "", 1);
#endif

    return msg;
}

protected void recompile_trigger()
{
    foreach (mixed t: trigger)
        if (pointerp(t))
        {
            string re = convert_glob(t[TRIGGER_TEXT], t[TRIGGER_OPTIONS]&TO_CASESENSITIVE);
            if(re)
                t[TRIGGER_REGEXP] = re;
        }
}
