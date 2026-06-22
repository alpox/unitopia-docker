// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/secure/master/destruct.inc
// Description: prepare_destruct wird vor einem destruct() aufgerufen
// Author:	Freaky
// Modified by:	Freaky (07.05.96) do_log_error und remove_shadow eingebaut
// 		Freaky (01.03.2000) an das neue add_encumbrance() angepasst

#pragma strict_types

#if __VERSION__ > "3.3.559"
#define CATCH(cmd) catch(cmd; reserve 5000)
#else
#define CATCH(cmd) ((get_eval_cost()>__CATCH_EVAL_COST__+7)?catch(cmd):"*Not enough eval time left for catch()\n")
#endif

private void destruct_error(object ob, object sh, varargs mixed *args)
{
    string msg;

    args[<1] = args[<1][1..];
    CATCH(msg = apply(#'sprintf,args));
    if (!msg)
	msg = implode(args," ");
    CATCH(do_log_error("FATAL: prepare_destruct(): " + msg,
		__FILE__,object_name(ob),
		0,explode(getuid(sh),":") + ({ getuid(sh) }),0));

    if(wizp(this_player()))
        write("FATAL: prepare_destruct(): " + msg +
	        "\nDieser Fehler MUSS sofort behoben werden!!!\n");
    else
    {
        write("Vor dir tut sich ein ganzer Riss im Raum-Zeit-Gef\u00fcge auf.\n"
              "Dieser Fehler MUSS sofort behoben werden!!!\n");
	debug_message("FATAL: prepare_destruct(): " + msg + "\n", DMSG_STDOUT | DMSG_LOGFILE | DMSG_STAMP);
#if __EFUN_DEFINED__(driver_info)
	debug_message(efun::driver_info(DI_TRACE_CURRENT_AS_STRING), DMSG_STDOUT | DMSG_LOGFILE | DMSG_STAMP);
#else
	debug_message(efun::debug_info(DINFO_TRACE,DIT_STR_CURRENT), DMSG_STDOUT | DMSG_LOGFILE | DMSG_STAMP);
#endif
    }
}

protected mixed prepare_destruct(object ob)
{
    object super, inv, sh, next;
    string tmp;

    // Wenn es kein Shadow ist, alle Shadows des Objektes unshadowen und
    // zerstoeren.
#if __EFUN_DEFINED__(query_shadowing)
    if (!query_shadowing(ob))
#else
    if (!efun::object_info(ob, OI_SHADOW_PREV))
#endif
    {
        if (tmp = CATCH(ob->notify("remove")))
	{
	    destruct_error(ob,ob,
		    "Fehler beim notify_remove() von:%O Fehler:%s",ob,tmp);
	}
	if (environment(ob) && tmp = CATCH(environment(ob)->notify("remove_inventory", ob)))
	{
	    destruct_error(ob,ob,
		    "Fehler beim notify_remove_inventory() von:%O Fehler:%s",ob,tmp);
	}
#if __EFUN_DEFINED__(query_shadowing)
	for (sh = shadow(ob, 0); sh; sh = next)
	{
	    next = shadow(sh, 0);
#else
	for (sh = efun::object_info(ob, OI_SHADOW_NEXT); sh; sh = next)
	{
	    next = efun::object_info(sh, OI_SHADOW_NEXT);
#endif
	    if (tmp = CATCH(sh->remove_shadow(sh)))
	    {
		destruct_error(sh,sh,
			"Fehler beim remove_shadow von:%O auf:%O Fehler:%s",
			sh,ob,tmp);
	    }
	    if (sh)
	    {
		funcall(bind_lambda(#'unshadow, sh)); // avoid deep recursion
		destruct(sh);
	    }
	}
    }
 
    super = environment(ob);
    if (!super)
    {
	if (object_name(ob) == "/room/void")
	    tmp = "/room/hell";
	else
	    tmp = "/room/void";
	while (ob && (inv = first_inventory(ob)))
#if __EFUN_DEFINED__(query_once_interactive)
	    if (query_once_interactive(inv))
#else
	    if (efun::object_info(inv, OI_ONCE_INTERACTIVE))
#endif
	    {
		tell_object(inv,
			    "Alles in deiner Umgebung l\u00f6st sich auf....\n");
		tell_room(tmp, funcall(symbol_function("wrap"),
                    funcall(symbol_function("Der"), inv) + " stolpert mit "
                    "einem \u00fcberraschten Gesichtsausdruck heran."));
		    
		inv->move(tmp,([MOVE_FLAGS:MOVE_MAGIC|MOVE_FORCE]));
	    }
	    else
	    {
		inv->remove();
		if (inv)
		    destruct(inv);
	    }
    }
    else
    {
	// Erst alles rausbewegen und dann den Rest zerstoeren.
	// (Hilft bei Objekten, die anderen Objekten folgen.)
	foreach(inv: all_inventory(ob))
	    if(inv && environment(inv)==ob &&
		tmp = CATCH(inv->move(super)))
	    {
		destruct_error(inv,sh,
			"Fehler beim MOVE von:%O nach:%O Fehler:%s",
			inv,super,tmp);
		if (inv)
		    destruct(inv);
	    }
	    
	while (ob && (inv = first_inventory(ob)))
	{
	    inv->remove();
	    if (inv)
	        destruct(inv);
	}

	if (tmp = CATCH(super->add_encumbrance(ob,ENC_REMOVE,0)))
	{
	    destruct_error(ob,sh,
		    "Fehler bei add_encumbrance von:%O in:%O Fehler:%s",
		    ob,super,tmp);
	}
    }

#if __VERSION__ < "3.6.3-U052"
    if (efun::interactive(ob))
    {
        // Flush buffer.
        funcall(bind_lambda(#'efun::binary_message, ob), b"", 0);
        configure_interactive(ob, IC_ENCODING, "UTF-8");
    }
#endif

    return 0; /* 0 for sucess */
}
