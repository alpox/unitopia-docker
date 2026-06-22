// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/i/player/more.c
// Description: More des Spielers
// Author:	Freaky (23.12.93)
// Modified by:	Freaky (14.07.95) kleinere Verschoenerungen im Code
//		Freaky (22.12.95) Check, ob man schon im input-to ist
//              Parsec (01.12.98) Halbe Seite vor- und rueckwaertsblaettern
//		Freaky (02.03.2000) M_HEADER_LINE implementiert

/*
FUNKTION: more
DEKLARATION: string more({string file | string *text} [, mixed status [, int begin_line [, int status_byte [, mixed more_id]]]] )
BESCHREIBUNG:
Diese Funktion ist dazu da, ein File oder sonstigen Text anzuzeigen.

Der Returnwert ist ein String. M_ERR(ret) ist die Fehlermeldung als String.
M_ERR_NUM(ret) ist der Fehlercode (definiert in <sys/more,h>)

Wenn der 1. Parameter ein String ist, wird das File file angezeigt.
Falls es ein String-Array ist, wird das Feld angezeigt.
Man kann hoch und runterblaettern.

Wenn status angegeben wird, dann werden Standardzeilen ueberlagert.
status kann folgendes Format haben:
 - Ein String: Die Statuszeile, das 1. %d wird durch die momentane Zeile,
               das 2. %d durch die Anzahl an Zeilen ersetzt.
 - Eine Closure: Es wird funcall(status,more_line,max_line,more_id) aufgerufen
                 und danach die %d's wie beim String ersetzt.
 - Ein Array: ({Statuszeile, Obere Linie, Untere Linie, Mittlere Linie oben,
              Mittlere Linie unten}) (Die hinteren Elemente koennen
	      weggelassen werden.)
      Statuszeile: String oder Closure wie oben
      Obere Linie: Bei M_FRAME oder M_HEADER_LINE die erste Zeile
      Untere Linie: Bei M_FRAME oder M_HEADER_LINE die unterste Zeile
      Mittlere Linie oben: Bei M_HEADER_LINE die erste Zeile der Ausgabe
      Untere Linie unten: Bei M_HEADER_LINE die letzte Zeile der Ausgabe

Fuer jede Eingabe vom Benutzer wird im previous_object() die Funktion
'more_action(string eingabe, int line, int max_line, mixed more_id)' aufgerufen,
wobei eingabe die Eingabe des Benutzers ist. Je nachdem, was diese Funktion
returned (siehe /sys/more.h) reagiert der more anders.
Wenn der more beendet wird, wird im previous_object() die Funktion
'more_end(string eingabe, int line, int max_line, mixed more_id)' aufgerufen.

Diese Features werden z.B. in /obj/newsreader.c und /obj/mailreader.c verwendet

begin_line gibt an, an welche Stelle im File gesprungen werden soll.
Wenn begin_line negativ ist, wird die Zeile vom Fileende her angesprungen.

BEISPIEL:
player ist ein Objektzeiger auf einen Spieler:

Einfachster Fall: der Spieler bekommt eine Datei angezeigt:
    player->more("/doc/hilfe/goetter");

Dasselbe mit der Meldung "--Mehr--" nach jeder Seite:
    player->more("/doc/hilfe/goetter", "--Mehr--");

Der Spieler bekommt ein String-Array angezeigt (Strings gehen nicht!),
es wird erst bei Zeile 5 begonnen, der more endet mit Erreichen der 
letzten Zeile, der informative String "[Zeile 20 von 70]" wird nach 
jeder (ausser der letzten) Seite ausgegeben.
    player->more(({"Hier steht ein haufen Zeug drin:", "blabla", .... }),
	"[Zeile %d von %d, weiter mit Return]", 5, M_AUTO_END);

VERWEISE: cat, tail, set_more_chunk, query_more_chunk, query_current_line,
	  query_max_line, query_more_file
GRUPPEN: Dateien
*/

#pragma save_types
#pragma strong_types

#include <config.h>
#include <message.h>
#include <more.h>
#include <input_to.h>
#include <files.h>
#include <level.h>
#include <rtlimits.h>
#include <telnet.h>
#include <interactive_info.h>

#include "player.h"

#define NORMAL_STAT "--MORE--(Zeile %d/%d) [q,<,>,u,<nr>,/<such>,?] "
#define LINE "-------------------------------------------------------------------------------"
#define CONT_LINE "...----------------------------------------------------------------------------"
#define DEFAULT_CHUNK 23
#define MAX_CHUNK 200
#define MAX_SIZE 500000
#if __VERSION__ > "3.6.5"
#define ENCODING "UTF-8//REPLACE"
#else
#define ENCODING "UTF-8"
#endif

