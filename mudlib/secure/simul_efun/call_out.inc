// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/secure/simul_efun/call_out.inc
// Description:	call_out Mechanismus
// Author:	Sissi (22.04.2001)
//              Eingeflossen sind auch Ideen aus MorgenGrauen

#pragma save_types
#pragma strict_types

#include <driver_info.h>
#include <config.h>
#include <rtlimits.h>

#ifdef USE_CALL_OUT_SIMUL_EFUN

#define LIMIT_FOR_DESTRUCT 500000
#define MAX_CALLOUT_DELAY 2147483647 // Bug im Driver

object bad_object = 0;
string bad_clone = 0;
string bad_uid = 0;

#define CALL_OUT_SYS_LOG_HEADER(x) \
    "----------\n"+timestr(time())+"\n"+x+"\n"

private void __call_out (varargs mixed args)
{
    mixed I, *coi;
    object *objs, *clones, *sameuid;
    string name, uid, stemp;
    int count, temp;

#ifdef __LPC_LWOBJECTS__
    // Kein call_out() fuer LWOs, denn die sind schwer wieder einzufangen.
    object|lwobject calling = previous_object(3);
    if (!objectp(calling))
        raise_error("call_out() from a lightweight object.\n");
    if (closurep(args[0]) && !to_object(args[0]))
        raise_error("Illegal closure for call_out().\n");
#else
    object calling = previous_object(3);
#endif

    // Anzahl aktiver call outs holen, debug info liefert leider
    // nicht das gewuenschte (zumindest nicht in der aktuellen
    // Treiberversion)

    if (bad_object && (calling == bad_object)) {
	limited (#'efun::destruct,({LIMIT_FOR_DESTRUCT}),calling);
	return;
    }

    if (bad_clone && (load_name (calling) == bad_clone)) {
	limited (#'efun::destruct,({LIMIT_FOR_DESTRUCT}),calling);
	objs = map (call_out_info (), (: $1[0] || to_object($1[1]) :));
	clones = filter (objs, (: load_name ($1) == $2 :), bad_clone);
	limited (#'efun::map, ({LIMIT_FOR_DESTRUCT}), clones, #'efun::destruct);
	bad_clone = 0;
	return;
    }

    if (bad_uid && (getuid (calling) == bad_uid)) {
	limited (#'efun::destruct,({LIMIT_FOR_DESTRUCT}),calling);
	objs = map (call_out_info (), (: $1[0] || to_object($1[1]) :));
	sameuid = filter (objs, (: getuid ($1) == $2 :), bad_uid);
	limited (#'efun::map, ({LIMIT_FOR_DESTRUCT}), sameuid, #'efun::destruct);
	bad_uid = 0;
	return;
    }

//    count = sizeof (coi = call_out_info ());

#if __EFUN_DEFINED__(driver_info)
    count = efun::driver_info(DI_NUM_CALLOUTS);
#else
    count = efun::debug_info (DINFO_DATA, DID_STATUS,
       DID_ST_CALLOUTS);
#endif

    if (count > CALLOUT_LIMIT_ONE) {
        coi = call_out_info ();
	objs = map (coi, (: $1[0] || to_object($1[1]) :));
	if ((temp = sizeof (filter (objs, #'==, calling)))
	    > MAX_CALLOUTS_FOR_ONE_OBJECT) {
	    name = object_name (bad_object = calling);
	    __MASTER_OBJECT__->do_log_error(
		stemp = sprintf ("Mehr als %d call outs für das "
                         "Objekt\n%s\naktiv, "
			 "nämlich %d.\nAktueller Aufruf: %s\n",
		    MAX_CALLOUTS_FOR_ONE_OBJECT,
		    name,
		    temp,
		    mixed2str(args[0])),
		name, name, 0);
	    sys_log ("CALL_OUT",CALL_OUT_SYS_LOG_HEADER(stemp));
	    limited (#'efun::destruct,({LIMIT_FOR_DESTRUCT}),calling);
	    return;
	}
	if (count > CALLOUT_LIMIT_TWO) {
	    name = load_name (calling);
	    clones = filter (objs, (: load_name ($1) == $2 :), name);
	    if (sizeof (clones) > MAX_CALLOUTS_FOR_ALL_CLONES) {
		bad_clone = name;
		__MASTER_OBJECT__->do_log_error (
		    stemp = sprintf ("Mehr als %d call outs, nämlich %d, aktiv "
			     "für alle Clones\nvon: %s,\n"
			     "aktueller Aufruf war für: %s\n",
			MAX_CALLOUTS_FOR_ALL_CLONES,
			sizeof (clones),
			name,
			mixed2str(args[0])),
		    name, name, 0);
		sys_log ("CALL_OUT",CALL_OUT_SYS_LOG_HEADER(stemp));
		limited (#'efun::map, ({LIMIT_FOR_DESTRUCT}), clones, #'efun::destruct);
		bad_clone = 0;
		return;
	    }
	}
        if (count > CALLOUT_LIMIT_THREE) {
	    uid = getuid (calling);
	    sameuid = filter (objs, (: getuid ($1) == $2 :), uid);
	    if (sizeof (sameuid) > MAX_CALLOUTS_PER_UID) {
		bad_uid = uid;
		name = load_name (calling);
		__MASTER_OBJECT__->do_log_error (
		    stemp = sprintf ("Mehr als %d call outs, nämlich %d, aktiv "
			     "für Objekte mit\nuid: %s,\n"
			     "aktueller Aufruf war für: %s\n",
			MAX_CALLOUTS_PER_UID,
			sizeof (sameuid),
			uid,
			mixed2str(args[0])),
		    name, name, 0);
		sys_log ("CALL_OUT",CALL_OUT_SYS_LOG_HEADER(stemp));
		sameuid = filter (objs, (: getuid ($1) == $2 :), uid);
		limited (#'efun::map, ({LIMIT_FOR_DESTRUCT}), sameuid, #'efun::destruct);
		bad_uid = 0;
		return;
	    }
	}
    }

#ifdef MAX_CALLOUT_DELAY
    if(sizeof(args)>1 && intp(args[1]) && args[1] > MAX_CALLOUT_DELAY)
        args[1] = MAX_CALLOUT_DELAY;
#endif

    I = this_object ();
    set_this_object (calling);
    apply (#'efun::call_out, args);
    set_this_object (I);

}

nomask void call_out (varargs mixed args)
{
    // Ueber LIMIT_UNLIMITED springen, damit die
    // Testevals nicht dem Aufrufer angelastet werden.
    limited(#'limited, ({ LIMIT_UNLIMITED}),
	#'efun::apply, ({500000}), #'__call_out, args);
}

#endif
