// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------------------
// File:        /obj/streichhoelzer.c
// Description: Zündhölzer
// Autoren:     Basierend auf einer Version von Kurdel, die von Zandru, Onit
//                und Jackie (Aug 96 bis Jan 99) weiterentwickelt wurde.
//              Neu überarbeitet von Anin, mit Hilfe von Gnomi, Sin und Croft
//                Januar 2001

// ----------------------------------------------------------------------------
// TODO - Streichhölzer zusamenführen überprüfen obs klappt
//        (hat man noch mehrere einzelne abgebrannte/brennende bis zum 1. move)
//      - klären warum virtuelles nicht geteste wird ob's schon brennt
//      - kurzzeitiger Licht-Effekt bim Zünden einbauen?
//        query_own_light in /obj/feuer_shadow bewirkt leider obwohl das
//        streichholz im inv dann leuchtet keinerlei erleuchtung des raumes
//
// ----------------------------------------------------------------------------

inherit "/i/object/multiob";
inherit "/i/value";
private inherit "/i/tools/item_types";

#include <parse_com.h>   // PARSE_*
#include <move.h>        // MOVE_OK
#include <message.h>     // MT_*, MA_*
#include <room_types.h>  // RT_LANDSCHAFT, L_UNTERWASSER
#include <notify_fail.h> // FAIL_*
#include <simul_efuns.h> // IST_SPACE_BEFORE

#define FEUER_SHADOW  "/obj/feuer_shadow"
#define TP this_player()
#define TO this_object()

// ----------------------------------------------------------------------------
/*
FUNKTION: query_streichholz
DEKLARATION: int query_streichholz()
BESCHREIBUNG:
Liefert 1 zurück, wenn das Objekt ein Streichholz ist
VERWEISE: query_burning, query_burned_out, query_anzuender
GRUPPEN: feuer
*/
int query_streichholz()
{
    return 1;
}
/*
FUNKTION: query_burning
DEKLARATION: int query_burning()
BESCHREIBUNG:
Liefert 1 zurück, wenn das Objekt brennt.
(offenes Feuer, was potentiell Dinge entzünden kann)
VERWEISE: query_streichholz, query_burned_out, query_anzuender
GRUPPEN: feuer
*/
int query_burning()
{
    return strstr(query_count_type(),"#brennend") != -1;
}

/*
FUNKTION: query_burned_out
DEKLARATION: int query_burned_out()
BESCHREIBUNG:
Liefert 1 zurück, wenn das Objekt schonmal gebrannt hatte und vor der totalen
Vernichtung gelöscht wurde.
Für Streichhölzer bedeutet dies, dass sie nicht mehr selbst zünden, sonden
nur noch mit Fremdeinwirkung brennen können.
VERWEISE: query_streichholz, query_burning, query_anzuender
GRUPPEN: feuer
*/
int query_burned_out()
{
    return strstr(query_count_type(),"#abgebrannt") != -1;
}

/*
FUNKTION: query_anzuender
DEKLARATION: int query_anzuender()
BESCHREIBUNG:
Liefert 1 zurück, wenn das Objekt noch zum Anzünden von etwas anderem taugt.
Auch dafür gedacht, dass man testen kann, ob Jemand einen Anzünder
parat hat, mit dem evtl. auch 'zünde bla' direkt (OHNE 'mit blubb') geht.
Bei Streichhölzern: entweder es brennt gerade, oder es kann noch von selbst
                     entzündet werden.
VERWEISE: query_streichholz, query_burning, query_burned_out
GRUPPEN: feuer
*/
int query_anzuender()
{
    return (query_burning() || !query_burned_out());
}
// ----------------------------------------------------------------------------

void create()
{
    ::create();
    seteuid(getuid());
    set_id(({"streichholz","holz","zündholz","feuerholz"}));
    set_plural_id(({"streichhölzer","hölzer","zündhölzer","feuerhölzer"}));
    set_singular_name("streichholz");
    set_plural_name("streichhölzer");
    set_gender("saechlich");
    set_plural(1);
    set_material(({"holz"}));
    set_count(10);
    set_count_type("streichhölzer");
    set_singular_value(1);
    add_controller(({"concerned_strike","notify_unshadow"}),this_object());
}

