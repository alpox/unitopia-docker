// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/room/rathaus/treppe.c
// Description:
// Modified by: Kurdel     (19.11.96) - Zeitungstisch statt v_items +
//                                      add_actions (Bugs), Rechtschreibung,
//                                      v_item-ids, here() in kratze(),
//                                      closure in "stufen"
//              Tmm        (28.05.00) - Anpassung an neues Vaniorh,
//                                      code formatiert
//                         (11.06.00) - kleiner Fix + Zeitung neu

inherit "/i/tools/security";
inherit "/i/room";
#ifdef UNItopia
#include "/d/Vaniorh/sys/vaniorh_exports.h"
inherit TOTENGRAEBERINHERIT;
#endif

#include <stats.h>
#include <config.h>
#include <deklin.h>
#include <description.h>
#include <editor.h>
#include <invis.h>
#include <message.h>
#include <misc.h>
#include <more.h>
#include <notify_fail.h>
#include <room.h>
#include <touch.h>
#include <umfragen.h>

#define SR_KANDIDATENLISTE "/apps/spielerrat_kandidatenliste"

int bereits_geholt;

string query_all_wahlplakate_long(mapping v_item, object viewer)
{
    string* names=SR_KANDIDATENLISTE->query_wahlplakat_namen();
    string ktxt = "";
    if (SR_KANDIDATENLISTE->ist_kandidat(viewer->query_real_name()))
        ktxt = " Als Kandidat kannst du Dein Wahlplakat verfassen.";
    switch (sizeof(names))
    {
        case 0:
            return wrap("Kein Kandidat hat ein Wahlplakat zur Verfügung "
                "gestellt."+ktxt);
        case 1:
            return wrap("Es gibt nur ein Wahlplakat von "+names[0]
                +". Man kann es mit 'lese plakat' lesen."+ktxt);
        default:
            return wrap_say("Es gibt "+sizeof(names)+" Wahlplakate "
                "('lese plakate'):", implode(names,", ")+"."+ktxt);
    }    
}

string read_one_wahlplakat(string parse_rest, 
            string str,mapping vitem, object leser)
{
    string * lines;
    if (member(vitem,"WAHLPLAKAT_NAME"))
    {
        lines = SR_KANDIDATENLISTE->query_one_plakat(vitem["WAHLPLAKAT_NAME"]);
        if (sizeof(lines))
        {
            leser->more(lines,"--Mehr--",0,M_AUTO_END);
            return "";
        }
    }
    return "Kein Wahlplakat gefunden.";
}

string read_all_wahlplakate(string parse_rest, 
            string str,mapping vitem, object leser)
{
    string* names=SR_KANDIDATENLISTE->query_wahlplakat_namen();
    string* lines = ({}),name;
    foreach (name:names)
    {
        lines += SR_KANDIDATENLISTE->query_one_plakat(name);
    }
    if (!sizeof(lines))
        return "Keine Wahlplakate gefunden!\n";
    leser->more(lines,"--Mehr--",0,M_AUTO_END);
    return "";
}

private mapping get_one_plakat(string name)
{
    return ([
        "name" : "wahlplakat",
        "WAHLPLAKAT_NAME":name,
        "id" : ({ "wahlplakat","plakat"}),
        "gender": "saechlich",
        "long": "Ein Wahlplakat von "+name+". Man kann es lesen.",
        "read": #'read_one_wahlplakat,
    ]);
}

private mapping *get_wahlplakate_v_items()
{
    string* names=SR_KANDIDATENLISTE->query_wahlplakat_namen();
    string name;
    mapping m,*result = ({});
    if (!sizeof(names))
    {
        return ({});
    }
    foreach (name : names)
    {
        m = get_one_plakat(name);
        if (mappingp(m))
            result += ({ m });
    }
    return result;
}

private int my_check_entry(mapping entry, string ids, string *adj)
{
    if (mappingp(entry) && HAS_ID(entry,ids)) {
        if (adj) {
            foreach(string str : adj) {
                if (member(entry["adjektiv"]||({}),str) == -1) {
                    return 0;
                }
            }
        }
        return 1;
    }
    return 0;
}

