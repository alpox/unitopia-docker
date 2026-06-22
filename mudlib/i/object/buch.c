// This file is part of UNItopia Mudlib.
// ---------------------------------------------------------------------
// File:          /i/object/buch.c
// Description:   Ein Buch
// Author:        Freaky
// Modified by:   Garthan (23.12.93)
// Major Rewrite: Sissi   (12. 5.95 - 7.1.96)
// Modified by:   Sissi   (10. 5.96): M_AUTO_END im more gesetzt
//                Sissi   (28. 6.97): more_end verbessert
//                Croft   ( 9.12.98): Auf- und Zuschlagen optional
//                                    Einige Meldungen dynamischer gemacht
//                                    seiten_desc eingebaut
//                Croft   (29.12.98)  seiten_desc erweitert
//                                    r eingebaut
//                                    Pseudoclosures erlaubt
//                Croft   ( 4. 1.99)  Bugfix in eval_entry
//                Croft   (25. 1.99)  Unsichtbare erzeugen keine Meldungen mehr
//                Croft   ( 1. 2.99)  catmore verdoppelt im moremode nicht mehr
//                                    das letzte \n, \n-Ausgabe vor
//                                    Befehlszeile verbessert
//                Croft   (22. 4.99)  saechlicher Genderfix in more_end
//                Croft   (24. 6.00)  suchen eingebaut

/*
FUNKTION: buch
DEKLARATION: zum Buch siehe /doc/funktionsweisen/buch
BESCHREIBUNG:
Wie man ein Buch macht steht ausführlich in der Datei
/doc/funktionsweisen/buch, auch mit der Enzy zu lesen unter
"menue", "funktionsweisen", "buecher".
GRUPPEN: buch
*/


#pragma save_types


virtual inherit "/i/item";
virtual inherit "/i/move";
virtual inherit "/i/value";


#include <apps.h>
#include <deklin.h>
#include <invis.h>
#include <level.h>
#include <more.h>
#include <uids.h>
#include <rtlimits.h>
#include <input_to.h>
#include <message.h>
#include <files.h>
#include <notify_fail.h>
#include <driver_info.h>
#include <buch.h>

private int current_page = -1;
private int max_page;
private string *page_names;
private string *page_names_no_ascii_art;
private string|int verzeichnis;
private string verzeichnis_no_ascii_art;
private int moremode;
private int wiz;
private int hlp;
private int open_close = 1;
private mapping seiten_desc = ([]);
private int searchable;
private string i_status_line, status_line;
private int more_status; // 1: more_end nur, weil ein neues more aktiv wurde

#define B_MODE_PAGE     0
#define B_MODE_MORE     1
#define B_MODE_MOREFILE 2

#define BUCH_MORE_ID(reader,page)		(["ID":"Mudlib - Buch", "Reader": reader, "Page": page])
#define BUCH_HELP_MORE_ID(reader,page,line)	(["ID":"Mudlib - Buch - Hilfe", "Reader": reader, "Page": page, "Line": line])
#define IS_BUCH_HELP_MORE_ID(id)		(mappingp(id) && (id)["ID"]=="Mudlib - Buch - Hilfe")
#define IS_BUCH_MORE_ID(id)			(mappingp(id) && (id)["ID"]=="Mudlib - Buch")
#define GET_PAGE_FROM_MORE_ID(id)		((id)["Page"])
#define GET_LINE_FROM_MORE_ID(id)		((id)["Line"])
#define GET_READER_FROM_MORE_ID(id)		((id)["Reader"])
#define ABS_PATH(path)				abs_path((path), extern_call() && implode(explode(object_name(previous_object()),"/")[..<2],"/"))

/*
FUNKTION: query_max_page
DEKLARATION: int query_max_page()
BESCHREIBUNG:
Liefert die Seitenzahl dieses Buches zurück.
VERWEISE: set_max_page, set_page_names
GRUPPEN: buch
*/
int query_max_page()
{
    return max_page;
}

/*
FUNKTION: set_max_page
DEKLARATION: int set_max_page(int max_page)
BESCHREIBUNG:
Damit setzt man die Seitenzahl dieses Buches.
Dies ist nur bei dynamischen Büchern erforderlich, wo die Seiten nicht
mittels set_page_names gesetzt wurden.
VERWEISE: query_max_page, set_page_names
GRUPPEN: buch
*/
int set_max_page(int x)
{
    return max_page = x;
}

/*
FUNKTION: query_current_page
DEKLARATION: int query_current_page()
BESCHREIBUNG:
Damit kann man den Status des Buches abfragen.
Ein negativer Wert zeigt an, dass dieses Buch geschlossen ist.
Bei 0 ist dieses Buch beim Inhaltsverzeichnis aufgeschlagen.
Positive Werte geben die Seitennummer an, bei welcher dieses Buch
aufgeschlagen ist.
Ist dieses Buch nicht zum Auf- und Zuschlagen, so kann trotzdem ein
negativer Wert zurückgeliefert werden, was dann das gleiche wie 0 bedeutet.
VERWEISE: set_current_page
GRUPPEN: buch
*/
int query_current_page()
{
    return current_page;
}

/*
FUNKTION: query_verzeichnis
DEKLARATION: string|int query_verzeichnis(int no_ascii_art_flag = 0)
BESCHREIBUNG:
Liefert die Informationen zum Inhaltsverzeichnis.

Bei no_ascii_art_flag == 0 wird das normale Inhaltsverzeichnis geliefert.
Folgende Ergebnisse sind möglich:
    0: Es gibt kein Inhaltsverzeichnis
    1: Das Inhaltsverzeichnis wird über query_verzeichnis_inhalt()
       abgefragt.
    String: Der Dateiname des Inhaltsverzeichnisses.

Bei no_ascii_art_flag == 1 wird das Inhaltsverzeichnis ohne ASCII-Graphiken
geliefert. Folgende Ergebnisse sind dabei möglich:
    0: Es gibt kein gesondertes Inhaltsverzeichnis ohne ASCII-Graphiken.
    String: Der Dateiname des Inhaltsverzeichnisses.

VERWEISE: set_verzeichnis, query_page_names, query_verzeichnis_inhalt
GRUPPEN: buch
*/
#if __VERSION__ > "3.6.3"
string|int query_verzeichnis(int no_ascii_art_flag = 0)
#else
varargs string|int query_verzeichnis(int no_ascii_art_flag)
#endif
{
    if(no_ascii_art_flag)
        return verzeichnis_no_ascii_art;
    else
        return verzeichnis;
}

/*
FUNKTION: query_page_names
DEKLARATION: string *query_page_names(int no_ascii_art_flag = 0)
BESCHREIBUNG:
Liefert ein Array mit den Dateinamen der einzelnen Seiten zurück bzw.,
falls keine Dateien angegeben wurden, 0.
Wenn no_ascii_art_flag == 0 ist, bekommt man die normale Version,
für no_ascii_art_flag != 0 bekommt man die Version ohne ASCII-Graphiken
(wenn dort 0 geliefert wird, gibt es keine gesonderte Version).
VERWEISE: set_page_names, query_verzeichnis, query_page_inhalt
GRUPPEN: buch
*/
#if __VERSION__ > "3.6.3"
string* query_page_names(int no_ascii_art_flag = 0)
#else
varargs string* query_page_names(int no_ascii_art_flag)
#endif
{
    if(no_ascii_art_flag)
        return page_names_no_ascii_art;
    else
        return page_names;
}