object split_object(int i)
{
    object ob;
    if((ob = multiob::split_object(i)) && TO->query_burning())
          clone_object(FEUER_SHADOW).init_shadow(ob);
    return ob;
}

void init()
{
    add_action("zuendele","zündele",-4);
    add_action("zuendele","entzünde",-7);
}
// ----------------------------------------------------------------------------

/*
FUNKTION: allowed_strike
DEKLARATION: allowed_strike()
BESCHREIBUNG:
Wenn das Streichholz prüft, ob etwas angezündet werden kann, und dies
gehen soll,
  * obwohl das Material nicht dafür spricht,
    z.B. trockenes Stroh das Material MAT_PFLANZLICH hat, welches ansich
    nicht brennt
  * obwohl es no_move oder invis ist
dann kann das zu prüfende Objekt mit dem controller allowed_strike dafür
sorgen.

Dies ist nicht dazu da, sich selbst um das Brennen zu kümmern, denn das
macht concerned_strike sondern dazu dass ein Feuer-Shadow von dem Anzünder
übergeworfen wird, als wäre das Objekt ein bewegliches Stück Holz.

Beispiel für einen Strohsessel:
im create():
   add_controller("allowed_strike", this_object());

außerdem:
int allowed_strike()
{
    return !query_no_move() && !IS_INVIS(TO);
}

VERWEISE: brennbar, notify_strike, do_strike, concerned_strike
GRUPPEN: feuer
*/

int brennbar(mixed ob)
{
    if (!objectp(ob))
        return 0;
    if (ob->allowed("strike"))
        return 1;
    return ::query_is_brennbar(ob);
}
// ----------------------------------------------------------------------------

/*
FUNKTION: prepare_strike
DEKLARATION: int prepare_strike()
BESCHREIBUNG:
Diese Funktion ist als Meldungs-Vorgeplänkel für ein erfolgreiches
Zünden gedacht und kann natürlich auch von Objekten, die sich selbst um ihr
Anzünden kümmern mitverwendet werden.
Es wird 1 returnt, damit man weiß, dass es die Funktion gab und was passierte.
Beispiel: (in einer Bombe:)
  void do_strike(object womit, mixed was, mixed melder)
  {
    womit->prepare_strike();
    send_message(MT_LOOK,MA_USE,
                 Der(TP)+" hält "+seinen(melder)+
                 " an die Zündschnur die sofort Feuer fängt.",
                 "Du hältst "+deinen(melder)+
                 " an die Zündschnur die sofort Feuer fängt.",
                 TP);
    womit->strike_done();

    send_message(MT_LOOK|MT_NOISE|MT_FEEL,MA_UNKNOWN, "KRAWUMM!");
    remove();
  }
VERWEISE: add_controller, concerned, notify_strike, do_strike, concerned_strike
GRUPPEN: feuer
*/
int prepare_strike()
{

    send_message_to(TP,MT_LOOK,MA_USE,
                    "Du kramst "+einen((["name":query_singular_name(),
                         "gender":query_gender()]),"passend")+" hervor.");
    return 1;
}
// ----------------------------------------------------------------------------
/*
FUNKTION: strike_done
DEKLARATION: int strike_done()
BESCHREIBUNG:
Diese Funktion gibt eine Meldung 'Dein Streichholz erlischt.' aus und
vernichtet ein Streichholz. Sie ist als Abschluß für ein erfolgreiches
Zünden gedacht und kann natürlich auch von Objekten, die sich selbst um ihr
Anzünden kümmern mitverwendet werden.
Es wird 1 returnt, damit man weiß, dass es die Funktion gab und was passierte.
Beispiel: (in einer Bombe:)
  void do_strike(object womit, mixed was, mixed melder)
  {
    send_message(MT_LOOK,MA_USE,
                 Der(TP)+" entzündet sich "+ein(melder)+" und hält "
                 +es(melder)+" an die Zündschnur, die sofort Feuer fängt.",
                 "Du entzündest dir "+ein(melder)+" und hältst "
                 +es(melder)+" an die Zündschnur, die sofort Feuer fängt.",
                 TP);
    womit->strike_done();

    send_message(MT_LOOK|MT_NOISE|MT_FEEL,MA_UNKNOWN, "KRAWUMM!");
    remove();
  }
VERWEISE: add_controller, concerned, notify_strike, do_strike, concerned_strike
GRUPPEN: feuer
*/
int strike_done()
{

    send_message(MT_LOOK, MA_USE,
                 Der((["name":query_singular_name(),"gender":query_gender()]))+
                 " erlischt.");
    add_count(-1);
    return 1;
}
// ----------------------------------------------------------------------------

