inherit "/i/monster/monster";

void create()
{
    monster::create();

    clone_object("/obj/soul")->move(this_object());

    initialize("mensch", 40);

    set_name("kadmos");
    set_npc_name("kadmos");
    set_id(({
        "kadmos", "dummy", "trainingsdummy", "uebungsdummy",
        "schattendummy", "simulacrum"
    }));

    set_short("Kadmos, der sprechende Trainingsdummy");

    set_long(
"Kadmos sieht aus wie ein Mann aus dunklem Rauch, Messing und altem Leder.\n"
"In seiner Brust glimmt ein kleiner blauer Stern, der bei jedem Treffer\n"
"kurz heller aufleuchtet. Er steht vollkommen ruhig da, als sei er eigens\n"
"dafuer erschaffen worden, Schlaege, Zauber und schlechte Laune zu ertragen.\n"
"Er ist offensichtlich ein Trainingsdummy, aber einer mit Manieren.\n");

    set_personal(1);
    set_gender("maennlich");
    set_align(-300);
    give_hp(500);
    give_sp(100);
    give_hands(2);
    give_weapon_level(10);
    give_armour_level(2);

    set_aggressive(0);
    set_dead_ob("/room/rathaus/fortuna_stadt/obj/kadmos_dead");

    ::load_chat(8, ({
        "Schlag ruhig zu. Ich nehme es nur halb persoenlich.",
        "Training ist die Kunst, denselben Fehler eleganter zu wiederholen.",
        "Ich bin nicht unsterblich. Ich bin nur sehr gut verwaltet.",
        "Ich bin bereit, sobald du es bist."
    }));

    ::add_chats(({
        "Schlag ruhig zu. Ich nehme es nur halb persoenlich.",
        "Ich bin bereit, sobald du es bist."
    }));
}