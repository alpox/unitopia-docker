// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /i/tools/dein_name.c
// Description: <dein_name>/ich-Befehl der Seele
// Author:      Pulami (03.03.94)
// Modified by: Monty (07.06 96): Nicht nur $, sondern auch & wird als
//		                  Maskierungszeichen erlaubt.
//		Freaky (02.06.1999) wrap_emote eingefuehrt.
//		Parsec (03.09.2000) Fuer Monster comm und emote eingebaut
//              Gnomi  (22.12.2000) Am Anfang auch 2-4. Fall zulassen.
//              Imp y Cellyn (03.07.2002) query_emote_case + tell_emote +
//                   parse_emote
//              Imp y Cellyn (04.07.2002) Fehler in parse_emote behoben

// Falls mehr Grammatikfunktionen implementiert werden sollen:
// An DREI bis VIER Stellen ist der Quelltext zu erweitern.

#pragma save_types
#pragma strong_types

#include <parse_com.h>
#include <message.h>
#include <commands.h>

#define MASK	({"$","&"})

// Bei zusaetzlichen Funktionen muessen noch die Funktionen pronemen
// und beschreibung geaendert werden.
#define ALLOWED_FUNS ({"der","des","dem","den","Der","Des", \
		       "Dem","Den","deR","deS","deM","deN"})

// Befehls-Ausfuehrung ------------------------------------------

// Haengt an jedes Element des Stringfeldes daran den String daher an, falls das
// zugehoerige Element in wer nicht wernicht ist.
// Wernichts daran-Element bekommt sonst angehaengt.
private string *to_array(string *daran, string daher, mixed *wer,
    mixed wernicht, string sonst)
{
   int i;
   for(i=0;i<sizeof(daran);i++)
   {
      if(wer[i]!=wernicht)
      {
         daran[i]+=daher;
      }
      else
      {
         daran[i]+=sonst;
      }
   }
   return daran;
}

private string wrap_emote( string str)
{
    return wrap( str) ;

    // Es gibt jetzt ja Farben ... Parsec & Freaky
    // return sprintf("%s%=-67s\n",str[0..7],str[8..]);
}


private string wrap_comm( string str)
{
    int  i ;

    if ( (i = strstr( str, ":")) == -1 )
        return wrap( str) ;
    else
        return wrap_say( str[0..i], regreplace( str[i+1..], "^  *", "", 1)) ;
}