/*
FUNKTION: do_notifying
DEKLARATION: void do_notifying(mixed what, object player, mixed test_ob)
BESCHREIBUNG:
Funktion, die das Verteilen der üblichen notifys übernimmt.
Dabei ist 'what' das was angezündet wurde, 'player' der Anzündende und
'test_ob' what["environment"] falls 'what' ein v_item ist.
VERWEISE: add_controller, concerned, notify_strike, do_strike, concerned_strike
GRUPPEN: feuer
*/
void do_notifying(mixed what, object player, mixed test_ob)
{
    object *n_obs;
    n_obs=({});

    notify("strike",what,player); // TO wird autom. angehängt

    //Sicherheitshalber mal über ein Feld, damit auch garantiert keine
    //Doppelaufrufe kommen (Idee von Parsec)

    n_obs=n_obs-({player})+({player});
    n_obs=n_obs-({environment(player)})+({environment(player)});
    if(objectp(what))
        n_obs=n_obs-({what})+({what});
    else
        if(objectp(test_ob))
            n_obs=n_obs-({test_ob})+({test_ob});
    map_objects(n_obs,"notify","strike",what,player,TO);

}
// ----------------------------------------------------------------------------
/*
FUNKTION: concerned_strike
DEKLARATION: int concerned_strike(object womit, mixed was)
BESCHREIBUNG:
Wenn ein Objekt 'was' angezündet werden soll, wird zuvor
was->concerned("strike",this_object()) aufgerufen. Die Returnwerte von 'was'
und per controller bei 'was' angemeldeten Objekten werden als Prioritaten
angesehen, wobei das Objekt 'ob' mit der höchsten Priorität den Zuschlag
erhält und per ob->do_strike(streichholz,was,melder); dazu aufgefordert wird
sich um das entzünden von 'was' zu kümmern.
Hinweis: Wenn 'was' ein v_item ist mit eintrag "einvironment" : env_ob,
         so wird env_ob->concerned("strike",this_object(),was) aufgerufen
VERWEISE: add_controller, concerned, notify_strike, do_strike
GRUPPEN: feuer, grundlegendes
*/
int concerned_strike(object womit, mixed was)
{
    // ist dieses Objekt wirklich das richtige?
    if ((was == TO) || (mappingp(was) && here(was["name"]))) {

        // kann das noch selbst zünden?
        // wenn nicht kümmert es sich garantiert nicht selbst ums Anzünden
        if (was->query_burned_out())
            return 0;
        else

            // ist der Anzünder ein Streichholz, dann mit hoher Priorität
            // selbstzünden ebenso wenn nix fremdes mit beim Zünden beteiligt
            if (!womit || womit->query_streichholz())
                return 100;
            else

                // niedrige Priorität, da wohl in den seltensten Fällen von
                // anderem nur ein Streichholz gezielt angezündet wird, es
                // aber trotzdem möglich sein soll.
                return 10;
    }
}

