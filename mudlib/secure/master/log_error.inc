// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/secure/master/log_error.inc
// Description: Loggt Runtime-Errors mit
// Author:	Freaky	(23.12.93)
// Modified by: Garthan	(02.09.94) Generalueberholung
// 		Freaky	(23.12.94) System-Fehler nach /log/sys/...
//		Monty	(27.05.95) log_fehler_new fuer das neue Fehler-
//				   Handling.
//		Freaky  (12.09.95) komplettes rewrite fuer die ERROR_DB
//		Freaky  (23.09.95) query_debugger() angepasst
//		Freaky	(18.11.95) call-out bei runtime_error wenn keine evals
//		Freaky  (13.05.97) ERROR_DB bekommt jetzt direkt die Debugger
//              Sissi   (31.07.97) access.c und WRITE in /p/
//              Tucita  (26.11.97) /p Gouverneur als Debugger
//		Freaky  (17.12.97) query_debugger erweitert um <typ>:<name>
//				   fuer z.B. 'Gilden:Abenteurergilde'
//		Freaky  (21.01.98) Bugfix in get_debugfile_offset()
//              Copper  (24.06.98) Bugfix fuer /p fuer Tucita eingebaut
//              Sissi   (07.02.99) query_debugger an ACLs und Gruppen angepasst
//              Sissi   (05.04.99) ACLs fuer query_debugger freigegeben
//              Parsec  (06.04.99) Doppelte Root-Debugger bei 'fehler mich' raus
//              Parsec  (04.05.99) Bugfix in query_debugger_new
//                                 Dateien ohne .c gehen jetzt auch
//              Rompler (16.08.99) Typo/Lob eingefuehrt
//              Parsec  (10.02.00) V-Item Nummer int Fehler anzeigen
//		Freaky  (12.05.2000) add_slash() eingebaut
//              Sissi   (19.05.2000) query_program_info modifiziert, sodass
//                                   es den ACLs bei Player-Objekten (die
//                                   haben ne Sonderbehandlung) und bei
//                                   obj - Objekten ne Chance laesst,
//                                   wenn es extra ACLs gibt (und nur dann).

#pragma strict_types

// #define VERB(x) (x)?to_string(({string})(x)->query_command()):""
#define VERB(x) (query_command() || "")
#define FIRST_ROOM(x) ((x)&&find_object(x))?({string})find_object(x)->query_first_room():0

#define ROOT_DEB ({ROOT_UID})

#define DEF_MSG "Da hat sich wohl ein Loch im Raum-Zeit-Gef\u00fcge aufgetan.\n"
#define LINE "------------------------------------------------------------------------------\n"

#define REST_MASTER_EVAL 5000
// EV_LIMIT_ERROR muss groesser sein, als catch-reserved-cost
// Und wir packen noch etwas drauf, damit auch ein 2. catch
// funktioniert.
#define EV_LIMIT_ERROR 150000

// Die momentane Laenge des Debug-Files
static private int debugfile_offset;

// Diese Funktionen werden vom zerneuere-Befehl mit der Option -t/-T
// benoetigt. Der Master verwaltet die umbenannten Dateien und
// benennt sie bei Fehlern gleich wieder zurueck.
static string *objects_to_renew;
static object object_to_destruct;

#define BACKUP_OBJ_NAME(name) regreplace(name,"/([^/]*)$","/$\\1$", 0)

#if ! __EFUN_DEFINED__(reverse)
#define reverse(arr) sort_array((arr),(:1:))
#endif

private void abort_renewal(string err_ob)
{
    if(object_to_destruct)
        destruct(object_to_destruct);

    if(sizeof(objects_to_renew))
    {
	if(err_ob && member(objects_to_renew, err_ob)>=0)
	{
	    object ob = find_object(err_ob);
	    if(ob)
	    	destruct(ob);
	}
	    
	foreach(string name:objects_to_renew)
	{
	    object ob, nob;

	    if(sscanf(name,"%~s#%~d")<2)
	    {
		// Das alte (gesicherte) Objekt
		ob = find_object(BACKUP_OBJ_NAME(name));
		// Das neue Objekt
		nob = find_object(name);
	    }
	    else
		ob = find_object(name);

	    if(ob)
	    {
	        objects_to_renew -= ({name});
		if(nob)	// Das wurde geschafft
		{
		    object_to_destruct = ob;
		    
		    ob->finish_renewal(nob);
		    if(ob)
			destruct(ob);
		}
		else
		{
		    if(sscanf(name,"%~s#%~d")<2)
			rename_object(ob, name);
		    ob->abort_renewal();
		}
	    }
	}
    }
}

