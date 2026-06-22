// ----------------------------------------------------------------
// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/i/living/voice.c
// Description: Kommandos im Player und Monster zur Kommunikation
// Modified by: Freaky (10.03.1998) message auf send_message umgebaut.
//              Sissi  (20.06.1999) reden zu mehreren Leuten gleichzeitig
//              Parsec (24.09.1999) Kommunikationskram aus Seele hier her
//              Parsec (04.01.2000) sage, frage, tuschle filtern exec_command-Ids

#pragma save_types
#pragma strong_types

inherit "/i/tools/soul_hlp";
//inherit "/p/parsec/seele/soul_hlp";

#include <apps.h>
#include <commands.h>
#include <config.h>
#include <deklin.h>
#include <hlp.h>
#include <input_to.h>
#include <invis.h>
#include <level.h>
#include <message.h>
#include <parse_com.h>
#include <portal.h>
#include <soul.h>
#include <stats.h>
#include <term.h>
#include <udp.h>

#define ENV(t)          environment(t)
#define COMM(x)         add_sum_comm(1)
#define owner           this_object()
#define IS_GHOST        (owner->query_ghost())
#define NO_GHOST        if( IS_GHOST ) \
 { notify_fail("Als Geist ist dir das leider nicht moeglich!\n"); return 0; }

#define FRAGE_MIT_PARTNER      1

// Prototypes:
void add_sum_comm(int i);
varargs void notify_message(string msg, int type);


#define FAIL(x) { notify_fail(x); return 0; }

#ifdef NEW_STATS
#define CHECK_INT \
    if (this_object()->query_stat(STAT_INT) < MIN_INT_COMM) \
    { \
	notify_fail("Dazu bist du nicht intelligent genug.\n"); \
	return 0; \
    }
#else
#define CHECK_INT ;
#endif

private static string partner;
private static string away;
private static int in_conversation;


void create()
{
    soul_hlp::set_owner( this_object()) ;
}
/*
FUNKTION: modify_comm 
DEKLARATION: void modify_comm(mapping data, string aktion, mixed opfer, object verursacher)
BESCHREIBUNG:
modify_comm wird bei jeder Kommunikation von verursacher im Raum raum in
allen mit verursacher->add_controller("modify_comm", other) oder
mit raum->add_controller("modify_comm", other) angemeldeten
Controllern other aufgerufen. Dabei wird data als Referenz uebergeben.
D.h. jede Veraenderung muss bei data direkt geschehen.
Die Variable data ist ein Mapping, welche folgende Eintraege beinhalten kann:

    "satz":	Enthaelt das, was derjenige aussprechen will.
    "adverb":	Ein Array aus Adverben, welches vor den eigentlichen Satz
                gestellt werden soll. (Kann auch 0 sein.)
    "flags":    Eine Kombinationen folgender Flags (definiert in soul.h):

                VOICE_FLUESTERMODUS     Umstehende hoeren den Satz nicht.
                VOICE_TUSCHELMODUS      Umstehende hoeren den Satz teilweise.
                VOICE_LALLEN            Der Satz wird gelallt.
                VOICE_LISPELN           Der Satz wird gelispelt.
                VOICE_STOTTERN          Der Satz wird gestottert.
                VOICE_SUMMEN            Der Satz wird gesummt.
                VOICE_NUSCHELN          Der Satz wird genuschelt.
                VOICE_HISSEN            Der Satz wird gehisst.
                VOICE_ZISCHEN           Der Satz wird gezischt.

Die angegebene Aktion kann folgende sein:
     sag, fluester, frag, antwort, bemerk, bruddel, denk, erlaeuter, kreisch,
     lall, lispel, murmel, sing, stotter, summ, traeller, tuschel, verkuend
     
Beispiel:
Du hast eine Art Megaphon gmacht und willst nun, dass bei jedem sage, alles
in Grossbuchstaben kommt. Du machst also ein Megaphone, in diesem Megaphone
machst Du ein 'spieler->add_controller("modify_comm",this_object());
Aufruf rein und hast folgende Funktion im Code:

    void modify_comm(mapping data, string aktion, mixed opfer, object verursacher)
    {
    	if (aktion == "sag") // Wir wollen nur auf sage reagieren
	{
	    data["satz"] = upper_case(data["satz"]);
        }
    }

VERWEISE: modify, notify_comm, forbidden_comm, add_controller, do_change_comm
GRUPPEN: seele, spieler, monster
*/

/*
FUNKTION: do_change_comm 
DEKLARATION: public nomask string do_change_comm(string to_modify,int flags)
BESCHREIBUNG:
do_change_comm liefert die Stringumwandlung aufgrund folgender Flags
und wird von modify_comm aufgerufen:
    "flags":    Eine Kombinationen folgender Flags (definiert in soul.h):
                VOICE_LALLEN            Der Satz wird gelallt.
                VOICE_LISPELN           Der Satz wird gelispelt.
                VOICE_STOTTERN          Der Satz wird gestottert.
                VOICE_SUMMEN            Der Satz wird gesummt.
                VOICE_NUSCHELN          Der Satz wird genuschelt.
                VOICE_HISSEN            Der Satz wird gehisst.
                VOICE_ZISCHEN           Der Satz wird gezischt.
VERWEISE: modify_comm
GRUPPEN: seele, spieler, monster
*/
public nomask string do_change_comm(string to_modify,int flags)
{
    string out = "";
    string str = to_modify;
    
    if(flags & VOICE_LALLEN)
    {
        out = "";
        str = to_modify;

        for(int i=0;i<strlen(str);i++)
        {
            if(!random(4))
            {
            switch(str[i..i])
            {
                case "l" :
                    out+="ll";
                    break;
                case "t" :
                    out+="dd";
                        break;
                case "k" :
                        break;
                case "f" :
                        out+="w";
                        break;
                case "i" :
                        out+="ie";
                        break;
                case "g" :
                        break;
                case " " :
                    if(!random(4))
                            out+=" *hick* ";
                    else
                            out+=" ";
                    break;
                default  :
                        out+=str[i..i];
                        break ;
                }
            }
            else
            out+=str[i..i];
        }
	
        to_modify = out;
    }
    if(flags & VOICE_LISPELN)
    {
        out = "";
        str = to_modify;
        
        for (int i=0; i<strlen(str); i++)
        {
            string add;
            
            switch (lower_case(str[i..i]))
            {
            case "x":
                if (i<strlen(str)-1 && lower_case(str[i+1..i+1])=="x")
                    i+=1;
                add ="kth";
                break;
            case "c":
                if (i<strlen(str)-2 && lower_case(str[i+1..i+2])=="hs")
                {
                    i+=2;
                    add ="kth";
                } else 
                    add = str[i..i];
                break;
            case "s":
                if (i<strlen(str)-1 &&( lower_case(str[i+1..i+1])=="s" ||
                    lower_case(str[i+1..i+1])=="h" || 
                    lower_case(str[i+1..i+1])=="z"))
                    i+=1;
                if (i<strlen(str)-2 && lower_case(str[i+1..i+2])=="ch")
                    i+=2;
                add = "th";
                break;
            case "z":
                if (i<strlen(str)-1 && lower_case(str[i+1..i+1])=="z")
                    i+=1;
                add = "dth";
                break;
            case "t":
                if (i<strlen(str)-1 &&(lower_case(str[i+1..i+1])=="z") ||
                            lower_case(str[i+1..i+1])=="s") {
                    i+=1;
                    add = "dth";
                } else 
                    add = str[i..i];
                break;
            default:
                add = str[i..i];
            }
            if(lower_case(str[i..i]) != str[i..i])
                out += capitalize(add);
            else
                out += add;
            }

        to_modify = out;
    }
    if(flags & VOICE_STOTTERN)
    {
        out = "";
        str = to_modify;

        for (int i=0; i<strlen(str); i++)
        {
            out+=str[i..i];
            switch (random(20))
            {
            case 0:
                    out+=str[i..i]+str[i..i];
                    break;
            case 1:
                out+="..."+str[i..i];
                break;
            case 2:
                out+=" pffff ";
                break;
            }
        }
        to_modify = out;
    }
	
    if(flags & VOICE_SUMMEN)
    {
        out = "";
        str = to_modify;

        for (int i=0; i<strlen(str); i++)
        {
            if (member("aeiou", str[i])!=-1)
                out += "h";
            else if(member("AEIOU", str[i])!=-1)
                out += "H";
            else if(member("!\"$%&/()=,.-;:_+*#' \t", str[i])!=-1)
                out+= str[i..i];
            else if(lower_case(str[i..i])==str[i..i])
                out += "m";
            else
                out += "M";
        }
        to_modify = out;
    }
    if(flags & VOICE_NUSCHELN)
    {
        out = "";
        str = to_modify;
        
        for (int i=0; i<strlen(str); i++)
        {
            string add;
            
            switch (lower_case(str[i..i]))
            {
            case "x":
                if (i<strlen(str)-1 && lower_case(str[i+1..i+1])=="x")
                    i+=1;
                add ="ksch";
                break;
            case "c":
                if (i<strlen(str)-1 &&( lower_case(str[i+1..i+1])=="h" ))
				{
					add ="sch";
                    i+=1;
				} else {
				if (i<strlen(str)-2 && lower_case(str[i+1..i+2])=="hs")
                {
                    i+=2;
                    add ="ksch";
                } else 
                    add = str[i..i];
				}
                break;
            case "s":
                if (i<strlen(str)-1 &&( lower_case(str[i+1..i+1])=="s" ||
                    lower_case(str[i+1..i+1])=="h" || 
                    lower_case(str[i+1..i+1])=="z"))
                    i+=1;
                if (i<strlen(str)-2 && lower_case(str[i+1..i+2])=="ch")
                    i+=2;
                add = "sch";
                break;
            case "z":
                if (i<strlen(str)-1 && lower_case(str[i+1..i+1])=="z")
                    i+=1;
                add = "tsch";
                break;
            case "t":
                if (i<strlen(str)-1 &&(lower_case(str[i+1..i+1])=="z") ||
                            lower_case(str[i+1..i+1])=="s") {
                    i+=1;
                    add = "dsch";
                } else 
                    add = str[i..i];
                break;
            default:
                add = str[i..i];
            }
            if(lower_case(str[i..i]) != str[i..i])
                out += capitalize(add);
            else
                out += add;
            }

        to_modify = out;
    }
    if(flags & (VOICE_ZISCHEN|VOICE_HISSEN))
    {
        out = "";
        str = to_modify;
        
        for (int i=0; i<strlen(str); i++)
        {
            string add;
            
            switch (lower_case(str[i..i]))
            {
            case "x":
                if (i<strlen(str)-1 && lower_case(str[i+1..i+1])=="x")
                    i+=1;
                add ="kss";
                break;
            case "c":
                if (i<strlen(str)-1 &&( lower_case(str[i+1..i+1])=="h" ))
				{
					add ="ss";
                    i+=1;
				} else {
				if (i<strlen(str)-2 && lower_case(str[i+1..i+2])=="hs")
                {
                    i+=2;
                    add ="kss";
                } else 
                    add = str[i..i];
				}
                break;
            case "s":
                if (i<strlen(str)-1 &&( lower_case(str[i+1..i+1])=="s" ||
                    lower_case(str[i+1..i+1])=="h" || 
                    lower_case(str[i+1..i+1])=="z"))
                    i+=1;
                if (i<strlen(str)-2 && lower_case(str[i+1..i+2])=="ch")
                    i+=2;
                add = "sss";
                break;
            case "z":
                if (i<strlen(str)-1 && lower_case(str[i+1..i+1])=="z")
                    i+=1;
                add = "tsch";
                break;
            case "t":
                if (i<strlen(str)-1 &&(lower_case(str[i+1..i+1])=="z") ||
                            lower_case(str[i+1..i+1])=="s") {
                    i+=1;
                    add = "dsch";
                } else 
                    add = str[i..i];
                break;
            default:
                add = str[i..i];
            }
            if(lower_case(str[i..i]) != str[i..i])
                out += capitalize(add);
            else
                out += add;
            }

        to_modify = out;
    }
    return to_modify;
}