/*
FUNKTION: set_wizard_book
DEKLARATION: int set_wizard_book (int wiz)
BESCHREIBUNG:
Damit setzt man, ob nur Götter dieses Buch lesen können (wiz=1) oder
auch Nicht-Götter dies dürfen (wiz=0). Eine entsprechende
Fehlermeldung kann man mit set_seiten_desc setzen.
VERWEISE: set_hlp_book, query_wizard_book, set_seiten_desc
GRUPPEN: buch
*/
int set_wizard_book(int i)
{
    return wiz = (i ? 1 : 0);
}

/*
FUNKTION: set_hlp_book
DEKLARATION: int set_hlp_book(int hlp)
BESCHREIBUNG:
Damit setzt man, ob nur Engel und Götter dieses Buch lesen können (hlp=1)
oder ob auch normale Spieler dies dürfen (hlp=0). Eine entsprechende
Fehlermeldung kann man mit set_seiten_desc setzen.
VERWEISE: set_wizard_book, query_hlp_book, set_setien_desc
GRUPPEN: buch
*/
int set_hlp_book(int i)
{
    return hlp = (i ? 1 : 0);
}

/*
FUNKTION: query_wizard_book
DEKLARATION: int query_wizard_book()
BESCHREIBUNG:
Liefert 1, wenn nur Götter dieses Buch lesen können, ansonsten 0.
VERWEISE: set_wizard_book, query_hlp_book
GRUPPEN: buch
*/
int query_wizard_book()
{
    return wiz;
}

/*
FUNKTION: query_hlp_book
DEKLARATION: int query_hlp_book()
BESCHREIBUNG:
Liefert 1, wenn nur Engel dieses Buch lesen können, ansonsten 0.
VERWEISE: set_hlp_book, query_wizard_book
GRUPPEN: buch
*/
int query_hlp_book ()
{
    return hlp;
}

/*
FUNKTION: set_open_close
DEKLARATION: int set_open_close(int cl)
BESCHREIBUNG:
Damit kann man angeben, ob man dieses Buch auf- und zuschlagen kann (cl=1)
oder ob das Buch automatisch aufgeschlagen ist (cl=0).
Bei letzten wird ein negativer Wert bei set_current_page wie 0 behandelt.
VERWEISE: query_open_close, set_current_page
GRUPPEN: buch
*/
int set_open_close (int i)
{
    return open_close = (i ? 1 : 0);
}

/*
FUNKTION: query_open_close
DEKLARATION: int query_open_close()
BESCHREIBUNG:
Liefert 1 zurück, wenn man dieses Buch auf- und zuschlagen kann, ansonsten 0.
VERWEISE: set_open_close
GRUPPEN: buch
*/
int query_open_close()
{
    return open_close;
}

/*
FUNKTION: set_searchable
DEKLARATION: int set_searchable(int searchable)
BESCHREIBUNG:
Damit kann man angeben, ob dieses Buch mit dem Befehl '/<suchstring>'
durchsuchbar sein soll. Falls 0 (also nicht durchsuchbar) angegeben wurde,
aber mittels set_page_mode auf more() umgeschaltet wurde, so bleibt
die Möglichkeit, innerhalb einer Seite zu suchen.
VERWEISE: query_searchable
GRUPPEN: buch
*/
int set_searchable(int i)
{
    return searchable = (i ? 1 : 0);
}

/*
FUNKTION: query_searchable
DEKLARATION: int query_searchable()
BESCHREIBUNG:
Liefert 1 zurück, wenn dieses Buch durchsuchbar ist.
VERWEISE: set_searchable
GRUPPEN: buch
*/
int query_searchable()
{
    return searchable;
}

/*
FUNKTION: set_status_line
DEKLARATION: void set_status_line(string inhaltsverzeichnis, string seite)
BESCHREIBUNG:
Damit setzt man die Statuszeile des more, falls eine Seite mittels more
ausgegeben wird. Der erste Parameter gibt die Statuszeile für das
Inhaltsverzeichnis, der zweite für alle weiteren Seiten an.
Ein %p wird (nicht beim Inhaltsverzeichnis) durch die aktuelle Seitenzahl,
das erste %d (dies aber auch beim Inhaltsverzeichnis) durch die Nummer der
aktuellen Zeile und das zweite %d durch die Anzahl an Zeilen ersetzt.
VERWEISE: query_status_line, set_page_mode, more
GRUPPEN: buch
*/
void set_status_line(string inhalt, string normal)
{
    i_status_line=inhalt;
    status_line=normal;
}

/*
FUNKTION: query_status_line
DEKLARATION: string *query_status_line()
BESCHREIBUNG:
Liefert die Statuszeile für das Inhaltsverzeichnis und alle weiteren Seiten
in einem zweielementigen Array zurück.
VERWEISE: set_status_line, query_page_mode, more
GRUPPEN: buch
*/
string *query_status_line()
{
    return ({i_status_line, status_line});
}

