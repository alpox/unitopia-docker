// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/room/death/death_room_massenmoerder.c
// Description: Der Todesraum fuer Massenmoerder.

inherit "/i/room/death_room";

#include <level.h>
#include <description.h>

object death;

void get_tod ()
{
   if (!death) 
      death = touch("/room/death/death_massenmoerder");
   if (!present (death))
      death->move(this_object());
}

void create()
{
   ::create ();
   set_nicht_tot_meldung ("Der Tod sagt: WAS TUST DU HIER? "
                           "DEINE ZEIT IST NOCH NICHT REIF!\n\n");
   set_ende_meldung ("Der Tod sagt: DU KANNST DEM TOD NICHT ENTRINNEN!\n");
   set_short("Arbeitszimmer des Todes");
   set_long(
      "Ein dunkler Raum, erleuchtet von dunklem Licht, das sich der Dunkelheit "
      "nicht so sehr zu widersetzen scheint, indem es leuchtet, als dass es " 
      "der dunkelste Punkt in einer weniger dunklen Umgebung ist. "
      "Im seltsamen Licht siehst du einen zentral aufgestellen Schreibtisch, "
      "der mit Diagrammen und Büchern bedeckt ist. "
      "Die Wände verschwinden hinter Regalen, die gefüllt sind mit in Leder "
      "gebundenen, dunklen Wälzern, von denen geheimnisvolle Runen leuchten.");
   add_v_item(([
      "name": "lars",
      "gender": "maennlich",
      "personal": 1,
      "living":1,
      "invis": T_OR(T_LESSER("sequence", 55),T_GE("sequence",75)),
      "long":"Lars Pensjö, eine historische Gestalt aus der Gründerzeit der "
	     "Muds. Warum er hier noch rumgeistert bleibt wohl sein Geheimnis.",      ]));
}

