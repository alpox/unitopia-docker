// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/i/player/files.c
// Description: Pfadkuerzel und cd-Befehl
// Author:	Freaky (23.12.93)

#pragma save_types
#pragma strong_types

#include <config.h>
#include <more.h>

#undef DEBUG

#define SECURE if (extern_call() && (!this_interactive() || \
	this_player() != this_interactive() || \
	this_interactive() != this_object() || \
	geteuid(this_interactive()) != geteuid(this_object()))) \
	return 0
#define HELP(x) SECURE; if (str == "?") \
	{ cat(HELP_PATH + "/goetter/" + x); return 1; }
#define DIR 1
#define LAST 2
#define MSG 4
#define MAX_PATHALIASES 100

#define PATH_ALIASES_FILE "/w/" + query_real_name() + "/.pfadkuerzel"

// Prototyping
string query_real_name();
varargs mixed query_eye_option(string option);

private varargs mapping read_path_aliases(string delete);

private int ed_setup;
private static mapping path_aliases;
private static string *path;

int query_ed_setup() { return ed_setup; }
void set_ed_setup(int i) { ed_setup = i; }

static void init_current_path()
{
    path = ({ "w", query_real_name() });
    path_aliases = read_path_aliases();
}

/*        
FUNKTION: compose_path
DEKLARATION: nomask string *compose_path(string path, int flag)
BESCHREIBUNG:
Bastelt aus dem uebergebenen Pfad - String einen "vollstaendigen, richtigen
Pfad", und liefert ihn als String - Array zurueck, bestehend aus den
einzelnen Pfad-Teilen.
Dabei werden beruecksichtigt:

 - ~/      Home-Verzeichnis des Gottes, bei Spielern kommt da auch was
           raus, aber das ist nicht wirklich sinnvoll,
 - .       Das aktuelle Verzeichnis, wo man grad ist,
 - ..      eine Verzeichnisebene weiter in Richtung Wurzel,
 - Pfadkuerzel des Gottes,

Flag ist eine Veroderung von MSG, DIR und LAST aus /i/zauberstab/zdefs.h.
Dabei bedeuten:
 - DIR  (1):  gib nur den Teil des Pfades zurueck, der auch existiert,
              und auch nur den Teil, der ein Verzeichnis ist, schneidet
              also einen am path anhaengenden Dateinamen ab,
 - LAST (2):  der letzte Teil des Pfades (gewoehnlich der Dateiname)
              muss nicht existieren, d.h. bei Angabe von MSG wird
	      keine Fehlermeldung ausgegeben, wenn nur das uebergeordnete
	      Verzeichnis existiert.
 - MSG  (4):  gib ne Fehlermeldung aus, wenn die Datei nicht existiert,

VERWEISE: 
GRUPPEN: spieler  
*/


