// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:          /i/money/verkaeufer.c
// Description:   Ein Verkaeufer
// Documentation: /doc/funktionsweisen/handel/verkaeufer
// Author:        Sissi   (1994, 1995, 1996)
// Aenderungen:   Monty   (17.04.96)  Messages eingebaut.
//                Sissi   (23.05.96)  Illusionen werden erkannt
//                Sissi   (30.05.96)  "wirf speer nach verkaeufer" fuehrt dazu,
//                                    dass der Verkaeufer den Speer zurueckwirft.
//                Sissi   (22.06.96)  standard_conversation eingefuehrt
//                Sissi   (16.07.96)  add_to_filter, delete_from_filter,
//                                    query_filter, set_filter
//                Mammi   (25.09.96)  set_standard_conversation respektive
//                                    get_conversation verarbeiten jetzt auch
//                                    closures richtig.
//                Finralf (08.03.97)  -alle messages verstehen jetzt '!' als
//                                     do_command (neue Funktion out_message)
//                                    -standard_conversation kann auch
//                                     Stringfelder enthalten.
//                                    -sagen auf do_command("sage..) umgebaut
//                Kurdel  (29.04.97)  Man sieht auch dem Verkaeufer die
//                                    Fitness an.
//		  Freaky  (09.03.98)  Einrueckungen
//                Freaky  (10.03.98)  message auf send_message umgebaut.
//                Sissi   (10.10.98)  Einrueckungen, "soldmessages" analog zu
//                                    "messages",
//                                    !GEBEN ermoeglicht als Bestandteil
//                                    der ...messages
//                Parsec  (07.04.99)  notify("sold", ...) eingefuehrt
//                Parsec  (08.01.00)  exec_command eingefuehrt
//                                    + Gimlis neues roemisch()

#pragma save_types

#define DEFAULT_VALUTA "taler"


inherit "/i/monster/monster";
virtual inherit "/i/money/exchange";

#include <stats.h>
#include <config.h>
#include <control.h>
#include <error.h>
#include <move.h>
#include <invis.h>
#include <deklin.h>
#include <message.h>
#include <commands.h>
#include <money.h>

#ifdef TestMUD
private mapping x, c;
#else
mapping x, c;
#endif

private mixed trinkgeld, dont_serve_murderers, messages;
private int roemodus, messagepoi, messagespeed, last_money;
private string *filter, lang, valuta, valutas, last_will, kundenname,
    lagerraum, *react_on, react_on_regexp;
private object kunde, last_given, was_geben, wem_geben;


/*
FUNKTION: set_valuta
DEKLARATION: void set_valuta (string s)
BESCHREIBUNG:
    Diese Funktion setzt im Verkaeufer die Waehrung, mit der er arbeitet.
    Er akzeptiert dann Geld in dieser Waehrung.
VERWEISE: query_valuta, query_valutas
GRUPPEN: verkaeufer, handel
*/

void set_valuta (string s)
{
    valuta = query_accepted_valuta (s);
    valutas = query_accepted_valutas (s);
    if ((!valuta) || (!valutas))
    raise_error ("Unbekannte Währung: "+s+".\n");
}



/*
FUNKTION: query_valuta
DEKLARATION: string query_valuta ()
BESCHREIBUNG:
    Diese Funktion liefert die Waehrung, welche der Verkaeufer
    als Zahlungsmittel akzeptiert.
VERWEISE: query_valuta, query_valutas
GRUPPEN: verkaeufer, handel
*/

string query_valuta()
{
    return valuta;
}



/*
FUNKTION: query_valutas
DEKLARATION: string query_valutas ()
BESCHREIBUNG:
    Diese Funktion liefert die Pluralform der Waehrung, welche der Verkaeufer
    als Zahlungsmittel akzeptiert.
VERWEISE: set_valuta, query_valuta
GRUPPEN: verkaeufer, handel, monster
*/

string query_valutas()
{
    return valutas;
}



/*
FUNKTION: set_lagerraum
DEKLARATION: void set_lagerraum (string dateiname)
BESCHREIBUNG:
    Mit dieser Funktion kann dem Verkaeufer ein Lagerraum gegeben werden;
    Er gibt dann auch Dinge an Spieler, wenn der Verkaeufer sie nicht bei
    sich hat, sie sich aber im Lagerraum befinden.
VERWEISE: query_lagerraum
GRUPPEN: verkaeufer, handel, monster
*/

void set_lagerraum (string s) { lagerraum = s; }



/*
FUNKTION: query_lagerraum
DEKLARATION: string query_lagerraum ()
BESCHREIBUNG:
    Mit dieser Funktion kann der Lagerraum des Verkaeufers abfragt werden.
    Die Funktion liefert 0, wenn der Verkaeufer keinen hat.
VERWEISE: set_lagerraum
GRUPPEN: verkaeufer, handel, monster
*/

string query_lagerraum () { return lagerraum; }



/*
FUNKTION: set_dont_serve_murderers
DEKLARATION: void set_dont_serve_murderers (mixed msg)
BESCHREIBUNG:
    Mit dieser Funktion setzt man, ob der Verkaeufer Moerder bedient.
    Als Argument 0, 1, oder der Text uebergeben, den Moerder erhalten.
    Beispiel:
    set_dont_serve_murderers("Franziska sagt: Moerder bediene ich nicht.");
    Mit set_dont_save_murders (1) kann das Bedienen von Moerdern ausgeschaltet
    werden, es wird dann ein "Standardtext" ausgegeben.
    Mit set_dont_serve_murderers (0) wird der Modus wieder ausgeschaltet.
    Standardmaessig werden Moerder bedient.
VERWEISE: query_dont_serve_murderers
GRUPPEN: verkaeufer, handel, monster
*/

void set_dont_serve_murderers (mixed msg)
{
    dont_serve_murderers = msg;
}

/*
FUNKTION: query_dont_serve_murderers
DEKLARATION: mixed query_dont_serve_murderers ()
BESCHREIBUNG:
    Mit dieser Funktion kann abgefragt werden, ob der Verkaeufer Moerder
    bedient. Bei 0 werden Moerder bedient. Bei 1 oder einem String
    werden Moerder nicht bedient.
VERWEISE: set_dont_server_murderers
GRUPPEN: verkaeufer, handel, monster
*/

mixed query_dont_serve_murderers ()
{
    return dont_serve_murderers;
}



/*
FUNKTION: set_roman
DEKLARATION: void set_roman (int r)
BESCHREIBUNG:
    Mit dieser Funktion kann man den Verkaeufer in den "Roemisch-Modus"
    setzen, Preisangaben werden dann in roemischen Ziffern gemacht.
    Einschalten mit set_roman (1), ausschalten mit set_roman (0).
VERWEISE: query_roman
GRUPPEN: verkaeufer, handel, monster
*/

void set_roman(int i)
{
    roemodus = i;
}


/*
FUNKTION: query_roman
DEKLARATION: int query_roman()
BESCHREIBUNG:
    Mit dieser Funktion gibt an object sich der Verkaeufer im "Roemisch-Modus"
    befindet.
VERWEISE: set_roman
GRUPPEN: verkaeufer, handel, monster
*/

int query_roman()
{
    return roemodus;
}



