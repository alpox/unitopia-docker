// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/obj/kompass.c
// Description: Ein Kompass zum Anpeilen des Mittelpunkts von UNItopia
// Author:	Francis

inherit "/i/item";
inherit "/i/move";
inherit "/i/value";
inherit "/i/tools/room_types";

#include <apps.h>
#include <config.h>
#include <deklin.h>
#include <level.h>
#include <invis.h>


string nadellong ()
{
    int     *karts, level, grad;
    int     *position;
    object  env, ob;
    string  domain, msg;
    mixed   tmp ;

    level = ({int})previous_object()->query_level();
    ob = this_player();
    env = get_environment(ob);

    if (!env)
        return wrap ("Die Nadeln sind nur noch schemenhaft zu "
            "erkennen, anscheinend befindest Du Dich im Nichts.");
    if ( env->query_type( "kompass_defekt") )
        return "Die Kompassnadeln drehen sich wie verrückt.\n";
    
    position = KOORDINATEN_MASTER->get_polar_coordinates(ob);
    if ( !position || position[0] < 0 )
        return wrap ("Die Nadeln hängen total schlapp und müde "
            "in ihrer Halterung; anscheinend befindest Du Dich weit, "
            "weit weg vom Mittelpunkt der Welt.");

    domain = env->query_room_domain() ;

    if ((tmp = env->query_type("kompass")) &&
         (stringp(tmp)))
    {
        msg = tmp ;
        msg = regreplace( msg, "%g",  (string) position[0], 1);
        msg = regreplace( msg, "%sm", (string) position[1], 1);
        return wrap(apply(CLOSURE_CONTAINER->do_bind(
            mixed_to_closure( msg, ({ 'kompass //'
                                   })), ({})), this_object()));
    }
    else if ( domain == "Campus" )
    {
        this_player()->set_handeln();
        return wrap ("Die rote Nadel zeigt genau nach Norden zum Nordpol. "
           "Die grüne Nadel... welche grüne Nadel? Die grüne Nadel hat "
           "sich in Luft aufgelöst.");
    }
    else if ( domain == "Ebenen" &&
              !env->query_type( "schiff_erlaubt")
              // damit der vom Ozean aus zugaengliche Teil der Ebenen normal
              // behandelt wird
        )
    {
        this_player()->set_handeln();
        return wrap("Die Kompassnadeln zittern stark und pendeln immer "
                    "wieder hin und her. "
                    "Anscheinend befindet sich der Mittelpunkt der Welt "
                    "nicht in dieser Dimension.");
    }
    else if (position[1] <= 0)
        return wrap ("Die rote Kompassnadel dreht sich wie verrückt, "
            "die grüne steht auf 0 sm. Anscheinend bist du in "
            "unmittelbarer Nähe des Mittelpunktes der Welt.");
    else
    {
        tmp = "Die rote Kompassnadel hat sich auf "+
              (string)position[0]+" Grad eingependelt.\n"
              "Die grüne Nadel zeigt auf "+(string)position[1]+" sm.\n";
        if (level >= LVL_WIZ)
        {
            grad = (position[0] + 180) % 360;
            karts = pol2xy( grad, position[1] );
            return tmp + 
                "\nDaraus berechneter Standort:  m"+
                karts[0]+"_"+karts[1]+".\n";
        }
        return tmp;
    }

}

string query_long(object who)
{
    // wenn man den Kompass selber in der Hand hat dreht man ihn
    // um und sieht die Gravur; bekommt man ihn gezeigt, so dreht
    // man ihn logischerweise nicht um.
    string long;
    long = wrap (
        "Du hast ein kleines, kreisrundes Messinggehäuse vor Dir, "
        "es ist mit einigen magischen und seemännischen Symbolen verziert. "
        "Es sieht fast aus wie eine Taschenuhr, unter einem gläsernen "
        "Deckel hat es nämlich auch zwei Zeiger, oder besser gesagt, "
        "zwei Nadeln. Die beide Nadeln sind unterschiedlich lang und zeigen "
        "auf unterschiedliche Skalen.");
    if (previous_object() == environment())
        return long + wrap (
            "Beim Umdrehen "+des()+" entdeckst Du auf der Unterseite "
            "eine Gravur.");
    return long;
}


