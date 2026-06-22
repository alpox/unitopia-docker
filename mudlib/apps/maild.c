// This file is part of UNItopia Mudlib.
// Copyright 1993 by Francis, Freaky, Garthan. All rights reserved.
// ----------------------------------------------------------------
// File:        /apps/maild.c
// Description: Postserver
// Author:      Freaky  (16.01.93)

// UID: Root

// Modified by: Garthan (13.12.94) erase_mailfolder() fuer suicid
// Modified by: Garthan (09.02.95) mailtarget admin
// Modified by: Garthan (27.02.95) major rewrite, neues Mailformat
// Modified by: Freaky  (14.03.95) UDP-Mail mit CC
// Modified by: Freaky  (18.05.95) SECURE verbessert
// Modified by: Sissi   (13.10.95) SECURE verbessert
// Modified by: Garthan (25.03.96) aliases auch von aussen erreichbar
//		Freaky  (10.04.96) Internet Mailer
//		Freaky  (30.11.1999) emails eingefuehrt wegen den Aliases
//		Freaky  (28.04.2000) send_to_one: check ob Spieler existiert

inherit "/i/tools/mail";

#pragma strong_types

#include <level.h>
#include <udp.h>
#include <mail.h>
#include <config.h>
#include <uids.h>
#include <error_db.h>
#include <apps.h>
#include <error.h>

// Wo sind die Postfiles zu finden?
#define POSTFILE(x) ("/var/spool/mail/" + x[0..0] + "/" + x)
#define POSTFILE_SUICID_LOCATION "/var/spool/mail/suicid"

// Security-Abfrage (muss mit DEFINE gemacht werden,
// da sonst das extern_call() nicht klappt)
#define SECURE (!extern_call() || this_interactive() && \
		this_interactive()==this_player() && \
		(!strstr(object_name(previous_object()),"/obj/mailreader") || \
		 !strstr(object_name(previous_object()),LOGIN_OB)) || \
		 previous_object() == this_object() \
		)

#define RESTRICTED_INTERNET     (time() >= 1230764400)

// fuer restore_object und save_object der mailfiles.
mapping mail;  // einzige nicht static var

// Die System-Aliases
static mapping aliases;
// alle E-Mail-Adressen, an die jeder schicken darf
static string *emails;

// Format des Mailmappings:
//
// mapping mail = ([ string folder1: mixed *msgs, ... ])
//
// mixed *msgs  = ({ mixed *msg1, mixed *msg2, ..., mixed *msgN });
//
// mixed *msgN  = ({ string from, string to, string *cc,
//                   string subject, int date, int flag, string text })

// Der maild schickt die neuen Mails in den Folder DEFFOLDER.

void create()
{
    int i, j;
    string *domains, *ind, *tmp;

    aliases = read_alias(SYSTEM_ALIASES);
    emails = ({});
    ind = m_indices(aliases);
    for (i = sizeof(ind); i--; )
    {
	tmp = aliases[ind[i]];
	for (j = sizeof(tmp); j--; )
	    if (member(tmp[j],'@') >= 0)
		emails += ({ lower_case(tmp[j]) });
    }
    for (i = sizeof(domains = DOMAIN_INFOS->query_domains()); i--; )
    {
	aliases["l."+lower_case(domains[i])] = ({ "$lords("+domains[i]+")" });
	aliases["m."+lower_case(domains[i])] = ({ "$members("+domains[i]+")" });
    }
}

// Liefert alle System-Aliases des Mailservers
varargs mapping query_aliases(int which)
{
    if(which)
	return m_reallocate(filter(aliases, (: $2[1] == $3 :), which-1),1);
    else
        return m_reallocate(aliases,1);
}

// Liefert alle bekannten E-Mail-Adressen, an die jeder schicken darf
string *query_emails()
{
    return copy(emails);
}

// ------------- feed mailreader -----------------

