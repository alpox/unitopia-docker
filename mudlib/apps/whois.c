// This file is part of UNItopia Mudlib.
// ---------------------------------------------------------------------
// File:	/apps/whois.c
// Description:	Back-end des Who-is-Who-Buchs /room/rathaus/obj/whois.c
//		Trennung aus Sicherheits- und Performancgruenden
// Author:	Sissi, Mai 1998

// UID: Root

#pragma strong_types

string *whois_liste, inhalt;

string *query_whois_liste ()
{
    return whois_liste;
}

string query_inhalt ()
{
    return inhalt;
}

string make_filename(string wer)
{
    return "/w/"+wer+"/.whois";
}

int has_page (string wer)
{
    return file_size (make_filename(wer)) > 0;
}

string query_whois (string wer)
{
    if (file_size (wer=make_filename(lower_case(wer))) > 0)
        return read_file (wer);
}

void reset ()
{
    int i;
    whois_liste = filter (get_dir("/w/"),#'has_page);
    for (inhalt = "", i=0; i < sizeof (whois_liste); i++) {
        if ((i % 4) == 0) inhalt += "\n";
        inhalt += right(to_string(i+1),4)+": "+left(capitalize(whois_liste[i]),10);
    }
}

void create ()
{
    reset();
}
