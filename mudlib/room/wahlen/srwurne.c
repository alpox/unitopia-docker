// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /room/wahlen/srwurne.c
// Description: Urne fuer die Wahl des Spielerrats
// Author:      Sissi

inherit "/i/object/urne";

#include <level.h>
#include <config.h>
#include <apps.h>
#include <time.h>
#include <more.h>

#include <srwahl.h>

#pragma no_clone
// dont clone me!

#ifndef SRWAHL_NEU_IST_AKTIV
// Januar - November ist Wahlende
#define WAHLENDE (timearray(time())[TM_MON]!=12)
#else
// TODO Texte auf Dezember, ggf über SR_WAHLRAUM Text bestimmen.
#define WAHLENDE (SR_WAHLRAUM.get_wahl_flag(SRWAHL_WAHL)==0)
#endif

void create()
{
   ::create();
   set_short ("Die Urne für die Wahl des Spielerrates");
   set_long(
      "Diese Wahlurne dient zur Wahl des Spielerrates. "
      "Um Deine Stimme abzugeben lies die Listen.");
}

varargs static mixed may_add_topic (object who, string subject, mixed subject_data, string topic)
{
   string name = lower_case(topic);
   
   if (adminp(who))
       return 1;
   if (WAHLENDE)
       return "Die Wahl läuft nur während des Dezembers.";
   if (who->query_age() < 432000)
       return "Du bist noch zu jung zum wählen.";
   if (who->query_stat(0)+who->query_stat(1)+who->query_stat(2)+
       who->query_stat(3)<140)
       return "Deine Fähigkeiten reichen noch nicht aus zum Wählen.";
   if (!player_exists(name))
       return "Es existiert kein Spieler diesen Namens.";
   if ("/secure/obj/login"->guest(name))
       return "Gäste können nicht in den Spielerrat gewählt werden.";
   if ("/apps/goetter_register"->is_wiz(name))
       return "Götter können nicht in den Spielerrat gewählt werden. "
           "Sollte es sich bei "+capitalize(name)+" nicht um einen Gott handeln, wende Dich an einen Admin.";
   if (adminp(who))
       return 1;
   if (wizp (who))
       return "Götter dürfen auf die Wahl keinen Einfluss nehmen.";
   if (testplayerp(name))
       return capitalize(name)+" kann nicht in den Spielerrat gewählt werden.";

   foreach(mixed *kand: query_topics(member(map(query_subjects(),#'[,0),subject))) //'))))
       if(lower_case(kand[0])==name)
           return "Der Kandidat steht bereits auf der Liste.";

   return 1;
}

static void do_add_topic(closure callback, object who, string subject, mixed subject_data, string topic)
{
    int tdata = !adminp(who) && who->query_real_name()!=lower_case(topic) && 1;
    
    if(tdata)
	subject_data |= 1;
	
    // Wir merken uns, ob derjenige von anderen vorgeschlagen wurde.
    funcall(callback, tdata);
}

static string print_topic(object who, string subject, mixed subject_data, string topic, mixed topic_data)
{
    return capitalize(lower_case(topic))+({""," (*)"," (!)"})[topic_data];
}

static string subject_info(object who, string subject, mixed subject_data)
{
    if(!subject_data)
	return 0;

    return "\n"+
	((subject_data&1)?"(*): Der Kandidat hat sich noch "
			  "nicht selbst zur Kandidatur geäußert.\n":"")+
	((subject_data&2)?"(!): Der Kandidat hat angekündigt, "
	                  "die Wahl *nicht* anzunehmen.\n":"");
}

varargs static mixed may_add_subject (object who, string subject)
{
   return adminp (who);
}

varargs static int may_delete(object who)
{
   return adminp(who);
}

varargs static int may_delete_empty_subject(object who)
{
   return adminp(who);
}

static int geheim ()
{
    if (this_interactive() && adminp (this_interactive())) return 0;
    if (WAHLENDE)
        return 1;
    return 2;
}

varargs static int may_vote(object who, string subject, mixed subject_data, string topic, mixed topic_data)
{
    if (WAHLENDE) {
        write ("Die Wahl läuft nur während des Dezembers.\n");
        return 0;
    }
#ifdef Orbit
    return 1;
#endif
    if (who->query_age() < 432000) {
        write ("Du bist noch zu jung zum wählen.\n");
        return 0;
    }
    if (who->query_stat(0)+who->query_stat(1)+who->query_stat(2)+
        who->query_stat(3)<140) {
        write ("Deine Fähigkeiten reichen noch nicht aus zum Wählen.\n");
        return 0;
    }
    return !wizp (who);
}

static int topic_action(object who, string cmd, string rest, string subject, mixed subjectdata, string topic, mixed topic_data)
{
    switch(cmd)
    {
	case "K":
	    if(!adminp(who) && who->query_real_name()!=lower_case(topic))
	    {
		write("Du darfst nur deine eigene Kandidatur bestätigen.\n");
		return NOTHING;
	    }
	    topic_data = 0;
	    return END_MORE;
	    
	case "K!":
	    if(!adminp(who) && who->query_real_name()!=lower_case(topic))
	    {
		write("Du darfst nur deine eigene Kandidatur ablehnen.\n");
		return NOTHING;
	    }
	    topic_data = 2;
	    subjectdata |= 2;
	    return END_MORE;

	case "K*":
	    if(adminp(who))
	    {
		topic_data = 1;
		subjectdata |= 1;
		return END_MORE;
	    }
    }
    return CONTINUE;
}

static string subject_help(object who, string subject, mixed subjectdata)
{
    return
"K <nr>       Die eigene Kandidatur bestätigen.\n"
"K! <nr>      Die eigene Kandidatur ablehnen.\n";
}

static void check_listen(int renew = 0)
{
    int year;
    int found = -1;
    mixed* subjects;

    if(!renew && WAHLENDE)
        return;

    year = timearray(time())[TM_YEAR];
    subjects = query_subjects();
    
    for(int i=0;i<sizeof(subjects);i++)
	if(timearray(subjects[i][3])[TM_YEAR] == year)
	{
	    found = i;
	    break;
	}

    if(found<0 || renew)
    {
	int subj;
	
	// Vorherige Wahlen rausschmeissen
	for(int i=sizeof(subjects);i--;)
	    delete_subject(0);
	
	if (!renew) year++; // Wir waehlen fuer das naechste Jahr.
	
	subj = add_subject(
	    "Wahl des Spielerrates für das Jahr "+year,
	    "Hier kannst Du die Leute in den Spielerrat für das Jahr "+year+
	    " wählen. Näheres hierzu findest Du an den Newsbrettern.\n",
	    3, 0);
	
	foreach(string name: "/apps/spielerrat_kandidatenliste"->query_kandidaten())
	    add_topic(subj, name, 0);
    }
}
