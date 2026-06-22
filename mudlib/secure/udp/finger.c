// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/secure/udp/finger.c
// Description: Erzeugt den Finger-Output
// Modified by:	Freaky (06.10.94) Added Domainhelfer
//              Sissi (02.09.95) Added "finger admin", "finger mudadmin", ...
//              Freaky (21.02.96) personal_title = 0 anstatt = ""
//              Mammi (27.06.97) Gildenanzeige
//              Sissi (07.07.97) Spielerrat
//              Mammi (08.07.97) Nochmal Gildenanzeige
//		Copper (03.07.97) check_present fuer Spielerrat.
//              Jesaia (13.12.97) npcfinger eingebaut
//              Jesaia (21.01.97) npcfinger erweitert um ptitle
//              Sissi (02.06.99) npcfinger um , im titel erweitert
//              Parsec (24.10.99) Ausgabe an neue Lebenslaeufe angepasst
//              Sissi (22.5.2000) finger fuer gestorbene Spieler
//              Sissi (29.7.2000) extra Text fuer Admin Finger
//              Jesaia (15.10.2000) npcfinger debugged.

#include <udp.h>
#include <level.h>
#include <gilden.h>
#include <config.h>
#include <quest.h>
#include <game.h>
#include <apps.h>
#include <mail.h>
#include <finger.h>
#include <time.h>
#include <deklin.h>

#define ADMINFINGER "/static/adm/ADMIN_FINGER"
#define VORSTANDFINGER "/static/adm/VORSTAND_FINGER"
#define NO_WWW_PAGES "/static/adm/NO_WWW_PAGES"

// Bei welcher idle-Zeit wird das im 'finger admins' gezeigt.
#define ADMIN_BUSY_TIME	600

string gender, personal_title, title, gilde, cap_name, last_host,
    admin_finger, vorstand_finger, last_town, finger_info, sperren, away,
    www_page;
int player_age, level, rang, skill, last_login, no_wer, sperre_bis, idle,
    own_finger_flags;

string *no_www_pages;            
string *opfer;
mixed *offline_opfer;
mixed level_dates;
mixed wizard_appreciations;

#define DEBUG(msg)	if (find_player("monty"))\
			tell_object(find_player("monty"), msg)

string query_gilde() { return gilde; }
int query_rang() { return rang; }
string query_gender() { return gender; }

void create()
{
    admin_finger = read_file (ADMINFINGER);
    vorstand_finger = read_file (VORSTANDFINGER);
    no_www_pages = explode(read_file(NO_WWW_PAGES)||"","\n");
}

string do_npcfinger(string name, mapping npc)
{
     object ob;
     string temp;

     if ((ob=find_object(npc["file"])) && living(ob)
        && (!npc["only_default"] || !strlen(npc["short"])))
	 temp = wrap(add_dot_to_msg(ob->query_short()));
     else
         temp = wrap(add_dot_to_msg(
	           (strlen(npc["ptitle"])?npc["ptitle"]+" ":"") +
	           (name != "tod"? capitalize(name) : "TOD") +
		   (!strlen(npc["short"])?"":
		    (npc["short"][0]==',')?npc["short"]:(" "+npc["short"]))));
     if (ob && ob->query_finger_info_text() && !npc["only_default"])
	 temp += ob->query_finger_info_text();
     else
	 temp += implode(explode(npc["default"],"\\n"),"\n");
     return temp;
}

string check_present(string str)
{
    object ob = find_player(str);
    if (!ob)
	return capitalize(str);
    else if(interactive(ob))
	return "*" + capitalize(str);
    else
	return "+" + capitalize(str);
}