#define MORE_FILE   0
#define MORE_FIND   1
#define ARR         2
#define MORE_LINE   3
#define MAX_LINE    4
#define STATUS_BYTE 5
#define THEPO       6
#define STAT        7
#define MORE_ID     8

#define WRITE(byte, str)	this_object()->send_message_to(this_object(), \
				    M_GET_TYPE(byte)||MT_NOTIFY, M_GET_ACTION(byte), str)

#define NOTIFY(str)		this_object()->send_message_to(this_object(), \
				    MT_NOTIFY, MA_UNKNOWN, str)

static private int in_more = 0;	// <0: Gerade in der Abarbeitung eines
			        //     input_to, >0: input_to wartet.
private int chunk;

private void end_more(mixed *mi, string str);
private void even_more(string str, mixed *mi);
private void ask_overwrite(string yesno, string file, mixed *mi);
int query_more_chunk();

protected void clean_more()
{
    in_more = 0;
}

// Zeigt die nachsten Zeilen ab der Zeile a an
// und liefert die Anzahl der angezeigten Zeilen
private int catting(int a, mixed *mi)
{
    int i, diff_chunk;
    string line, *tmp_arr;

    if (mi[STATUS_BYTE] & M_SCROLL)
        diff_chunk = mi[MAX_LINE]+3;
    else
        diff_chunk = query_more_chunk();
    
    if(i=query_limits()[LIMIT_ARRAY])
        diff_chunk = min(diff_chunk, i);
    if(i=query_limits()[LIMIT_FILE])
        diff_chunk = min(diff_chunk, i/25);
    
    // Rahmen ziechnen ?
    if (mi[STATUS_BYTE] & M_FRAME)
    {
	diff_chunk-=2;
        if (a > 1)
	    WRITE(mi[STATUS_BYTE],mi[STAT][3]+"\n");
	else
	    WRITE(mi[STATUS_BYTE],mi[STAT][1]+"\n");
        if (a + diff_chunk <= mi[MAX_LINE])
	    line = mi[STAT][4]+"\n";
        else
	    line = mi[STAT][2]+"\n";
    }
    else if(mi[STATUS_BYTE] & M_HEADER_LINE)
    {
        if (a <= 1)
        {
            WRITE(mi[STATUS_BYTE],mi[STAT][1]+"\n");
            diff_chunk--;
        }
        if (a + diff_chunk > mi[MAX_LINE])
        {
            diff_chunk--;
	    if(a + diff_chunk > mi[MAX_LINE])
        	line = mi[STAT][2]+"\n";
        }
    }
    
    // Wenn es ein File ist:
    if (mi[MORE_FILE])
    {    
       mixed tmp;
       tmp = read_file(mi[MORE_FILE],a,diff_chunk,ENCODING);
       if(!tmp)
       {
          // Das passiert leider bei den ganzen savefiles,
	  // weil dort 'ne Menge in einer Zeile steht...
          if(diff_chunk>49) diff_chunk=49;
	  i = cat(mi[MORE_FILE],a,diff_chunk);
	  if (line)
	     WRITE(mi[STATUS_BYTE],line);
	  return i;
       }
       tmp_arr=explode(tmp,"\n");
       if(tmp_arr[<1]=="") tmp_arr = tmp_arr[0..<2];
    }
    else
       tmp_arr = mi[ARR][a - 1..a + diff_chunk - 2];

    i = sizeof(tmp_arr);
    // Es ist ein Array:

    // Zeilen-Nummern angeben (geht nur bei Arrays)
    if (mi[STATUS_BYTE] & M_LINE_NUMBERS)
    {
        int j, ab;
	ab = strlen(to_string(mi[MAX_LINE])) + 1;
	for(j = 0; j < i; j++)
	    WRITE(mi[STATUS_BYTE],right(to_string(j + a),ab) + ": " + tmp_arr[j] + "\n");
    }
    else if (i)
        WRITE(mi[STATUS_BYTE],implode(tmp_arr,"\n") + "\n");

    if (line)
	WRITE(mi[STATUS_BYTE],line);
    return i;
}