/*
FUNKTION: set_accept_tip
DEKLARATION: void set_accept_tip (mixed t)
BESCHREIBUNG:
    Mit dieser Funktion kann man setzen, ob ein Verkaeufer Trinkgeld
    akzeptiert. Standardmaessig nimmt er kein Trinkgeld.
    t == 0 wird als "kein Trinkgeld" interpretiert, alles andere als
    akzepiere Trinkgeld und tue das folgende:
    t == 1: nix besonderes
    t ist ein String: Der String wird als Dankeschoen ausgegeben
        (also set_accept_tip ("Otto sagt: Danke fuers Trinkgeld!"))
    t ist ein Funktionsaufrufspaar ({objektzeiger/String,Funktionsname})
        oder eine Closure: Dann wird die entsprechende Funktion
        aufgerufen, uebergeben wird ein int - Argument mit der Hoehe
        des Trinkgeldes; es werden keine weiteren Meldungen generiert
VERWEISE:
GRUPPEN: verkaeufer, handel, monster
*/

void set_accept_tip (mixed t)
{
    trinkgeld = t;
}

/*
FUNKTION: set_standard_conversation
DEKLARATION: void set_standard_conversation(mapping m)
BESCHREIBUNG:
   Mit dieser Funktion koennen die Standard - Meldungen des Verkaeufers
   beeinflusst werden. Die Mappingsvalueys koennen Strings, Stringarrays oder
   Closures (keine Pseudoclosures) sein. In dem uebergebenen Mapping sollten
   nur die Eintraege drinstehen, die veraendert werden sollen.
   An der Stelle, an welcher im Text ein Parameter rein muss, fuegt man
   ein $ - Zeichen ein. Mit dem Befehl "bsp? set_standard_conversation" gibt
   es ein Beispiel mit allen setzbaren Einstellunge (das mehr sagt als 
   tausend Worte).
   Addendum: Wird im value eines Mapping-Keys eine Closure angegeben, so
             wird diese ausgefuehrt. Der Returnwert dieser Closure darf
             keine Pseudoclosures ($der() usw) enthalten, da die nicht
             nochmal ausgewertet werden. Die Paramter zu jeder Closure ist
             in /doc/funktionsweisen/handel/verkaeufer zu finden.
BEISPIEL:
   siehe: bsp? set_standard_conversation
VERWEISE: query_standard_conversation, change_standard_conversation
GRUPPEN: verkaeufer, handel, monster
*/

/*
BEISPIEL: set_standard_conversation
    verkaeufer->set_standard_conversation (
    ([
    "unsichtbarer_sagt":
        ({"!sage Nanu, jetzt hoer ich schon Gespenster.",
        "!mache kratzt sich reichlich ratlos hinterm Ohr."}),
    "unsichtbarer_gibt":
        "!frage Nanu, wo kommt denn $ her?",
    "unsichtbarer_gibt_verschwinden":
        "Hans wirft $ in den Brunnen.",
    "al_kann_nicht_mehr_tragen":
        "!sage Du kannst $ nicht mehr tragen, Du hast zuviel bei Dir.",
        // Der Verkaeufer verkauft einen AutoLoader, den der Spieler nicht
        // mehr tragen kann.
    "bedient_bereits_diesen_spieler":
        "!sage Moment, eines nach dem anderen.",
        // Wenn der Spieler, der gerade bedient wird, noch etwas will,
        // bevor er bezahlt hat
    "bedient_bereits_anderen_spieler":
        "!sage Moment, ich bediene gerade $.",
        // Wenn bereits ein anderer Spieler bedient wird.
    "kunde_verschwunden":
        "!frage Nanu, wo ist denn $ hin?",
        // ein Kunde ist davongelaufen, hat sich ausgeloggt, ...
    "was_willst_du_haben":
        "!frage Bitte? Was moechtest Du gerne haben?",
        // Wird benutzt, falls ein Spieler nur "will" oder "moechte"
        // eingibt, nicht jedoch, was er will.
    "hab_ich_nicht":
        "!sage Tut mir leid, sowas hab ich nicht.",
        // Wird benutzt, falls "want" nix zurueckgibt
    "das_kostet":
        "!sage Das kostet $, wenn ich bitten darf...",
        // wird verwendet, wnn keine procemessage angegeben ist
    "zu_wenig_geld":
        "!sage Das ist zu wenig, das kostet $.",
        // Der Spieler hat Hans zu wenig Geld gegeben.
    "zu_viel_geld":
        "!sage Das ist zu viel, das kostet $.",
        // Der Spieler hat dem Verkaeufefr zu viel Geld gegeben, der
        // Verkaeufer akzeptiert kein Trinkgeld (-> set_accept_tip)
    "bittesehr":
        "!sage Da hast Du $, viel Spass damit!",
    "dann_halt_nicht":
        "!sage Dann eben nicht."
        "!emote seufzt tief.",
    "nanu":
        "!frage Nanu, was ist denn das und was soll ich damit?",
        // Dem Verkaeufer wurde was einfach so gegeben,
        // given hat nicht reagiert, oder der Spieler hat bei einem
        // Tauschgeschaeft das Falsche gegeben.
    "illusion":
        ({"!betrachte meine illusion",
        "!lache",
        "!GEBEN",
        "!sage Bin ich blind oder was?"}),
        // Reaktion auf Illusionen
    "moment":
        "!sage Moment, Moment.",
        // Der Verkaeufer tut grad was (beispielsweise reparieren)
    "falsche_waehrung":
        "!sage Das ist jetzt aber die falsche Waehrung, "
            "$ nehm ich nicht an.",
    "fehler":
        "!sage Tut mir leid, das ist mir ausgegangen",
        // Programmierer hat nen Fehler gemacht,
        // erhaelt mit do_error eine Nachricht,
        // aber der Spieler soll auch was kriegen.
    ]));
VERWEISE: 
GRUPPEN: verkaeufer, handel, monster
*/

void set_standard_conversation (mapping m)
{
    c = m;
}

/*
FUNKTION: query_standard_conversation
DEKLARATION: mapping query_standard_conversation()
BESCHREIBUNG:
   Liefert ein Mapping mit den Standard - Meldungen des Verkaeufers.
   Fuer naehere Beschreibung siehe set_standard_conversation.
VERWEISE: set_standard_conversation, change_standard_conversation
GRUPPEN: verkaeufer, handel, monster
*/

mapping query_standard_conversation ()
{
    return c;
}

/*
FUNKTION: change_standard_conversation
DEKLARATION: void change_standard_conversation(mapping changes)
BESCHREIBUNG:
   Aendert die aktuelle standard conversation (siehe set_standard_conversation)
   ab, indem es die Eintraege aus der bisherigen standard conversation um
   die Eintraege aus dem uebergebenen Parameter modifiziert.
   Soll ein Eintrag hinzugefuegt werden oder geaendert werden, geschieht
   dies mit (["das_kostet":"Hans sagt: schieb mal $ rueber, zack zack."]),
   loeschen kann man ein Eintrag durch ne leere Angabe (["das_kostet":0]).
VERWEISE: set_standard_conversation, query_standard_conversation
GRUPPEN: verkaeufer, handel, monster
*/

void change_standard_conversation (mapping changes)
{
    if (c)
        c += changes;
    else
        c = changes;
}

