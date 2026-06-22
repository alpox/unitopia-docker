// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/obj/enzyclopedia.c
// Description: Enzyclopedia UNItopia
// Author:	Freaky (23.12.93)

#pragma strict_types

inherit "/i/install";
inherit "/i/item";

#include <more.h>
#include <level.h>
#include <config.h>
#include <commands.h>
#include <notify_fail.h>
#include <message.h>
#include <apps.h>
#undef HELP_TOOL
#include <help_tool.h>

#define MENUE_ID "Enzy-Menue"
#define MORE_ID "Enzy-More"

#define FAIL(x) { notify_fail(x); return 0; }
#define LINE "-------------------------------------------------------------------------------\n"
#define START_MORE ({string})this_player()->more(menue[M_TEXT],\
  menue[M_STATUS_ZEILE], LAST_STATUS[S_LINE],\
  (menue[M_MAX_MENUE]&&pointerp(menue[M_TEXT]))?M_LINE_NUMBERS|M_DO_NOT_END|M_FRAME:\
  menue[M_MAX_MENUE]?M_DO_NOT_END|M_FRAME:0, MENUE_ID)

#define LAST_STATUS menue[M_STATUS][<1]

mixed menue;
int yet_started;
int enzy_art;
int original;
mapping gruppen = ([]); // Wildcard-Angabe: FLags (Bit 0: 0: Haben will, 1: Nicht haben will)
static string *gruppen_namen = map(({string*})HELP_TOOL->query_lfun_gruppen(),#'lower_case);

static mapping patterns = ([
    "controller":"(^(notify_|forbidden_|allowed_|concerned_))",
    ]);

string query_long (object who) {
   if (!wizp(who))
      return "Dies ist "+der()+".\n"+
        ((enzy_art==2)?"In großen leuchtenden Lettern liest Du folgendes:\n\n"
          "                Keine Panik!\n\n":"");
   return
      "Dies ist "+der()+".\n"
      +((enzy_art==2)?"In großen leuchtenden Lettern liest Du folgendes:\n\n"
          "                Keine Panik!\n\n\n":"\n")+
      "Befehle:\n"
      +(enzy_art==1
      ?"    menü, enzy                Menü, in welchem so ziemlich alles, was zum\n"
      :"    menü, panik               Menü, in welchem so ziemlich alles, was zum\n")+
      "                              Programmieren benötigt wird, zu finden ist.\n"
      "    dek <text>                listet alle der Enzy bekannten Deklarationen von\n"
      "                              Funktionen auf, deren Name dem angegebenen Text\n"
      "                              (welcher ein regulärer Ausdruck sein kann)\n"
      "                              enthalten.\n"
      "                              \"dek\" ist daher prima dazu geeignet,\n"
      "                              Funktionen zu finden, deren Namen man nicht\n"
      "                              (mehr) genau kennt.\n"
      "    dek! <text>               Wie 'dek' für alle Gruppen.\n"
      "    ? funktionsname           Beschreibung einer bestimmten Funktion.\n"
      "    ?! funktionsname          Wie '?' für alle Gruppen.\n"
      "    bsp? beispiel             Beschreibung für ein bestimmten Beispielnamen.\n"
      "    bsp?! beispiel            Wie 'bsp?' für alle Gruppen.\n"
      "    bsp <text>                Wie 'dek' nur für Beispiele.\n"
      "    bsp! <text>               Wie 'bsp' für alle Gruppen.\n"
      "    efunoriginal an | aus     EFun-Doku immer im Original auf Englisch anzeigen\n"
      "                              anstelle der deutschen Übersetzung.\n"
      "    ? funktionsname .         Original-Flag für einzelne Abfrage umkehren.\n"
      "    listgroup <Gruppe>        Fügt diesen Gruppennamen zur Liste der Gruppen\n"
      "                              hinzu, deren Funktionen angezeigt werden sollen.\n"
      "                              * ist als Platzhälter für ganze Gruppennamen\n"
      "                              zugelassen.\n"
      "    notlistgroup <Grupp>      Fügt diese Gruppe zur Liste der Gruppen hinzu,\n"
      "                              deren Funktionen nicht angezeigt werden sollen.\n"
      "    dekgrp <Gruppe> [<Text>]  listet alle Funktionen der Gruppe auf, welche\n"
      "                              dem angegebenen regulären Ausdrueck (sofern\n"
      "                              überhaupt angegeben) entsprechen.\n"
      "    enzygrp efun | lfun       listet alle lfun oder efun Gruppen auf.\n"
      "    deksource <File> [<Txt>]  listet alle Funktionen der Datei auf, welche\n"
      "                              dem angegebenen regulären Ausdrueck (sofern\n"
      "                              überhaupt angegeben) entsprechen. Die Funktion\n"
      "                              ist vom aktuellen Verzeichnis abhaenig.\n"
      +(enzy_art==1?
      "    verwandle enzy            verwandelt die Enzy in das Buch\n"
      "                              Per Anhalter durch "+MUD_NAME+".\n" :
      "    verwandle anhalter        verwandelt den Anhalter in die\n"
      "                              Enzyclopaedia "+MUD_NAME+".\n")+
      "\n"
      "Als regulärer Ausdruck ist zusätzlich zu den normalen Konstrukten\n"
      "noch {controller} erlaubt, welches alle Controller auflistet.\n";
}

void setup_enzy ()
{
    switch (enzy_art) {
        case 1:
            set_name("Enzyclopedia "+MUD_NAME);
            set_gender("weiblich");
            set_id(({"enzyclopedia","enzy",
                "enzyclopedia "+lower_case(MUD_NAME),
	        "buch"}));
	    break;
        case 2:
            set_name("Buch \"Per Anhalter durch "+MUD_NAME+"\"");
            set_gender("saechlich");
            set_id(({"enzyclopedia","enzy",
                "anhalter","buch"}));
	    break;
    }
}

int check_group(string gruppe, mixed *gruppen_regexps,string *gruppen_mitgl)
{
    gruppe = lower_case(gruppe);
    foreach(mixed* re: gruppen_regexps)
	if(sizeof(regexp(({gruppe}),re[0])))
	    return !(re[1]&1);
    if(strstr(gruppe,":")<0 && !({string})GROUP_MASTER->adjust_group_case(gruppe))
	return 1;
    gruppe = gruppe+":";
    foreach(string grp:gruppen_mitgl)
	if(!strstr(gruppe, grp) || !strstr(grp, gruppe))
	   return 1;
    return 0;
}

void convert_gruppen()
{
    mixed *gruppen_regexps;
    string *gruppen_m1,*gruppen_m2;
    string *gruppen_mitgl;

    // TODO: Diesen Schwachsinn abschaffen    
    if(get_eval_cost()<400000)
    {
        call_out("convert_gruppen", 2);
        return;
    }

    // Braucht moeglicherweise sehr viel Evals, daher jetzt
    // schon berechnen.
    gruppen_mitgl = ({string*}) GROUP_MASTER->query_all_memberships_of(
          environment()->query_real_name());

    if(get_eval_cost()<400000)
    {
	call_out("convert_gruppen", 2);
	return;
    }
    
    // Array aus Paaren ({ regexp, rang })
    // (Rang ist fuer die Sortierreihenfolge, je mehr Wildcards desto weiter
    // vorn, nolistgroups vor listgroups bei gleichen Wildcards.)
    gruppen_regexps = sort_array(
	map(transpose_array(unmkmapping(gruppen)), 
	(:
	    ({ "^"+regreplace(lower_case($1[0]), "\\*", ".*", 1)+"$",
	       sizeof(explode($1[0],"*")-({"",":"}))*2+$1[1]
	    })
	:)), (: $1[1]<$2[1] :));

    // Alle moeglichen Gruppen
    gruppen_m1 = ({string*})HELP_TOOL->query_lfun_gruppen();
    // Direkt Mitglied, sofort uebernehmen
    gruppen_m2 = gruppen_m1 & gruppen_mitgl;
    gruppen_mitgl = map(gruppen_mitgl, (: lower_case($1)+":" :));
    // Den Rest durch check_group jagen
    gruppen_namen = map(gruppen_m2 + filter(gruppen_m1 - gruppen_m2,
        #'check_group, gruppen_regexps,gruppen_mitgl),#'lower_case);
}

int show_group(string gruppe)
{
    return member(gruppen_namen, lower_case(gruppe))>=0;
}

private int handle_group_action(string str, int flag)
{
    if(!strlen(str=trim(str||"")))
    {
	string *list = sort_array(m_indices(filter(gruppen, (: $2==$3 :), flag)),#'>);
	if(!sizeof(list))
	    send_message_to(this_player(), MT_NOTIFY, MA_UNKNOWN, "Keine Gruppen eingetragen.\n");
	else
	    this_player()->more(({"Diese Gruppen werden "+(flag?"nicht":"zusätzlich")+" angezeigt:",""})+list,
		0, 0, M_AUTO_END);
	return 1;
    }
    if(str[0]=='-')
    {
	str = trim(str[1..<1]);
	if(!strlen(str))
	    return notify_fail(query_verb()+" [+|-] <Gruppe>\n",FAIL_WRONG_ARG);
	if(!member(gruppen,lower_case(str)) || gruppen[lower_case(str)]!=flag)
	    return notify_fail(wrap("Die Gruppe '"+str+"' ist nicht eingetragen."), FAIL_INTERNAL);
	gruppen-=([lower_case(str)]);
    }
    else
    {
	if(str[0]=='+')
	    str=trim(str[1..<1]);
	if(!strlen(str))
	    return notify_fail(query_verb()+" [+|-] <Gruppe>\n",FAIL_WRONG_ARG);
	if(!sizeof(regexp(({str}),"^(([A-Za-z0-9]+|\\*):)*([A-Za-z0-9]+|\\*)$")))
	    return notify_fail(wrap("Diese Gruppe enthält ungültige Zeichen "
		"oder inkorrekte Wildcards."));
	gruppen[lower_case(str)] = flag;
    }
    convert_gruppen();
    send_message_to(this_player(), MT_NOTIFY, MA_UNKNOWN, "Ok.\n");
    return 1;
}

int listgroup(string str)
{
    return handle_group_action(str,0);
}

int notlistgroup(string str)
{
    return handle_group_action(str,1);
}

void create() {
    enzy_art = 1;
    setup_enzy();
    set_eigen(1);
    seteuid(getuid());
    menue=({mixed})HELP_TOOL->get_menu(0,0,0);
}

void init() {
    if (this_player()==environment()) {

        if(sizeof(filter(all_inventory(this_player()),
            (: load_name($1) == "/obj/enzyclopedia" :))) > 1)
        {
            send_message_to(this_player(), MT_NOTIFY, MA_UNKNOWN,
                wrap(Dein(this_object(), 0, this_player())+" hüpft beleidigt "
                    "auf und davon, als "+er()+" bemerkt, dass du bereits ein "
                    "solches Exemplar besitzt."));
            this_object()->remove();
        }

	add_action("hilfe","?",AA_NOSPACE);
	add_action("hilfe","?!",AA_NOSPACE);
	add_action("hilfe_bsp","bsp?",AA_NOSPACE);
	add_action("hilfe_bsp","bsp?!",AA_NOSPACE);
	// add_action("hilfe","p");
	add_action("beispiele","bsp");
	add_action("beispiele","bsp!");
	add_action("deklara","deklaration",-3);
	add_action("deklara","dek!");
	add_action("deklara","deklaration!");
	add_action("enzygrp","enzygrp");
        add_action("dekgrp","dekgrp");
	add_action("deksource","deksource");
	add_action("menu","menü");
	add_action("menu","enzy");
	add_action("menu","anhalter");
	add_action("menu","panik");
	add_action("verwandle","verwand",1);
	add_action("listgroup","listgroup");
	add_action("listgroup","listgrp");
	add_action("listgroup","lstgrp");
	add_action("listgroup","lgrp");
	add_action("notlistgroup","notlistgroup");
	add_action("notlistgroup","notlistgrp");
	add_action("notlistgroup","nlstgrp");
	add_action("notlistgroup","nlgrp");
        add_action("efunoriginal", "efunoriginal");
	}
}

int efunoriginal(string s)
{
    if(s == "an")
    {
        original = 1;
        send_message_to(this_player(), MT_NOTIFY, MA_UNKNOWN,
            "EFun-Doku siehst du jetzt im englischen Original.\n");
        return 1;
    }

    else if(s == "aus")
    {
        original = 0;
        send_message_to(this_player(), MT_NOTIFY, MA_UNKNOWN,
            "EFun-Doku siehst du jetzt in der deutschen Übersetzung.\n"
            "Natürlich nur, solange es eine Übersetzung gibt. :-)\n"
            "Für einzelne Abfragen umkehren kannst du das mit '? funktion .'\n");
        return 1;
    }

    notify_fail("efunoriginal an oder aus?\n");
}

int verwandle (string s)
{
    if (!me (s)) return notify_fail("Was willst Du verwandeln?\n");
    s = "Du verwandelst "+deinen()+" in ";
    enzy_art = 3 - enzy_art;
    setup_enzy();
    s += einen()+".";
    write (wrap (s));
    return 1;
}

mapping query_auto_load()
{
    if (wizp (environment()) || testplayerp (environment()))
        return (["art":enzy_art,"groups":gruppen, "original":original]);
}

void init_arg(mixed arg)
{
    if(intp(arg))
	enzy_art = arg;
    else if(mappingp(arg))
    {
	enzy_art = arg["art"];
	gruppen = arg["groups"];
        original = arg["original"];
	call_out(#'convert_gruppen, 2); // convert_gruppen();
    }
    setup_enzy();
}

int hilfe_bsp(string str)
{
    string bsp,*ret;
    int own_groups = member(query_verb(),'!')<0;
    int flag = original;
    
    if (!str)
	FAIL("Von welchem Beispiel?\n");

    str = trim(str);

    if(str[<1] == '.')
    {
        flag = !flag;
        str = str[..<2];
    }

    str = trim(str);
    if (str[<2..]=="()")
    	str = str[0..<3];

    bsp=({string})HELP_TOOL->query_bsp_texte(own_groups && gruppen_namen, str);
    if(!bsp)
        return 0;

    ret = ({});
    if(bsp)
        ret += ({"Beispiel "+str}) + explode(LINE+bsp+LINE[0..<2],"\n");
    this_player()->more(ret,0,0,M_AUTO_END,MORE_ID);
    return 1;
}

int beispiele(string str)
{
    mixed ret, all="";
    int own_groups = (member(query_verb(),'!')<0);

    str = regreplace(str||"","{[^}]*}",(:patterns[$1[1..<2]]||"":),1);
    ret = ({mapping})HELP_TOOL->query_bsp_deklarationen(
        own_groups && gruppen_namen, str);
    if (sizeof(ret))
    {
        all="Beispiele:\n";
        foreach(string fun: sort_array(m_indices(ret),#'>))
            all+=sprintf("%-20s %=-58s\n", fun, ret[fun]);
    }

    if (!strlen(all))
	FAIL("Eine solches Beispiel kenne ich nicht.\n");
    this_player()->more(explode(all[0..<2],"\n"),0,0,M_AUTO_END,MORE_ID);
    return 1;
}

int hilfe(string str)
{
    string efun,lfun,driver,*ret;
    int own_groups = member(query_verb(),'!')<0;
    int flag = original;
    
    if (!str)
	FAIL("Von welcher Funktion?\n");

    str = trim(str);

    if(str[<1] == '.')
    {
        flag = !flag;
        str = str[..<2];
    }

    str = trim(str);
    if (str[<2..]=="()")
    	str = str[0..<3];

    lfun=({string})HELP_TOOL->query_lfun_texte(own_groups && gruppen_namen, str);
    efun=({string})HELP_TOOL->query_efun_hilfetext(str, flag);
    driver=({string})HELP_TOOL->query_driver_hilfetext(str, flag);
    if(!lfun && !efun && !driver)
        return 0;

    ret = ({});
    if(lfun)
        ret += ({"Lfun "+str}) + explode(LINE+lfun+LINE[0..<2],"\n");
    if(lfun && efun)
        ret += ({""});
    if(efun)
        ret += ({"Efun "+str}) + explode(LINE+efun+LINE[0..<2],"\n");
    if((lfun || efun) && driver)
	ret += ({""});
    if(driver)
	ret += explode(LINE+driver+LINE[0..<2],"\n");
    this_player()->more(ret,0,0,M_AUTO_END,MORE_ID);
    return 1;
}

int deklara(string str)
{
    mixed ret, all;
    int own_groups = (member(query_verb(),'!')<0);

    if (!str)
	FAIL("Von welcher Funktion?\n");

    str = regreplace(str,"{[^}]*}",(:patterns[$1[1..<2]]||"":),1);
    ret=({mapping})HELP_TOOL->query_lfun_deklarationen(
        own_groups && gruppen_namen, str);
    if (!ret)
	return 0;

    all="";
    if (sizeof(ret))
    {
	all+="Lfuns:\n";

	foreach(string fun: sort_array(m_indices(ret),#'>))
	    all+=sprintf("%-20s %=-58s\n", fun, ret[fun]);

	if(strlen(all)==7)
	    all="";
    }

    ret=({string *})HELP_TOOL->query_efuns(str);
    if (sizeof(ret))
    {
	if (strlen(all))
	    all+="\n";
	all+="Efuns:\n";

	foreach(string fun: ret)
	    all+=sprintf("%-20s %=-58s\n", fun,
		({string})HELP_TOOL->query_efun_deklaration(fun));
    }
    
    ret = ({mapping})HELP_TOOL->query_bsp_deklarationen(
        own_groups && gruppen_namen, str);
    if (sizeof(ret))
    {
        if (strlen(all))
            all+="\n";
        all+="Beispiele:\n";
        foreach(string fun: sort_array(m_indices(ret),#'>))
            all+=sprintf("%-20s %=-58s\n", fun, ret[fun]);
    }

    if (!strlen(all))
	FAIL("Eine solche Funktion kenne ich nicht.\n");
    this_player()->more(explode(all[0..<2],"\n"),0,0,M_AUTO_END,MORE_ID);
    return 1;
}

int enzygrp(string str)
{
    string *ret;
    if(!str)
	FAIL("Welcher Gruppentyp (lfun oder efun oder beispiele)?\n");
    if (strstr(lower_case(str),"efun",0) != -1)
        ret=({"EFUN-GRUPPEN:"})+({ string *})HELP_TOOL->query_efun_gruppen();
    else
      if(strstr(lower_case(str),"lfun",0) != -1)
         ret=({"LFUN-GRUPPEN:"})
        +filter(({string *})HELP_TOOL->query_lfun_gruppen(),
        (:!member($2,explode($1,":")[0]):), mkmapping(({string*})GROUP_MASTER->query_maingroups()));
      else if (strstr(lower_case(str),"beispiel",0) != -1)
          ret=({"BEISPIEL-GRUPPEN:"})
                +({ string *})HELP_TOOL->query_bsp_gruppen();
    if (!ret)
       FAIL("Welcher Gruppentyp (lfun oder efun oder beispiele)?\n");
    this_player()->more(ret[0..0] +
        explode(sprintf("%-75#s\n",implode(ret[1..<1],"\n")),"\n")[0..<2],
	0,0,M_AUTO_END,MORE_ID);
    return 1;
}

int deksource(string str)
{
    string file, pat, all;
    mixed ret;

    if (!str)
        FAIL("Von welcher Datei?\n");
    ret = explode(str," ") - ({""});
    file = ret[0];

    if(sizeof(ret)>0)
    {
        pat = implode(ret[1..<1]," ");
        pat = regreplace(pat,"{[^}]*}",(:patterns[$1[1..<2]]||"":),1);
    }

    file = ({string})this_player()->add_path(file);

    all="";
    ret=({mapping})HELP_TOOL->query_lfun_deklarationen(0, pat, file);
    if (mappingp(ret)&& sizeof(ret))
    {
        if (sizeof(ret))
        {
            all+="Funktionen in "+file+":\n"+LINE;

            foreach(string fun: sort_array(m_indices(ret),#'>))
                all+=sprintf("%-20s %=-58s\n", fun, ret[fun]);
        }
    }
    ret=({mapping})HELP_TOOL->query_bsp_deklarationen(0, pat, file);
    if (mappingp(ret)&& sizeof(ret))
    {
        if (sizeof(ret))
        {
            all+="Beispiele in "+file+":\n"+LINE;

            foreach(string fun: sort_array(m_indices(ret),#'>))
                all+=sprintf("%-20s %=-58s\n", fun, ret[fun]);
        }
    }
    if (all == "")
       FAIL("Keine Funktionen oder Beispiele zu dieser Datei gefunden!\n");
    this_player()->more(explode(all[0..<2],"\n"),0,0,M_AUTO_END,MORE_ID);    
    return 1;
} 

int dekgrp(string str)    
{
    string grp, pat, all;
    mixed ret;
    
    if (!str)
        FAIL("Von welcher Gruppe?\n");
    ret = explode(str," ");
    grp = ret[0];

    if(sizeof(ret)>0)
    {
        pat = implode(ret[1..<1]," ");
        pat = regreplace(pat,"{[^}]*}",(:patterns[$1[1..<2]]||"":),1);
    }

    ret=({mapping})HELP_TOOL->query_lfun_deklarationen( ({lower_case(grp)}), pat);
    if (!ret)
	return 0;

    all="";
    if (sizeof(ret)) 
    {
        all+="Lfuns der Gruppe "+grp+":\n"+LINE;

	foreach(string fun: sort_array(m_indices(ret),#'>))
	    all+=sprintf("%-20s %=-58s\n", fun, ret[fun]);
    }

    ret=({string *})HELP_TOOL->query_efun_gruppen_funktionen(grp,pat);
    if (sizeof(ret)) 
    {
	if (strlen(all))
    	    all+="\n";
	all+="Efuns der Gruppe "+grp+":\n"+LINE;
	
	foreach(string fun: sort_array(ret,#'>))
    	    all+=sprintf("%-20s %=-58s\n",fun,
		({string})HELP_TOOL->query_efun_deklaration(fun));
    }
    
    ret=({mapping})HELP_TOOL->query_bsp_deklarationen( ({lower_case(grp)}), pat);
    if (sizeof(ret)) 
    {
        all+="Beispiele der Gruppe "+grp+":\n"+LINE;

        foreach(string fun: sort_array(m_indices(ret),#'>))
            all+=sprintf("%-20s %=-58s\n", fun, ret[fun]);
    }


    if (!strlen(all))
        FAIL("Eine solche Gruppe kenne ich nicht.\n");
    this_player()->more(explode(all[0..<2],"\n"),0,0,M_AUTO_END,MORE_ID);
    return 1;
}

int menu(string str) {
    string ret;

    if (!str) {
	ret=START_MORE;
	if (ret) {
	    menue=({mixed})HELP_TOOL->get_menu(0,0,original);
	    ret=START_MORE;
	    if(ret) {
		notify_fail(M_ERR(ret));
		return 0;
		}
	    }
	return 1;
	}
    notify_fail(query_verb()+" was?\n");
    return 0;
}

varargs string query_read(string rest, string str, object leser)
{
    if (!wizp (leser))
        return "Du siehst lauter wild auf und ab tanzende Buchstaben.\n";
    menu (0);
    return "";
}

int more_action(string str, int line, int max_line, mixed more_id) {
    int num;
    mixed tmp;
    
    if(more_id!=MENUE_ID)
	return CONTINUE;

    switch(str) {
      case "z":
	tmp=({mixed})HELP_TOOL->get_menu(menue[M_STATUS][0..<2],0,original);
	if (stringp(tmp)) {
	    write(tmp);
	    return NOTHING;
	    }
	else {
	    string ret;
	    
	    menue=tmp;
	    yet_started=1;
	    ret = START_MORE;
	    if(ret)
	    {
	        write(M_ERR(ret));
		return NOTHING;
	    }
	    return END_MORE;
	    }
      case "U":
	if (adminp(this_player())) {
	    HELP_TOOL->do_read(10);
	    tmp=({mixed})HELP_TOOL->get_menu(menue[M_STATUS],0,original);
	    if (stringp(tmp)) {
		write(tmp);
		menue=({mixed})HELP_TOOL->get_menu(0,0,original);
		}
	    else
		menue=tmp;
	    yet_started=1;
	    START_MORE;
	    return END_MORE;
	    }
      default:
	 if (sscanf(str,"%d",num))
	    if (num>0 && num<=menue[M_MAX_MENUE]) {
	        string ret;
		
		LAST_STATUS[S_LINE]=max(1,
                    line-({int})this_player()->query_more_chunk()+3);
		tmp=({mixed})HELP_TOOL->get_menu(menue[M_STATUS],num,original);
		if (stringp(tmp)) {
		    write(tmp);
		    return NOTHING;
		    }
		menue=tmp;
		yet_started=1;
	        ret = START_MORE;
	        if(ret)
	        {
		    write(M_ERR(ret));
		    return NOTHING;
		}
		return END_MORE;
		}
	     else if (menue[M_MAX_MENUE]) {
                write("Den Menü-Punkt "+num+" gibt es nicht.\n");
		return NOTHING;
		}
      }
}

void more_end(string str, int line, int max_line, mixed more_id) {
    if (more_id!=MENUE_ID)
	return;
    if (!yet_started && !menue[M_MAX_MENUE]) {
	more_action("z",line,max_line,MENUE_ID);
	return;
	}
    if (str=="q")
	LAST_STATUS[S_LINE]=max(1,
            line-({int})this_player()->query_more_chunk()+3);

    yet_started=0;
}

#ifdef DEBUG
mixed query_menue() { return menue; }
#endif