nomask string *compose_path(string pa, int flag)
{
    string *pfad, tmp, *tp;
    <string|int>* dir;
    int i, j, a;

    if (!pa || pa=="")
	return path;
    pfad = explode(pa,"/") - ({"","."});

    // Den Anfang (tp) finden.
    // (Also den Pfad, zu welchem 'pa' relativ angegeben wurde.)
    if (pa[0]=='/')
	tp = ({});
    else if (!sizeof(pfad))
	tp = path;
    else if (pa[0..1] == "~/" || pa == "~")
    {
	tp = ({"w",query_real_name()});
	i = 1;
    }
    else
    {
	while(i < sizeof(pfad) && pfad[i] == "..")
	    i++;
	if (i > 0 || pa[0..1] == "./")
	    tp = path[0..<i + 1];
	else
       	{
	    tmp = path_aliases[pfad[0]];
	    if (tmp)
	    {
		if (pfad[0] != "-")
		    dir = get_dir(implode(({""}) + path,"/") + "/" + pfad[0]);
#ifdef DEBUG
    printf("tmp:%O:\npath:%O:\npf:%O:\ndir:%O:\n",tmp,path,implode(({""})+path,"/")+"/"+pfad[0],dir);
#endif
		if (sizeof(dir))
		    tp = path;
		else
	       	{
		    if ((flag & DIR) && file_size("/" + tmp) != -2)
		    {
			if (flag & MSG)
		    	    write("Das Verzeichnis /" + tmp +
				    " existiert nicht.\n");
			return ({});
		    }
		    tp = explode(tmp,"/") - ({""});;
		    i = 1;
		}
	    }
	    else
		tp = path;
	}
#ifdef DEBUG
	printf("tp:%O:\npath:%O:\ni:%O:\n",tp,path,i);
#endif
    }

    if (flag & DIR)
	a = sizeof(pfad);
    else
	a = sizeof(pfad) - 1;

    // tp ist unserer aktueller (bereits ueberpruefter) Pfad.
    // i ist unsere Position innerhalb von pfad.
    // a ist die Anzahl zu verarbeitender Teile von pfad.
    
    tmp = implode(({""}) + tp,"/") + "/";
    for (; i < a; i++)
    {
	if(pfad[i]==".")
	    continue;
	else if(pfad[i]=="..")
	{
	    tp = tp[0..<2];
	    tmp = implode(({""}) + tp,"/") + "/";
	    continue;
	}
	
	dir = get_dir(tmp + pfad[i],3);
	if(sizeof(dir) && dir[0]==".") dir=dir[2..<1];
	if(sizeof(dir) && dir[0]=="..") dir=dir[2..<1];
	if (!sizeof(dir))
       	{
	    if (flag & MSG)
		if (!dir)
		{
		    if(member(pfad[i],' ')<0)
		        write("Das Verzeichnis " + tmp +
		            ((file_size(tmp+pfad[i])==-2)?(pfad[i]+"/"):"") +
			    " darfst du nicht lesen.\n");
		    else
		        write("Ungültiger Verzeichnisname \"" +
		            pfad[i] + "\".\n");
		}
		else
		    write("Das Verzeichnis " + tmp + pfad[i] +
			    " existiert nicht.\n");
	    if (flag & DIR)
		return tp;
	    else
		return tp + pfad[i..];
	}

	if ((j = member(dir,-2)) >= 0)
       	{
	    tmp += dir[j - 1] + "/";
	    tp += ({ dir[j - 1] });
	}
	else
       	{
	    if (flag & MSG)
		write("Das Verzeichnis " + tmp + pfad[i] +
			" existiert nicht.\n");
	    if (flag & DIR)
		return tp;
	    else
		return tp + pfad[i..];
	}
    }
    if (flag & DIR)
	return tp;

    if (i > a)
	return tp;

    if (pfad[<1]==".")
	return tp;
    if (pfad[<1]=="..")
	return tp[0..<2];

    if (flag & LAST)
	    return tp + ({ pfad[<1] });

    if (!(dir = get_dir(tmp + pfad[<1])) || !sizeof(dir=dir-({".",".."})))
    {
	if (flag & MSG)
	    write("Das File " + tmp + pfad[<1] + " existiert nicht.\n");
	return tp + ({ pfad[<1] });
    }

    return tp + ({ dir[0] });
}

nomask int set_current_path(string pfad)
{
    SECURE;
    if (stringp(pfad))
    {
	path = compose_path(pfad,DIR);
	this_object()->update_directory();
	return 1;
    }
    return 0;
}

nomask string query_current_path()
{
    return "/" + implode(path,"/");
}

nomask varargs string add_path(string str, int flag)
{
    return "/" + implode(compose_path(str,flag),"/");
}

private varargs mapping read_path_aliases(string delete)
{
    string file, *lines, tmp, *path;
    int rewriteflag;
    mapping ret = ([:1]);

    file = read_file(PATH_ALIASES_FILE);

    if(!file)
	return ret;

    lines = explode(file, "\n");

    for(int line = sizeof(lines); line--; )
    // rueckwaerts durchgehen, damit bei doppelten Eintraegen
    // der neueste verwendet wird
    {
	tmp = space(lines[line]);

	if(!strlen(tmp) || tmp[0] == '#')
	    continue;

	path = explode(tmp," ");

	if(sizeof(path) != 2)
	{
	    write("Falscher Pfadkürzel-Eintrag in Zeile "+(line+1)+".\n");
            lines[line] = "#" + lines[line];
            rewriteflag = 1;
	    continue;
	}

	// ist es zufaellig ein zu loeschender Eintrag?
	if(delete && path[0]==delete) 
        {
	    lines[line] = 0;
	    rewriteflag = 1;
	    continue;
	}

	// nur, wenn der Eintrag noch nicht vorhanden ist, hinzufuegen.
	// andernfalls loeschen.
	if (ret[path[0]]) 
        {
            lines[line] = 0;
	    rewriteflag = 1;
	}

	else
        {
	    ret[path[0]] = path[1][1..];

            if(!valid_file_name(path[1]))
                write("Pfadkürzel "+path[0]+" "+path[1]+": " 
                      "Verzeichnis existiert nicht.\n");
        }
    }

    if(rewriteflag) 
    {
        if(rm(PATH_ALIASES_FILE) == 1)
            write_file(PATH_ALIASES_FILE, implode(lines-({0}), "\n"));
    }

    return ret;
}