mixed get_conversation (string entry, string param, mixed standard)
{
    mixed t,y;
    int p,i;

    if (!c || !(t = c[entry])) return standard;
    if (closurep(t)) return t;

    if(param) {
        if(pointerp(t)) {
            y = allocate(sizeof (t));
            for(i=sizeof(t);i--;) {
        	y[i] = t[i];
        	if (stringp(t[i]) && (p = strstr (t[i],"$")) != -1)
		    y[i][p..p]=param;
	    }
	    return y;
	}
	else if ((p = strstr (t,"$")) != -1)
	    t[p..p]=param;
    }
    return t;
}



/*
FUNKTION: query_kunde
DEKLARATION: object query_kunde()
BESCHREIBUNG:
Liefert den momentanen Kunden des Verkaeufers.
Es kann dabei sein, dass der Kunde nicht 'present' ist.
VERWEISE:
GRUPPEN: verkaeufer, handel, monster
*/
object query_kunde()
{
    if (messagepoi || x)
        return kunde;
    return 0;
}


/*
FUNKTION: want
DEKLARATION: mapping want (string s, object who)
BESCHREIBUNG:
    Diese Funktion wird aufgerufen, wenn ein Spieler sagt, dass er etwas
    will (oder moechte usw.), uebergeben wird der Text, was der Spieler
    moechte sowie der Spieler selber. Naehers siehe bitte in der Enzy
    unter "Funktionsweisen", "Handel", "Verkaeufer".
VERWEISE: given, want_while_serving
GRUPPEN: verkaeufer, handel, monster
*/

mapping want (string s, object who)
{
    return environment(this_object())->want (s, who);
    // So gemacht, damit mans leicht ueberschreiben kann.
}

/*
FUNKTION: want_while_serving
DEKLARATION: int want_while_serving(string was, object wer)
BESCHREIBUNG:
    Diese Methode wird aufgerufen, wenn ein Spieler sagt, dass er etwas
    will (oder moechte usw.), waehrend der Verkaeufer gerade jemand anderen
    bedient. Sinn dieser Methode ist es, zu ermoeglichen, dass man bei
    Aktionen wie "will karte" bei Wirten reagieren kann, wenn das vom Spieler
    gewollte keine weiteren Reaktionen nach sich zieht und vom Verkaeufer mal
    kurz nebenbei zwischen reingeschoben werden koennen.
    Gibt die Methode den Wert 0 zurueck, so arbeitet der Verkaeufer ganz
    normal weiter (und sagt beispielsweise "Moment, ich bediene gerade."),
    liefert die Methode etwas von 0 verschiedenes, so geht der Verkaeufer
    davon aus, dass want_while_serving ihre Arbeit erfuellt hat und er nicht
    mehr weiters reagieren braucht.
    Siehe auch in der Enzy unter "Funktionsweisen", "Handel", "Verkaeufer".
VERWEISE: want, given
GRUPPEN: verkaeufer, handel, monster
*/

int want_while_serving (string was, object wer)
{
    return environment(this_object())->want_while_serving (was, wer);
    // So gemacht, damit mans leicht ueberschreiben kann.
}

void fehlermeldung (string s)
{
    do_error (wrap (s,60));
}


static string umbruch (mixed s)
{
    int i;
    string e;

    if (!s)
        return "";
    if (stringp(s))
    {
        if (s == "")
            return "";
        if (member(s,'\n') == -1)
            return wrap(s);
        return s;
    }
    if (!pointerp(s) || !sizeof(s))
    {
        fehlermeldung("Verkäufer: Es wurde ein Text angegeben, der weder "
           "ein String noch ein Stringarray war.");
        return "<FEHLER> ";
    }
    e = "";
    for (i = 0; i < sizeof(s); i++)
        e += umbruch(s[i]);
    return e;
}

/*
 * Funktion gibt einen String oder ein stringarray aus
 *  Wenn '!' am Anfang des Strings auftritt, wird do_command verwendet
 *  ansonsten tell_room.
 */
void geben_wtp (object was, object wem);