void create()
{
    int i;
    set_gender("maennlich");
    set_id(({"kompass", "magnetkompass","marinekompass"}));
    set_name("kompass");
    set_material( ({"metall"}) );
    set_value(50);
    set_weight(1);
    set_smell ("Salziger Meeresgeruch haftet an dem Gehäuse.");
    set_feel ("Das Glas auf der Oberseite ist Glasglatt, die "
        "Gravur an der Unterseite ist rauh, im Messinggehäuse "
        "fühlst Du einige Kratzer.");

    if (clonep())
    {
        add_v_item (([
            "name":"nadeln","gender":"weiblich","plural":1,
            "id":({"nadel","nadeln","zeiger","kompassnadel","kompassnadeln"}),
            "long":#'nadellong
        ]));
        add_v_item_master(load_name());
        clear_initial_conservation_data();
    }
    else
    {
       string* symbollongs = ({
    	    "Dieses Symbol ist ein Anker, leicht zu erkennen.",
    	    "Dieses Symbol lässt sich mit etwas gutem "
    		"Willen als ein Leuchtturm erkennen.",
    	    "Dieses Symbol zeigt drei unterschiedlich große, in einander "
    		"liegende Ringe.",
    	    "Dieses Symbol identifizierst Du als einen Schwimmring.",
    	    "Dieses Symbol zeigt einen eiförmigen Gegenstand, allerdings "
    		"ist der Rand nicht glatt wie bei einem "
    		"Ei sondern stark gewellt und gezackt.",
    	    "Dieses Symbol könnte eine Haifischflosse darstellen.",
    	    "Dieses Symbol zeigt ein Fast-Quadrat; eine "
    		"der vier Ecken ist nämlich abgerundet.",
    	    "Dieses Symbol scheint einen Seestern darzustellen.",
    	    "Dieses Symbol ist ein ungleichseitiges Dreieck "
    		"mit einem eingeschlossenen Kreis.",
    	    "Dieses Symbol sieht aus wie eine Gangway."
    	});
    	add_v_item (([
    	    "name":"skalen","gender":"weiblich","plural":1,
    	    "long":"Zwei Skalen sind in dem Messinggehäuse angebracht. "
    		"Jede der beiden Nadeln zeigt auf eine andere. Die äußere "
    		"Skala in grüner Farbe zeigt Seemeilen, die innere Skala "
    		"in roter Farbe zeigt Gradangaben."
    	]));
    	add_v_item (([
    	    "name":"skala","gender":"weiblich","adjektiv":({"äußer"}),
    	    "long":"Die äußere Skala in grüner Farbe zeigt Seemeilen."
    	]));
    	add_v_item (([
    	    "name":"skala","gender":"weiblich","adjektiv":({"inner"}),
    	    "long":"Die innere Skala in roter Farbe zeigt Gradangaben."
    	]));
    	add_v_item (([
    	    "name":"deckel","gender":"maennlich",
    	    "id":({"deckel","glas"}),"adjektiv":"gläsern",
    	    "long":"Das Glas des Deckels ist schon etwas milchig geworden."
    	]));
    	add_v_item (([
    	    "name":"symbole","gender":"saechlich","plural":1,
    	    "id":({"symbole"}),
    	    "long":"Du siehst einige magische und einige seemännische "
    		"Symbole, zehn Stück, um genau zu sein. "
    		"Schau sie Dir doch ruhig einzeln der Reihe nach an."
    	]));
    	for (i = 0; i < sizeof (symbollongs); i++)
    	    add_v_item (([
    		"name":"symbol","gender":"saechlich",
    		"id":({"symbol"}),
    		"long":symbollongs[i]
    	    ]));
    	add_v_item (([
    	    "name":"messinggehäuse","gender":"saechlich",
    	    "id":({"gehaeusse","messinggehaeusse","gehäuse",
                   "messinggehäuse"}),
    	    "long":"Das Messinggehäuse ist mit Symbolen verziert; "
    		"es hat schon einige Kratzer abbekommen.",
    	    "feel":"Du spürst einige Kratzer im einst glatten "
    		"Messinggehäuse."
    	]));
    	add_v_item (([
    	    "name":"kratzer","gender":"maennlich","plural":1,
    	    "long":"Langweilige Kratzer, wie sie entstehen, wenn man "
    		"einen Gegenstand nicht ständig in Watte einhüllt.",
    	    "smell":"Kaum zu glauben, Du riechst an den Kratzern. "
    		"Aber Deine Mühe wird belohnt: Du riechst den "
    		"Geruch des großen, weiten Magyras.",
    	    "feel":"Die Kratzer fühlen sich an dünne, lange, nach innen "
    		"gerichtete Unregelmäßigkeiten im Messinggehäuse."
    	]));
    	add_v_item (([
    	    "name":"gravur","gender":"weiblich",
    	    "id":({"gravur","buchstaben"}),
    	    "long":"Auf dem Boden des Messinggehäuses sind einige "
    		"recht kleine Buchstaben, nicht allzu leicht zu lesen, "
    		"eingraviert. Was sie wohl bedeuten mögen, wenn man "
    		"sie liest?",
    	    "read":"Die Gravur ist schwer zu lesen; "
    		"mit viel Mühe entzifferst Du:\n"
    		"Rote Nadel   -> Richtung Tadmor\n"
    		"Grüne Nadel -> Entfernung Tadmor\n"
    	]));
    }
}
