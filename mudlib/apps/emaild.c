// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/apps/emaild.c
// Description:	Mail-Daemon fuer Internet-Mails
// Author:	Freaky (10.04.96)

// UID: Apps

#include <mail.h>
#include <apps.h>
#include <level.h>
#include <erq.h>
#include <config.h>
#include <quest.h>
#include <stats.h>
#include <error.h>

#define SAVE_FILE "/var/adm/email"
#define MAIL_FILE(x) MQUEUE+"/OUT_"+x
#define E_TO     0
#define E_FROM   1
#define E_TIME   2
#define E_RESENT 3
#define E_ERROR  4
#define E_TEXT   5
#define E_HEADER 6
#define E_SIZE   7

// Ab 01.01.2009 duerfen nur noch Goetter ins Internet schicken
#define RESTRICTED_INTERNET	(time() >= 1230764400)
#define LOG(x) do_sys_log("deliver_mail",sprintf("%s\nFrom: %O\nSendTo: %O\nTo: %O\n\n",(x),from,sendto,to))

mapping pending;
int id;

private void do_sys_log(string fun, string msg)
{
    sys_log("Mail",shorttimestr(time()) + " " + __FILE__ + "::" +
	    fun + "() " + msg);
}

static void callback(mixed msg, int id)
{
    /*
    sys_log("Mail",shorttimestr(time()) +
	    sprintf(" ID:%d MSG:%s\n",id,mixed2str(msg)));
    */
    if (msg == -1)
    	return;

    if (msg[0] == ERQ_OK)
    {
    	if (!member(pending,id))
	    do_sys_log("callback","Mail war nicht im Pending: " + id + "\n");
	else
	{
	    if (file_size(MAIL_FILE(id)) >= 0)
		do_sys_log("callback","Mail ist noch im mqueue: " + id + "\n");
	    else
		m_delete(pending,id);
	}
    }
    else
    {
        mixed *err;

    	err = pending[id,E_ERROR];
	if (!err)
	    err = ({});
	pending[id,E_ERROR] = err + ({ msg });
	do_sys_log("callback","Fehler (" + id + ") " + msg[0] + " " +
		to_string(msg[1..]) + ".\n");
    }
}

