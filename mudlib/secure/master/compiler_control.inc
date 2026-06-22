// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/secure/master/compiler_control.inc
// Description: Funktionen die den LPC-Compiler betreffen.
// Modified by:	Freaky (16.06.1998) rename_allowed eingebaut
//		Freaky (05.02.1999) rename_allowed: check auf environment()
//			und bessere Fehlermeldung (reason)
//			privi-violation: rename_object: /room/serv raus
//			privi-violation: command: alles noetige eingebaut

#pragma strict_types

/*
 * Give a path to a simul_efun file. Observe that it is a string returned,
 * not an object. But the object has to be loaded here. Return 0 if this
 * feature isn't wanted.
 */
string get_simul_efun()
{
    string err;

    if (!loading_simul_efun)
    {
	loading_simul_efun = 1;
	if (err = catch(load_object(SIMUL_EFUN)))
	{
	    loading_simul_efun = 0;
	    debug_message("Failed to load Simul-Efun " + SIMUL_EFUN +
		    " " + err, DMSG_STDOUT | DMSG_LOGFILE);
	}
	else
	{
	    loading_simul_efun = 0;
	    return SIMUL_EFUN;
	}
    }
    if (err = catch(load_object(SPARE_SIMUL_EFUN)))
    {
	debug_message("Failed to load Spare-Simul-Efun " + SPARE_SIMUL_EFUN +
		" " + err, DMSG_STDOUT | DMSG_LOGFILE);
	efun::shutdown();
	return 0;
    }
    return SPARE_SIMUL_EFUN;
}

/*
 * Diese Funktion wird von printf("%O") aufgerufen, wenn die Variable ein
 * objekt enthaelt.
 * Seit 3.2.6-dev63 gibt es eine efun object_name. Deswegen wurde das hier
 * umbenannt nach printf_obj_name.
 */
string printf_obj_name(object ob)
{
    string tmp;

    tmp = ({string})ob->query_real_name();
    if (stringp(tmp) && tmp != "")
	return tmp;
    tmp = ({string})ob->query_name();
    if (stringp(tmp) && tmp != "")
	return tmp;
    return "UID:" + getuid(ob);
}

void dangling_lfun_closure()
{
    set_this_object(previous_object());
    raise_error("dangling lfun closure\n");
}

int valid_file_name(string name)
{
    return !sizeof(lower_case(name) -
	    "abcdefghijklmnopqrstuvwxyz1234567890/-_.@ ");
}

// Hiermit kann man ueberpruefen lassen, ob ein Objekt mit der Funktion
// rename_object umbenannt werden darf.
// Wenn flag != 0 ist, wird das Objekt zerstoert, und eine Fehlermeldung
// erzeugt, wenn dies der Fall ist.
#define ALLOWED_RENAME_OBS ({ "d", "obj", "p", "w", "z","room","tests" })
int rename_allowed(object ob, string to_name, int flag)
{
    string *path, file, reason;

    file = object_name(ob);
    path = explode(file,"/");
    // Check, ob es ein Clone ist.
    if (sscanf(path[<1],"%~s#%~d") == 2)
    {
	// Check, ob das Objekt aus einem legalen Verzeichnis kommt
	if (member(ALLOWED_RENAME_OBS,path[1]) >= 0)
	{
	    // Check, ob das Objekt aus .../obj/... kommt
	    if (member(path,"obj") >= 0)
	    {
		if (!environment(ob))
		{
		    if (valid_file_name(to_name))
		    {
		        // In /room duerfen die Objekte nur
		        // dort verwendet werden.
		        if (path[1] == "room" &&
		            strstr(file, implode(explode(to_name, "/")[..<2], "/")))
		            reason = "Not allowed to rename this object from /room.";
		        else
		            return 1;
		    }
		    else
		    {
			reason = "Can't rename to an invalid filename (" +
			    to_name + ")";
		    }
		}
		else
		{
		    reason = "Object has an environment";
		}
	    }
	    else
	    {
		reason = "Object does not resides in a '/obj/'-dir";
	    }
	}
	else
	{
	    reason = "Not allowed to rename this Object";
	}
    }
    else
    {
	reason = "Object is no clone";
    }

    // Objekt darf nicht umbenannt werden
    write_file("/log/sys/PRIVI","Illegal use of rename_object: " + file +
	    " PO:" + object_name(previous_object()) + " " + reason + ".\n");
    if (flag)
    {
	// wenn flag != 0 ist, dann hier Objekt zerstoeren und
	// Raise-error aufrufen
	ob->remove();
	if (ob)
	    destruct(ob);
	raise_error("Illegal use of rename_object by a virtual_compiler: " +
		"Illegal to rename object '" + file + "': " + reason + ".\n");
    }
    return 0;
}