void save_objects_for_renewal(string *names)
{
    object ob;
    
    if(strstr(object_name(previous_object()),"/obj/zauberstab#") ||
#if __EFUN_DEFINED__(driver_info)
	efun::driver_info(DI_TRACE_CURRENT)[<2][TRACE_PROGRAM] !=
#else
	efun::debug_info(DINFO_TRACE, DIT_CURRENT)[<2][TRACE_PROGRAM] !=
#endif
	    "i/zauberstab/zauberstab.c (/i/zauberstab/zmarker.inc)")
	return;

    object_to_destruct = 0;
    objects_to_renew = ({});
    foreach(string name:names)
	if(ob=find_object(name))
	{
	    ob->prepare_renewal();
	    if(sscanf(name,"%~s#%~d")<2)
		rename_object(ob, BACKUP_OBJ_NAME(name));
	    objects_to_renew += ({name});
	}
}

void finish_renewal(int failure, object new_clone)
{
    if(strstr(object_name(previous_object()),"/obj/zauberstab#") ||
#if __EFUN_DEFINED__(driver_info)
	efun::driver_info(DI_TRACE_CURRENT)[<2][TRACE_PROGRAM] !=
#else
	efun::debug_info(DINFO_TRACE, DIT_CURRENT)[<2][TRACE_PROGRAM] !=
#endif
	    "i/zauberstab/zauberstab.c (/i/zauberstab/zmarker.inc)")
	return;
	
    if(failure)
    {
	abort_renewal(0);
	return;
    }

    if(sizeof(objects_to_renew))
    {
	foreach(string name:objects_to_renew)
	{
	    object ob, nob;

	    if(sscanf(name,"%~s#%~d")<2)
	    {
		ob = find_object(BACKUP_OBJ_NAME(name));
		nob = find_object(name);
	    }
	    else
	    {
		ob = find_object(name);
		nob = new_clone;
	    }

	    if(ob)
	    {
	        objects_to_renew -= ({name});
	
		if(nob)
		{
		    object_to_destruct = ob;
		    
		    ob->finish_renewal(nob);
		    if(ob)
			destruct(ob);
		}
		else
		{
		    if(sscanf(name,"%~s#%~d")<2)
			rename_object(ob, name);
		    ob->abort_renewal();
		}
	    }
	}
    }
    
    object_to_destruct = 0;
}

// Wird auch vom Preload aufgerufen.
private int get_debugfile_offset()
{
#ifdef UNItopia
    debugfile_offset = file_size("/UNItopia.debug.log");
#else
    debugfile_offset = file_size("/" __HOST_NAME__ ".debug.log");
#endif
    return debugfile_offset;
    
}

private string get_debugfile_text()
{
    int old_offset = debugfile_offset;
    int new_offset = get_debugfile_offset();
    
#if __VERSION__ > "3.5.2"
    return to_text(read_bytes(
#else
    return read_bytes(
#endif
#ifdef UNItopia
	"/UNItopia.debug.log",
#else
	"/" __HOST_NAME__ ".debug.log",
#endif
#if __VERSION__ > "3.5.2"
	old_offset, new_offset-old_offset)||to_bytes(({})), "utf-8");
#else
	old_offset, new_offset-old_offset);
#endif
}

// Mischt 2 Arrays ineinander, ohne doppelte Eintraege
private string *merge_array(string *a, string *b)
{
    return a + (b - a);
}