// Liefert die groesse des Post-Files von this_interactive()
int query_post_size()
{
    string name;

    if (this_interactive())
    {
	name = this_interactive()->query_real_name();
	return file_size(POSTFILE(name) + ".o");
    }
}

// liefert die Post von this_interactive()
mixed query_post()
{
    string name;

    if (!SECURE)
	return "maild: Du bekommst keinen Zugriff auf die Post!\n";

    name = this_interactive()->query_real_name();
    mail = 0;
    restore_object(POSTFILE(name));
    if (!mappingp(mail))
	mail = ([ ]);
    if (!member(mail, DEFFOLDER))
	mail[DEFFOLDER] = ({});
    return mail;
}

// Loescht alle als deleted markierten Briefe aus dem Post-Mapping
mapping cleanup_post(mapping post)
{
    int i, j;
    string *idxs;

    for (i = sizeof(idxs = m_indices(post)); i--; )
    {
	for (j = sizeof(post[idxs[i]]); j--; )
	    if (post[idxs[i]][j][MM_FLAG] & MM_DEL)
		post[idxs[i]][j] = 0;
	    else
		post[idxs[i]][j][MM_FLAG] |= MM_OLD;
	post[idxs[i]] -= ({ 0 });
    }
    return post;
}


// Speichert die Post fuer this_interactive ab.
string save_post(mapping post)
{
    string name;

    if (!SECURE)
	return "maild: Du bekommst keinen Zugriff auf die Post!\n";
    name = this_interactive()->query_real_name();
    mail = 0;
    if (!mappingp(post))
	return "Keine Post übergeben.\n";
    post = cleanup_post(post);
    if (!sizeof(post[DEFFOLDER]))
	m_delete(post, DEFFOLDER);
    if (!sizeof(post))
	rm(POSTFILE(name) + ".o");
    else
    {
	mail = post;
	save_object(POSTFILE(name));
    }
    mail = 0;
}

// ------------- sendmail -----------------

// Ueberprueft, ob die Mailadresse aufloesbar/gueltig ist.
string known_mud_player(string name)
{
    string mud, player, expmud, tmp;

    if (!stringp(name))
	return "Fehler im Namen.\n";
    tmp = lower_case(name);
    if (sscanf(name,"%s@%s",player,mud) == 2)
    {
	if (member(mud,'.') >= 0)
	{
	    if (!INTERNET_MAILD->valid_internet_mail_address(tmp))
		return "Illegale Email-Adresse.\n";
	    if (this_interactive() && member(emails,name) < 0 &&
		    !INTERNET_MAILD->valid_internet_mail_user(
			this_interactive()->query_real_name(),tmp))
		return "Du darfst keine Email ins Internet verschicken.\n";
	    return 0;
	}
	if (!expmud = INETD->known_mud(mud))
	    return "Unbekanntes Mud '" + mud + "'\n";
	if (expmud == MUD_NAME)
	    name = player;
	else
	    name = player + "@" + expmud;
	return 0;
    }
    if (!player_exists(tmp) &&
        !member(aliases, tmp) &&
	!(player=GROUP_MASTER->adjust_group_case(regreplace(tmp,"\\.",":",1))))
	return "Spieler '" + tmp + "' ist in " + MUD_NAME + " unbekannt.\n";
    if(player)
	name = player;
    if (member(GUEST_NAMES, tmp) >= 0)
	return "An " + MUD_NAME +"-Gäste kannst du keine Briefe schreiben.\n";
}

// Speichert die Mail 'msg' im Folder 'folder' fuer User 'to' ab.
private void save_mail(string to, mixed *msg, string folder)
{
    mail = 0;
    restore_object(POSTFILE(to));

    if (!mappingp(mail))
	mail = ([ ]);
    if (!member(mail, folder))
	mail[folder] = ({});

    mail[folder] += ({ msg });
    save_object(POSTFILE(to));
    mail = 0;
}

