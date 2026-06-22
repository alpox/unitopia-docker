// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/obj/bauarbeiter.c
// Description:
// Modified:	Offler 14.5.98 - Name, Datum und Kommentar eingefuegt.

#include <invis.h>
#include <level.h>
#include <move.h>
#include <room.h>

inherit "/i/install";
inherit "/i/item";

#define S_TIME		"time"
#define S_WIZ		"wiz"
#define S_TYP		"typ"
#define S_PLINFO	"plinfo"
#define S_WIZINFO	"wizinfo"
#define S_FLAG		"flag"

#define ST_ROT		1
#define ST_GRUEN	2

#define SF_EXIT_VIEW	1 // Nur temporaer

mapping sperren;

#define DIR2ADJ(x) ((["norden":	"noerdlich",\
		  "nordosten":	"nordoestlich",\
		  "osten":	"oestlich",\
		  "suedosten":	"suedoestlich",\
		  "sueden":	"suedlich",\
		  "suedwesten":	"suedwestlich",\
		  "westen":	"westlich",\
		  "nordwesten":	"nordwestlich",\
		])[(x)])
#define ZAHLEN ({"keine","eine","zwei","drei","vier","fuenf","sechs","sieben","acht","neun","zehn","elf","zwoelf"})

private string* ausgaenge()
{
    return sort_array(m_indices(sperren),#'>);
}

private mixed* sm_values(mapping data)
{
    return map(sort_array(m_indices(data),#'>), data);
}

string schild_zeile(string dir, mapping data)
{
    if (!data)
        return "Frisch Gestrichen!";

    return sprintf("%s am %s von %s %s",
	    capitalize(dir), data[S_TIME]?timestr(data[S_TIME]):"irgendwann",
	    data[S_WIZ] || "irgendwem",
	    data[S_WIZINFO]?"gesperrt:\n"+wrap(data[S_WIZINFO], 75,4)+
			    (sizeof(data[S_PLINFO])?"\n"+wrap(data[S_PLINFO],75,4):"")
			   :"kommentarlos gesperrt.\n");
}

string query_long(object who)
{
    string text;
    mapping ausgaenge = ([:1]);
    
    if(!sizeof(sperren))
	return "Eine nutzlose Schranke.\n";

    text = ::query_short(who)+".";
    
    foreach(string dir: ausgaenge())
    {
	mapping info = sperren[dir];

	if(!member(ausgaenge, info[S_TYP]))
	    m_add(ausgaenge, info[S_TYP], ({}));
	ausgaenge[info[S_TYP]]+=({dir});
    }
	
    if(sizeof(ausgaenge[ST_ROT]))
    {
	if(sizeof(ausgaenge[ST_ROT])==1)
	    text += " Die rot-weiße Schranke versperrt Dir den Weg nach ";
	else
	    text += " Die rot-weißen Schranken versperren Dir die Wege nach ";
	
	text += liste(map(ausgaenge[ST_ROT],#'capitalize))+", damit dort die "
		"Bauarbeiter in Ruhe arbeiten können. Komm doch demnächst "
		"nochmal vorbei, um zu sehen, ob sie etwas Interessantes gebaut "
		"haben.";
    }

    if(sizeof(ausgaenge[ST_GRUEN]))
    {
	if(sizeof(ausgaenge[ST_GRUEN])==1)
	    text += " Die grün-weiße Schranke warnt Dich vor dem Weg nach ";
	else
	    text += " Die grün-weißen Schranken warnen Dich vor den Wegen nach ";
	
	text += liste(map(ausgaenge[ST_GRUEN],#'capitalize))+", denn dort "
	    "können sich Löcher im Raum-Zeit-Gefüge und andere, womöglich "
	    "gefährliche Fehler aufhalten. Du darfst hier gern testen, bitte "
	    "melde dabei alle Fehler. Beachte aber, dass das Pantheon für "
	    "dabei eventuell auftretenden Schaden keinen Ersatz leistet.";
    }

    if(!sizeof(sperren))
        ; // Kein Text...
    else if(sizeof(sperren)==1)
	text += " Ein wichtiges Schild ist daran befestigt.";
    else
	text += " Wichtige Schilder sind an ihnen befestigt.";
    
    return wrap(text);
}

string schilder_read(string rest, string str, mapping vitem, object who)
{
    string text;
    string* nix;
    
    if (!sperren)
        return "Es sind keine Schilder an der Schranke befestigt.\n";
    if (wizp(who))
        return "Auf den Schildern steht:\n"+
            implode(sm_values(map(sperren,#'schild_zeile)),"\n")+"\n";

    text = "";
    nix = ({});
    
    foreach(string dir: ausgaenge())
    {
    	mapping info = sperren[dir];

	if(sizeof(info[S_PLINFO]))
	    text += wrap("Auf "+dem((["name":"schild","gender":"saechlich"]),
			    DIR2ADJ(dir))+" steht:")+
		    wrap(info[S_PLINFO],75,4);
	else
	    nix += ({dir});
    }
    
    if(sizeof(text))
    {
	if(sizeof(nix) == 1)
	    text += wrap("Die krakeligen Buchstaben auf "+
		    dem((["name":"schild","gender":"saechlich"]), DIR2ADJ(nix[0]))+
		    " kannst Du beim besten Willen nicht entziffern.");
	else if(sizeof(nix)>1)
	    text += wrap("Die krakeligen Buchstaben auf den Schildern "
		    "in Richtung "+liste(map(nix,#'capitalize))+
		    " kannst Du beim besten Willen nicht entziffern.");
    }
    else if(sizeof(nix))
	text = wrap("Die krakeligen Buchstaben, die über "+
		((sizeof(nix)==1)?"das gesamte Schild":"allen Schildern")+
        	" verteilt sind, könnte man bestenfalls als kreativ "
		"bezeichnen. Du kannst beim besten Willen keinen Sinn "
		"darin erkennen. Typisch Götter eben.");

    return text;
}

string schilder_long(mapping vitem, object who)
{
    if (!sperren)
        return "Es sind keine Schilder an der Schranke befestigt.\n";

    if (wizp(who) || sizeof(filter(sperren,(:sizeof($2[S_PLINFO]):))))
        return schilder_read(0,0,vitem,who);
	
    if(sizeof(sperren)>1)
	return "Ganz so wichtig sind die Schilder wohl doch nicht.\n";

    return "Ganz so wichtig ist das Schild wohl doch nicht.\n";
}

string sperre_long(mapping vitem, object who)
{
    string dir = vitem["exit"];
    mapping info = sperren[dir];
    
    if(info[S_TYP] == ST_ROT)
	return sprintf("%=-75s\n",
	    "Eine rot-weiße Schranke, die Dir den Weg nach "+capitalize(vitem["exit"])+
    	    " versperrt, damit die Bauarbeiter dort in Ruhe arbeiten können. "
	    "Komm doch demnächst nochmal vorbei, um zu sehen, ob sie "
	    "etwas Interessantes gebaut haben. Ein wichtiges Schild ist "
	    "daran befestigt.");
    else if(info[S_TYP] == ST_GRUEN)
	return sprintf("%=-75s\n",
	    "Eine grün-weiße Schranke, die vor dem Weg nach "+capitalize(vitem["exit"])+
    	    " warnt. Dort können sich Löcher im Raum-Zeit-Gefüge und andere, "
	    "womöglich gefährliche Fehler aufhalten. Du darfst hier gern "
	    "testen, bitte melde dabei alle Fehler. Beachte aber, dass das Pantheon "
	    "für dabei eventuell auftretenden Schaden keinen Ersatz leistet. "
	    "Ein wichtiges Schild ist daran befestigt.");
}

string schild_read(string rest, string str, mapping vitem, object who)
{
    string dir = vitem["exit"];
    mapping info = sperren[dir];
    
    if(!dir || !info)
        return "Das Schild ist absolut leer.\n";

    if (wizp(who))
        return "Auf dem Schild steht:\n"+
	    schild_zeile(dir, info);

    if(sizeof(info[S_PLINFO]))
	return "Auf dem Schild steht:\n"+
	    wrap(info[S_PLINFO]);
    
    return wrap("Die krakeligen Buchstaben, die über das gesamte Schild "
       "verteilt sind, könnte man bestenfalls als kreativ bezeichnen. "
       "Du kannst beim besten Willen keinen Sinn darin erkennen. Typisch "
       "Götter eben.");
}

string schild_long(mapping vitem, object who)
{
    string dir = vitem["exit"];
    mapping info = sperren[dir];

    if (wizp(who) || sizeof(info[S_PLINFO]))
        return schild_read(0,0,vitem,who);
    return "Ganz so wichtig ist das Schild wohl doch nicht.\n";
}


void create()
{
  set_name("schranke");
  set_gender("weiblich");
  set_id(({"schranke", "sperre","Schranke","schranken","sperren","root # sperre"}));
  set_adjektiv(({"rotweiß"}));
  seteuid(getuid());
  add_v_item(([
    "name":"schilder",
    "gender":"saechlich",
    "adjektiv":({"wichtig"}),
    "long":#'schilder_long,
    "read":#'schilder_read
  ]));
}

mapping query_block_info()
{
    return sperren;
}

void install_parameter(string str)
{
    int typ, last_pl;
    mapping count = ([:1]);
    string stxt;
    
    if(sizeof(str) && str[0]=='#') // Neue Version
	sperren = restore_value(str);
    else
    {
	mixed * info;
	
	sperren = ([:1]);
	
	if (strstr(str,";") == -1)	// Alte version (Leerzeichen als Trenner)
	    info = map(explode(str," ")-({""}),#'explode,";");
	else
	    info = map(explode(str,"|")-({""}),#'explode,";");
	
	foreach(mixed * sperre: info)
	    m_add(sperren, sperre[0],
		([
		    S_TIME:	sizeof(sperre)>1 && to_int(sperre[1]),
		    S_WIZ:	sizeof(sperre)>2 && sperre[2],
		    S_WIZINFO:	sizeof(sperre)>3 && sperre[3],
		    S_TYP:	ST_ROT,
		]));
    }

    environment()->add_controller("forbidden_move_out",this_object());

    foreach(string dir, mapping info: sperren)
	if(info[S_TYP] == ST_ROT)
	{
	    environment()->lock_exit(dir);
	    if(environment()->query_exit_flag(dir,EXIT_VIEW))
	    {
		info[S_FLAG] |= SF_EXIT_VIEW;
		environment()->add_exit_flag(dir, EXIT_ATOM_NOT | EXIT_ATOM_VIEW);
	    }
	    else
		info[S_FLAG] &= ~SF_EXIT_VIEW;
	}

    if(sizeof(sperren)>1)	
    {
	delete_id(({"schranke","sperre"}));
	set_name("schranken");
	set_plural(1);
    }
    
    foreach(string dir, mapping info: sperren)
    {
	if(!info[S_TYP])
	    info[S_TYP] = ST_ROT;
	
	typ |= (1<<info[S_TYP]);
	
	add_v_item(([
	    "name":	"schranke",
	    "gender":	"weiblich",
	    "id":	({"schranke","sperre",dir}),
	    "adjektiv":	((info[S_TYP]==ST_ROT)?({"rot-weiß",DIR2ADJ(dir)}):
			 (info[S_TYP]==ST_GRUEN)?({"grün-weiß",DIR2ADJ(dir)}):
			 ({DIR2ADJ(dir)}))-({0}),
	    "exit":	dir,
	    "noowner":	1,
	    "long":	#'sperre_long,
	]));
	add_v_item(([
	    "name":	"schild",
	    "gender":	"saechlich",
	    "exit":	dir,
	    "noowner":	1,
	    "long":	#'schild_long,
	    "read":	#'schild_read,
	]), ({dir}));
	add_v_item(([
	    "name":	"schild",
	    "gender":	"saechlich",
	    "adjektiv":	((info[S_TYP]==ST_ROT)?({"rot-weiß",DIR2ADJ(dir)}):
			 (info[S_TYP]==ST_GRUEN)?({"grün-weiß",DIR2ADJ(dir)}):
			 ({DIR2ADJ(dir)}))-({0}),
	    "exit":	dir,
	    "noowner":	1,
	    "long":	#'schild_long,
	    "read":	#'schild_read,
	]));
	
	count[info[S_TYP]]++;
    }
    
    if(typ == (1<<ST_ROT))
	set_adjektiv("rot-weiß");
    else if(typ == (1<<ST_GRUEN))
	set_adjektiv("grün-weiß");
    else
	set_adjektiv(0);

    if(count[ST_ROT])
    {
	if(count[ST_ROT]<13)
	    stxt = ZAHLEN[count[ST_ROT]];
	else
	    stxt = to_string(count[ST_ROT]);
	
	stxt+=" rot-weiße ";
	last_pl = count[ST_ROT]>1;
    }
    
    if(count[ST_GRUEN])
    {
	if(stxt)
	    stxt +="und ";
	else
	    stxt = "";
	
	if(count[ST_GRUEN]<13)
	    stxt += ZAHLEN[count[ST_GRUEN]];
	else
	    stxt += to_string(count[ST_GRUEN]);
	
	stxt+=" grün-weiße ";
	last_pl = count[ST_GRUEN]>1;
    }
    
    if(stxt)
	set_short(capitalize(stxt)+"Schranke"+(last_pl?"n":""));
    else
	set_short("Eine nutzlose Schranke");
}

void init()
{
    environment()->add_controller("forbidden_move_out",this_object());
    if (sperren)
	foreach(string dir, mapping info: sperren)
	    if(info[S_TYP] == ST_ROT)
	    {
		environment()->lock_exit(dir);
		if(environment()->query_exit_flag(dir,EXIT_VIEW))
		{
		    info[S_FLAG] |= SF_EXIT_VIEW;
		    environment()->add_exit_flag(dir, EXIT_ATOM_NOT | EXIT_ATOM_VIEW);
		}
	    }
}

int remove()
{
    foreach(string dir, mapping info: sperren)
        if(info[S_TYP] == ST_ROT)
	{
	    environment()->unlock_exit(dir);
	    
	    if(info[S_FLAG] & SF_EXIT_VIEW)
		environment()->add_exit_flag(dir, EXIT_VIEW);
	}

    install::remove();
}

int query_enable_cleanup() { return 1; }

string query_short(object who)
{
    if(!wizp(who))
	return ::query_short(who);
    else
	return ::query_short(who) +
	    (sizeof(sperren)?" ("+liste(map(ausgaenge(),#'capitalize))+")":"");
}

int query_schwimmfaehig()
// Schranken sollten im Meer nicht untergehen
{
    return 1;
}

int forbidden_move_out(mapping mv_infos)
{
    object wer = mv_infos[MOVE_OBJECT];
    string dir = mv_infos[MOVE_DIRECTION] 
              && convert_umlaute(lower_case(mv_infos[MOVE_DIRECTION]));
    
    foreach(string sdir, mapping info: sperren)
    {
        sdir = convert_umlaute(sdir);
        if(info[S_TYP] == ST_ROT && (sdir==dir
            || sdir == convert_umlaute(environment()->query_exit_command(
                        mv_infos[MOVE_NEW_ROOM], 1) || "")))
        {
            wer->set_not_moved_reason("Diese Richtung ist gesperrt.\n");
            return (playerp(wer) && !wizp(wer)) ||
		(sdir==dir && !IS_INVIS(wer));
        }
    }
    return 0;
}
