inherit "/i/room";

void create()
{
    ::create();
    set_short("Der Spiegelhof");
    set_long(
"Ein kleiner Hof ist vollgestellt mit mannshohen Spiegeln, deren Rahmen\n"
"aus Messing und Knochen bestehen. In einigen Spiegeln siehst du die\n"
"Stadt bei Nacht, in anderen bei Regen, in einem einzigen bei hellem\n"
"Morgenlicht. Ein Brunnen ohne Wasser steht in der Mitte; stattdessen\n"
"faellt darin leise Sternenstaub in die Tiefe.\n"
    );
    set_room_domain("Pantheon");

    add_exit("/room/rathaus/fortuna_stadt/n", "westen");
    add_exit("/room/rathaus/fortuna_stadt/e", "sueden");
}