static void out_message(mixed m, varargs mixed *pars)
{
    if (!m)
	return;
    if (closurep(m))
    {
	m = apply(m, this_object(), pars);
	if(!stringp(m) && !pointerp(m)) return;
    }
    if (stringp(m))
	m = ({m});
    if(!pointerp(m) || !sizeof(m))
    {
	fehlermeldung("Verkäufer: Es wurde ein Text angegeben, der weder "
		   "ein String noch ein Stringarray war.");
	return;
    }
    m = apply(#'map,m,#'funcall,this_object(),pars);
    if (sizeof(filter(m,(:sizeof($1) && $1[0]=='!':))))
    {
	int i;
	for(i = 0; i < sizeof(m); i++)
	{
	    if (m[i] == "!GEBEN")
	       geben_wtp(was_geben,wem_geben);
	    else if (m[i][0] == '!')
		do_command(m[i][1..]);
	    else
		send_message(MT_UNKNOWN,MA_UNKNOWN,umbruch(m[i]));
	}
    }
    else
    {
	send_message(MT_UNKNOWN,MA_UNKNOWN,umbruch(m));
    }
}

int im_lager (object ob)
{
    object lr;
    if (lagerraum && ob && (lr = find_object (lagerraum))
      && (environment (ob) == lr)) return 1;
    return 0;
}

/*
FUNKTION: deliver
DEKLARATION: int deliver (object ob, object whom, int reason)
BESCHREIBUNG:
    Diese Funktion wird aufgerufen, wenn der Verkaeufer dem Spieler whom
    einen Gegenstand ob aushaendigen will. reason gibt an, wieso
    diese Funktion aufgerufen wurde (definiert in <money.h>):
	VD_NORMAL	Normales Geben
	VD_RECP_FULL	Derjenige kann nix mehr aufnehmen
	VD_RECP_UNAVAIL	Derjenige ist nicht mehr im Raum.
    Liefert die Funktion einen Wert 0, so kommt das Standardgeben
    des Verkaeufers zum Zug.
VERWEISE: given, want_while_serving
GRUPPEN: verkaeufer, handel, monster
*/

int deliver (object ob, object whom, int reason)
{
    return environment(this_object())->deliver (ob, whom, reason);
    // So gemacht, damit mans leicht ueberschreiben kann.
}

void geben (object was, object wem)
// gibt "was" an "wem", wobei was entweder im Besitz von this_object() sein
// muss oder gar kein environment besitzen darf oder sich im "Lagerraum"
// befindet (->set_lagerraum, query_lagerraum). Was kann auch ein
// AutoLoader sein; hat der Spieler zuviel dabei, wird was, sofern
// was kein AL ist, hingelegt, andernfalls removt.
// Die entsprechenden Meldungen werden generiert.
// refuse nicht verwendet, da refuse weder bei AutoLoadern "richtig" tut
// noch bei Gegenstaenden, die kein environment haben.
{
    string ew, dw;
    int move_ergebnis;
    object tr;

    if (lagerraum && im_lager (was))
      was->move (this_object ());
    if (!was || (!tr = environment ()) ||
      (environment (was) && (environment (was) != this_object())))
      return;
    ew = einen (was); dw = den (was);
    if (!wem || (environment(wem) != environment())) {
	if(!deliver(was, wem, VD_RECP_UNAVAIL)) {

	    if(was->query_no_move() || was->query_auto_load())
	    {
		was->close_con();
		was->remove();
		return;
	    }
	    
	    move_ergebnis = was->move(tr);
    	    if (move_ergebnis != MOVE_OK) {
        	write ("Ohje... Ein unerklärlicher Fehler ist aufgetreten.\n");
		was->close_con();
    	        was->remove();
        	return;
	    }
    	    send_message(MT_LOOK,MA_PUT,umbruch(Der(this_object())+" legt "+ew+" hin."));
	}
        environment(this_object())->notify("sold", this_object(), was, wem);
        do_notifies(C_RESORT,"sold",
            ({"_me","_what","_whom","_here" }), 
            ({this_object(),was,wem,environment(this_object())}) );
        return;
    }
    
    if(!deliver(was, wem, VD_NORMAL)) {
	move_ergebnis = was->move(wem);
	if (move_ergebnis != MOVE_OK) {
            if (!deliver(was, wem, VD_RECP_FULL)) {
    	        if (was->query_no_move() || was->query_auto_load()) {
        	    out_message(get_conversation(
        	      "al_kann_nicht_mehr_tragen",
        	      den(was),
        	      "!sage Du kannst "+den(was)+" nicht mehr tragen, Du hast"
		      " zuviel bei Dir."), wem, was);
		    was->close_con();
    	            was->remove();
        	    if (last_money) {
            	        object geld;
            	        geld = clone_object ("/obj/money");
            	        geld->set_money (last_money);
            	        geld->set_valuta (valuta);
            	        geld->set_valutas (valutas);
            	        geben (geld,wem);
            	        if (geld && !environment (geld)) geld->remove ();
        	    }
        	    return;
    	        }
    	        move_ergebnis = was->move(tr);
    	        if (move_ergebnis != MOVE_OK) {
        	    write ("Ohje... Ein unerklärlicher Fehler ist aufgetreten.\n");
        	    was->remove();
        	    return;
	        }
    	        send_message(MT_LOOK,MA_PUT,
        	    umbruch(
            	        Der(this_object()) +" legt "+ew+" für "+den(wem)
		        +" hin, da "+er(wem)+" "+dw +" nicht mehr tragen "
                        "kann."),
    	            umbruch(
                        Der(this_object())  +" legt "+ew+" für Dich hin, "
                        "da Du "+dw +" nicht mehr tragen kannst."),wem);
            }
        } else {
            send_message(MT_LOOK,MA_PUT,
              umbruch(
                Der(this_object())+" gibt "+dem(wem) +" "+ew+"."),
              umbruch(
                Der(this_object())+" gibt Dir "+ew+"."),wem);
        }
    }
    environment(this_object())->notify("sold", this_object(), was, wem);
    do_notifies(C_RESORT,"sold",
            ({"_me","_what","_whom","_here" }), 
            ({this_object(),was,wem,environment(this_object())}) );
}


void geben_wtp (object was, object wem)
{
    call_with_this_player (#'geben, was, wem);
}


static int herstellen (object geld, object tp)
// stellt die Ware her, egal, ob diese als Dateiname, als
// Funktionsaufrufspaar oder als Closure vorliegt.
{
    if (objectp (x["good"])) return 1;
    if (stringp(x["good"])) {
      x["good"] = clone_object (x["good"]);
      if (objectp (x["good"])) return 1;
      else return 0;
    }
    else if (closurep (x["good"])) {
      x["good"] = funcall (x["good"], last_will, last_given, geld, tp);
      return 1;
    }
    else if (pointerp (x["good"]) && (sizeof (x["good"]) == 2)) {
      x["good"] = call_other (x["good"][0],x["good"][1],
        last_will, last_given, geld, tp);
      return 1;
    }
    return 0;
}


static int umrechnen (int x)
{
    if (!valuta)
        raise_error ("Verkäufer: Währung nicht mit set_valuta gesetzt!\n");
    return convert_rounded(x, 0, valuta);
}


#if 0
#define ziffern ({"M","IM","VM","XM","LM","CM","D","ID","VD","XD","LD","CD",\
"C","IC","VC","XC","L","IL","VL","XL","X","IX","V","IV","I","*"})

#define zahlen ({1000,999,995,990,950,900,500,499,495,490,450,400,100,99,\
95,90,50,49,45,40,10,9,5,4,1,0})

string roemisch(int zahl)
// Author: Fletch
{
    int i;
    i=0;
    if (zahl <= 0) return "";
    for (i = 0; zahlen[i] > zahl; i++);
    return ziffern[i] + roemisch(zahl-zahlen[i]);
}
#else
string *roem_e = ({ "","I","II","III","IV","V","VI","VII","VIII","IX" });
string *roem_z = ({ "","X","XX","XXX","XL","L","LX","LXX","LXXX","XC" });
string *roem_h = ({ "","C","CC","CCC","CD","D","DC","DCC","DCCC","CM" });

string roemisch_klein(int zahl)
{
    int i;
    string tmp;

    tmp = "";
    i = zahl / 1000;
    zahl %= 1000;
    tmp += copies("M", i);

    i = zahl / 100;
    zahl %= 100;
    tmp += roem_h[i];

    i = zahl / 10;
    zahl %= 10;
    tmp += (roem_z[i] + roem_e[zahl]);
    return tmp;
}

string roemisch(int zahl)
{
    string tmp;
    int i, j;

    if (zahl <= 0) return "";
    if (zahl > 4999) {
        tmp = "";
        j = 0;
        while (zahl) {
            i = zahl % 1000;
            zahl = zahl / 1000;
            if (i) {
                tmp = (zahl ? " + " : "") + roemisch_klein(i)
                       + copies(" * M", j) + tmp;
            }
        j++;
        }
        return tmp;
    }
    else {
        return roemisch_klein(zahl);
    }
}
#endif


static string betrag_text (int i)
{
    if (roemodus)
        return roemisch(i);
    else
        return sprintf("%d", i);
}


static void kaufaktion (string str, object wer)
{
    if (find_call_out ("kaufaktion_c") == -1)
        call_out ("kaufaktion_c",2,str,wer);
}

static void bearbeite_mapping (int cv, object who); // Prototyp

