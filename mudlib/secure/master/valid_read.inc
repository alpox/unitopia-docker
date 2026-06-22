// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/secure/master/valid_read.inc
// Description: valid_read
// Author:	Freaky
// Modified by: Kurdel (04.11.97) Admin-Verzeichnisse fuer Domains
//		Freaky (29.03.1999) in /tar/Gilden/ duerfen die Gildenadmins
//			alles lesen.

#pragma strict_types

int valid_read_in_w(string* path, string wiz)
{
    string dir;
    string *readers;
    int priv = rmember(path, "priv");
    
    if(sizeof(path)<2)
	return 0;

    if(priv<1) priv = 1;
    dir = "/"+implode(path[0..priv-1],"/");
    
    foreach(string subdir: path[priv..])
    {
	string tmp;
	
	dir += "/" + subdir;
	tmp = read_file(dir + "/READ");
	if(tmp)
	{
	    readers = explode(tmp, "\n");
	    if(member(readers, "*"+wiz) >= 0 || member(readers, "*alle") >= 0)
		return 1;
	}
	else
	    readers = 0;
    }

    if(readers && 
	(member(readers, wiz) >= 0 || member(readers, "alle") >= 0))
	return 1;
}


/*
 * Function name:   valid_read
 * Description:     Checks if a certain user has the right to read a file
 * Arguments:       file - File name of the file to be read
 *                  user - Name of the wizard responsible for the read.
 *		    func - The calling function.
 * Returns:         True if read is allowed.
 */
