// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /apps/statistik.c
// Description: Statistik ueber eingeloggte Spieler
// Author:      Freaky '93-'95

// UID: Apps
 
// Modified by: Sissi   (28.04.1996): Monatslisten
//		Freaky  (16.05.1996): login_time usw werden im create() initiali..
//              Sissi (11./12.10.98): Gildenstatistik

#include <time.h>
#include <level.h>
#include <message.h>
#include <simul_efuns.h>
 
#define ST_SAVE_TIME 300
#define STATISTIK_DIR "/var/statistik"
#define SAVE_FILE STATISTIK_DIR "/statistik"
#define STATISTIK_BUCH_DIR STATISTIK_DIR "/buch"
 
mapping login_time, total_day_time, month_time, gilden_statistik,
        brueller_namen, todesursachen, rettungsgruende;

mixed *letzte_tode, *letzte_rettungen;

string month_start, oldmonth;

int max_users, max_users_time, max_users_ever, max_users_ever_time,
    brueller_spieler, brueller_engel, brueller_goetter,
    gilden_statistik_time, todes_statistik_time, bruell_statistik_time,
    users_integral;


static mapping top_month_time;
static string *gilden_statistik_string;
static int save, five_time, last_five_time;

void reset();
 
int query_max_users() { return max_users; }
int query_max_users_time() { return max_users_time; }
int query_max_users_ever() { return max_users_ever; }
int query_max_users_ever_time() { return max_users_ever_time; }
int query_users_integral() { return users_integral; }
int query_sum_users() { return sizeof(total_day_time); }
int query_sum_users_month() { return sizeof(month_time); }
int query_five_time() { return five_time; }

void process_logon(string str)
{
    int us, age;
    object who;
 

    if (stringp(str) && !testplayerp (str))
    {
	login_time[str] = time();
	if (!member(total_day_time,str))
	    total_day_time[str] = 0; // Damit es angelegt ist
	if (who = find_player(str))
	{
	    age = who->query_age();
	    if (!member(month_time,str))
		month_time[str,1] = age;
	    month_time[str,2] = age;
	}
    }
    us = sizeof(users());
    if (us > max_users)
    {
	max_users = us;
	max_users_time = time();
	if (max_users > max_users_ever)
	{
	    max_users_ever = max_users;
	    max_users_ever_time = time();
	}
    }
}

void process_logoff(string str)
{
    object who;
 
    if (!stringp(str) || testplayerp (str))
	return;
    if (login_time[str])
	total_day_time[str]+=time()-login_time[str];
    m_delete(login_time,str);
    if ((who = find_player(str)) && member(month_time,str))
	month_time[str,2] = who->query_age();
}

void logon(string str)
{
    process_logon(str);
}

void logoff(string str)
{
    process_logoff(str);
}

private int get_system_usage()
{
    int *ru = rusage();
    float sum = to_float(ru[0]) + to_float(ru[1]);
    if(sum < 0)
        sum = sum + __INT_MAX__ + __INT_MAX__;

    return to_int(sum);
}

#ifndef TestMUD
void do_only_save()
{
   save_object(SAVE_FILE);
}
 
int remove()
{
   do_only_save();
   return 0;
}

void prepare_renewal()
{
   do_only_save();
}

void abort_renewal() {}
void finish_renewal(object neu) {}

private void write_users()
{
    int tim, us;
    string day;
 
    tim = time();
    us = sizeof(users());
    if (us > max_users)
    {
	max_users = us;
	max_users_time = time();
	if (max_users > max_users_ever)
	{
	    max_users_ever = max_users;
	    max_users_ever_time = time();
	}
    }
    users_integral += us;
    write_file(STATISTIK_DIR "/time.u",""+tim+" "+us+" 0\n");
    day = strftime("%Y%m%d");
    tim = (tim+TIME_ADJUST) % DAY;
    write_file(STATISTIK_DIR "/ti."+day,""+tim+" "+us+" 0\n");
}
 
static void do_save()
{
    int tmp;
 
    call_out("do_save",ST_SAVE_TIME-time()%ST_SAVE_TIME);
    write_users();
    if (save)
    {
	save = 0;
	do_only_save();
    }
    else
	save = 1;
    tmp = get_system_usage();
    five_time = tmp-last_five_time;
    last_five_time = tmp;
}
#endif
 
