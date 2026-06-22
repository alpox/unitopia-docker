// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /room/rathaus/reinkarnation.c
// Description:
// Modified by:   Croft   (23.12.98): Inschriften ausgelagert
//                Copper  (18.10.99): Wwahlmaschine angeworfen
//                Nethar  (16.09.01): Anstatt vieler Tafeln nur noch 2
//                                    (Wahl- / Pflichtraetsel)

inherit "/i/room";

#include <quest.h>
#include <invis.h>
#include <monster.h>
#include <more.h>
#include <move.h>
#include <level.h>
#include <time.h>
#include <umfragen.h>
#ifdef UNItopia
#include "/d/Levitanis/sys/levitanis.h"
#endif

//Die Fun holt die Wahlurne und Liste

#define SRWAHL

#ifdef SRWAHL

// Im Dezember und Januar laden wir die Wahlurne


void get_urnen()
{
   object urne,liste;
   int* now;
   
   now = timearray(time());

   if (now[TM_MON]==12 || now[TM_MON]==1)
   {
	if(!present(urne = touch("/room/wahlen/srwurne.c")))
	    urne->move(this_object());
	liste = find_object("/apps/spielerrat_kandidatenliste");
	if(liste && present(liste))
	    liste->remove();
   }
     
   // Nur im November anzeigen, ab Dezember gibt es die srwurne.
   if (//now[TM_MON]>11 || 
	(now[TM_MON]==11 && now[TM_MDAY]>=15))
	    if(!present(liste = touch("/apps/spielerrat_kandidatenliste")))
		liste->move(this_object());

}
#endif


void get_leo()
{
   object leo;

   if(!present(leo = touch(abs_path("div/leo"))))
      leo->move(this_object());
}


void get_inschriften()
{
   object inschriften;

   if(!present(inschriften = touch(abs_path("div/inschriften"))))
      inschriften->move(this_object());
}


void reset()
{
    get_leo();
#ifdef SRWAHL
    get_urnen();
#endif
    get_inschriften();
}

void init()
{
    reset ();
}

string show_pflichtraetsel()
{
    string text, *raetsel;
    int i;

    raetsel = QUEST_ROOM->query_quests(Q_NECESSARY);
    text  = "\n Pflichtabenteuer\n ----------------\n\n";
    text += " Diese Abenteuer musst Du bestehen, um Engel werden zu "
            "können:\n\n";
    for(i=0; i < sizeof(raetsel); ++i)
        text += "-----\n"+wrap(QUEST_ROOM->query_quest_info(raetsel[i]));
    text += "-----\n";
    this_player()->more(explode(text,"\n"), 0, 0, M_AUTO_END);    
    return "";
}

string show_wahlraetsel()
{
    string text, *raetsel;
    int i;

    raetsel = QUEST_ROOM->query_quests(Q_WAHL);
    text  = "\n Wahlabenteuer\n -------------\n\n";
    text += "Wähle aus dieser Liste "+NEEDED_CHOICE_QUESTS+
            " Abenteuer aus:\n\n";
    for(i=0; i < sizeof(raetsel); ++i)
        text += "-----\n"+wrap(QUEST_ROOM->query_quest_info(raetsel[i]));
    text += "-----\n";
    this_player()->more(explode(text,"\n"), 0, 0, M_AUTO_END);    
    return "";
}

//-----------------------------------------------------Anfang uiko
private int hat_uiko(object pl)
{
    if (!living(pl))
    {
        return 0;
    }
    object * ua = filter(all_inventory(pl), (: $1->id("uiko") :) );
    return sizeof(ua) > 0;
}