// adverb: Angenommen wird 0, ein String oder String-Array.
//         Zurueckgeliefert wird ein String mit vorangehendem Leerzeichen.
// flags:  VOICE_LALLEN, VOICE_LISPELN, VOICE_STOTTERN und VOICE_SUMMEN
//         wird hier ausgewertet. VOICE_FLUESTERMODUS und VOICE_TUSCHELMODUS
//         werden in diesem Parameter zurueckgeliefert.
private string lalle_string(string str);
private string do_modify_comm(mixed opfer, string to_modify, string action, mixed adverb, int flags)
{
    mapping data = (["satz": to_modify]);
    object env;
    
    if(adverb && sizeof(adverb))
        data["adverb"] = stringp(adverb)?({adverb}):adverb;
    
    data["flags"] = flags;

    this_object()->modify("comm", &data, action, opfer, this_object());

    env = environment(this_object());
    if (env)
        env->modify("comm", &data, action, opfer, this_object());

    if(data["adverb"])
    {
        if(pointerp(data["adverb"]) && sizeof(data["adverb"]))
            adverb = " "+liste(data["adverb"]);
        else if(stringp(data["adverb"]) && strlen(data["adverb"]))
            adverb = " "+data["adverb"];
        else
            adverb = "";
    }
    else
        adverb = "";
    
    return do_change_comm(data["satz"],flags = data["flags"]);
}

// teil2 wird nur beim Fluestern und Tuscheln eingesetzt.
private string calc_msg_other(string teil1, string teil2_fluester, string teil3, string satz, int flags)
{
    if((flags & VOICE_FLUESTERMODUS) || space(satz)=="")
	return wrap(implode(({teil1,teil2_fluester,teil3})-({0})," ")+".");
    if(flags & VOICE_TUSCHELMODUS)
    {
	satz = implode(map(explode(satz||""," "),
	    (:
		switch ( random( 10) )
		{
		    case 0..5 : return $1;
		    case 6..7 : return "...";
		    default   : return "";
		}
	    :))-({""})," ");
    
	return wrap(implode(({teil1,teil2_fluester,teil3})-({0})," ")+".") +
		((satz=="")?"":
                 Wrap_say("Du kannst folgendes aufschnappen:", satz));
    }
    return Wrap_say(implode(({teil1,teil3})-({0})," ")+":", satz);
}

// set_owner() nach aussen schuetzen. Darf nicht veraendert werden koennen
private void set_owner( object ob)
{
    soul_hlp::set_owner( ob);
}


/* --- add_actions: --- */

protected void add_actions()
{
    add_action("say_command",	"sage:",-3);
    add_action("say_command",	"sag:");
    add_action("say_command","'",AA_NOSPACE);
    add_action("whisper_command","flüstere",-7);
/*
Funktionen ohne Partner und Adverb sind mit "*" gekennzeichnet, wenn sie
eine Kommndozeile beruecksichtigen, ist noch ein "+" dran...
Wenn ein "P" dransteht, wird ein Partner beruecksichtigt, bei "A" ein
Adverb.
*/
    add_action( "antworte_command",      "antworte:",-7);   	// *+
    add_action( "antworte_command",      "antwort:");   	// *+
    add_action( "bemerke_command",       "bemerke",-6);    	// *+
    add_action( "bruddle_command",       "bruddel");       	// *+
    add_action( "bruddle_command",       "bruddle");
    add_action( "denke_command",         "denke",-4);      	// *
    add_action( "erlaeutere_command",    "erläutere",-8); 	// *+
    add_action( "frage_command",         "frage:",-4);      	// P+
    add_action( "frage_command",         "frag:");     		// P+
    add_action( "groele_command",        "gröle",-4);     	// *+
    add_action( "groele_command",        "groehle",-6);    	// *+
    add_action( "kreische_command",      "kreische",-7);	// *+
    add_action( "lalle_command",         "lalle",-4);      	// *+
    add_action( "lisple_command",        "lisple");     	// *+
    add_action( "lisple_command",        "lispel");
    add_action( "murmle_command",        "murmel");  		// *+
    add_action( "murmle_command",        "murmle");
    add_action( "singe_command",         "singe",-4);   	// *+
    add_action( "stottere_command",      "stottere",-7);   	// *+
    add_action( "summe_command",         "summe",-4);      	// *+
    add_action( "traellere_command",     "trällere",-7);	// *+
    add_action( "tuschle_command",       "tuschle");		// P+
    add_action( "tuschle_command",       "tuschele",-7);
    add_action( "verkuende_command",     "verkünde",-7);	// *+
    add_action( "tell_command",          "rede",-3);            // *+
    add_action( "tell_command",          "teile",-4);           // *+
    add_action( "shout_command",         "brülle",-5);         // *+
}