int last_compiled_objects_time;
mapping last_compiled_objects = ([:0]);  // Ein Mapping mit den Namen aller zu ladenden Dateien, um Rekursionen zu finden.

object compile_object(string str)
{
    object ob;
    mapping lco; // Referenz auf last_compiled_objects wegen set_this_object.
    string *path, dir;
    string|object vc;
    object to = this_object();

#if 1
    if ((object_name(previous_object()) + ".c") != SIMUL_EFUN)
	if (find_player("freaky"))
	    tell_object(find_player("freaky"),sprintf("$$ CO:%O: TP:%O PO:%O\n",
			str,this_player(),previous_object()));
#endif

    // Filenamen anpassen
    str = str[0..<3];
    if (str[0] != '/')
	str = "/" + str;

    if(last_compiled_objects_time == time() &&
       member(last_compiled_objects, str))
    {
        do_log_error("Rekursiver Aufruf von compile_object() "
                     "mit gleichem Dateinamen: " + str + "\n",
                     __FILE__,
                     object_name(
                         ((object_name(previous_object()) + ".c") != SIMUL_EFUN)?
                         previous_object(1):previous_object()),
                     __LINE__, 0, 1);
    }
    else
    {
        if(last_compiled_objects_time != time())
        {
            last_compiled_objects_time = time();
            last_compiled_objects = ([str]);
        }
        else
            m_add(last_compiled_objects, str);
    }
    lco = last_compiled_objects;

    path = explode(str,"/");
    if (sizeof(path) < 3)
    {
        // Keine virtuellen Objekte auf Root-Ebene
        m_delete(lco, str);
        return 0;
    }

    // MAP-Raeume.
    // Achtung, das ist leicht rekursiv. Daher bei /map/m?_? testen,
    // ob dort nicht die Datei zum Laden rumliegt.
    if ((path[1] == "map" && file_size(str + ".c") < 0) &&
        sscanf(path[<1],"m%~d_%~d") == 2)
    {
        set_this_object(previous_object());
        ob = ({object})MAP_OB->create_map_object(str);
        set_this_object(to);
        m_delete(lco, str);
        return ob;
    }

    // Virtual Compiler
    dir = implode(path[0..<2],"/");
    vc = dir + "/virtual_compiler.c";
    vc = find_object(vc) || (file_size(vc) > 0 ? vc : 0);

    // VC nur laden, wenn er in einem ladbaren Verzeichnis liegt
    if(stringp(vc) && no_load_file(vc))
        vc = 0;

    if (vc)
    {
        ob = funcall(function object() : object po = previous_object()
        {
            set_this_object(po);

            if(stringp(vc))
                catch(vc = load_object(vc); publish);

            if(objectp(vc))
                return ({object})vc->virtual_compiler(path[<1], dir + "/");
        });

        if (ob && !objectp(ob))
        {
            do_log_error("virtual_compiler lieferte weder 0 noch ein Objekt zur\u00fcck.\n",
                dir + "/virtual_compiler.c", objectp(vc)?object_name(vc):vc, 0, 0, 0);
            m_delete(lco, str);
            return 0;
        }
    }

    if (!ob || object_name(ob) == str)
    {
        m_delete(lco, str);
        return ob;
    }

    if (rename_allowed(ob,str,1))
    {
        string old_name = object_name(ob);
        rename_object(ob,str);
        if(vc)
            vc->setup_renamed_object(ob, old_name);
        if(ob)
            ob->setup_renamed_object(stringp(vc)?find_object(vc):vc, old_name); 
        m_delete(lco, str);
        return ob;
    }
    m_delete(lco, str);
    return 0;
}

/*
 * privilege_violation is called when objects try to do illegal things,
 * or files being compiled request a privileged efun.
 *
 * return values: 
 *   1: The caller/file is allowed to use the privilege.
 *   0: The caller was probably misleaded; try to fix the error.
 *  -1: A real privilege violation. Handle it as error.
 */
#define PROFILE_PRIVI
#ifdef PROFILE_PRIVI
private static mapping profile_privi;
mapping query_profile_privi() { return copy(profile_privi); }
#endif

mapping admin_dirs;

