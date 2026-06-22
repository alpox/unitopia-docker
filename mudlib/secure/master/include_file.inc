// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/secure/master/include_file.inc
// Description:	Regelt die #include-Direktive
// Author:	Freaky (13.11.1999)
// Modified by:	Freaky (13.01.2000) rel_path und search_file eingebaut

#pragma strict_types

// Wird von include_file() und inherit_file() verwendet
private string rel_path(string file, string compiled_file)
{
    string map, *co;
    int i;

    // Wenn das File mit einem / anfaengt einfach so zurueckgeben.
    if (file[0] == '/')
        return file;

    // Wenn es ein Map-Raum ist, dann aufloesen.
    if (map = funcall(symbol_function("map2domain"),compiled_file))
        compiled_file = map;
    else
    {
        int slashpos = strrstr(compiled_file, "/");
        string dir=compiled_file[0..slashpos];
        object vc =find_object(dir+"virtual_compiler");
	
	
        if (vc && stringp(map =
            ({string})vc->real_filename(compiled_file[slashpos+1..<1],dir)))
            compiled_file = map;
    }

    // Jetzt relative Pfade machen
    if (file[0..2] == "../")
    {
        string *file_path = explode(file,"/");
        co = explode(compiled_file,"/");

        while (file_path[i] == "..")
            i++;

        if (i > sizeof(co) - 3)
            return "/" + implode(file_path[i..],"/");

        return implode(co[0..<(i + 2)],"/") + "/" + implode(file_path[i..],"/");
    }

    return implode(explode(compiled_file,"/")[0..<2],"/") + "/" + file;
}

// Wird von include_file() und inherit_file() verwendet
private string search_file(string file, string compiled_file)
{
    string tmp, *co;
    int i;

    // Wenn es ein Map-Raum ist, dann aufloesen.
    if (tmp = funcall(symbol_function("map2domain"),compiled_file))
        compiled_file = tmp;
    else
    {
        int slashpos = strrstr(compiled_file, "/");
        string dir=compiled_file[0..slashpos];
        object vc =find_object(dir+"virtual_compiler");
	
	
        if (vc && stringp(tmp =
            ({string})vc->real_filename(compiled_file[slashpos+1..<1],dir)))
            compiled_file = tmp;
    }

    co = explode(compiled_file,"/");
    // Wenn in config.h ein #include <config.h> gemacht wird, dann die
    // erste Ebene ueberspringen (also bei i = sizeof(co) - 3 anfangen)
    // Sonst gibt es ein rekurives include.
    i = sizeof(explode(file,"/"))-1;
    if (file == "/" + implode(co[<i..],"/"))
        i = sizeof(co) - i - 1;
    else
        i = sizeof(co) - 1;
    for (; i-- > 1; )
        if (file_size(tmp = implode(co[0..i],"/") + file) >= 0)
            return tmp;
    if (file_size(file) >= 0)
        return file;
    return 0;
}

#define NO_INCLUDE_DIRS ({ "/INFO/", "/doc/", "/save/", \
	"/var/", "/static/", "/priv/", "/log/", "/Old/", "/OLD/" })

// Fremde duerfen nix aus diesem Verzeichnis erben
// In /secure oder /room haben fremde nix zu suchen.
// Files aus /w duerfen nur von Files aus /w included werden.
#define NO_INCLUDE_TOPLEVEL_DIRS ({ "/room/", "/w/" })
// "/secure/" noch nicht, zuviele Probleme innerhalb der Mudlib.

// Nur Dateien aus diesen Verzeichnissen duerfen andere
// Dateien aus diesen Verzeichnissen includen.
#define SECURE_TOPLEVEL_DIRS ({ "/apps/", "/i/", "/obj/", "/room/", "/secure/" })
// Folgende darf jeder includen:
#define SECURE_EXCEPTIONS ({ "/i/hlp/lander.inc", \
			     "/i/object/soul_commands.inc", \
			     "/i/zauberstab/zdefs.h", \
			     "/secure/simul_efun.h", \
			  })
  

