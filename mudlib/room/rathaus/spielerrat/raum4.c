// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/room/rathaus/spielerrat/raum4.c
// Description: Rathaus, Spielerratsraeume
// Author:      Goldie (11.2.2001)

inherit "%room";
inherit "%rauswurf";

#include <apps.h>
#include <invis.h>
#include <message.h>
#include <move.h>
#include <level.h>
#include <simul_efuns.h>

void reset()
{
    object tisch;
    
    "*"::reset();

    if(!present("Tisch1"))
    {
        tisch = clone_object("/obj/tisch");
        tisch->set_name("abstelltisch");
        tisch->set_gender("maennlich");
        tisch->set_adjektiv("klein");
        tisch->set_id(({"Tisch1","tisch","abstelltisch"}));
        tisch->set_long("Ein kleiner Abstelltisch steht in der Mitte des "
            "Raumes. Praktischerweise wurde er so plaziert, dass er vom "
            "Sofa aus problemlos und bequem zu erreichen ist.");
        tisch->set_smell ("Er riecht angenehm nach Holz.");
        tisch->set_feel  ("Du fühlst glattes poliertes Holz.");
        tisch->set_put_verb ("leg");
        tisch->set_content_message ("        Auf dem Tisch liegt herum:");
        tisch->set_invis(V_NOLIST);
        tisch->move(this_object());
    }
#ifdef UNItopia
    if(!present("Sofaecke1"))
    {
        object sofa=clone_object("/p/Item/Moebel/Sessel/obj/sessel");
        sofa->set_name("sofaecke");
        sofa->set_gender("weiblich");
        sofa->set_id(({"sofa","Sofaecke1","sofaecke","ecke","sessel"}));
        sofa->set_long("Diese Ecke ist sehr nett eingerichtet. Mehrere Sofas "
            "und Sessel stehen um einen Tisch herum und laden zum Entspannen "
            "ein.");
        sofa->set_invis(V_NOLIST);
	sofa->set_sitting_invis(V_VIS);
        sofa->set_feel("Die Polster der Möbel fühlen sich weich an.");
        sofa->set_no_move_reason("Warum möchtest Du denn die Möbel "
            "mitnehmen? Lass sie lieber hier, da haben mehr Leute was davon.");
        sofa->set_commands(({({"sitz","in"}), ({"setze","in"}),
            ({"setz","in"}),({"sitz","auf"}), ({"setze","auf"}),
            ({"setz","auf"}), }),({({"steh","auf"}), ({"stehe","auf"}) }) );
        sofa->set_sit_message("Du nimmst in der Sofaecke Platz.",
            "$Der(OBJ_TP) nimmt in der Sofaecke Platz.");
        sofa->set_adjectiv(0);
        sofa->set_long_text(" hast es Dir hier gemütlich gemacht.\n",
            " haben es sich in der Ecke gemütlich gemacht.\n",
            " hat es sich dort gemütlich gemacht.\n");
        sofa->set_fail_messages("In $dem(OBJ_TO) ist kein Platz mehr.");
        sofa->set_heal_data(0,0,0);
        sofa->set_max_persons(20);
        sofa->move(this_object());
    }
    if(!present("rosenspiegel"))
    {
        object rosenspiegel=clone_object("/p/Item/Rosen/obj/rosenspiegel");
	rosenspiegel->set_invis(V_NOLIST);
	rosenspiegel->move(this_object());
    }	
#endif
}

