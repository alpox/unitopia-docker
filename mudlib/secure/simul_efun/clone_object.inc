// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/secure/simul_efun/clone_object.inc
// Description:	Prüft, daß Clones eine Umgebung erhalten
// Author:	Menaures

/* --- Globale Variablen: --- */
mapping clones;

/* --- Initialisierung: --- */
protected void clone_object_create()
{
    clones = global_info["clones"] ||= ([:3]);
}

protected void clone_object_reset()
{
    /* Nicht referenzierte Objekte killen: */
    mixed type;
    mapping c = clones;

    clones = global_info["clones"] = ([:3]);

    foreach(object ob, string creator, int line, string trace : c)
    {
        // Ausnahmefall/Driverbug:
        if(!ob) continue;

        if(environment(ob) || query_shadowing(ob) || interactive(ob) ||
           !clonep(ob) || player_present(ob))
        {
            // OK.
            continue;
        }

        type = 1;
        catch(type = ({mixed})ob->query_type("nocleanup") ||
                     ({mixed})ob->query_type("repraesentant") ||
                     ({mixed})ob->query_type("umgebung"); publish);

        if(type)
        {
            // Entweder OK oder Fehler.
            // Bleibt manueller Ueberpruefung ueberlassen.
            continue;
        }

        if(time() - object_time(ob) < 120)
        {
            // Schonzeit 120 Sekunden (Verkaeufer):
            clones[ob,0] = c[ob,0];
            clones[ob,1] = c[ob,1];
            clones[ob,2] = c[ob,2];
            continue;
        }

        // Wir haben ein nicht referenziertes Objekt.
        do_warning2(sprintf("Clone hatte keine Umgebung und wurde daher "
                            "zerstört.\nTrace:\n%s", trace),
                    creator, object_name(ob), line);
        catch(ob->remove(); publish);
        catch(destruct(ob); publish);
    }
}

/* --- Simul-Efun: --- */

// Damit das set_this_object wieder vorbei ist nach dem clone_object
private object clone_object_set_this_object(string|object ob, object po, mixed* args)
{
    set_this_object(po);
    return efun::clone_object(ob, args...);
}


object clone_object(string|object toclone, varargs mixed* args)
{
    object clone;

    if (stringp(toclone))
    {
        string path;

        if(toclone[0]=='%')
        {
            path = search_file(toclone, 0, "obj", ".c");
            if (path)
                path = path[..<3];
        }
        else
            path = abs_path(toclone);

        if(!path)
        {
            set_this_object(previous_object());  
            raise_error("Invalid filename for clone_object(): '" + toclone + "'\n");
        }

        toclone = path;
    }

    clone = funcall(#'clone_object_set_this_object, toclone, previous_object(), args);
    if(clone)
    {
        clones[clone,0] = efun::load_name(previous_object());
#if __EFUN_DEFINED__(driver_info)
        clones[clone,1] = efun::driver_info(DI_TRACE_CURRENT)[<2][TRACE_LOC];
        clones[clone,2] = efun::driver_info(DI_TRACE_CURRENT_AS_STRING);
#else
        clones[clone,1] = efun::debug_info(DINFO_TRACE, DIT_CURRENT)[<2][TRACE_LOC];
        clones[clone,2] = efun::debug_info(DINFO_TRACE, DIT_STR_CURRENT);
#endif
    }

    return clone;
}