string ecke_long(mapping vitem, object wer)
{
    if (file_size(UMFRAGE_MASTER+".c")==-1)
    {
        return wrap("Leider ist kein Umfrage- und Ideenkobold mehr da, "
            "und kommt voraussichtlich nicht mehr wieder.");
    }
    else if (hat_uiko(wer))
    {
        return wrap("Die Ecke ist leer, der UIKO sitzt Dir schon auf der "
                "Schulter.");
    }
    else if (guestp(wer))
    {
        return wrap("Die Ecke ist leer, "
            "Gästen ist der Uiko nicht zu Diensten.");
    }
    else if (touch(UMFRAGE_MASTER)->ist_uiko_aktiv())
    {
        return wrap("Der Umfrage- und Ideenkobold bemerkt und erwidert Deinen "
                "Blick und möchte von Dir aufgenommen werden.");
    }
    else
    {
        return wrap("Leider ist kein Umfrage- und Ideenkobold mehr da, "
            "versuch es bitte später nochmal.");
    }
}

int invis_uiko(mapping vitem)
{
    object who = this_player();
    return (hat_uiko(who)||file_size(UMFRAGE_MASTER+".c")==-1
        ||!touch(UMFRAGE_MASTER)->ist_uiko_aktiv()) ? V_INVIS : V_VIS;
}

private void secure_ghost(object pl)
{
    if (objectp(pl))
    {
        pl->close_con();
    }
}

