// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/secure/master/creator_file.inc
// Description: Welche UID soll ein neues Objekt bekommen ?
// Author:	Freaky (23.12.93)
// Modified by:	Freaky (25.09.95) Security-Check bei /map/mX_Y.c
//              Kurdel (20.08.97) Clones unter /map/obj/ erhalten MAP_UID

#pragma strict_types

#define NO_LOAD_DIRS ({ "/sys/", "/INFO/", "/doc/", "/save/", \
	"/var/", "/static/", "/priv/", "/log/", "/Old/", "/OLD/", "/demo/" })
#define NO_CLONE_DIRS ({ "/touch/", "/apps/", "/i/" })

private string get_apps_uid(string name)
{
#if __VERSION__ >= "3.5.99"
    bytes fil;
    int pos1, pos2;

    fil = read_bytes(name + ".c",0,10000);
    pos1 = strstr(fil || b"", b"\n// UID: ");
    if (pos1 > 0)
    {
        pos2 = strstr(fil, to_bytes(({'\n'})), pos1+9);
        if (pos2 > 0)
            return to_text(fil[pos1+9..pos2-1], "ascii");
    }
#else
    string fil, uid;

    fil = read_bytes(name + ".c",0,10000);
    if (fil && (sscanf(fil,"%~s\n// UID: %s\n",uid) == 2))
	return uid;
#endif
    return GENERAL_UID;
}

int no_load_file(string file)
{
    foreach(string dir : NO_LOAD_DIRS)
	if (strstr(file,dir) >= 0)
	    return 1;
#ifndef TestMUD
    if(strstr(file, "/Orbit/") >= 0)
	return 1;
#endif

    if (strstr(file,"/Admin/") >= 0)
    {
	// TODO: Rausnehmen, da es /Admin/Secure/ gibt.
	if (strstr(file,"/z/Raetsel/Admin/") == 0)
	    return 0;
	// in /{z,d}/*/Admin/Secure/ duerfen von den Admins genehmigte
	// Objekte geladen werden
	if (strstr(file,"/Admin/Secure/") >= 0)
	{
	    string *path;

	    path = explode(file,"/");
	    if ((path[1] == "z" || path[1] == "d") && path[3] == "Admin" &&
		    path[4] == "Secure")
		return 0;
	}
	return 1;
    }
}

int no_clone_file(string file)
{
    foreach(string dir : NO_CLONE_DIRS)
	if (strstr(file,dir) >= 0)
	    return 1;
    if (strstr(file,"/obj/") >= 0)
	return 0;

    /*
    if (strstr(file,"/Admin/") >= 0)
	return 1;
    if (strstr(file,"/room/") >= 0)
	return 1;
    */

    return 1;
}

// from_hook == 1, wenn wir vom Driver-Hook beim Laden eines Objektes
//                 aufgerufen wurden.
// from_hook == 0, wenn wir vom /map/map oder ls-Befehl aufgerufen wurden.

mixed get_load_uid(string str, int from_hook)
{
    string *name;
    int s;

#if 0
    if (find_player("gnomi"))
	//tell_object(find_player("freaky"),sprintf("LU:%O: po:%O:\n",str,previous_object(1)));
	tell_object(find_player("gnomi"),sprintf("LU:%O: po:%O:\n",str,caller_stack()));
#endif

    name = explode(str,"/");
    s = sizeof(name);
    if (s < 2)
	return 0;
    if (!valid_file_name(str))
	return 0;

    if (no_load_file("/" + str))
	return 0;

    switch(name[0])
    {
      case "d":
        // Map-Raeume nur von /map.
    	if (from_hook && funcall(symbol_function("domain2map"), "/"+str))
	    return 0;
#ifdef UNItopia	
	if(name[1] == "Levitanis")
	{
	    if(s==3)
		return 0;
	    
	    switch(name[2])
	    {
		case "i":
		case "obj":
		case "apps":
		    return name[1]+":"+name[2];
		
		case "public":
		case "wiz":
		{
		    object ob = find_object(str);
		    if(!ob)
			return 0;
		
		    // Inherits testen	
		    foreach(mixed inh: efun::inherit_list(ob,INHLIST_TREE))
		    {
			string inhname = stringp(inh)?inh:(inh[0]);
			if(strstr(inhname,"/d/Levitanis/i/") &&
			   inhname != "/"+str+".c")
			    return 0;
		    }
		    
		    // Funktionen pruefen
		    if(sizeof(efun::functionlist(ob, NAME_INHERITED|RETURN_FUNCTION_NAME)
			-({"create","__INIT"})))
			return 0;
		    
		    if(name[2]=="public" || s==4)
			return name[1]+":"+name[2];
		    return name[1]+":"+name[3];
		}
	    }
	    return 0;
	}
#endif

	if (s > 3)
	{
	    /*
	    if (GOETTER_REGISTER->is_wiz(name[2]))
	    	return name[1] + ":" + name[2];
	    if (s > 4 && name[2][0] >= 'A' && name[2][0] <= 'Z' &&
	    		GOETTER_REGISTER->is_wiz(name[3]))
	    	return name[1] + ":" + name[3];
	    */
	    return name[1] + ":" + name[2];
	}
	return s == 3 ? name[1] + ":Std" : 0;
      case "map":
      {
	if (str == "map/map")
	    return ROOT_UID;
	// Freaky: Hier sollte man besser noch abpruefen, ob prev_ob==/map/map
	//         ist...
	if (sscanf(name[1],"m%~d_%~d") == 2)
	    return previous_object() &&
		   (object_name(previous_object()) == MAP_OB) && BACKBONE_UID;
	return MAP_UID;
      }
      case "z":
        return s > 3 ? name[1] + ":" + name[2] : 0;
      case "p":
	return s > 2 ? "p:" + name[1] : 0;
      case "w":
        // LPC-File bekommt echte UID
        if (s == 3 && name[2] == "LPC_zst")
	    return name[1];
	return s > 2 ? "w:" + name[1] : 0;
      case "obj":
      case "i":
      case "lwo":
      case "room":
      case "tests":
	return GENERAL_UID;
      case "secure":
	return ({ ROOT_UID + ":" + name[<1], ROOT_UID });
      case "apps":
        {
	  string apps_uid = get_apps_uid(str);
	  return ({apps_uid+":"+name[<1], apps_uid});
	}
    }
    return 0;
}

