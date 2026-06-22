// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /map/obj/hai.c
// Description: Ein gefaehrlicher Hai fuer die Ozean-Map
// Author:      Pif (20.08.97)
// Modified by: Pif (5.4.98)
//              Mit dem Stranden-inherit versehen.

inherit "/map/i/stranden";

#include <add_hp.h>
#include <stats.h>

string query_leiche_kurz()
{
  return "Die Leiche eines Haies";
}

string query_leiche_lang()
{
  return "Die Leiche eines Haies, der auf das Land geraten ist.";
}

string query_far()
{
  return wrap(Der()+" ist ständig in Bewegung, so dass Du "
         "nicht so einfach an "+ihn()+" herankommst.");
}

void create()
{
  ::create();
  initialize("hai",1);
#ifndef NEW_STATS
  set_one_stat(STAT_STR,60);
  set_one_stat(STAT_INT,40);
  set_one_stat(STAT_CON,50);
  set_one_stat(STAT_DEX,60);
  give_hands(1);
  give_hp(60);
  give_weapon_level(20);
  give_armour_level(1);
#endif
  set_gender("maennlich");
  set_id(({"hai","fisch","haifisch","knorpelfisch",
           "weisshai","carcharodon", "map fish"}));
  set_long("Unter der Wasseroberfläche kann man einen etwa fünf "
           "Meter langen Hai erkennen, der sich bedenklich oft in "
           "Deine Nähe begibt. Manchmal schwimmt er sehr dicht "
           "unter der Wasseroberfläche, so dass man ihn auch "
           "detaillierter betrachten kann."); 
  set_smell("Aus dem Maul des Haies dringt der Geruch des "
            "Blutes seiner letzten Opfer."); // Uiuiui, wie gruslig ;)
  set_aggressive(1);
  set_align(-50);

  // Er ist zwar agressiv, aber ich finde, es  muss sein:
  add_random_activities(([
    "!echo $Der() schwimmt dicht unter Dir durchs Wasser." : 0,
    "!echo $Der() zieht enger werdende Kreise um Dich." : 0,
    "!echo $Der() streift Deinen Fuß, als er dicht an "
    "Dir vorbeischwimmt." : 0,
    "!echo $Der() streift im Vorbeischwimmen Deine Hand." : 0,      
    "!echo $Der() schwimmt eine Weile knapp unter der "
    "Wasseroberfläche und taucht blitzschnell wieder weg." : 0,
    "!echo Eine dreieckige Rückenflosse durchschneidet "
    "unweit von Dir das Wasser.": 0
    ]));
  set_activity(5);
  
  set_share_v_items(1);
  add_v_item(([
    "name"     : "zähne",
    "gender"   : "maennlich",
    "plural"   : 1,
    "id"       : ({"zahn", "zähne", "zahnreihe", "zahnreihen", 
		   "reihe", "reihen", "natural#weapon"}),
    "long"     : "Das Maul des Haies ist mit mehreren Reihen "
		 "spitzer, mit Widerhaken versehener Zähne bestückt.",
    "look_msg" : "$Dem() graut es beim Anblick der spitzen Haizähne.",
    "far"      : #'query_far
    ]));

  add_v_item(([
    "name":"maul",
    "gender":"saechlich",
    "id":({"maul","schnauze","mund","oberkiefer","unterkiefer"}),
    "long":"Das Maul des Haies wird von seiner spitzen Schnauze "
	   "weit überragt. Ist es geöffnet, kann man die scharfen "
	   "Zahnreihen des Haies an Ober- und Unterkiefer erkennen.",
    "look_msg":"$Der() erschaudert beim Betrachten des Haimaules.",
    "far": #'query_far
    ]));
    
  add_v_item(([
    "name":"körper",
    "gender":"maennlich",
    "id":({"körper","gestalt","haikörper"}),
    "long":"Der Hai besitzt einen langgestreckten Körper, "
	   "der mit einer Vielzahl von Flossen versehen ist.",
    "look_msg":"$Der() betrachtet den Hai vom Kopf bis zum Schwanz.",
    "far": #'query_far
    ]));
  
  add_v_item(([
    "name":"kopf",
    "gender":"maennlich",
    "id":({"kopf","haikopf","vorderende"}),
    "long":"Der Haikopf wird zum Vorderende hin zunehmend schmaler "
	   "und endet in einer, den Unterkiefer weit überragenden, "
	   "spitzen Schnauze mit den Nasenlöchern an beiden Seiten. "
	   "Direkt hinter dem Schädel beginnt die Kiemenregion, "
	   "erkennbar an den länglichen Oeffnungen an der Seite "
	   "des Körpers.",
    "look_msg":"$Der() schaut sich den Kopf des Haies an.",
    "far": #'query_far
    ]));
	
  add_v_item(([
    "name":"schwanz",
    "gender":"maennlich",
    "id":({"schwanz","hinterende"}),
    "long":"Der schmale Schwanz des Haies reicht vom Ansatz "
	   "der Analflosse bis zur Schwanzflosse. "
	   "Auf der Oberseite des Schwanzes sitzt die hintere "
	   "Rückenflosse.",
    "look_msg":"$Der() mustert das Hinterende des Haies.",
    "far": #'query_far
    ]));
  
  add_v_item(([
    "name":"rücken",
    "gender":"maennlich",
    "id":({"rücken","oberseite","dorsalseite"}),
    "long":"Auf dem stärker pigmentierten Rücken des Haies befinden "
	   "sich die beiden Rückenflossen.",
    "look_msg":"$Der() schaut auf den Hai herunter.",
    "far": #'query_far
    ]));
    
  add_v_item(([
    "name":"bauch",
    "gender":"maennlich",
    "id":({"bauch","unterseite"}),
    "long":"Der cremeweiße Bereich des Bauches zeichnet sich scharf "
	   "gegen den dunklen Rücken ab und erstreckt sich von der "
	   "Analflosse bis zum Maul.",
    "look_msg":"$Der() versucht, einen Blick auf die Unterseite "
	       "des Haies zu werfen.",
    "far":#'query_far
    ]));
    
  add_v_item(([
    "name":"haut",
    "gender":"weiblich",
    "id":({"haut","oberfläche","farbe","hautfarbe","natural#armour"}),
    "long":"Die Haut des Haies ist am Rücken bläulich grau gefärbt, "
	   "der weiße Bauch dagegen setzt sich in einer scharfen Linie "
	   "vom dunklen Rücken ab.",
    "look_msg":"$Der() betrachtet den Haikörper von allen "
	       "möglichen Seiten.",
    "far":#'query_far
    ]));
    
  add_v_item(([
    "name":"flossen",
    "gender":"weiblich",
    "plural":1,
    "id":({"flossen","flosse","haiflosse","haiflossen",
	   "fischflosse","fischflossen"}),
    "long":"Der Hai besitzt vielerlei Flossen, so zum Beispiel "
	   "die Analflosse an der Unterseite, die seitlich "
	   "abstehenden Brustflossen, die beiden Rückenflossen "
	   "und die Schwanzflosse.",
    "look_msg":"$Der() sucht den Haikörper nach Flossen ab.",
    "far":#'query_far
    ]));
    
  add_v_item(([
    "name":"rückenflossen",
    "gender":"weiblich",
    "plural":1,
    "id":({"rückenflossen","rückenflosse","dorsalflosse",
	   "dorsalflossen"}),
    "long":"Die beiden Rückenflossen - eine größere vordere "
	   "und eine kleinere hintere - sitzen nacheinander auf "
	   "dem Rücken des Haies.",
    "look_msg":"$Der() lässt $seinen(([name:blick,gender:maennlich]),"
	       "0,OBJ_TP) über den Rücken des Haies wandern.",
    "far":#'query_far
    ]));
    
  add_v_item(([
    "name":"brustflossen",
    "gender":"weiblich",
    "plural":1,
    "id":({"brustflossen","brustflosse"}),
    "long":"Die länglichen, seitlich abstehenden Brustflossen setzen "
	   "direkt hinter der Kiemenregion an.",
    "look_msg":"$Der() beobachtet die Bewegungen der Brustflossen.",
    "far":#'query_far
    ]));
    
  add_v_item(([
    "name":"schwanzflosse",
    "gender":"weiblich",
    "id":({"schwanzflosse"}),
    "long":"Die kräftige Schwanzflosse ermöglicht dem Hai durch "
	   "seitliche Bewegungen ein schnelles Schwimmen.",
    "look_msg":"$Der() begutachtet die Schwanzflosse des Haies.",
    "far":#'query_far
    ]));
    
  add_v_item(([
    "name":"analflosse", 
    "gender":"weiblich",
    "id":({"analflosse"}),  
    "long":"Die kleine Analflosse befindet sich am Ansatz "
	   "des Schwanzes auf der Bauchseite des Haikörpers.",
    "look_msg":"$Der() müht sich ab, um einen Blick auf die "
	       "Unterseite des Haies werfen zu können.",
    "far":#'query_far
    ]));                  
    
  add_v_item(([
    "name":"augen",
    "gender":"saechlich",
    "plural":1,
    "id":({"augen","auge"}),
    "long":"Die schwarzen Augen des Haies befinden sich seitlich "
	   "am Kopf.",
    "look_msg":"$Der() versucht, dem Hai in die Augen zu schauen.",
    "far":#'query_far
    ]));
    
  add_v_item(([
    "name":"kiemenöffnungen",
    "gender":"weiblich",
    "plural":1,
    "id":({"kiemen","kieme","kiemenbereich","kiemenöffnungen",
	   "kiemenoeffnung","schlitze","öffnungen","öffnung"}),
    "long":"Hinter dem Kopf, seitlich am Haikörper fallen bei "
	   "genauerem Hinsehen die lamellenartig angeordneten, "
	   "schlitzförmigen Kiemenöffnungen auf.",
    "look_msg":"$Der() schaut sich den Hai genauer an.",
    "far":#'query_far
    ]));
			 
  add_v_item(([
    "name":"nasenlöcher",
    "gender":"saechlich",
    "plural":1,
    "id":({"nasenlöcher","nasenloch","loch","löcher"}), 
    "long":"Seitlich an der Schnauzenspitze kann man jeweils ein "
	   "Nasenloch erkennen.",
    "look_msg":"$Der() versucht, mit $seinem(([name:blick,gender:maennlich]),"
	       "0,OBJ_TP) die Schnauzenspitze des Haies zu verfolgen.",
    "far":#'query_far
    ]));                       
  set_share_v_items(0);
  set_erf_tod_message(
    ([
	AH_ERF_TOD: "Du wurdest von $dem(OBJ_TO) verschlungen.",
	AH_ERF_TOD_OTHER: "$Der(OBJ_TP) wurde von $dem(OBJ_TO) verschluckt.",
	AH_ERF_RETTUNG: "$Der(OBJ_TP) wurde gerade noch rechtzeitig aus dem "
	    "sich schließenden Maul eines Hais gezogen."
    ]));
}