static int send_mail(int id)
{
    if(pending[id, E_HEADER])
    {
        "/secure/dbus/mail"->send_email(
            function void()
            {
                m_delete(pending, id);
                save_object(SAVE_FILE);
            },
            function void(string errmsg)
            {
                do_my_error("RPC-Fehler: "+errmsg+"\n");
            },
            pending[id, E_FROM], pending[id, E_TO], pending[id, E_HEADER], 0, 0, pending[id, E_TEXT]);
    }
    else if (file_size(MAIL_FILE(id)) > 0)
    {
	pending[id,E_RESENT]++;
#if __VERSION__ > "3.5.2"
	return send_erq(ERQ_EXECUTE,to_bytes("mailrelay "+id+" "+pending[id,E_FROM]+
		" "+pending[id,E_TO], "UTF-8"),lambda(({'x}),({#'callback, 'x, id})));
#else
	return send_erq(ERQ_EXECUTE,"mailrelay "+id+" "+pending[id,E_FROM]+
		" "+pending[id,E_TO],lambda(({'x}),({#'callback, 'x, id})));
#endif
    }
    else
	m_delete(pending,id);
}

void resend_pending()
{
    int i, *ids;

    ids = m_indices(pending);
    for (i=sizeof(ids); i--; )
    	call_out("send_mail",i*2,ids[i]);
}

void create()
{
    if (!pending)
    {
	restore_object(SAVE_FILE);
	if (!pending)
	    pending = m_allocate(0,E_SIZE);
	else
	{
	    if(widthof(pending) < E_SIZE)
		pending = m_reallocate(pending, E_SIZE);
	    resend_pending();
	}
    }
}

void reset()
{
    resend_pending();
    save_object(SAVE_FILE);
}

int remove()
{
    save_object(SAVE_FILE);
    destruct(this_object());
    return 1;
}

int valid_internet_mail_address(string str)
{
    return !strlen(lower_case(str) -
	    "abcdefghijklmnopqrstuvwxyz1234567890-_.+@");
}

int valid_internet_mail_user(string who, string to)
{
    int sum_real_stat;
    object ob;
    int qcount;
    
    if (GOETTER_REGISTER->is_wiz(who))
	return 1;
    if (member(MAILD->query_emails(),lower_case(to)) >= 0)
	return 1;

    if(RESTRICTED_INTERNET)
	return 0;

    ob = find_player(who) || find_player("MAIL "+who);
    if (hlpp(find_player(who)))
	return 1;

    // Test, ob er kein Engel sein darf.
    if(!ob || guestp(ob))
        return 0;
    if(ob->query_sum_skill() < TOTAL_EXPERIENCE)
        return 0;
	
    foreach(string quest: QUEST_ROOM->query_quests(Q_NECESSARY))
        if(!QUEST_ROOM->quest_min_solved(quest, ob))
            return 0;
    foreach(string quest: QUEST_ROOM->query_quests(Q_WAHL))
        if(QUEST_ROOM->quest_min_solved(quest, ob))
	    qcount++;
    if(qcount < NEEDED_CHOICE_QUESTS)
        return 0;

    for(int a=0; a<STAT_NUMBER; a++)
        sum_real_stat += ob->query_real_stat(a);
    if(sum_real_stat/STAT_NUMBER < 80)
        return 0;

    return 1;
}

void deliver_mail(string sendto, string to, string from, string *cc,
		string subject, string text, int no_user_check,
		string inet_header)
{
    string user, host, out, orig_from;
    int tim, i;

    if (object_name(previous_object()) != MAILD)
    	return;

    if (!valid_internet_mail_address(sendto))
    {
	write("Illegale Mailadresse angegeben.\n");
	LOG("Illegal address");
    	return;
    }

    if (!valid_internet_mail_address(from))
    {
	write("Illegale From-Adresse.\n");
	LOG("Illegal from");
    	return;
    }

    if (!no_user_check && !valid_internet_mail_user(from,sendto))
    {
	write("Du darfst keine Mail ins Internet schicken.\n");
    	return;
    }

    if (i = sizeof(cc))
    {
	// Kopieren, sonst wird es fuer die anderen Mails veraendert :(
	cc = ({}) + cc;
	for (; i--; )
	    if (sscanf(cc[i],"%s@%s",user,host) != 2)
		cc[i] = regreplace(cc[i],":",".",1) + "@" HOST_NAME;
	    else if (member(host,'.') < 0)
		cc[i] = user + "%" + host + "@" HOST_NAME;
    }

    orig_from = from;

    if (sscanf(from,"%s@%s",user,host) != 2)
	from += "@" HOST_NAME " ("+capitalize(from)+")";
    else if (member(host,'.') < 0)
	from = user + "%" + host + "@" HOST_NAME " ("+
		capitalize(from)+")";

    if (!sizeof(to))
        to = 0;
    else if (sscanf(to,"%s@%s",user,host) != 2)
	to = regreplace(to,":",".",1) + "@" HOST_NAME;
    else if (member(host,'.') < 0)
	to = user + "%" + host + "@" HOST_NAME;

    out = (inet_header||"") + "Subject: " + subject + (to ? "\nTo: " + to : "") + "\n";
    if (sizeof(cc))
    {
	int len;
	
	out += "Cc: " + cc[0];
	i = 1;
	len = 4 + strlen(cc[0]);
	
	while(i<sizeof(cc))
	{
	    if(len+strlen(cc[i])<77)
	    {
		out += ", " + cc[i];
		len += 2 + strlen(cc[i]);
	    }
	    else
	    {
		out += ",\n    " + cc[i];
		len  = 4 + strlen(cc[i]);
	    }
	    i++;
	}
	out +="\n";
	
	// sprintf bricht bei zu langen Adressen auch die Adressen um.
	// out += sprintf("%s%=-70s\n","CC: ",implode(cc,", "));
    }

    out += "From: " + from + "\n\n" + text;
    do
    {
    	id++;
    }
    while(file_size(MAIL_FILE(id)) >= 0);

    write_file(MAIL_FILE(id),out+"\n");
    tim = time();
    pending[id,E_TIME] = tim;
    pending[id,E_TO] = sendto;
    // Das gehoert so. Sonst gibt es Aerger mit dem ERQ! (Freaky)
    pending[id,E_FROM] = orig_from;
    // pending[id,E_FROM] = from;
    // Hier speichern, damit nichts verloren geht
    save_object(SAVE_FILE);
    if (!send_mail(id))
    {
	MASTER_OB->stale_erq();
	if (!send_mail(id))
	{
	    LOG("Mail konnte nicht verschickt werden. (ERQ-Problem)\n"
		    "Wird später nochmal probiert.");
	    write("Konnte Mail nicht an " + capitalize(to) + " schicken.\n"
	    	  "Es wird versucht die Mail später zu schicken.\n");
	}
    }
}

void deliver_mail_new(string sendto, string to, string from, string *cc,
		string subject, string text, int no_user_check,
		string inet_header)
{
    string user, host, orig_from;
    int i, intermud;

    if (object_name(previous_object()) != MAILD)
    	return;

    if (!valid_internet_mail_address(sendto))
    {
	write("Illegale Mailadresse angegeben.\n");
	LOG("Illegal address");
    	return;
    }

    if (!valid_internet_mail_address(from))
    {
	write("Illegale From-Adresse.\n");
	LOG("Illegal from");
    	return;
    }

    if (!no_user_check && !valid_internet_mail_user(from,sendto))
    {
	write("Du darfst keine Mail ins Internet schicken.\n");
    	return;
    }

    if (i = sizeof(cc))
    {
	// Kopieren, sonst wird es fuer die anderen Mails veraendert :(
	cc = ({}) + cc;
	for (; i--; )
	    if (sscanf(cc[i],"%s@%s",user,host) != 2)
		cc[i] = regreplace(cc[i],":",".",1) + "@" HOST_NAME;
	    else if (member(host,'.') < 0)
		cc[i] = user + "%" + host + "@" HOST_NAME;
    }

    inet_header ||= "";
    inet_header += sprintf("Subject: %s\n", subject);

    orig_from = from;

    if (!sizeof(from))
    {
    }
    else if (sscanf(from,"%s@%s",user,host) != 2)
    {
        inet_header += sprintf("From: %s@" HOST_NAME " (%s)\n", from, capitalize(from));
        orig_from += "@" HOST_NAME;
    }
    else if (member(host,'.') < 0)
    {
        // InterMUD-Mails
        inet_header += sprintf("From: %s%%%s@" HOST_NAME " (%s)\n", user, host, capitalize(from));
	orig_from = user + "%" + host + "@" HOST_NAME;
	intermud = 1;
    }
    else
        inet_header += sprintf("From: %s\n", from);

    if (!sizeof(to))
    {
        // No recipient...
    }
    else if (sscanf(to,"%s@%s",user,host) != 2)
    {
        inet_header += sprintf("To: %s@" HOST_NAME "\n", regreplace(to,":",".",1));
    }
    else if (member(host,'.') < 0)
    {
        inet_header += sprintf("To: %s%%%s@" HOST_NAME "\n", user, host);
    }
    else
        inet_header += sprintf("To: %s\n", to);

    if (member(sendto, '@') < 0)
        sendto += "@" HOST_NAME;

    if(intermud)
        inet_header += "Sender: postmaster@" HOST_NAME "\n";

    if (sizeof(cc))
    {
	int len;
	
	inet_header += "Cc: " + cc[0];
	i = 1;
	len = 4 + strlen(cc[0]);
	
	while(i<sizeof(cc))
	{
	    if(len+strlen(cc[i])<77)
	    {
		inet_header += ", " + cc[i];
		len += 2 + strlen(cc[i]);
	    }
	    else
	    {
		inet_header += ",\n    " + cc[i];
		len  = 4 + strlen(cc[i]);
	    }
	    i++;
	}
	inet_header +="\n";
	
	// sprintf bricht bei zu langen Adressen auch die Adressen um.
	// out += sprintf("%s%=-70s\n","CC: ",implode(cc,", "));
    }

    do
    {
    	id++;
    }
    while(file_size(MAIL_FILE(id)) >= 0);

    pending[id,E_TIME] = time();
    pending[id,E_TO] = sendto;
    pending[id,E_FROM] = orig_from;
    pending[id,E_TEXT] = text;
    pending[id,E_HEADER] = inet_header;
    save_object(SAVE_FILE);

    "/secure/dbus/mail"->send_email(
        function void() : int my_id = id
        {
            m_delete(pending, my_id);
            save_object(SAVE_FILE);
        },
        function void(string errmsg)
        {
            do_my_error("RPC-Fehler: "+errmsg+"\n");
            LOG("Mail konnte nicht verschickt werden.\n"
                "Wird später nochmal probiert.");
            write("Konnte Mail nicht an " + capitalize(to) + " schicken.\n"
                "Es wird versucht die Mail später zu schicken.\n");
        },
        orig_from, sendto, inet_header, 0, 0, text);
}

void prepare_renewal()
{
    save_object(SAVE_FILE);
}
void finish_renewal(object neu) {}
void abort_renewal() {}