void create()
{
    "*"::create();
    set_short("Spielerratsraum");
    set_long(
        "Hier wurde eine gemütliche Sofaecke für Spielerräte und Admins "
        "geschaffen. Gedämpftes Licht gibt dem Raum eine erholsame "
        "Atmosphäre. An der hinteren Wand befindet sich eine kleine Saftbar, "
	"über der ein großer silberner Spiegel hängt, "
    	"und in der Mitte des Raumes ein kleiner Abstelltisch. Eine "
        "unscheinbare Tafel hängt auch noch an der Wand.");
#ifndef UNItopia
    add_v_item(([
        "name":   "sofaecke",
        "gender": "weiblich",
        "id":   ({"sofa","sofaecke","ecke","sessel"}),
        "long":   "Diese Ecke ist sehr nett eingerichtet. Mehrere Sofas und "
                  "Sessel stehen um einen Tisch herum und laden zum "
                  "Entspannen ein.",
        "feel":   "Die Polster der Möbel fühlen sich weich an.",
        "take":   "Warum möchtest Du denn die Möbel mitnehmen? Lass sie "
                  "lieber hier, da haben mehr Leute was davon." ]));
#endif
    add_v_item(([
        "name":   "saftbar",
        "gender": "weiblich",
        "id":   ({"bar","saftbar","tresen"}),
        "long":   "Die Saftbar besteht eigentlich nur aus einem kleinen "
                  "Tresen, an dem man sich verschiedene Säfte und Shakes "
                  "mixen kann. An der Wand dahinter hängt eine kleine Liste.",
        "smell":  "Sie hat einen fruchtigen Geruch, wohl von den "
                  "Fruchtsäften.",
        "take":   "Und wo soll man dann seine Säfte herbekommen?",    
        "look_msg":  "$Der() wirft einen durstigen Blick in Richtung "
                  "Saftbar." ]));

    add_v_item(([
        "name":   "wände",
        "gender": "weiblich",
        "plural": 1,
        "id":   ({"wand","wände"}),
        "long":   "Die Wände sind in ein schummriges Licht getaucht und an "
                  "der hinteren befindet sich eine Saftbar. Auch hängt dort "
                  "eine kleine Liste.",
        "take":   "Na klar, und dann stürzt das ganze Rathaus ein." ]));

    add_v_item(([
        "name":   "liste",
        "gender": "weiblich",
        "id":   ({"liste"}),
        "long":   "Eine Liste hängt an der hinteren Wand. Sie ist schon "
                  "etwas vergilbt.",
        "read":   "  .+\"+.+\"+.+\"+.+\"+.+\"+.+\"+.+\"+.+\"+.+\"+.+\"+.+\"+."
		          "+\"+.+\"+.+\"+.+\"+.\n"
                  " |                                                 "
		          "            |\n"
                  " | Man kann sich an der Saftbar verschiedene Sachen"
		          " mixen:     |\n"
                  " |                                                 "
		          "            |\n"
                  " | Säfte:   Fruchtsaft            Apfelsaft        "
		          "            |\n"
                  " |          Traubensaft           Orangensaft      "
		          "            |\n"
                  " |          Tomatensaft           Bananensaft      "
		          "            |\n"
                  " |          Kirschsaft            Ananassaft       "
                  "            |\n"
                  " |          Nektarinensaft        Multivitaminsaft "
                  "            |\n"
                  " |          Pfirsischsaft         Grapefruitsaft   "
                  "            |\n"
                  " |          Karottensaft          Aprikosensaft    "
                  "            |\n"
                  " |          Himbeersaft           Waldmeistersaft  "
                  "            |\n"
                  " |          Brombeersaft          Johannisbeersaft "
                  "            |\n"
                  " |          Birnensaft            Orange-Marakuja-"
                  "Saft         |\n"
                  " |          Tropicsaft            Orange-Kiwi-Saft "
                  "            |\n"
                  " |          Gummibärsaft          Heidelbeersaft   "
                  "            |\n"
                  " |                                                 "
                  "            |\n"
                  " | Milch:  Vanillemilch        Limonade:  Zitronenlimonade "
                  "    |\n"
                  " |         Erdbeermilch                            "
                  "            |\n"
                  " |         Bananenmilch                            "
                  "            |\n"
                  " |         Schokomilch                             "
                  "            |\n"
                  " |                                                 "
                  "            |\n"
                  " |            oder man mixt einfach drauf los und  "
                  "            |\n"
                  " |                 lässt sich überraschen.       "
                  "            |\n"
                  " |                                                 "
                  "            |\n"
                  "  \"+.+\"+.+\"+.+\"+.+\"+.+\"+.+\"+.+\"+.+\"+.+\"+."
                  "+\"+.+\"+.+\"+.+\"+.+\"+.+\" ",
        "look_msg":  "$Der() schaut sich die Liste an der Wand an.",
        "read_msg":  "$Der() liest sich die Liste über der Saftbar durch.",
        "far": 1 ]));

    set_tuer_richtung("norden");
    set_exit("gang2","norden");
    reset();
}