int do_sequence (object wer, int nr)
{
#define WRITE(x) tell_object(wer, x)
#define SPEAK(x) tell_object(wer, "Der Tod sagt: " + x)

    int female;
    if (nr > 125) return -1; // sollte an sich nie passieren.
    female =  wer->query_gender()=="weiblich";
    switch(nr) {
        case 10:
	    SPEAK("DIE ZEIT IST REIF!\n");
	    WRITE("\nDer Tod hebt einen Arm und macht eine winkende Bewegung.\n"
		  "Du bist recht sicher, dass, falls Du noch am Leben wärest, Du vor Angst auf\n"
		  "der Stelle gestorben wärest. Seltsamerweise fühlst Du gar nichts derartiges.\n"
		  "Lediglich eine leichte Neugier.\n\n");
	    return 0;
	case 15:
	    SPEAK("KEINE DRÜSEN, DAS IST DER GRUND.\n");
	    WRITE("\nDer Tod scheint ein wenig zu lächeln. Andererseits ist das ein bisschen schwer\n" 
		  "zu sagen. Das könnte ebensogut sein normaler Gesichtsausdruck sein...\n\n");
	    return 0;
	case 20:
	    SPEAK("OHNE DRÜSEN FÜHLST DU NICHTS, ÜBERHAUPT NICHTS.\n");
	    WRITE("\nNun, er scheint Recht zu haben. Statt vor Angst verrückt zu werden, beginnst\n"
	          "Du dich zu langweilen. Du wünschst Dir, dass sehr bald etwas passieren möge.\n\n");
	    return 0;
	case 27:
	    SPEAK("KOMM ZU MIR, ICH MUSS IN DEINER SEELE LESEN.\n");
	    WRITE("\nDer Tod kommt näher, streckt seine Knochenhand direkt in Deinen Körper\n"
	          "und greift darin nach etwas! Du fühlst ein seltsames Reißen in Deinem\n"
		  "Inneren, als Deine Seele für die Untersuchung entfernt wird... Plötzlich\n"
		  "sammelt der Tod Deine körperlose Existenz mit einer umfassenden,\n"
		  "wischenden Bewegung seiner Skeletthand und legt Dich in eine kleine\n"
		  "Glasschale, die er in seine rechte Augenhöhle einsetzt! Du fühlst ein\n"
		  "komisches, blaues Licht aus seinem augenlosen Schädel in Dich eindringen,\n"
		  "als er sich über das Diagramm beugt.\n\n");
	    return 0;
	case 40:
	    SPEAK("DEINER SÜNDEN SIND SO VIELE WIE SANDKÖRNER IN DER WÜSTE.\n"
		  "VIELLEICHT BIST DU EIN SCHLIMMERES WESEN ALS ICH! HAHAHAHAHAHA!\n\n");
	    return 0;
	case 47:
	    SPEAK("NUN, ICH GLAUBE, DAS WAR'S ERSTMAL FÜR DICH. WIR SEHEN UNS AM\n"
		  "JÜNGSTEN TAG! HAHAHAHAHAHAHAHAHAHAHAHAHAHAHAHAHAHAHAHAH!\n\n");
	    WRITE("Der Tod entfernt die Schale aus seinem Auge und steht auf. Plötzlich dreht er\n"
		  "sich um und verlässt den Raum durch die nächste Wand. Dabei hält er Dich im-\n"
		  "mer noch in seiner Hand. Er geht schnell durch einen dunklen, sich windenden\n"
		  "Korridor, eine Wendeltreppe herunter zum tiefsten Raum in seinem Keller.\n"
		  "Schließlich stoppt er vor einer Tür, auf der in schwarzen Buchstaben das\n"
		  "Wort 'EWIGKEIT' steht. Auf dem Boden siehst Du eine kleine Luke, die der Tod\n"
		  "öffnet und aus der Du dann das Murmeln von vielen Millionen Seelen hörst.\n"
		  "Langsam bewegt er Dich auf das gähnende Loch zu.\n\n");
	    return 0;
	case 55:
	    WRITE("Lars erscheint durch einen Riss im Vorhang der Welten.\n");
	    return 0;
	case 58:
	    WRITE("Lars lächelt Dich an.\n");
	    return 0;
	case 61:
	    WRITE("Lars flüstert dem Tod etwas zu.\n");
	    return 0;
	case 63:
	    SPEAK("WAS? KOMMT GAR NICHT IN FRAGE! DAS WEISST DU DOCH, LARS!\n\n");
	    return 0;
	case 66:
	    WRITE("Lars seufzt tief.\n"); 
	    return 0;
	case 69:
	    WRITE("Lars flüstert dem Tod etwas zu.\n");
	    return 0;
	case 72:
	    SPEAK("REINKARNATION? FÜR DIESE"+(female?"":"N")+" HIER? "
	          "DAS IST "+(female?"SIE":"ER")+" NICHT WERT!\n"
	          "BITTE SEI VERNÜNFTIG, LARS!\n\n");
	    return 0;
	case 75:
	    WRITE("Lars sagt: Ja, da hast Du eigentlich recht.\n");
	    WRITE("Lars seufzt.\n");
	    WRITE("Lars verschwindet durch einen Riss im Vorhang der Welten.\n");
	    return 0;
	case 82:
	    WRITE("Der Tod schaut Dich an mit einem Blick, der etwas ausdrückt, das Ekel\n"
		  "sein muss, auch wenn es schwer zu sagen ist. Sein Gesicht ist nicht sehr gut\n"
		  "geeignet, Gefühle auszudrücken, aber diesmal bist du einigermaßen sicher.\n\n");
	    return 0;
	case 86:
	    SPEAK("DANN MAL HURTIG ANS WERK.\n");
	    WRITE("Der Tod schwingt prüfend seine Sense.\n");
	    return 0;
	case 90:
	    WRITE("Der Tod schwingt seine Sense mitten durch dich hindurch.\n"
	          "Du spürst einen stechenden Schmerz, und doch spürst du nichts.\n"
	          "Du merkst nur, dass etwas von dir fehlt... dein Wissen, deine Erfahrung.\n");
	    return 0;
	case 94:
	    WRITE("Der Tod schwingt wieder seine Sense, und wieder mitten durch dich hindurch.\n"
	          "Du spürst wieder einen stechenden Schmerz, und obwohl du nichts spürst,\n"
	          "fühlt es sich diesmal ganz anders an.\n"
	          "Es fühlt sich wieder an, als ob Dir etwas wichtiges verloren gegangen ist.\n"
	          "So als ob Ausdauer und Stärke dahinschwinden.\n");
	    return 0;
	case 98:
	    WRITE("Der Tod schwingt erneut seine Sense, diesmal mit einer völlig merkwürdigen\n"
	          "Bewegung, und wieder mitten durch dich hindurch.\n"
	          "Du kommst dir völlig ungeschickt vor... und dumm wie ein Kartoffelsack.\n"
	          "Du schämst dich.\n");
	    return 0;

	case 102:
	    WRITE("Lars erscheint durch einen Riss im Vorhang der Welten.\n");
	    return 0;
	case 106:
	    WRITE("Lars seufzt tief.\n");
	    return 0;
	case 108:
	    WRITE("Lars flüstert dem Tod etwas zu.\n");
	    return 0;
	case 112:
	    SPEAK("EWIGEN FRIEDEN? DU HAST RECHT, DEN HAT DIESE"
	        +(female?"":"N")+" HIER NICHT VERDIENT!\n");
	    return 0;
	case 116:
	    WRITE("Lars schmunzelt.\n");
	    WRITE("Lars verschwindet durch einen Riss im Vorhang der Welten.\n");
	    return 0;
	case 120:
	    WRITE("Der Tod schaut Dich an mit einem Blick, der etwas ausdrückt, das Schadenfreude\n"
		  "sein muss, auch wenn es schwer zu sagen ist. Sein Gesicht ist nicht sehr gut\n"
		  "geeignet, Gefühle auszudrücken, aber diesmal bist du einigermaßen sicher.\n\n");
	    return 0;
	case 124:
	       SPEAK("SO LEBE DEIN SCHRECKLICHES LEBEN WEITER. BIS ZUM NÄCHSTEN MAL!\n\n");
	       WRITE("Plötzlich schleudert der Tod Dich in die Luft, Du hast eine seltsame Empfin-\n"
		     "dung als Du durch die Wände des Gebäudes, hinaus in die frische Luft, durch\n"
		     "einige andere Wände und ziemlich überraschte Pferde fliegst und schließlich\n"
		     "in einem anderen Gebäude landest. Es wirkt vage vertraut...\n");
	       return -1;
        default: return 0;
    }
}

