// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /room/kirche/balkon.c
// Description: Der beruehmte Balkon der Kathedrale
// Author:

inherit "/i/room";

#include <add_hp.h>
#include <move.h>
#include <level.h>
#include <message.h>
#ifdef UNItopia
#include "/d/Vaniorh/sys/vaniorh_exports.h"
#endif

void create()
{
   set_short("Engelsschanze");
   set_long(
   "Ein kleiner, schmaler Vorsprung mit einem fein verzierten, gusseisernen "
   "Geländer davor bildet hier einen winzigen Balkon, auf dem man gerade mal "
   "zu zweit bequem stehen kann. Links und rechts befinden sich zwei Engels"
   "figuren, die kunstvoll in einer Tadmorer Bildhauerwerkstatt vor Urzeiten "
   "gefertigt wurden und seitdem hier diesen im Volksmund 'Engelsschanze' "
   "genannten Balkon bewachen. Von hier hat man einen wunderschönen Ausblick "
   "über die Ile de la Cite, die von den beiden Armen des Dijala "
   "umschlungen wird und die wichtigsten Gebäude der Stadt beherbergt. "
   "Mittels eines kleinen Durchgangs kann man den windigen Balkon wieder in "
   "Richtung Osten verlassen.");
   set_exits(({"verdombde"}), ({"osten"}));

   add_v_item(([
      "name": "geländer",
      "gender": "saechlich",
      "adjektiv": ({"verziert","gusseisern"}),
      "long": "Das Muster des Geländers zeigt eine Engelsschar, die "
	      "auf einer Wolke herumtollt.",
      "feel": "Obwohl das Geländer offensichtlich schon sehr alt ist, "
              "fühlt es sich sehr stabil an und ist fest an der Wand "
	      "verankert. Du musst Dir also keine Gedanken machen, da "
	      "runterzufallen, wenn Du Dich daran festhältst oder "
	      "dagegenlehnst.",
      "noise":"Das Geländer ist nicht sonderlich mitteilsam. Alles, "
              "was Du hörst, ist das Pfeifen des Windes, der um "
	      "diesen hohen Turm weht.",
      "smell":"Selbst wenn daran irgendetwas zu riechen wäre, würde "
              "der Geruch sehr schnell von dem starken Wind hier oben "
	      "verweht.",
	]));

   add_v_item(([
      "name": "engelsfiguren",
      "gender": "weiblich",
      "plural": 1,
      "id": ({ "engel","engelsfiguren", "figuren", "engelsfigur", "figur" }),
      "long": "Die linke Figur scheint Dir direkt in die Augen zu sehen "
	      "und trägt die Aufschrift: \"Rufe\". Die andere lächelt "
	      "Dich auffordernd an und ist mit dem seltsamen Wort: "
	      "\"Lyrion\" versehen."]));

   add_v_item(([
      "name":     "durchgang",
      "gender":   "maennlich",
      "adjektiv": "klein",
      "id":     ({"durchgang","gang"}),
      "long":     "Der Durchgang führt nach Osten ins innere des Turms der "
                  "Kathedrale." ]));      

    add_v_item(([
	"name":	  "dijala",
	"gender": "maennlich",
	"id":      ({"dijala", "fluss", "strom"}),
	"far":    "Zurück in die Kathedrale, die Treppe runter, auf den "
	          "Kartukultininurta-Platz und dann wahlweise nach Norden "
		  "oder Süden, dort findest Du den Dijala in der Nähe "
		  "der Brücken.",
	"long":	  "Du siehst das breite Band des Dijala, das sich von Westen "
	          "her durch die Stadt schlängelt. Etwa auf Höhe der "
		  "Bibliothek spaltet der Fluss sich in zwei Arme auf, "
		  "die links und rechts um die Ile de la Cite herum und "
		  "damit auch aus Deinem Blickfeld fließen. Soweit du "
		  "weißt, treffen sich die Arme hinter der Kathedrale "
		  "wieder und fließen dann nach Osten ins Meer.",
	"feel":   "Wenn Du mit dem Fluss auf Tuchfühlung gehen willst, "
	          "dann geh einfach mal auf eine der Brücken und spring "
		  "rein. Hier oben mit den Armen rumzufuchteln hilft nicht "
		  "viel.",
	"feel_msg": "", // Keine Meldung an die Umstehenden.
	"noise":  "Du bist viel zu weit weg, um irgendetwas davon zu hören. "
	          "Einzig und allein das Pfeifen des Windes dringt an Deine "
		  "Ohren.",
	"hear_msg":"$Der(OBJ_TP) spannt seine Ohren gen Dijala auf.",
	"smell":  "Du hängst Deine Nase in den Wind, aber scheinbar gibt "
	          "es gerade keine Luftverwirbelungen, die 150 Meter "
		  "entfernte Gerüche zu Dir hochtragen. Geh einfach mal "
		  "näher ran.",
	"smell_msg": "$Der(OBJ_TP) hängt seine Nase in den Wind, aber "
	          "anscheinend erfolglos.",
	]));

    add_v_item(([
	"name":   "tadmor",
	"personal": 1,
	"gender": "weiblich",
	"id":     ({"tadmor", "stadt", "aussicht"}),
	"long":   "Tief unter Dir breitet sich die Stadt Tadmor in all "
	          "ihrer Pracht aus. Von Deinem luftigen Standpunkt hier "
		  "oben kannst Du den kompletten Westteil der Stadt "
		  "überblicken und siehst Menschen wie Ameisen zwischen "
		  "den Häusern umherlaufen. Du lässt Deinen Blick über "
		  "solche Sehenswürdigkeiten wie das Theater, das Rathaus "
		  "und die Bibliothek streifen und beugst Dich sogar etwas "
		  "übers Geländer, um vielleicht einen Blick auf den "
		  "Magierturm zu erhaschen, aber dafür ist Dir leider die "
		  "Wandung des Kathedralenturms im Weg.",
	"feel":   "Hohe Aussichtspunkte sind zum Schauen zwar ganz praktisch, "
	          "aber zum Fühlen sollte man doch lieber auf armlange Nähe "
		  "rangehen. Am besten gehst Du die Treppe, die Du "
		  "hochgekommen bist, wieder runter und schaust Dich unten "
		  "in der Stadt genau um - so gewinnst Du bestimmt schnell "
		  "ein Gefühl dafür, was Tadmor ausmacht.",
	"feel_msg": "",
	"noise":  "Obwohl in Tadmor zu jeder Tages- und Nachtzeit ein "
	          "geschäftiges Treiben herrscht, dringt kaum ein Geräusch "
		  "bis zu Dir hoch. Das einzige, was Du hörst, ist das leise "
		  "Pfeifen des Windes.",
	"hear_msg": "$Der(OBJ_TP) lauscht gespannt den Geräuschen der Stadt.",
	"smell":  "Nein, nichts in Tadmor 'stinkt zum Himmel', so dass Du es "
	          "hier oben riechen könntest. Die Alchemistengilde ist mit "
		  "ihrer dazugehörigen Geruchskulisse bereits vor Jahren "
		  "nach Borsippa umgezogen.",
	"smell_msg": "$Der(OBJ_TP) versucht, den Geruch Tadmors aufzunehmen.",
	]));

}

