inherit "/i/room";

void create()
{
    ::create();
    set_short("Die regennasse Maskengasse");
    set_long(
"Eine enge Gasse liegt hier wie ein dunkler Schnitt zwischen hohen,\n"
"schiefen Haeusern. Ueber den Fenstern haengen Masken aus Silber,\n"
"Glas und Schattenholz. Manche laecheln, obwohl niemand sie beruehrt.\n"
"Aus den Ritzen des Pflasters steigt blauer Dampf, und irgendwo spielt\n"
"eine unsichtbare Geige einen Tango, der einen Schritt zu langsam ist.\n"
    );
    set_room_domain("Pantheon");

    add_exit("/room/rathaus/fortuna_stadt/n", "osten");
    add_exit("/room/rathaus/fortuna_stadt/w", "sueden");
}