/*
FUNKTION: do_strike
DEKLARATION: int do_strike(object womit, mixed was, mixed melder)
BESCHREIBUNG:
Diese Funktion wird in dem Objekt aufgerufen, welches über concerned_strike
die höchste Priorität zurückgeliefert hat und sich somit dafür zuständigst
erklärt hat sich um das Anzünden von was zu kümmern.
( Dabei kann was auch ein v_item sein )
Es sollte innerhalb der Funktion was wenn möglich irgendwie angezündet werden
und im Erfolgsfall 1 sowie im Misserfolgsfall 0 returnt werden.
Dieser Returnwert wird im zünden der Streichhölzer übernommen,
also return 0 nur wenn wirklich aus einem triftigen Grund nix geht
( sonst wäre man ja lieber nicht zuständig ;)
'melder' ist dabei das womit angezündet wird, in einer Form, die zur Erzeugung
von Meldungen sinnig ist, (Bei Streichholz ein mapping mit einzelnem Holz und
nicht alle auf einmal, bei Gildenzaubern wäre das ein mapping mit dem Namen
für den Zauber, damit nicht das gilden_ob oder so kommt)
VERWEISE: add_controller, notify_strike, concerned_strike
GRUPPEN: feuer, grundlegendes
*/
int do_strike(object womit, mixed was, mixed melder)
{
    object countob;

    if ((was == TO) || (mappingp(was) && here(was["name"])))
    {
        countob = TO.split_object(1);
        if (!countob) // sollte nicht vorkommen
            return notify_fail("Sehr merkwürdig. Das geht nicht!\n", FAIL_INTERNAL);
        countob.set_count_type(countob.query_count_type()+"#brennend");
        clone_object(FEUER_SHADOW).init_shadow(countob);

        // kein Platz für brennendes Holz -> weg damit & nichtbrennendes zurück
        if (!environment(countob) && countob.move(environment(TO)) != MOVE_OK)
        {
            countob.remove();
            if (TO)
                TO.add_count(1);
            return notify_fail("Du hast schon alle Hände voll mit Deinem "
                        "vielen Gepäck.\n", FAIL_INTERNAL);
        }
        send_message(MT_LOOK,
                     MA_USE,
                     Der(TP)+" entzündet "
                          +einen((["name":query_singular_name(),
                                   "gender":query_gender()])) + ".",
                     "Du entzündest "
                          +einen((["name":query_singular_name(),
                                   "gender":query_gender()])) + ".",
                     TP);
        return 1;
    }
}
// ----------------------------------------------------------------------------

/*
FUNKTION: notify_strike
DEKLARATION: void notify_strike(mixed what, object player, object with)
BESCHREIBUNG:
Wenn ein Objekt angezündet wurde, d.h. entweder die Streichhölzer haben es
angezündet (feuer-shadow wurde übergeworfen) oder das Objekt was sich um das
Anzünden kümmert hat do_notifying in den streichhölzern aufgerufen,

dann wird notify("strike",what,player,with) in
den Streichhölzern, this_player(), in environment(this_player()) und
falls 'what' ein Objekt ist, in 'what' und, falls 'what' ein v_item mit
v_item["environment"] ist, in v_item["environment"]
aufgerufen.
'what' ist dabei das angezündete Objekt (resp. v_item), 'with' sind die
Streichhölzer und 'player' ist this_player().
VERWEISE: add_controller, concerned_strike, do_strike
GRUPPEN: feuer, grundlegendes
*/
// ----------------------------------------------------------------------------

