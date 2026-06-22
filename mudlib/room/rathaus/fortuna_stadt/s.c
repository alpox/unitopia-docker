inherit "/i/room";

void create()
{
    ::create();
    set_short("Die mondhelle Tramhaltestelle");
    set_long(
"Eine verlassene Tramhaltestelle steht hier, obwohl keine Schienen zu\n"
"sehen sind. Der Fahrplan zeigt Ziele, die es in keiner Stadtkarte gibt:\n"
"Rueckseite des Mondes, Letzte Haltestelle, Unteres Gewissen.\n"
"Eine Bank aus dunklem Holz wartet unter einer Lampe, deren Licht wie\n"
"fluessiges Silber auf den Boden tropft.\n"
    );
    set_room_domain("Pantheon");

    add_exit("/room/rathaus/fortuna_stadt/zentrum", "norden");
    add_exit("/room/rathaus/fortuna_stadt/sw", "westen");
    add_exit("/room/rathaus/fortuna_stadt/se", "osten");
}