private int find_string(string str, int fl, mixed *mi)
{
    string *lines;
    int line;

    line=mi[MORE_LINE]-query_more_chunk()+2;
    if (str)
	mi[MORE_FIND]=lower_case(str);
    else if (!mi[MORE_FIND])
    {
	NOTIFY("Kein Such-String gegeben.\n");
	return NOTHING;
    }
    if (mi[MORE_FILE])
    {
	while (line<=mi[MAX_LINE] &&
               strstr(lower_case(read_file(mi[MORE_FILE]||"",line,20,ENCODING)||""),
                   mi[MORE_FIND])==-1)
	    line+=20;
	if (line<=mi[MAX_LINE])
	{
	    int i;
	    lines=explode(read_file(mi[MORE_FILE],line,20,ENCODING),"\n");
	    while (strstr(lower_case(lines[i++]||""),mi[MORE_FIND])==-1)
		;
	    mi[MORE_LINE]=line+i-2;
	    return CONTINUE;
	}
    }
    else
    {
	line=mi[MORE_LINE]-query_more_chunk()+1;
	if (line<0)
	    line=0;
	for (; line<mi[MAX_LINE]; line++)
	    if (strstr(lower_case(mi[ARR][line]||""),mi[MORE_FIND])!=-1)
	    {
		mi[MORE_LINE]=line;
		return CONTINUE;
	    }
    }
    NOTIFY("Nicht gefunden.\n");
    return NOTHING;
}

private int file_lenght(string str)
{
    string tmp;
    int siz, i, ok;

    // Anzahl der auf einmal gelesenen Zeilen (Muss vielfaches von 2 sein)
    // Bei der Annahme, dass ein File ca. 30 Zeichen pro Zeile hat, und
    // man nur 50000 Bytes auf einmal lesen kann:
#   define READ_CHUNK 1024

    tmp=read_file(str,1,READ_CHUNK,ENCODING);
    if (tmp && (siz=sizeof(explode(tmp,"\n"))) < READ_CHUNK )
	return siz-((tmp[<1]=='\n')?1:0);

    i = (siz == (READ_CHUNK + 1) ? READ_CHUNK * 2 + 1 : READ_CHUNK + 1);

    while(read_file(str,i,1,ENCODING))
	i += READ_CHUNK;

    if (i > (READ_CHUNK + 1) && (tmp=read_file(str,i-READ_CHUNK,READ_CHUNK,ENCODING)))
	return i + sizeof(explode(tmp,"\n")) - (READ_CHUNK +2);

    ok = READ_CHUNK / 2;
    do {
	if (!read_file(str,i-ok,1,ENCODING))
	    i -= ok;
	ok /= 2;
    } while(ok);
    return i - 2;
}

private void save_text(string file, mixed *mi)
{
    if(!valid_file_name(file))
	NOTIFY("Der Dateiname ist ungültig.\n");
    else if(mi[MORE_FILE]) // Eine Datei
    {
	if(copy_file(mi[MORE_FILE],file))
	    NOTIFY("Der Text konnte nicht gespeichert werden.\n");
	else
	    NOTIFY("Ok.\n");
    }
    else if(!write_file(file,implode(mi[ARR],"\n")+"\n"))
        NOTIFY("Der Text konnte nicht gespeichert werden.\n");
    else
	NOTIFY("Ok.\n");
}