/*
FUNKTION: set_seiten_desc
DEKLARATION: void set_seiten_desc(mapping m)
BESCHREIBUNG:
Damit kann man den Seiten eines Buches einen anderen Namen geben und
dazu noch einige Fehlermeldungen ändern.
Folgende Einträge sind möglich:

 PAGE_DESC_NAME           Der neue Name einer Seite
 PAGE_DESC_GENDER         Das dazugehörige Geschlecht
 PAGE_DESC_IDS            IDs, welche beim Lese-Befehl erkannt werden sollen

 PAGE_DESC_NO_PAGES       Keine Seiten im Buch
 PAGE_DESC_WHICH          Es wurde nicht angegeben welche Seite man lesen will.
 PAGE_DESC_INVALID        Die erwünschte Seitennummer ist <= 0.
 PAGE_DESC_OVERFLOW       Die erwünschte Seitennummer ist zu groß.
 PAGE_DESC_NO_PREV        Es wurde versucht zu weit zurückzublättern (-).
 PAGE_DESC_NO_NEXT        Es wurde versucht zu weit vorzublättern (+).
 PAGE_DESC_READ_MSG       Meldung beim Lesen an Andere
 PAGE_DESC_READ_TOC_MSG   Meldung beim Lesen des Inhaltsverzeichnisses
 PAGE_DESC_STOP           Man hört auf zu lesen
 PAGE_DESC_STOP_MSG       Aufhörenmeldung für Andere
 PAGE_DESC_OPEN           Man öffnet das Buch
 PAGE_DESC_OPEN_MSG       Öffnenmeldung für Andere
 PAGE_DESC_CLOSE          Man schließt das Buch
 PAGE_DESC_CLOSE_MSG      Schließenmeldung für Andere
 PAGE_DESC_NOT_ALLOWED    Meldungen für Nicht-Götter bei Götterbüchern bzw.
                          Nicht-Engel bei Engelsbüchern, wenn sie dieses
                          Buch lesen wollen.
 PAGE_DESC_NOT_SEARCHABLE In dem Buch ist die Suche nicht aktiviert.
 PAGE_DESC_EMPTY_SEARCH   Der Suchstring ist leer.
 PAGE_DESC_SEARCH_BEYOND  Suche hinter Buchende nicht möglich.
 PAGE_DESC_SEARCH_TLE     Evals sind bei der Suche ausgegangen.
 PAGE_DESC_SEARCH_REACHED_LAST Ende des Buches ohne Treffer erreicht.

Bei den Fehlermeldungen dürfen Pseudoclosures verwendet werden.
'book_ob ist dabei das Buchobjekt und 'page_ob entweder das übergebene
Mapping (falls dort ein Eintrag PAGE_DESC_NAME existiert) oder eine
V-Item-Beschreibung einer Seite. 'page_nr ist die aktuelle Seitennummer.


Beispiel:

      set_seiten_desc(([
         PAGE_DESC_NAME: "abschnitt",
       PAGE_DESC_GENDER: "maennlich",
          PAGE_DESC_IDS: ({"inschrift", "absatz", "abschnitt"}),
     PAGE_DESC_NO_PAGES: "Alle Abschnitte wurden wohl von einem Vandalen "
                         "unleserlich gemacht.",
        PAGE_DESC_WHICH: "Welchen Abschnitt willst du lesen?",
      PAGE_DESC_INVALID: "Diesen Abschnitt gibt es nicht.",
     PAGE_DESC_OVERFLOW: "Soviele Abschnitte gibt es nicht.",
      PAGE_DESC_NO_PREV: "Das ist schon der erste Abschnitt.",
      PAGE_DESC_NO_NEXT: "Das ist schon der letzte Abschnitt.",
     PAGE_DESC_READ_MSG: "$Der(OBJ_TP) liest sich eine Inschrift durch.",
 PAGE_DESC_READ_TOC_MSG: "$Der(OBJ_TP) liest das Inhaltsverzeichnis.",
         PAGE_DESC_STOP: "Du hörst auf, $den('book_ob) zu studieren.",
     PAGE_DESC_STOP_MSG: "$Der(OBJ_TP) hört auf, $den('book_ob) zu studieren.",
  PAGE_DESC_NOT_ALLOWED: "Eine seltsame Schrift... "
                         "Du kannst sie nicht entziffern...",
      ]));
VERWEISE: query_seiten_desc
GRUPPEN: buch
*/
void set_seiten_desc(mapping m)
{
    seiten_desc = m || ([]);
}

/*
FUNKTION: query_seiten_desc
DEKLARATION: mapping query_seiten_desc()
BESCHREIBUNG:
Liefert die mit set_seiten_desc gesetzte Beschreibung zurück.
VERWEISE: set_seiten_desc
GRUPPEN: buch
*/
mapping query_seiten_desc()
{
    return copy(seiten_desc);
}

/*
FUNKTION: set_page_mode
DEKLARATION: void set_page_mode(string str)
BESCHREIBUNG:
Damit wird eingestellt, auf welche Art die Seiten ausgegeben werden.
Es gibt drei Modi:

    PAGE_MODE_PAGE:     Die Seite wird auf einmal ausgegeben.
    PAGE_MODE_MORE:     Die Seite wird per more() ausgegeben.
    PAGE_MODE_MOREFILE: Eine Datei wird ausgegeben. (siehe
                        query_verzeichnis_inhalt und query_page_inhalt)

Standardmäßig ist PAGE_MODE_PAGE eingestellt.
VERWEISE: query_page_mode, more, set_status_line,
          query_verzeichnis_inhalt, query_page_inhalt
GRUPPEN: buch
*/
void set_page_mode(string str)
{
    mapping map =
        ([PAGE_MODE_PAGE: B_MODE_PAGE,
          PAGE_MODE_MORE: B_MODE_MORE,
          PAGE_MODE_MOREFILE: B_MODE_MOREFILE]);
    if(!member(map,str))
    {
        raise_error(wrap("set_page_mode: unzulässiger Modus '"+str+"' gesetzt"
                         ", nur "+liste(m_indices(map))+" sind erlaubt."));
    }
    moremode = map[str];
}

/*
FUNKTION: query_page_mode
DEKLARATION: string query_page_mode()
BESCHREIBUNG:
Liefert den mit set_page_mode gesetzten Modus zurück.
VERWEISE: set_page_mode
GRUPPEN: buch
*/
string query_page_mode()
{
    switch (moremode)
    {
        case B_MODE_PAGE:
            return PAGE_MODE_PAGE;
        case B_MODE_MORE:
            return PAGE_MODE_MORE;
        case B_MODE_MOREFILE:
            return PAGE_MODE_MOREFILE;
    }
    return 0;
}


void create()
{
    set_id("buch");
    set_name("buch");
    set_gender("saechlich");
    set_long("Dies ist ein Buch. Bücher sind zum aufschlagen, lesen "
             "und wieder zumachen da.");
    set_material("papier");
    // UID setzen, damit das Buch files lesen kann. Aber nur die geringste
    // UID um keine Sicherheitslöcher zu öffnen. Fürs /w brauchen wir
    // das aber nicht, da die w-UID schon restriktiv genug ist.
    if(strstr(getuid(),"w:"))
	seteuid(NOBODY_UID);
    set_weight(1);
    current_page = -1;
    moremode = B_MODE_PAGE;
    open_close = 1;
    seiten_desc = ([]);
}


void init()
{
    if (open_close)
    {
        add_action ("oeffne",  "öffne");
        add_action ("schlag",  "schlage",-6);
        add_action ("schliess","schließe",-7);
    }
}

