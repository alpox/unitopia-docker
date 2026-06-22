// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/secure/simul_efun/compat.inc
// Description: Funktionen fuer Kompatibilitaet zu alten Drivern
// Author:	Freaky (09.10.1999)

#pragma strict_types
#pragma save_types

#if  __VERSION__ < "3.5.0.5-U038"
mixed * transpose_array(mixed *arr)
{
    object to = this_object();
    mixed* res;

    set_this_object(previous_object());
    res = efun::transpose_array(efun::copy(arr));
    set_this_object(to);

    return res;
}
#endif