varargs mapping query_v_item(mixed *pfad, int flag)
{
    mapping ret, what, *visa;
    string id, *adj;
    int nummer;

    visa = get_wahlplakate_v_items();
    if(!(ret = ::query_v_item(pfad,flag)) &&
       sizeof(pfad)==1)
    {
        if(stringp(pfad[0]))
        {
            id = lower_case(pfad[0]);
        }
        else if(mappingp(pfad[0]))
        {
            id = lower_case(pfad[0]["name"]);
            adj = pfad[0]["adjektiv"];
            nummer=pfad[0]["nummer"];
        }
        visa=filter(visa,#'my_check_entry,id,adj);
        if(sizeof(visa))
        {
            if(nummer > 1)
            {
                if(sizeof(visa)>=nummer)
                    what = visa[nummer-1];
                else
                    return 0;
            } else {
                what = visa[0];
            }
            ret = what + ([
              "environment"   : this_object(),
              "v_item_master" : this_object(),
                              ]);
        }
    }
    return ret;
}

mixed *query_all_v_items()
{
    mapping *visa;
    visa = get_wahlplakate_v_items();
    return (::query_all_v_items()||({}))
            +map(visa,
                 function(mapping entry)
                 {
                     return entry + ([
                 "environment"   : this_object(),
                 "v_item_master" : this_object(),
                                 ]);
                     });
}

int invis_wahlplakate()
{
    return SR_KANDIDATENLISTE->get_phase() ? V_VIS : V_INVIS;
}

private void ed_end(string *lines)
{
    string cname = this_player() ? this_player()->query_real_cap_name() : 0;
    if (!cname) return;
    string result = SR_KANDIDATENLISTE->update_my_plakat(cname,lines);
    this_player()->send_message_to(this_player(),MT_NOTIFY,MA_UNKNOWN,wrap(result));
}

int cmd_verfasse(string str)
{
    if (space(str) != "mein wahlplakat")
    {
        FAILWP("verfasse mein wahlplakat?",FAIL_NOT_OBJ);
    }
    if (!check_security())
    {
        FAILWP("So nicht.",FAIL_INTERNAL);
    }
    string cname = this_player()->query_real_cap_name();
    string rname = this_player()->query_real_name();
    string *text = 0;
    if (!rname || !cname || rname != lower_case(cname))
    {
        FAILWP("Name nicht identifiziert.",FAIL_INTERNAL);
    }
    if (!SR_KANDIDATENLISTE->ist_kandidat(rname))
    {
        FAILWP("Du bist kein Kandidat, also kein Plakat.",FAIL_INTERNAL);
    }
    text = SR_KANDIDATENLISTE->query_my_plakat(cname);
    if (this_player()->mini_ed(#'ed_end, 0,0,0,([
               MINI_ED_FORCE_WRAP:1,
               MINI_ED_TITLE:"Wahlplakat beschreiben"]),text))
    {
        return 1;
    }
    FAILWP("Aufruf Editor fehlgeschlagen.",FAIL_INTERNAL);
}

void init()
{
  add_action("kratze","kratze",-5);
  add_action("cmd_verfasse","verfasse",-7);
}

void reset()
{
  bereits_geholt = 0;
  if (!present("messer"))
  {
    object tmp;

    tmp = clone_object("/obj/nahkampf_waffe");
    tmp->set_id("messer");
    tmp->set_value(35);
    tmp->set_name("messer");
    tmp->set_long("Ein scharfes Messer. "
                  "Als reines Essutensil geht das nicht mehr durch. "
                  "Es besteht aus Metall.");
    tmp->set_gender("saechlich");
    tmp->set_material("metall");
    tmp->set_weight(2);
    tmp->set_skill_path(({ "skill", "offensiv", "scharf", "messer" }));
    tmp->set_used_stats(({ STAT_STR, STAT_DEX, STAT_DEX, STAT_INT }));
    tmp->set_damage(2,7);
    tmp->set_life(100);
    tmp->move(this_object());
  }

#ifdef UNItopia
  object ob = touch(BETTLER);
  if (ob && !environment(ob))
    ob->move(this_object());
  if (!present("tisch",this_object()))
  {
    object tisch=clone_object("/p/Item/Install/obj/desk");
    tisch->move(this_object());
    clone_object(ZEITUNG)->move(tisch); // Inhaltstyp festlegen
    tisch->reset(); // Auffuellen
  }
#endif
}

