// This file was part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /room/death/death_room_sand.c
// Description: Der Todesraum mit Sandmaennchen.
// Authors:     Wluut, Mirza
// Date:        04. April 1995

inherit "/i/room/death_room";

#include <level.h>
#include <message.h>
#include <move.h>

object death;

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

void get_tod ()
{
   if(!death) death = touch("/room/death/death_sand");
   if(!present (death))
      death->move(this_object());
}

void create()
{
   ::create ();
   set_nicht_tot_meldung (
       wrap("Das Sandmännchen hoppst herbei und fragt Dich vorwurfsvoll:\n"+
            "Willst Du mich etwa besuchen...? Tut aber echt leid, ich habe "+
            "gerade keine Zeit...\n")+
       "Das Sandmännchen winkt liebevoll zum Abschied...\n");
   set_ende_meldung(
       "Das Sandmännchen sagt: Du kannst mir nicht entkommen, Du Angsthase!");
   set_short("Arbeitszimmer des Todes");
   set_long(
      "Ein dunkler Raum, erleuchtet von dunklem Licht, das sich der Dunkelheit "      "nicht so sehr zu widersetzen scheint, indem es leuchtet, als dass es " 
      "der dunkelste Punkt in einer weniger dunklen Umgebung ist. "
      "Im seltsamen Licht siehst du einen zentral aufgestellen Schreibtisch, "
      "der mit Diagrammen und Büchern bedeckt ist. "
      "Die Wände verschwinden hinter Regalen, die gefüllt sind mit in Leder "
      "gebundenen, dunklen Wälzern, von denen geheimnisvolle Runen leuchten.");//   touch("/room/death/death")->move(this_object());
}


int do_sequence (object wer, int nr)
{
#define WRITE(x)  send_message_to(wer, MT_LOOK|MT_NOTIFY, MA_UNKNOWN, wrap(x))
#define MWRITE(x) send_message_to(wer, MT_LOOK, MA_MOVE, wrap(x))
#define EWRITE(x) send_message_to(wer, MT_LOOK, MA_EMOTE, wrap(x))
#define SAND(x)   send_message_to(wer, MT_NOISE, MA_COMM, wrap_say("Das Sandmaennchen sagt:",x))
#define LARS(x)   send_message_to(wer, MT_NOISE, MA_COMM, wrap_say("Lars sagt:",x))

    if (nr > 75) return -1;

    switch (nr) {
        case 5:
               MWRITE("Das Sandmännchen springt herbei und macht eine anmutige Verbeugung.");
               return 0;

        case 7:
               SAND("Hallo, ich bin die Vertretung vom Tod, der ist im Urlaub, musste sich mal "+
                    "ausruhen, es ist ja auch zu anstrengend, immer all diese Leute zu meucheln...");
               EWRITE("Das Sandmännchen seufzt tief.");
               return 0;

        case 10:
               SAND("Na, dann wollen wir doch mal anfangen, nicht wahr?");
               return 0;

        case 12:
               WRITE("Das Sandmännchen beginnt seine magische Rassel zu schwingen und mit ulkigen "+
                     "Verrenkungen um Dich herumzutanzen... Als es sich plötzlich unterbricht!");
               return 0;

        case 14:
               SAND("Mist! Falsche Zeremonie, tut mir leid, aber wir müssen noch mal von vorne "+
                    "anfangen.");
               WRITE("Das Sandmännchen schaut sich unsicher im Raum um...");
               return 0;

        case 18:
               WRITE("Das Sandmännchen nimmt eine scharfe Sense aus einem Regal und seufzt zufrieden.");
               SAND("Das scheint das richtige zu sein, nicht wahr?");
               return 0;

        case 20:
               WRITE("Das Sandmännchen beginnt die Sense zu schwingen und in kleinen Schritten auf "+
                     "Dich zuzugehen.");
               WRITE("Das Sandmännchen fragt unsicher: mache ich das so richtig?");
               return 0;

        case 22:
               WRITE("Das Sandmännchen ist schon ganz nahe, mit einem Pfeifen saust die Sense hin und "+
                     "her, Dir wird sehr mulmig in den Knien.");
               return 0;

        case 25:
               SAND("Wie war doch gleich der Text...? Achja: KEINE DRÜSEN, DAS IST DER GRUND!");
               return 0;               

        case 30:
               MWRITE("Lars erscheint durch einen Riss im Vorhang der Welten.");
               return 0;

        case 35:
               EWRITE("Lars lächelt Dich an!");
               return 0;

        case 40:
               WRITE("Lars schaut das Sandmännchen an...");
               return 0;

        case 45:
               LARS("Du machst das so gut, dass ich beruhigt wieder gehen kann...");
               WRITE("Lars winkt noch einmal freundlich und verschwindet durch den Riss im Vorhang der Welten.");
               return 0;

        case 50:
               SAND("Immer diese Störungen... Jetzt bin ich völlig aus der Rolle...");
               EWRITE("Das Sandmännchen kratzt sich hilflos am Kopf...");
               return 0;

        case 55:
               SAND("Ich kriege das nicht mehr auf die Reihe, ich bin völlig raus...");
               WRITE("Das Sandmännchen rennt wütend auf und ab, leise hörst Du es murmeln:\n"+
                     "Ach, wenn doch Rotkäppchen da wäre, dann hätte ich besseres zu tun...");
               return 0;

        case 60:
               EWRITE("Das Sandmännchen stampft wütend mit dem Fuß auf und funkelt Dich böse an.");
               SAND("Mir reicht's jetzt!");
               return 0;

        case 63:
               SAND("Hör zu, wenn Du niemanden etwas davon erzählst, kannst Du gehen.");
               SAND("Ist das ok?");
               return 0;

        case 67:
               WRITE("Das Sandmännchen schaut Dich fragend an, als es bemerkt, dass Du wegen Deines "+
                     "nicht-materiellen Zustandes ja gar nicht antworten kannst...");
               EWRITE("Das Sandmännchen seufzt und schaut nachdenklich drein.");
               return 0;

        case 75:
               SAND("Verdammt, dann geh halt ohne Versprechen!");
               WRITE("Das Sandmännchen gibt Dir einen derben Stoß, Du taumelst, fällst "+
                     "durch die Wände des Gebäudes, hinaus in die frische Luft, durch "+
                     "einige andere Wände und ziemlich überraschte Pferde fliegst und schließlich "+
                     "in einem anderen Gebäude landest. Es wirkt vage vertraut...");
               return -1;
        default:
               return 0;
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
      SAND("DU LÄSST SCHÖN DIE FINGER DAVON!\n");
      return 1; 
   }
   return res;
}  