string nimm_uiko(mapping was)
{
    object ob;
    if(this_player() && (this_player()==previous_object()))
    {
        if (file_size(UMFRAGE_MASTER+".c")==-1)
        {
            return wrap("Da hat es keinen Uiko mehr.");
        }
        if (hat_uiko(this_player()))
        {
            return wrap("Du hast bereits einen Umfrage- und Ideenkobold.");
        }
        if (!touch(UMFRAGE_MASTER)->ist_uiko_aktiv())
        {
            return wrap("Leider ist kein Umfrage- und Ideenkobold mehr da, "
                "versuch es bitte später nochmal.");
        }
        if (guestp(this_player()))
        {
            return wrap("Gästen ist der Uiko nicht zu Diensten.");
        }
        ob = clone_object(UMFRAGE_AUTOLOAD);
        if (this_player()->query_ghost())
        {
            this_player()->open_con();
            call_out(#'secure_ghost,0,this_player());
        }
        ob->move(this_player(), ([MOVE_FLAGS:MOVE_ERR_REMOVE]));
        if (this_player()->query_ghost())
        {
            secure_ghost(this_player());
            remove_call_out(#'secure_ghost);
        }
        
        if (!objectp(ob)) {
            return wrap("Du kannst den UIKO nicht nehmen, "
                    "evtl. zuviel dabei?");
        }
        return wrap("Du nimmst "+einen(ob)+".");
    }
    else
    {
        return wrap("Abfrage durch ein Tool zum Nehmen des UIKO.");
    }
}

//-----------------------------------------------------Ende uiko

void create()
{
   set_own_light(1);
   add_type("kunstlicht",1);
   add_type("kaempfen_verboten",1);
   add_type("landeplatz","treppe");
   set_short("Halle der Reinkarnation");
   set_long(
      "Du bist in der Halle der Reinkarnation. Sie hat einen "
      "quadratischen Grundriss von etwa 20 Metern Kantenlänge; ihr "
      "Boden ist mit einem Mosaik aus verschiedenfarbigen "
      "Marmorplatten ausgelegt. Eine stattliche Anzahl von an den "
      "Wänden befestigten Fackeln taucht die Halle in ein unruhiges "
      "Dämmerlicht und die Decke verliert sich in einem Gewirr von "
      "rußgeschwärzten Balken.\n"
      "Rechts und links neben dem Torbogen befindet sich jeweils eine "
      "mannshohe Tafel. In der Mitte des Raumes steht auf einer großen "
      "schwarzen Platte die Säule der Weisheit. Vor den beiden "
      "Tafeln und rund um die Platte ist der Boden beachtlich "
      "abgenutzt.\n"
      "An der östlichen Seite der Halle führt eine alte, "
      "ausgetretene Treppe nach oben.");
   set_exits( ({"foyer","forum"}), ({"süden","hoch"}) );
   set_exit_msg("hoch",
       "$Der(OBJ_TP) geht die Treppe hinauf",
       "$Ein(OBJ_TP) kommt von unten herauf");
    add_v_item(([
      "name":"boden",
      "gender": "maennlich",
      "id":({"boden","platten","mosaik"}),
      "long":"Die Marmorplatten bilden ein geometrisches Muster aus "
        "ineinander ver-\nschlungenen, aus einer roten und einer schwarzen "
        "Linie auf weißem Grund\ngebildeten Doppelspiralen. In der Mitte "
        "der Halle ist eine große schwarze\nPlatte eingelassen, auf deren "
        "Zentrum die Säule der Weisheit steht.\n",
      "look_msg":"$Der() mustert den Boden der Halle",
      "feel":  "Der Boden fühlt sich glatt und sauber an. Offenbar wird "
               "hier regelmäßig gereinigt.",
      "smeel": "Marmor stinkt nicht!",
      "noise": "Ab und zu hörst du jemanden leise über den Boden laufen. "
               "Sonst ist es verdächtig still.",
       ]));

    add_v_item(([
      "name":"platte",
      "gender": "weiblich",
      "long":"Die Platte ist kreisrund und eine Inschrift ist spiralförmig "+
        "von außen nach\ninnen eingemeißelt:\n\n"
        "                        Folge dem EINEN Weg\n"
        "                        und Du wirst es nicht bereuen,\n"
        "                        denn es ist der Weg der Götter.\n\n"
        "Etliche weitere Inschriften folgen.\n",
      "read": "                        Folge dem EINEN Weg\n"
        "                        und Du wirst es nicht bereuen,\n"
        "                        denn es ist der Weg der Götter.\n"
        "Etliche weitere Inschriften folgen.\n",
      "look_msg":"$Der() schaut sich die Platte unter der Säule der Weisheit "+
        "an" ]));

    add_v_item(([
      "name":	"treppe",
      "gender":	"weiblich",
      "id":	({"treppe","holztreppe"}),
      "long":	"Die Treppe ist wohl uralt, die Stufen schon ziemlich "
                "mitgenommen, sie führt nach oben. Unter der Treppe "
                "befindet sich eine Nische.",
      "noise":	"Die Treppe gibt leider kein Geräusch von sich. Nur, "
                "wenn jemand gerade hinaufgeht oder herunterfällt, "
		"kannst du dort welche vernehmen.",
    ]));
    
    add_v_item( ([
      "name" : "treppennische",
      "gender" : "weiblich",
      "id" : ({ "treppennische", "nische" }),
      "long" : "Die Nische unter der Treppe kann man betreten.",
      "enter_room" : "/room/rathaus/treppennische",
    ]) );

    add_v_item(([
      "name":"decke",
      "gender": "weiblich",
      "id":({"decke","balken"}),
      "long":"Bei näherem Hinsehen erkennst du in dem Gewirr ein "+
        "dreidimensionales\nFachwerk aus einer Unmenge an verschiedenen "+
        "Vielecken.\n",
      "look_msg":"$Der() wirft einen Blick zur Decke" ]));

    add_v_item(([
      "name":"säule",
      "gender": "weiblich",
      "long":"Du siehst eine fünf Meter hohe und ein Meter durchmessende "+
        "Säule aus\nweißem Marmor, die mit mehreren eingemeißelten "+
        "Inschriften versehen ist.\nSie steht auf einer schwarzen, aus dem "+
        "Boden herausragenden Platte, die\nebenfalls mit Schriftzeichen "+
        "versehen ist.\n",
      "smell":"Der Marmor ist absolut geruchsneutral! Ehrlich!",
      "feel":"Der Marmor fühlt sich glatt und etwas kühl an - "
             "außer natürlich an den Stellen mit den Inschriften.",
      "noise":"Nein, die Säule spricht nicht mit dir! Nur die "
              "Inschriften sind interessant, und die musst du lesen - "
	      "so du denn des Lesens mächtig bist.",
	]));

    add_v_item(([
      "name":"schriftzeichen",
      "id":({"schriftzeichen","zeichen","schrift"}),
      "gender":"saechlich",
      "plural":1,
      "long":"Zeichen eben, man kann sie lesen, wenn man kann.",
      "read":"$B)hs0-*(k/ZDFInisoTndzxfQSDJH!\n"
        "Dir kommt das reichlich spanisch vor..."]));

    add_v_item(([
      "name":"marmor",
      "gender":"maennlich",
      "long":"Marmor... Da Du keine Ahnung von Steinen hast, findest Du ihn "
        "eher langweilig."]));

    add_v_item(([
      "name":"marmorsäule",
      "gender":"weiblich",
      "id":({"säule","marmorsäule"}),
      "long":"Eine Säule aus Marmor. Sie ist mehr hoch als breit, und wenn "
        "sie nicht da wäre, würde Dir wohl die Decke auf den Kopf fallen. "
        "Oder auch nicht."]));

    add_v_item(([
      "name":"tafel",
      "gender":"weiblich",
      "adjektiv":"recht",
      "id":({"tafel", "granittafel", "granit"}),
      "read":#'show_pflichtraetsel,
      "long":"Es sind zwei aus Granit gehauene Tafeln, die sich rechts "
             "und links neben dem Torbogen befinden. Du solltest "
             "sie lesen (lies rechte/linke Tafel)."]));

    add_v_item(([
      "name":"tafel",
      "gender":"weiblich",
      "adjektiv":"link",
      "id":({"tafel", "granittafel", "granit"}),
      "read":#'show_wahlraetsel,
      "long":"Es sind zwei aus Granit gehauene Tafeln, die sich rechts "
             "und links neben dem Torbogen befinden. Du solltest "
             "sie lesen (lies rechte/linke Tafel)."]));
    add_v_item (([
	"name":"wände",
	"gender":"weiblich",
	"plural":1,
	"id":({"wand","wände"}),
	"long":"Ein Meer von Fackeln siehst Du an den Wänden. "
	       "Sie geben dieser Halle eine besondere Atmosphäre. "
           // "Eine Ecke fällt dir auf." // Hinweis auf uiko -myonara
           // Hinweis entfernt  -myonara.
#ifdef UNItopia
	       " Ein kleines Bild hingegen führt an der hinteren Wand ein "
	       "Schattendasein."
    ]));
    add_v_item (([
        "name":"bild",
        "gender":"saechlich",
        "adjektiv":"belanglos",
        "id":({"bild"}),
        "long":(: wrap("Ein wunderschön gemaltes Bild. Es ist so gut gemalt als "
              "könnte man hindurchsteigen und die gemalte Szene betreten. "
              "Du betrachtest die Szene genauer: "+
              LEVITANIS_MASTER->query_anknuepfhinweis(LEVITANIS_MASTER->query_anknuepfpunkt())) :),
        "feel":"Es fühlt sich an wie eine bemalte Leinwand.",
        "smell":"Es riecht ganz leicht nach Farbe.",
        "take":"Das Bild ist so fest mit der Wand verbunden das du es "
    	       "einfach nicht davon loskriegst."
#endif
    ]));

    add_v_item( ([
            "name" : "ecke",
            "id" : ({ "ecke" }),
            "gender" : "weiblich",
            "long" : #'ecke_long, //'
            ]) );
    add_v_item( ([
            "name" : "uiko",
            "id" : ({ "uiko","kobold","umfragekobold","ideenkobold" }),
            "gender" : "maennlich",
            "long" : #'ecke_long,//'
            "take" : #'nimm_uiko,//'
            "take_msg" : "$der() fummelt in der Ecke herum.",
            "invis": #'invis_uiko,//'
            ]) );   

    
    add_v_item(([
	"name": "fackeln",
	"gender": "weiblich",
	"plural": 1,
	"long": 
	    "Viele, viele Fackeln sind an den Wänden angebracht und sorgen "
	    "dafür, dass diese Halle ständig in ein unruhiges Dämmerlicht "
	    "getaucht wird. Die daraus resultierenden Schatten scheinen sich "
	    "ständig zu bewegen.",
	"feel":
	    "Alle Fackeln auf einmal kannst du nicht befühlen, also "
	    "entscheidest du dich für eine und verbrennst dir furchtbar "
	    "die Finger. Die sind doch angezündet!",
	"noise":
	    "Das Feuer der Fackeln knistert ab und zu leise.",
	"smell":
	    "Die Fackeln riechen harzig und leicht verbrannt.",
    ]));

    reset();
}

/*
varargs mapping query_v_item(mixed *path, int flag)
{
    int nr;
    string long, name, read, *raetsel, *spiele;
    string look_msg, read_msg;

    if (sizeof(path)!=1)
        return room::query_v_item(path,flag);
    if (stringp(path[0]))
        name=path[0];
    else
        name=path[0]["name"];

    if (!name || name=="")
        return 0;

    nr=mappingp(path[0])?path[0]["nummer"]:0;

    if (!nr)
    {
        if (name == "tafeln" || name == "tafel")
        {
            raetsel = QUEST_ROOM->query_quest_objects();
            name="tafeln";
            if (sizeof(raetsel) <= 0)
            {
                long="Es sind genau sieben aus Granit gehauene und "+
                     "unbeschriftete Tafeln.\n";
                read="Es steht nichts auf den Tafeln.\n";
            }
            else
            {
                long="Es sind genau "+sizeof(raetsel)+
                     " aus Granit gehauene und beschriftete Tafeln.\n";
                read="Welche Tafel genau willst Du lesen?\n"
                     "Versuch es mit z.B. 'lies 3. Tafel'.\n";
            }
            read_msg = Der(this_player())+" schaut sich die Tafeln an.";
        }
    }
    else
    {
        if (name == "tafel")
        {
            raetsel = QUEST_ROOM->query_quests();
            if (nr <= sizeof(raetsel) && nr > 0)
            {
                look_msg = Der(this_player())+" schaut sich die "+nr+
                        ". Tafel an.";
                read_msg = Der(this_player())+" liest die "+nr+". Tafel.";
                long=read=QUEST_ROOM->query_quest_info(raetsel[nr-1]);
            }
        }
    }
    if (long)
        return (["name":name,
                 "plural":1,
                 "look_msg":look_msg,
                 "read_msg":read_msg,
                 "long":long,
                 "read":read
               ]);
    else
        return room::query_v_item(path,flag);
}
*/

int filter_hoch(object who)
{
   if(!wizp(who))
      {
        tell_object(who, wrap(
            "Du betrittst die Treppe, die nach oben führt; doch bereits "
            "nach der dritten Stufe verwandelt sie sich unter Deinen Füßen "
            "in eine glatte Rutschbahn, die Du auch prompt runterrutschst. "
            "Sofort verwandelt sich die Rutsche wieder zurück in eine "
            "Treppe und tut so, als ob nichts gewesen wäre."));
        tell_room(this_object(), wrap(
            Der(who)+" betritt die Treppe, die nach oben führt; doch bereits "
            "nach der dritten Stufe verwandelt sich diese unter den Füßen "+
            des(who)+" in eine glatte Rutschbahn, die "+
            der(who)+" auch prompt runterrutscht. "
            "Sofort verwandelt sich die Rutsche wieder zurück in eine "
            "Treppe und tut so, als ob nichts gewesen wäre."),
            ({who}));
        return 1;
    }
}

int key_reinkarnation(string str, string verb, object monster, object player,
    int flags)
{
    monster->do_command("sage Zu Leo und zur Halle der Reinkarnation "
        "geht es runter in den Spielerbereich.");
}

int key_aufstieg(string str, string verb, object monster, object player,
    int flags)
{
    monster->do_command("sage Der Aufstieg von Lehrling zu Geselle zu Vogt "
        "behandelt Leo in der Halle der Reinkarnation (Ausgang runter).");
}


mixed *query_keyword_rules()
{
    return ({
"key_reinkarnation: spieler || reinkarnation || leo ", PARSE_SAY|PARSE_CONTINUE,
"key_aufstieg: aufstieg || lehrling || geselle || vogt", 
        PARSE_SAY|PARSE_CONTINUE,
    });
}
