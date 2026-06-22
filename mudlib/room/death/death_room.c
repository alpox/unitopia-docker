// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/room/death/death_room.c
// Description: Der Todesraum.

inherit "/i/room/death_room";

#include <level.h>
#include <description.h>
#include <message.h>
#include <move.h>

object death;

void get_tod ()
{
   if (!death) 
      death = touch("/room/death/death");
   if (!present (death))
      death->move(this_object());
}

void reset()
{
#ifdef UNItopia
   object rune;
   if(!present("mage#runenstein"))
   {
      rune = clone_object("/z/Gilden/Magiergilde/obj/rune");
      rune->set_runenstein("xen");
      rune->move(this_object());
   }
#endif
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
      "Im seltsamen Licht siehst du einen zentral aufgestellten Schreibtisch, "
      "der mit Diagrammen und Büchern bedeckt ist. "
      "Die Wände verschwinden hinter Regalen, die gefüllt sind mit in Leder "
      "gebundenen, dunklen Wälzern, von denen geheimnisvolle Runen leuchten.");
   add_v_item(([
      "name": "lars",
      "gender": "maennlich",
      "personal": 1,
      "living":1,
      "invis": T_OR(T_LESSER("sequence", 55),T_GE("sequence",75)),
      "long": "Lars Pensjö, eine historische Gestalt aus der Gründerzeit der "
	     "Muds. Warum er hier noch rumgeistert bleibt wohl sein Geheimnis.",
      ]));
}