int no_include_file(string inc_file, string comp_file)
{
    foreach(string dir : NO_INCLUDE_DIRS)
	if (strstr(inc_file,dir) >= 0)
	    return 1;

    foreach(string dir : NO_INCLUDE_TOPLEVEL_DIRS)
	if (!strstr(inc_file, dir) && strstr(comp_file, dir))
	    return 1;

    foreach(string dir : SECURE_TOPLEVEL_DIRS)
	if (!strstr(inc_file, dir))
	{
	    int secure;

	    if(member(SECURE_EXCEPTIONS, inc_file)>=0)
		break;
	    
	    foreach(string cdir: SECURE_TOPLEVEL_DIRS)
		if (!strstr(comp_file, cdir))
		{
		    secure = 1;
		    break;
		}
	
	    if(!secure)
		return 1;

	    break;
	}

    if (strstr(inc_file,"/Admin/") >= 0)
    {
	// /z/Raetsel/Admin/ darf Files daraus auch includen
	if (strstr(inc_file,"/z/Raetsel/Admin/") == 0 &&
		strstr(comp_file,"/z/Raetsel/Admin/") == 0)
	    return 0;

	// Geheime Raetsel-Includes:
	if (strstr(inc_file,"/z/Raetsel/Admin/Secure/") == 0)
	{
	    if (strstr(comp_file,"/z/Raetsel/") == 0)
	    {
		// apps (Raetsel-Objekte) duerfen das includen.
		if (strstr(comp_file,"/z/Raetsel/apps/") == 0)
		    return 0;
		// /z/Raetzel/AAA/... darf /z/Raetsel/Admin/Secure/AAA/...
		// includen
		if (explode(inc_file,"/")[5] ==
			explode(comp_file,"/")[3])
		    return 0;
	    }
	}

	// /z/Spiele/Admin/ darf Files daraus auch includen
	if (strstr(inc_file,"/z/Spiele/Admin/") == 0 &&
		strstr(comp_file,"/z/Spiele/Admin/") == 0)
	    return 0;

	// Geheime Spiele-Includes:
	if (strstr(inc_file,"/z/Spiele/Admin/Secure/") == 0)
	{
	    if (strstr(comp_file,"/z/Spiele/") == 0)
	    {
		// apps (Spiele-Objekte) duerfen das includen.
		if (strstr(comp_file,"/z/Spiele/apps/") == 0)
		    return 0;
		// /z/Spiele/AAA/... darf /z/Spiele/Admin/Secure/AAA/...
		// includen
		if (explode(inc_file,"/")[5] ==
			explode(comp_file,"/")[3])
		    return 0;
	    }
	}
	return 1;
    }
}

/*
    mixed include_file (string file, string compiled_file, int sys_include)

        Arguments:
          previous_object(): The object causing the compile.
          file             : The name given in the #include directive.
          compiled_file    : The object file which is just compiled.
          sys_include      : TRUE for #include <> directives.

        Result:
          0: use the normal include filename generation (""-includes are
             used as they are, <>-includes are handled according to
             H_INCLUDE_DIRS).

          <path>: the full absolute pathname of the file to include, without
             leading "/" and without parentdir parts ("/../").

          else: The include directive is not legal.
*/
mixed include_file(string file, string compiled_file, int sys_include)
{
    string ret;
    int check_exist = 0;

    // Nur Files mit der Endung '.h' oder '.inc' zulassen
    if (file[<2..] != ".h" && file[<4..] != ".inc")
        return 1;

    if (file[0..0] == "?")
    {
        file = file[1..];
        check_exist = 1;
    }
    if (sys_include)
    {
        // Wenn es ein #include <...> ist, dann das File suchen
        ret = search_file("/sys/" + file,compiled_file);
        if (!ret)
        {
            if (!strstr(file, "p/") && file_size("/p/sys/" + file[2..])>=0)
                ret = "/p/sys/"+file[2..];
            else if (file_size(strstr(file,"/")?"/"+file:file)>=0)
                do_log_error("#include: Vollst\u00e4ndigen Dateinamen in \"\" "
                             "statt <> setzen.\n",
                             compiled_file, compiled_file, 
                             to_int(expand_define("__LINE__")),0,1);
        }
    }
    else
    {
	// sonst relativen Pfad auswerten
	ret = rel_path(file,compiled_file);
        if (no_include_file(ret,compiled_file))
            return 1;
    }
    
    if (check_exist)
    {
        if (stringp(ret) && file_size(ret)>=0)
        {
            return ret;
        }
        return "/secure/master/dummy.h"; // Leere Datei...
    }

    return ret;
}