// ruft save_mail auf und benachrichtigt den Spieler ueber die neue Mail.
private void internal_mail(string to, mixed *msg, string folder)
{
    object player, reader;

    save_mail(to, msg, folder);
    if (player = find_player(to))
    {
	if (reader = present("mailreader", player))
	    reader->neue_post(msg, folder);
	tell_object(player,"Du hast Post von " + capitalize(msg[MM_FROM]) +
		" bekommen.\n");
    }
}

// Schickt die Mail 'msg' an den User 'to'
private void send_to_one(string to, mixed *msg, int forw_or_answer)
{
    string who, mud, forward;
    object player;

    if(sizeof(msg) < MM_SIZE)
	msg += ({ 0 }) * (MM_SIZE - sizeof(msg));

    to = lower_case(to);
    // Soll die Mail an ein anderes Mud ?
    if (sscanf(to , "%s@%s", who, mud) == 2)
    {
	if (member(mud,'.')>=0)
	    INTERNET_MAILD->deliver_mail_new(to,msg[MM_TO],msg[MM_FROM],msg[MM_CC],
		    msg[MM_SUBJECT],msg[MM_TEXT],forw_or_answer, msg[MM_HEADER]);
	else
	    UDP_MAILD->deliver_mail(who, mud, msg[MM_FROM], msg[MM_CC],
		    msg[MM_SUBJECT],msg[MM_TEXT]);
    }
    else
    {
#if __VERSION__ > "3.5.2"
	bytes buf;
#endif
	if (!player_exists(to))
	{
	    do_error2(
		sprintf("send_to_one: '%s' ist kein Spieler, wurde aber als Empfänger angegeben.\n"
			"forward or answer %d\n"
			"From: %O\nTo: %O\nCc: %O\n",
			    to, forw_or_answer, msg[MM_FROM], msg[MM_TO], msg[MM_CC]),
		__FILE__, object_name(), __LINE__);
	    return;
	}
#if __VERSION__ > "3.5.2"
	if ((buf = (read_bytes("/w/" + to + "/.forward",0,80) ||
		   read_bytes("/w/" + to + "/priv/.forward",0,80))) &&
	    !catch(forward = to_text(buf, "ascii");reserve 10000))
#else
	if (forward = (read_bytes("/w/" + to + "/.forward",0,80) ||
		       read_bytes("/w/" + to + "/priv/.forward",0,80)))
#endif
	{
	    // wenn es sich um eine eMail aus einem anderen Mud handelt,
	    // die geforwardet werden soll, so muss das @-Zeichen ersetzt
	    // werden, sonst kommt die Mail nie an.
	    // Dies tut aber INTERNET_MAILD fuer uns.
	    send_to_one(explode(forward,"\n")[0],msg,1);
	    if (player = find_player(to))
		tell_object(player,wrap("Ein Brief von " +
			    capitalize(msg[MM_FROM]) +
			    ((stringp(msg[MM_SUBJECT]) &&
			      strlen(msg[MM_SUBJECT])) ? " mit dem Titel: \"" +
			     msg[MM_SUBJECT]+"\"":"")+" wurde von der Post "
			    "an Dich weitergeleitet."));
	}
	else
	    internal_mail(to, msg, DEFFOLDER);
    }
}

private varargs string *expand_aliases(mixed address, string writer)
{
    return GROUP_MASTER->expand_groups(
	map(recurse_aliases(address, aliases, writer, 0),
	function string(string addr)
	{
	    return SECOND->is_special(addr) || addr;
	}), 7);
}