int do_sequence (object wer, int nr)
{
#define WRITE(x) send_message_to(wer, MT_LOOK|MT_NOTIFY, MA_UNKNOWN, x)
#define EWRITE(x) send_message_to(wer, MT_LOOK, MA_EMOTE, x)
#define MWRITE(x) send_message_to(wer, MT_LOOK, MA_MOVE, x)
#define SPEAK(x) send_message_to(wer, MT_NOISE, MA_COMM, wrap_say("Der Tod sagt:", x))

    int align, female;
    if (nr > 85) return -1; // sollte an sich nie passieren.
    female =  wer->query_gender()=="weiblich";
    switch(nr) {
        case 10:
	    SPEAK("DIE ZEIT IST REIF!");
	    WRITE("\nDer Tod hebt einen Arm und macht eine winkende Bewegung.\n"
		  "Du bist recht sicher, dass, falls Du noch am Leben wärest, Du vor Angst auf\n"
		  "der Stelle gestorben wärest. Seltsamerweise fühlst Du gar nichts derartiges.\n"
		  "Lediglich eine leichte Neugier.\n\n");
	    return 0;
	case 15:
	    SPEAK("KEINE DRÜSEN, DAS IST DER GRUND.");
	    WRITE("\nDer Tod scheint ein wenig zu lächeln. Andererseits ist das ein bisschen schwer\n" 
		  "zu sagen. Das könnte ebensogut sein normaler Gesichtsausdruck sein...\n\n");
	    return 0;
	case 20:
	    SPEAK("OHNE DRÜSEN FÜHLST DU NICHTS, ÜBERHAUPT NICHTS.");
	    WRITE("\nNun, er scheint Recht zu haben. Statt vor Angst verrückt zu werden, beginnst\n"
	          "Du dich zu langweilen. Du wünschst Dir, dass sehr bald etwas passieren möge.\n\n");
	    return 0;
	case 27:
	    SPEAK("KOMM ZU MIR, ICH MUSS IN DEINER SEELE LESEN.");
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
	    align = wer->query_align();
	    if (align < -1000)
		SPEAK("DEINER SÜNDEN SIND SO VIELE WIE SANDKÖRNER IN DER WÜSTE. "
		      "VIELLEICHT BIST DU EIN SCHLIMMERES WESEN ALS ICH! HAHAHAHAHAHA!");
	    else if (align < -500)
		SPEAK("OH WAS FÜR EINE VERACHTENSWERTE WANZE WIR HIER HABEN. STIEHLT "
		      "ZWEIFELLOS BABIES IHRE LOLLIES UND VERPRÜGELT ALTE DAMEN. NUN KÖNNEN "
		      "SIE AUF DEINEM GRABE TANZEN. HAHAHAHAHA!");
	    else if (align < -200)
		SPEAK("HAT DIR SCHON MAL JEMAND WAS VON REUE UND SÜHNE ERZÄHLT? NEIN? "
		      "DAS DACHTE ICH MIR. MAN WIRD ES DIR JEDENFALLS JETZT ERZÄHLEN, "
		      "FÜR ALLE EWIGKEIT! HAHAHAHAHA!");
	    else if (align < 0)
		SPEAK("SCHÄME DICH, STERBLICHE"+(female?"":"R")+"! "
		      "STEHLEN UND MORDEN, IST DAS ALLES, "
		      "WORAN DU DENKEN KANNST? NUN, JETZT WIRD DIR ZEIT GEGEBEN, DEINE "
		      "TATEN ZU BEREUEN. FÜR IMMER! HAHAHAHA!");
	    else if (align == 0)
		SPEAK("WAS FÜR EINE"+(female?"":"N")+" HABEN WIR DENN HIER? HAST DICH NIE "
		      "ENTSCHLIESSEN KÖNNEN IN DEINEM GANZEN LEBEN, WIE? TJA, MACH DIR KEINE SORGEN. "
		      "DU WIRST ES NUN AUCH NICHT MÜSSEN! HAHAHAHAHA!");
	    else if (align < 200)
		SPEAK("OH, WAS FÜR EINEN NETTEN MENSCHEN HABEN WIR HIER. IMMER AUF "
		      "DER BREITEN STRASSE GEWANDELT, NICHT? NUN, JETZT WIRST DU NIE "
		      "MEHR ERFAHREN, WIE DIE ANDERE SEITE AUSSIEHT! HAHAHAHAHA!");
	    else if (align < 500)
		SPEAK("HAST IN DEINEM GANZEN LEBEN KEIN SCHMUTZIGES WORT IN DEN MUND "
		      "GENOMMEN, WAS? TJA, JETZT IST ES ZU SPÄT, DEINEN ENTSCHLUSS "
		      "NOCH ZU ÄNDERN. HAHAHA! NEIN, "+
		      (female?"MISS LUCKY LADY":"MISTER NICE GUY")+", DU BLEIBST WAS "
		      "DU IMMER WARST! HAHAHAHA!");
	    else if (align < 1000)
		SPEAK("ICH HABE GEHÖRT, DASS SIE IM HIMMEL NOCH ERZENGEL SUCHEN. VIEL"
		      "LEICHT SOLLTEST DU DICH UM DEN JOB BEWERBEN? ICH HOFFE, DU KANNST HARFE "
		      "SPIELEN, SONST WERDEN SIE "+
		      (female?"EINER ANDEREN":"EINEM ANDEREN")+" DEN JOB GEBEN! HAHAHA!");
	    else
		SPEAK("VERSUCHST WOHL, GOTT DIE STELLE WEGZUSCHNAPPEN, WAS? HAHAHA! "
		      "BEVOR DU IRGENDWAS UNTERSCHREIBST, MÖCHTE ICH DIR EIN WENIG ÜBER DEN JOB ER"
		      "ZÄHLEN: VIEL ARBEIT, KEIN DANK, NIEMALS URLAUB. GLAUB MIR, DU WILLST "
		      "ES NICHT!");
	    WRITE("\n");
	    return 0;
	case 47:
	    SPEAK("NUN, ICH GLAUBE, DAS WAR'S ERSTMAL FÜR DICH. WIR SEHEN UNS AM "
		  "JÜNGSTEN TAG! HAHAHAHAHAHAHAHAHAHAHAHAHAHAHAHAHAHAHAHAH!");
	    WRITE("\nDer Tod entfernt die Schale aus seinem Auge und steht auf. Plötzlich dreht er\n"
		  "sich um und verlässt den Raum durch die nächste Wand. Dabei hält er Dich im-\n"
		  "mer noch in seiner Hand. Er geht schnell durch einen dunklen, sich windenden\n"
		  "Korridor, eine Wendeltreppe herunter zum tiefsten Raum in seinem Keller.\n"
		  "Schließlich stoppt er vor einer Tür, auf der in schwarzen Buchstaben das\n"
		  "Wort 'EWIGKEIT' steht. Auf dem Boden siehst Du eine kleine Luke, die der Tod\n"
		  "öffnet und aus der Du dann das Murmeln von vielen Millionen Seelen hörst.\n"
		  "Langsam bewegt er Dich auf das gähnende Loch zu.\n\n");
	    return 0;
	case 55:
	    MWRITE("Lars erscheint durch einen Riss im Vorhang der Welten.\n");
	    return 0;
	case 58:
	    EWRITE("Lars lächelt Dich an.\n");
	    return 0;
	case 61:
	    WRITE("Lars flüstert dem Tod etwas zu.\n");
	    return 0;
	case 63:
	    SPEAK("WAS? KOMMT GAR NICHT IN FRAGE! DAS WEISST DU DOCH, LARS!");
	    WRITE("\n");
	    return 0;
	case 66:
	    EWRITE("Lars seufzt tief.\n"); 
	    return 0;
	case 69:
	    WRITE("Lars flüstert dem Tod etwas zu.\n");
	    return 0;
	case 72:
	    SPEAK("REINKARNATION? FÜR DIESE"+(female?"":"N")+" HIER? "
	          "DAS IST "+choose_by_gender(wer, ({"ES","ER","SIE"}))+
		  " NICHT WERT! BITTE SEI VERNÜNFTIG, LARS!");
	    WRITE("\n");
	    return 0;
	case 75:
	    WRITE("Lars verzieht sich schmollend in eine Ecke.\n");
	    MWRITE("Lars verschwindet durch einen Riss im Vorhang der Welten.\n");
	    WRITE("Der Tod schaut Dich an mit einem Blick, der etwas ausdrückt, das Ekel\n"
		  "sein muss, auch wenn es schwer zu sagen ist. Sein Gesicht ist nicht sehr gut\n"
		  "geeignet, Gefühle auszudrücken, aber diesmal bist du einigermaßen sicher.\n\n");
	    return 0;
	case 85:
	       SPEAK("OK! ICH KANN WARTEN.\n"
	             "EINES TAGES WIRST DU AUF JEDEN FALL MIR GEHÖREN!");
	       WRITE("\nPlötzlich schleudert der Tod Dich in die Luft, Du hast eine seltsame Empfin-\n"
		     "dung als Du durch die Wände des Gebäudes, hinaus in die frische Luft, durch\n"
		     "einige andere Wände und ziemlich überraschte Pferde fliegst und schließlich\n"
		     "in einem anderen Gebäude landest. Es wirkt vage vertraut...\n");
	       return -1;
        default: return 0;
    }
}

<int|string> let_not_out(mapping mv_infos)
{
   int res;
   object wer,move_objekt = mv_infos[MOVE_OBJECT];
   if(!(res = ::let_not_out(mv_infos)) &&
      move_objekt->query_runenstein() == "xen" &&
      move_objekt->id("mage#runenstein") &&
      (wer = this_interactive() || this_player()) &&
      guestp(wer) || newbiep(wer) || testplayerp(wer))
   {
      SPEAK("DU LÄSST SCHÖN DIE FINGER DAVON!\n");
      return 1; 
   }
   return res;
}  