private int add_line(string str, mixed *mi)
{
    int tmp,dchunk;

    if (mi[THEPO])
    {
        in_more = -1;
	tmp = mi[THEPO]->more_action(str,mi[MORE_LINE],mi[MAX_LINE],mi[MORE_ID]);
        in_more = 0;
	if (tmp != 0)
	    return tmp;
    }

    dchunk=query_more_chunk();
    if (mi[STATUS_BYTE] & M_FRAME)
	dchunk -= 2;
    else if (mi[STATUS_BYTE] & M_HEADER_LINE)
	dchunk--;


    switch(str)
    {
	case "q":
	case "e":
	    return END_MORE;
	case "<":
	    mi[MORE_LINE] = 0;
	    mi[STATUS_BYTE] &= ~M_AUTO_END;
	    break;
	case ">":
	    mi[MORE_LINE] = mi[MAX_LINE] - dchunk;
	    mi[STATUS_BYTE] &= ~M_AUTO_END;
	    break;
	case "r":
	    mi[MORE_LINE] -= dchunk;
	    break;
        case "u":
	case "b":
	case "-":
	    mi[MORE_LINE] -= 2 * dchunk;
	    mi[STATUS_BYTE] &= ~M_AUTO_END;
	    break;
	case "d":
	case "D":
	    mi[MORE_LINE] -= dchunk / 2;
	    mi[STATUS_BYTE] &= ~M_AUTO_END;
	    break;
	case "B":
	case "U":
	    mi[MORE_LINE] -= (3 * dchunk) / 2;
	    mi[STATUS_BYTE] &= ~M_AUTO_END;
	    break;
	case "/":
	    mi[STATUS_BYTE] &= ~M_AUTO_END;
	    if (mi[STATUS_BYTE] & M_CHARMODE)
	    {
		input_to(#'find_string,0,0,mi);
		return INPUT;
	    }
	    return find_string(0,0,mi);
        case "c": //Rest durchscrollen
            mi[STATUS_BYTE] |= M_SCROLL;
            break;
	case "?":
	    if(wizp(this_object()))
		cat(HELP_PATH+"/goetter/zauberstab/more");
	    else
		cat(HELP_PATH+"/more");
	    return NOTHING;
	default:
	    if (!strlen(str))
		break;
	    else if (sscanf(str,"%d",tmp)==1)
	    {
		mi[STATUS_BYTE] &= ~M_AUTO_END;
		mi[MORE_LINE] = tmp - 1;
		if (mi[MORE_LINE] >= mi[MAX_LINE])
		    mi[MORE_LINE] = mi[MAX_LINE] - dchunk;
	    }
	    else if (str[0] == '/')
	    {
		mi[STATUS_BYTE] &= ~M_AUTO_END;
		return find_string(str[1..],0,mi);
	    }
	    else if (str[0] == 'F')
	    {
	         if (sizeof(str)<=2 || str[1]!=' ') {
	            NOTIFY("Bitte den Fehlertext angeben: F text\n");
	            return NOTHING;
	         }
	         if (mi[MORE_FILE]) {
	             if (this_object()->fehler_an_datei(
	                 str[1..],mi[MORE_FILE],
	                 (["more_id":mixed2str(mi[MORE_ID])]) )) {
	               NOTIFY("Fehler gespeichert.\n");
	             } else {
	               NOTIFY("Fehler nicht gespeichert.\n");
	             }
	         } else if (objectp(mi[THEPO]))
	         {
	             if (this_object()->fehler_an_po(
	                 str[1..],mi[THEPO],
	                 (["more_id":mixed2str(mi[MORE_ID])]) )) {
	               NOTIFY("Fehler gespeichert.\n");
	             } else {
	               NOTIFY("Fehler nicht gespeichert.\n");
	             }
	         } else {
	             NOTIFY(wrap("Intern: Fehler konnte nicht "
	                 "zugordnet werden."));
	         }
	         return NOTHING;
	    }
	    else if (str[0] == 'w' && wizp(this_object()))
	    {
		if((strlen(str)>1 && str[1] != ' ') ||
		    !strlen(str=trim(str[2..<1])))
		{
		    NOTIFY("w Dateiname\n");
		    return NOTHING;
		}
		str = this_object()->add_path(str);
		if(!valid_file_name(str))
		{
		    NOTIFY("Ungültiger Dateiname.\n");
		    return NOTHING;
		}
		if(file_size(str)==FSIZE_DIR)
		{
		    NOTIFY("'"+str+"' ist ein Verzeichnis!\n");
		    return NOTHING;
		}
		if(file_size(str)>0) // =0 koennen wir ruhig ueberschreiben
		{
		    input_to(#'ask_overwrite, INPUT_PROMPT,
			"'"+str+"' überschreiben? ", str,mi);
		    return INPUT;
		}
		else
		    save_text(str,mi);
		return NOTHING;
	    }
    }

    if (mi[MORE_LINE] < 0)
	mi[MORE_LINE] = 0;

    if (mi[MORE_LINE] >= mi[MAX_LINE]
     && !(mi[STATUS_BYTE] & M_DO_NOT_END))
	return END_MORE;

    return CONTINUE;
}

private void prompt_more(mixed *mi)
{
    string str;
    str=funcall(mi[STAT][0],mi[MORE_LINE],
        mi[MAX_LINE],mi[MORE_ID]);

    if(!mi[THEPO]) // Objekt ist weg...
    {
	end_more(mi, 0);
	return;
    }

    in_more = 1;
    
    input_to(#'even_more,
	INPUT_PROMPT | ((mi[STATUS_BYTE] & M_CHARMODE)?INPUT_CHARMODE:0),
	lambda(0,({#',,({#'=,#'in_more,1}),sprintf(str||"",mi[MORE_LINE],mi[MAX_LINE])})),
	mi);
}

private void end_more(mixed *mi, string str)
{
    if (mi[THEPO])
	mi[THEPO]->more_end(str,mi[MORE_LINE],mi[MAX_LINE],mi[MORE_ID]);

    in_more = 0;
}

private int insecure_more(mixed *mi)
{
    int res;
    
    if (mi[THEPO] &&
	call_resolved(&res, mi[THEPO], "more_insecure",
	mi[MORE_LINE],mi[MAX_LINE],mi[MORE_ID]))
	    return res;

    return END_MORE;
}

private void even_more(string str, mixed *mi)
{
    int ac;

    this_object()->set_active_prompt(0);
    
    ac = add_line(str, mi);
    if(ac==CONTINUE && (mi[STATUS_BYTE]&M_SECURE) &&
#if __EFUN_DEFINED__(query_snoop)
	query_snoop(this_object()))
#else
        efun::interactive_info(this_object(), II_SNOOP_NEXT))
#endif
	    ac = insecure_more(mi);
    
    switch(ac)
    {
      case CONTINUE:
	if(mi[MORE_LINE]<mi[MAX_LINE])
	    mi[MORE_LINE]+=catting(mi[MORE_LINE]+1,mi);
	if ((mi[STATUS_BYTE] & M_AUTO_END) && (mi[MORE_LINE]>=mi[MAX_LINE]))
	{
	    end_more(mi,str);
	    return;
	}
        prompt_more(mi);
	return;
      case END_MORE:
        end_more(mi,str);
	return;
      case NOTHING:
        prompt_more(mi);
	return;
      case INPUT:
	return;
    }
}

private void ask_overwrite(string yesno, string file, mixed *mi)
{
    if(!strlen(yesno))
    {
	input_to(#'ask_overwrite, INPUT_PROMPT,
	    "'"+file+"' überschreiben? Ja/Nein? ",file,mi);
	return;
    }
    yesno = lower_case(yesno);
    if(!strstr("nein",yesno) || !strstr("nö",yesno))
    {
        prompt_more(mi);
        return;
    }
    if(strstr("ja",yesno) && strstr("yes",yesno) && strstr("klar",yesno))
    {
        input_to(#'ask_overwrite, INPUT_PROMPT,
    	    "'"+file+"' überschreiben? Ja/Nein? ",file,mi);
        return;
    }
    if(!rm(file))
    {
	NOTIFY("'"+file+"' konnte nicht gelöscht werden.\n");
    }
    else
	save_text(file,mi);
    prompt_more(mi);
}

private string check_read_file(string str)
{
    int size;

    if (!(MASTER_OB->valid_read(str,geteuid(),"print_file",this_object())))
    {
	if(wizp(this_object()) || testplayerp(this_object()))
	    return "3 Kann File '"+str+"'nicht lesen.\n";
	return "3 Du kannst den Text nicht lesen.\n";
    }
    if ((size = file_size(str)) < 0)
    {
	if(wizp(this_object()) || testplayerp(this_object()))
	    return "4 File '"+str+"' nicht gefunden.\n";
	return "4 Der Text fehlt.\n";
    }
    if (size == 0)
	return "5 File ist leer.\n";
    if (size > MAX_SIZE)
	return "7 File ist zu lang.\n";
}

int query_client_width();
private string more_internal(string|string* str, int begin_line, mixed *mi)
{
    string tmp;

    if (begin_line == 0)
	begin_line = 1;

    if (stringp(str))
    {
	if (tmp = check_read_file(str))
	    return tmp;
	mi[MORE_FILE]=str;
	mi[MAX_LINE]=file_lenght(str);
    }
    else
    {
	mi[ARR]=foldl(str, ({}), function string*(string *anfang, string text) : int width = wizp(this_player()) ? query_client_width() : 79
	{
	    string wrapped = terminal_colour(
	        regreplace(regreplace(text, "(\n|)\r[\r\n]*", "\n", 1),
	            "(\e(\\[[^a-zA-Z]*[a-zA-Z]|[A-Za-z]))|%\\^", "%^\\1%^", 1),
	            function string(string str) { return str; }, width, 0);
	    if(sizeof(wrapped) && wrapped[<1] == '\n')
	        return anfang + explode(wrapped, "\n")[0..<2];
	    else
	        return anfang + explode(wrapped, "\n");
	});
	
	mi[MAX_LINE]=sizeof(mi[ARR]);
	if (mi[MAX_LINE]<=0 && !(mi[STATUS_BYTE]&M_FORCE))
	    return "6 Das Array ist leer.\n";
    }

    if (begin_line < 0)
    {
        begin_line = mi[MAX_LINE] + begin_line + 1;
        if (begin_line < 1)
	    begin_line = 1;
    }

    // Wenn ueber die Grenze hinaus, dann zeigen wir alles bis zum Ende
    if (begin_line>mi[MAX_LINE] && 
        !(mi[STATUS_BYTE] & M_NO_FIRST_SCREEN))
    {
	int diff_chunk=query_more_chunk();
	if (mi[STATUS_BYTE] & M_FRAME)
	    diff_chunk-=2;
	else if (mi[STATUS_BYTE] & M_HEADER_LINE)
	    diff_chunk--;

        begin_line=mi[MAX_LINE]-diff_chunk+1;
        if (begin_line<1)
	    begin_line=1;
    }
    else if (begin_line>mi[MAX_LINE])
        begin_line = mi[MAX_LINE]+1;
    if((begin_line<=mi[MAX_LINE] || mi[STATUS_BYTE]&M_FORCE) &&
        !(mi[STATUS_BYTE]&M_NO_FIRST_SCREEN))
	mi[MORE_LINE]=begin_line-1+catting(begin_line,mi);
    else
	mi[MORE_LINE]=begin_line-1;

    if ((mi[STATUS_BYTE] & M_AUTO_END) && (mi[MORE_LINE]>=mi[MAX_LINE]))
    {
        end_more(mi,0);
	return 0;
    }
    prompt_more(mi);
    return 0;
}

varargs string more(string|string* str, mixed stats, int begin_line, int byte, mixed more_id)
{
    mixed *mi;
    
    if (!stringp(str) && !pointerp(str))
	return "2 Kein Filename oder Array übergeben.\n";
    
#if __EFUN_DEFINED__(query_snoop)
    if ((byte & M_SECURE) && query_snoop(this_object()))
#else
    if ((byte & M_SECURE) && interactive_info(this_object(), II_SNOOP_NEXT))
#endif
	return "8 Du wirst beobachtet.\n";

    if(stringp(stats) || closurep(stats))
	stats = ({stats,LINE,LINE,CONT_LINE,CONT_LINE});
    else if(pointerp(stats))
	stats += ({NORMAL_STAT, LINE, LINE, CONT_LINE, CONT_LINE})[sizeof(stats)..<1];
    else
	stats = ({NORMAL_STAT, LINE, LINE, CONT_LINE, CONT_LINE});
    
    mi=({ 0, 0, 0, 0, 0, byte, previous_object(), stats, more_id});
    
    if(this_player() != this_object())
	return call_with_this_player(#'more_internal, str, begin_line, mi);
    else
	return more_internal(str, begin_line, mi);
}


/*
FUNKTION: query_more_chunk
DEKLARATION: int query_more_chunk()
BESCHREIBUNG:
Mit dieser Funktion kann man abfragen, nach wievielen Zeilen beim more
gewartet wird.
VERWEISE: more,set_more_chunk
GRUPPEN: Dateien
*/
int query_more_chunk()
{
    mixed sb;
    int sub;
    
    if(chunk)
	return chunk;

    if(this_object()->query_client_option(CLIENT_VT100))
	sub = 3;

    if(this_object()->query_telnet(TELOPT_NAWS,&sb) && sizeof(sb) > 1 && sb[1]>4)
	return sb[1]-1-sub;

    return DEFAULT_CHUNK-sub;
}

nomask int query_real_more_chunk()
{
    return chunk;
}

/*
FUNKTION: set_more_chunk
DEKLARATION: void set_more_chunk(int chunk)
BESCHREIBUNG:
Mit dieser Funktion kann man setzen, nach wievielen Zeilen beim more
gewartet wird.
VERWEISE: more,query_more_chunk
GRUPPEN: Dateien
*/
void set_more_chunk(int i)
{
    if(!i)
	chunk = 0;
    else if (intp(i) && i > 0)
	chunk = i < MAX_CHUNK ? i : MAX_CHUNK;
}

/*
FUNKTION: query_in_more
DEKLARATION: int query_in_more()
BESCHREIBUNG:
Liefert 1, wenn gerade ein more() laeuft, 0 anderenfalls.
VERWEISE: more
GRUPPEN: Dateien
*/
int query_in_more()
{
    if(!in_more)
	return 0;
	
    if(in_more<0 || query_input_pending(this_object()))
	return 1;
    
    in_more = 0;
}
