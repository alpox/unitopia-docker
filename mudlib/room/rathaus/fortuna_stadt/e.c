inherit "/i/room";

void create()
{
    ::create();
    set_short("Der Neonkanal");
    set_long(
"Ein schmaler Kanal zieht sich zwischen den Haeusern hindurch. Das Wasser\n"
"ist schwarz, doch unter seiner Oberflaeche treiben helle Zeichen wie\n"
"versunkene Reklametafeln. Eine Bruecke aus gruen angelaufenem Kupfer\n"
"fuehrt ueber den Kanal. Auf dem Gelaender sitzen steinerne Kraehen und\n"
"tun so, als waeren sie keine Zeugen.\n"
    );
    set_room_domain("Pantheon");

    add_exit("/room/rathaus/fortuna_stadt/ne", "norden");
    add_exit("/room/rathaus/fortuna_stadt/zentrum", "westen");
    add_exit("/room/rathaus/fortuna_stadt/se", "sueden");
}