// Wrapperfunktion fuer limited mit weniger als den normalen Evals
private mixed limited_eval(closure c, int evals, varargs mixed* arg)
{
    return limited( (: apply(#'limited, $1,({$2}), $3) :),
	    ({LIMIT_UNLIMITED}), c, evals, arg);
}

string *expand_debugger(string *debugger)
{
    return ({string *}) limited_eval(
	symbol_function("expand_groups",GROUP_MASTER),
	EV_LIMIT_ERROR, debugger);
}

string *query_debugger_acl(string *path);

private string *query_debugger_new(string file)
{
    string *path, *deb, wiz;
    int x, y;
    object ob;
    mixed tmp;

    if (!stringp(file))
    	return ROOT_DEB;

    path = explode(file[1..],"/");

    // bei dateiname#objektnummer den # und die objektnummer wegschnippeln
    tmp = path [sizeof(path)-1];
    if (strstr (tmp,"#") != -1)
        tmp = implode (explode (tmp,"#")[0..<2],"#");
    path [sizeof(path)-1] = tmp;
    switch(path[0])
    {
      case "d":
      case "z":
        // Hierfuer sind die ACLs zustaendig
	if((ob=find_object(file)) && !catch(tmp=({string})ob->query_file_name()) && stringp(tmp))
	    deb = query_debugger_acl( explode(tmp[1..],"/"));
	else
	    deb = query_debugger_acl (path);

#ifdef UNItopia
	if(!strstr(file,"/d/Levitanis/wiz/"))
	    deb += ({ path[3] });
#endif

        if (sizeof (deb)) return deb;
        break;
      case "p":
        // Sowohl die ACLs beachten, als auch den creator || first_room auswerten
	// Wenn es ein Autoloader ist, soll nicht first-room ueberprueft werden
	if ((ob = find_object(file)) && !({mixed})ob->query_auto_load() &&
	    member(object_name(ob),'#')>0 &&
	    tmp = (({string})ob->query_creator() || ({string})ob->query_first_room()))
        {
	    tmp = trim(explode(tmp,"(")[0]);
	    if(!strstr(tmp, "/obj/zauberstab#"))
	    {
		if(ob = find_object(tmp))
		    deb = merge_array(({geteuid(ob)}), query_debugger_acl(path))-ROOT_DEB;
		else
		    deb = query_debugger_acl(path)-ROOT_DEB;
	    }
	    else
	    {
		tmp = explode(tmp,"#")[0];
		deb = merge_array(
		    query_debugger_new(tmp),
		    query_debugger_acl(path))-ROOT_DEB;
	    }
            if(sizeof(deb))
		return deb;
        }
	else
	{
    	    deb = query_debugger_acl (path);
    	    if (sizeof (deb))
		return deb;
	}
        break;
      case "map":
	// Fehler in der Map: man suche nach der passenden Domain,
	// sonst an Admin.
	if (sscanf(file, "/map/m%d_%d",x,y) == 2 &&
	    (tmp = ({string})MAP_OB->query_map_file_name(x, y)) != file)
	    return query_debugger_new(tmp);
	break;
      case "w":
	if (sizeof(path)>1 && ({int})GOETTER_REGISTER->is_wiz(path[1]))
            return path[1..1];
        break;
      case "obj":
	// Wenn es ein Autoloader ist, soll nicht first-room ueberprueft werden
	if (!(ob = find_object(file)) || ({mixed})ob->query_auto_load())
	    break;
	// Player sollen nicht auf first-room ueberprueft werden
	if (ob && playerp(ob))
	    break;
	if (strstr(file, "/obj/zauberstab#") && efun::clonep(ob) &&
	    tmp = (({string})ob->query_creator() || ({string})ob->query_first_room()))
        {
	    tmp = query_debugger_new(trim(explode(tmp,"(")[0]))+query_debugger_acl(path)-ROOT_DEB;
            if(sizeof(tmp)) return tmp;
        }
	wiz = explode(getuid(ob),":")[<1];
	if (!({int})GOETTER_REGISTER->is_wiz(wiz))
	    wiz = 0;
    }
    // Fehler konnte nicht zugeordnet werden, er geht
    // an Root und, falls wiz gesetzt ist, an wiz.
    // ACLs abfragen.
    deb = query_debugger_acl (path);
    if (wiz)
        return sizeof (deb) ? deb + ({wiz}) : ROOT_DEB + ({wiz});
    return sizeof (deb) ? deb : ROOT_DEB;
}

string *query_debugger(string file)
{
    if (!stringp(file))
    	return ROOT_DEB;

    return limited_eval(#'query_debugger_new, EV_LIMIT_ERROR, file);
}

// private // wird vom Zauberstab aufgerufen, daher nicht private.
string add_slash(string str)
{
    if (str && str[0] != '/' && str[0] != '#' && str[0] != '<')
	return "/" + str;
    return str;
}

private object get_current_player()
{
    object pl = this_interactive();
    if (pl && load_name(pl) == PORTAL_CONNECTION)
        pl = funcall(symbol_function("this_interactive")); // Sefun fragen
    pl ||= this_player();
    return pl;
}

private void add_error(mapping error)
{
    string ca_msg;
    
    m_add(error, EDB_E_DATE, time());
    if(!member(error, EDB_E_PLAYER))
    {
	object pl = get_current_player();
	if(playerp(pl))
	    m_add(error, EDB_E_PLAYER, ({string})pl->query_real_name());
    }
    if(!member(error, EDB_E_WHO))
    {
	mixed *stack;
	
	if(this_interactive())
	    catch(m_add(error, EDB_E_WHO, sprintf("this_interactive(): %O",
		get_current_player())); reserve 10000);
	else if(this_player())
	    catch(m_add(error, EDB_E_WHO, sprintf("this_player(): %O",
		this_player())); reserve 10000);
	else if(pointerp(stack=error["__stack"]) && sizeof(stack)>1)
	{
	    mixed ob = stack[<2];
	    if(stringp(ob))
		ob = find_object(add_slash(ob));
	    if(objectp(ob))
		catch(m_add(error, EDB_E_WHO, sprintf("previous_object(): %O",
		    ob)); reserve 10000);
	}
    }
    
    m_delete(error, "__stack");
    
    ca_msg = catch(ERROR_DB->add_error(error); reserve 2000);
    if (ca_msg)
        write_file("/log/sys/ERROR_DB",time()
            +": Fehler bei add_error: ("+get_eval_cost()+") "+
            (ca_msg[<1..<1]=="\n"?ca_msg:ca_msg+"\n"));
}

private mapping update_obj_error(mapping error, object ob)
{
    string * dbg,*propdbg;
    if(ob)
    {
        m_add(error, EDB_E_USER_INFO,
            (error[EDB_E_USER_INFO]||([]))
            +((({mapping})ob->query(P_DEBUG_INFO))||([]))
            +((({mapping})ob->query_debug_info())||([])));
        m_add(error, EDB_E_FIRSTROOM, ({string})ob->query_first_room());
        propdbg = ({string*})ob->query(P_DEBUG_GROUP)||({});
        if (sizeof(propdbg))
        {
            dbg = error[EDB_E_DEBUGGER];
            m_add(error, EDB_E_DEBUGGER, merge_array(dbg,propdbg));
        }
    }
    return error;
}

private void add_obj_error(mapping error)
{
    object ob = error[EDB_E_OBJ] && find_object(error[EDB_E_OBJ]);

    if (ob)
        catch(error = update_obj_error(error,ob); reserve 5000);
        // call others ueber catch absichern.
    
    add_error(error);
}

private string online_error_message(object target, string prg, int line,
				    string curobj, string err, int prev);
private string channel_error_message(object target, string prg, int line,
				     string curobj, string err, object prevobj);

private void add_runtime_error(mapping error)
{
    object target = get_current_player();
    string target_wiz_name;
    string *curobj_debugger, *prg_debugger,
           *exp_curobj_debugger, *exp_prg_debugger,
	   *zus;

    // target_wiz_name wird gesetzt, wenn target ein wiz ist
    if (target && wizp(target))
	target_wiz_name=({string})target->query_real_name();

    // Jetzt bekommt derjenige, der den Fehler erzeugt hat, noch eine
    // direkt eine Meldung auf den Bildschirm.
    if (target && (!error[EDB_E_WARNING] || target_wiz_name))
	tell_object(target, target_wiz_name ?
	       online_error_message(target,
	          error[EDB_E_FILE], error[EDB_E_LINE],
		  error[EDB_E_OBJ], error[EDB_E_ERROR], 0) :
	    	(target==error["__culprit"]) ? "" : DEF_MSG);

    
    if (playerp(error["__culprit"]))
	tell_object(error["__culprit"],
	    "Der Game-Driver redet zu dir: Dein Herz ist stehengeblieben!\n");
    m_delete(error, "__culprit");
    
    // Root ist erstmal nie Debugger...
    curobj_debugger = query_debugger(error[EDB_E_OBJ]) - ({"Root"});
    prg_debugger = query_debugger(error[EDB_E_FILE]) - ({"Root"});

    if(pointerp(error[EDB_E_DEBUGGER]))
	curobj_debugger = merge_array(error[EDB_E_DEBUGGER], curobj_debugger);

    // ... Es sei denn, es findet sich sonst niemand.
    if(!sizeof(curobj_debugger+prg_debugger))
        curobj_debugger=({"Root"});

    exp_curobj_debugger = expand_debugger(curobj_debugger);
    exp_prg_debugger = expand_debugger(prg_debugger);

    // Die zustaendigen Wizzes bekommt eine Meldung ueber den Kurier/Runtime
    // falls sie anwesend sind und der Erzeuger nicht dafuer zustaendig ist.
    zus = merge_array(exp_curobj_debugger,exp_prg_debugger);
    if (member(zus,target_wiz_name) < 0)
    {
	string msg;

	foreach(string wiz: zus)
	{
	    object wiz_ob = find_player(wiz);
	    
	    if(wiz_ob && interactive(wiz_ob))
	    {
		if(!msg)
		{
		    mixed stack = error["__stack"];
		    mixed prev_ob;
		    if(pointerp(stack) && sizeof(stack)>1)
		    {
			prev_ob = stack[<2];
			if(stringp(prev_ob))
			    prev_ob = find_object(add_slash(prev_ob));
		    }
		    
		    msg = channel_error_message(target, 
	        	error[EDB_E_FILE], error[EDB_E_LINE],
			error[EDB_E_OBJ], error[EDB_E_ERROR],
			prev_ob);
		}
		catch(EVENT_MASTER->talk(wiz_ob, "Runtime", target, msg);
		    reserve 20000);
	    }
	}
    }

    // Start: Filelogging

    // Falls derjenige, der den Fehler auf den Bildschirm bekommt
    // einer der Debugger fuer BEIDE Files ist, wird der Fehler nicht geloggt
    if(target_wiz_name && error[EDB_E_FILE] != sprintf("/w/%s/LPC_zst.c", target_wiz_name) && find_object("/obj/zauberstab"))
    {
	object z = present_clone("/obj/zauberstab", target);
	if(z && ({int})z->get_own_rtes())
	    target_wiz_name = 0;
    }
    
    if (target_wiz_name && interactive(target) &&
	(!sizeof(exp_curobj_debugger) ||
	 member(exp_curobj_debugger,target_wiz_name) >= 0) &&
	(!sizeof(exp_prg_debugger) ||
         member(exp_prg_debugger,target_wiz_name) >= 0))
    	return;

    m_add(error, EDB_E_DEBUGGER, merge_array(curobj_debugger,prg_debugger));
    m_add(error, EDB_E_TYPE, EDB_RUNTIME);
    if(!member(error, EDB_E_TRACE))
	m_add(error, EDB_E_TRACE, get_debugfile_text());
    m_add(error, EDB_E_COMMAND, query_command());
    if (playerp(target))
    {
        mapping dinfo = error[EDB_E_USER_INFO] ||([]);
        dinfo["RTE_TP"] = sprintf("%Q in %Q",target,environment(target));
        m_add(error,EDB_E_USER_INFO,dinfo);
    }

    add_obj_error(error);
}

// ============ COMPILE TIME ERROR ============================================
//
// The error occured in the object 'file', giving the error message 'message'.

// Bei einem Syntax-Fehler in einem File
protected void log_error(string file, string message, int warn)
{
    object target;
    int no_tell;
    string *debugger;
    file = add_slash(file);

    target = get_current_player();

    message -= "\r";  // Bei Windows-Dateien

    if (warn)
	message = "Warning: " + message;

    // Fuer den Fall, dass es ein Fehler waehrend des Ladens der simul-efun
    // war, sollte hier kein Objekt geladen werden. Deswegen lieber raus.
    if (loading_simul_efun)
        return;

#ifdef Orbit
    // Fuer Tools, die Verzeichnisse in Orbit durchladen.
    // Sie koennen damit die Fehler abfangen.
     if(({mixed})CONTROL->forbidden("compile_error",file,message,warn))
        return;
#endif

    if (target && wizp(target))
    {
	tell_object(target, message);
	if(file != sprintf("/w/%s/LPC_zst.c", ({string})target->query_real_name()) && find_object("/obj/zauberstab"))
	{
	    object z = present_clone("/obj/zauberstab", target);
	    if(z && ({int})z->get_own_rtes())
		no_tell = 1;
	}
    }
    else
	no_tell = 1;

#if defined(UNItopia) && !defined(TestMUD)
    if (warn && sscanf(message, "%~svariable%~swas%~snever%~sused%~s") == 5)
        return;
#endif

    debugger = query_debugger(file);
    if (no_tell || member(expand_debugger(debugger),
			  ({string})target->query_real_name()) < 0)
    {
	int line;
	
	sscanf(message,"%!s line %d%!s",line);
	
	add_error(([
	    EDB_E_TYPE: EDB_COMPILE,
	    EDB_E_WARNING: warn,
	    EDB_E_ERROR: message,
	    EDB_E_DEBUGGER: debugger,
	    EDB_E_FILE: file,
	    EDB_E_LINE: line,
	    "__stack": caller_stack(),
	]));
    }

    if(!warn) abort_renewal(0);
}

// ============ RUN TIME ERROR ================================================

mixed include_file(string file, string compiled_file, int sys_include);

//Die Meldung, die TI/TP bekommt, wenn er ein Gott ist.
//Ist prev==0, so wird die Zeile im Quelltext angezeigt.
private string online_error_message(object target, string prg, int line,
				    string curobj, string err, int prev)
{
    string str, context, main, incl, file;

    str = LINE +
       	"File:   " + prg + "  Zeile: " + line + "\n"
	"Objekt: " + curobj + "\n";

    // Show Context
    if (!prev)
    {
	if (prg && (sscanf(prg,"%s (%s)",main,incl) == 2))
	{
	    file = include_file(incl,main,0);
	    if (!stringp(file))
		file = 0;
	}
	else
	{
	    file = prg;
	}
	if (!stringp(file))
	    write_file("/log/sys/LOG_ERR",sprintf("PRG: %O\n",prg));

	if (file && !catch(context = read_file(file,line - 1,3)) && context)
	    str += "\n" + context + "\n";
    }

    if(strstr(err, "Warning: "))
	str += "Fehler: " + err + LINE;
    else
	str += "Warnung: " + err[9..<1] + LINE;
    return str;
}

// Die Meldung, die an die eingeloggten Goetter gehen soll.
private string channel_error_message(object target, string prg, int line,
				     string curobj, string err, object prevobj)
{
    return
	"RUNTIME-ERROR:\nWer:    " +
       	sprintf("%O", target ? target : prevobj) +
	"   Command: '" + VERB(target) +
	"'\nFile:   " + prg + "   Zeile: " + line +
       	"\nObjekt: " + curobj + 
	(strstr(err,"Warning: ")?("\nFehler: " + err):("\nWarnung: " + err[9..<1]));
}

// The error 'err' occured in the object 'curobj', file 'prg', line 'line'.
// Wird von runtime_error aufgerufen
private void log_runtime_error(string err, string prg, string curobj,
			      int line, mixed* stack, mixed culprit, int caught, int warn)
{
    if(warn)
#if __EFUN_DEFINED__(driver_info)
	debug_message(efun::driver_info(DI_TRACE_CURRENT_AS_STRING), DMSG_STDOUT | DMSG_LOGFILE | DMSG_STAMP);
#else
	debug_message(efun::debug_info(DINFO_TRACE,DIT_STR_CURRENT),
	    DMSG_STDOUT|DMSG_LOGFILE);
#endif
#ifdef Orbit
    // Fuer Tools, die Verzeichnisse in Orbit durchladen.
    // Sie koennen damit die Fehler abfangen.
     if(({mixed})CONTROL->forbidden(warn ? "runtime_warning" : "runtime_error",curobj,err,prg,line))
        return;
#endif

    // Sonderbehandlung bei touch(), clone_object(), oder create_map()
    // Diese Fehler werden nur in Kurzform an den Erzeuger geschickt,
    // sonst versinken wir in Fehlermeldungen...
    if ((!strstr(err,"Error in loading object: ") 

    // Diese 'failed to load file'-Meldungen muessen in die FDB, weil
    // es keine Compile-Fehler dazu gibt.
    //	|| !strstr(err,"Failed to load file: ")
	
	) &&
	    ((prg == SIMUL_EFUN" (/secure/simul_efun/map.inc)") ||
	     (prg == MASTER_OB".c (/secure/master/compiler_control.inc)") ||
	     (prg == SIMUL_EFUN" (/secure/simul_efun/clone_object.inc)" ||
	      prg == "/map/map.c") &&
	      curobj == "/map/map"))
    {
	object target = get_current_player();
	int wiz = wizp(target);

	if(target && (!warn || wiz))
	    tell_object(target, wiz ? err : DEF_MSG);
	abort_renewal(curobj);
	return;
    }

    if (warn && strstr(err, "Warning:"))
	err = "Warning: " +err;

    // Bei Fehlern in der simul-efun oder /obj/-Dateien das aufrufende
    // Objekt suchen, da diese meist daran Schuld haben
    if(!warn && curobj && (curobj + ".c" == SIMUL_EFUN || curobj[0..4] == "/obj/"))
    {
#if __EFUN_DEFINED__(driver_info)
	mixed* error_trace = efun::driver_info(DI_TRACE_LAST_ERROR);
#else
	mixed* error_trace = efun::debug_info(DINFO_TRACE, DIT_ERROR);
#endif
	string new_obj;
	object z_ob;
	int i;

        if((i=sizeof(error_trace))>1)
            for(;--i;) // i=0 darf nicht auftreten.
            {
                new_obj = add_slash(error_trace[i][TRACE_OBJECT] || error_trace[i][TRACE_PROGRAM]);
                if((new_obj + ".c") !=SIMUL_EFUN && new_obj[0..4] != "/obj/")
                {
                    curobj = new_obj;
                    break;
                }
                else if(new_obj[0..15] == "/obj/zauberstab#" &&
                        (z_ob=find_object(new_obj)) && 
                        geteuid(z_ob)!="Learner")
                {
		    curobj = new_obj;
                    //curobj_debugger=({geteuid(prg_wiz)});
                    break;
                }
            }
    }
    else if(!warn && (curobj == object_name() || curobj == "/apps/control"))
    {
#if __EFUN_DEFINED__(driver_info)
	mixed* error_trace = efun::driver_info(DI_TRACE_LAST_ERROR);
#else
	mixed* error_trace = efun::debug_info(DINFO_TRACE, DIT_ERROR);
#endif
	
	// Besondere Behandlung bei cron-jobs, um die Debuginfos
	// von /apps/cron mitzubekommen.
	if(sizeof(error_trace)>1 &&
	    add_slash(error_trace[1][TRACE_OBJECT])=="/apps/cron" &&
	    error_trace[1][TRACE_NAME]=="job")
		curobj = "/apps/cron";
    }

    add_runtime_error(([
	EDB_E_SUBTYPE: (intp(culprit) && culprit<0) ? EDB_RUN_RUNTIME : EDB_RUN_HB,
	EDB_E_WARNING: warn,
	EDB_E_ERROR: err,
	EDB_E_OBJ: curobj,
	EDB_E_FILE: prg,
	EDB_E_LINE: line,
	"__culprit": culprit,
	"__stack": stack - ({0}),
    ]));

    if(!warn)
	abort_renewal(curobj);
}

// Wird bei einem Runtime-Fehler aufgerufen
protected void runtime_error(string err, string prg, string curobj, int line, mixed culprit, int caught)
{
    /* Keine Fehlerbehandlung ohne Simul-Efun. */
    if (loading_simul_efun)
        return;

    limited_eval(#'log_runtime_error, EV_LIMIT_ERROR,
	err, add_slash(prg), add_slash(curobj), line,
#if __EFUN_DEFINED__(driver_info)
	reverse(map(efun::driver_info(DI_TRACE_LAST_ERROR)[1..<1],#'[,TRACE_OBJECT)),
#else
	reverse(map(efun::debug_info(DINFO_TRACE, DIT_ERROR)[1..<1],#'[,TRACE_OBJECT)),
#endif
	culprit, caught, 0);
}

protected void runtime_warning(string err, string curobj, string prg, int line, int caught)
{
    limited_eval(#'log_runtime_error, EV_LIMIT_ERROR,
	err, add_slash(prg), add_slash(curobj), line,
	caller_stack(), -1, caught, 1);
}


// ============ HEART BEAT ERROR ==============================================
//
int heart_beat_error(object ob, string err, string prg, string curobj, int line, int caught)
{
    return 0; /* Don't restart */
}


// ============ FEHLER LOGGING fuer do_error ==================================
//
// Hiermit werden Fehler geloggt, die mit do_error() aus <error.h> erzeugt
// wurden. ("userdefined errors")
// (eine entschaerfte Version von runtime_error())

void do_log_error(string err, string prg, string curobj, int line, string *debugger, int no_msg)
{
    // Filegroesse neu berechnen.
    get_debugfile_offset();
    if (!stringp(err) || !stringp(curobj) || !stringp(prg))
	return;
    
    debug_message("do_error: " + err,DMSG_STDOUT|DMSG_LOGFILE|DMSG_STAMP);
    debug_message("program: " + prg + ", object: " + curobj + " line " +
	    line + "\n",DMSG_STDOUT|DMSG_LOGFILE|DMSG_STAMP);
#if __EFUN_DEFINED__(driver_info)
    debug_message(efun::driver_info(DI_TRACE_CURRENT_AS_STRING),
	    DMSG_STDOUT | DMSG_LOGFILE);
#else
    debug_message(efun::debug_info(DINFO_TRACE,DIT_STR_CURRENT),
	    DMSG_STDOUT|DMSG_LOGFILE);
#endif

    if (!pointerp(debugger))
	debugger = 0;
    else
    {
	string *tmps = map(debugger,"valid_debugger",ERROR_DB) - ({0});
	debugger = ({});
	foreach(string deb: tmps)
	    if(member(debugger,deb) == -1)
		debugger += ({deb});
    }

    add_runtime_error(([
	EDB_E_SUBTYPE: EDB_RUN_USER_DEFINED,
	EDB_E_WARNING: no_msg,
	EDB_E_ERROR: err,
	EDB_E_OBJ: curobj,
	EDB_E_FILE: prg,
	EDB_E_LINE: line,
	EDB_E_DEBUGGER: debugger,
	"__stack": caller_stack(),
    ]));
}

// ============ FEHLER LOGGING fuer das 'fehler' Kommando =====================
//
// Alle Fehler die durch das 'fehler' Kommando des players gemeldet werden,
// werden hier verarbeitet.


// findet die debugger fuer beliebige Objekte und v_items
private mapping query_program_info(mixed ob, object who)
{
    string *path;
    mapping res;
    int x;

    if (stringp(ob))
	return (["debugger":query_debugger(ob),"name":ob,"info":""]);

    if (mappingp(ob))
    {
	// Hat wohl einer einen Fehler in einem VItem gefunden:
	// geht an den passenden vi-Master, Raum oder das passende Objekt
	res = query_program_info(
        ob["origin"] || ob["v_item_master"] || ob["environment"]
        || environment(who),
        who);
	return ([
            "debugger" : res["debugger"],
            "name"     : res["name"],
            "info"     : res["info"]+"V-Item    : "+
			 (!pointerp(ob["nummer"])
			  ?(((ob["nummer"]) ? ob["nummer"]+". " : "")+ob["name"])
			  :(sizeof(ob["nummer"])>1)
			  ?((ob["id"] && (x=member(ob["id"], ob["name"]))>=0)
			    ?(ob["nummer"][x]?ob["nummer"][x]+". ":"")+ob["name"]
			    :(ob["nummer"][0]?ob["nummer"][0]+". ":"")+ob["name"])
			  :((ob["nummer"][0]?ob["nummer"][0]+". ":"")+ob["name"]))
			 +"\n"
            ]);
    }
    if (objectp(ob))
    {
	// Objekte:
        if ( playerp(ob) && environment(ob) )
        {
            res = query_program_info(environment(ob), ob);
            res["debugger"] = (res["debugger"] - ROOT_DEB) 
                + query_debugger(object_name(ob)) ;

            if (wizp(ob) && member(res["debugger"],
                    ({string})ob->query_real_name()) == -1)
                res["debugger"] += ({ ({string})ob->query_real_name() }) ;
            res["info"] = sprintf("%sObjekt    : %O\n", res["info"], ob);
            // Achtung: Sollte obiger Text / Formatierung jemals geaendert
            // werden, so muss sie auch in /i/zauberstab/zfehler.inc beim
            // Fehlerfeedback angepasst werden, einfach nach Objekt   suchen
            return res ;
        }
	path = explode(object_name(ob),"/")-({""});
	if ((path[0]=="obj" || (path[0]=="p" && member(path,"obj")>0)) &&
	    !({mixed})ob->query_auto_load() && ({string})ob->query_first_room())
	{
	    // Die Objekte unter /obj werden fast immer nur geclont und
	    // in einem Raum angepasst, also an den erschaffenden Raum. Bei
	    // Autoloadern liegt das anders, da sollte der Fehler an
	    // Admin gehen.
	    res = query_program_info(explode(({string})ob->query_first_room(),
	    	"(")[0], who);
            // WENN es ACLs fuer die /obj/ - Datei gibt, DANN sollen die
            // AUCH zum Zug kommen.

            res["debugger"] = query_debugger(object_name(ob)) ;
	    return (["debugger":res["debugger"], "name":res["name"],
		"info":sprintf("%sObjekt    : %O\n",res["info"],ob) ]);
	}
        res =  query_program_info(object_name(ob), who);
	if(member(path,"obj")>=0)
	    res["info"]=sprintf("%sObjekt    : %O\n",res["info"],ob);
	return res;
    }
    if (environment(who))
  	return query_program_info(environment(who), who);
    return (["debugger":ROOT_DEB,"name":sprintf("%Q\n", ob),"info":""]);
}

// TODO: nach log_fehler umbenennen.
// (Wird sowohl vom /i/player/fehler als auch von Defines in error.h aufgerufen.)
void log_fehler_new(string text, mixed what, object who, int flag, mapping dinfo2)
{
    mapping res, dinfo;
    string wozu,*propdbg=({});

    // Die Debugger ermitteln:
    res = query_program_info(what, who);

    // wozu ermitteln:
    if (objectp (what)) 
    {
        if (({int})what->query_room() && ({string})what->query_short())
            wozu = "Ort: "+({string})what->query_short()
            + (({string})what->query_room_domain()?" ("+({string})what->query_room_domain()+")":"");
        else if (({string})what->query_gender() && ({string})what->query_name())
            wozu = living(what)
                ? "Lebewesen: "+({string})what->query_name()
                : "Gegenstand: "+({string})what->query_name();
        dinfo = (({mapping})what->query(P_DEBUG_INFO)||([]))
              + ((({mapping})what->query_debug_info())||([]));
        if (sizeof(dinfo)==0) dinfo = 0;
        propdbg = ({string*})what->query(P_DEBUG_GROUP)||({});
    }
    else if (mappingp (what)) {
        mixed env;

        if (what["gender"] && what ["name"])
            wozu = "Detail: "+what["name"];

        env = what;
        while(mappingp(env) && !dinfo)
        {
            dinfo = funcall(env["debug_info"],env);
            env = env["environment"];
        }
        if (objectp(env))
            propdbg = ({string*})env->query(P_DEBUG_GROUP)||({});
        if(!dinfo && objectp(env))
        {
            dinfo = ({mapping}) env->query_debug_info();
            if (mappingp(dinfo))
            {
                dinfo += ({mapping})env->query(P_DEBUG_INFO)||([]);
            }
            else
            {
                dinfo = ({mapping})env->query(P_DEBUG_INFO);
                if (!sizeof(dinfo))
                {
                    dinfo = 0;
                }
            }
        }
    }
    if (mappingp(dinfo2)) {
        if (mappingp(dinfo)) {
            dinfo = dinfo + dinfo2;
        } else {
            dinfo = dinfo2;
        }
    }
    if (sizeof(propdbg))
    {
        res["debugger"] = merge_array(res["debugger"],propdbg);
    }

    // Monty: Fehlerlogging fuer Idee (vorlaeufig)
    ERROR_DB->add_error( ([
	EDB_E_TYPE: flag,
	EDB_E_FILE: res["name"],
	EDB_E_ERROR: res["info"] + sprintf("%-=79s\n",text),
	EDB_E_DATE: time(),
	EDB_E_DEBUGGER: res["debugger"],
	EDB_E_PLAYER: ({string})who->query_real_name(),
	EDB_E_ROOM: object_name(environment(who)),
	EDB_E_OBJ_DESC: wozu,
	EDB_E_USER_INFO: dinfo
    ]) );
}
