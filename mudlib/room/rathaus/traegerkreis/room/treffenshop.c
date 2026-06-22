// This file is part of UNItopia
// ---------------------------------------------------------------------------
// Description:
//   Der Treffen-Shop. Hier gibt's ne kleine Belohnung fuer alle Treffengeher
// Author: Tiberian
// Created: 03.JUN.2008

inherit "%room";

#include <landschaft.h>
#include <treffenshop.h>

void reset()
{
    object obj;

    "*"::reset();

    if(!present("shop # keeper"))
    {
        touch(VEREIN_TOUCH+"shopkeeper")->move(this_object());
    }

    if(!present("treffen # brett"))
    {
        obj = clone_object("/obj/brett");
        obj->set_brett_name("Spieler","Treffen");
        obj->add_id("treffen # brett");
        obj->move(this_object());
    }
}

void create()
{
    "*"::create();
    set_short("Der Treffenshop");
    add_type("kunstlicht",1);
    add_type("landschaft", L_DRINNEN);
    set_own_light(1);
    set_long("Du stehst im Treffenshop UNItopias. Der längliche, rechteckige "+
        "Raum wird von einem großen Tresen dominiert, hinter dem sich "+
        "Regale türmen. Von der Decke hängt ein etwas vergilbtes "+
        "Transparent mit dem UNItopia-Logo drauf. Der Nordwand entlang "+
        "stehen verschiedene Treffenüberbleibsel aus frühren Jahren "+
        "(und Jahrzehnten...) auf einem Korpus. An der Südwand hängt "+
        "ein schwarzes Brett.");

    add_v_item(([
        "name"      : "treffenüberbleibsel",
        "id"        : ({"treffenüberbleibsel","überbleibsel"}),
        "gender"    : "maennlich",
        "plural"    : 1,
        "take"      : "Als Du Dir eines der Treffenüberbleibsel aneignen "+
                      "willst, fühlst Du Dich urplötzlich etwas unwohl. "+
                      "Irgendwie spürst Du dieses Prickeln im Rücken, "+
                      "das man kriegt, wenn man von einem Admin beobachtet "+
                      "wird. Da Du auch morgen noch die Wunder Magyras "+
                      "entdecken möchtest, entschließt Du Dich, alles "+
                      "an seinem Platz zu lassen... bei Admins weiß man "+
                      "ja nie...",
        "smell"     : "Riecht nach OeZ.",
        "feel"      : "Du fummelst an den Treffenüberbleibseln rum, kommst "+
                      "aber bloß zur Einsicht, dass jetzt die Putzfrau "+
                      "eine Extraschicht wird einlegen müssen, um die "+
                      "ganzen Abdrücke wegzuwischen.",
    ]));

    add_v_item(([
        "name"      : "abdrücke",
        "gender"    : "maennlich",
        "plural"    : 1,
        "long"      : "Uuuuh, da hat jemand Spuren hinterlassen!",
        "smell"     : "Du riechst nur den Geruch der Gegenstände, auf denen "+
                      "die Abdrücke hinterlassen wurden.",
        "feel"      : "Na toll, jetzt hast du noch mehr Abdrücke "+
                      "hinterlassen... hast du eigentlich schon mal ans "+
                      "Raumpflegepersonal gedacht?!",
        "take"      : "Diese hauchdünnen Schmutz- und Fettspuren kann man "+
                      "nicht mitnehmen."
    ]));

    add_v_item(([
        "name"      : "korpus",
        "gender"    : "maennlich",
        "id"        : ({"korpus","quader"}),
        "material"  : ({"holz"}),
        "long"      : "Ein Korpus aus Holz, wie es scheint: ein länglicher "+
                      "Quader, knapp zwergenhoch. Auf dem Korpus stehen "+
                      "allerlei Treffenüberbleibsel.",
        "smell"     : "Geruchlos. Nicht mal staubi. Der Shop wird gut "+
                      "gepflegt.",
        "take"      : "Der Korpus ist zu schwer und unhandlich, um ihn "+
                      "einfach so durch die Gegend zu tragen."
    ]));

    reset();
}