private string fmt_sec(int t)
{
    string ret;
 
    ret = sprintf("%2d h  ",t/3600);
    t %= 3600;
    ret += sprintf("%2d m  ",t/60);
    t %= 60;
    return ret+sprintf("%2d s",t);
}
 
private string fmt_min(int t)
{
    string ret;
 
    ret = sprintf("%2d Tage  ",t/86400);
    t %= 86400;
    ret += sprintf("%2d Stunden  ",t/3600);
    t %= 3600;
    return ret+sprintf("%2d Minuten",t/60);
}
 
#ifndef TestMUD
static void compute_diff(string wer, int a, int b, int c)
{
  if (!testplayerp (wer))
    if (c - b > 86399)
        top_month_time[wer] = c - b;
}
 
static void do_day()
{
    string ret, *pp, mon, newmon, sday;
    int i, end, day;
 
    day = DAY - (time()+TIME_ADJUST) % DAY;
    call_out("do_day",day);
    // wenn der Tag noch nicht ganz vorbei ist, dann nichts machen.
    if (day < (DAY - 7000))
	return;
 
    write_file(STATISTIK_DIR "/time.m",(time()-7000)/DAY+" "+max_users+" "+
	    sizeof(total_day_time)+ " " + users_integral + "\n");
    max_users = 0;
    users_integral = 0;
    pp = sort_array(m_indices(total_day_time),#'>);
    ret = "";
    for (i=0; i<sizeof(pp); i++)
	ret += sprintf("%-12s %10d sec %3d %%   %38s\n",pp[i],
		total_day_time[pp[i]],total_day_time[pp[i]]*100/DAY,
		fmt_sec(total_day_time[pp[i]]));
 
    write_file(STATISTIK_DIR "/time.d","\n\n"+timestr(time()-9999)[0..<9]+
		"\n"+ret);
 
    pp = sort_array(m_indices(total_day_time),
	lambda(({'x, 'y}),({#'<,({#'[, total_day_time, 'x}),
				({#'[, total_day_time, 'y})})));
    end = sizeof(pp);
    end = end<22?end:22;
    ret = "";
    for (i=0; i<end; i++)
	ret += sprintf("%-12s %10d sec %3d %%   %38s\n",pp[i],
		total_day_time[pp[i]],total_day_time[pp[i]]*100/DAY,
		fmt_sec(total_day_time[pp[i]]));

    mon = strftime("%b", time()-7000, 0);
    newmon = strftime("%b", time()+7000, 0);
    sday = strftime("%d", time()-7000, 0);

    if (!oldmonth) oldmonth = mon;
    rm(STATISTIK_BUCH_DIR "/times."+sday);
    write_file(STATISTIK_BUCH_DIR "/times."+sday,
	    timestr(time()-7000, TIMESTR_ONLY_DATE)+"\n"+ret);
 
    total_day_time = ([]);
    login_time = ([]);
 
    // Jetzt noch die Monatsliste
    top_month_time = ([]);
    walk_mapping(month_time,#'compute_diff);
    pp = sort_array(m_indices(top_month_time),
	lambda(({'x, 'y}),({#'<,({#'[, top_month_time, 'x}),
				({#'[, top_month_time, 'y})})));
    end = sizeof(pp);
    end = end<100?end:100;
    ret = "";
    for (i=0; i<end; i++)
	ret += sprintf("%8s   %-12s    %38s\n",
		(i+1)+".",
		capitalize(pp[i]),
		fmt_min(top_month_time[pp[i]]));
 
    rm(STATISTIK_BUCH_DIR "/month."+mon);
    write_file(STATISTIK_BUCH_DIR "/month."+mon,
	    center("Monatsliste von "+month_start,75)+"\n"
	   +center("bis "+timestr(time()),75)
	   +"\n\n"+ret);
 
    if (newmon != oldmonth)
    {   // Neuer Monat!
	month_start = timestr(time());        
	month_time = m_allocate(1,3);
        oldmonth = newmon;
        // gilden_statistik = m_allocate(1,7);
    }
    reset();
}

string *query_monats_statistik()
{
    return explode(read_file(STATISTIK_BUCH_DIR "/month."+strftime("%b", time(), 0)),"\n")[0..<2];
}
#endif
 
void reset()
{
    string *us, *pp;
    int i, t;
    object who;
 
#ifndef TestMUD
    if (find_call_out("do_day") < 0)
    {
	call_out("do_day",DAY-(time()+TIME_ADJUST)%DAY);
	sys_log("TIME","CALL_OUT weg : do_day "+timestr(time())+"\n");
    }
 
    if (find_call_out("do_save") < 0)
    {
	call_out("do_save",ST_SAVE_TIME-time()%ST_SAVE_TIME);
	sys_log("TIME","CALL_OUT weg : do_save "+timestr(time())+"\n");
    }
#endif
 
    us = map_objects(users(),"query_real_name");
    pp = m_indices(login_time);
    for (i=0; i<sizeof(us); i++)
	if (member(pp,us[i])<0)
	    process_logon(us[i]);
 
    t = time();
    for (i=0; i<sizeof(pp); i++)
    {
	if (member(us,pp[i])<0)
	    m_delete(login_time,pp[i]);
	else
	{
	    total_day_time[pp[i]] += t-login_time[pp[i]];
	    login_time[pp[i]] = t;
	}
    }
    for (i=0; i<sizeof(us); i++)
	if ((who = find_player(us[i])) && member(month_time,us[i]))
	    month_time[us[i],2] = who->query_age();
 
#ifndef TestMUD
    do_only_save();
#endif
}
 
void create()
{
    if (!login_time)
    {
	login_time = ([]);
	total_day_time = ([]);
	month_time = m_allocate(1,3);
        gilden_statistik = m_allocate(1,7);

#ifndef TestMUD
	restore_object(SAVE_FILE);
        //gilden_statistik = m_allocate(1,7);
	call_out("do_save",ST_SAVE_TIME-time()%ST_SAVE_TIME);
	call_out("do_day",DAY-(time()+TIME_ADJUST)%DAY);
#endif
	last_five_time = get_system_usage();
	if (!month_start)
	    month_start = timestr(time());
	// Start der Ermittlung der Monatsliste
	reset();
    }
}

void reset_gilden_statistik()
{
    if ((strstr(object_name(previous_object()),"/obj/zauberstab"))
        || !adminp (this_interactive()))
            return;
   gilden_statistik_time = time ();
   gilden_statistik = m_allocate (1,7);
   write ("Gildenstatistik zurückgesetzt.\n");
}

void reset_todes_statistik()
{
    if ((strstr(object_name(previous_object()),"/obj/zauberstab"))
        || !adminp (this_interactive()))
            return;
   todes_statistik_time = time ();
   todesursachen = 0;
   rettungsgruende = 0;
   write ("Todesursachenstatistik zurückgesetzt.\n");
}
            
void reset_bruell_statistik()
{
    if ((strstr(object_name(previous_object()),"/obj/zauberstab"))
        || !adminp (this_interactive()))
            return;
   bruell_statistik_time = time ();
   brueller_namen = 0;
   brueller_spieler = brueller_engel = brueller_goetter = 0;
   write ("Bruellstatistik zurückgesetzt.\n");
}
            

string query_month_time_of(mixed s)
{
    if (objectp(s))
	s = s->query_real_name();
    if (!stringp (s) || !member(month_time,s))
       	return 0;
    return format_seconds(((month_time[s,2] - month_time[s,1]) / 3600) * 3600);
}

void enter_gilde (string gilde, int level, int erste_gilde)
{
    if (level == LVL_PLAYER)
        gilden_statistik[gilde,0]++;
    else if (level == LVL_HLP)
        gilden_statistik[gilde,1]++;
    if (erste_gilde && (level == LVL_PLAYER))
    // Wenn ein Engel da mit erste Gilde kommt, dann ist das
    // hoechstwahrscheinlich einer, der aelter als die
    // Gildenliste in /i/player/login ist.
        gilden_statistik[gilde,4]++;
}

void leave_gilde (string gilde, int level)
{
    if (level == LVL_PLAYER)
        gilden_statistik[gilde,2]++;
    else if (level == LVL_HLP)
        gilden_statistik[gilde,3]++;
}

void suicid_gilde (string gilde, int level)
{
    if (level == LVL_PLAYER)
        gilden_statistik[gilde,5]++;
    else if (level == LVL_HLP)
        gilden_statistik[gilde,6]++;
}

void walk_gilden_statistik (string gilde, int player_ein, int hlp_ein,
    int player_aus, int hlp_aus, int erst_gilde, int player_sui, int hlp_sui)
{
    gilden_statistik_string += ({
	    sprintf("%-27s %4d %4d %4d %9d %4d %4d %12d",
		gilde,player_ein,player_aus,player_sui,
		hlp_ein,hlp_aus,hlp_sui,
		erst_gilde) });
}

string *query_gilden_statistik()
{
    string *ret;

    gilden_statistik = m_delete (gilden_statistik,
        "/z/Gilden/Hexenvolk/apps/gilden_ob");
    gilden_statistik_string = ({});
    walk_mapping(gilden_statistik,#'walk_gilden_statistik);
    gilden_statistik_string = sort_array (gilden_statistik_string,#'>);
    ret = ({ (" " * 25) + "Gildenstatistik seit "
           +shorttimestr (gilden_statistik_time,1,2),
	    "",
	    sprintf("%32s%-21s%-16s davon","","Spieler","Engel"),
	    "Gilde                        Ein  Aus  Sui       "
		"Ein  Aus  Sui      Erstgilde",
	    "~~~~~                        ~~~~~~~~~~~~~       "
	    	"~~~~~~~~~~~~~      ~~~~~~~~~" }) +
	gilden_statistik_string;
    gilden_statistik_string = 0;
    return ret;
}

string convert_todesursache (string nam, string s, string gender)
{
    string pronnom;
    s = regreplace (s, "Du hast Dich",nam+" hat sich",1);
    // Das erste Vorkommen durch den Namen ersetzen, den Rest durch "sich".
    s = regreplace (s, "\\<(Du|Dich|du|dich|Dir|dir)\\>", nam, 0);
    if(gender)
    {
	// Du am Satzanfang
	s = regreplace (s, "([.?!] |^)Du\\>",
	    ([  "saechlich":"\\1Es",
		"maennlich":"\\1Er",
		"weiblich":"\\1Sie"
	    ])[gender] || "\\1Es", 1);
	s = regreplace (s, "\\<[Dd]u\\>",
	    pronnom = ([  "saechlich":"es",
		"maennlich":"er",
		"weiblich":"sie"
	    ])[gender] || "es", 1);
    }
    else
	s = regreplace (s, "\\<(Du|du)\\>", pronnom = nam, 1);
    // Nach Saetzen aufsplitten und Dich/Dir entweder durch Ihm/Ihr/Sie oder
    // Sich ersetzen.
    s=implode(map(regexplode(s, "[.?!]"),
    (:
	if(sizeof(regexp( ({lower_case($1)}), "\\<("+lower_case($2)+"|"+$4+")\\>")))
	// Derjenige kommt als Subjekt vor.
	{
	    $1 = regreplace ($1, "([.?!] |^)(Dich|Dir)\\>","\\1Sich",1);
	    return regreplace ($1, "\\<([Dd]ich|[Dd]ir)\\>","sich",1);
	}
	else
	{
	    string prondat = 
		([  "saechlich":"ihm",
		    "maennlich":"ihm",
		    "weiblich":"ihr"
		])[$3] || $2;
	    string pronakk = 
		([  "saechlich":"es",
		    "maennlich":"ihn",
		    "weiblich":"sie"
		])[$3] || $2;
	    $1 = regreplace ($1, "([.?!] |^)Dich\\>","\\1"+capitalize(pronakk),1);
	    $1 = regreplace ($1, "([.?!] |^)Dir\\>","\\1"+capitalize(prondat),1);
	    $1 = regreplace ($1, "\\<[Dd]ich\\>",pronakk,1);
	    return regreplace ($1, "\\<[Dd]ir\\>",prondat,1);
	}
    :),nam,gender,pronnom),"");
    if(strstr(s,nam)<0)
	s = regreplace (s, "\\<[Dd]ein(em|en|es|e|)\\>",get_genitiv(nam),0);
    s = regreplace (s, "([.?!] |^)Dein", (gender=="weiblich")?"\\1Ihr":"\\1Sein",1);
    s = regreplace (s, "\\<[Dd]ein",(gender=="weiblich")?"ihr":"sein",1);

    // Nun noch die ganzen Verben:
    s = regreplace (s, "\\<bist\\>","ist", 1);
    s = regreplace (s, "\\<hast\\>","hat", 1);
    s = regreplace (s, "\\<wirst\\>","wird",1);
    s = regreplace (s, "\\<warst\\>","war",1);
    s = regreplace (s, "\\<wei(ss|ß)t>\\>","weiß",1);
    s = regreplace (s, "\\<(wurde|hatte|haette|hätte|wollte|konnte|soll|sollte)st\\>","\\1",1);
    if (strstr (s, nam) == -1)
        s = nam + ": "+s;
    return s;
}

string convert_rettungsgrund (string nam, string s)
{
    s = regreplace (s, "Todesort:", "Ort der Rettung:",1);
    s = regreplace (s, "Du hast Dich beim Schwimmen übernommen und bist ertrunken",
        nam+" wäre beinahe ertrunken",1);
    s = regreplace (s, "Du hast Dich",nam+" hat sich beinahe",1);
    s = regreplace (s, "Du dir ", nam+" sich ", 1);
    s = regreplace (s, "du dir ", nam+" sich ", 1);
    s = regreplace (s, "Du |Dich |du |dich |Dir |dir ", nam+" ", 1);
    s = regreplace (s, " Du\\.| Dich\\.| du\\.| dich\\.| Dir\\.| dir\\.", " "+nam+".", 1);
    s = regreplace (s, "eigenen Willenkraft","eigener Willenskraft",1);
    s = regreplace (s, "eigenen Willenskraft","eigener Willenskraft",1);
    s = regreplace (s, "Deiner|deiner",get_genitiv(nam),1);
    s = regreplace (s, "Deinem|deinem",get_genitiv(nam),1);
    s = regreplace (s, "Deinen|deinen",get_genitiv(nam),1);
    s = regreplace (s, "Deines|deines",get_genitiv(nam),1);
    s = regreplace (s, "Deine|deine",get_genitiv(nam),1);
    s = regreplace (s, "Dein|dein",get_genitiv(nam),1);
    s = regreplace (s, "\\<bist\\>","wäre beinahe", 1);
    s = regreplace (s, "\\<wurdest\\>","wurde beinahe",1);
    s = regreplace (s, "\\<hast\\>","hätte beinahe", 1);
    s = regreplace (s, "\\<hattest\\>","hätte beinahe",1);
    s = regreplace (s, "\\<hat\\>","hätte beinahe", 1);
    s = regreplace (s, "\\<hättest\\>","hätte",1);
    s = regreplace (s, "\\<haettest\\>","hätte",1);
    s = regreplace (s, "\\<wirst\\>","wurde beinahe",1);
    s = regreplace (s, "\\<wolltest\\>","wollte",1);
    s = regreplace (s, "\\<weißt>\\>","wüsste beinahe",1);
    s = regreplace (s, "\\<weisst>\\>","wüsste beinahe",1);
    // bei mehreren beinahes alle bis aufs erste rausmachen
    int beinahe1;
    beinahe1 = strstr (s,"beinahe");
    if (beinahe1 != -1) {
        int beinahe2;
        while ((beinahe2 = strstr (s," beinahe",beinahe1)) != -1) {
            s = s[0..beinahe2-1]+s[beinahe2+8..];
        }
    }
    // das Wort "leider" rausnehmen
    int leider;
    while ((leider = strstr (s," leider")) != -1) {
        s = s[0..leider-1]+s[leider+7..];
    }
    if (strstr (s, nam) == -1)
        s = nam + ": "+s;
    return s;
}

void player_died (string erf_tod_other, string erf_tod)
{
    if (!playerp (previous_object ()) || testplayerp (previous_object()))
        return;

    if (!todesursachen)
        todesursachen = ([erf_tod: 1]);
    else
        todesursachen[erf_tod]++;

    if (!letzte_tode)
        letzte_tode = ({ ({
	    previous_object()->query_real_cap_name(), time(), erf_tod_other
	}) });
    else
        letzte_tode += ({ ({
	    previous_object()->query_real_cap_name(), time(), erf_tod_other
	}) });
    call_out ("carve_in_stone",10);
}

void player_saved (string erf_tod_other, string erf_tod)
{
    if (!playerp (previous_object ()) || testplayerp (previous_object()))
        return;
	
    if (!rettungsgruende)
        rettungsgruende = ([erf_tod: 1]);
    else
        rettungsgruende [erf_tod]++;

    if (!letzte_rettungen)
        letzte_rettungen = ({({
	    previous_object()->query_real_cap_name(), time (), erf_tod_other
	})});
    else
        letzte_rettungen += ({({
	    previous_object()->query_real_cap_name(), time(), erf_tod_other
	})});

    call_out ("carve_in_cloud",10);
}

void carve_in_stone ()
{
    object kirche;
    mixed gedenktafel;
    if (!(kirche = touch (DEFAULT_ROOM_AFTER_DEATH))) return;
    if ((!(gedenktafel = present ("gedenktafel", kirche)))
        && (!(gedenktafel = kirche->query_v_item(({"gedenktafel"})))))
        return;
    kirche->send_message (MT_LOOK, MA_MAGIC,
        wrap ("Ein Lichtschimmer hüllt "+den(gedenktafel)
        +" ein. Du siehst, wie die Buchstaben auf "+dem(gedenktafel)
        +" in Bewegung geraten und zusammenrücken, um neuen Buchstaben "
        "platz zu machen, die auf "+dem(gedenktafel)
        +" zunächst hell leuchtend aus dem Nichts heraus entstehen, "
        "um sich dann dunkler werdend in die vorhandenen "
        "Buchstabenreihen als neue Reihe einzureihen."));
}

void carve_in_cloud ()
{
    object wolke;
    mixed gedenktafel;
    if (!(wolke = touch ("/room/hlp/wolke"))) return;
    if ((!(gedenktafel = present ("gedenktafel", wolke)))
        && (!(gedenktafel = wolke->query_v_item(({"gedenktafel"})))))
        return;
    wolke->send_message (MT_LOOK, MA_MAGIC,
        wrap ("Ein Lichtschimmer hüllt "+den(gedenktafel)
        +" ein. Du siehst, wie die Buchstaben auf "+dem(gedenktafel)
        +" in Bewegung geraten und zusammenrücken, um neuen Buchstaben "
        "platz zu machen, die auf "+dem(gedenktafel)
        +" zunächst hell leuchtend aus dem Nichts heraus entstehen, "
        "um sich dann dunkler werdend in die vorhandenen "
        "Buchstabenreihen als neue Reihe einzureihen."));
}

int filter_mapping_verkleinerer (string dummy, int anzahl, int min)
{
    return anzahl > min;
}

int sort_player_died_statistik (string a, string b)
{
    return (todesursachen[a] < todesursachen[b]) ||
           (todesursachen[a] == todesursachen[b] && a > b);
}

string *player_died_statistik()
{
    mapping tode;
    string *res;
    int i,alt,neu;
    if (!todesursachen)
        return ({"Keine Todesstatistik vorhanden."});

    tode = copy (todesursachen);
    if(sizeof(tode)>100)
	tode = filter(tode, #'filter_mapping_verkleinerer, 
			sort_array(m_values(tode), #'<)[100]);
    
    res = sort_array (m_indices (tode), #'sort_player_died_statistik);
    alt = 0;
    for (i = 0; i < sizeof (res); i++) {
        if ((neu = tode[res[i]]) != alt) {
            alt = neu;
            res[i..i] = ({"",(neu>1)?neu+" Tode:":"1 Tod:",res[i]});
            i += 2;
        }
    }
    return ({"Die häufigsten Todesursachen seit "
       +shorttimestr(todes_statistik_time,1,2)+":"})+res;
}

string *query_letzte_tode ()
{
    string *res, s, h;
    int i;
    if ((!letzte_tode) || (!sizeof (letzte_tode)))
        return ({"Es ist schon lange niemand mehr gestorben."}); 
    for (i = sizeof (letzte_tode), res = ({}); i--; )
        if (time() - letzte_tode[i][1] > 3504000)
            letzte_tode = arr_delete (letzte_tode, i);
        else {
            s = letzte_tode [i][2];
            res += explode(wrap_say (h=shortvtimestr(time_to_vtime(letzte_tode [i][1]),1,2)+":",s,
                0,strlen(h)+1)[0..<2],"\n");
        }
    return ({"Zu beklagende Tode des letzten Jahres:"}) + res;
}

int sort_player_saved_statistik (string a, string b)
{
    return (rettungsgruende[a] < rettungsgruende[b]) ||
           (rettungsgruende[a] == rettungsgruende[b] && a > b);
}

string *player_saved_statistik()
{
    mapping tode;
    string *res;
    int i,alt,neu;
    if (!rettungsgruende)
        return ({"Keine Rettungsgründestatistik vorhanden."});

    tode = copy (rettungsgruende);
    if(sizeof(tode)>100)
	tode = filter(tode, #'filter_mapping_verkleinerer, 
			sort_array(m_values(tode), #'<)[100]);

    res = sort_array (m_indices (tode), #'sort_player_saved_statistik);
    alt = 0;
    for (i = 0; i < sizeof (res); i++) {
        if ((neu = tode[res[i]]) != alt) {
            alt = neu;
            res[i..i] = ({"",(neu>1)?neu+" Rettungen:":"1 Rettung:",res[i]});
            i += 2;
        }
    }
    return ({"Die häufigsten Rettungsgründe seit "
       +shorttimestr(todes_statistik_time,1,2)+":"})+res;
}

string *query_letzte_rettungen_debug ()
{
    string *res, s, nam;
    int i;
    if ((!letzte_rettungen) || (!sizeof (letzte_rettungen)))
        return ({"Es ist schon lange niemand mehr gerettet worden."}); 
    for (i = sizeof (letzte_rettungen), res = ({}); i--; )
    // erstmal viele Texte sammeln, damit man sie gescheit auswerten kann
    //    if (time() - letzte_rettungen[i][1] > 3504000)
    //        letzte_rettungen = arr_delete (letzte_rettungen, i);
    //    else
        {
            s = letzte_rettungen [i][2];
            nam = letzte_rettungen [i][0];
            // sobald convert rettungsgrund gut funktioniert kann folgende Zeile raus:
            res += ({"",s});
            s = convert_rettungsgrund (nam, s);
            res += ({wrap_say (shortvtimestr(time_to_vtime(letzte_rettungen [i][1]),1,2)+":",s,
                0,10)[0..<2]});
        }
    return ({"Zu bejubelnde Rettungen des letzten Jahres:"}) + res;
}

string *query_letzte_rettungen ()
{
    string *res, s;
    int i;
    if ((!letzte_rettungen) || (!sizeof (letzte_rettungen)))
        return ({"Es ist schon lange niemand mehr gerettet worden."}); 
    for (i = sizeof (letzte_rettungen), res = ({}); i--; )
        if (time() - letzte_rettungen[i][1] > 3504000)
            letzte_rettungen = arr_delete (letzte_rettungen, i);
        else
        {
            s = letzte_rettungen [i][2];
            res += ({wrap_say (shortvtimestr(time_to_vtime(letzte_rettungen [i][1]),1,2)+":",s,
                0,10)[0..<2]});
        }
    return ({"Zu bejubelnde Rettungen des letzten Jahres:"}) + res;
}

void shout ()
{
    object ob;
    string nam;
    if (!playerp (ob = previous_object())) return;
    nam = ob->query_real_name ();
    if (!brueller_namen)
        brueller_namen = ([]);
    if (member (brueller_namen, nam))
        brueller_namen [nam]++;
    else
        brueller_namen [nam] = 1;
    if (wizp (ob))
        brueller_goetter++;
    else if (hlpp (ob))
        brueller_engel++;
    else brueller_spieler++;
}

int sort_shout_statistik (string a, string b)
{
    return (brueller_namen[a] < brueller_namen[b]) ||
           (brueller_namen[a] == brueller_namen[b] && a > b);
}

string* query_brueller_statistik ()
{
    mapping bruell;
    string *res;
    int i, alt, neu;
    if (!brueller_namen || !sizeof (brueller_namen))
        return (({"Keine Einträge in der Brüller - Statistik"}));
        
    bruell = copy (brueller_namen);
    if(sizeof(bruell)>100)
	bruell = filter(bruell, #'filter_mapping_verkleinerer, 
			sort_array(m_values(bruell), #'<)[100]);

    res = sort_array (m_indices (bruell), #'sort_shout_statistik);
    alt = 0;
    for (i = 0; i < sizeof (res); i++) {
        neu = brueller_namen[res[i]];
        if (neu != alt) {
            res[i] = right (neu,10)
               + " Brüller: "+capitalize(res[i]);
            alt = neu;
        } else
            res[i] = right (" ",10)
               + "           "+capitalize(res[i]);
    }
    return ({
        "Die meisten Brüller seit "+shorttimestr(bruell_statistik_time,1,2)
        +" fallen auf:",
        "Spieler: "+brueller_spieler,
        "Engel: "+brueller_engel,
        "Götter: "+brueller_goetter,
        "",
        "Die größten Brüllaffen seit "
        +shorttimestr(bruell_statistik_time,1,2)+":",
        })+res;
}

#ifdef TestMUD
#pragma no_warn_unused_variables
#endif