void init()
{
    "*"::init();
    add_action ("mixfunktion","mixe", -3);
}

void getraenk(int was)
{
    object saft, *ob;
    ob = filter_objects(deep_inventory(this_player()),"query_stay_in_pub");
    if(sizeof(ob)>0)
    { 
        send_message_to(this_player(),MT_NOTIFY,MA_UNKNOWN,
			"Du hast doch schon was zu Trinken.\n");
	return;
    }
    saft = clone_object("/obj/getraenk");
    saft->set_gender("maennlich");
    switch(was) {
        case 0:
	    saft->set_name("vanillemilch");
	    saft->set_gender("weiblich");
	    saft->set_long("Eine leckere weiße Vanillemilch.");
	    saft->set_id(({"drink","vanillemilch","getränk","milch"}));
	    saft->set_smell("Sie riecht nach Vanille, etwas wie Eiscreme.");
	    saft->set_success_message("Sie schmeckt nach Vanille und nach "
				      "Milch. Jamm!");
        break;
        case 1:
	    saft->set_name("erdbeermilch");
	    saft->set_gender("weiblich");
	    saft->set_long("Eine leckere kühle Milch. Sie ist weiß, mit "
			   "einem Touch rot.");
	    saft->set_id(({"drink","erdbeermilch","getränk","milch"}));
	    saft->set_smell("Sie riecht nach Milch und nach Erdbeeren.");
	    saft->set_success_message("Schluck! Schluck! Einfach lecker.");
        break;
        case 2:
	    saft->set_name("schokomilch");
	    saft->set_gender("weiblich");
	    saft->set_long("Eine leckere kühle Milch, sie hat eine leicht "
			   "schokoladige Farbe.");
	    saft->set_id(({"drink","schokomilch","getränk","milch"}));
	    saft->set_smell("Sie riecht nach Kakao.");
	    saft->set_success_message("Hmmm! Schokoladig...");
	break;
        case 3:
	    saft->set_name("bananenmilch");
	    saft->set_gender("weiblich");
	    saft->set_long("Eine leckere kühle Bananenmilch.");
	    saft->set_id(({"drink","getränk","bananenmilch","milch"}));
	    saft->set_smell("Sie riecht nach Milch und nach Bananen.");
	    saft->set_success_message("Sie schmeckt nach Bananen, "
				      "aber das war ja klar.");
	break;
        case 4:
	    saft->set_name("zitronenlimonade");
	    saft->set_gender("weiblich");
	    saft->set_long("Sie sieht aus wie... ja, wie Zitronenlimonade. "
			   "Ist wohl auch besser so, wäre ja schlimm wenn "
			   "sie anders aussehen würde.");
	    saft->set_id(({"saft","drink","limo","getränk","limonade",
			     "zitronenlimo","zitronenlimonade"}));
	    saft->set_smell("Sie riecht nach Limonade und Zitrone.");
	    saft->set_success_message("Hmm... schmeckt das erfrischend.");
	break;
        case 5:
	    saft->set_name("fruchtsaft");
	    saft->set_long("Ein leckerer roter Fruchtsaft.");
	    saft->set_id(({"saft","drink","fruchtsaft","getränk"}));
	    saft->set_smell("Er riecht sehr fruchtig.");
	    saft->set_success_message("Du kannst gar nicht schmecken, welche "
				      "Früchte da alle drin sind, aber es "
				      "schmeckt einfach lecker.");
	break;
        case 6:
	    saft->set_name("apfelsaft");
	    saft->set_long("Ein leckerer Apfelsaft.");
	    saft->set_id(({"saft","drink","apfelsaft","getränk","asaft"}));
	    saft->set_smell("Er riecht nach reifen Äpfeln.");
	    saft->set_success_message("Er schmeckt sehr erfrischend nach "
				      "Apfel.");
        break;
        case 7:
	    saft->set_name("traubensaft");
	    saft->set_long("Ein leckerer Traubensaft, nein, kein Wein.");
	    saft->set_id(({"saft","drink","traubensaft","getränk"}));
	    saft->set_smell("Er riecht nach Weintraube.");
	    saft->set_success_message("Er schmeckt nach Weintrauben, einfach "
				      "toll.");
        break;
        case 8:
	    saft->set_name("orangensaft");
	    saft->set_long("Ein frischgepresster leckerer Orangensaft.");
	    saft->set_id(({"saft","drink","orangesaft","getränk","osaft"}));
	    saft->set_smell("Er riecht nach sonnengereiften Orangen.");
	    saft->set_success_message("Schlürf! So müssen Orangen "
				      "schmecken.");
        break;
        case 9:
	    saft->set_name("tomatensaft");
	    saft->set_long("Ein roter Tomatensaft. Sieht aus wie Ketchup, "
			   "nur etwas flüssiger.");
	    saft->set_id(({"saft","drink","tomatensaft","getränk"}));
	    saft->set_smell("Er riecht sehr fruchtig.");
	    saft->set_success_message("Du trinkst den Tomatensaft mit Genuss "
				      "aus.");
        break;
        case 10:
	    saft->set_name("bananensaft");
	    saft->set_long("Ein leckerer Bananensaft.");
	    saft->set_id(({"saft","drink","bananensaft","getränk"}));
	    saft->set_smell("Er riecht nach Banane, wonach auch sonst.");
	    saft->set_success_message("Er schmeckt richtig bananig.");
        break;
        case 11:
	    saft->set_name("kirschsaft");
	    saft->set_long("Ein weinroter Kirschsaft.");
	    saft->set_id(({"saft","drink","kirschsaft","getränk"}));
	    saft->set_smell("Er riecht sehr fruchtig.");
	    saft->set_success_message("So kann auch nur ein Kirschsaft "
				      "schmecken.");
        break;
        case 12:
	    saft->set_name("ananassaft");
	    saft->set_long("Ein leicht trüber Ananassaft.");
	    saft->set_id(({"saft","drink","ananassaft","getränk"}));
	    saft->set_smell("Er riecht sehr tropisch.");
	    saft->set_success_message("Hmmm... Ananas. Einfach toll!");
        break;
        case 13:
	    saft->set_name("multivitaminsaft");
	    saft->set_long("Es ist eine trübe Flüssigkeit, aber wenns schon "
			   "Multivitaminsaft heißt, wird es wohl auch "
			   "welcher sein.");
	    saft->set_id(({"saft","drink","multisaft","multivitaminsaft",
			   "getränk"}));
	    saft->set_smell("Ui, riecht das gesund!");
	    saft->set_success_message("Das schmeckt verdammt gesund.");
        break;
        case 14:
	    saft->set_name("grapefruitsaft");
	    saft->set_long("Also wäre das jetzt rot-grün-gepunktet, "
			   "würdest Du wohl nicht glauben, dass dies aus "
			   "Grapefruits ist, aber da es mehr milchig gelb "
			   "ist, scheints wohl zu stimmen.");
	    saft->set_id(({"saft","drink","grapefruitsaft","getränk"}));
	    saft->set_smell("Es riecht ziemlich bitter.");
	    saft->set_success_message("WUAH! Schmeckt das bitter, aber Du "
				      "hast auch garnix anders erwartet.");
        break;
        case 15:
	    saft->set_name("nektarinensaft");
	    saft->set_long("Sieht irgendwie aus wie Pfirsischsaft.");
	    saft->set_id(({"saft","drink","nektarinensaft","getränk"}));
	    saft->set_smell("Er riecht fruchtig.");
	    saft->set_success_message("Obwohl er wie Pfirsischsaft aussieht, "
				      "schmeckt er doch nach Nektarinen.");
        break;
        case 16:
	    saft->set_name("pfirsischsaft");
	    saft->set_long("Sieht irgendwie aus wie Nektarinensaft.");
	    saft->set_id(({"saft","drink","pfirsischsaft","getränk"}));
	    saft->set_smell("Er riecht fruchtig.");
	    saft->set_success_message("Obwohl er wie Nektarinensaft aussieht, "
				      "schmeckt er doch nach Pfirsischen.");
        break;
        case 17:
	    saft->set_name("karottensaft");
	    saft->set_long("Der Saft hat einen rötlichen Farbton und "
			   "erinnert Dich irgendwie an Babynahrung. "
			   "Warum nur?");
	    saft->set_id(({"saft","drink","karottensaft","getränk"}));
	    saft->set_smell("Er riecht süßlich.");
	    saft->set_success_message("Er schmeckt süßlich und erinnert "
				      "Dich an Kindertage.");
        break;
        case 18:
	    saft->set_name("aprikosensaft");
	    saft->set_long("Aprikosensaft? Sieht auch nicht viel anders aus "
			   "als Pfirsisch- oder Nektarinensaft.");
	    saft->set_id(({"saft","drink","aprikosensaft","getränk"}));
	    saft->set_smell("Er riecht fruchtig");
	    saft->set_success_message("Ja, so muss Aprikosensaft schmecken. "
				      "Hmmm!");
        break;
        case 19:
	    saft->set_name("waldmeistersaft");
	    saft->set_long("So schön grün wie der ist kann das nur "
			   "Waldmeister oder einer von Mammis Hexentränken "
			   "sein.");
	    saft->set_id(({"saft","drink","waldmeistersaft","getränk"}));
	    saft->set_smell("Du kannst den Geruch nicht ganz bestimmen.");
	    saft->set_success_message("Hmmm, schmeckt das erfrischend.");
        break;
        case 20:
	    saft->set_name("himbeersaft");
	    saft->set_long("Der Himbeersaft hat eine leuchtend rote Farbe, "
			   "sieht riechtig lecker aus.");
	    saft->set_id(({"saft","drink","himbeersaft","getränk"}));
	    saft->set_smell("Er riecht beerig.");
	    saft->set_success_message("Hmm, schmeckt nicht schlecht, aber "
				      "als Wein wäre er bestimmt noch besser "
				      "gewesen.");
        break;
        case 21:
	    saft->set_name("johannisbeersaft");
	    saft->set_adjektiv("rot");
	    saft->set_long("Sieht sehr nach Johannesbeersaft aus, und weil "
			   "er so schön rot ist, wirds wohl von roten "
			   "Johannesbeeren sein.");
	    saft->set_id(({"saft","drink","johannisbeersaft","getränk"}));
	    saft->set_smell("Er riecht richtig fruchtig nach Johannisbeeren.");
	    saft->set_success_message("Hmmm... schmeckt das gut.");
        break;
        case 22:
	    saft->set_name("johannisbeersaft");
	    saft->set_adjektiv("schwarz");
	    saft->set_long("Sieht sehr nach Johannisbeersaft aus, und weil er "
			   "so eine dunkle Farbe hat, wirds wohl von "
			   "schwarzen Johannisbeeren sein.");
	    saft->set_id(({"saft","drink","johannissaft","getränk"}));
	    saft->set_smell("Er riecht fruchtig nach Johannisbeeren.");
	    saft->set_success_message("Hmmm... schmeckt das lecker.");
        break;
        case 23:
	    saft->set_name("brombeersaft");
	    saft->set_long("Ja, das scheint wirklich Brombeersaft zu sein. "
			   "Er hat eine sehr dunkelrote Farbe.");
	    saft->set_id(({"saft","drink","brombeersaft","getränk"}));
	    saft->set_smell("Er riecht richtig beerig.");
	    saft->set_success_message("Er schmeckt fruchtig nach Brombeeren.");
        break;
        case 24:
	    saft->set_name("birnensaft");
	    saft->set_long("Er sieht aus wie Bananensaft, nur etwas dünner "
			   "und etwas heller.");
	    saft->set_id(({"saft","drink","birnensaft","getränk"}));
	    saft->set_smell("Doch, Du kannst genau die Birnen darin riechen.");
	    saft->set_success_message("Der Saft schmeckt irgendwie nach "
				      "Birnen.");
        break;
        case 25:
	    saft->set_name("tropicsaft");
	    saft->set_long("Tropicsaft, was da wohl alles drin ist? Also die "
			   "Farbe ist schon sehr interessant.");
	    saft->set_id(({"saft","drink","tropicsaft","getränk"}));
	    saft->set_smell("Er riecht sehr tropisch.");
	    saft->set_success_message("Er schmeckt tropisch lecker, aber "
				      "einzelne Früchte kannst Du nicht "
				      "herausschmecken.");
        break;
        case 26:
	    saft->set_name("orange-Marakuja-Saft");
	    saft->set_long("Er sieht aus wie normaler Orangensaft, nur etwas "
			   "dunkler, was wohl an der Marakuja liegt.");
	    saft->set_id(({"saft","drink","orangensaft","getränk",
			   "marakujasaft","orangenmarakujasaft"}));
	    saft->set_smell("Er riecht etwas nach Orange und etwas nach "
			    "Marakuja. Eine interessante Kombination.");
	    saft->set_success_message("Es schmeckt interessant und fruchtig.");
	break;
        case 27:
	    saft->set_name("orange-Kiwi-Saft");
	    saft->set_long("Der Orange-Kiwi-Saft hat eine gelb-grüne Farbe, "
			   "kommt wohl weil Orangen gelb und Kiwis grün "
			   "sind, könnte aber auch an etwas anderem liegen.");
	    saft->set_id(({"saft","drink","orangensaft","getränk","kiwisaft",
			     "orangekiwisaft"}));
	    saft->set_smell("Er riecht fruchtig nach Orangen und Kiwis.");
	    saft->set_success_message("Er schmeckt interessant und fruchtig.");
        break;
        case 28:
	    saft->set_name("heidelbeersaft");
	    saft->set_long("Heidelbeersaft, aus Blaubeeren gepresst und "
			   "deswegen wohl auch die dunkle blaue Farbe.");
	    saft->set_id(({"saft","drink","heidelbeersaft","getränk"}));
	    saft->set_smell("Er riecht beerig lecker.");
	    saft->set_success_message("Er schmeckt sehr lecker und richtig "
				      "beerig.");
        break;
        case 29:
	    saft->set_name("gummibärsaft");
	    saft->set_long("Gummibärsaft? Ob der aus Gummibären gepresst "
			   "wurde? Auf jeden Fall sieht er nicht so bunt "
			   "aus.");
	    saft->set_id(({"saft","drink","gummibärsaft","getränk"}));
	    saft->set_smell("Er riecht wie ne ganze Tüte Gummibärchen.");
	    saft->set_success_message("Er schmeckt sehr süß, wie "
				      "Gummibären.");
        break;
    }
    send_message(MT_LOOK,MA_USE,
        wrap(Der(this_player())+" mixt sich etwas zu Trinken zusammen."),
        wrap("Du mixt Dir "+einen( saft, "nett" )+" zusammen."),this_player());
    saft->set_stay_in_pub(1);
    saft->set_value(0);
    saft->set_amount(40);
    if(saft->move(this_player())!=MOVE_OK)
    {
        send_message(MT_NOTIFY,MA_UNKNOWN,
            wrap(Der(this_player())+" scheint aber keinen Platz zu haben und "
		 "schüttet es wieder weg."), 
	    wrap("Da Du aber keinen Platz hast, schüttest Du "+ihn(saft)+
		 " wieder weg."),
	    this_player());
	saft->remove();
    }
}