// Schickt die Mail 'msg' ab. (wird von /obj/mailreader aufgerufen)
// Falls answerto an Array ist, so darf der Spieler an diese
// Adressen mailen, auch wenn es Internet-Email-Adressen sind.
mixed send_mail(mixed *msg, string *answerto)
{
    // *msg = ({ from, to, cc, subject, 0, 0, text })

    string *cc, *verts;
    mixed *expanded;
    int i;

    if (!SECURE)
	return "maild: So darfst Du keine Post schicken!\n";
    if (extern_call())
	msg[MM_FROM] = this_interactive()->query_real_name();
    if (!pointerp(msg) || sizeof(msg) != MM_MIN_SIZE)
	return "Ein gültiger Brief besteht aus mindestens " + MM_MIN_SIZE + " Einträgen.\n";
    if (!stringp(msg[MM_TO]))
	return "Fehler in der Adresse.\n";
    if (!stringp(msg[MM_FROM]))
	return "Fehler im Absender.\n";
    if (!stringp(msg[MM_SUBJECT]))
	return "Fehler im Titel.\n";
    msg[MM_SUBJECT] = implode(explode(msg[MM_SUBJECT],"\n")," ")[0..65];
    if (!stringp(msg[MM_TEXT]))
	return "Fehler im Text.\n";
    if ((cc = msg[MM_CC]) && !pointerp(cc))
	return "Fehler im Verteiler.\n";

    /*
    if (cc)
	cc -= ({ msg[MM_TO] });
    */

    verts = ({});
    for (i = 0; i < sizeof(cc); i++)
    {
	if (!stringp(cc[i]))
	    return "Fehler im Verteiler.\n";
	if (known_mud_player(&(cc[i])))
	    continue;
	if (member(verts, cc[i]) < 0)
	    verts += ({ cc[i] });
    }

    verts -= ({ msg[MM_TO] });
    msg[MM_CC] = sizeof(verts) ? verts : 0;
    verts = ({ msg[MM_TO] }) + verts;

    msg[MM_DATE] = time();
    msg[MM_FLAG] = 0;

    expanded = ({});
    for (i = sizeof(verts); i--; )
	expanded = expanded + (expand_aliases(verts[i]) - expanded);

    for (i = sizeof(answerto); i--; )
	answerto += expand_aliases(answerto[i]) - answerto;

    for (i = 0; i < sizeof(expanded); i++)
	send_to_one(expanded[i], msg,
	    RESTRICTED_INTERNET?0:
	    (pointerp(answerto) && (member(answerto, expanded[i])>=0)));

/*    for (i = 0; i < sizeof(verts); i++)
	send_to_one(verts[i], msg,
	    pointerp(answerto) && (member(answerto, verts[i])>=0));
*/
    return verts;
}

void send_to_admin(string betreff, string text)
{
    if(!extern_call() || object_name(previous_object())!=MASTER_OB)
	return;

#ifdef UNItopia
    send_mail( ({
	"Master",	// MM_FROM
	"mudadm",	// MM_TO
	({}),		// MM_CC
	"[Master-Ob] "+betreff,	// MM_SUBJECT
	0,		// MM_DATE
	0,		// MM_FLAGS
	text		// MM_TEXT
    }), 0);
#endif
}

// ------------- neue post -----------------

private mixed *new_mail(string name)
{
    int mask, i, j, n, anz, neu;
    string *folders, *where;
    mixed *msgs;

    where = ({});
    if (stringp(name))
    {
	mail = 0;
	restore_object(POSTFILE(name));
	if (mappingp(mail))
	{
	    mask = MM_DEL | MM_READ | MM_OLD;
	    for (i = sizeof(folders = m_indices(mail)); i--;)
	    {
		for (n = 0, j = sizeof(msgs = mail[folders[i]]); j--;)
		    if (!(msgs[j][MM_FLAG] & mask))
			n++;
		if (n)
		    where += folders[i..i];
		neu += n;
		anz += sizeof(msgs);
	    }
	}
	mail = 0;
    }
    return ({ anz, neu, where });
}

// wird von /secure/udp/finger.c aufgerufen
int unread_mail(string name)
{
    return new_mail(name)[1] != 0;
}

private string z(int i)
{
    if (i >= 1 && i <= 12)
	return ({0,"einen", "zwei", "drei", "vier", "fünf", "sechs",
		"sieben", "acht", "neun", "zehn", "elf", "zwölf"})[i];
    return to_string(i);
}