/*
FUNKTION: set_away
DEKLARATION: void set_away(string str)
BESCHREIBUNG:
Benimmt sich wie das Spieler-Kommando 'weg'. Kann natuerlich auch bei
Monstern gesetzt werden, aber wozu?
VERWEISE: query_away
GRUPPEN: spieler, monster
*/
void set_away(string str) { away = str; }

/*
FUNKTION: query_away
DEKLARATION: string query_away()
BESCHREIBUNG:
Liefert den String zurueck, den ein Spieler mit dem 'weg'-Kommando gesetzt
hat. Klappt natuerlich auch bei Monstern, aber wozu?
VERWEISE: set_away
GRUPPEN: spieler, monster
*/
string query_away()
{
    return away;
}

/*
FUNKTION: wrap_string
DEKLARATION: string wrap_string(string st1, string st2)
BESCHREIBUNG:
Bricht st2 wie im 'sage', 'bruelle' und 'fluestere'-Kommando: Erst wird
st1 erzeugt (zb Der Hanswurst sagt:) dann wird st2 angehaengt und auf
die Breite 75 Zeichen umgebrochen, in der 2. Zeile beginnt die Augabe
von st2 um 8 Zeichen eingerueckt. Es wird allerdings nur die
simul_efun wrap_say() aufgerufen, man nehme also lieber diese.

Machen wir ein Beispiel:
    send_message(MT_NOISE,MA_COMM,wrap_string(Der(this_object())+" sagt:",
	"Ey Mann, da kann ich viel erzaehlen! Also passma auf: Ich sach "+
	"zu meiner Oma, ey Oma, sach ich zu ihr, geh nich raus bei dem "+
	"Glatteis, aber, ey, ich sach Dir, meine Oma hoert mal wieder "+
	"nich! Un dann ... [bla, fasel]")

    erzeugt im Raum (wenn das Monster Hanswurst heisst und maennlich ist):

Der Hanswurst sagt: Ey Mann, da kann ich viel erzaehlen! Also passma
	auf: Ich sach zu meiner Oma, ey Oma, sach ich zu ihr, geh nich
	raus bei dem Glatteis, aber, ey, ich sach Dir, meine Oma hoert
	mal wieder nich! Un dann ... [bla, fasel]
VERWEISE: wrap, wrap_say
GRUPPEN: spieler, monster
*/
string wrap_string(string st1, string st2)
{
    return wrap_say(st1, st2);
}

private string echo_str( string s1, string s2)
{
    if ( this_object()->query_echomode() )
        return Wrap_say( s1, s2) ;
    else
        return "Ok.\n" ;
}

private string korrigiere_sage_puffer(string s1, string s2)
{
    if (!this_object()->query_echomode() )
	this_object()->korrigiere_meldungspuffer("sage","Ok.\n",
	    Wrap_say(s1,s2));
}

int whisper_command( string str)
{
    mixed   *parsed, was ;
    string  tmp, adv ;
    int flags;

    if ( str )
        str = regreplace( str, "^ *(zu|zum|zur) ", "", 1) ;

    parsed = parse_com( str, ({environment(), this_object()}), 0, PARSE_NO_V_ITEMS);
    if ( parse_com_error( parsed, "Flüstere wem was?\n", 1) )
        return 0;

    was = parsed[PARSE_OBS][0];
    str = parsed[PARSE_REST];

    if ( was == owner )
    {
        msg_notify( "Wenn Du schon Selbstgespräche führen "
                    "willst, dann bitte laut, damit auch andere etwas davon "
                    "haben!") ;
        return 1 ;
    }
    else
    {
        if ( str == "" )
        {
            msg_notify( "Was willst du zu "+xdeinem( was)+" flüstern?") ;
            return 1 ;
        }
	flags = VOICE_FLUESTERMODUS;
        str = do_modify_comm(was,str,"fluester", &adv, &flags);
        if ( forbidden_msg_soul_action_partner_notify(
            was, "fluester", str, NEUTRAL, 0,
            echo_str( tmp="Du flüsterst"+adv+" zu "+xdeinem( was)+":", str),
            Wrap_say( Wer( owner, ART_AAA)+" flüstert"+adv+" zu "+xdir( was)+":", str),
	    calc_msg_other( Wer(owner, ART_AAA)+" flüstert"+adv,"etwas",
		"zu "+xseinem(was), str, flags),
            MA_COMM, MT_NOISE, MT_NOISE, MT_NOTIFY) )
            return 1 ;
	korrigiere_sage_puffer(tmp,str);
    }

    COMM( owner) ;
    return 1 ;
}

int away_command(string str)
{
   if(!str)
      if(away)
      {
         set_away(0);
         notify_message("Schön, dass Du wieder da bist.\n");
         return 1;
      }
      else
         FAIL("weg <Meldung>\n");
   set_away(str);
   notify_message("Ok.\n");
   return 1;
}

int say_command( string str)
{
    mixed   *parsed, was ;
    string  tmp, outstr;
    int wiz;

    if ( !str || trim(str) == "" )
    {
        notify_fail( "Sage was?\n") ;
        return 0 ;
    }

    wiz=(query_verb() && query_verb()[0]=='z' && wizp(this_object()));
    str = trim(str);

    if ( !in_conversation && sizeof( regexp( ({ str }), "^ *(zu|zum|zur) ")) &&
	!(query_verb() && query_verb()[<1]==':'))
    {
        tmp = regreplace( str, "^ *(zu|zum|zur) ", "", 1) ;
        parsed = parse_text( tmp, 0, wiz && PARSE_NOSHIMMER_INVIS) ;
    }

    if( away )
        msg_notify( "(Eigentlich bist du ja weg: "+away+")");

    if ( parsed )
    {
        was = parsed[0] ;
        str = parsed[1] ;

        outstr=wiz?("\""+str+"\""):str;
        // exec_command() Id gefunden
        if ( !was )
            return 0 ;

        if ( was == owner )
        {
            if ( str == "" )
            {
                msg_notify( "Was willst du dir sagen?");
                return 1 ;
            }
	    if(wiz)
	    {
		string prefix="["+owner->query_real_cap_name()+"] ";
		this_object()->send_message( MT_NOISE|MT_DEBUG, MA_COMM,
		    wrap(prefix + Wer( owner, ART_AAA) +
            		" führt merkwürdige Selbstgespräche."),
		    Wrap_say(prefix+"Du sagst dir:", outstr), this_object());
	    }
	    else
	    {
		string adv, verb="sag";
		int flags = VOICE_FLUESTERMODUS;

		// Ab 2/3 max_alc lallen.
		if(playerp(this_object()) &&
		    this_object()->query_alc()*3 > this_object()->query_max_alc()*2)
			flags = VOICE_LALLEN;
		
    		outstr=do_modify_comm(was,outstr,"sag",&adv,&flags);
		if(flags&VOICE_LALLEN)
		    verb = "lall";
		    
		if ( forbidden_msg_soul_action_partner_notify(
            	    was, verb, str, NEUTRAL, 0,
        	    Wrap_say( "Du "+verb+"st dir"+adv+":", outstr),
            	    0,
		    Wer( owner, ART_AAA) + ((verb=="sag")
			    ?" führt merkwürdige Selbstgespräche."
			    :" lallt etwas vor sich hin."),
            	    MA_COMM, MT_NOISE) )
            		return 1 ;
	    }
        }
        else
        {
            if ( str == "" )
            {
                msg_notify( "Was willst du zu "+xdeinem( was)+" sagen?") ;
                return 1 ;
            }
            if(wiz)
	    {
                string prefix="["+owner->query_real_cap_name()+"->"+
                       (mappingp(was)
                        ?(QUERY("cap_name",was)||capitalize(QUERY("name",was)))
                        :(was->query_real_cap_name()||was->query_cap_name()))+"] ";
		this_object()->send_message(MT_NOISE|MT_DEBUG, MA_COMM,
            	    Wrap_say( prefix+Wer( owner, ART_AAA)+" sagt zu "+xseinem( was)+":", outstr),
            	    Wrap_say( prefix+Wer( owner, ART_AAA)+" sagt zu "+xdir( was)+":", outstr),
		    was);
		this_object()->send_message_to(this_object(),
		    MT_NOISE|MT_NOTIFY|MT_DEBUG, MA_COMM,
		    echo_str( tmp= prefix+"Du sagst zu "+xdeinem( was)+":", outstr));
	    }
	    else
	    {
		string verb, adv;
		int flags;
		
		if(playerp(this_object()) &&
		    this_object()->query_alc()*3 > this_object()->query_max_alc()*2)
		    flags = VOICE_LALLEN;

        	outstr=do_modify_comm(was,outstr,"sag",&adv,&flags);

		if(flags&VOICE_LALLEN)
		    verb = "lall";
		else
		    verb = "sag";

		if ( forbidden_msg_soul_action_partner_notify(
            		was, verb, str, NEUTRAL, 0,
            	    echo_str( tmp= "Du "+verb+"st"+adv+" zu "+xdeinem( was)+":", outstr),
            	    Wrap_say( Wer( owner, ART_AAA)+" "+verb+"t"+adv+" zu "+xdir( was)+":", outstr),
		    calc_msg_other(Wer(owner, ART_AAA)+" "+verb+"t"+adv,
			"etwas", "zu "+xseinem( was), outstr, flags),
            	    MA_COMM, MT_NOISE) )
            	    return 1;
	    }
	    korrigiere_sage_puffer(tmp,str);
        }
    }
    else
    {
	string verb = "sag";
	string prefix = "";
	string adv = "";
	int flags;
	
        outstr=wiz?("\""+str+"\""):str;
        if(wiz) 
	{
	    prefix="["+owner->query_real_cap_name()+"] ";
	    this_object()->send_message(MT_NOISE|MT_DEBUG, MA_COMM,
		Wrap_say( prefix + Wer( owner, ART_AAA)+" sagt:", outstr));
	    this_object()->send_message_to(this_object(),
		MT_NOISE|MT_NOTIFY|MT_DEBUG, MA_COMM,
		tmp = echo_str( prefix + "Du sagst:", outstr));
	}
	else
	{
	    if(playerp(this_object()) &&
		this_object()->query_alc()*3 > this_object()->query_max_alc()*2)
		    flags = VOICE_LALLEN;

    	    outstr=do_modify_comm(0,outstr,"sag",&adv,&flags);

	    if(flags&VOICE_LALLEN)
	        verb = "lall";
	    else
	        verb = "sag";
		
    	    if(in_conversation)
        	tmp = 0 ;
    	    else
        	tmp = echo_str( "Du "+verb+"st"+adv+":", outstr) ;

    	    if ( forbidden_msg_soul_action_notify(
        	verb, str, NEUTRAL, 0,
        	tmp,
		calc_msg_other(Wer(owner, ART_AAA)+" "+verb+"t"+adv, "etwas",
		    0, outstr, flags),
        	MA_COMM, MT_NOISE) )
        	return 1 ;
	}
	if(tmp)
	    korrigiere_sage_puffer(prefix + "Du "+verb+"st"+adv+":",outstr);
	else
	    this_object()->add_to_meldungspuffer("sage",
		Name(this_object()), Wrap_say(prefix + "Du "+verb+"st"+adv+":",outstr));
    }

    COMM( owner) ;
    return 1 ;
}