#if 0
#define TEST_OBJ if (strstr(str,"/obj/") == -1) \
    write_file("/log/sys/CLONE",sprintf("CU-NoObj:%O PO:%O\n",str,caller_stack()));
#else
#define TEST_OBJ
#endif

mixed get_clone_uid(string str)
{
    string *name;
    int s;

#if 0
    if (find_player("gnomi"))
	tell_object(find_player("gnomi"),sprintf("CU:%O:\n",str));
#endif

    name = explode(str,"/");
    s = sizeof(name);
    if (s < 2)
	return 0;

    if (no_clone_file("/" + str))
	return 0;

    switch(name[0])
    {
      case "d":
      	TEST_OBJ
	if (s > 3)
	{
	    /*
	    if (GOETTER_REGISTER->is_wiz(name[2]))
	    	return name[1] + ":" + name[2];
	    if (s > 4 && name[2][0] >= 'A' && name[2][0] <= 'Z' &&
	    		GOETTER_REGISTER->is_wiz(name[3]))
	    	return name[1] + ":" + name[3];
	    */
	    return name[1] + ":" + name[2];
	}
	return s == 3 ? name[1] + ":Std" : 0;
      case "obj":
	return BACKBONE_UID;
      case "map":
        return name[1] == "obj" ? MAP_UID : 0;
      case "z":
        return s > 3 && strstr(str,"/obj/") != -1 ? name[1] + ":"+ name[2] : 0;
      case "p":
      	TEST_OBJ
	return s > 2 ? "p:" + name[1] : 0;
      case "w":
      	TEST_OBJ
	/*
	if (strstr(str,"/obj/") == -1)
	    write_file("/log/sys/CLONE",sprintf("CU-NoObj:%O PO:%O\n",str,
			caller_stack()));
	*/
	return s > 2 ? "w:" + name[1] : 0;
      case "room":
      case "tests":
	return GENERAL_UID;
      case "secure":
	return name[1] == "obj" ? ({ ROOT_UID + ":" + explode(name[<1],"#")[0], ROOT_UID }) : 
	       name[2] == "obj" ? ({ ROOT_UID + ":" + name[1], ROOT_UID }) : 0;
    }
    return 0;
}

// Fuer die Driver-Hooks
private string h_load_uids(string obj_name)
{
    string uid;

    uid = get_load_uid(obj_name, 1);
    if (uid == BACKBONE_UID)
	return geteuid(previous_object());
    return uid;
}

private string h_clone_uids(string blueprint, string new_name)
{
    string uid;
    object po = previous_object();

#if ! __EFUN_DEFINED__(export_uid)
    if (object_name(po) == "/map/map" && !member(([ROOT_UID, MAP_UID]), geteuid(po)))
        return geteuid(po);
#endif

    uid = get_clone_uid(new_name);
    if (uid == BACKBONE_UID)
	return geteuid(po);
    return uid;
}

#ifdef __LPC_LWOBJECTS__
private string h_lwobject_uids(object blueprint)
{
    string file = object_name(blueprint);
    string *path = explode(file[1..], "/");
    int len = sizeof(path);

    if (len < 2)
        return 0;

    foreach(string dir : NO_CLONE_DIRS)
        if (strstr(file,dir) >= 0)
            return 0;
    if (strstr(file,"/lwo/") < 0)
        return 0;

    switch(path[0])
    {
        case "d":
            if (len > 3)
                return path[1] + ":" + path[2];
            else if (len == 3)
                return path[1] + ":Std";
            else
                return 0;

        case "p":
            if (len > 2)
                return "p:" + path[1];
            else
                return 0;

        case "z":
            if (len > 3)
                return path[1] + ":"+ path[2];
            else
                return 0;

        case "w":
            if (len > 2)
                return "w:" + path[1];
            return 0;

        case "lwo":
            return geteuid(previous_object());

        case "room":
            return GENERAL_UID;
    }
    return 0;
}
#endif