int hat_wendeltreppe()
{
    object master = touch(UMFRAGE_MASTER, NO_LOG|NO_WRITE);
    return master && master->ist_wolke_aktiv();
}

<int|string> filter_hoch(object wer)
{
    if (hat_wendeltreppe())
        return 0;
    return "Da geht's nicht weiter.\n";
}

string query_long(object wer)
{
    string t = ::query_long(wer);
    if (hat_wendeltreppe())
    {
        t+= wrap("Ueber dem Rathaus schwebt eine goldene Wolke, "
            "eine goldene Wendeltreppe fuehrt hoch zu ihr.");
    }
    if (SR_KANDIDATENLISTE->get_phase())
        t += wrap("Hier stehen ein paar Wahlplakate zur "
            "SpielerratsWAHL herum.");
    return t;
}

void create()
{
  set_short("Rathaustreppe");
  set_long("Du stehst auf den Treppen vor dem Portal des Rathauses. An der "
           "Wand neben dem linken Türflügel hängt eine Tafel. Meistens "
	   "treiben sich hier ein paar Bettler herum.");
  add_type("graben_verboten",
           "Hier auf der Treppe graben? Wie soll das "
             "gehen?");
  init_security_for_actions();
  set_exits(({
#ifdef UNItopia
  K_PLATZ_NW,
#else
  "/room/church",
#endif
              "foyer"}),
            ({
#ifdef UNItopia
            "süden",
#else
            "osten",
#endif
            "norden"}));
  add_exit("aufderwendeltreppe","hoch",EXIT_NOLIST);
  add_v_item( ([
        "name":"wahlplakate",
        "id": ({"wahlplakate","plakate"}),
        "gender": "saechlich",
        "plural": 1,
        "long":#'query_all_wahlplakate_long,
        "read":#'read_all_wahlplakate,
        "invis":#'invis_wahlplakate, 
    ]) );
add_v_item(([
      "name"   : "tafel",
      "gender" : "weiblich",
      "id"     : ({"tafel", "wand"}),
      "long"   : "Auf der Tafel an der Wand steht etwas geschrieben.",
      "read"   : ({T_NO_ASCII_ART,
                 "BETTELN UND HAUSIEREN VERBOTEN! Der Bürgermeister.\n",
                   T_ELSE,
                 "\n"
                 "          +-------------------------------+\n"
                 "          |                               |\n"
                 "          |     BETTELN UND HAUSIEREN     |\n"
                 "          |          VERBOTEN !           |\n"
                 "          |                               |\n"
                 "          |            Der Bürgermeister. |\n"
                 "          +-------------------------------+\n\n"})]));
  add_v_item(([
               "name"   : "treppe",
               "gender" : "weiblich",
               "id"     : ({"treppe", "treppen"}),
               "long"   :"Fünf Stufen aus Marmor."
            ]));
  add_v_item(([
               "name"   : "portal",
               "gender" : "saechlich",
               "id"     : ({"portal", "rundbogen", "bogen"}),
               "long"   : "Vor dir steht ein mächtiger, aus einem massiven "
                          "Sandsteinblock gehauener und mit Ornamenten "
                            "verzierter Rundbogen."]));
  add_v_item(([
               "name"   : "ornamente",
               "plural" : 1,
               "gender" : "saechlich",
               "id"     : ({"ornamente", "ornament"}),
               "long"   : "Du siehst sechs Linien, die sich auf ihrem Weg "
                        "längs des Bogens mehrmals verknäueln und wieder "
                        "entwirren."]));
  add_v_item(([
               "name"     : "linien",
               "gender"   : "weiblich",
               "plural"   : 1,
               "id"       : ({"linie", "linien", "gold", "spur", "spuren"}),
               "long"     : "Täuscht das oder sind die Linien tatsächlich "
                            "vergoldet?",
               "look_msg" : "$Der(OBJ_TP) sieht sich die Ornamente ganz genau "
                            "an"]));
  add_v_item(([
               "name"   : "rathaus",
               "gender" : "saechlich",
               "id"     : ({"rathaus", "haus", "gebäude", "quader",
                            "sandsteinquader", "sandstein-quader", "giebel",
                            "fachwerk"}),
               "long"   : "Der aus großen Sandstein-Quadern gemauerte "
                        "zweistöckige, aber recht schmale Giebel wird "
                        "nur durch wenige, kleine Fenster durchbrochen. "
                        "Links und rechts schließen sich jedoch "
                        "augenscheinlich jüngere, einstöckige, nach "
                        "Fachwerkart gebaute Flügel an."]));
  add_v_item(([
               "name"   : "türflügel",
               "gender" : "maennlich",
               "plural" : 1,
               "id"     : ({"tür", "flügel", "türflügel", "eisen",
                            "eichenholz","holz"}),
               "long"   : "Zwei mächtige, aus altem Eichenholz gebaute und "
                        "mit viel Eisen beschlagene Türflügel stehen "
                        "einladend offen."]));
  add_v_item(([
               "name"     : "stufen",
               "gender"   : "weiblich",
               "plural"   : 1,
               "id"       : ({"stufe", "stufen", "treppenstufe",
                            "treppenstufen", "marmor"}),
               "long"     : "Vier Meter breit, 30 cm hoch und aus Marmor.",
               "look_msg" : lambda(({}),({#'wrap,({#'+,({#'+,({#'+,({#'Der,
                            ({#'this_player})})," mustert "}),
                            ({#'seinen,(["name":"füße", "gender":"maennlich",
                            "plural":1]),0,({#'this_player})})}),
                            " - oder die Treppe?"})}))]));
  add_v_item(([
               "name"     : "wendeltreppe",
               "gender"   : "weiblich",
               "attribut" : ({ "golden" }),
               "id"       : ({"wendeltreppe", "treppe"}),
               "long"     : "Eine goldene Wendeltreppe führt hoch in eine "
                            "goldene Wolke.",
               "invis"    : (: hat_wendeltreppe() ? V_VIS : V_INVIS :),
            ]));
  add_v_item(([
               "name"     : "wolke",
               "gender"   : "weiblich",
               "attribut" : ({ "golden" }),
               "id"       : ({"wolke"}),
               "long"     : "Eine goldene Wolke schwebt über dem Rathaus, "
                            "eine Wendeltreppe führt hoch zu ihr.",
               "invis"    : (: hat_wendeltreppe() ? V_VIS : V_INVIS :),
            ]));
  reset();
}

int kratze(string str)
{
  object staub;

  if (!str || !here(lower_case(str),"gold"))
  {
    notify_fail("Kratze was ?\n");
    return 0;
  }
  if (bereits_geholt)
  {
    write(wrap("Da war wohl bereits einer vor dir da, sämtliches Gold "+
 	       "ist bis auf Spuren abgekratzt."));
    return 1;
  }
  if (this_player()->free_hand()<0)
  {
    write("Ohne eine freie Hand?\n");
    return 1;
  }
  bereits_geholt = 1;
  write(wrap("Du kratzt etwas von dem Gold ab, das herunterfällt."));
  say(wrap(Der(this_player())+" kratzt mit bloßen Händen an den Ornamenten "
           "des Portals herum."));
  this_player()->set_handeln();
  this_player()->add_align(-3*ALIGN_STRETCH);
  staub = clone_object("/obj/schatz");
  staub->set_id(({"staub","goldstaub","gold"}));
  staub->set_name("goldstaub");
  staub->set_gender("maennlich");
  staub->set_long("Ein kleines Häufchen Goldstaub.");
  staub->set_smell("Es riecht nach Reichtum.");
  staub->set_menge((["name":"häufchen","gender":"saechlich"]));
  staub->set_value(100);
  staub->set_material("edelmetall");
  staub->move(this_object());
  return 1;
}
