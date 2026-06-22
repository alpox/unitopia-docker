// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /secure/dbus/news.c
// Description: RPC-Funktionen fuers Usenet
// Author:      Gnomi

inherit "/i/tools/coroutine";

#include <news.h>

#define WEB_EXPORTED_BOARDS ({ "Computer", "Flohmarkt", "Medien", "Schiffe", "Smalltalk", "Spieler", "Traegerkreis" })
#define WEB_EXCLUDED_GROUPS ({ "Laber_und_Flames" })
#define WEB_URL_PREFIX "/cgi-bin/news?news="

static void news_importnews(string* gruppen, string autor, string subject, int date,
    string mid, string* refs, string text)
{
    INEWSD->importnews(gruppen, autor, subject, date, mid, refs, text);
}

static void news_cancelnews(string *gruppen, string autor, string mid)
{
    INEWSD->cancelnews(gruppen, autor, mid);
}

static string** news_get_newsgroups(string name)
{
    return INEWSD->get_newsgroups(name);
}

private string html_escape(string text)
{
   text = implode(explode(text, ">"), "&gt;");
   text = implode(explode(text, "<"), "&lt;");
   return text;
}

private string do_tree(string brett, string gruppe, mixed entries, int nr)
{
   string res = "<li><a href = " WEB_URL_PREFIX "/" + brett + "/" + gruppe + "/" + entries[NUMMER] + ">" + html_escape(entries[TITEL]) + "</a>\n";

   nr++;
   foreach(mixed entry: entries[SUBTREE])
      res += do_tree(brett, gruppe, entry, &nr);

   return res;
}

static string news_get_boards_html()
{
    string res = "<h2>UNItopia News</h2>" 
                 "Folgende Newsbretter stehen zur Auswahl:\n"
                 "<ul>\n";

    foreach(string brett: WEB_EXPORTED_BOARDS)
        res += "  <li><a href = " WEB_URL_PREFIX "/" + brett + ">" + brett + "</a>\n";
    res += "</ul>\n";

    return res;
}

static string news_get_groups_html(string brett)
{
    string res = "<h2>UNItopia News: Brett " + html_escape(brett) + "</h2>\n";

    if (!(brett in WEB_EXPORTED_BOARDS))
        return res + "Das Newsbrett " + html_escape(brett) + " existiert nicht.\n";

    res += "Folgende Gruppen stehen zur Auswahl:\n"
           "<ul>\n";

    foreach(string gruppe: NEWSD->query_gruppen(brett) - WEB_EXCLUDED_GROUPS)
        res += "  <li><a href = " WEB_URL_PREFIX "/" + brett + "/" + gruppe + ">" + gruppe + "</a>\n";
    res += "</ul>\n";

    return res;
}

static string news_get_articles_html(string brett, string gruppe)
{
    string res = "<h2>UNItopia News: Brett " + html_escape(brett) + ", Gruppe " + html_escape(gruppe) + "</h2>\n";
    mixed *entries;
    int nr;

    if(!(brett in WEB_EXPORTED_BOARDS))
        return res + "Das Newsbrett " + html_escape(brett) + " existiert nicht.\n";
    if(gruppe in WEB_EXCLUDED_GROUPS)
        return res + "Die Gruppe " + html_escape(gruppe) + " existiert nicht.\n";

    entries = NEWSD->query_artikel(brett, gruppe);
    if(!sizeof(entries))
        return res + "Die Gruppe " + html_escape(gruppe)+" am Brett " + brett + " existiert nicht.\n";

    res += "Folgende Artikel stehen zur Auswahl:\n"
           "<pre><ul>\n";

    foreach(mixed entry: reversed(entries))
    {
        if (nr >= 200)
            break;

        res += do_tree(brett, gruppe, entry, &nr);
    }
    res += "</ul></pre>";

    return res;
}

static string news_get_article(string brett, string gruppe, int artikel)
{
    string res = "<h2>UNItopia News: Brett " + html_escape(brett) + ", Gruppe " + html_escape(gruppe) + ", Artikel " + artikel + "</h2>\n";
    string text;

    if(!(brett in WEB_EXPORTED_BOARDS))
        return res + "Das Newsbrett " + html_escape(brett) + " existiert nicht.\n";
    if(gruppe in WEB_EXCLUDED_GROUPS)
        return res + "Die Gruppe " + html_escape(gruppe) + " existiert nicht.\n";

    text = read_file(NEWSD->query_file_name(brett, gruppe, artikel));
    if (!text)
        return res + "Artikel " + artikel + " existiert nicht in der Gruppe " + html_escape(gruppe) + " am Brett " + brett + ".\n";

    return res + "<pre>\n" + html_escape(text) + "</pre>\n";
}

/* Aufrufe an externe Objekte */
void export_news(string newsgroup, string autor, string addr, string subject, int date, string mid, string* ref, string text)
{
    if(object_name(previous_object()) != INEWSD)
        return;

    dbus_call_method(function void(string errname, varargs mixed* args) {},
        "de.system.inn2", "/de/system/inn2", "de.unitopia.Inn2", "exportnews", "ssssxsass",
        newsgroup, autor, addr, subject, date, mid, ref || ({}), text);
}

void cancel_news(string mid)
{
    if(object_name(previous_object()) != INEWSD)
        return;

    dbus_call_method(function void(string errname, varargs mixed* args) {},
        "de.system.inn2", "/de/system/inn2", "de.unitopia.Inn2", "deletenews", "s", mid);
}
