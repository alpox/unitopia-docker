// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/i/tools/message_parser.c
// Description: Umwandeln von Meldungsstrings in Closures
// Author:      Garthan (23.12.93)

#pragma save_types

// Diese Funktionen werden im Prinzip nur von /i/move.c
// und /i/room.c gebraucht, um die Bewegungsmeldungen 
// von beweglichen Objekten zu realisieren.

// string_parser() ist eine simul_efun aus /secure/deklin
closure message_parser(string str)
{
   return str && lambda(({'tp,'richtung }), string_parser(str,0,1));
}
