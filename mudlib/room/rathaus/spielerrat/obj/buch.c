// This file is part of UNItopia Mudlib.
// ---------------------------------------------------------------------
// File:          /room/rathaus/spielerrat/obj/buch.c
// Description:   Ein Buch fuer den Spielerrat
// Author:        Gnomi

inherit "/i/object/buch";
inherit "/i/tools/security";

#include <apps.h>
#include <editor.h>
#include <level.h>
#include <news.h>
#include <more.h>
#include <move.h>
#include <message.h>
#include <input_to.h>

#define SECURE(x) if(!spielerratp(this_player()) && !adminp(this_player()) ||\
		     !check_security())\
		  return (x)
#define CAN_WRITE (adminp(this_player()) || \
    NEWSD->is_owner(this_player(),"Ratsmitglieder",0))

#define ARCHIV_DIR "/var/adm/spielerratsarchiv/"
/*
 * Die Buecher des Archives im Spielerrat.
 * Jedes Buch hat entsprechend seinem Inhalt eine ID.
 * mit welcher es seine Daten aus SPIELERRATSARCHIV anfordern kann.
 * Jedes Spielerratsmitglied kann es lesen, Newsmoderatoren fuer
 * das Brett 'Ratsmitglieder' koennen es schreiben.
 *
 * Format der Seiten vom Spielerrat:
 *  Array aus den einzelnen Seiten, jede einzelne Seite hat folgenden Aufbau:
 *  ({string Titel, string Datum, string Text})
 */

private string ratsid,titel;
private string* oldfiles;

/*
 * Neue Ids muessen im Spielerratsarchiv angemeldet werden: (Admins only)
 * zc /apps/spielerratsarchiv->set_pages(id,({}));
 */
 
void init_book(string rats_id, string title)
{
  ratsid = rats_id;
  titel = title;
  set_max_page(sizeof(SPIELERRATSARCHIV->get_pages(ratsid)));
}

void create()
{
  ::create();
  set_verzeichnis(1);
  set_page_mode("morefile");
  set_no_store(1);
  set_searchable(0);
  init_security_for_actions();
}

varargs string query_read(string str, string all, object wer)
{
  SECURE("Du kannst keinen einzigen Buchstaben entdecken.\n");
  if(wer!=this_player())
    return "Du kannst keinen einzigen Buchstaben entdecken.\n";
  return ::query_read(str,all,wer);
}

mixed query_page_inhalt (int seite) //seite: 1-max_page
{
  mixed pages;
  SECURE("\n"*10);

  pages = SPIELERRATSARCHIV->get_pages(ratsid);
  set_max_page(sizeof(pages));
  if(seite>sizeof(pages))
    return ({"Diese Seite wurde anscheinend gerade herausgerissen."});
  return ARCHIV_DIR + pages[seite-1][2];
}

