inherit "/i/room";

void create()
{
    ::create();
    set_short("Die Schattenarkade");
    set_long(
"Unter einer langen Arkade stehen Laeden, die nur dann geoeffnet sind,\n"
"wenn man nicht direkt hinsieht. Samtvorhaenge bewegen sich ohne Wind,\n"
"und aus einer Tuer riecht es nach Kaffee, Regen und altem Papier.\n"
"Die Schatten der Passanten bleiben manchmal einen Herzschlag laenger\n"
"stehen als ihre Besitzer.\n"
    );
    set_room_domain("Pantheon");

    add_exit("/room/rathaus/fortuna_stadt/nw", "norden");
    add_exit("/room/rathaus/fortuna_stadt/zentrum", "osten");
    add_exit("/room/rathaus/fortuna_stadt/sw", "sueden");
}