#ifdef __LPC_LWOBJECTS__
mixed valid_read(string file, string eff_user, string func, object|lwobject caller)
#else
mixed valid_read(string file, string eff_user, string func, object caller)
#endif
{
    string *path, tmp;
    mixed destfile = 1;
    int level;

#if 0
    if (find_player("gnomi") && !strstr(file,"/var/adm"))
    {
	tmp="READ:FN:"+file+", EffUser:"+eff_user+",\nfunc:"+func+", caller:";
	if (caller) tmp+=object_name(caller)+".\n";
	else tmp+="0.\n";
#if 0
	tmp+=sprintf("PO:%O:\n",previous_object());
	tmp+=sprintf("TI:%O:\n",this_interactive());
	tmp+=sprintf("TP:%O:\n",this_player());
#endif
	tell_object(find_player("gnomi"),tmp);
    }
#endif

    // Gueltige Filenamen haben keine Spaces
    // Effektiver User muss angeben sein
    if (!file || file == "" || member(file,' ')>=0 || !eff_user)
	return 0;

    // Root darf alles lesen
    // file_size darf man auf alles machen
    if (eff_user == ROOT_UID)
	return 1;
	
    // Wenn // oder /./ im Pfad ist, darf man nicht lesen:
    // read_file("/w/freaky/./priv/");
    if (file[0] != '/' || strstr(file,"//") >= 0 || strstr(file,"/./") >= 0 ||
	strstr(file,"/../") >= 0)
    	return 0;

    path = explode(file[1..],"/");
    if (sizeof(path & ({".git"})))
        return 0;

#if defined(Orbit) && !defined(UnionFS)
    if(sizeof(path)>1 && path[0] == "var" && member(({"d","p","z"}),path[1])>=0)
    {
	path = path[1..<1];
	destfile = file;
    }
    else if(member(({"d","p","z"}),path[0])>=0 && (file_size("/var"+file)>=0 ||
	file_size("/var"+file+".__mark__")>=0))
	destfile = "/var"+file;
#endif

    if(func == "file_size" && file[0..8]!="/var/adm/")
	return destfile;

    if (sizeof(path) > 1)
    {
	//path = (path[0..<2] - ({"","."})) + path[<1..<1];
	switch(path[0])
	{
	  case "var":
	    // Apps darf /var/* lesen (alles)
	    if (eff_user == APPS_UID)
		return 1;
	    // /var/* darf man lesen (keine unterverzeichnisse)
	    if (sizeof(path) < 3)
		return 1;
	    // Player duerfen sich restoren
	    if (path[1] == "players" &&
		    func == "restore_object" &&
		    sizeof(path) == 4 &&
		    playerp(previous_object()))
		return 1;
            if (sizeof(path)==4 && path[1] == "spool" && path[2]=="edit"
                && ((object_name(previous_object()) == "/secure/rpc/gmcp")
                    ||(object_name(previous_object()) == "/secure/dbus/gmcp")
                    ||(playerp(previous_object()) 
                && !strstr(path[3],({string})previous_object()->query_real_name()+"_"))))
              return 1;


	    // bei /var/spool/news wird NEWSD gefragt
	    if (sizeof(path) > 4 && path[2] == "news" &&
		    this_player() == this_interactive() && 
		    (caller == this_player() ||
		     load_name(caller) == "/obj/newsreader") &&
		    ({int})NEWSD->valid_read(this_player(),path[3],path[4]))
		return 1;

	    if (sizeof(path) > 3 && path[1] == "adm" &&
		    path[2] == "spielerratsarchiv" && 
		    this_player() == this_interactive() && 
		    caller == this_player() && 
		    (adminp(this_player()) || spielerratp(this_player())))
		return 1;
	    break;
	  case "w":
	    // Jeder eff_user darf sein /w/eff_user/ lesen
	    if (sizeof(path) > 1 && path[1] == eff_user)
		return 1;
            // von /save/ duerfen sich auch Objekte mit UID w:<name> restoren.
            if (member(path,"save") > 1)
                if(eff_user == ("w:" + path[1]))
                    return 1;
	    if (sizeof(path) == 2 && func == "get_dir")
                return 1;
            if (sizeof(path) > 2 && path[2] == "public")
                return 1;
            break;
	  case "z":
	    // das Admin-Verzeichnis duerfen nur die jeweilige Admins lesen.
	    if (sizeof(path) < 3 || (path[2] != "Admin" &&
	       (sizeof(path) < 4 || path[1] != "Gilden" || path[3] != "Admin")))
	    	return destfile;
	    if ((member(({string *})FILED->query_auth(lower_case(path[1]))
		    || ({}),eff_user) >= 0) ||
		(path[1]=="Gilden" && (({int})FILED->is_coder_of(lower_case(path[1]),
		    path[2], eff_user) 
		    || query_access(path[0..2],eff_user,ACL_WRITE))))
				return destfile;
	    break;
	  case "d":
	    // das Admin-Verzeichnis duerfen nur die Lords und Helfer lesen
	    if (sizeof(path) < 3 || path[2] != "Admin" ||
	        ({int})DOMAIN_INFOS->domain_lord(path[1],eff_user) ||
	        ({int})DOMAIN_INFOS->domain_helfer(path[1],eff_user))
		return destfile;

	    /*
	    if (member(query_domains_of(eff_user), path[1]) >= 0
                || member(query_domainhelfer_of(eff_user), path[1]) >= 0)
		return 1;
	    */
	    break;
	  case "tar":
	     // Verzeichnis darf jeder lesen
	     if(func == "get_dir")
		return 1;

	     // Doku darf auch jeder lesen
	     if(path[1]=="Doc")
	        return 1;
		
	     // Domainlords und Domainhelfer duerfen ihr .tar.gz lesen
	     // wenn sie via ftp zugreifen, Files ohne .tar.gz als Endung
	     // darf jeder lesen
	     if (sscanf(path[<1],"%s.tar.gz",tmp) == 1 ||
	         sscanf(path[<1],"%s.tar.bz2",tmp) == 1)
	     {
		if((!strstr(object_name(previous_object()), "/secure/rpc/") ||
		    !strstr(object_name(previous_object()), "/secure/dbus/")) && // FTP
			(tmp == "p"
			|| tmp == "exports"
			|| ({int})DOMAIN_INFOS->domain_lord(tmp,eff_user)
			|| ({int})DOMAIN_INFOS->domain_helfer(tmp,eff_user)
			/*
		    	|| member(query_domains_of(eff_user),tmp) >= 0
			|| member(query_domainhelfer_of(eff_user),tmp) >= 0
			*/
                        || member(({string*})FILED->query_coder_of("gilden",tmp) || ({}),
			   	eff_user) >= 0
			|| member(({string*})FILED->query_auth(lower_case(tmp)) || ({}),
				eff_user) >= 0
			|| (sizeof(path) > 2 && path[1] == "Gilden" &&
			    member(({string*})FILED->query_auth("gilden") || ({}),
				eff_user) >= 0)
			)
		    )
		    return 1;
                else
		   break;
	    }
	    return 1;
	
	  case "pub":
	    if(sizeof(path)>2 && path[1]=="incoming")
	    {
		return 0;
	    }
			    
	    return 1;


	  // Alles andere darf jeder lesen
	  default:
	    return destfile;
        }
    }
    else
    {
        // /* darf man lesen
        return 1;
    }


    // Player oriented special privileges
    //
    if(!strstr(object_name(previous_object()), "/secure/rpc/") ||
       !strstr(object_name(previous_object()), "/secure/dbus/"))
    {
        // FTPD
        if(!({int})GOETTER_REGISTER->is_wiz(eff_user))
            return 0;
    }
    else if(!this_interactive() ||
            geteuid(this_interactive()) != eff_user ||
            !(level = wizp(this_interactive())))
        return 0;

    // Only wizards after this point.
    // (Gott <eff_user>.)

    // Admin darf alles lesen
    if(adminp(this_interactive()))
        return 1;

    if(sizeof(({string*}) DOMAIN_INFOS->query_domains_of(eff_user)) ||
        member(({string*}) FILED->query_all_auth(), eff_user) >= 0)
    {
        // Lords duerfen /w/ ausser /w/*/priv/ lesen
        if(path[0] == "w" && (sizeof(path) < 3 || path[2] != "priv") &&
           (sizeof(path)<4 || path[1]!="OLD" || path[3]!="priv"))
               return 1;

          // Lords duerfen /var/intern lesen
          if(sizeof(path) > 1 && path[0] == "var" && path[1] == "intern")
           return 1;
    }

    // Normale Goetter:
    if(path[0] == "w")
    {
	// Learner haben noch keine eigene uid, deshalb
	// test auf real_name , so dass sie ihr /w/<real_name> lesen koennen
	if (level == LVL_LEARNER && sizeof(path) > 1 &&
	    (({string})this_interactive()->query_real_name() == path[1]))
	        return 1;
	
	if (valid_read_in_w(file_size(file) == -2 ?path[0..]:path[0..<2],
		level == LVL_LEARNER?({string})this_interactive()->query_real_name():eff_user))
	    return 1;
	    
        // Wenn man Lehrmeister dieses Gott ist, darf man auch dort 
	// lesen, ausser in /w/*/priv
	if(member(({string *})GOETTER_REGISTER->query_lehrmeisters(path[1]), eff_user) >= 0 &&
		  (sizeof(path) < 3 || path[2] != "priv"))
	    return 1;
    }

    return 0;
}