void kaufaktion_c (string str, object tp)
// nicht static machbar wg. call_out
{
    object tr;
    if (!tp || (!tr = environment ()) || !present (tp,tr)) return;
    if (tp->query_invis() & V_ATOM_INVIS) {
      out_message(
        get_conversation ("unsichtbarer_sagt",0,
          ({"!sage Nanu, jetzt hör ich schon Gespenster.",
            Der()+" kratzt sich reichlich ratlos am Ohr.",
            "!sage Hier spukt's."})), tp);
      return;
    }
    if (dont_serve_murderers && tp->query_moerder()) {
      out_message(
        stringp (dont_serve_murderers)
        ? dont_serve_murderers
        : "!sage Mörder bediene ich nicht.");
      return;
    }
    // wenn wir schon einen Kunden haben, erstmal den bedienen;
    if (messagepoi || (x && kunde && present (kunde,tr))) {
        if (want_while_serving (str, tp)) return;
        if (!kundenname && kunde) kundenname = kunde->query_cap_name();
        else if (!kundenname) kundenname = "jemand anderen";
        if (kunde == tp)
            out_message(
              get_conversation("bedient_bereits_diesen_spieler",
              0,
              "!sage Moment, eines nach dem anderen."), tp);
        else
            out_message(
              get_conversation (
              "bedient_bereits_anderen_spieler",
              kundenname,
              "!sage Moment, ich bediene gerade "+kundenname+"."), tp, kunde);
        return;
    }
    // wenn uns ein Kunde davongelaufen ist, den neuen Kunden bedienen
    // und "aufraeumen"
    while (remove_call_out ("dann_halt_nicht") != -1);
    if (x) {
      out_message(
          ({Der(this_object())+" staunt.",
            get_conversation (
              "kunde_verschwunden", kundenname,
              "!sage Nanu, wo ist denn "+kundenname+" hin?") }), kunde);
      if (x["good"] && objectp(x["good"]))
        call_other (x["good"],"remove");
      last_will = 0;
      if (last_given && !im_lager (last_given)) geben_wtp (last_given,0);
      last_given = 0;
      x = 0;
    }
    if (!str || (str == "")) {
      out_message(
        get_conversation ("was_willst_du_haben",0,
          "!sage Bitte? Was möchtest Du gerne haben?"), tp);
      last_will = 0;
      return;
    }
    x = want (str,tp);
    if (!x) {
      out_message(
          get_conversation ("hab_ich_nicht",0,
            "!sage Tut mir leid, sowas hab ich nicht."), tp, str);
      last_will = 0;
      return;
    }
    last_will = str; // der Aufruf war erfolgreich, merken
    bearbeite_mapping (0,tp);
    return;
}