// wird von /secure/obj/login.c aufgerufen.
int neue_post(int msg)
{
    mixed *nm;
    int weit;

    if (!SECURE)
    {
	write("maild: So geht das aber nicht.\n");
	return -1;
    }

    nm = new_mail(this_interactive()->query_real_name());

    if (msg)
    {
	if (nm[0])
	{
	    weit = nm[0] - nm[1];
	    write("\nDu hast " +
		    (nm[1] ? z(nm[1]) + " NEUE" + (nm[1] == 1 ? "N" : "") :"")+
		    (nm[1] && weit ? " und " : "") +
		    (weit ? z(weit) +
		     (nm[1] ? " weitere" + (weit == 1 ? "n" : "") : "") : "") +
		    " Brief" + ((weit ? weit == 1 : nm[1] == 1) ? "" : "e") +
		    " auf der Post liegen.\n" +
		    (sizeof(nm[2]) > 1 ?
		     "Deine neuen Briefe liegen in den Fächern " +
		     klist(nm[2]) + ".\n" : "")
		    );
	}
#       ifdef UNItopia
	if(nm[1] && !this_player()->query_wiz_level()) {
	    string post = "Ein Postamt befindet sich in Tadmor.\n";
	    mixed start = this_player()->query_start_room();
	    if (start == 0) start = "";
            if (pointerp(start)) start = implode(start,"");
	    if (strstr(start,"/d/Doerrland") >= 0)
		post = "Ein Postamt befindet sich in Dörrstadt.\n";
	    else if (strstr(start,"/d/Kokosinsel") >= 0)
		post = "Ein Postamt befindet sich in Knossos.\n";
	    else if (strstr(start,"/d/Gallien") >= 0)
		post = "Ein Postamt befindet sich in Lutetia.\n";
	    else if (strstr(start,"/d/Maerchenland") >= 0)
		post = "Ein Postamt befindet sich in Koboldingen.\n";
	    else if (strstr(start,"/d/Arktis") >= 0)
		post = "Ein Postamt befindet sich in Phexcaer.\n";
	    else if (strstr(start,"/d/Midgard") >= 0)
		post = "Ein Postamt befindet sich in Fabeln.\n";
	    else if (strstr(start,"/d/Ebenen") >= 0)
		post = "Ein Postamt befindet sich in Foo-Ling-Yoo.\n";
            write(post);
        }
#       endif
	if (query_post_size() > MAILSIZE_WARN)
	    write("Dein Postfile ist zu groß, Du musst Briefe löschen!\n");
	write("\n");
    }

    return nm[1];
}

// --------------- udp mail -------------------

#define FAIL_UDP_MAIL(x) { \
	sys_log("Mail", \
		sprintf("UDP_MAIL: From:%O: To:%O: Subject:%O: Err:%s", \
			    from,to,subject,x)); return; }

// called by /secure/udp/mail
// called by /secure/udp/email
int query_recipient_ok(string name)
{
    int i;
    string *expanded;

    for (i = sizeof(expanded = expand_aliases(name)); i--;)
	if (!player_exists(expanded[i]))
	    return 0;
    return 1;
}