/*
FUNKTION: tell_emote
DEKLARATION: void tell_emote(string *ausgabe, <object|mapping>* an_wen_separat, object wer, string befehl, int msg_action, int msg_type)
BESCHREIBUNG:
Gib das aus, was parse_emote zurueckliefert

Die Parameter im einzelnen:
  *ausgabe:        Ist das 1. Element im Rueckgabearray von parse_emote
  *an_wen_separat: Ist das 2. Element im Rueckgabearray von parse_emote
                   (Kann Objekte und V-Items enthalten)
  wer:             Wer emotet?
  befehl:          query_verb()
  msg_type:        Ein MA_T Define aus message.h oder < 0, wenn die Funktion
                   Default-Werte benutzen soll
  msg_action:      Ein MA_X Define aus message.h oder < 0, wenn die Funktion
                   Default-Werte benutzen soll

VERWEISE: parse_emote, tell_emote
GRUPPEN: monster, spieler, seele
*/
void tell_emote(string *ausgabe, <object|mapping> *an_wen_separat, object wer,
  string befehl, int msg_type, int msg_action)
{
    int i;

    if ( befehl == "comm" || befehl == "comm:" )
    {
        if(msg_action < 0)
          msg_action = MA_COMM ;
        if(msg_type < 0)
          msg_type   = MT_NOISE ;
        ausgabe = map( ausgabe, #'wrap_comm) ;
    }
    else
    {
        if(msg_type < 0)
          msg_type = MT_UNKNOWN ;
        if(msg_action < 0)
        {
            if ( playerp (environment()) ||
		 befehl == "emote" || befehl == "emote:" )
                msg_action = MA_EMOTE;
            else
                msg_action = MA_UNKNOWN;
        }

        ausgabe = map( ausgabe, #'wrap_emote) ;
    }

    // Nachricht [0] an alle schicken, die nicht in an_wen_separat sind
    wer->send_message(
        msg_type, msg_action,
        ausgabe[0], 0, filter(an_wen_separat, #'objectp));
    // Ist "wer" nicht in an_wen_separat, dann kriegt auch er sie
    if( member(an_wen_separat,wer) < 0 )
        this_player()->send_message_to(
            wer, msg_type, msg_action,
            ausgabe[0]);
    // Zugehoerige Nachrichten an alle schicken, die in an_wen_separat
    // enthalten sind
    for( i = 1 ; i < sizeof( an_wen_separat) ; i++ )
    	if ( objectp( an_wen_separat[i]) )
	    wer->send_message_to(
                an_wen_separat[i], msg_type,
	    	msg_action, ausgabe[i]);
}

#if 0
// Gibt den entsprechenden Text an die entsprechenden Personen aus.
void tell( string *ausgabe, object *an_wen_separat)
{
    int i, msg_action, msg_type;

    if ( query_verb() == "comm" )
    {
        msg_action = MA_COMM ;
        msg_type   = MT_NOISE ;
        ausgabe = map( ausgabe, #'wrap_comm) ;
    }
    else
    {
        msg_type = MT_UNKNOWN ;
        if ( playerp (environment()) || query_verb() == "emote" )
            msg_action = MA_EMOTE;
        else
            msg_action = MA_UNKNOWN;

        ausgabe = map( ausgabe, #'wrap_emote) ;
    }

    // Nachricht [0] an alle schicken, die nicht in an_wen_separat sind
    this_player()->send_message(
        msg_type, msg_action,
        ausgabe[0], 0, an_wen_separat);
    // Ist TP nicht an an_wen_separat, dann kriegt auch er sie
    if( member(an_wen_separat,this_player()) < 0 )
        this_player()->send_message_to(
            this_player(), msg_type, msg_action,
            ausgabe[0]);
    // Zugehoerige Nachrichten an alle schicken, die in an_wen_separat
    // enthalten sind
    for( i = 1 ; i < sizeof( an_wen_separat) ; i++ )
    	if ( objectp( an_wen_separat[i]) )
	    this_player()->send_message_to(
                an_wen_separat[i], msg_type,
	    	msg_action, ausgabe[i]);
}
#endif

// Pronomen zusammenbauen zum entsprechenden Fall:
string pronomen(string fall)
{
    return ({"du","deiner","dir","dich","Du","Deiner","Dir","Dich",
	"Du","Deiner","Dir","Dich"})[member(({"der","des","dem",
	"den","Der","Des","Dem","Den","deR","deS","deM","deN"}),fall)];
}

// Objektbeschreibung zusammenbauen zum entsprechenden Fall:
string beschreibung(string fall, mixed wer)
{
    switch(fall)
    {
        case "der": return der(wer);
        case "des": return des(wer);
        case "dem": return dem(wer);
        case "den": return den(wer);
        case "Der": return Der(wer);
        case "Des": return Des(wer);
        case "Dem": return Dem(wer);
        case "Den": return Den(wer);
        case "deR": return der(wer);
        case "deS": return des(wer);
        case "deM": return dem(wer);
        case "deN": return den(wer);
    }
}

mixed dein_name_parse_com(string ziel, object wer)
{
    // WICHTIG: Falls wer != this_player()!
    return parse_com(ziel, ({environment(wer), wer}));
}

/*
FUNKTION: parse_emote
DEKLARATION: mixed parse_emote(string eingabe, int flagge, object wer, string befehl)
BESCHREIBUNG:
Parsed den Seele-Befehl "eingabe".

Die Parameter im einzelnen:
 eingabe:   zu parsende Eingabe ohne query_verb()
            siehe query_emote_case => rest_string
 flagge:    Fall des Emotes.
            siehe query_emote_case
 wer:       Person, die emotet
 befehl:    query_verb()

Beispiel:
---------
- "ich huepft wild umher"
  - eingabe: "huepft wild umher"
  - flagge:  0
  - wer:     [Ausfuehrender]
  - befehl:  "ich"

- ";Ohren wackeln"
  - eingabe: "Ohren wackeln"
  - flagge:  2
  - wer:     [Ausfuehrender]
  - befehl:  ";Ohren"

Die Funktion liefert folgendes zurueck:
---------------------------------------
- im Fehlerfall:
  einen String mit der Fehlermeldung

- im Erfolgfall
   Array mit zwei Elementen:
   [0]: Message-Array mit x Elementen:
        [0]: Msg an alle, die NICHT im Personen-Array drinstehen
        [1..x]: Msg fuer 1-xte Person im Personen-Array
   [1]: Personen-Array mit x Elementen:
        [0]: Ist immer 0
        [1..x]: Personen, die die xte Meldung aus dem Message-Array erhalten

VERWEISE: query_emote_case, tell_emote
GRUPPEN: monster, spieler, seele
*/
mixed parse_emote(string eingabe, int flagge, object wer,
  string befehl)
{
    // Neuer ich-Befehl.
    // parsed Player-Beschreibungen raus zur Unterscheidung
    // von 'den <Person>' und 'Dich' etc.

    mixed *personen;    // Die Ausgabe an Personen; ausgabe[0] ist
                        // die allgemeine Ausgabe. Kann auch vitems beinhalten.
    string *ausgabe;    // Liste von Personen mit separater Ausgabe;
                        // person[0] ist 0.
    string wort;        // Zum Zwischenspeichern von der aktuellen
                        // Ausgabe.
    string artikel;     // Enthaelt die Grammatikfunktion.
    string ziel;        // Enthaelt die Person, auf die die
                        // Grammatikfunktion angewendet
                        // werden soll.
    string modus;       // Parsermodus.
    int j;              // Schleifenzaehler.
    int first;          // 1, wenn die erste Grammatikfunktion der Spieler
                        // selber sein muss (und gross geschrieben werden soll)
    mixed ob;           // Das Zielobjekt.
    mixed sub;          // Das Subjekt fuer die Verbenfunktion.
    string verb1, verb2;// Die Verben; verb1 ist das allgemeine,
                        // verb2 ist das spezielle Verb.
    string pe_ret;      // Returnstring fuer parse_com_error_string

    if(eingabe=="" || !eingabe)
        return "WAS willst Du von Dir geben?\n" ;

    switch(flagge)
    {
        case 0: // Normaler Aufruf.
            personen=({0});                     // Personen initialisiert.
            ausgabe=({beschreibung("Der",wer)+" "}); // Allgemeine Ausgabe.
            break;
        case 1: // Aufruf durch $Der(<dein_name>).
            wort=befehl;
            if(member(MASK,wort[0..0])>=0)
            {
                first=1;
                eingabe=wort+" "+eingabe;
                personen=({0});
                ausgabe=({""});
            }
            else
            {
                personen=({0,wer});
                ausgabe=({beschreibung("Der",wer)+" ","Du "});
                sub=wer;
            }
            break;
        case 2: // Genitiv
            personen=({0});                     // Personen initialisiert.
            ausgabe=({beschreibung("Des",wer)+" "}); // Allgemeine Ausgabe.
            if(lower_case(befehl[0..3])=="mein")
            {
              personen+=({wer});
              ausgabe+=({"D"+befehl[1..<1]+" "});
            }
            break;
        case 3: // Dativ
            personen=({0,wer});             // Personen initialisiert.
            ausgabe=({beschreibung("Dem",wer)+" ","Dir "}); // Allgemeine Ausgabe.
            break;
        case 4: // Akkusativ
            personen=({0,wer});             // Personen initialisiert.
            ausgabe=({beschreibung("Den",wer)+" ","Dich "}); // Allgemeine Ausgabe.
            break;
	case 5: // Gar nix voranstellen
	    personen = ({0});
	    ausgabe = ({""});
	    break;
    }

    wort="";
    modus="";
    ob=wer;

    for(j=0;j<strlen(eingabe);j++)
    {
        switch(modus)
        {
            case "":
        	if(member(MASK, eingabe[j..j])!=-1)
        	    modus="$";
        	else
        	    wort+=eingabe[j..j];
        	break;
            case "$":
        	switch(eingabe[j..j])
                {
        	    case "$":
                        wort+="$";
                        modus="";
                        break;
        	    case "&":
                        wort+="&";
                        modus="";
                        break;
        	    case "(":
                        modus="verb1";
                        verb1="";
                        break;
        	    default:
                        modus="artikel";
                        artikel=eingabe[j..j];
                        break;
                }
                break;
            case "artikel":
                if(eingabe[j..j]=="(")
                {
                    modus="ziel";
                    ziel="";
                }
                else
                    artikel+=eingabe[j..j];
                break;
            case "ziel":
                if(eingabe[j..j]==")")
                {
                     mixed *parse;

                    // Ziel berechen:
                    if(lower_case(space(ziel))=="ich")
                        ob=wer;
                    else
                    {
			parse = dein_name_parse_com(ziel, wer);

                        if (pe_ret =
                             parse_com_error_string(parse,befehl+" was?\n",1))
                            return pe_ret;
                        ob = parse[PARSE_OBS][0];
                    }

                    // Ist die angegebene Grammatikfunktion ueberhaupt eine?
                    if(-1==member(ALLOWED_FUNS,artikel))
                    {
                        return(wrap("'"+artikel+"' ist keine "
                            "zulässige Grammatikfunktion."));
                    }

                    // Wird ein neues Subjekt angegeben?
                    if(-1!=member(({"der","Der","deR"}),artikel))
                        sub=ob;

                    if(first)
                    {
                        artikel=capitalize(lower_case(artikel));
                        if(ob!=wer)
                            return "Dein Name muss am Anfang "
                                "des Satzes stehen.\n";
                    }

                    // Ausgabe zusammenbauen:
                    if(-1==member(personen,ob) && (!first || artikel!="Des"))
                    {
                        personen+=({ob});        // Person hinzufuegen.
                        ausgabe+=({ausgabe[0]}); // Ihre Ausgabe ist
                                                 // bisher die allgemeine.
                    }
                    ausgabe=to_array(ausgabe,wort+beschreibung(artikel, ob),
                        personen, ob, wort+pronomen(artikel));

                    modus="";
                    wort="";
                    first=0;
                }
                else
                    ziel+=eingabe[j..j];
                break;
            case "verb1":
                switch(eingabe[j..j])
                {
                    case ",":
                        modus="verb2";
                        verb2="";
                        break;
                    default:
                        verb1+=eingabe[j..j];
                        break;
                }
                break;
            case "verb2":
                switch(eingabe[j..j])
                {
                    case ")":
                    // Ist ein Subjekt definiert?
        	        if(!sub)
                        {
                            return wrap("Du hast bisher kein Subjekt "
                                "angegeben. Das macht man mit $Der(<name>), "
                                "$der(<name>), $deR(<name>).");
                        }
                        // Ausgabe zusammenbauen:
                        if(-1==member(personen,sub))
                        {
                             personen+=({sub}); // Person hinzufuegen.
                             // Ihre Ausgabe ist bisher die allgemeine.
                             ausgabe+=({ausgabe[0]});
                        }
                        ausgabe=to_array(ausgabe,wort+verb1, personen,
                            sub, wort+verb2);

                        modus="";
                        wort="";
                        break;
                    default:
                        verb2+=eingabe[j..j];
                        break;
                }
                break;
        }
    }

    // In welchem Modus befindet sich die Schleife nach Schleifenende?
    switch(modus)
    {
        case "$":
            return wrap("Da steht ein einzelnes '$' am Ende!");
        case "artikel":
            return wrap("'$"+artikel+"' ist keine Grammatikfunktion!");
        case "ziel":
            return wrap("'$"+artikel+"("+ziel+
        	"' ist keine Grammatikfunktion!");
        case "verb1":
            return wrap("'$"+verb1+"' ist keine Verbenfunktion!");
        case "verb2":
            return wrap("'$("+verb1+","+verb2+
        	"' ist keine Verbenfunktion!");
    }

    // Existiert ein Rest des Wortes? Dann noch anhaengen:
    if(strlen(wort)!=0)
        for(j=0;j<sizeof(ausgabe);j++)
            ausgabe[j]+=wort;

    // Kein Fehler aufgetreten
    return ({ ausgabe, personen });
}

int mache( string eingabe, int flagge)
{
  mixed ret;
  
  if(this_player()->knebel_emote_verboten())
    return 1;

  if(stringp(ret = parse_emote(eingabe, flagge, this_player(), query_verb())))
    return notify_fail(ret);

  // Text ausgeben:
  tell_emote(ret[0], ret[1], 
             this_player(), query_verb(), -1, -1);
  return 1;
}

/*
FUNKTION: query_emote_case
DEKLARATION: int query_emote_case(string str, object obj, string rest_string)
BESCHREIBUNG:
Gibt den Wert zurueck, der bei "flagge" in "parse_emote" verwendet werden
muss, damit der "str" ordentlich geparsed werden kann.

Die Funktion liefert -1, wenn der Befehl "str" nicht als Seele-Befehl
erkannt wird.

Die Parameter im einzelnen:
  str:         Zu pruefender Befehl (query_verb() + Reststring)
  obj:         Objekt, was den Befehl ausfuehren soll
  rest_string: In ihm wird der Rest-String gespeichert (ohne query_verb())
               Muss call-by-reference uebergeben werden

VERWEISE: parse_emote, tell_emote
GRUPPEN: monster, spieler, seele
*/
int query_emote_case(string str, object obj, string rest_string)
{
  string qn, qn2;     // query_real_name + genitiv

  if(!str || ((str=trim(str)) == "") || !obj)
    return -1;

  qn = obj->query_real_name();
  qn2 = get_genitiv(qn);

  if(str[0..0] == ":")
  {
    rest_string = str[1..<1];
    return 0;
  }

  if(str[0..4] == "mach " ||
     str[0..5] == "mache " ||
     str[0..3] == "ich " ||
     str[0..(sizeof(qn)-1)]+" " == qn+" " ||
     str[0..(sizeof(qn)-1)]+" " == capitalize(qn)+" ")
  {
    rest_string = implode(explode(str," ")[1..<1], " ");
    return 0;
  }

  if(str[0..0] == "$" ||
     str[0..0] == "&" ||
     str[0..5] == "emote " ||
     str[0..4] == "comm ")
  {
    rest_string = implode(explode(str," ")[1..<1], " ");
    return 1;
  }

  if(str[0..0] == ";")
  {
    rest_string = str[1..<1];
    return 2;
  }

  if(str[0..(sizeof(qn2)-1)]+" " == qn2+" " ||
     str[0..(sizeof(qn2)-1)]+" " == capitalize(qn2)+" " ||
     str[0..4] == "ichs " ||
     str[0..3] == "mein" ||
     str[0..3] == "Mein")
  {
    rest_string = implode(explode(str," ")[1..<1], " ");
    return 2;
  }

  if(str[0..3] == "mir " ||
     str[0..3] == "Mir ")
  {
    rest_string = implode(explode(str," ")[1..<1], " ");
    return 3;
  }

  if(str[0..4] == "mich " ||
     str[0..4] == "Mich ")
  {
    rest_string = implode(explode(str," ")[1..<1], " ");
    return 4;
  }

  return -1;
}


// Befehlsverwaltung: -------------------------------------
// ACHTUNG: Beim Hinzufuegen neuer Befehle unbedingt query_emote_case
// anpassen

int dein_name(string str) {return mache(str, 0);}
int dein_name_2 (string str)
{
    if (this_player()->query_gilde() != "Magiergilde")
        return mache (str, 0);
    return 0;
}

int deines_namens(string str) { return mache(str, 2); }
int deinem_namen(string str)  { return mache(str, 3); }
int deinen_namen(string str)  { return mache(str, 4); }
int dollar_dein_name(string str) {return mache( str, 1);}
int mache_echo(string str) { return mache(str, 5); }

private void add_my_actions()
{
    string name;
    
    add_action("dein_name","ich");
    add_action("dein_name","mache",-4);
    add_action("dein_name_2",":",AA_NOSPACE);

    name=this_player()->query_real_name();
    if( !name )
        name=this_player()->query_name();

    add_action("dein_name",name);
    add_action("dein_name",capitalize(name));
    add_action("deines_namens",get_genitiv(name));
    add_action("deines_namens",get_genitiv(capitalize(name)));
    add_action("deines_namens",";",AA_NOSPACE);
    add_action("deines_namens","ichs");
    add_action("deines_namens","mein",AA_SHORT);
    add_action("deines_namens","Mein",AA_SHORT);
    add_action("deinem_namen","mir");
    add_action("deinem_namen","Mir");
    add_action("deinen_namen","mich");
    add_action("deinen_namen","Mich");
    add_action("dollar_dein_name","$",AA_SHORT);
    add_action("dollar_dein_name","&",AA_SHORT);

    if ( !playerp( this_player()) )
    {
        add_action( "dollar_dein_name", "emote") ;
        add_action( "mache_echo", "emote:") ;
        add_action( "dollar_dein_name", "comm") ;
        add_action( "mache_echo", "comm:") ;
    }
}

void init()
{
    // Falls es im Monster inheritet wurde
    if(!living(this_object()))
	add_my_actions();
}

void create()
{
    if(this_player()==this_object())
	add_my_actions();
}
