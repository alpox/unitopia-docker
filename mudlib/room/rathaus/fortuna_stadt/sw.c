inherit "/i/room";

void create()
{
    ::create();

    set_short("Der Hof der verlorenen Namen");
    set_long(
"Der Hof ist klein, aber unmoeglich still. An den Mauern haengen\n"
"unzaehlige Messingschilder, auf denen Namen eingeritzt sind, die niemand\n"
"mehr laut aussprechen sollte. Ein alter Kastanienbaum waechst aus einer\n"
"zerbrochenen Statue und traegt Blaetter aus duennem Pergament.\n");

    set_room_domain("Pantheon");

    add_exit("/room/rathaus/fortuna_stadt/w", "norden");
    add_exit("/room/rathaus/fortuna_stadt/s", "osten");
}