// wird von
//   /secure/udp/mail::udp_mail(), (Mails von anderen MUDs)
//   /secure/udp/udp_mail::udp_reply(), (unknown player im anderen MUD)
//   /secure/udp/udp_mail::retry_send(), (Wenn es nicht geklappt hat zu senden)
//   /secure/udp/udp_mail::failed_to_deliver(), ( "" )
//   /secure/udp/email::udp_email() (Mails von aussen)
//   /secure/rpc/mail::receivemail() (Mails von aussen)
// aufgerufen.
void deliver_mail(mixed to, string from, string *cc, string subject,
	string mail_body, string real_to, string inet_header)
{
    int i;
    string *expanded;

    if (previous_object() &&
	    (object_name(previous_object()) == UDP_MAILD ||
	     object_name(previous_object()) == "/secure/udp/email" ||
	     object_name(previous_object()) == "/secure/udp/mail" ||
	     object_name(previous_object()) == "/secure/dbus/mail" ||
	     object_name(previous_object()) == "/secure/rpc/mail"))
    {
	if (!stringp(to) && !pointerp(to))
	    FAIL_UDP_MAIL("Fehler in der Adresse.\n");
	if (!stringp(from))
	    FAIL_UDP_MAIL("Fehler im Absender.\n");
	if (!stringp(mail_body))
	    FAIL_UDP_MAIL("Fehler im Text.\n");
	if (cc && !pointerp(cc))
	    FAIL_UDP_MAIL("Fehler im Verteiler.\n");
	if (!stringp(subject))
	    subject = "<none>";
	subject = implode(explode(subject,"\n")," ")[0..65];

	if (stringp(to))
	{
	    real_to ||= to;
	    to = ({ to });
	}
	if (!cc)
	    cc = ({});
	expanded = ({});
	for (i = sizeof(to); i--; )
	    expanded = expanded + (expand_aliases(to[i]) - expanded);
	for (i = sizeof(expanded); i--;)
	    send_to_one(expanded[i],
		    ({  from,
			real_to,
			cc,
			subject,
			time(),
			0,
			mail_body,
			inet_header }), 0 );
    }
}


// -------------  suicid ----------------------

int erase_mailfolder(string name)
{
    string file;

    if (previous_object() && geteuid(previous_object()) == ROOT_UID &&
	    stringp(name) && name != "")
    {
	file = POSTFILE(name) + ".o";
	if (file_size(file) != -1)
	    return !rename(file,
		    POSTFILE_SUICID_LOCATION + "/" + name + "_" + time()+".o");
	else
	    return 1;
    }
}


// ------------- GEBETE -----------------------

#ifdef UNItopia
void write_gebet(string name, string str, string from)
{
    if (previous_object() &&
	    !strstr(object_name(previous_object()),
	    "/d/Ebenen/Asia/KlosterBabba/obj/muehle"))
	internal_mail(name,
		({ from, name, 0, "Gebet von " + capitalize(from),
		   time(), 0, str}), "Gebete");
}
#endif

// ------------- zfehler -----------------------


private string *zfehler_remap(string *cc)
{
    int i;

    // lebenswichtig sonst zerbroeselt's die Fehlerdatenbank
    cc = copy(cc||({}));

    for (i = sizeof(cc); i--; )
	if (cc[i] == "Root") // Root
	    cc[i] = "mudadm";
    return cc;
}

private string zfehler_title(int art, string from)
{
    switch(art)
    {
	case EDB_FEHLER:
	    return "Fehlermeldung von " + from;
	case EDB_IDEE:
	    return "Idee von " + from;
	case EDB_RUNTIME:
	    return "Runtimefehler";
	case EDB_COMPILE:
	    return "Compilefehler";
	case EDB_LOB:
	    return "Würdigung von " + from;
	case EDB_TYPO:
	    return "Typo von " + from;
    case EDB_ARCHIVE:
        return "Archivierter Fehler";
    case EDB_ERRLIST:
        return "Fehlerzusammenfassung";
	default:
	    return "Fehler";
    }
}

void zfehler(string from, string *cc, int date, string text, int art)
{
    string to;

    if (this_interactive() && this_interactive() == this_player() &&
	    geteuid(previous_object()) == geteuid(this_player()))
    {
	to = this_interactive()->query_real_name();
	send_to_one(to,
		({ from, to, zfehler_remap(cc) - ({ to }),
		   "[FDB] "+zfehler_title(art, capitalize(from)),
		   date, 0, text }), 0);
    }
}

// ------------- newsreader -----------------------

void send_news(string from, int date, string subject, string txt)
{
    if (previous_object() &&
        !strstr(object_name(previous_object()),"/obj/newsreader") &&
        this_interactive() && this_interactive()==this_player())
    {
        string to=this_player()->query_real_name();
        send_to_one(to,
            ({from, to, 0, subject, date, 0, txt}),0);
    }
}

void prepare_renewal() {}
void finish_renewal(object neu) {}
void abort_renewal() {}
