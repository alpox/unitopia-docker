inherit "/i/room";

void create()
{
    ::create();
    set_short("Das Nordtor der verborgenen Stadt");
    set_long(
"Hier oeffnet sich ein Torbogen aus schwarzem Stein, in dessen Adern\n"
"goldenes Licht pulsiert. Hinter dir liegt das Rathaus, vor dir breitet\n"
"sich eine Stadt aus, die sich nicht recht entscheiden kann, ob sie\n"
"wirklich existiert oder nur im Spiegelbild einer nassen Strasse wohnt.\n"
"Laternen schweben ueber dem Platz, und jede von ihnen flackert in einer\n"
"anderen Farbe.\n"
    );
    set_room_domain("Pantheon");

    add_exit("/room/rathaus/treppe", "norden");
    add_exit("/room/rathaus/fortuna_stadt/nw", "westen");
    add_exit("/room/rathaus/fortuna_stadt/ne", "osten");
    add_exit("/room/rathaus/fortuna_stadt/zentrum", "sueden");
}