int mixfunktion(string str)
{
    switch(lower_case(str||""))
    {
        case "":
        case "getränk":
        case "drink":
	    getraenk(random(30));
	    return 1;
        case "saft":
	    getraenk(random(25)+5);
	    return 1;
        case "milch":
	    getraenk(random(4));
	    return 1;
        case "vanillemilch":
	    getraenk(0);
	    return 1;
        case "erdbeermilch":
	    getraenk(1);
	    return 1;
        case "schokomilch":
	    getraenk(2);
	    return 1;
        case "bananenmilch":
	    getraenk(3);
	    return 1;
        case "limonade":
        case "limo":
        case "zitronenlimonade":
        case "zitronenlimo":
	    getraenk(4);
	    return 1;
        case "fruchtsaft":
 	    getraenk(5);
	    return 1;
        case "apfelsaft":
        case "asaft":
	    getraenk(6);
	    return 1;
        case "traubensaft":
	    getraenk(7);
	    return 1;
        case "orangensaft":
        case "osaft":
            getraenk(8);
	    return 1;
        case "tomatensaft":
	    getraenk(9);
	    return 1;
        case "bananensaft":
	    getraenk(10);
	    return 1;
        case "kirschsaft":
	    getraenk(11);
	    return 1;
        case "ananassaft":
	    getraenk(12);
	    return 1;
        case "multisaft":
        case "multivitaminsaft":
	    getraenk(13);
	    return 1;
        case "grapefruitsaft":
	    getraenk(14);
	    return 1;
        case "nektarinensaft":
	    getraenk(15);
	    return 1;
        case "pfirsischsaft":
	    getraenk(16);
	    return 1;
        case "karottensaft":
	    getraenk(17);
	    return 1;
        case "aprikosensaft":
	    getraenk(18);
	    return 1;
        case "waldmeistersaft":
	    getraenk(19);
	    return 1;
        case "himbeersaft":
	    getraenk(20);
	    return 1;
        case "johannissaft":
        case "johannisbeersaft":
	    getraenk(21+random(2));
	    return 1;
        case "brombeersaft":
	    getraenk(23);
	    return 1;
        case "birnensaft":
	    getraenk(24);
	    return 1;
        case "tropicsaft":
	    getraenk(25);
	    return 1;
        case "marakujasaft":
        case "orangemarakujasaft":
        case "orange-Marakuja-Saft":
	    getraenk(26);
	    return 1;
        case "orange-Kiwi-Saft":
        case "kiwisaft":
        case "orangekiwisaft":
	    getraenk(27);
	    return 1;
        case "heidelbeersaft":
	    getraenk(28);
	    return 1;
        case "gummibärsaft":
	    getraenk(29);
	    return 1;
    }
    notify_fail("Was willst Du Dir mixen?\n");
    return 0;
}

<int|string> let_not_out(mapping mv_infos)
{
    object wer = mv_infos[MOVE_OBJECT];
    object wohin = mv_infos[MOVE_NEW_ROOM];
    if(object_name(wohin) == "/room/void") 
        return 0;
	
    if(wizp(wer) || deep_present(wohin,this_object()))
        return ::let_not_out(mv_infos);

    if(wer->query_stay_in_pub() ||
       cond_deep_present(0, wer, CDP_DEPTH_FIRST, "query_stay_in_pub"))
    {
        return "Genieß Dein Getränk doch lieber hier.";
    } 
    
    return ::let_not_out(mv_infos);
}   

void moved_out(mapping mv_infos)
{
    object wer = mv_infos[MOVE_OBJECT];
    object wohin = mv_infos[MOVE_NEW_ROOM];
    if(!wizp(wer) && !deep_present(wohin,this_object()))
    {
        if(wer->query_stay_in_pub())
            call_out((: $1 && $1->remove() :), 0, wer);
        else
            filter_objects(deep_inventory(wer), "query_stay_in_pub")->remove();
    }

    ::moved_out(mv_infos);	
}