void init()
{
   add_action("rufe", "rufe", -3);
   add_action("springfkt", "springe", -6);
}


int rufe(string str)
{
   if(!str)
   {
      notify_fail("Rufe wen?\n");
      return 0;
   }
   if(lower_case(str) != "lyrion")
   {
      notify_fail(capitalize(str)+" hört Dich hier nicht!\n");
      return 0;
   }
   say(Der(this_player(),"")+" ruft den Engel Lyrion!\n");
   write("Laut und deutlich rufst Du: LYRION!\n");

   if(!hlpp(this_player()) && !wizp(this_player()))
   {
      say("Ein gewaltiger Engel gleitet vom Himmel herab und mustert "+
	  den(this_player(),"")+".\nEr sagt: Nein, Du bist noch nicht "
	  "bereit!\nDaraufhin entschwindet er wieder in den Himmel.\n");
      write("Ein gewaltiger Engel gleitet vom Himmel herab und mustert Dich.\n"
	  "Er sagt: Nein, Du bist noch nicht "
	  "bereit!\nDaraufhin entschwindet er wieder in den Himmel.\n");
   }
   else if(present("hlp#tool",this_player()))
   {
      say(wrap(
	 "Lyrion, der gewaltigste Engel aller Zeiten gleitet elegant vom "
	 "Himmel herab und spricht zu "+dem(this_player())+":\n"
	 "Hallo, findest Du mal wieder den Weg in den Himmel nicht? "
	 "Folge mir nun!"));
      write(wrap(
	 "Lyrion, der gewaltigste Engel aller Zeiten gleitet elegant vom "
	 "Himmel herab und spricht zu Dir:\n"
	 "Hallo, findest Du mal wieder den Weg in den Himmel nicht? "
	 "Folge mir nun!\n"
	 "Du erhebst Dich unter Lyrions Anweisungen, schwingst Dich über "
	 "das Geländer und .... FÄLLST.\n\n"
	 "Bruchteile von Sekunden später erinnerst Du Dich wieder daran, "
	 "Deine Flügel zu entfalten, und Du folgst auf fast gerader Linie "
	 "dem vorauseilenden Lyrion gen Himmel."));

      this_player()->move("/room/hlp/wolke", ([MOVE_FLAGS:MOVE_MAGIC ,
	 MOVE_MSG_LEAVE:"$Der() und Lyrion erheben sich gemeinsam in die "
	 "Lüfte.\nNach einiger Zeit sind sie nur noch zwei kleine Punkte am "
	 "Himmel\nund dann sind sie ganz verschwunden",
	 MOVE_MSG_ENTER:"Lyrion gleitet eilig davon und kehrt mit $dem() zurück"
     ]));
   }
   else
   {
      say(wrap(
	 "Lyrion, der gewaltigste Engel aller Zeiten gleitet elegant vom "
	 "Himmel herab und spricht zu "+dem(this_player())+":\n"
	 "Willkommen in der Schar der Engel. Folge mir nun!"));
      write(wrap(
	 "Lyrion, der gewaltigste Engel aller Zeiten gleitet elegant vom "
	 "Himmel herab und spricht zu Dir:\n"
	 "Willkommen in der Schar der Engel. Folge mir nun!\n"
	 "Du erhebst Dich unter Lyrions Anweisungen, schwingst Dich über "
	 "das Geländer und .... FÄLLST.\n\n"
	 "Bruchteile von Sekunden später entfaltest Du Deine Flügel und "
	 "folgst himmelhochjauchzend ob des beeindruckenden Gefühls mit "
	 "leichtem Flügelschlage dem vorauseilenden Lyrion gen Himmel!!!"));

      this_player()->move("/room/hlp/wolke", ([MOVE_FLAGS:MOVE_MAGIC ,
	 MOVE_MSG_LEAVE:"$Der() und Lyrion erheben sich gemeinsam in die "
	 "Lüfte.\nNach einiger Zeit sind sie nur noch zwei kleine Punkte am "
	 "Himmel\nund dann sind sie ganz verschwunden",
	 MOVE_MSG_ENTER:"Lyrion gleitet eilig davon und kehrt mit $dem() zurück"
     ]));
   }
   return 1;
}