void add_message(mixed m, varargs mixed *pars)
{
    if(messages)
        messages+=({ ({m})+pars });
    else
        apply(#'out_message,m,pars);
}

void do_message ()
{
    apply(#'out_message,messages[messagepoi]);
    messagepoi++;
    if (messagepoi >= sizeof (messages)) {
        messagepoi = 0;
        messages = 0;
    }
    else call_out("do_message",messagespeed);
}


static void bearbeite_mapping (int cv, object tp)
// cv == 0: wird aufgerufen von kaufaktion_c, wenn "want" erfolgreich war
// cv == 1: wird aufgerufen von just_given, wenn "given" erfolgreich war
{
    int waittime;
    string v_artikel;
    
    messages = 0;

    // Wenn wir eine Ware haben, dann den Preis rauskriegen
    if (x["good"]) {
        if (!member(x, "price")) {
            // muss so gemacht werden, da der Preis ja auch 0 sein kann,
            // also die Ware umsonst waere
            // Preis ausrechnen, gegebenfalls dazu Ware erschaffen
            if (stringp (x["good"]))
                x["good"] = clone_object (x["good"]);
            if (objectp (x["good"]))
                x["price"] = umrechnen (2 * x["good"]->query_value());
            else {
                fehlermeldung ("Verkäufer: Wenn ein \"good\" gesetzt "
                    "ist, das weder String noch Object ist, so muss der "
                    "\"price\" gesetzt sein.");
                out_message(
                    get_conversation ("fehler",0,"!sage Das ist mir ausgegangen."),
                    tp, x["good"]);
                x = 0;
                last_will = 0;
                if (last_given && !im_lager (last_given)) {
                    geben_wtp (last_given, tp);
                    last_given = 0;
                }
                return;
            }
        }
    }

    if (x["message"])
        out_message(x["message"],tp,
	    x["good"] || (!x["keep_given"] && !im_lager (last_given) && last_given));
    else if (messages = x["messages"]) {
        if(closurep(messages))
            messages=funcall(messages, this_object(), tp,
		x["good"] || (!x["keep_given"] && !im_lager (last_given) && last_given));
        if (!closurep(messages) && (!pointerp (messages) || !sizeof (messages)))
            fehlermeldung ("Verkäufer: Auf den Wunsch "+last_will+
              " hin wurde ein Mapping übergeben, das \"messages\""
              " enthielt, welche aber kein Array waren.");
        else {
            messagepoi = 0;
            messagespeed = x["messages_speed"];
            if (messagespeed < 2) messagespeed = 2;
            messages = map(messages,(: ({$1,$2,$3}) :), tp,
		x["good"] || (!x["keep_given"] && !im_lager (last_given) && last_given));
            do_message ();
            waittime = sizeof (messages) * messagespeed;
        }
    }
    // wenn es eine Ware gibt und diese kostenlos ist, diese an
    // den Kunden geben
    if ((!x["price"]) && x["good"]) {
        if (!herstellen (0, tp)) {
            if (!cv)
                fehlermeldung ("Verkäufer: Auf den Wunsch "+last_will+
                    " hin wurde ein Mapping übergeben, dessen \"price\" 0 "
                    " ist, dessen \"good\" nicht erschaffbar war.");
            else
                fehlermeldung ("Verkäufer: Auf die Übergabe von "
                    +(string)last_given+", "+einem(last_given)+
                    ", hin wurde ein Mapping übergeben, dessen \"price\" 0 "
                    " ist, dessen \"good\" nicht erschaffbar war.");
            out_message(
                get_conversation ("fehler",0,
                    "!sage Tut mir leid, das hab ich nicht mehr."), tp, x["good"]);
            last_will = 0;
            if (last_given && !im_lager (last_given)) {
                if (waittime) {
                    was_geben = last_given;
                    wem_geben = tp;
                    call_out ("geben_wtp",waittime+2,was_geben, wem_geben);
                }
                else geben_wtp (last_given, tp);
                last_given = 0;
            }
            x = 0;
            return;
        }
        if (waittime) {
            was_geben = x["good"];
            wem_geben = tp;
            call_out ("geben_wtp",waittime+2,was_geben,wem_geben);
        }
        else geben_wtp (x["good"],tp);
        last_will = 0;
        if (last_given && !x["keep_given"] && !im_lager (last_given)) {
            if (waittime) call_out ("geben_wtp",waittime+2,last_given, tp);
            else geben_wtp (last_given, tp);
        }
        last_given = 0;
        x = 0;
        return; // fertig.
    }

    if (!x["price"]) { // keine Ware, ggf. gegebenes Objekt zurueckgeben
        last_will = 0;
        if (last_given && !x["keep_given"] && !im_lager (last_given)) {
            if (waittime) {
                was_geben = last_given;
                wem_geben = tp;
                call_out ("geben_wtp",waittime+2,was_geben,wem_geben);
            }
            else geben_wtp (last_given, tp);
        }
        last_given = 0;
        x = 0;
        return;
    }

    // Die Ware ist nicht kostenlos
    if (x["timeout"])
        call_out ("dann_halt_nicht",x["timeout"]+waittime);
    else
        call_out ("dann_halt_nicht",30+waittime);
    if (x["pricemessage"])
        add_message(x["pricemessage"],tp,x["good"],x["price"]);
    else
    if (intp (x["price"])) { // koennte ja auch ein Tauschgeschaeft sein
    if (x["good"] && objectp (x["good"]))
        // v_artikel = Der(x["good"])[0..2]+
        v_artikel = Wer(x["good"], ART_NUR_DER)+
        (x["good"]->query_plural() ? " kosten ": " kostet ");
    else v_artikel = "Das macht dann ";
    add_message(
          get_conversation (
            "das_kostet",
            betrag_text(x["price"])+" "+capitalize(valutas),
            "!sage "+v_artikel+betrag_text(x["price"])+" "+
            capitalize(valutas)+", wenn ich bitten darf..."),
          tp, x["good"], x["price"]);
    }
    kunde = tp;
    kundenname = tp->query_cap_name();
    return;
}


/*
FUNKTION: given
DEKLARATION: varargs mapping given (object what, object who, string wanted)
BESCHREIBUNG:
    Diese Funktion wird aufgerufen, wenn ein Spieler einem Verkaeufer etwas
    gibt und der Verkaeufer nichts mit dem Gegenstand anfangen kann.
    Uebergeben wir der Gegenstand (what), der uebergeben wurde und der
    Spieler, der dem Verkaeufer etwas gegeben hat (who). Falls der Spieler
    vorher was wollte, so ist dies in 'wanted' enthalten.
    Naehers siehe bitte in der Enzy unter "Menue", "Funktionsweisen",
    "Handel", "Verkaeufer".
VERWEISE: want
GRUPPEN: verkaeufer, handel, monster
*/


mapping given (object what, object who)
{
    return environment(this_object())->given (what, who, last_will);
    // So gemacht, damit mans leicht ueberschreiben kann.
}



int just_given (object ob, object woher, object me, object tp, mapping mv_infos)
{
    int time_left, diff;

    if (woher && !living (woher) && this_player())
	woher = this_player();

    if ((!woher) || (woher == this_object()))
	return 1;

    if (!ob || !woher)
	return 0;

    if (ob->query_invis())
	return 1;

    if (mv_infos && mv_infos[MOVE_TYPE] == MOVE_TYPE_SCHIESSEN)
	return 1;

#ifdef UNItopia
    if (ob->id("floh")) return 1;
#endif

    last_money = 0;
    if (query_in_fight () && ob->query_weapon_class("wurf") &&
	    !ob->query_broken())
    {
	exec_command("führe", ob);
	exec_command("wirf", ob, "nach", woher);
	return 1;
    }
    
    if (woher->query_invis() & V_ATOM_INVIS)
    {
	if (ob->query_plural())
	    out_message(
		get_conversation ("unsichtbarer_gibt", der(ob), 
		    "!sage Nanu, wo kommen denn auf einmal " +
		    (ob->query_money() ? "die " : "") + der(ob)+" her?"),
		woher, ob);
	else
	    out_message(
		get_conversation ("unsichtbarer_gibt",der(ob),
		    "!sage Nanu, wo kommt denn auf einmal "+der(ob)+" her?"),
		woher, ob);
	
	out_message(
	    get_conversation("unsichtbarer_gibt_verschwinden",den(ob),
		Der()+" lässt "+den(ob)+" verschwinden."),
	    woher, ob);	
	ob->close_con();
	ob->remove();
        return 1;
    }
    if (ob->id ("illusion"))
    {
	mixed*|string s;
	s = get_conversation ("illusion",der(ob),
		({  "!betrachte meine illusion",
		    "!lache",
		    "!GEBEN",
		    "!sage Bin ich blind oder was?"
		}));
	
	if (pointerp (s))
	{
	    for (int i = 0; i < sizeof (s); i++)
		if (s[i] == "!GEBEN")
		    s[i] = "!gib meine illusion an "+woher->query_name();
	}
	else
	{
	    if (stringp (s) && (string)s == "!GEBEN")
		s = "!gib meine illusion an "+woher->query_name();
	}
	out_message (s);
	return 1; 
    }
    
    if (dont_serve_murderers && woher->query_moerder())
    {
	out_message(stringp (dont_serve_murderers) ? dont_serve_murderers :
	    "!sage Mörder bediene ich nicht.");
	geben_wtp (ob, woher);
	return 1;
    }

    if (messagepoi)
    {
	do_command("sage Moment, Moment.");
	geben_wtp (ob, woher);
	return 1;
    }
    
    if (x && (kunde != this_player()))
    {
	out_message(
	    get_conversation ("bedient_bereits_anderen_spieler",kundenname,
		"!sage Moment, ich bediene gerade "+kundenname+"."),
	    this_player(), kunde);
	geben_wtp (ob, woher);
	return 1;
    }

    if (!x) // der Spieler hat uns das einfach so gegeben
    {
	if (!x = funcall(#'given, ob, this_player(), last_will))
	{
	    if (ob)
	    {
		out_message(
		    get_conversation ("nanu",0,
			"!sage Nanu, was ist denn das und was soll ich damit?"),
		    kunde, ob);
		geben_wtp (ob, woher);
	    }
	    
	    return 1;
	}
	
	last_given = ob;
	bearbeite_mapping (1,woher);
	return 1;
    }

    time_left = remove_call_out ("dann_halt_nicht");
    if (time_left == -1)
    {
	do_command("sage Nanu? Mir ist auf einmal so schwindlig...");
	call_out ("dann_halt_nicht",10);
    }
    else
	call_out ("dann_halt_nicht",time_left+5);

    // Test, ob es ein Geldpreis ist und der Kunde mit Geld bezahlt oder
    // ob es ein Sachpreis und der Kunde eine Sache gegeben hat, die passt

    if (intp(x["price"]) && ob->query_money() &&
	    (ob->id(valuta) || ob->id(valutas)))
    {
	// Normalfall, der Spieler bezahlt eine Ware mit Geld
	if (ob->query_money() < x["price"])
	{
	    out_message(
		get_conversation("zu_wenig_geld",
		    betrag_text(x["price"])+" "+capitalize(valutas),
		    "!sage Das ist zu wenig, das kostet " +
			betrag_text(x["price"]) + " " + capitalize(valutas)+"."),
		    kunde, x["good"], x["price"], ob->query_money());
	    geben_wtp (ob, woher);
	    return 1;
	}
	
	remove_call_out ("dann_halt_nicht");
	
	if ((ob->query_money() != x["price"]) && (!trinkgeld))
	{
	    out_message(
		get_conversation("zu_viel_geld",
		    betrag_text(x["price"])+" "+capitalize(valutas),
		    "!sage Das ist zu viel, das kostet " +
		    betrag_text(x["price"])+" "+capitalize(valutas)+"."),
		kunde, x["good"], x["price"], ob->query_money());
	    ob->add_money(-x["price"]);
	    geben_wtp (ob, woher);
	}
	else if (diff = (ob->query_money() - x["price"]))
	{
	    if (intp(trinkgeld))
		do_command("sage Oh, dankeschön für das großzügige "
			    "Trinkgeld!");
	    else if (stringp(trinkgeld))
		out_message(trinkgeld);
	    else if (closurep (trinkgeld))
		funcall (trinkgeld,diff);
	    else if (pointerp (trinkgeld) && (sizeof (trinkgeld) == 2))
		call_other (trinkgeld[0],trinkgeld[1], diff);
	}
	
	if (environment(ob) == this_object())
	{
	    if (ob->query_money())
		last_money = ob->query_money();
	    ob->close_con();
	    ob->remove ();
	    // andernfalls wurde es als Wechselgeld zurueckgegeben
	}
    }
    else if (pointerp (x["price"]))
    {
	int i;
	for (i = sizeof(x["price"])-1; i>=0 && !ob->id(x["price"][i]); i--);
	if (i == -1)
	{ // tauschhandel, aber falsches gegeben.
	    out_message(
		get_conversation ("nanu",0,
		    "!sage Nanu, was soll ich denn damit?"),
		kunde, ob);
	    
	    if (x["pricemessage"])
		out_message(x["pricemessage"], kunde, x["good"], x["price"]);
	    geben_wtp (ob, woher);
	    return 1;
	}
    }
    else if (closurep (x["price"]))
    {
	int res = funcall(x["price"], last_will, last_given, ob, woher, x, kunde);
	
	if(res < 0) // Closure hat das Objekt verbraucht, zurueckgegeben
	    return 1;
	else if(!res) // Wir geben es zurueck
	{ // tauschhandel, aber falsches gegeben.
	    out_message(
		get_conversation ("nanu",0,
		    "!sage Nanu, was soll ich denn damit?"),
		kunde, ob);
	    
	    if (x["pricemessage"])
		out_message(x["pricemessage"], kunde, x["good"], x["price"]);
	    geben_wtp (ob, woher);
	    return 1;
	}
    }
    else
    {
	// der Spieler hat einfach so uns was gegeben, was gar nicht passt
	// Mal gucken, obs Geld ist...
	if (ob->query_money())
    	    out_message(
        	get_conversation("falsche_waehrung",
		    capitalize(ob->query_valutas()),
		    "!sage Mit der Währung kann ich nichts anfangen, ich "
		    "nehme keine "+capitalize(ob->query_valutas())+"."),
		kunde, ob);
	else
	    out_message(
		get_conversation ("nanu",0,
		    "!sage Nanu, was soll ich denn damit?"),
		kunde, ob);

	if (x["pricemessage"])
	    out_message(x["pricemessage"], kunde, x["good"], x["price"]);
	geben_wtp (ob, woher);
	return 1;
    }
    
    if (herstellen (ob, woher))
    {
	int waittime;
	
        if (x["soldmessage"])
            out_message(x["soldmessage"], kunde, x["good"]);
        else if (messages = x["soldmessages"])
	{
            if(closurep(messages))
                messages=funcall(messages, this_object(), kunde, x["good"]);

            if (!pointerp (messages) || !sizeof (messages))
                fehlermeldung ("Verkäufer: Auf den Wunsch "+last_will+
                    " hin wurde ein Mapping übergeben, das \"soldmessages\""
                    " enthielt, welche aber kein Array waren.");
            else
	    {
                messagepoi = 0;
                messagespeed = x["messages_speed"];
                if (messagespeed < 2) messagespeed = 2;
                messages = map(messages,(: ({$1,$2,$3}) :), kunde, x["good"]);
                do_message ();
                waittime = sizeof (messages) * messagespeed;
            }
        }
	else // weder soldmessage noch soldmessages
	{ 
            out_message(
		get_conversation("bittesehr", einen(x["good"]),
		    "!sage Bitte sehr, "+woher->query_cap_name()+", "+
		    ein(x["good"])+"."),
		kunde, x["good"]);
        }
	
        if (waittime)
	{
            was_geben = x["good"];
            if (!was_geben)
		was_geben = last_given;
            wem_geben = this_player();
	    
            call_out ("geben_wtp",waittime+2,was_geben, wem_geben);
        }
        else
	    geben_wtp (x["good"],this_player());
    }
    
    if (ob && (environment (ob) == this_object()))
    {
        if (ob->query_money())
            last_money = ob->query_money ();
	ob->close_con();
        ob->remove ();
    }
    
    x = 0;
    if (last_given && (was_geben != last_given) && !im_lager (last_given))
    {
        geben_wtp (last_given,woher);
        last_given = 0;
    }
    return 1;
}


/*
FUNKTION: query_current_working_data
DEKLARATION: mapping query_current_working_data()
BESCHREIBUNG:
    Liefert fuer einen Verkaeufer eine Kopie des Mappings mit den Daten
    ueber den aktuellen Verkaufsvorgang.
GRUPPEN: verkaeufer, handel, monster
*/

mapping query_current_working_data()
{
    return copy (x);
}


void starte_gegeben()
{
    set_accept_objects (({#'just_given}));
}


static string leere (string str)
{
    string *exp, *ufilter;
    int i;

    exp = regexplode(str, "[ \t.,!]+");
    ufilter = map(filter, #'convert_umlaute);

    // Worte aus filter entfernen, ohne Kommata zu entfernen.
    // TODO: Das ist nur eine Notloesung. Geht sicher auch anders.
    for(i = 0; i < sizeof(exp); i += 2)
        if(member(ufilter, convert_umlaute(exp[i])) != -1) {
            exp[i] = 0;
            if(i+1 < sizeof(exp))
                exp[i+1] = 0;
        }
    exp -= ({0});
    return implode(exp, "");
}


int kauf (string str)
{
    notify_fail ("Sage doch einfach, was Du willst.\n");
    return 0;
}


void dann_halt_nicht ()
{
    if (!x || !mappingp (x)) return;
    if (x["timeoutmessage"])
      out_message(x["timeoutmessage"],kunde, x["good"]);
    else
      out_message(get_conversation("dann_halt_nicht",0,
          ({Der(this_object())+" seufzt tief.",
            "!sage Dann halt nicht."})), kunde, x["good"]);
    if (last_given && !im_lager (last_given))
      geben_wtp (last_given, kunde);
    if (x["good"] && objectp (x["good"]))
      x["good"]->remove ();
    x = 0;
    last_given = 0;
    last_will = 0;
}


void set_long (mixed x)
{
    if (x && !stringp (x)) {
      ::set_long (x);
      lang = 0;
    }
    else lang = x;
}


protected string query_long_postprocess(string msg, mapping info)
{
    mixed ob;

    if (!lang) return ::query_long_postprocess(msg,info);
        
    if (x && (ob = x["good"]) && objectp (ob) && !environment (ob))
        return wrap(lang)+this_object()->query_hp_string()+
               wrap(Er(this_object())+" hält "+einen(ob)+" in "+
                    seinem((["name":"hand","gender":"weiblich"]),({}),
                           this_object())+".");

    return wrap(lang)+this_object()->query_hp_string();
}


/*
FUNKTION: add_to_filter
DEKLARATION: void add_to_filter(mixed s)
BESCHREIBUNG:
    Fuegt beim Verkaeufer einen String oder ein Stringarray zu den
    Woertern hinzu, die er herausfiltert.
VERWEISE: delete_from_filter, query_filter, set_filter, query_react_on
GRUPPEN: verkaeufer, handel, monster
*/
/*
FUNKTION: delete_from_filter
DEKLARATION: void delete_from_filter(mixed s)
BESCHREIBUNG:
    Entfernt beim Verkaeufer einen String oder ein Stringarray von den
    Woertern, die er herausfiltert.
VERWEISE: add_to_filter, query_filter, set_filter, query_react_on
GRUPPEN: verkaeufer, handel, monster
*/
/*
FUNKTION: query_filter
DEKLARATION: string* query_filter()
BESCHREIBUNG:
    Liefert vom Verkaeufer das Stringarray der Woerter, die er herausfiltert.
VERWEISE: add_to_filter, delete_from_filter, set_filter, query_react_on
GRUPPEN: verkaeufer, handel, monster
*/
/*
FUNKTION: set_filter
DEKLARATION: void set_filter(string* s)
BESCHREIBUNG:
    Setzt beim Verkaeufer das Stringarray mit den Woertern, die er
    herausfiltert.
VERWEISE: add_to_filter, delete_from_filter, query_filter, query_react_on
GRUPPEN: verkaeufer, handel, monster
*/

/*
FUNKTION: add_react_on
DEKLARATION: void add_react_on(mixed s)
BESCHREIBUNG:
    Fuegt beim Verkaeufer einen String oder ein Stringarray zu den
    Woertern hinzu, auf die er reagiert.
VERWEISE: delete_react_on, set_react_on, query_react_on, query_filter
GRUPPEN: verkaeufer, handel, monster
*/
/*
FUNKTION: delete_react_on
DEKLARATION: void delete_react_on(mixed s)
BESCHREIBUNG:
    Entfernt beim Verkaeufer einen String oder ein Stringarray von den
    Woertern, auf die er reagiert.
VERWEISE: add_react_on, set_react_on, query_react_on, query_filter
GRUPPEN: verkaeufer, handel, monster
*/
/*
FUNKTION: query_react_on
DEKLARATION: string* query_react_on()
BESCHREIBUNG:
    Liefert vom Verkaeufer das Stringarray der Woerter, auf die er reagiert.
VERWEISE: add_react_on, delete_react_on, set_react_on, query_filter
GRUPPEN: verkaeufer, handel, monster
*/
/*
FUNKTION: set_react_on
DEKLARATION: void set_react_on(string* s)
BESCHREIBUNG:
    Setzt beim Verkaeufer das Stringarray mit den Woertern, auf die er
    reagiert.
VERWEISE: add_react_on, delete_react_on, query_react_on, query_filter
GRUPPEN: verkaeufer, handel, monster
*/

void add_to_filter (mixed s)
{
    if (stringp (s)) filter += ({s});
    else filter += s;
}

void delete_from_filter (mixed s)
{
    if (stringp (s)) filter -= ({s});
    else filter -= s;
}

string *query_filter ()
{
    return filter;
}

void set_filter (string *s)
{
    filter = s;
}

private void compute_react_on_regexp ()
{
    react_on_regexp = sizeof(react_on) && convert_umlaute("("+implode (react_on,"|")+")");
}

string* query_react_on ()
{
    return react_on;
}

void add_react_on (mixed add)
{
    if (stringp (add))
        react_on += ({add});
    else if (pointerp (add))
        react_on += add;
    else fehlermeldung ("Verkäufer: add_react_on wurde etwas "
        "übergeben, was weder string noch ein Array ist, nämlich: "
        +mixed2str (add));
    compute_react_on_regexp ();
}

void set_react_on (string* add)
{
    react_on = add;
    compute_react_on_regexp ();
}

void delete_react_on (mixed del)
{
    if (stringp (del))
        react_on -= ({del});
    else if (pointerp (del))
        react_on -= del;
    else fehlermeldung ("Verkäufer: delete_react_on wurde etwas "
        "übergeben, was weder string noch ein Array ist, nämlich: "
        +mixed2str (del));
    compute_react_on_regexp ();
}

void notify_comm(object wer, mixed wen, string what, string adverb, int flags,
    int msg_typ_wer, int msg_typ_wen, int msg_typ_andere)
{
    if (adverb) {
        adverb = lower_case (adverb);
        if (wer!=this_object() && (wen==this_object() || !wen)
            && (what=="sag" || what=="frag" || what=="fluester" || what=="lall")
            && react_on_regexp
	    && (sizeof (regexp (({convert_umlaute(adverb)}), react_on_regexp)) > 0))
            kaufaktion(leere(adverb), wer);
    }
}

void create ()
{
    ::create ();
    initialize ("mensch",60);
    add_id ("verkäufer");
    add_id ("verkäuferin");
    set_personal (1);
    give_hp (111); give_sp (0);
    give_weapon_level (6); // give_armour_level (3);
    // set_weight(300);
    seteuid (getuid ());
    set_valuta (DEFAULT_VALUTA);
    filter = ({"will","möcht","möchte","gern","gerne","bitte","haben",
               "ein","eine","einen","kaufen","einkaufen",""});
    react_on = ({"\\<haett","\\<moecht","\\<kaufen\\>","\\<bitte\\>",
        "\\<will\\>","\\<brauch\\>","\\<brauche\\>"});
    compute_react_on_regexp ();
    call_out ("starte_gegeben",2);

    add_controller("notify_comm", this_object());
}


void init()
{
    ::init ();
    add_action("kauf","kaufe",-4);
    add_action("kauf","bestelle",-7);
}


int remove ()
{
    if (x && x["good"] && objectp(x["good"]))
      x["good"]->remove ();
    return ::remove ();
}

/*
FUNKTION: notify_sold
DEKLARATION: void notify_sold(object seller,object what, object whom, object where)
BESCHREIBUNG:
When what durch seller an whom verkauft wurde, wird 
ENV(seller)->notify("sold",seller,what,whom) aufgerufen.
Hinweis: Es sollte stattdessen notify_sold_here genutzt werden.
VERWEISE: notify_sold_here, notify_sold_me, notify_sold_whom, notify_sold_what
GRUPPEN: verkaeufer, handel
*/

/*
FUNKTION: notify_sold_here
DEKLARATION: void notify_sold_here(object seller,object what, object whom, object where)
BESCHREIBUNG:
When what durch seller an whom verkauft wurde, wird 
ENV(seller)->notify("sold_here",seller,what,whom) aufgerufen.
VERWEISE: notify_sold_me, notify_sold_whom, notify_sold_what
GRUPPEN: verkaeufer, handel
*/

/*
FUNKTION: notify_sold_me
DEKLARATION: void notify_sold_me(object what, object whom, object where,object seller)
BESCHREIBUNG:
When what durch seller an whom verkauft wurde, wird 
seller->notify("sold_me",what,whom,where) aufgerufen.
VERWEISE: notify_sold_here, notify_sold_whom, notify_sold_what
GRUPPEN: verkaeufer, handel
*/

/*
FUNKTION: notify_sold_what
DEKLARATION: void notify_sold_what(object seller, object whom, object where,object what)
BESCHREIBUNG:
When what durch seller an whom verkauft wurde, wird 
what->notify("sold_what",seller,whom,where) aufgerufen.
VERWEISE: notify_sold_here, notify_sold_whom, notify_sold_me
GRUPPEN: verkaeufer, handel
*/

/*
FUNKTION: notify_sold_whom
DEKLARATION: void notify_sold_whom(object seller,object what, object where, object whom)
BESCHREIBUNG:
When what durch seller an whom verkauft wurde, wird 
whom->notify("sold_whom",seller,what,where) aufgerufen.
VERWEISE: notify_sold_here, notify_sold_what, notify_sold_me
GRUPPEN: verkaeufer, handel
*/