int tell_command(string s)
{
#define SR_TAG     0b01
#define ADM_TAG    0b10
#define SRADM_TAG  0b11
#define TAG(x) (([ SR_TAG : "<SR> ", ADM_TAG : "<ADM> ", SRADM_TAG : "<SR+ADM> " ])[x] || "")

    mixed *destinations;
    object destination,portal_guest;
    string *destination_names, destination_string, message, away_message,
        buffer, redebuffer, wer, mud, tmp;
    int kosten, kostenflag, idle_time, teile, i, j, tagflag, 
        portal_flag;
    string *intermud, *aclgroups = ({}), acltag;
    string fail;

    CHECK_INT;
    if ((query_verb()=="teil")||(query_verb()=="teile"))
    {
      teile = 1;
      fail=query_notify_fail();
    }
    
    if(!fail) fail="rede zu wem was?\n";

    
    if (!s || !strlen (s))
        FAIL (fail);

    if (!teile)
        sscanf(s, "zu %s", s);

    // fuehrende Kommas rauswerfen
    while (strlen (s) && ((s[0]==',') || (s[0] == ' ')))
       s = s[1..];
    // ersetze am Anfang(!) alle ", " durch "," (wir wissen, dass s[0] kein Komma ist
    for (i=1, j=1; (i < strlen(s)-2) && j;) {
        if ((s[i] == ',') && (s[i+1] == ' '))
            s = s[0..i]+s[i+2..];
        else if ((s[i] == ' ') && (s[i-1] != ','))
            j = 0;
        else i++;
    }
    // mal schauen, ob noch was uebrig geblieben ist...
    if (!strlen(s))
        FAIL (fail);

    kostenflag = !(wizp(this_object()) || GABE(this_object(),"sb"));

    if (sscanf(s,"%s %s", destination_string, message) != 2)
        FAIL (fail);

    /* Die Angeredeten suchen. */
    destination_names = explode(destination_string,",");

    for (i=sizeof(destination_names), destinations = ({}), intermud = ({}); i--; )
    {
        intermud = ({0}) + intermud;
        portal_flag = 0;
        if (sscanf(destination_names[i],"%s@%s",wer,mud) == 2 && member(wer,' ') < 0)
        {
#ifdef PORTAL_SERVER
            portal_guest = PORTAL_SERVER->query_guest(destination_names[i]);
#endif
            portal_flag = objectp(portal_guest);
            if (!portal_flag && !playerp(this_object()))
                return 0;
            // wo kommen wir denn hin, wenn Monster sich ueber Intermud unterhalten?
            if (portal_flag)
            {
                ; // do nothing??-Myonara@Orbit-
            }
            else if (strstr(lower_case(MUD_NAME), lower_case(mud)) == 0)
               destination_names[i] = wer;
            else if (!(tmp=INETD->known_mud(mud)))
                FAIL ("Das Mud "+capitalize(mud)+" ist unbekannt oder nicht eindeutig.\n"
                    "Das Kommando 'muds' liefert eine Übersicht.\n")
            else if(member(intermud,tmp=lower_case(wer)+"@"+tmp)!=-1)
            {
                destination_names = arr_delete(destination_names,i);
                continue; // Damit das nicht als nicht-Intermudrede gilt.
            }
            else
            {
                intermud [0] = tmp;
                destinations = ({lower_case(destination_names[i])}) + destinations;
                // destination_names[i] = capitalize(lower_case(wer))+"@"+capitalize(mud);
                destination_names[i] = capitalize(tmp);
            }
        }
        if (!intermud[0])
        {
            if (wizp(this_object()) && GROUP_MASTER->group_exists(destination_names[i]))
            {
                string * u2 = ({});
                string * u1 = GROUP_MASTER->query_all_wiz_group_members_of(
                        destination_names[i]);
                foreach (string element: u1)
                {
                    if (member(u2, element) < 0)
                    {
                        u2 += ({ element });
                    }
                }
                object *gm=map(u2, #'find_player)-({0});
                object *igm=filter(gm,#'interactive);
                if (!sizeof(gm))
                    FAIL("Von "+destination_names[i]+" ist gerade niemand anwesend.\n");
                if (!sizeof(igm))
                    FAIL("Von "+destination_names[i]+" sind alle Statue.\n");
                gm -= ({this_object()});
                igm-= ({this_object()});
                if (!sizeof(gm))
                    FAIL("Von "+destination_names[i]+" ist außer dir gerade niemand anwesend.\n");
                if (!sizeof(igm))
                    FAIL("Von "+destination_names[i]+" sind außer dir alle Statue.\n");
                igm = gm-igm;
                gm-=destinations;
                aclgroups += ({ destination_names[i] });
                destinations = gm + destinations;
                destination_names[i..i]=map_objects(gm,"query_real_cap_name");
                if(sizeof(igm))
                    notify_message(wrap(liste(igm->query_real_cap_name())+
                    ((sizeof(igm)>1)?" sind übrigens versteinert."
                        :" ist übrigens versteinert.")),
                    MA_SENSE);
            }
            else if (member((["spielerrat","spielerräte","sr"]),lower_case(destination_names[i])))
            {
            object *sr=map(SPIELERRAT->query_spielerrat(),#'find_player)-({0});
            object *isr=filter(sr,#'interactive);
            if(!sizeof(sr))
                FAIL("Vom Spielerrat ist gerade niemand anwesend.\n");
            if(!sizeof(isr))
                FAIL("Die anwesenden Spielerräte sind zu Statuen erstarrt.\n");
            sr-=({this_object()});
            isr-=({this_object()});
            if(!sizeof(sr))
                FAIL("Vom Spielerrat ist außer dir gerade niemand anwesend.\n");
            if(!sizeof(isr))
                FAIL("Außer Dir sind alle anwesenden Spielerräte zu Statuen erstarrt.\n");
            isr = sr - isr;
            sr-=destinations;
            destinations = sr + destinations;
            destination_names[i..i]=map_objects(sr,"query_real_cap_name");
            if(sizeof(isr))
                    notify_message(wrap(liste(isr->query_real_cap_name())+
                    ((sizeof(isr)>1)?" sind übrigens versteinert."
                        :" ist übrigens versteinert.")),
                    MA_SENSE);
            tagflag |= SR_TAG;
            }
            else if((spielerratp(this_object()) || wizp(this_object())) &&
                    member((["mudadm","adm","admin","admins"]),lower_case(destination_names[i])))
            {
                object * admin = map(ADMINS, #'find_player)-({0});
                object * iadmin = filter(admin, #'interactive);

                admin -= ({ this_object() });
                iadmin -= ({ this_object() });

                if(!sizeof(admin))
                    FAIL((adminp(this_object())?"Außer dir":"Es")+" sind gerade keine Admins anwesend.\n");
                if(!sizeof(iadmin))
                    FAIL("Die anwesenden Admins sind zu Statuen erstarrt.\n");

                iadmin = admin - iadmin;
                admin -= destinations;
                destinations = admin + destinations;
                destination_names[i..i]=map_objects(admin,"query_real_cap_name");

                if(sizeof(iadmin))
                    notify_message(wrap(liste(iadmin->query_real_cap_name())+
                        ((sizeof(admin)>1)?" sind übrigens versteinert."
                                        :" ist übrigens versteinert.")),
                        MA_SENSE);
                tagflag |= ADM_TAG;
            }
            else if(wizp(this_object()) && lower_case(destination_names[i])=="notruf")
            {
                destination_names[i]="dem Notruf";
                destinations = ({
                    (:
                        EVENT_MASTER->event(24, this_object(), $1, $3);
                    :) }) + destinations;
            }
            else if (!(destination = find_living(lower_case(destination_names[i]))))
                FAIL (wrap(capitalize(destination_names[i]) + " ist nicht "
                    "aufzufinden."))
          else
          {
                if (destination == this_object())
                    FAIL("Führst Du Selbstgespräche?\n");
                if (query_once_interactive(destination) && !interactive(destination))
                    FAIL(wrap(Der(destination)+" ist versteinert und würde nicht antworten!"));

	        // im Folgenden aufpassen, dass kein Ziel zwei mal angequatscht
	        // wird und einem Spieler, der ein Ziel doppelt drin hatte, nicht
	        // doppelte Kosten entstehen.
            if(member(destinations,destination) != -1)
                    destination_names = arr_delete (destination_names, i);
            else
            {
                destinations = ({destination}) + destinations;
                if (playerp (destination) &&
                    (destination->query_cap_name() != destination->query_real_cap_name()))
                    destination_names[i] = capitalize(destination_names[i]);
                else if (playerp (destination))
                    destination_names[i] = dem(
                            (["name":destination->query_real_name(),
                        "cap_name":destination->query_real_cap_name(),
                              "gender":destination->query_real_gender(),
                              "personal_title":destination->query_personal_title(),
                              "personal":destination->query_personal()]),
                             destination->query_adjektiv());
                else
                    destination_names[i] = dem(destination);
                if (kostenflag)
                    kosten += TELL_COST;
            }
          }
        }
    }
    if (teile)
	sscanf(message,"mit %s",message);
	
    if(tagflag)
	kostenflag = 0;
    
    if (kostenflag && this_object()->query_sp() < kosten)
        FAIL(wrap("Du hast nicht genug "+this_object()->query_sp_name()+"."));

    // Abgesehen davon, dass wir nicht wissen koennen, ob die Intermud - Redes
    // ankommen werden, steht jetzt einer erfolgreichen Kommunikation nichts
    // mehr im Wege.

    if (kostenflag)
        this_object()->add_sp(-kosten);
        
    acltag = sizeof(aclgroups) ? "<"+liste(aclgroups,", ")+"> " : "";
        
    for (i=sizeof(destinations); i--;)
    {
        if (stringp (destinations[i]))
        {
            sscanf(destinations[i],"%s@%s",wer,mud);
            // dass es geht wurde bereits "oben" ueberprueft.
            INETD->send_udp(mud, ([
                REQUEST :        "tell",
                RECIPIENT :      wer,
                SENDER : this_object()->query_real_cap_name(),
                DATA :   message
            ]), 1);
        }
        else
     	{
            for (j=sizeof(destinations), buffer = ""; j--;)
                if (i != j)
	       	{
                    if (strlen (buffer))
                        buffer = ", "+buffer;
                    buffer = destination_names[j] + buffer;
                }
            destination = destinations[i];
	    if(closurep(destination))
	    {
		funcall(destination, message, destination, buffer);
		continue;
	    }
            if (away_message = destination->query_away())
                notify_message(wrap(Der(destination) + " ist weg: " + 
			    away_message), MA_SENSE);
            if (interactive(destination) &&
		    (idle_time = query_idle(destination)) > TELL_IDLE)
                notify_message(wrap(Der(destination) + " ist übrigens seit " +
		    format_seconds(idle_time/60*60) + " untätig."),MA_SENSE);

            redebuffer = wrap_string(
                (playerp (this_object())
                  ? acltag+TAG(tagflag)+Ein(
                    (["name":this_object()->query_real_name(),
                      "cap_name":this_object()->query_real_cap_name(),
                      "gender":this_object()->query_real_gender(),
                      "personal_title":this_object()->query_personal_title(),
                      "personal":this_object()->query_personal()]),
                      this_object()->query_adjektiv())
                  : Ein())
                +
                " redete zu "+ (strlen (buffer) ? buffer + " und zu dir:" : "dir:"),
                message);
            buffer = wrap_string(
                (playerp (this_object())
                  ? acltag+TAG(tagflag)+Ein(
                    (["name":this_object()->query_real_name(),
                      "cap_name":this_object()->query_real_cap_name(),
                      "personal":this_object()->query_personal(),
                      "personal_title":this_object()->query_personal_title(),
                      "gender":this_object()->query_real_gender()]),
                    this_object()->query_adjektiv())
                  : Ein())
                +
                " redet zu "+ (strlen (buffer) ? buffer + " und zu dir:" : "dir:"),
                message+(idle_time > TELL_IDLE? " ["+shorttimestr(time(),0,1)+"]":""));

            this_object()->send_message_to(destination,MT_SENSE|MT_FAR,MA_COMM,buffer);
	    if(!playerp(destination))
		destination->notify("tell_me", this_object(), destination,
		    filter(destinations, #'objectp)-({destination}),
		    message);
            destination->add_to_rede_puffer(redebuffer);
            add_sum_comm(1); // nur bei nicht-Intermud
        }
    }

    if (away)
	notify_message(wrap("(Eigentlich bist du ja weg: "+away+")"),MA_UNKNOWN);

    for (i=sizeof(destinations), buffer = ""; i--;)
    {
        if (i==sizeof(destinations)-2)
            buffer = " und "+buffer;
        else if (strlen (buffer))
	    buffer = ", "+buffer;
        buffer = destination_names[i] + buffer;
    }
    redebuffer = wrap_string(acltag+TAG(tagflag)+"Du redetest zu",buffer+": "+message);
    buffer = wrap_string(acltag+TAG(tagflag)+"Du redest zu",buffer+": "+message);
    if (!in_conversation)
	if (this_object()->query_echomode())
	    this_object()->send_message_to(this_object(),
	        MT_NOTIFY|MT_SENSE|MT_FAR,MA_COMM,buffer);
	else
	    this_object()->send_message_to(this_object(),
	        MT_NOTIFY|MT_SENSE|MT_FAR,MA_COMM,"Ok.\n");
    this_object()->add_to_rede_puffer(redebuffer);
    return 1;
}
/*
FUNKTION: notify_tell_me
DEKLARATION: void notify_tell_me(object wer, object wen, object *wen_noch, string was)
BESCHREIBUNG:
Wenn 'wer' 'wen' und 'wen_noch' mit dem Text 'was' anredet, so wird
wen->notify("tell_me", wer, wen, wen_noch, was) aufgerufen.
notify ruft dann in allen mit wen->add_controller("notify_tell_me", other)
angemeldeten Controllern other die Funktion other->notify_tell_me(wer, wen,
wen_noch, was) auf.
Diese haben dann die Moeglichkeit, auf das Anreden zu reagieren.
VERWEISE:
GRUPPEN: Spieler, Monster
*/

int shout_command(string str)
{
    int kosten;
    int i;
    int len;
    string strc;

    CHECK_INT;
    
    if (!str)
        FAIL("Brülle was?\n");

    kosten = strlen (str) / SHOUT_CHARS_PER_SP;
    if (kosten < MIN_SHOUT_COST) kosten = MIN_SHOUT_COST;
    
    if (GABE(this_object(),"sr"))
        kosten = kosten / 2;
        
    if (wizp(this_object()))
        kosten = 0;
        
    if (this_object()->query_sp() < kosten)
        FAIL(wrap("Du hast nicht genug "+this_object()->query_sp_name()+"."));
    for(i = 0, len = strlen(strc = str), str = ""; i < len; i++)
    {
       if(strc[i] < 32)
          str += sprintf("^%c", '@'+strc[i]);
       else if(strstr("ÄÖÜäöüẞß",strc[i..i]) != -1)
          str += strc[i..i];
       else if(strc[i] >= 128)
          ;
       else if(strc[i] == 127)
          str += "^?";
       else
          str += strc[i..i];
    }
    if (kosten)
        this_object()->add_sp(-kosten);
    if (playerp (this_object()))
        shout(wrap_string(Ein(
	    this_object()->query_magic_disguise())
            +" brüllt:",str));
    else
    {
	<string|mixed*> adj;
	if ((adj = this_object()->query_adjektiv()) && sizeof (adj))
	{
    	    adj = adj[0];
	    if ((strstr((string)adj,"sitzend") != -1)
	    ||  (strstr((string)adj,"liegend") != -1))
               adj = ({});
	}
	shout(wrap_string(Ein(0,adj)
            +" brüllt:",str));
    }
// Bruellen ist jetzt eh nicht mehr unsichtbar:
//    if (IS_INVIS(this_object()))
//       notify_message("(Du weisst, dass Du unsichtbar bist?)\n",MA_UNKNOWN);

    add_sum_comm(1);
    notify_message("Ok.\n",MA_UNKNOWN);
    CONTROL->notify("shout", this_object(), str);
    if (playerp (this_object()))
        STATISTIK->shout ();
    return 1;
}

int conversation_command(string str)
{
    CHECK_INT;
   if (wizp(this_player()) && GROUP_MASTER->group_exists(partner=space(str)))
   {
       notify_message("Versuch eines Gesprächs mit "+partner+". ",MA_COMM);
   }
   else if(partner = str ? lower_case(str) : 0)
   {
      object dest = find_living(partner);
      if(!dest)
         FAIL(capitalize(partner)+" ist nicht aufzufinden.\n");
      if(dest == this_object())
    	 FAIL("Führst Du Selbstgespräche?\n");

      notify_message("Gespräch mit "+capitalize(partner)+". ",MA_COMM);
   }
   else
      notify_message("Gespräch mit den hier Anwesenden. ",MA_COMM);
   notify_message("Beenden mit ** oder . oder ~q\n");
   in_conversation = 1;
   input_to("conversation_loop", INPUT_PROMPT,
       	partner ? "rede "+partner+" " : "'");
   return 1;
}

static void conversation_loop(string str)
{
#ifdef NEW_STATS
    if (this_object()->query_stat(STAT_INT) < MIN_INT_COMM)
    {
	write("Dazu bist du nicht intelligent genug.\n");
	return;
    }
#endif
   if(str && (str == "." || str == "**" || str == "~q"))
   {
      notify_message("Ende des Gesprächs.\n");
      in_conversation = 0;
      return;
   }
   if( str && str != "" &&
       !( partner ?
	     tell_command(partner+" "+str) :
	     say_command(str) ) )
   {
      notify_message("ABBRUCH des Gesprächs. "+
            "(Partner weg oder keine ZP mehr.)\n");
      in_conversation = 0;
      return;
   }
   input_to("conversation_loop", INPUT_PROMPT,
      partner ? "rede "+partner+" " : "'");
}

int antworte_command( string str)
{
    mixed parsed;
    string adv, tmp;
    int flags;

//    NO_GHOST;

    if (!str)
        return notify_fail( "antworte <wem> was?\n");
    else if ( !(query_verb() && query_verb()[<1]==':') &&
	(parsed = parse_text( str)) && strlen (parsed[1]))
    {
        string rest = do_modify_comm(parsed[0], parsed[1], "antwort", &adv, &flags);
	if ( forbidden_msg_soul_action_partner_notify(
                parsed[0], "antwort", rest, NEUTRAL, 0,
                echo_str( tmp="Du antwortest "+xdeinem(parsed[0])+adv+":", rest),
                Wrap_say( Wer( owner, ART_AAA)+" antwortet "+xdir(parsed[0])+adv+":", rest),
		calc_msg_other(Wer(owner, ART_AAA)+" antwortet "+xseinem(parsed[0])+adv,
		    0, 0, rest, flags),
                MA_COMM, MT_NOISE) )
        	    return 1 ;
	korrigiere_sage_puffer(tmp,str);
        COMM(owner);
        return 1;
    }
    str = do_modify_comm(0, str, "antwort", &adv, &flags);
    if ( forbidden_msg_soul_action_notify(
        "antwort", str, NEUTRAL, 0,
        echo_str( tmp="Du antwortest"+adv+":", str),
	calc_msg_other(Wer( owner, ART_AAA)+" antwortet"+adv,0,0, str, flags),
        MA_COMM, MT_NOISE) )
    	    return 1 ;
    korrigiere_sage_puffer(tmp,str);
    COMM(owner);

    return 1;
}


int bemerke_command( string str)
{
//    NO_GHOST;

    if ( !str )
        msg_notify( "Bemerke was?");
    else
    {
	string adv;
	int flags;
	
        str = do_modify_comm(0,str,"bemerk",&adv,&flags);
	if ( forbidden_msg_soul_action_notify(
            "bemerk", str, NEUTRAL, 0,
            echo_str( "Du bemerkst"+adv+":", str),
	    calc_msg_other(Wer(owner, ART_AAA)+" bemerkt"+adv,"etwas",0,
		str, flags),
            MA_COMM, MT_NOISE) )
            return 1 ;
	korrigiere_sage_puffer("Du bemerkst"+adv+":",str);
        COMM(owner);
    }
    return 1;
}


int bruddle_command( string str)
{
    string adv;
    int flags;
    
    NO_GHOST;

    if ( !str || str == "" )
        return soul_plus_adv( "", "bruddel",
                              NEUTRAL, 0, MA_EMOTE, MT_LOOK | MT_NOISE, 0,
                              "griesgrämig herum");

    str= do_modify_comm(0,str,"bruddel",&adv,&flags);
    if ( forbidden_msg_soul_action_notify(
        "bruddel", str, NEUTRAL, 0,
        echo_str( "Du bruddelst"+adv+":", str),
        calc_msg_other(Wer( owner, ART_AAA)+" bruddelt"+adv,0,0,str,flags),
        MA_COMM, MT_LOOK | MT_NOISE) )
        return 1 ;

    korrigiere_sage_puffer("Du bruddelst"+adv+":",str);
    COMM(owner);
    return 1;
}


int denke_command( string str)
{
//    NO_GHOST;
    if ( !str )
    {
        if ( forbidden_msg_soul_action_notify(
            "denk", str, NEUTRAL, 0,
            "Du wirst plötzlich sehr nachdenklich... Hmmmm.",
            Wer( owner, ART_AAA)+" wird plötzlich sehr nachdenklich.",
            MA_EMOTE, MT_LOOK) )
            return 1 ;

        feel( owner);
    }
    else
    {
	string adv;
	int flags;
	
        str = do_modify_comm(0,str,"denk",&adv,&flags);
	if ( forbidden_msg_soul_action_notify(
            "denk", str, NEUTRAL, 0,
            echo_str( "Du denkst"+adv+":",str),
            calc_msg_other(Wer(owner, ART_AAA)+" denkt"+adv,"nach",0,
		str,flags),
            MA_COMM, MT_SENSE) )
            return 1 ;
	korrigiere_sage_puffer("Du denkst"+adv+":",str);
        COMM(owner);
    }
    return 1;
}


int erlaeutere_command( string str)
{
//    NO_GHOST;
    if (!str)
        msg_notify( "Erläutere was?");
    else
    {
	string adv;
	int flags;
        str = do_modify_comm(0,str,"erlaeuter",&adv,&flags);
	if ( forbidden_msg_soul_action_notify(
            "erlaeuter", str, NEUTRAL, 0,
            echo_str( "Du erläuterst"+adv+":", str),
            calc_msg_other(Wer(owner, ART_AAA)+" erläutert"+adv,"etwas",0,
		str, flags),
            MA_COMM, MT_NOISE) )
            return 1 ;

	korrigiere_sage_puffer("Du erläuterst"+adv+":",str);
        COMM(owner);
    }
    return 1;
}


int frage_command( string str)
{
    string  tmp, adv ;
    mixed   *parsed, was ;
    int flags;

    if ( !str )
    {
        if ( forbidden_msg_soul_action_notify(
            "frag", 0, NEUTRAL, 0,
            echo_str( "Du fragst:", "?"),
            Wrap_say( Wer( owner, ART_AAA)+" fragt:", "?"),
            MA_COMM, MT_NOISE) )
            return 1 ;

	korrigiere_sage_puffer("Du fragst:","?");
        COMM( owner) ;
        return 1 ;
    }

#if FRAGE_MIT_PARTNER
    if ( !(query_verb() && query_verb()[<1]==':') &&
	(parsed = parse_text( str)))
    {
        was = parsed[0] ;
        str = parsed[1] ;

        // exec_command() Id gefunden
        if ( !was )
            return 0 ;

        if ( !sizeof(str) || member("!?.,;:",str[<1]) < 0)
            str += "?" ;
        str = do_modify_comm(was,str,"frag",&adv,&flags);
        if ( was == owner )
        {
            if ( forbidden_msg_soul_action_partner_notify(
                was, "frag", str, NEUTRAL, 0,
                Wrap_say( "Du fragst dich"+adv+":", str),
                0,
		calc_msg_other(Wer( owner, ART_AAA)+" fragt sich"+adv,"etwas",0,str,flags),
                MA_COMM, MT_NOISE) )
                return 1 ;
        }
        else
        {
            if ( forbidden_msg_soul_action_partner_notify(
                was, "frag", str, NEUTRAL, 0,
                echo_str( tmp = "Du fragst "+xdeinen( was)+adv+":", str),
                Wrap_say( Wer( owner, ART_AAA)+" fragt "+xdich( was)+adv+":", str),
                calc_msg_other( Wer( owner, ART_AAA)+" fragt "+xseinen( was)+adv,"etwas",0,str,flags),
                MA_COMM, MT_NOISE) )
                return 1 ;
	    korrigiere_sage_puffer(tmp,str);
        }
    }
    
    if(!parsed)
    {
#endif
        if ( !str[<1] || member("!?.,;:",str[<1]) < 0)
            str += "?" ;
        str= do_modify_comm(0,str,"frag",&adv,&flags);
        if ( forbidden_msg_soul_action_notify(
            "frag", str, NEUTRAL, 0,
            echo_str( tmp="Du fragst"+adv+":", str),
            calc_msg_other(Wer(owner, ART_AAA)+" fragt"+adv,"etwas",0,str,flags),
            MA_COMM, MT_NOISE) )
            return 1 ;
	korrigiere_sage_puffer(tmp,str);
#if FRAGE_MIT_PARTNER
    }
#endif

    COMM( owner) ;
    return 1 ;
}


int groele_command( string str)
{
    string adv;
    int flags;
    
    NO_GHOST;
    if ( !str )
    {
        if ( forbidden_msg_soul_action_notify(
            "groel", 0, NEUTRAL, 0,
            "Du grölst aus vollem Halse.",
            Wer( owner, ART_AAA)+" grölt aus vollem Halse.",
            MA_EMOTE, MT_LOOK | MT_NOISE) )
            return 1 ;

        feel( owner);
        return 1;
    }
    str = do_modify_comm(0,str,"groel",&adv,&flags);
    if ( forbidden_msg_soul_action_notify(
        "groel", str, NEUTRAL, 0,
        echo_str( "Du grölst"+(strlen(adv)?adv:" aus vollem Halse")+":", str),
        calc_msg_other( Wer( owner, ART_AAA)+" grölt"+adv,0,0,str,flags),
        MA_COMM, MT_LOOK | MT_NOISE) )
        return 1 ;
    korrigiere_sage_puffer("Du grölst"+(strlen(adv)?adv:" aus vollem Halse")+":",str);

    COMM(owner);
    return 1;
}


int kreische_command( string str)
{
    string adv;
    int flags;
    
//    NO_GHOST;
    if (!str || str == "")
        return soul_plus_adv( "", "kreisch",
                              NEUTRAL, 0, MA_EMOTE, MT_LOOK | MT_NOISE, 0,
                              "wie ein Jochgeier herum");
    str=do_modify_comm(0,str,"kreisch",&adv,&flags);
    if ( forbidden_msg_soul_action_notify(
        "kreisch", str, NEUTRAL, 0,
        echo_str( "Du kreischst"+adv+":", str),
        calc_msg_other( Wer( owner, ART_AAA)+" kreischt"+adv,0,0,str,flags),
        MA_COMM, MT_NOISE) )
        return 1 ;
    korrigiere_sage_puffer("Du kreischst"+adv+":",str);

    COMM(owner);
    return 1;
}


private string lalle_string(string str)
{
    string out = "";
    for(int i=0;i<strlen(str);i++)
    {
        if(!random(4))
        {
            switch(str[i..i])
            {
                case "l" :
                    out+="ll";
                    break;
                case "t" :
                    out+="dd";
                    break;
                case "k" :
                    break;
                case "f" :
                    out+="w";
                    break;
                case "i" :
                    out+="ie";
                    break;
                case "g" :
                    break;
                case " " :
                    if(!random(4))
                        out+=" *hick* ";
                    else
                        out+=" ";
                    break;
                default  :
                    out+=str[i..i];
                    break ;
            }
        }
        else
	    out+=str[i..i];
    }
    return out;
}

int lalle_command( string str)
{
    string adv;
    int flags;

    NO_GHOST;
    if (!str || str == "" )
    {
        if ( forbidden_msg_soul_action_notify(
            "lall", 0, NEUTRAL, 0,
            "Du lallst ein wenig herum.",
            Wer( owner, ART_AAA)+" verdreht die Augen, lässt "+
              seinen((["name":"zunge","gender":"weiblich"]),0,owner)+
              " heraushängen und lallt ein wenig herum: Glglglgl!",
            MA_EMOTE, MT_LOOK | MT_NOISE) )
            return 1 ;

        feel( owner);
        return 1;
    }
    flags = VOICE_LALLEN;
    str=do_modify_comm(0,str,"lall",&adv,&flags);

    if ( forbidden_msg_soul_action_notify(
        "lall", str, NEUTRAL, 0,
        echo_str( "Du lallst"+adv+":", str),
        calc_msg_other(Wer( owner, ART_AAA)+" lallt"+adv,0,0,str,flags),
        MA_COMM, MT_NOISE) )
        return 1 ;
    korrigiere_sage_puffer("Du lallst"+adv+":",str);

    COMM(owner);
    return 1;
}


int lisple_command( string str)
{
    int flags;
    string adv;

    NO_GHOST;

    if ( !str )
    {
        msg_notify( "Was willst du lispeln?");
        return 1;
    }
    flags = VOICE_LISPELN;
    str=do_modify_comm(0,str,"lispel",&adv,&flags);
    
    if ( forbidden_msg_soul_action_notify(
        "lispel", str, NEUTRAL, 0,
        echo_str( "Du lispelst"+adv+":",str),
        calc_msg_other(Wer(owner, ART_AAA)+" lispelt"+adv,0,0,str,flags),
        MA_COMM, MT_NOISE) )
        return 1 ;
    korrigiere_sage_puffer("Du lispelst"+adv+":",str);

    COMM(owner);
    return 1;
}


int murmle_command(string str)
{
    NO_GHOST;
    if ( !str )
    {
        if ( forbidden_msg_soul_action_notify(
            "murmel", 0, NEUTRAL, 0,
            "Du murmelst etwas Unverständliches.",
            Wer( owner, ART_AAA)+" murmelt etwas Unverständliches vor sich hin.",
            MA_EMOTE, MT_LOOK | MT_NOISE) )
            return 1 ;

        feel( owner);
    }
    else
    {
	 string adv;
	 int flags;
         str=do_modify_comm(0,str,"murmel",&adv,&flags);
     	 if ( forbidden_msg_soul_action_notify(
            "murmel", str, NEUTRAL, 0,
            echo_str( "Du murmelst"+adv+":",str),
            calc_msg_other(Wer( owner, ART_AAA)+" murmelt"+adv,"etwas",0,str,flags),
            MA_COMM, MT_NOISE) )
            return 1 ;
	korrigiere_sage_puffer("Du murmelst"+adv+":",str);

        COMM(owner);
    }
    return 1;
}


int singe_command( string str)
{
    //    NO_GHOST;
    if ( !str )
    {
        if ( forbidden_msg_soul_action_notify(
            "sing", 0, NETT, 0,
            "Juppheidi juppheida!",
            Wer( owner, ART_AAA)+" singt vor sich hin.",
            MA_EMOTE, MT_LOOK | MT_NOISE) )
            return 1 ;

        feel( owner);
    }
    else
    {
	string adv;
	int flags;
	
        str=do_modify_comm(0,str,"sing",&adv,&flags);
	if ( forbidden_msg_soul_action_notify(
            "sing", str, NEUTRAL, 0,
            echo_str( "Du singst"+adv+":", str),
            calc_msg_other(Wer( owner, ART_AAA)+" singt"+adv,0,0,str,flags),
            MA_COMM, MT_NOISE) )
            return 1 ;
	korrigiere_sage_puffer("Du singst"+adv+":",str);

        COMM(owner);
    }
    return 1;
}


int stottere_command( string str)
{
    int flags;
    string adv;

    //    NO_GHOST;

    if ( !str )
    {
        if ( forbidden_msg_soul_action_notify(
            "stotter", 0, NEUTRAL, 0,
            "Du stotterst ein wenig herum.",
            Wer( owner, ART_AAA)+" stottert ein wenig herum.",
            MA_EMOTE, MT_LOOK | MT_NOISE) )
            return 1 ;

        feel( owner);
        return 1;
    }
    flags = VOICE_STOTTERN;
    str=do_modify_comm(0,str,"stotter",&adv, &flags);

    if ( forbidden_msg_soul_action_notify(
        "stotter", str, NEUTRAL, 0,
        echo_str( "Du stotterst"+adv+":",str),
        calc_msg_other(Wer(owner, ART_AAA)+" stottert"+adv,0,0,str,flags),
        MA_COMM, MT_NOISE) )
        return 1 ;
    korrigiere_sage_puffer("Du stotterst"+adv+":",str);

    COMM(owner);
    return 1;
}
int summe_command( string str)
// wird vom Bardentool benutzt, beim Umbenennen oder verschieben
// denen bescheid sagen.
{
    int flags;
    string adv;

    //    NO_GHOST;
    if (!str || str =="")
    {
        if ( forbidden_msg_soul_action_notify(
            "summ", 0, NETT, 0,
            "Du summst ein kleines Liedchen: mmmh mmmmh!",
            Wer( owner, ART_AAA)+" summt ein kleines Liedchen: mmmh mmmmh!",
            MA_EMOTE, MT_LOOK | MT_NOISE) )
            return 1 ;

        feel( owner);
        return 1;
    }

    flags = VOICE_SUMMEN;
    str=do_modify_comm(0,str,"summ",&adv,&flags);
    
    if ( forbidden_msg_soul_action_notify(
        "summ", str, NEUTRAL, 0,
        echo_str( "Du summst"+adv+":", str),
        calc_msg_other(Wer(owner, ART_AAA)+" summt"+adv,0,0,str,flags),
        MA_COMM, MT_NOISE) )
        return 1 ;
    korrigiere_sage_puffer("Du summst"+adv+":",str);

    COMM(owner);
    return 1;
}


int traellere_command( string str)
{
    string adv;
    int flags;
    
    NO_GHOST;
    if ( !str )
    {
        if ( forbidden_msg_soul_action_notify(
            "traeller", 0, NEUTRAL, 0,
            "Du trällerst ein kleines Liedchen: Lalali, lalala!",
            Wer( owner, ART_AAA)+" trällert ein kleines Liedchen vor sich hin.",
            MA_EMOTE, MT_LOOK | MT_NOISE) )
            return 1 ;

        feel( owner);
        return 1;
    }
    str=do_modify_comm(0,str,"traeller",&adv,&flags);
    if ( forbidden_msg_soul_action_notify(
        "traeller", str, NEUTRAL, 0,
        echo_str( "Du trällerst"+adv+":",str),
        calc_msg_other( Wer( owner, ART_AAA)+" trällert"+adv,"ein Liedchen",0,str,flags),
        MA_COMM, MT_NOISE) )
        return 1 ;
    korrigiere_sage_puffer("Du trällerst"+adv+":",str);

    COMM(owner);
    return 1;
}


int tuschle_command( string str)
{
    mixed   *parsed, dest;
    string  mess, mess_other, tmp, adv;
    int     flags;

    NO_GHOST;

    if ( !str )
    {
        msg_notify( "Tuschle wem was?") ;
        return 1;
    }

    if ( str )
        str = regreplace( str, "^ *(mit|zu|zur|zum) ", "", 1) ;

    if ( !(parsed = parse_text( str, 0, 0 ,"Tuschle wem was?\n")) )
        return 0;

    dest = parsed[0];
    mess = parsed[1];

    // exec_command() Id gefunden
    if ( !dest )
        return 0 ;

    if ( dest == owner )
    {
        msg_notify( "Mit dir selbst Tuscheln? Wie soll das denn gehen?") ;
        return 1 ;
    }

    flags = VOICE_TUSCHELMODUS;
    mess=do_modify_comm(dest,mess,"tuschel",&adv,&flags);

    mess_other =
         ( livingp( dest) ) ?
         Wer( owner, ART_AAA)+" und "+ xsein(dest)+" stecken die Köpfe zusammen, "
         "blicken ganz geheimnisvoll und tuscheln"+adv+" miteinander" :
         Wer( owner, ART_AAA)+" blickt ganz geheimnisvoll und tuschelt"+adv+" mit "+
         xseinem( dest);

    if ( space( mess || "") == "" )
    {
        if ( forbidden_msg_soul_action_partner_notify(
            dest, "tuschel", 0, NEUTRAL, 0,
            "Du tuschelst etwas ganz geheimes mit "+xdeinem(dest)+".",
            Wer( owner, ART_AAA)+" tuschelt etwas ganz geheimes mit "+xdir( dest)+".",
	    mess_other,
            MA_EMOTE, MT_LOOK | MT_NOISE) )
            return 1 ;
    }
    else
    {
        if ( forbidden_msg_soul_action_partner_notify(
            dest, "tuschel", mess, NEUTRAL, 0,
            echo_str( tmp="Du tuschelst"+adv+" mit "+xdeinem( dest)+":", mess),
            Wrap_say( Wer( owner, ART_AAA)+" tuschelt"+adv+" zu "+xdir( dest)+":",mess),
            calc_msg_other(mess_other, 0, 0, mess, flags),
            MA_COMM, MT_LOOK | MT_NOISE) )
            return 1 ;
	korrigiere_sage_puffer(tmp,str);
    }

    COMM(owner);
    return 1;
}


int verkuende_command( string str)
{
    string adv;
    int flags;
    
    //    NO_GHOST;
    if ( !str )
    {
        msg_notify( "Verkünde was?");
        return 1;
    }
    str=do_modify_comm(0,str,"verkuend",&adv,&flags);
    if ( forbidden_msg_soul_action_notify(
        "verkuend", str, NEUTRAL, 0,
        echo_str( "Du verkündest"+adv+":", str),
        calc_msg_other(Wer( owner, ART_AAA)+" verkündet"+adv,"etwas",0,str,flags),
        MA_COMM, MT_NOISE) )
        return 1 ;
    korrigiere_sage_puffer("Du verkündest"+adv+":",str);

    COMM(owner);
    return 1;
}
