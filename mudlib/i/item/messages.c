// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/i/item/messages.c
// Description: Umwandeln von Formatstrings in Closures
// Author:      Garthan (23.12.93)
// Modified by: Freaky	(23.12.93)
// Modified by: Garthan	(10.01.94)

// Dokumentation siehe: /doc/funktionsweisen/messages
//

#pragma save_types
#pragma strong_types

#include <apps.h>

/*
FUNKTION: closure_to_string
DEKLARATION: varargs string closure_to_string(closure c, mixed *args)
BESCHREIBUNG:
Dokumentation siehe: /doc/funktionsweisen/pseudoclosures
VERWEISE: mixed_to_closure, string_parser
GRUPPEN: grundlegendes
*/
varargs string closure_to_string(closure c, mixed *args)
{
   return apply(CLOSURE_CONTAINER->do_bind(c), args?args:({}));
}

// Restliche Funktionen in OBJ:/secure/simul_efun,
//			   FILE:/secure/simul_efun/deklin.c
// (mixed_to_closure, string_parser)