int zuendele(string str)
{
    mixed ob, zustaendig, *parsed, *obs, test_ob;
    object env;
    int pos;


    str = lower_case(space(str));

    // testen ob nicht jemand anderes gemeint war ( 'zünde bla mit fackel' )

    if((pos = strstr(str," mit ")) !=-1)
    {

        if(!me(str[pos+4..]))
            return notify_fail("Zünde was womit? "
                        "Bist Du sicher, dass das geht?\n",
			FAIL_NOT_OBJ);
    }

    // erstmal testen ob das Streichholz da wo es ist überhaupt brennen darf
    // wenn nicht geht natürlich auch kein Zünden ;)

    env=environment();
    while(env)
    {
        if(env->allowed("burn",TO))
            break;
        else if(env->forbidden("burn",TO))
            return notify_fail(wrap("Du kannst "+den()+" hier nicht anzünden."),
		FAIL_INTERNAL);
        else if(env->query_room()&&(env->query_type(RT_LANDSCHAFT)&L_UNTERWASSER))
            return notify_fail(wrap("Du kannst "+den()+" unter Wasser nicht "
                             "anzünden."), FAIL_INTERNAL);
        env=environment(env);
    }

    // Nehmen vor dem Zünden

    if(environment(TO) != TP)
        return notify_fail("Erst nehmen, dann zünden!\n", FAIL_INTERNAL);
    if (TP->query_num_free_hands()<=0)
        return notify_fail("Du hast keine freie Hand zum zündeln.",
            FAIL_INTERNAL);
    if(query_burned_out())
        return notify_fail(wrap(Der(TO)+" entzündet sich nicht mehr."),
	    FAIL_INTERNAL);
    // bis hier geht es alleine um das Streichholz, das anzünden soll


    if (!sizeof(parsed = parse_com(str,0,0,PARSE_ALL_MATCHES)[PARSE_OBS]))
        return notify_fail("Was möchtest Du anzünden?\n", FAIL_WRONG_ARG);
    if(!sizeof(obs = filter(parsed,#'objectp)))
        ob = parsed[0]; // dann eben eines der virtuellen Objekte

    // TODO hmm virtuelle objekte werden nicht getestet ob sie schon brennen!?

    else
        if (sizeof(parsed = filter(obs,
                                (: return ((!$1->query_burning()) &&
                                           (!$1->query_is_lighted())); :) )))

                ob = parsed[0]; // das erste nicht brennende Objekt
        else
            return notify_fail(wrap(Der(obs[0],({}))+
		    plural(" brennt"," brennen", obs[0])+
                    " doch schon!"), FAIL_INTERNAL);
    // ok, parsen soweit ok...

    if (objectp(ob))
    {
        // wenn ein Streichholz im Raum liegt wird das beim Parsen gefunden
        // ist aber blöd mit Fehlermeldung abzubrechen, weil man ja ganausogut
        // das Streichholz was man bei sich hat anzünden kann -> kleiner Hack:

        if ((ob->query_streichholz()) && (environment(ob) != TP) && me(str))
        {
            if (TO->query_burning())
                return notify_fail(wrap(Dein(TO)+
		    plural(" brennt", " brennen", TO)+" doch schon!"),
		    FAIL_INTERNAL);

            ob = TO;
        }
    }
    else if(QUERY("far",ob))
	return notify_fail(wrap(Dein(ob)+ist(ob, IST_SPACE_BEFORE)+" viel zu weit "
            "weg, um "+ihn(ob)+" anzuzünden."), FAIL_WRONG_ARG);

    // Ab hier Controller eingebaut:

    if (objectp(ob))
        zustaendig = ob->concerned("strike", TO); //ob wird autom. angehängt
    else
    {
        test_ob = ob;
        while(mappingp(test_ob = test_ob["environment"]));
        zustaendig = ( objectp(test_ob) ?
                       test_ob->concerned("strike",TO,ob) :
                       environment(TP)->concerned("strike",TO,ob) );
    }


    if (zustaendig)
    {
        // das ganze Anzünden und evtl Aufruf von strike_done() und
        // do_notifying(...) wird dem Zuständigen in do_strike überlassen

        if(closurep(zustaendig))
            return funcall(zustaendig,"do_strike",TO,ob,
                           (["name":query_singular_name(),
                             "gender":query_gender()]));
        else
            return zustaendig->do_strike(TO,ob,(["name":query_singular_name(),
                                                 "gender":query_gender()]));
    }

    // Ende Controller

    prepare_strike();

    send_message(MT_LOOK, MA_USE,
                 Der(TP)+" versucht, mit "
                      +einem((["name":query_singular_name(),
                               "gender":query_gender()]))+
                      " "+den(ob)+" anzuzünden.",
                 "Du versuchst, mit "
                      +einem((["name":query_singular_name(),
                               "gender":query_gender()]))+
                      " "+den(ob)+" anzuzünden.",
                 TP);

    if (brennbar(ob))
    {
        clone_object(FEUER_SHADOW).init_shadow(ob);
        send_message(MT_LOOK, MA_USE,
                     Der(ob,({}))+ plural(" fängt", " fangen", ob)+
                          " an zu brennen.");
        do_notifying(ob,TP,test_ob);
    }
    else
        send_message(MT_LOOK, MA_USE,
                     "Jedoch ohne den gewünschten Erfolg.");

    return strike_done();
}

// ----------------------------------------------------------------------------
// Brennende Streichhölzer nicht mit anderen vermischen, nach Löschen
// aber rücksetzen

void notify_unshadow(object shadow, object owner)
{
    if(load_name(shadow) == FEUER_SHADOW)
    {
        set_count_type(explode(query_count_type(),"#")[0]+"#abgebrannt");
        set_adjektiv("abgebrannt");
    }
}
// ----------------------------------------------------------------------------
