// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /map/obj/killerwal.c
// Description: Ein Killerwal fuer den Ozean
// Author:      Anin (13.05.2008)

inherit "/map/i/stranden";

#include <message.h>

string query_leiche_kurz()
{
  return "Die Leiche eines großen Schwertwals";
}

string query_leiche_lang()
{
  return "Die Leiche eines großen Schwertwals, der auf das Land geraten ist.";
}

void create()
{
    ::create();
    initialize("killerwal",50);
    set_name("schwertwal");
    set_adjektiv(({"groß"}));
    set_gender("maennlich");
    set_id(({"wal","killerwal","orca","schwertwal", "map fish"}));
    set_long("Oberkopf, Brustflossen sowie Rücken- und Schwanzflosse dieses "
             "großen Schwertwals sind schwarz, die Kehle und der Bauch "
             "weiß. Die Seiten und der Rücken sind tief schwarz mit Ausnahme "
             "eines weißen, ovalen Flecks hinter dem Auge. Hinter der großen "
             "Rückenfinne ist noch ein gräulicher Sattelfleck. Der Killerwal "
             "hat riesige scharfe Zähne in seinem Maul!");
    set_noise("Im Moment scheint er gerade nicht mehr zu singen.");
    set_aggressive(1);
    set_align(-30);

    load_chat(5,
              ({ ({ MT_LOOK, MA_MOVE,
                    "$Der() springt aus dem Wasser." }),
                 ({ MT_LOOK, MA_MOVE,
                    "$Der() schwimmt um dich herum." }),
                 ({ MT_LOOK, MA_CRAFT,
                    "$Der() macht Wellen, die dich ordentlich "
                    "durchschaukeln." }),
                 ({ MT_LOOK, MA_CRAFT,
                    "$Der() hebt den Oberkörper aus dem Wasser, indem "
                    "$er() mit den Brustflossen rudert." }),
                 ({ MT_LOOK, MA_CRAFT,
                    "$Der() zeigt dir seine Zähne." }),
                 ({ MT_LOOK, MA_MOVE,
                    "$Der() springt aus dem Wasser." }),
                 ({ MT_NOISE, MA_NOISE,
                    "Du hörst den Walgesang $des()." }),
                 ({ MT_LOOK, MA_CRAFT,
                    "$Der() taucht auf und pustet eine Blaswolke aus seinem "
                    "Blasloch." }),
                 ({ MT_NOISE, MA_NOISE,
                    "Die Schwanzfluke $des() klatscht laut auf die "
                    "Wasseroberfläche." }),
                 }) );

    if(clonep())
    {
        add_v_item_master(load_name());
    }
    else
    {
        add_v_item(([
	   "name"     : "zähne",
	   "gender"   : "maennlich",
	   "plural"   : 1,
	   "id"       : ({"zahn", "zähne", "maul", "mund", "kiefer",
                          "oberkiefer", "unterkiefer", "natural#weapon"}),
	   "long"     : "Du schätzt, dass da gut 50 große konische Zähne "
                        "auf ein Opfer warten. Hoffentlich stehst du nicht auf "
                        "dem Speiseplan!",
           "far"      : "Du traust dich nicht nahe genug ran!",
	]));
        add_v_item(([
           "name"     : "rücken",
           "gender"   : "maennlich",
           "id"       : ({"rücken","oberseite","dorsalseite","nacken"}),
           "long"     : "Hinter der Finne liegt ein gräulicher Fleck auf dem "
                        "sonst komplett schwarzen Rücken. Diesen nennt man "
                        "Sattelfleck.",
           "far"      : "Du kommst nicht nahe genug ran!",
                        ]));
        add_v_item(([
           "name"     : "sattelfleck",
           "gender"   : "maennlich",
           "id"       : ({"sattelfleck","sattel","klammer"}),
           "adjektiv" : ({"gräulich"}),
           "long"     : "Hinter der Finne liegt der gräuliche Sattelfleck. "
                        "Bei diesem Schwertwal sieht er aus wie eine riesige "
                        "geschweifte Klammer.",
           "far"      : "Du kommst nicht nahe genug ran!",
                        ]));
        add_v_item(([
           "name"     : "flossen",
           "gender"   : "weiblich",
           "plural"   : 1,
           "id"       : ({"flossen","flosse","walflosse","walflossen",
       	                  "fischflosse","fischflossen"}),
           "long"     : "Du findest eine Schwanzflosse, zwei Brustflossen, "
                        "und eine Rückenflosse.",
           "look_msg" : "$Der() sucht den Walkörper nach Flossen ab.",
           "far"      : "Du kommst nicht nahe genug ran!",
                        ]));
        add_v_item(([
           "name"     : "brustflossen",
           "gender"   : "weiblich",
           "plural"   : 1,
           "id"       : ({"brustflossen","brustflosse","flipper"}),
           "long"     : "Du beobachtest die Brustflossen des Schwertwals: Die "
                        "Flipper sind paddelförmig und ungefähr einen Meter "
                        "lang.",
           "look_msg" : "$Der() beobachtet die Bewegungen der Brustflossen.",
           "far"      : "Du kommst nicht nahe genug ran!",
                        ]));
        add_v_item(([
           "name"     : "rückenflosse",
           "gender"   : "weiblich",
           "id"       : ({"rückenflosse","finne","rückenfinne"}),
           "long"     : "Die Finne ist bestimmt einen Meter hoch und spitz.",
           "look_msg" : "$Der() lässt $seinen(([name:blick,gender:maennlich]),"
       	                "0,OBJ_TP) über den Rücken des Schwertwals wandern.",
           "far"      : "Du kommst nicht nahe genug ran!",
                        ]));
        add_v_item(([
           "name"     : "schwanzflosse",
           "gender"   : "weiblich",
           "id"       : ({"schwanzflosse","fluke","schwanz","hinterende",
                          "schwanzfluke"}),
           "long"     : "Die Fluke ist schwarz und an der Unterseite weiß. Sie "
                        "ist leicht gebogen, deutlich gekerbt und läuft an den "
                        "Enden spitz aus.",
           "look_msg" : "$Der() begutachtet die Schwanzflosse des Schwertwals.",
           "far"      : "Du kommst nicht nahe genug ran!",
                        ]));
        add_v_item(([
           "name"     : "kopf",
           "gender"   : "maennlich",
           "id"       : ({"kopf","walkopf","oberkopf","vorderende","schnauze",
                          "unterkopf"}),
           "long"     : "Der Kopf ist abgeflacht und hat eine kurze, leicht "
                        "abgesetzte Schnauze. Der Oberkopf einschließlich "
                        "Oberkiefer ist komplett schwarz, der untere Teil "
                        "komplett weiß.",
           "look_msg" : "$Der() schaut sich den Kopf des Schwertwals an.",
           "far"      : "Du kommst nicht nahe genug ran!",
                        ]));
        add_v_item(([
           "name"     : "unterseite",
           "gender"   : "weiblich",
           "id"       : ({"unterseite","kehle","bauch"}),
           "long"     : "Die komplette Unterseite von Unterkiefer über Kehle "
                        "und Bauch bis zur Fluke ist weiß.",
           "look_msg" : "$Der() begutachtet den Schwertwal so gut $er() kann "
                        "von unten.",
           "far"      : "Du kommst nicht nahe genug ran!",
                        ]));
        add_v_item(([
           "name"     : "augen",
           "gender"   : "saechlich",
           "plural"   : 1,
           "id"       : ({"augen","auge","fleck"}),
           "long"     : "Hinter den dunklen Augen ist jeweils ein weißer "
                        "Fleck zu sehen.",
           "look_msg" : "$Der() versucht, dem Schwertwal in die Augen zu "
                        "schauen.",
           "far"      : "Du kommst nicht nahe genug ran!",
                        ]));
        add_v_item(([
           "name"     : "blasloch",
           "gender"   : "saechlich",
           "id"       : ({"blasloch","loch","atemwegsoeffnung","atemloch",
                          "atemöffnung"}),
           "long"     : "Zwischen Kopf und Rücken des Schwertwals kannst du "
                        "sein Blasloch entdecken, welches ihm als Atemöffnung "
                        "dient.",
           "look_msg" : "$Dem() gelingt es, einen Blick auf das Blasloch "
                        "des Schwertwals zu werfen.",
           "far"      : "Du kommst nicht nahe genug ran!",
                        ]));
        add_v_item(([
           "name"     : "haut",
           "gender"   : "weiblich",
           "id"       : ({"haut","oberfläche","farbe","hautfarbe","blubber",
                          "natural#armour"}),
           "long"     : "Die Grundfarben das Schwertwals sind eindeutig "
                        "Schwarz und Weiß. Einzig der Sattelfleck ist grau. "
                        "Die grobe Aufteilung ist auch recht einfach gehalten: "
                        "Oben schwarz, unten weiß.",
           "look_msg" : "$Der() betrachtet den Walkörper von allen "
       	                "möglichen Seiten.",
           "far"      : "Du kommst nicht nahe genug ran!",
                        ]));
    }
}