int springfkt(string str)
{
    if(!str || str=="")
    {
	notify_fail("Wohin willst du springen? Doch nicht etwa runter?\n");
	return 0;
    }

    str=lower_case(str);
    if(str!="runter" && str!="hinunter" && str[0..8]!="mutwillig")
    {
	notify_fail("Wohin willst du springen? Doch nicht etwa runter?\n");
	return 0;
    }

    if(str=="runter" || str=="hinunter")
    {
	send_message_to(this_player(),MT_UNKNOWN,MA_UNKNOWN,
	    wrap("Da willst du runterspringen? Na wenn du wirklich willst, "
		 "dann springe doch mutwillig."));
	return 1;
    }
    else
    {
	send_message_to(this_player(),MT_UNKNOWN,MA_UNKNOWN,
	    "Du kletterst auf das Geländer...\n");
	call_out(
	(:
	    if(!this_player() || !present(this_player(), this_object()))
		return;
	    send_message_to(this_player(),MT_UNKNOWN,MA_UNKNOWN,
		"... springst!\n");
	    if(this_player()->move(
#ifdef UNItopia
		K_PLATZ_SO, 
#else
		"/room/rathaus/treppe",
#endif
		([ MOVE_FLAGS:MOVE_NORMAL,
		MOVE_MSG_LEAVE:"$Der() klettert auf's Geländer und "
            "stürzt sich in die Tiefe.",
		MOVE_MSG_ENTER:"$Der() schlägt neben dir auf den Boden auf."]))
            ==MOVE_OK)
	    {
		this_player()->add_hp(-(this_player()->query_max_hp()*3)/4, ([
		    AH_ERF_TOD: "Du hast vergeblich auf Lyrion gehofft.",
		]));
	    }
	:), 2);
	return 1;
    }
}