static string query_gesperrt()
{
    mapping wtage;
    string wtag;
    int tag, stu, min, bitnr, i;
	    
    if (!sperren)
	return 0;
    wtage = (["Mon":0,"Tue":1,"Wed":2,"Thu":3,"Fri":4,"Sat":5,"Sun":6]);
    sscanf(ctime(time()),"%s %~s %d %d:%d:%~d %~d",
	wtag,tag,stu,min);
    bitnr = wtage[wtag] * 96 + stu * 4 + min / 15;
    if (!test_bit(sperren,bitnr))
	return 0;
    if (bitnr == 671)
	i = 0;
    else
	i = bitnr + 1;
    while (test_bit(sperren,i))
	if (i == bitnr)
	    return "nach dem Ende der Welt ";
	else if (i == 671)
	    i = 0;
	else
	    i++;

    bitnr = i;
    tag = bitnr / 96;
    bitnr -= tag * 96;
    stu = bitnr / 4;
    bitnr -= stu * 4;
    min = bitnr * 15;
    return ({"Montag","Dienstag","Mittwoch","Donnerstag","Freitag",
	"Samstag","Sonntag"})[tag]+", "+stu+" Uhr "+(min?(string)min+" ":"");
}

// Wird auch von /i/living/face genutzt.
string moerder_text(string *opfer, string gender)
{
    string *fach = ({0,"zweifach","dreifach","vierfach","mehrfach"});
    string moerder = choose_by_gender(gender,
			({"moerderwesen", "mörder", "mörderin"}));
    string *mliste = ({});
    mapping mfach = ([]);
    int *count = ({0})*sizeof(opfer);
	
    for(int i=sizeof(opfer);i--;)
    {
        count[member(opfer, opfer[i])]++;
        if(count[i])
        {
	    if(count[i]>5)
	        count[i] = 5;
	    if(!mfach[count[i]])
		mfach[count[i]] = ({opfer[i]});
	    else
		mfach[count[i]] += ({opfer[i]});
	}
    }
    for(int i=1;i<6;i++)
	if(mfach[i])
	    mliste += ({ wer((["name": moerder, "gender": gender,
		    "adjektiv": fach[i-1] && ({fach[i-1]}) ]), ART_KEINS)
		   + " von " + liste(map(mfach[i],#'capitalize)) });//'
    return liste(mliste);
}

varargs string do_local_finger(string user, int local, int other_flags)
{
    string ret,tmp,rang_name;
    mixed tmp_mixed,tmp_mixed_2;
    string stat, er, bann;
    object ob;
    int local_wiz, i, j;
    mixed f_npc;
    mapping banish;


    if (!stringp(user))
	return 0;

#define ADMIN_ARR ({"admin","admins","mudadmin","mudadmins","oberste rat", \
	"oberste","oberster rat","oberste raete","oberste räte","adm","mudadm"})
#define VORSTAND_MAP (["vorstand","vereinsvorstand","traegerkreis","trägerkreis","verein"])

    local_wiz = local && this_interactive() && wizp(this_interactive());
    user = lower_case(user);
    if (admin_finger && (member(ADMIN_ARR,user) != -1))
    {
	ret = admin_finger;
	foreach(string admin_name: ADMINS)
	{
	    int pos = strstr(lower_case(admin_finger), admin_name);
	    if(pos < 0)
		continue;

	    for(i = strstr(admin_finger,"\n"); pos > (j = strstr(admin_finger,"\n",i+1)) && j>0; i=j);
	    i = strstr(admin_finger,"*",i);
	    if(i<0 || i>j)
		continue;
	
	    ob = find_player(admin_name);
	    if(!ob || ob->query_no_wer() || !interactive(ob))
	    {
		if(i && admin_finger[i-1..i+1]=="(*)")
		    ret[i-1..i+1] = "   ";
		else
		    ret[i]=' ';
	    }
	    else if(query_idle(ob)>ADMIN_BUSY_TIME)
		ret[i]='i';
	    
	}
	return ret;
    }
    if (vorstand_finger && member(VORSTAND_MAP,user))
        return vorstand_finger;

#ifdef ADMINS
    if (member(ADMIN_ARR,user) != -1)
    {
#ifdef MUD_NAME
#ifdef UNItopia
	ret = "Der Oberste Rat UNItopias hat im Moment die folgenden "
	      "Mitglieder:\n";
#else
	ret = "Die Admins von "MUD_NAME" sind zur Zeit:\n";
#endif
#else
	ret = "Die Admins sind zur Zeit:\n";
#endif
	 return ret + liste(map(ADMINS,#'capitalize))+".\n";
    }
#endif
#ifdef UNItopia
    if (member(VORSTAND_MAP,user))
        return wrap("Der Vorstand des Trägerkreises UNItopia e.V. "
	    "besteht aus: "+liste(map(VORSTAND,#'capitalize))+".");
	    
    if (-1 != member(({"spielerrat","spielerräte"}),user)) {
	if (SPIELERRAT->query_spielerrat())
	    return "Der Spielerrat "+MUD_NAME+"s hat im Moment die folgenden "
		"Mitglieder:\n"
		+wrap(liste(map(sort_array(
			SPIELERRAT->query_spielerrat(),#'>)
			,#'check_present))
		     +".\n(* bedeutet dabei, dass die-, das- oder derjenige gerade anwesend ist.)");
	return "Im Moment hat der Spielerrat keine Mitglieder.\n";
    }
    if ((user=="rat") || (user == "räte") || (user == "raete"))
	return "Spielerrat oder Oberster Rat?\n";
#endif            
    
    for (i = strlen(user); i--; )
	if (user[i] < 'a' || user[i] > 'z')
	    return "Der Name darf nur aus Buchstaben bestehen.\n";

    // Globale Variablen zuruecksetzen.

    // Strings auf "" setzen
    gender = title = gilde = last_host = last_town =
	finger_info = www_page = "";
    // Oder auch auf 0...
    personal_title = 0;
    // Ints auf 0 setzen
    idle = sperre_bis = player_age = level = rang = skill = last_login =
	own_finger_flags = 0;
    // Strings auf 0 setzen
    away = sperren = 0;
    // Mixed auf irgendwas setzen.
    level_dates = 0;
    opfer = ({}); ret = ""; offline_opfer = ({});
    wizard_appreciations = 0;
    cap_name = capitalize(user);
    
    // Bann-Text abfragen
    banish = BANISHD->query_banish();
    i=BANISHD->query_banished(user);
    switch(i)
    {
	case 0:
	    if(!banish[i][user])
		bann="Name ist verboten.";
	    else
		bann="Name ist reserviert. ("+
		     capitalize(banish[i][user])+
		     (banish[i][user,2]?": "+banish[i][user,2]:"")+")";
	    break;
	case 1..LVL_MAX_LEVEL-1:
    	    bann="Gesperrt für alle Level >= "+i+". ("+
		 capitalize(banish[i][user])+
		 (banish[i][user,2]?": "+banish[i][user,2]:"")+")";
            break;
    }
    
    f_npc=NPC_FINGER->query_info_of_npc(user);
    
    if (mappingp(f_npc) && f_npc["only_default"] &&
        (SECOND->is_testplayer(user) || SECOND->is_special(user)))
    {
	ret=do_npcfinger(user,f_npc);
	if(!local_wiz) return ret;
	ret+="\n"+center(" Der Spieler ",52,"-")+"\n";
    }
    
    if (ob=find_player(user) || ob = find_player("STATUE "+user))
    {
	// cap_name = ob->query_cap_name();
	cap_name = funcall(symbol_function("query_cap_name",ob));
	if(!sizeof(regexp(({lower_case(cap_name)}),"\\<"+user+"\\>")))
	    cap_name = capitalize(user);
	gender = funcall(symbol_function("query_gender",ob));
	title  = funcall(symbol_function("query_title",ob));
	personal_title = funcall(symbol_function("query_personal_title",ob));
	gilde = funcall(symbol_function("query_gilde",ob));
	player_age = funcall(symbol_function("query_age",ob));
	level = funcall(symbol_function("query_level",ob));
	rang = funcall(symbol_function("query_rang",ob));
	skill = funcall(symbol_function("query_sum_skill",ob));
	opfer = funcall(symbol_function("query_opfer",ob));
	level_dates = funcall(symbol_function("query_level_dates",ob));
	own_finger_flags = funcall(symbol_function("query_own_finger_flags",ob));
	www_page = funcall(symbol_function("query_www_page",ob));
	away = funcall(symbol_function("query_away",ob));
	if (interactive(ob)) 
	    idle = query_idle(ob);
	finger_info = funcall (symbol_function("query_finger_info",ob));
	no_wer = (wizp(ob) && funcall (symbol_function ("query_no_wer",ob))
	    && !local_wiz);
	wizard_appreciations = funcall(symbol_function("query_wiz_appreciations",ob));
	er = Er((["name":user,"gender":gender]));

	if (query_living_name(ob) != user)
	{
	    if (local_wiz)
		stat = er+" steht im Statuenraum.\n";
	    else
	    {
		no_wer = 1;
		stat= er+" ist gerade nicht da.\n";
	    }
	}
	else if (!no_wer && !interactive(ob)) 
	    stat = er+" ist gerade versteinert.\n";
	else if (!no_wer) {
	    stat = er+" ist gerade eingeloggt.\n";
	} else
	    stat= er+" ist gerade nicht da.\n";
	// Sperren bei eingeloggten Spielern nicht abfragen!
    }
    else if (restore_object(PLAYER_FILE(user)))
    {
	er = Er((["name":user,"gender":gender]));
	stat = er+" ist gerade nicht da.\n";
        if(offline_opfer)
            opfer = sort_array( ( opfer||({}) )+
		map(offline_opfer, (: lower_case($1[0]) :) ), #'> );
    }
    else
    {
	if (ret = BANISHD->query_special_finger(user))
	    return wrap (ret);
	if (GOETTER_REGISTER->query_banished_wiz(user))
	    return capitalize(user)+
		" war einst Mitglied des "+MUD_NAME+" Pantheons.\n";
	if (mappingp(f_npc) )
	    return do_npcfinger(user,f_npc);
	else
	    return "Von "+capitalize(user)+
		" hat in "+MUD_NAME+" noch niemand etwas gehört.\n" +
		((local_wiz && bann)?wrap(bann):"");
    }
    
    if (!(own_finger_flags & FINGER_FLAG_VALID))
        own_finger_flags = FINGER_FLAG_OWN_DEFAULT;

    if (stringp(personal_title))
	ret += personal_title+" ";
    if (stringp(cap_name))
	ret += cap_name;
    else
       ret += capitalize(user);
    //ret += cap_name;
    if (stringp(title) && title != "")
	ret += (title[0] == ',' ? "" : " ") + title;
    ret = wrap(add_dot_to_msg(ret));
    ret += stat;
    if (!no_wer && away && stringp (away))
	ret += wrap_say(er+" ist gerade weg: "+away,"");
    if (!no_wer && idle > 120)
	ret += er+" ist seit "+regreplace(format_seconds(idle/60*60),"Tage","Tagen", 0)
	    + ((level >= LVL_WIZ)?" schöpferisch tätig.\n":" völlig untätig.\n");
    if (no_wer || !ob || !interactive(ob)) 
      if (local_wiz || (own_finger_flags & FINGER_FLAG_OWN_DATE)
          || (own_finger_flags & FINGER_FLAG_OWN_TIME))
	ret += "Zuletzt war "+lower_case(er)+
	    (last_login 
	       ? ((local_wiz || (own_finger_flags & FINGER_FLAG_OWN_DATE))
	          ? " am " : " um ") +
	         (timestr(last_login,local_wiz ? 0 : 
		  (own_finger_flags & FINGER_FLAG_OWN_DATE)
		  &&(own_finger_flags & FINGER_FLAG_OWN_TIME)
		  ? 0 : (own_finger_flags & FINGER_FLAG_OWN_DATE) ? 2 : 1))
	       : " am "+timestr(file_time(PLAYER_FILE(user)+".o"), local_wiz ? 0 : 2))
	    +" da.\n";
    if (own_finger_flags & FINGER_FLAG_OWN_BIRTHDAY) {
        string bdays;
        int bday;
        if (ob && (bdays = funcall(symbol_function("query_birthday",ob))))
            ret += er+" wurde am "+bdays+" geboren.\n";
        else {
            // folgende schleife unbedingt rueckwaerts durchlaufen, sonst stimmts nicht
            for (i = sizeof(level_dates); i--; )
                if (level_dates[i][LVL_D_LEVEL] == LVL_PLAYER)
                    bday = level_dates[i][LVL_D_DATE];
            if (bday)
                ret += er+" wurde am "+vtimestr(bday - BIRTHDAY)+" geboren.\n";
        }
    }
                	    
    if (local_wiz && last_host && last_host != "" && 
	    (adminp(previous_object()) || member(GUEST_NAMES, user)>=0))
	ret += "Das letzte Mal kam "+lower_case(er)+" von "+last_host+"\n";
    else if (local_wiz && last_town && last_town != "" && 
	    (adminp(previous_object()) || member(GUEST_NAMES, user)>=0))
	ret += "Das letzte Mal kam "+lower_case(er)+" aus "+last_town+".\n";
    if (www_page && strlen (www_page) && member(no_www_pages,user)<0)
        ret += wrap_say("WWW Adresse:",www_page);
    if (sperre_bis && (sperre_bis > time()))
	ret += er+" hat sich selbst bis zum "+timestr(sperre_bis)+
	      " gesperrt.\n";
    else if (tmp=query_gesperrt())
	ret += wrap(er+" hat sich selbst gesperrt und darf "
	      "UNItopia leider erst wieder "+tmp+"betreten.");
    if (MAILD->unread_mail(user))
	ret += er+" hat ungelesene Post.\n";
    ret += "\n";
    if (local_wiz && (tmp = SECOND->is_testplayer(user)))
	ret += er+" ist Testie von "+capitalize(tmp)+".\n";
    if (local_wiz && (tmp = SECOND->is_special(user)))
    {
        ret += er+" ist ein Charakter von "+tmp+".\n";
	if(ob && stringp(tmp=ob->query_current_wiz_owner()))
	    ret += "Und wird gerade von "+capitalize(tmp)+" gespielt.\n";
    }
	
if (local_wiz)
//    Keine Gilden-Finger Anzeige mehr
    if (rang_name = GILDEN_OB->query_gilden_info(this_object(),RANG))                    
    {
	// Gibt es einen Eintrag GILDEN_FINGER im gilden_ob, so
	// Wird dieser Eintrag als Format genommen, ansonsten das
	// Standardformat "Er/Sie/Es ist <Rang> in dem(Gilde)"
	// Ansonsten Eintrag: 'who entspricht dem Spielermapping,
	//                    'rank entspricht dem aktuellen Rangmapping,
	//                    'guild entspricht der aktuellen Gilde.
	
	if (tmp_mixed = 
	    GILDEN_OB->query_one_gilden_info(gilde,GILDEN_FINGER))
	{          
	    tmp = apply(CLOSURE_CONTAINER->do_bind(
			mixed_to_closure(
			    tmp_mixed,
			    ({'who,'rank,'guild,'wiz}))),
			({(["name":user,"gender":gender]),
			  (["name":rang_name,"gender":gender]),
			  (["name":gilde,
			    "gender":GILDEN_OB->query_one_gilden_info(
					gilde,GILDEN_GESCHLECHT)
					|| "weiblich"]),
			  local_wiz
			}));
	    if (tmp)
		ret += wrap(tmp);
	}
	else
	{
	    ret += er+" ist "+rang_name+" in "
		 +dem((["name":gilde,
		 "gender":GILDEN_OB->query_one_gilden_info(gilde,GILDEN_GESCHLECHT)
		    ||"weiblich"]))+".\n";
	}
    }

    if (level == LVL_HLP)
	ret += er+" gehoert der Engelschar "+MUD_NAME+"s an.\n";
    else if (level >= LVL_WIZ)
    {
	ret += er+" gehoert dem "+
	    ((level >= LVL_ADMIN) && (member (ADMINS,lower_case(user)) != -1)
	      ? "Obersten Rat des Pantheons" : "Pantheon")
	    +" an.\n";
	if ((local_wiz
		|| (local && this_interactive() && hlpp(this_interactive())))
	    && (tmp = GOETTER_REGISTER->query_lehrerlaubnis(user)))
	    ret += er+" hat die Lizenz zum Ausbilden" 
	        + ((local_wiz && tmp != user)?" von "+capitalize(tmp)+" erhalten.\n":".\n");
	    // klingt so schoen nach Lizenz zum toeten, was auch passt
	    // Na ja.... G.
	if(local_wiz)
	{
	    // Lehrmeisteranzeige:
	    for (tmp_mixed = ({}), i = 0; i < 3; i++)
		tmp_mixed += ({GOETTER_REGISTER->query_lehrmeister(user,i)});
	    if (tmp_mixed && (tmp_mixed[0]=="gastgott"))
		ret += er+" ist ein Gast im Pantheon.\n";
	    else if (sizeof(tmp_mixed -= ({0,"leo"})))
		ret += er+
		    (level > LVL_GESELLE ? " wurde " : " wird ")
		    +"von "+liste(map(tmp_mixed,#'capitalize))
		    +" ausgebildet.\n";
	    
	    // Gouverneuranzeige:
	    if (sizeof(tmp_mixed = FILED->query_auth_of(user)||({})))
		ret += er+" ist zustaendig fuer "
		      +liste(map(tmp_mixed,#'capitalize))+".\n";

	    tmp = "";
	    //Domainlordanzeige:
	    if (sizeof(tmp_mixed = DOMAIN_INFOS->query_domains_of(user)||({})))
		tmp += er+" ist "
		    +(gender == "weiblich" ? "Domainlady" : "Domainlord")
		    +" von "+liste(tmp_mixed);            
	    
	    tmp_mixed_2 = DOMAIN_INFOS->query_domainhelfer_of(user)||({});
	    tmp_mixed = (DOMAIN_INFOS->query_memberships_of(user)||({})) 
		      - tmp_mixed - tmp_mixed_2;
	    
	    // Domainhelferanzeige:
	    if (sizeof(tmp_mixed_2))
	    {
		if (tmp != "")
		    tmp += sizeof(tmp_mixed) ? ", " : " und ";
		else
		    tmp += er+" ist ";
		tmp += "Domainhelfer von "+liste(tmp_mixed_2);
	    }                 
		
	    // Mitgliedanzeige:
	    if (sizeof(tmp_mixed))
	    {
		if (tmp != "")
		    tmp += " und ";
		else    
		    tmp += er+" ist ";
		tmp += "Mitglied in "+liste(tmp_mixed);                    
	    }
	    if (tmp != "")
		tmp = wrap(tmp+".");
	    
	    // Gildenlisting:
	    if (sizeof(tmp_mixed = GILDEN_OB->query_programmed_gilden(user)))
	    {
		tmp_mixed_2 = ({});
		for (i=sizeof(tmp_mixed);i--;)
		     tmp_mixed_2 += ({
			(["name": tmp_mixed[i], 
			  "gender": 
			    (GILDEN_OB->query_one_gilden_info(tmp_mixed[i],
			     GILDEN_GESCHLECHT)||"weiblich")
			])});           
		tmp += wrap(capitalize(liste(map(tmp_mixed_2,#'der)))
			+(sizeof(tmp_mixed_2) > 1 ? " werden " : " wird ")
			+"von "+ihm((["name":user,"gender":gender]))
			+" betreut.");
	    }
/*
	    // Raetsellisting:
	    if (sizeof(tmp_mixed = QUEST_ROOM->query_programmed_quests(user)))
	    {
		tmp += wrap(er+" hat sich "
		      +(sizeof(tmp_mixed) > 1 ? "die Raetsel ":"das Raetsel ")
		      +liste(tmp_mixed)+" ausgedacht.");
	    }
	    // Spielelisting:
	    if (sizeof(tmp_mixed = GAME_ROOM->query_programmed_games(user)))
	    {
		tmp += wrap(er+" hat "
		      +(sizeof(tmp_mixed) > 1 ? "die Spiele ":"das Spiel ")
		      +liste(tmp_mixed)+" programmiert.");
	    }
*/	    
	    if (tmp != "")
		ret += "\n"+tmp;
	}
    }
    
    if (local_wiz && GOETTER_REGISTER->is_wiz_on_vacation(user))
	ret += er+" ist als Gott gerade im Urlaub.\n";

    if (local_wiz && sizeof(wizard_appreciations)
      && (other_flags & FINGER_FLAG_OTHER_APPRECIATIONS))
        if (other_flags & FINGER_FLAG_OTHER_APPRECIATIONS_SORT_BY_DATE)
            ret += "\n"+implode(map(wizard_appreciations,
		(: wrap($1[3]) :)),"")+"\n";
        else {
            // sortieren nach Gebiet
            string *dom;
            int found;
            ret += "\n";
            dom = map (sort_array (DOMAIN_INFOS->query_domains(),#'>)
                + sort_array (FILED->query_types()+({"admin"}),#'>),
                #'lower_case);
            for (i = 0; i < sizeof (dom); i++) {
                for (found = j = 0; j < sizeof (wizard_appreciations); j++)
                    if (wizard_appreciations[j][0] == dom[i]) {
                        if (!found)
                            ret += capitalize (dom[i])+":\n";
                        found = 1;
			ret += sprintf("    %-=73s\n",wizard_appreciations[j][3]);
                        wizard_appreciations[j][0] = "";
                    }
            }
        }

#ifdef UNItopia
    if (spielerratp(user))
	ret += er+" ist Mitglied des Spielerrates UNItopias.\n";
#endif
    if (sizeof (opfer)) opfer -= ({0});
    if (sizeof(opfer) && level < LVL_WIZ)
	ret += wrap(er+" ist "+moerder_text(opfer, gender)+".");
    
    if (local_wiz)
    {
// TODO:  Diese Levelassoziation wird sicher auch noch andernorts
// TODO:: benoetigt (z.B.: Levelaenderungskanal...)
// TODO:: Funktion anbieten, die aus int level einen Beschreibungsstring
// TODO:: erzeugt
	ret += "\nLevel     : ";
#define CH(a,b) (random(100)?(a):(b))
	if ("/apps/second"->is_testplayer(user))
	    ret += "Testie (Level "+level+")\n";
	else if (level == 0)
	    ret += "Monster (Level 0)\n";
	else if (level == LVL_PLAYER)
	    ret += "Spieler\n";
	else if (level == LVL_HLP)
	    ret += "Engel\n";
	else if (level == LVL_LEARNER)
	    ret += CH("Lehrling\n","Brix\n");
	else if (level == LVL_GESELLE)
	    ret += CH("Geselle\n","Zaddelbrix\n");
	else if (level != LVL_VOGT)
	    ret += level+"\n";
	else if (member(ADMINS,user)!=-1)
	    ret += CH("Admin\n","Daddelfghlaarwapolchtozaddelbrix\n"); 
	else if (sizeof("/secure/master"->query_domains_of(user)) ||
	     (member("/apps/filed"->query_all_auth(), user) >= 0))
	    ret += CH("Lord\n","Fghlaarwapolchtozaddelbrix\n"); 
	else ret += CH("Vogt\n","Polchtozaddelbrix\n");
	if(bann)
	    ret+=sprintf("Status    : %=-66s\n",bann);
	ret += "Erfahrung : "+skill+" ("+(skill*100/TOTAL_EXPERIENCE)+","
	    + ((skill*1000/TOTAL_EXPERIENCE)%10) + "%)\n";
	ret += "Alter     : "+format_seconds(player_age)+"\n";
	if ((other_flags & FINGER_FLAG_OTHER_CURRICULUM_VITAE)
	  && (level_dates && pointerp(level_dates) && sizeof(level_dates)))
	{
	    ret += "\nLebenslauf:\n";
	    for(i = 0; i < sizeof(level_dates); i++)
		ret += sprintf(
		    "    %2d:  %s%-30s\n",
		    level_dates[i][LVL_D_LEVEL],
		    level_dates[i][LVL_D_DATE] ?
		    " "+shorttimestr(level_dates[i][LVL_D_DATE])+"    " :
		    "<"+shorttimestr(DATUM_KONV_VOLLST_LEBENSLAEUFE)+"   <",
		    level_dates[i][LVL_D_AGE] ?
		    format_seconds(level_dates[i][LVL_D_AGE]) : "");
	}
    }

    if (other_flags & FINGER_FLAG_OTHER_TEXT)
    {
	if (finger_info && (finger_info != ""))
	    ret += "\n" + (tmp = stringp(cap_name) ? cap_name :
	    capitalize(user))+" über " + tmp + ":\n" +wrap(finger_info);

	if(level >= LVL_WIZ &&
#if __VERSION__ > "3.5.2"
	   sizeof(tmp=to_text(read_bytes("/w/"+user+"/.finger",0,1500)||b"", "UTF-8//REPLACE")))
#else
	   (tmp=read_bytes("/w/"+user+"/.finger",0,1500)))
#endif
	    ret+="\n"+tmp;
    }
    
    if (other_flags & FINGER_FLAG_OTHER_PLAN)
    {
	if(local_wiz && level >= LVL_WIZ &&
#if __VERSION__ > "3.5.2"
	   sizeof(tmp=to_text(read_bytes("/w/"+user+"/.plan",0,1500)||b"", "UTF-8//REPLACE")))
#else
	   (tmp=read_bytes("/w/"+user+"/.plan",0,1500)))
#endif
	    ret+="\n"+tmp;
    }    
    return ret;
}

string do_finger(string str,int other_flags)
{
    string user, host;

    if (!str)
	return "Bitte einen String angeben.\n";
    if ((member(str,' ')>=0) && strstr (lower_case (str),"oberste rat")
	&& strstr (lower_case (str),"oberster rat")
	&& strstr (lower_case (str),"oberste raete")
	&& strstr (lower_case (str),"oberste räte"))
	return "Ein Name darf keine Leerzeichen beinhalten.\n";
    if (!this_interactive())
	return "So geht das leider nicht.\n";
    str=lower_case(str);
    if (sscanf(str,"%s@%s",user,host)==2)
    {
	if (strstr(lower_case(MUD_NAME),host))
	{
	    if (!INETD->known_mud(host))
		return "Unbekanntes Mud.\n";
	    INETD->send_udp(host,([REQUEST:"finger",
				   DATA:user,
				   SENDER:
				     this_interactive()->query_real_name()
				   ]),1);
	    return "";
	}
	else
	    return do_local_finger(user,1,other_flags);
    }
    else
	return do_local_finger(str,1,other_flags);
}

void udp_finger(mapping data)
{
    INETD->send_udp(data[NAME], ([
	REQUEST: REPLY,
	RECIPIENT: data[SENDER],
	ID: data[ID],
	DATA: do_local_finger(data[DATA],0,FINGER_FLAG_OTHER_EXTERN)
    ]) );
}

/* 
FUNKTION: query_finger_info_text
DEKLARATION: string query_finger_info_text()
BESCHREIBUNG:
Diese Funktion kann im Monster geschrieben werden, um einem Spieler
beim Anfingern des NPCs einen Text auszugeben.

Der NPC muss dazu (zu diesem Zeitpunkt) geladen sein, im Rathaus als
anfingerbarer NPC im Rathaus in /room/rathaus/npcfinger eingetragen
worden sein, und ist muss dort angegeben werden, dass dieser NPC
nurdefault=nein hat. Liefert die Funktion query_finger_info_text dann
einen String != 0 zurueck, so wird dieser einem Spieler beim Anfingern
des NPCs ausgegeben.
GRUPPEN: monster
*/
