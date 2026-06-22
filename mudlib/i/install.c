// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/i/install.c
// Description: Fuer Objekte, die garantiert nie bewegt werden sollen,
//              nachdem sie ein Environment bekommen haben.
// Modified by:	Garthan (10.01.94) set_no_move ueberarbeitet

// entweder /i/move ODER /i/install , niemals beides.
//

#pragma save_types

inherit "/i/move";

#include <install.h>