private string get_default_entry(string index, object leser)
{
    switch (index)
    {
        case PAGE_DESC_NO_PAGES:
            return "Es sind leider alle Seiten herausgerissen worden.";

        case PAGE_DESC_WHICH:
            return "Welche Seite willst Du lesen?";

        case PAGE_DESC_INVALID:
            return "Diese Seite gibts nicht.";

        case PAGE_DESC_OVERFLOW:
            return "Soviele Seiten "+plural("hat ","haben ")+der()+" nicht.";

        case PAGE_DESC_NO_PREV:
            return "Weiter zurück geht es nicht.";

        case PAGE_DESC_NO_NEXT:
            return "Weiter geht's nicht mehr, "+der()+" "+ist()+" zu Ende.";

        case PAGE_DESC_NOT_ALLOWED:
            return "Eine seltsame Schrift... Du kannst sie nicht entziffern...";

        case PAGE_DESC_READ_MSG:
            return Der(leser)+plural(" liest in ", " lesen in ", leser)+seinem()+".";

        case PAGE_DESC_READ_TOC_MSG:
            return Der(leser)+plural(" liest ", " lesen ", leser)
                             +ihren((["name":"inhaltsverzeichnis", "gender":"saechlich"]),0,this_object())+".";

        case PAGE_DESC_STOP:
            return "Du hörst auf, in "+deinem()+" zu lesen.";

        case PAGE_DESC_STOP_MSG:
            return Der(leser)+plural(" hört auf, in "," hören auf, in ",leser)
                             +seinem(this_object(),0,leser)+" zu lesen.";

        case PAGE_DESC_OPEN:
            return "Du schlägst "+deinen()+" auf.";

        case PAGE_DESC_OPEN_MSG:
            return Der(leser)+plural(" schlägt "," schlagen ",leser)+seinen()+" auf.";

        case PAGE_DESC_CLOSE:
            return "Du schließt "+deinen()+".";

        case PAGE_DESC_CLOSE_MSG:
            return Der(leser)+plural(" schließt "," schließen ",leser)+seinen()+".";

        case PAGE_DESC_NOT_SEARCHABLE:
            return "In "+dem()+" kannst du nichts suchen.";

        case PAGE_DESC_EMPTY_SEARCH:
            return "Wonach willst du suchen?";

        case PAGE_DESC_SEARCH_BEYOND:
            return "Weiter kannst du nicht mehr suchen, "+der()+" "+ist()+" zu Ende.";

        case PAGE_DESC_SEARCH_TLE:
            return "Du durchsuchst "+den()+" ab Seite "+current_page+" einige Seiten, findest aber nichts.";

        case PAGE_DESC_SEARCH_REACHED_LAST:
            return "Du durchsuchst "+den()+" ab Seite " +current_page+" bis zur letzten Seite, findest aber nichts.";
    }

    return 0;
}

