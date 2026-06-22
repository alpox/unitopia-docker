// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	    /room/rathaus/obj/knochenmesser.c
// Description: Teil des Startersets für kleine Spieler, welches sie 
//              nach Armageddon zugesteckt bekommen.
        
inherit "/i/weapon/nahkampf_waffe";

//--------------------------------------------------------------------
void create()
{
    "*"::create();
    
    init_weapon("messer", 80, 60, 100);

    set_name("knochenmesser");
    set_gender("saechlich");
    set_id( ({ "knochenmesser", "messer" }) );
    set_adjektiv( ({ "handlich" }) );

    set_long("Aus einem Knochen wurde ein handliches Messer "
        "geschnitzt. Die Klinge wurde glatt poliert, der Griff ist "
        "mit roter Schnur umwickelt.");
    set_feel("Du Knochen fühlt sich glatt und hart wie Stahl an.");

    set_material("bein");

    set_broken_message("KNACK!!! Das Knochenmesser ist zerbrochen.");
    set_broken_adjektiv("zerbrochen");

    add_v_item(([
    "name":     "klinge",
    "gender":   "weiblich",
    "id":       ({"messerklinge","klinge","schneide" }),
    "adjektiv": ({"glattpoliert" }),
    "long":     "Die knöcherne Klinge glänzt richtig, so glatt ist "
                "sie poliert. Und sie sieht auch recht scharf aus.",
    "feel":     "Vorsichtig, sie ist wirklich scharf.",
    "noise":    "Du kannst nichts hören. Aber beinahe hättest du dir "
                "dein Ohr abgeschnitten.",
    "hear_msg": "$Der(OBJ_TP) hält sich $seinen(OBJ_TO) ans Ohr. "
                "$Er(OBJ_TP) wird sich doch nicht ein Ohr "
                "abschneiden wollen?",
    ]));

    add_v_item(([
    "name":     "griff",
    "gender":   "weiblich",
    "id":       ({"messergriff","griff","schnur" }),
    "long":     "Der Messergriff ist komplett mit roter Schnur "
                "umwickelt. Dadurch ist er besonders griffig.",
    "feel":     "Das fühlt sich sehr griffig an.",
    ]));
}

//--------------------------------------------------------------------
int query_sellable()
{
    return 0;
}

//--------------------------------------------------------------------
int query_repairable()
{
    return 0;
}
