// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/secure/simul_efun/simul_efun.c
// Description: simul_efun enthaelt Funktionen, die wie echte
//              efuns des Drivers von jedem Objekt aus 
//              direkt aufgerufen werden koennen.
//              simul_efun wird vom Master aus geladen.

#pragma no_inherit
#pragma strong_types
#pragma save_types

#include "/secure/simul_efun.h"

#if __FILE__ == SPARE_SIMUL_EFUN
   inherit "/secure/simul_efun/backup/deklin";
   inherit "/secure/simul_efun/backup/parse_com";
#else
   inherit "/secure/simul_efun/deklin";
   inherit "/secure/simul_efun/parse_com";
#endif

#include "/sys/config.h"
#include "/sys/time.h"
#include "/sys/touch.h"
#include "/sys/parse_com.h"
#include "/sys/math.h"
#include "/sys/invis.h"
#include "/sys/lpctypes.h"
#include "/sys/files.h"
#include "/sys/level.h"
#include "/sys/error.h"
#include "/sys/regexp.h"
#include "/sys/simul_efuns.h"
#include "/sys/apps.h"
#include "/sys/portal.h"
#include "/sys/debug_info.h"
#include "/sys/configuration.h"
#include "/sys/driver_info.h"
#include "/sys/object_info.h"
#include "/sys/interactive_info.h"
#include "/sys/comm.h"

/* --- Globale Variablen: --- */
// Die Variablen, die im Driver-Space gespeichert werden.
private static mapping global_info;

/* --- Initialisierung: --- */
protected void living_create();
protected void living_reset();
protected void string_create();
protected void clone_object_create();
protected void clone_object_reset();

protected void reset()
{
    set_next_reset(60);

    // Module resetten:
    living_reset();
    clone_object_reset();
}

protected void create()
{
    if (!(global_info = get_extra_wizinfo(0)))
	set_extra_wizinfo(0, global_info = m_allocate(0, 1));

    // Module initialisieren:
    string_create();
    living_create();
    clone_object_create();

    reset();
}

/* --- Module: --- */
#include "compat.inc"
#include "living.inc"
#include "log.inc"
#include "string.inc"
#include "interactive.inc"
#include "deprecated.inc"
#include "util.inc"
#include "convert.inc"
#include "time.inc"
#include "math.inc"
#include "map.inc"
#include "comm.inc"
#include "secure.inc"
#include "search_object.inc"
#include "paths.inc"
#include "input_to.inc"
#include "clone_object.inc"
#include "call_out.inc"
#include "notify_fail.inc"