private string eval_entry(string index, object leser)
{
    string entry;

    if (!member(seiten_desc, index))
        return get_default_entry(index, leser);

    entry = seiten_desc[index];
    if (!stringp(entry) || !sizeof(entry))
        return "";

    entry = regreplace(entry, "'page_nr", to_string(current_page), 1);
    return apply(CLOSURE_CONTAINER->do_bind(mixed_to_closure(entry,
                                            ({ 'book_ob,
                                               'page_ob,
                                               'reader_ob
                                            }), 0, 'reader_ob), ({})),
                 this_object(),
                 (seiten_desc[PAGE_DESC_NAME]
                  ? (["name": seiten_desc[PAGE_DESC_NAME], "gender": seiten_desc[PAGE_DESC_GENDER] ])
                  : (["name":"seite", "gender":"weiblich"])),
                 leser);
}

private void send_book_message(object leser, int mt, int ma, string key_msg_me, string key_msg_other)
{
    if (!objectp(leser))
        return;

    if (key_msg_me)
        leser->send_message_to(leser, mt, ma, eval_entry(key_msg_me, leser));

    if (key_msg_other && leser && !IS_INVIS(leser))
        leser->send_message(mt, ma, eval_entry(key_msg_other, leser));
}

/*
FUNKTION: set_current_page
DEKLARATION: int set_current_page(int page)
BESCHREIBUNG:
Damit kann man das Buch an einer bestimmten Stelle aufschlagen.
Mit einem negativen Wert schlägt man das Buch zu, mit 0 am Inhaltsverzeichnis
und mit postiven Werten bei der entsprechenden Seite auf.
Diese Funktion liefert den neuen Wert außerdem zurück.
VERWEISE: set_current_page
GRUPPEN: buch
*/
int set_current_page(int page)
{
    return (page <= max_page)
            ? (current_page = (page<0) ? -1 : page)
            : current_page;
}

/*
FUNKTION: set_verzeichnis
DEKLARATION: void set_verzeichnis(string|int verzeichnis, string verzeichnis_no_ascii_art = 0)
BESCHREIBUNG:
Damit kann man angeben, ob dieses Buch ein Inhaltsverzeichnis besitzt
(verzeichnis=1), falls man dieses dynamisch über query_verzeichnis_inhalt
nachliefert. Liegt das Inhaltsverzeichnis bereits als Datei vor,
so kann man dessen Dateinamen als Parameter angeben. Relative
Pfadnamen sind dabei erlaubt.
Der optionale zweite Parameter setzt den Dateinamen für das Verzeichnis
für Spieler, die no_ascii_art aktiviert haben.
VERWEISE: set_page_names, query_verzeichnis, query_verzeichnis_inhalt
GRUPPEN: buch
*/
#if __VERSION__ > "3.6.3"
void set_verzeichnis(string|int verz, string verz_no_ascii_art = 0)
#else
varargs void set_verzeichnis(string|int verz, string verz_no_ascii_art)
#endif
{
    if(stringp(verz))
        verzeichnis = ABS_PATH(verz);
    else
        verzeichnis = verz;

    verzeichnis_no_ascii_art = ABS_PATH(verz_no_ascii_art);
}

/*
FUNKTION: set_page_names
DEKLARATION: void set_page_names(string* files, string* files_no_ascii_art = 0)
BESCHREIBUNG:
Hiermit gibt man die Dateinamen der einzelnen Seiten an.
Wildcards und relative Pfadangaben sind möglich.
Bei Wildcards werden die Dateinamen alphabetisch sortiert eingefügt.

Wenn files_no_ascii_art verwendet wird, muss die Anzahl der Seiten gleich sein.

Beispiel:
    set_page_names( ({ "../texte/atlasseite*",
                       "../texte/einzelseite" }) );
Mit Seiten für no_ascii_art:
    set_page_names( ({ "../texte/seite1",
                       "../texte/seite2" }),
                    ({ "../texte/seite_na1",
                       "../texte/seite_na2" }) );

VERWEISE: set_verzeichnis, query_page_names, query_page_inhalt
GRUPPEN: buch
*/
#if __VERSION__ > "3.6.3"
void set_page_names(string* files, string* files_no_ascii_art = 0)
#else
void set_page_names(string* files, string* files_no_ascii_art)
#endif
{
    if(!sizeof(files))
    {
        page_names = 0;
        max_page = 0;
        return;
    }

    page_names = ({});
    foreach (string file: files)
    {
        <string|int>* dir = get_dir(ABS_PATH(file), GETDIR_PATH | GETDIR_SIZES);
        for(int i = 0; i < sizeof(dir); i+=2)
            if (dir[i+1] > 0)
                page_names += dir[i..i];
    }
    max_page = page_names ? sizeof(page_names) : 0;

    // Das gleiche für die Seiten mit no_ascii_art.
    if(files_no_ascii_art)
    {
        page_names_no_ascii_art = ({});
        foreach (string file: files_no_ascii_art)
        {
            <string|int>* dir = get_dir(ABS_PATH(file), GETDIR_PATH | GETDIR_SIZES);
            for(int i = 0; i < sizeof(dir); i+=2)
                if (dir[i+1] > 0)
                    page_names_no_ascii_art += dir[i..i];
        }

        // Wegen der Wildcards können wir das erst hier prüfen.
        if(sizeof(page_names) != sizeof(page_names_no_ascii_art))
            raise_error("Die Anzahl der Seiten muss in beiden Fällen gleich "
                        "sein.");
    }
}

private string vorherige_naechste_seite_str()
{
    return (seiten_desc[PAGE_DESC_NAME]
            ? choose_by_gender(seiten_desc[PAGE_DESC_GENDER],
                               ({ "vorheriges / nächstes ",
                                  "vorherigen / nächsten ",
                                  "vorherige / nächste " }))
              +capitalize(seiten_desc[PAGE_DESC_NAME])
            : "vorherige / nächste Seite");
}

private string seite_str()
{
    return capitalize(seiten_desc[PAGE_DESC_NAME] || "seite");
}

private void handle_input_to(string str, object leser);
private void start_input_to(object leser)
{
    string cmdline;
    if(moremode != B_MODE_PAGE)
        return;

    cmdline = query_cap_name()
        + (current_page ? " "+seite_str()+" "+to_string(current_page) : "")
        + ": ";
    cmdline += "[-/+/1.."+max_page+",r,";
    if (verzeichnis) cmdline += "i,";
    if (searchable) cmdline += "/<such>,";
    cmdline += "q,?]: ";
    if (strlen (cmdline) > 75)
        cmdline = wrap(cmdline)[0..<2];

    input_to(#'handle_input_to, INPUT_PROMPT, cmdline, leser);
}

// Dies wird nur vom more-Modus genutzt
private string more_status_line(int more_line, int max_line, mixed more_id)
{
    string line;
    int chunk, page,zulang;
    if(!IS_BUCH_MORE_ID(more_id))
	raise_error("more_status_line() mit falscher more-ID aufgerufen!\n");
    page = GET_PAGE_FROM_MORE_ID(more_id);
    if(page<=0) // Inhaltsverzeichnis
    {
	if(i_status_line)
	    return i_status_line;
	line = query_cap_name()+": ";
    }
    else
    {
	if(status_line)
	    return regreplace(status_line,"%p",to_string(page),1);
	line = query_cap_name()+" "+seite_str()+" "+to_string(page)+": ";
    }
    chunk = GET_READER_FROM_MORE_ID(more_id)->query_more_chunk();
    if(zulang = (more_line && (chunk<more_line || more_line<max_line)))
	line += "Zeilen "+((chunk>more_line)?1:(more_line-(more_line-1)%chunk))
	     +  "-"+more_line+" von "+max_line+" ";
	
    line +=  "[-/+/1.."+max_page+",r,";
    if(verzeichnis && page>=0) line += "i,";
    if(searchable) line += "/<such>,";
    if(zulang) line += "<,>,u,";
    line += "q,?] ";
    if (strlen (line) > 78)
	line = wrap(line)[0..<2];
    return line;
}

private void catmore(string|string* was, int line, int quiet, object leser)
{
    if (!was)
        return;
    switch(moremode)
    {
        case B_MODE_PAGE:
            send_message_to(leser, MT_LOOK, MA_READ, was);
            break;
        case B_MODE_MORE:
        {
            string* zeilen = explode(was, "\n");
            if (zeilen[<1] == "")
                zeilen = zeilen[0..<2];
            leser->more(zeilen, #'more_status_line,
                        line,
                        M_DO_NOT_END|(quiet && M_NO_FIRST_SCREEN),
                        BUCH_MORE_ID(leser, current_page));
            break;
        }
        case B_MODE_MOREFILE:
            leser->more(was, #'more_status_line,
                        line,
                        M_DO_NOT_END|(quiet && M_NO_FIRST_SCREEN),
                        BUCH_MORE_ID(leser, current_page));
            break;
    }
}

private void show_page(object leser)
{
    if ((current_page <= 0) || (current_page > max_page))
        return;

    send_book_message(leser, MT_LOOK, MA_READ, 0, PAGE_DESC_READ_MSG);

    catmore(this_object().query_page_inhalt(current_page,leser),0,0,leser);
}

private void show_verzeichnis(object leser)
{
    if (!verzeichnis)
        return;

    send_book_message(leser, MT_LOOK, MA_READ, 0, PAGE_DESC_READ_TOC_MSG);

    catmore(this_object().query_verzeichnis_inhalt(leser),0,0,leser);
}

/*
FUNKTION: query_verzeichnis_inhalt
DEKLARATION: string|string* query_verzeichnis_inhalt(object leser)
BESCHREIBUNG:
Diese Funktion liefert den Text des Inhaltsverzeichnisses (als String) zurück.
Sie ist nicht dazu gedacht, um von außen aufgerufen zu werden, sondern
zur Überlagerung, um dynamische Bücher zu gestaltet.

Im "morefile" Modus wird der zurückgelieferte Text als Dateiname
interpretiert. Ein normaler Text muss dann als Array aus den einzelnen
Zeilen zurückgeliefert werden.

Beim Überlagern bitte query_no_ascii_art beachten.
VERWEISE: set_verzeichnis, query_page_inhalt
GRUPPEN: buch
*/
#if __VERSION__ > "3.6.3"
string|string* query_verzeichnis_inhalt(object leser=0)
#else
varargs string|string* query_verzeichnis_inhalt(object leser)
#endif
{
    string|int what_to_show =
        ( objectp(leser) && leser->query_no_ascii_art() &&
          verzeichnis_no_ascii_art
          ? verzeichnis_no_ascii_art : verzeichnis );
    if (what_to_show && stringp(what_to_show))
        return (moremode == B_MODE_MOREFILE)
            ? what_to_show : read_file(what_to_show);
}

/*
FUNKTION: query_page_inhalt
DEKLARATION: string|string* query_page_inhalt(int nr, object leser)
BESCHREIBUNG:
Diese Funktion liefert den Text (als String) der Seite mit der Nummer nr
zurück. Sie ist nicht dazu gedacht, um von außen aufgerufen zu werden,
sondern zur Überlagerung, um dynamische Bücher zu gestaltet.

Im "morefile" Modus wird der zurückgelieferte String als Dateiname
interpretiert. Ein normaler Text muss dann als Array aus den einzelnen
Zeilen zurückgeliefert werden.

Beim Überlagern bitte query_no_ascii_art beachten.
VERWEISE: set_page_names, set_max_page, query_verzeichnis_inhalt
GRUPPEN: buch
*/
#if __VERSION__ > "3.6.3"
string|string* query_page_inhalt(int seitenr, object leser=0)
#else
varargs string|string* query_page_inhalt(int seitenr, object leser)
#endif
{
    string* pages = (page_names_no_ascii_art && objectp(leser) && leser->query_no_ascii_art())
                  ? page_names_no_ascii_art : page_names;

    if (seitenr >= 0 && seitenr <= sizeof(pages))
        return (moremode == B_MODE_MOREFILE)
            ? pages[seitenr-1] : read_file(pages[seitenr-1]);
}

#if __VERSION__ > "3.6.3"
void stop_reading(object leser=0)
#else
void stop_reading(object leser)
#endif
{
    send_book_message(leser, MT_LOOK, MA_READ, PAGE_DESC_STOP, PAGE_DESC_STOP_MSG);
}


#if __VERSION__ > "3.6.3"
void oeffnen(object leser=0)
#else
void oeffnen(object leser)
#endif
{
    // aus Kompatibilitätsgründen zu altem Kram:
    leser ||= this_player();

    if (current_page >= 0)
    {
        if (open_close)
        {
            send_message_to(leser, MT_NOTIFY, MA_USE,
                            Dein()+" "+ist()+" doch bereits aufgeschlagen.");
        }
        return;
    }
    if (open_close)
        send_book_message(leser, MT_LOOK, MA_USE, PAGE_DESC_OPEN, PAGE_DESC_OPEN_MSG);

    if (verzeichnis)
        current_page = 0;
    else
        current_page = 1;
}


#if __VERSION__ > "3.6.3"
void schliessen(object leser=0)
#else
void schliessen(object leser)
#endif
{
    leser ||= this_player();

    if (current_page == -1)
    {
        if (open_close)
        {
            send_message_to(leser, MT_NOTIFY, MA_USE,
                            Dein()+" "+ist()+" doch gar nicht aufgeschlagen.");
        }
        return;
    }
    if (open_close)
    {
        send_book_message(leser, MT_LOOK, MA_USE, PAGE_DESC_CLOSE, PAGE_DESC_CLOSE_MSG);
    }
    else
    {
        stop_reading(leser);
    }
    current_page = -1;
}

/*
FUNKTION: book_action
DEKLARATION: mixed book_action(string str, int page, int more_line, int max_line)
BESCHREIBUNG:
Wenn man diese Funktion überlagert, kann man eigene Befehle im Buch
implementieren. Dies gilt nur für die Modi "more" und "morefile".
In str wird der eingebene Befehl beim Lesen des Buches
übergeben, page enthält die Seitennummer (0 fürs Inhaltsverzeichnis),
more_line die aktuelle Zeile und max_line die Anzahl an Zeilen in dieser Seite.

Zurückliefern kann man entweder wie bei more_action die Konstanten:
    CONTINUE    Mit dem Lesen weitermachen
    END_MORE    Mit dem Lesen aufhören (Hierbei wird einfach abgebrochen,
                man kann das Lesen dann mit continue_read fortsetzen.)
    NOTHING     Nix machen, Prompt nochmal ausgeben
Diese Konstanten sind in more.h definiert.

Man kann außerdem ein Array der Form ({neue Seite, Zeilennummer}) übergeben,
was bedeutet, dass die angegebene Seite ab der angegebenen Zeile angezeigt
werden soll.
VERWEISE: book_help, set_page_mode, continue_read
GRUPPEN: buch
*/
mixed book_action(string str, int page, int more_line, int max_line)
{
}

/*
FUNKTION: book_help
DEKLARATION: string* book_help(int page, int more_line, int max_line)
BESCHREIBUNG:
Wenn man diese Funktion überlagert, kann man die Hilfe des Buches erweitern
oder verändern. Standardmäßig liefert die Funktion die originale Hilfe
als Array aus den einzelnen Zeilen zurück.
Dies gilt bisher aber nur für die Modi "more" und "morefile".
page enthält die Seitennummer (0 fürs Inhaltsverzeichnis),
more_line die aktuelle Zeile und max_line die Anzahl an Zeilen in dieser Seite.
VERWEISE: book_action, set_page_mode, continue_read
GRUPPEN: buch
*/
string *book_help(int page, int more_line, int max_line)
{
    return explode(wrap("Folgendes kannst du beim Lesen "+des()+" machen:"), "\n")
        +({
                "  1..       "+seite_str()+" x lesen",
                "  -/+       "+vorherige_naechste_seite_str()+" lesen",
                "  r         "+seite_str()+" neu ausgeben",
          })
        +(verzeichnis
          ? ({
                "  i,z       Inhaltsverzeichnis anschauen",
            })
          : ({}) )
        +(searchable
            ? ({
                "  /<wort>   sucht ab "+
                                (seiten_desc[PAGE_DESC_NAME]
                                 ? dem(seiten_desc,"nächst")
                                 : "der nächsten Seite")+
                                " nach dem Wort",
              })
            : ({}) )
        +(open_close
            ? ({
                "  a         aufhören zu lesen ("+wer(this_object(), ART_KEINS)
                                                 +plural(" bleibt offen)"," bleiben offen)"),
                "  q         aufhören zu lesen, "+wer(this_object(), ART_KEINS)+" schließen",
              })
            : ({
                "  a,q       aufhören zu lesen",
              }) );
}

/*
FUNKTION: continue_read
DEKLARATION: varargs void continue_read(int page, int line, int quiet)
BESCHREIBUNG:
Damit wird ein durch END_MORE bei book_action abgebrochenes Lesen an
der Seite page, Zeile line fortgesetzt. Wenn quiet != 0 ist, dann
wird erstmal nur die Statuszeile ausgegeben.
VERWEISE: book_action
GRUPPEN: buch
*/
varargs void continue_read(int page, int line, int quiet, object leser)
{
    current_page = page;
    catmore( current_page
             ? this_object().query_page_inhalt(current_page, leser)
             : this_object().query_verzeichnis_inhalt(leser),
             line, quiet, leser);
}

nomask int more_action(string str, int more_line, int max_line, mixed more_id)
{
    mixed ac;
    int seite;
    object leser;

    if(!IS_BUCH_MORE_ID(more_id))       // Aus Kompatibilitätsgruenden
	return CONTINUE;

    leser = GET_READER_FROM_MORE_ID(more_id);

    ac = this_object().book_action(str, GET_PAGE_FROM_MORE_ID(more_id),
        more_line, max_line, leser);
    if(ac)
    {
	if(pointerp(ac))
	{
	    continue_read(ac[0], sizeof(ac)>1 && ac[1], sizeof(ac)>2 && ac[2], leser);
	    more_status = 1;
	    return END_MORE;
	}
	else
	{
	    if(ac==END_MORE)
		more_status = 1;
	    return ac;
	}
    }

    switch(trim(lower_case(str)))
    {
	case "-":
	    if(current_page > 1)
	    {
        	current_page--;
		show_page(leser);
		more_status = 1;
        	return END_MORE;
	    }
	    else if (verzeichnis && (current_page == 1))
	    {
        	current_page--;
		show_verzeichnis(leser);
		more_status = 1;
        	return END_MORE;
	    }

	    send_book_message(leser, MT_NOTIFY, MA_READ, PAGE_DESC_NO_PREV, 0);
	    return NOTHING;
	case "+":
    	    if (current_page < max_page)
	    {
        	current_page++;
		show_page(leser);
		more_status = 1;
        	return END_MORE;
    	    }
	    send_book_message(leser, MT_NOTIFY, MA_READ, PAGE_DESC_NO_NEXT, 0);
	    return NOTHING;
	case "z":
    	    if (!current_page)
	    {
		send_message_to(leser, MT_NOTIFY, MA_READ,
		                "Du bist doch schon im Inhaltsverzeichnis.");
		return NOTHING;
	    }
	case "i":
    	    if (verzeichnis)
	    {
		current_page = 0;
		show_verzeichnis(leser);
		more_status = 1;
        	return END_MORE;
	    }
	    send_message_to(leser, MT_NOTIFY, MA_READ,
	                    Dieser(this_object(),0)+plural(" hat"," haben")
	                    +" kein Inhaltsverzeichnis.");
	    return NOTHING;
	case "a":
	    return END_MORE;
	case "?":
	    leser->more(
		this_object().book_help(GET_PAGE_FROM_MORE_ID(more_id), more_line, max_line),
		"Hilfe zu "+dem()+": ",
		0, M_AUTO_END | M_HEADER_LINE,
		BUCH_HELP_MORE_ID(leser, GET_PAGE_FROM_MORE_ID(more_id), more_line-leser->query_more_chunk()));
	    more_status = 1;
	    return END_MORE;
	default:
	    if (seite = to_int (str))
	    {
		if( seite > 0 && seite <= max_page)
		{
		    if(current_page==seite)
		    {
			send_message_to(leser, MT_NOTIFY, MA_READ,
			    "Du befindest dich bereits auf "
			    + (seiten_desc[PAGE_DESC_NAME] ? dem(seiten_desc) : "der Seite")
			    + " " + seite +".");
			return NOTHING;
		    }
		    current_page = seite;
		    show_page(leser);
		    more_status = 1;
		    return END_MORE;
    		}
    		if (seite <= 0)
		    send_book_message(leser, MT_NOTIFY, MA_READ, PAGE_DESC_INVALID, 0);
		else
		    send_book_message(leser, MT_NOTIFY, MA_READ, PAGE_DESC_OVERFLOW, 0);
		return NOTHING;
    	    }
	    if (str[0..0] == "/" && searchable)
	    {
    		int search;

    		search = current_page;
    		str = str[1..];

		if (str=="")
		{
		    send_book_message(leser, MT_NOTIFY, MA_READ, PAGE_DESC_EMPTY_SEARCH, 0);
		    return NOTHING;
    		}
		if (current_page == max_page)
		{
		    send_book_message(leser, MT_NOTIFY, MA_READ, PAGE_DESC_SEARCH_BEYOND, 0);
		    return NOTHING;
		}
    		current_page++;
    		while (search < max_page)
		{
        	    search++;
		    if(moremode == B_MODE_MOREFILE)
		    {
			string|string* datei = query_page_inhalt(search, leser);
			int groesse;
			if(!stringp(datei))
			{
			    if(strstr(implode(datei,"\n"),str) != -1)
			    {
				current_page = search;
				show_page(leser);
				more_status = 1;
				return END_MORE;
			    }
			}
			else if((groesse = file_size(datei))>0 && groesse < 500000)
			{
			    int pos = 0;
			    int blen = driver_info(DI_CURRENT_RUNTIME_LIMITS)[LIMIT_FILE];
			    while(pos<groesse &&
				strstr(read_bytes(datei, pos, blen),str)==-1)
				    pos+=blen-strlen(str);
			    if(pos<groesse)
			    {
				current_page = search;
				show_page(leser);
				more_status = 1;
				return END_MORE;
			    }
			}
		    }
		    else
		    {
			if (strstr(query_page_inhalt(search, leser), str) != -1)
			{
			    current_page = search;
			    show_page(leser);
			    more_status = 1;
			    return END_MORE;
			}
		    }
        	    if (get_eval_cost() < 150000)
		    {
			send_book_message(leser, MT_NOTIFY, MA_READ, PAGE_DESC_SEARCH_TLE, 0);
            		current_page = search;
			return NOTHING;
        	    }
    		}
		send_book_message(leser, MT_NOTIFY, MA_READ, PAGE_DESC_SEARCH_REACHED_LAST, 0);
    		current_page = search;
		return NOTHING;
	    }
    }
}

nomask void more_end(string str, int line, int max_line, mixed more_id)
{
    object leser = GET_READER_FROM_MORE_ID(more_id);

    if(IS_BUCH_HELP_MORE_ID(more_id))
    {
	current_page = GET_PAGE_FROM_MORE_ID(more_id);
	catmore(
	    current_page
	    ? this_object().query_page_inhalt(current_page, leser)
	    : this_object().query_verzeichnis_inhalt(leser),
	    GET_LINE_FROM_MORE_ID(more_id)-leser->query_more_chunk(),
	    1,
	    leser);
	return;
    }
    if(IS_BUCH_MORE_ID(more_id))        // Jemand will das Buch schließen
    {
	if(more_status)
	    more_status = 0;
	else
	{
	    if(stringp(str) && trim(lower_case(str))=="a")
	        stop_reading(leser);
	    else
	        schliessen(leser);
	}
	return;
    }
}

private void handle_input_to(string str, object leser)
{
    int seite;

    if (str == "-")
    {
        if (current_page > 1)
        {
            current_page--;
            show_page(leser);
            start_input_to(leser);
            return;
        }
        if (verzeichnis && (current_page == 1))
        {
            current_page--;
            show_verzeichnis(leser);
            start_input_to(leser);
            return;
        }
        send_book_message(leser, MT_NOTIFY, MA_READ, PAGE_DESC_NO_PREV, 0);
        start_input_to(leser);
        return;
    }
    if ((str == "+") || ((str == "") && (moremode == B_MODE_PAGE)))
    {
        if (current_page < max_page)
        {
            current_page++;
            show_page(leser);
            start_input_to(leser);
            return;
        }
        send_book_message(leser, MT_NOTIFY, MA_READ, PAGE_DESC_NO_NEXT, 0);
        start_input_to(leser);
        return;
    }
    if ((str == "z") || (str == "Z") || (str == "i") || (str == "I"))
    {
        if (!current_page && ((str == "z") || (str == "Z")))
        {
            send_message_to(leser, MT_NOTIFY, MA_READ,
                            "Du bist doch schon im Inhaltsverzeichnis.");
        }
        else if (verzeichnis)
        {
            current_page = 0;
            show_verzeichnis(leser);
            start_input_to(leser);
            return;
        }
        else
        {
            send_message_to(leser, MT_NOTIFY, MA_READ,
                            Dieser(this_object(),0)+plural(" hat"," haben")+
                            " kein Inhaltsverzeichnis.");
        }
        start_input_to(leser);
        return;
    }
    if (seite = to_int(str))
    {
        if ((seite > 0) && (seite <= max_page))
        {
            current_page = seite;
            show_page(leser);
            start_input_to(leser);
            return;
        }
        if (seite <= 0)
            send_book_message(leser, MT_NOTIFY, MA_READ, PAGE_DESC_INVALID, 0);
        else
            send_book_message(leser, MT_NOTIFY, MA_READ, PAGE_DESC_OVERFLOW, 0);
        start_input_to(leser);
        return;
    }
    if ((str == "r") || (str == "R"))
    {
	if(current_page)
	    show_page(leser);
	else
	    show_verzeichnis(leser);
        start_input_to(leser);
        return;
    }
    if ((str == "q") || (str == "Q"))
    {
        schliessen(leser);
        return;
    }
    if ((str == "a") || (str == "A"))
    {
        stop_reading(leser);
        return;
    }
    if (str == "?")
    {
        string* tmp = this_object().book_help(0, 0, 0);
        send_message_to(leser, MT_NOTIFY, MA_READ, implode(tmp,"\n"));
        start_input_to(leser);
        return;
    }
    if (str[0..0] == "/")
    {
        int search;

        search = current_page;
        if (!searchable)
        {
            send_book_message(leser, MT_NOTIFY, MA_USE, PAGE_DESC_NOT_SEARCHABLE, 0);
            start_input_to(leser);
            return;
        }
        str = str[1..];
        if (str=="")
        {
            send_book_message(leser, MT_NOTIFY, MA_USE, PAGE_DESC_EMPTY_SEARCH, 0);
            start_input_to(leser);
            return;
        }
        if (current_page == max_page)
        {
            send_book_message(leser, MT_NOTIFY, MA_USE, PAGE_DESC_SEARCH_BEYOND, 0);
            start_input_to(leser);
            return;
        }
        current_page++;
        while (search < max_page)
        {
            search++;
            if (strstr(query_page_inhalt(search, leser), str) != -1)
            {
                current_page = search;
                show_page(leser);
                start_input_to(leser);
                return;
            }
            if (get_eval_cost() < 150000)
            {
                send_book_message(leser, MT_NOTIFY, MA_USE, PAGE_DESC_SEARCH_TLE, 0);
                current_page = search;
                start_input_to(leser);
                return;
            }
        }
        send_book_message(leser, MT_NOTIFY, MA_USE, PAGE_DESC_SEARCH_REACHED_LAST, 0);
        current_page = search;
        start_input_to(leser);
        return;
    }

    start_input_to(leser);
}

/*
FUNKTION: forbidden_read
DEKLARATION: mixed forbidden_read(mixed buch, object wer)
BESCHREIBUNG:
Wenn der Spieler 'wer' das Buch 'buch' (oder ein anderes Objekt mit dieser
Abfrage) lesen will, ruft das Buch wer->forbidden("read", buch) auf.
forbidden ruft dann in allen mit wer->add_controller("forbidden_read", other)
angemeldeten Controllern other die Funktion other->forbidden_read(buch, wer)
auf.

Liefert eine dieser Funktionen einen Wert != 0 zurück, so kann derjenige
das Buch nicht lesen. Ist dieser Rückgabewert ein String, so wird dieser
ausgegeben, ansonsten (wenn es eine Zahl ist), wird die Standardfehlermeldung
ausgegeben.
VERWEISE: forbidden, add_controller, forbidden_read_me, query_read
GRUPPEN: buch
*/
/*
FUNKTION: forbidden_read_me
DEKLARATION: mixed forbidden_read_me(object wer, mixed buch)
BESCHREIBUNG:
Wenn der Spieler 'wer' das Buch 'buch' (oder ein anderes Objekt mit dieser
Abfrage) lesen will, ruft das Buch buch->forbidden("read_me", wer) auf.
forbidden ruft dann in allen mit buch->add_controller("forbidden_read_me",
other) angemeldeten Controllern other die Funktion other->forbidden_read_me(
wer, buch) auf.

Liefert eine dieser Funktionen einen Wert != 0 zurück, so kann derjenige
das Buch nicht lesen. Ist dieser Rückgabewert ein String, so wird dieser
ausgegeben, ansonsten (wenn es eine Zahl ist), wird die Standardfehlermeldung
ausgegeben.
VERWEISE: forbidden, add_controller, forbidden_read, query_read
GRUPPEN: buch
*/

varargs string query_read(string str, string all, object leser)
{
    string side;
    int page;
    mixed err;

    if ((max_page <= 0) && !verzeichnis)
        return eval_entry(PAGE_DESC_NO_PAGES,leser);

    if ( (hlp && !hlpp(leser) && !wizp(leser)) || (wiz && !wizp(leser)) )
        return eval_entry(PAGE_DESC_NOT_ALLOWED,leser);

    err = (objectp(leser) && leser->forbidden("read", this_object())) ||
          this_object()->forbidden("read_me", leser);
    if(stringp(err))
	return err;
    else if(err)
        return eval_entry(PAGE_DESC_NOT_ALLOWED,leser);

    page = current_page;
    if (str && (str != ""))
    {
        str = lower_case(strip(str));
        if (str == "inhaltsverzeichnis" || str == "verzeichnis" ||
            str == "index" || str == "inhalt")
        {
            if (verzeichnis)
                current_page = 0;
            else
                return Der()+plural(" hat"," haben")+" kein Inhaltsverzeichnis.";
        }
        else
        {
            sscanf(str,"%s %d", side, page);
            if (side)
            {
                if (member(seiten_desc[PAGE_DESC_IDS] || ({ "seite" }),
			    lower_case(side)) == -1)
                    return wrap ("Lies "+den()+" oder lies "+den()+" "+
                                 seite_str()+" <1..>"+
                           (verzeichnis ? " oder lies "+den()+
                               " Inhaltsverzeichnis" : ""));
                if (page == 0)
                    return eval_entry(PAGE_DESC_WHICH,leser);
                if (page < 0)
                    return eval_entry(PAGE_DESC_INVALID,leser);
                if (page > max_page)
                    return eval_entry(PAGE_DESC_OVERFLOW,leser);
                current_page = page;
            }
            else
                return "Lies "+den()+" oder lies "+den()+" "+seite_str()+" <1..>"+
                       (verzeichnis ? " oder lies "+den()+" Inhaltsverzeichnis" : "");
        }
    }
    if (current_page == -1)
        oeffnen(leser);
    if (current_page == 0)
        show_verzeichnis(leser);
    else
        show_page(leser);
    start_input_to(leser);
    return "";
}


int oeffne(string str)
{
    if (!open_close)
    {
        return 0;
    }
    if (!me(str))
    {
        return notify_fail("Öffne was?");
    }
    oeffnen(this_player());
    return 1;
}


int schliess(string s)
{
    if (!open_close)
    {
        return 0;
    }
    if (!me(s))
    {
        return notify_fail("Schließe was?");
    }
    schliessen(this_player());
    return 1;
}


int schlag(string s)
{
    string wie;
    if (!open_close)
    {
        return 0;
    }
    if (!(wie=me(s)))
    {
        return notify_fail("Schlage was auf oder zu?");
    }
    wie = lower_case (wie);
    if (wie == "auf")
    {
        oeffnen(this_player());
        return 1;
    }
    if (wie == "zu")
    {
        schliessen(this_player());
        return 1;
    }
    return notify_fail("Schlage "+den()+" auf oder zu?", FAIL_INTERNAL);
}

/*
FUNKTION: query_book
DEKLARATION: int query_book()
BESCHREIBUNG:
Liefert bei einem Buch 1 zurück.
VERWEISE:
GRUPPEN: buch
*/
int query_book()
{
    return 1;
}


string query_short (object betrachter)
{
    if (open_close && (current_page >= 0))
        return ::query_short(betrachter)+" (aufgeschlagen)";
    return ::query_short(betrachter);
}