int privilege_violation(string what, mixed who, mixed where, mixed how, mixed data)
{
#if 0
    if (find_player("menaures"))
	tell_object(find_player("menaures"),
		sprintf("WHAT:%O:\nWHO:%O:\nWHERE:%O:\nHOW:%O\n",what,who,where,how));
#endif


#ifdef UNItopia
#   define FTOOL "w/freaky/obj/ftool"
#   define TERMINAL_1993 "d/Campus/obj/1993/connect"
#else
#   ifdef NO_PRIVILEGE_VIOLATIONS
	return 1;
#   endif
#endif

#ifdef PROFILE_PRIVI
    if (!profile_privi)
	profile_privi = ([]);
    profile_privi[what]++;
#endif
    switch(what)
    {
      case "configure_driver":
        if (load_name(who) == PORTAL_CONNECTION && where == DC_TLS_CERTIFICATE)
           return 1;
        break;

      case "configure_object":
        switch (how)
        {
            case OC_EUID:
                if (playerp(who) && geteuid(who) == data && geteuid(where) == 0)
                    return 1;
                break;
        }
        break;

      case "input_to":
	if (!strstr(object_name(who),"/obj/zauberstab#") ||
		!strstr(object_name(who),"/secure/obj/login#") ||
                object_name(who) == PLAYER_SECOND ||
                !strstr(object_name(who), PORTAL_ROOM_DIR) ||
#ifdef TERMINAL_1993
                !strstr(object_name(who), "/"TERMINAL_1993"#") ||
#endif
                playerp(who))
	    return 1;
	break;
      case "bind_lambda":
	if (object_name(who) == MAP_OB)
	    return 1;
#ifdef FTOOL
	if (object_name(who) == "/" FTOOL)
	    return 1;
	if (object_name(who) == "/w/freaky/LPC_zst")
	    return 1;
	if (object_name(who) == "/w/gnomi/LPC_zst")
	    return 1;
	if (object_name(who) == "/w/sorcerer/LPC_zst")
	    return 1;
	if (object_name(who) == "/w/menaures/LPC_zst")
	    return 1;
	if (object_name(who) == "/w/myonara/LPC_zst")
	    return 1;
#endif
	break;
      case "rename_object":
	if (object_name(who) == MAP_OB)
	    return 1;
	break;
      case "send_udp":
	if (object_name(who) == INETD)
	    return 1;
	break;
      case "erq":
	if (object_name(who) == INTERNET_MAILD || playerp(who) ||
		object_name(who) == NEWS_INDEX
#ifdef UNItopia
                || load_name(who) == "/z/Spiele/Go/obj/gnugo"
		|| load_name(who) == "/w/gnomi/erqtest"
#endif
                )
	    return 1;
	break;
      case "limited":
	if (object_name(who) == ERROR_DB)
	    return 1;
	if (object_name(who) == ERROR_ARCHIVE)
	    return 1;
	if (!strstr(object_name(who),LOGIN_OB "#"))
	    return 1;
	if (playerp(who))
	    return 1;
	if (object_name(who) == GOETTER_REGISTER)
	    return 1;
	if (object_name(who) == QUEST_ROOM)
	    return 1;
	if (object_name(who) == GILDEN_OB)
	    return 1;
	if (object_name(who) == "/apps/autoloader")
	    return 1;
	if (!strstr(object_name(who),"/obj/zauberstab#") ||
            !strstr(object_name(who),"/obj/wizard_shell#"))
	    return 1;
	if (object_name(who) == "/apps/plugin")
	    return 1;
	if (object_name(who) == HELP_TOOL)
	    return 1;
	if (object_name(who) == "/w/gnomi/LPC_zst")
	    return 1;
        break;
      case "call_out_info":
	if (!strstr(object_name(who),"/obj/zauberstab#"))
	    return 1;
	if (object_name(who) == "/apps/top")
	    return 1;
	break;
      case "nomask simul_efun":
	if (objectp(who))
	    break;
	switch (where)
	{
	  case "binary_message":
	    if (who == "secure/obj/login.c" ||
	        who == "secure/obj/protocol_tester.c" ||
	        who == "i/player/webmud.c" ||
	        who == "i/player/options.c" ||
	        who == "i/player/telnet_neg.c" ||
	        who == "i/player/gmcp.c")
		return 1;
	    break;
	  case "move_object":
	    if (who == "i/move.c" ||
		    who == "i/tools/update_actions.c" ||
		    who == "i/player/login.c")
		return 1;
	    break;
	  case "query_ip_number":
	  case "query_ip_name":
	  case "interactive_info":
	    if (who == "apps/town.c" ||
		    who == "apps/statistik.c" ||
		    who == "i/player/login.c" ||
		    who == "i/player/more.c" ||
		    who == "i/player/webmud.c" ||
		    who == "i/player/webmud3.c" ||
		    who == "i/player/vt100client.c" ||
		    who == "i/player/player.c" ||
		    who == "secure/obj/login.c" ||
		    who == "secure/i/sperre.c" ||
		    who == "secure/dbus/webmud2.c" ||
		    who == "secure/level_lister.c" ||
		    who == "secure/rpc/webmud.c" ||
		    who == "secure/portal/portal.c" ||
		    who == "w/sissi/priv/wahl/wurne.c")
		return 1;
	    break;
	  case "tell_object":
	    if (who == "i/player/player.c" ||
		    who == "obj/pplayer.c" ||
		    who == "secure/obj/protocol_tester.c" ||
		    who == LOGIN_OB[1..] + ".c" ||
		    who == "i/wizard_shell/wizard_shell.c" ||
		    who == "i/player/telnet_neg.c" ||
		    who == "i/player/vt100client.c" ||
		    who == PORTAL_CONNECTION[1..] + ".c"
#ifdef TERMINAL_1993
                 || who == TERMINAL_1993".c"
#endif
               )
		return 1;
	    break;
	  case "command":
	    if (who == "i/player/tippse.c" ||
		    who == "i/player/player.c" ||
		    who == "i/player/vt100client.c" ||
		    who == "i/player/intermud.c" ||
		    who == "i/wizard_shell/shell.inc" ||
		    who == "i/monster/monster.c")
		return 1;
	    break;
	  case "command_stack":
	    if (who == "i/player/player.c")
	        return 1;
	  case "garbage_collection":
	  case "shutdown":
	    if (who == "room/rathaus/div/shut.c"
#ifdef Orbit
            ||  who == "apps/make_error_log.c"
#endif
	    )
		return 1;

	    break;
	  case "debug_info":
	    if (who == "room/rathaus/div/shut.c" ||
	        who == "apps/object_db.c" ||
		    who == "p/Plugins/zdebuginfo.c" ||
		    who == "p/mammi/plugin/zdebuginfo.c" ||
#ifdef UNItopia
		    who == "w/sissi/workroom.c" ||
#endif
		    who == "i/player/wiz_soul.c" ||
		    who == "i/living/hands.c")
		return 1;
	    break;
	  case "object_info":
	    if (who == "p/mammi/plugin/zobjectinfo.c" ||
	        who == "apps/object_db.c" ||
	        who == "p/Plugins/zobjectinfo.c")
		return 1;
	    break;
	  case "functionlist":
	    if (who == "p/Plugins/zshowfunction.c")
		return 1;
	    if (who == "apps/make_error_log.c")
	        return 1;
	    break;
	  case "variable_list":
	    if (who == "i/player/intermud.c" ||
	        who == "apps/make_error_log.c" ||
	        who == "tests/tests.c")
	        return 1;
	    break;
	  case "query_actions":
	    if (who == "i/player/tippse.c")
		return 1;
	    break;
	    
	  case "set_this_player":
	    if (who == "obj/newsreader.c"
#ifdef TERMINAL_1993
                || who == TERMINAL_1993".c"
#endif
               )
		return 1;
	    break;
	}
	// Damit bei einem Compile Fehler man nicht diese Privis
	// auf dem Schirm bekommt, sondern die Compile Fehler.
	if (!strstr(who, "secure/master/") ||
	    !strstr(who, "secure/simul_efun/"))
		return 1;
	/*
        if (who == SIMUL_SECURE[1..] || who == SPARE_SIMUL_SECURE[1..])
	    return 1;
	if (who == "secure/simul_efun/call_out.inc")
	    return 1;
	*/
	if (who[0..12] == "i/zauberstab/")
	    return 1;
#ifdef FTOOL
	if (who == FTOOL ".c")
	    return 1;
	if (who == "w/freaky/LPC_zst.c")
	    return 1;
	if (who == "w/gnomi/LPC_zst.c")
	    return 1;
        if (who == "w/menaures/LPC_zst.c")
            return 1;
        if (who == "w/sorcerer/LPC_zst.c")
            return 1;
        if (who == "w/myonara/LPC_zst.c")
            return 1;
#endif
	break;

      case "garbage_collection":
      case "shutdown":
	if (object_name(who) == "/room/rathaus/div/shut"
#ifdef Orbit
         || object_name(who) == "/apps/make_error_log"
#endif
	)
	    return 1;
	break;

      case "set_extra_wizinfo":
      case "set_extra_wizinfo_size":
      case "get_extra_wizinfo":
	if (who == find_object(SIMUL_EFUN) ||
		who == find_object(SPARE_SIMUL_EFUN))
	    return 1;
	if (object_name(who) == "/secure/dbus/system")
	    return 1;
#ifdef FTOOL
	if (!strstr(object_name(who),"/" FTOOL "#"))
	    return 1;
	if (object_name(who) == "/w/freaky/LPC_zst")
	    return 1;
#endif
	break;
      case "wizlist_info":
	if (object_name(who) == "/apps/top")
	    return 1;
#ifdef FTOOL
	if (!strstr(object_name(who),"/" FTOOL "#"))
	    return 1;
	if (object_name(who) == "/w/freaky/LPC_zst")
	    return 1;
#endif
        if (object_name(who) == "/w/menaures/apps/top")
            return 1;
        break;

      case "variable_list":
#ifdef FTOOL
        if (object_name(who) == "/w/gnomi/LPC_zst")
	    return 1;
        if (object_name(who) == "/w/sorcerer/LPC_zst")
	    return 1;
        if (object_name(who) == "/w/myonara/LPC_zst")
	    return 1;
#endif
        if (object_name(who) == "/tests/tests")
            return 1;
	break;

      case "net_connect":
#ifdef UNItopia
        // Fuer das UNItopia 1993 Terminal:
        if(!strstr(object_name(who), "/"TERMINAL_1993"#")
           && where == "127.0.0.1" && how == 1993)
           return 1;
#endif
        if( load_name(who) == PORTAL_CONNECTION)
           return 1;
        break;
      case "enable_telnet":
        if(
#ifdef UNItopia
            !strstr(object_name(who), "/"TERMINAL_1993"#") ||
#endif
	    load_name(who) == PORTAL_CONNECTION)
           return 1;
        break;

      case "mysql":
        break;
	
      case "sqlite_pragma":
        switch(lower_case(where))
	{
	    case "table_info":
	    case "auto_vacuum":
	    case "count_changes":
	    case "case_sensitive_like":
	    case "data_version":
	    case "user_version":
	    case "foreign_keys":
		return 1;
	}
	    
#ifdef UNItopia
        if(!strstr(object_name(who), "/w/gnomi/"))
	   return 1;
#endif
	break;

      case "set_limits": // Neue Werte in 'where'
      case "attach_erq_demon":
      case "execute_command":
      case "set_auto_include_string":
      case "set_driver_hook":
      case "set_this_object":
      case "shadow_add_action":
      case "symbol_variable":
	break;

      case "git_commit":
        if (object_name(who) == "/apps/git")
            return 1;
        break;

      case "dbus_publish_object":
        if (object_name(who) == "/secure/dbus/dbus")
            return 1;
        break;

      case "dbus_call_method":
      case "dbus_register_signal_listener":
      case "dbus_unregister_signal_listener":
      case "dbus_emit_signal":
        if (!strstr(object_name(who), "/secure/dbus/"))
            return 1;
        break;

#if __EFUN_DEFINED__(trace_call)
      case "trace_call":
      {
        struct trace_call_options opts = ({struct trace_call_options}) where;

        if (!admin_dirs)
        {
            admin_dirs = mkmapping(filter(map(get_dir("/d/"), function string(string dom)
            {
                return "/d/" + dom + "/Admin/";
            }) + map(get_dir("/z/"), function string(string dom)
            {
                return "/z/" + dom + "/Admin/";
            }) + map(get_dir("/z/Gilden/"), function string(string dom)
            {
                return "/z/Gilden/" + dom + "/Admin/";
            }), function int(string dir)
            {
                return file_size(dir) == FSIZE_DIR;
            }));
            admin_dirs += ([ "/apps/", "/secure/" ]);
        }

        // Ein paar Mindesteinstellungen:
        opts.granularity = max(1, opts.granularity);
        if (!opts.exclude)
            opts.exclude = ([]);
        opts.exclude += admin_dirs;
        opts.variable_format_depth = min(3, opts.variable_format_depth);
        return 1;
      }
#endif
    }
    write_file("/log/sys/PRIVI",sprintf("what:%O: who:%O: where:%O:\n",
		what,who,where));
    return -1;
}
