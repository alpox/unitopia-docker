inherit "/i/room";

void create()
{
    ::create();
    set_short("Die Bibliothek der stillen Versprechen");
    set_long(
"Zwischen zwei hohen Haeusern liegt eine Bibliothek, deren Fassade aus\n"
"dunklem Glas besteht. Hinter den Scheiben schweben Buecher in langsamen\n"
"Kreisen, als wuerden sie auf Leser warten, die ihre eigenen Erinnerungen\n"
"als Pfand hinterlegen. Ueber der Tuer steht in goldenen Lettern:\n"
"Was du suchst, sucht laengst dich.\n"
    );
    set_room_domain("Pantheon");

    add_exit("/room/rathaus/fortuna_stadt/e", "norden");
    add_exit("/room/rathaus/fortuna_stadt/s", "westen");
}