nomask int pk(string str)
{
    int i;
    string *tmps, *out, kurz, lang;

#ifdef DEBUG
    printf("%O\n",path_aliases);
#endif
    HELP("pk")
    if (!str)
    {
	out = ({ "Deine Pfadkürzel:" });
	tmps = sort_array(m_indices(path_aliases),#'>);
	for (i = 0; i < sizeof(tmps); i++)
	    out += ({ sprintf("%-10s /%s",tmps[i],path_aliases[tmps[i]]) });
	this_object()->more(out,0,0,M_AUTO_END);
    }
    else if (str == "-r")
    {
	path_aliases = read_path_aliases();
	write("Ok.\n");
	return 1;
    }
    else if (sscanf(space(str),"%s %s",kurz,lang) == 2)
    {
        int reread;
        if (path_aliases[kurz]) reread = 1;
        lang = add_path(lang,MSG);
	path_aliases[kurz] = lang[1..];
	write_file(PATH_ALIASES_FILE,sprintf("%-10s %s\n",kurz,lang));
        if (reread)
            path_aliases = read_path_aliases();

    }
    else if (path_aliases[str])
	printf("%-10s /%s\n",str,path_aliases[str]);
    else
	write(str + " ist kein Pfad-Kürzel!\n");

    return 1;
}

nomask int unpk(string str)
{
    HELP("unpk")
    if (!str)
    {
        write ("Welches Pfadkürzel möchtest Du löschen?\n"
               "Syntax: unpk <name des zu löschenden Pfadkuerzels\n");
    }
    else if (path_aliases[str]) {
	printf("Pfadkürzel %s für %s wird entfernt.\n",
	    str, path_aliases[str]);
	path_aliases = read_path_aliases (str);
    }
    else
	write(str + " ist kein Pfad-Kürzel!\n");
    return 1;
}

int pwd(string str)
{
    HELP("pwd");
    write("/" + implode(path,"/") + "\n");
    return 1;
}

// wird auch vom Zauberstab (vom "zcd" - Befehl aus) aufgerufen
int cd(string str)
{
    string tmp, fil;
    object *cs = caller_stack(0);
    if (!this_interactive() 
        && sizeof(cs)==3 && sizeof(cs-({this_object()}))==0)
    {
        if (str == "../") str = "..";
    }
    else
    {
        HELP("cd");
    }
    tmp = implode(path,"/");
    if (str && strstr(str,"...") >= 0)
       return notify_fail("... ist nicht erlaubt als Verzeichnisname.\n");

    if (!str)
	init_current_path();
    else if (str == "hier" && environment())
    {
	fil = (fil = map2domain(object_name(environment()),1)) ? fil :
		object_name(environment());
	path = explode(fil,"/")[1..<2];
    }
    else
	path = compose_path(str,DIR | MSG);

    write("/" + implode(path,"/") + "\n");
    path_aliases["-"] = tmp;

    if (query_eye_option("dirinfo"))
        if (!sizeof(path))
	    this_object()->more("/.info","--Mehr--",0,M_AUTO_END);
        else
	    this_object()->more("/" + implode(path,"/") + "/.info",
		    "--Mehr--",0,M_AUTO_END);
    this_object()->gmcp_send_dir("/" + implode(path,"/"));
    this_object()->update_directory();
    return 1;
}

/*        
FUNKTION: query_path_aliases
DEKLARATION: mapping query_path_aliases()
BESCHREIBUNG:
Damit bekommt man ein mapping, in dem alle Pfadkuerzel eines Spielers stehen.
Wird von der Wizard - Shell benoetigt.
VERWEISE: 
GRUPPEN: spieler  
*/
mapping query_path_aliases()
{
    if (wizardshellp(previous_object()))
	return path_aliases;
}

static void set_path_aliases(mapping pa)
{
    if (mappingp(pa))
	path_aliases = pa;
}