mixed query_verzeichnis_inhalt()
{
  mixed pages, inhalt;
  int nr,b_datum, b_nr, b_zeilen;
  string format_str;
  SECURE(sprintf("%75=|s\n\n%75|s\n","Inhaltsverzeichnis","Leer"));

  inhalt = explode(sprintf("%75=|s\n\n  Inhalt:\n",titel||"Leer"),"\n");
  pages = SPIELERRATSARCHIV->get_pages(ratsid);
  set_max_page(sizeof(pages));
  oldfiles = map(pages||({}), #'[, 2);
  if(!sizeof(pages))
    return inhalt+({"    Keine Seiten vorhanden."});

  //Erstmal die Breiten der einzelnen Spalten ermitteln:
  b_datum = sort_array(map(pages,(:strlen($1[1]):)),#'<)[0];
  b_nr = strlen(to_string(sizeof(pages)));
  b_zeilen = sort_array(map(pages,(:strlen(to_string($1[3])):)),#'<)[0];

  format_str = "  %" + b_nr + "i  %-:" + (65-b_nr-b_datum-b_zeilen) +
               "s %" + b_datum + "s %" + (b_zeilen+2) + "s";
  foreach(mixed t:pages)
  {
    nr++;
    inhalt += ({sprintf(format_str,nr,t[0],t[1],"("+t[3]+")")});
  }
  return inhalt+({""});
}

mixed book_action(string eingabe, int seite, int line, int max_line)
{
  SECURE(NOTHING);
  if(previous_object() != this_object() || previous_object(1)!=this_player())
      return NOTHING;
  if(strlen(eingabe) && CAN_WRITE && (eingabe[1..1]-" ")=="")
  {
    int nr;
    mixed pages;
    switch(eingabe[0])
    {
      case 's': // Artikel schreiben
        pages = SPIELERRATSARCHIV->get_pages(ratsid);
        if(!sscanf(eingabe[2..<1],"%d",nr))
	  nr=query_current_page()||sizeof(pages);
	if(nr<0)
	{
	  write("Es gibt keine negativen Seitenzahlen.\n");
	  return NOTHING;
	}
        say(wrap(Der(this_player())+" beginnt etwas in "+den()+
	    " hineinzuschreiben."));
	input_to("input_text", INPUT_PROMPT, "Titel: ", nr, 0);
	return END_MORE;
      case 'l': // Artikel loeschen
        if(!sscanf(eingabe[2..<1],"%d",nr))
	  nr=query_current_page();
        if(!nr)
	  write("Das Inhaltsverzeichnis kannst du nicht löschen.\n");
	else if(nr<0)
	  write("Es gibt keine negativen Seitenzahlen.\n");
	else
	{
          pages = SPIELERRATSARCHIV->get_pages(ratsid);
	  if(nr>sizeof(pages))
	    write("Diese Seite gibt es nicht mehr.\n");
	  else if(!oldfiles || nr>sizeof(oldfiles) || pages[nr-1][2]!=oldfiles[nr-1])
	    write("Die Seiten haben sich geändert!\n"
		  "Schau Dir sicherheitshalber vorher nochmal das Inhaltsverzeichnis an.\n");
	  else
	  {
            say(wrap(Der(this_player())+" zerreißt eine Seite aus "+
	        dem()+"."));
	    write("Ok.\n");
	    SPIELERRATSARCHIV->remove_page(pages[nr-1][2]);
	    pages=arr_delete(pages,nr-1);
	    SPIELERRATSARCHIV->set_pages(ratsid,pages);
	    set_max_page(sizeof(pages));
	  }
	}
	return NOTHING;
      case '?':
        write("1..:      lies Seite x\n"
	      "-/+:      lies vorherige / nächste Seite\n"
	      "r:        Seite neu ausgeben\n"
	      "i,z:      Inhaltsverzeichnis\n"
	      "s         Eine neue Seite schreiben\n"
	      "s <nr>    Eine neue Seite nach <nr> schreiben\n"
	      "l         Aktuelle Seite entfernen\n"
	      "l <nr>    Seite <nr> entfernen\n"
	      "/<wort>:  sucht in dieser Seite nach dem Wort\n"
	      "a:        aufhören zu lesen (Buch bleibt offen)\n"
	      "q:        aufhören zu lesen, Buch schließen\n");
	return NOTHING;
    }
  }
  return ::book_action(eingabe, seite, line, max_line);
}

static void input_text(string str,int nr, string titel)
{
    if(!str) 
    {
	continue_read(query_current_page());
	return;
    }
    if(!titel)
    {
	input_to("input_text", INPUT_PROMPT,
	    str?"Datum: ":"Titel: ", nr, str);
	return;
    }
    this_player()->mini_ed(lambda(({'txt}),({
	(:
	    if($1)
	    {
		mixed pages = SPIELERRATSARCHIV->get_pages(ratsid);
		string file = SPIELERRATSARCHIV->create_page(implode($1,"\n"));
    		if($2>sizeof(pages)) $2=sizeof(pages);
		pages = pages[0..$2-1]+({({$3, $4, file, sizeof($1)})})+pages[$2..<1];
		SPIELERRATSARCHIV->set_pages(ratsid,pages);
		oldfiles = 0;
		set_max_page(sizeof(pages));
	    }
	    continue_read(query_current_page());
	:), 'txt, nr, titel, str })),0,0,
        ([MINI_ED_TITLE:"Spielerratsarchiveintrag"]));
    return;
}

// Hack, damit Geister diese Buecher aufnehmen koennen.
// (Diese Buecher sind OOC.)
int no_take(object wer, object woher)
{
    if(!spielerratp(wer) || !wer->query_con_close())
	return 0;

    if (this_object()->move(wer,([MOVE_FLAGS:MOVE_FORCE]))==MOVE_OK)
    {
	wer->send_message(MT_LOOK, MA_TAKE,
	    wrap("Wie mit Geisterhand nimmt "+der(wer)+den()+" aus "+dem(woher)+"."),
	    wrap("Wie mit Geisterhand nimmst Du "+den()+" aus "+dem(woher)+"."),
	    wer);
	return 1;
    }
}
