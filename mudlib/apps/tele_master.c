// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/apps/tele_master.c
// Description: Server fuer das Zauberstab-Kommando 'zgehzu'
// Author:	Freaky

// UID: Apps

// Config-File: /var/tele_liste

#include <level.h>
#include <config.h>
#include <apps.h>
#include <touch.h>

mapping teles;
mapping invers;
static mapping kuerzel=(["g":"gilde","k":"kirche","s":"schaenke","b":"bank",
		  "h":"hafen","p":"post","l":"laden","z":"zentrum"]);
string telestr;


#define TELEPORT "/room/rathaus/teleport"
#define GOTO_FILE "/var/tele_liste"
#define ERR(x) write(x);

// gehoert zu load()
static mixed do_ins(string str, mapping map, string was, mapping rest)
{
    mixed ret;

    if (!rest[str]) {
	if (!(ret=map[was+str]))
	    ERR("FEHLER: "+str+" nicht gefunden.\n");
	return map_indices(ret,"do_ins",this_object(),map,was+str+" ",ret);
	}
    else
	return rest[str];
}

// gehoert zu all_teles()
// und zu load()  (caching fuer all_teles)
private string do_dump(string *str, mapping map, string tel,
		       object kugel, mixed lord_or_domainname)
{
    mixed tmp;
    int i, is_lord, all_domains;
    string ret;
    string *domains;

    domains = ({});
    if(objectp(lord_or_domainname) && (is_lord = lordp(lord_or_domainname)))
    {
       domains = map(
           DOMAIN_INFOS->query_domains_of(lord_or_domainname->query_real_name()),
	   (: "/d/"+$1+"/" :)) +
		 map(
	   FILED->query_auth_of(lord_or_domainname->query_real_name()),
	   (: ($1=="p") ? "/p/" : ("/z/"+capitalize($1)+"/") :));
    }

    if (stringp (lord_or_domainname) &&
        (member (DOMAIN_INFOS->query_domains(),lord_or_domainname) != -1))
        domains = ({"/d/"+lord_or_domainname+"/"});
    all_domains = sizeof(domains) == 0;
    ret="";
    for (i=0; i<sizeof(str); i++)
	if (stringp(tmp=map[str[i]]))
	{
	    if (str[i]=="*")
		str[i]="";
	    // Engel
	    if(kugel && kugel->id("hlp#tool"))
	    {
	        if((all_domains ||
		    sizeof(filter(domains, (: strstr($2, $1)>=0 :), tmp)))
		   &&
		   (kugel->query_ort(tmp)))
		    ret+=tel+str[i]+"\n";
	    }
	    // Goetter
	    else
	    {
		if(!invers[tmp])
		   invers[tmp] = ({});
		invers[tmp] += ({ trim(tel+str[i]) });
	        if((!is_lord ||
		    sizeof(filter(domains, (: strstr($2, $1)>=0 :), tmp)))
		   &&
		   (!kugel || kugel->query_ort(tmp)))
		   ret += sprintf("%-25=s  %-52=s\n",tel+str[i],tmp);
	    }
	}
	else
    	    ret+=do_dump(sort_array(m_indices(tmp),#'>),
    	                 tmp,tel+str[i]+" ",kugel, lord_or_domainname);
    return ret;
}

void update_info()
{
    telestr = do_dump(sort_array(m_indices(teles), #'>), teles, "", 0, 0);
}

// Diese Funktion laedt das Configfile in die globalen Variablen ein
//
void load()
{
    string *sec, *lines, title, alias, path, file;
    int i, j;
    mapping map, tmpmap, rootmap;

    teles=([]);
    invers=([]);
    telestr="";
    /* erstmal File einlesen, alle Leerzeilen und alle Kommentar-Zeilen
     * rausschmeissen..
     * Dann explode auf "\n["
     */
    map=([]);
    file=read_file(GOTO_FILE);
    if (!file)
	return;
    sec=explode(implode(regexp(explode(space(file),"\n"),"^[^#]"),"\n"),"\n[");
    /* bei sec[0] ist noch ein "[" davor (weg damit) */
    sec[0]=sec[0][1..];
    /* erstmal ein Mapping mit allen Sections anlegen (unsortiert) */
    for(i=0; i<sizeof(sec); i++) {
	lines=explode(sec[i],"\n");
	/* Section-Name holen */
	if (!sscanf(lines[0],"%s]",title)) {
	    ERR("Fehler im File : ["+lines[0]+".\n");
	    continue;
	    }
	tmpmap=([]);
	for (j=1; j<sizeof(lines); j++) {
	    if (!sscanf(lines[j],"%s %s",alias,path)) {
		alias=lines[j];
		path=0;
		}
	    tmpmap[alias]=path;
	    }
	map[title]=tmpmap;
	}

    if (!sizeof(rootmap=map["root"])) {
	ERR("FEHLER: kein root.\n");
	return;
	}
    /* nun das Mapping sortieren */
    teles=map_indices(rootmap,"do_ins",this_object(),map,"",rootmap);

    update_info();
}

void create()
{
   load();
}


private string get_eindeutiges_ziel(string * ziele)
{
    ziele -= ({ 0 });
    if (!sizeof(ziele))
        return 0;

    string ziel = ziele[0];

    foreach (string z2 : ziele)
    {
        if (z2 != ziel)
            return 0; // nicht eindeutig
    }
    return ziel;
}

// gehoert zu get_tele
private string search_it(string *strs, mapping map)
{
    if (!sizeof(strs))
        return map["*"];

    string str = strs[0];

    mixed * ziele = ({ map[str] }) - ({ 0 });
    if (!sizeof(ziele))
    {
        // Kein direkter Treffer, daher alle Einträge zusammensuchen, 
        // dessen Keys mit dem Suchstring beginnen.
        // Wir sammeln hier erstmal alle passenden Einträge zusammen, 
        // damit bei Mehrfachtreffern nicht nur der gefunden wird, 
        // der zufällig als erstes im Mapping steht (z.B. "eingang", 
        // "eisbahn", "einhorn" bei Suchstring "ei"). Unten werden 
        // dann alle nach den weiteren Suchstrings durchsucht, und nur
        // wenn es genau einen Treffer gibt wird der zurückgegeben.
        int len = strlen(str) - 1;
        foreach (string key, mixed val : map)
        {
            if (str == key[0..len])
            {
                ziele += ({ val });
            }
        }
    }

    if (!sizeof(ziele))
        return 0;

    if (sizeof(strs)<2)
    {
        // Kein weiterer Suchstring, jetzt muss ein eindeutiges Ziel 
        // gefunden worden sein.
        // Vorher müssen mappings noch per Key "*" (was für 'kein 
        // weiteres Wort' steht) aufgelöst werden. Wenn z.B. nach 
        // "forum" gesucht wird ist hier ein Mapping enthalten mit 
        // dem Key "*" für nur "forum" und dem Key "gallien" für 
        // "forum gallien".
        ziele = map(ziele, function string(mixed zielmap)
        {
            if (stringp(zielmap))
                return zielmap;

            if (mappingp(zielmap))
                return zielmap["*"];

            return 0;
        });
        return get_eindeutiges_ziel(ziele);
    }

    // Mit den restlichen Suchstrings weiter in die Tiefe suchen
    strs = strs[1..];
    ziele = map(ziele, function string(mixed zielmap)
    {
        return mappingp(zielmap) ? search_it(strs, zielmap) : 0;
    });

    return get_eindeutiges_ziel(ziele);
}

// INTERFACE:
// Liefert den Filenamen eines Anflugzieles 
// (das umgekehrte zu is_tele)
string get_tele(string str)
{
    string *strs, tmp;

    if (!str)
	return 0;
    strs = explode(convert_umlaute(str)," ") - ({""});
    if (!sizeof(strs))
	return 0;
    if (tmp = kuerzel[strs[0]])
	strs[0] = tmp;

    return search_it(strs,teles);
}

// INTERFACE:
// Liefert String aller Teleportziele
// fuer Zauberstab und Engelskugel (hlptool)
varargs string all_teles(object kugel, mixed lord_or_domainname)
{
    if(kugel || lord_or_domainname)
       return do_dump(sort_array(m_indices(teles),#'>),teles,"", kugel, lord_or_domainname);
    return telestr;
}

// Hilfsfunktion fuer filter_teles
private string * do_filter(string* keys, mapping tmp, string str)
{
    int i,j,k;
    string *result = ({});
    string *split = explode(space(str)," ");
    mixed tstr;
    for (i=0;i<sizeof(keys);i++)
    {
        if (strstr(keys[i],str)!=-1)
        {
            result += ({ keys[i] });
            continue;
        }
        for (j=0,k=0;j<sizeof(split);j++) 
        {
            if (strstr(keys[i],split[j])!=-1)
            {
                k++;
            }
        }
        if (k==sizeof(split)) 
        {
            result += ({ keys[i] });
            continue;
        }
        if (stringp(tstr = tmp[keys[i]]))
        {
            if (strstr(tstr,str)!=-1)
            {
                result += ({ keys[i] });
                continue;
            }
        }
        else
        {
            if (sizeof(do_filter(m_indices(tstr),tstr,str)))
            {
                result += ({ keys[i] });
                continue;
            }
        }
    }
    return result;
}

// INTERFACE:
// Liefert den String der gefilterten Teleportziele fuer den Zauberstab.
varargs string filter_teles(string str)
{
    if (stringp(str) && sizeof(str))
    {
        string * result = do_filter(m_indices(teles),teles,str);
        return do_dump(sort_array(result,#'>),teles,"", 0, 0);
    }
    return telestr;
}

// INTERFACE:
// Liefert zu einem Filenamen ein Array aller Anflugzielnamen
// (das umgekehrte zu get_tele)
string *is_tele(string str)
{
    return invers[str];
}

// Testfunktion, welche Zielraeume gibt es nicht mehr?
string *check_teles()
{
   int i;
   string *rooms, *res;

   for(res = ({}), i = sizeof(rooms = m_indices(invers)); i--;)
      if(file_size(rooms[i] + ".c") < 1 && !touch(rooms[i],NO_LOG|NO_WRITE))
         res += rooms[i..i];
   return res;
}


// fuer subtree
private void insert(mapping target, string section, mixed *value)
{
   if(!target[section])
      target[section] =  ({ value });
   else 
      target[section] += ({ value });
}

// fuer save()
private void subtree(mapping target, mapping teles, string section)
{
   int i;
   string *idxs;
   mixed sub;
   string subsect;

   for(i = sizeof(idxs = m_indices(teles)); i--;)
      if(mappingp(sub = teles[idxs[i]]))
      {
	 subsect = (section != "root" ? section + " " : "") + idxs[i];
	 insert(target, section, ({idxs[i], "" }));
	 subtree(target, sub, subsect);
      }
      else
	 insert(target, section, ({idxs[i], sub }));
}

// Abspeichern der Teleliste
//
void save()
{
   mapping target;
   string *idxs;
   string save;
   int i, j;
   mixed *values;

   save =
   "# Die Liste für das 'zgehzu'-Kommando\n"
   "# Automatisch erstellt von "+__FILE__+" am "+shorttimestr(time())+".\n"
   "#\n";
   for(i = sizeof(idxs = sort_array(m_indices(kuerzel),#'<)); i--;)
      save += "# "+idxs[i]+" "+kuerzel[idxs[i]]+"\n";

   target = ([]);
   subtree(target, teles, "root");
   idxs = sort_array(m_indices(target), #'<);
   idxs -= ({ "root" });
   idxs += ({ "root" });
   for(i = sizeof(idxs); i--;)
   {
      save += "\n["+idxs[i]+"]\n";
      values = sort_array(target[idxs[i]], 
	 lambda(({'a,'b}), ({ #'<, ({ #'[, 'a, 0 }), ({ #'[, 'b, 0 }) }) ));
      for(j = sizeof(values); j--;)
	 save += values[j][0] +
		 (values[j][1] != "" ? "\t"+
				(strlen(values[j][0]) < 8 ? "\t" : "") +
				values[j][1] : "")+"\n";
   }
   rm(GOTO_FILE);
   write_file(GOTO_FILE, save);
   load (); // invers neu berechnen
}


// Testfunktion, dont use it!
mapping query_teles()
{
   return deep_copy(teles);
}

private int secure()
{
   return object_name(previous_object()) == TELEPORT;
}

int add_tele(string name, string filename)
{
   string *parts;
   mapping submap;
   int i;

   if(!secure())
      return -10;
   if(!stringp(name) || !stringp(filename))
      return -1;
   if(file_size(filename+".c") < 1 && !touch(filename, NO_LOG|NO_WRITE))
      return -2;
   if(!sizeof(parts = explode(trim(lower_case(name)), " ")-({""})))
      return -3;

   for(submap = teles, i = 0; i < sizeof(parts)-1; i++)
   {
      if(stringp(submap[parts[i]]))
         return -4;
      if(!mappingp(submap[parts[i]]))
         submap[parts[i]] = ([]);
      submap = submap[parts[i]];
   }
   if(mappingp(submap[parts[<1]]))
      return -5;
   submap[parts[<1]] = filename;

   save();
   update_info();
}

int delete_tele(string name)
{
   string *parts, *ziele;
   mapping submap;
   mapping *submaps;
   int i;

   if(!secure())
      return -10;
   if(!stringp(name))
      return -1;
   if(name != "" && name[0] == '/' && strstr(name, " ") < 0 &&
      (i = sizeof(ziele = is_tele(name))))
   {
      for(;i--;)
         if(delete_tele(ziele[i]) == -5)
            delete_tele(ziele[i]+" *");
      return 0;
   }
   if(!sizeof(parts = explode(trim(lower_case(name)), " ")-({""})))
      return -3;
   submap = teles;
   submaps = ({});
   for(submap = teles, i = 0; i < sizeof(parts)-1; i++)
   {
      submaps += ({ submap });
      if(mappingp(submap[parts[i]]))
         submap = submap[parts[i]];
      else if(stringp(submap[parts[i]]))
         return -4;
   }
   if(mappingp(submap[parts[<1]]))
      return -5;
   if(!stringp(submap[parts[<1]]))
      return -6;
   m_delete(submap, parts[<1]);
   for(i = sizeof(submaps); i--;)
      if(!sizeof(submaps[i][parts[i]]))
         m_delete(submaps[i], parts[i]);

   save();
   update_info();
}

// gehoert zu change_filename()
//
private int do_change(mapping submap, string old, string new)
{
   int i, res;
   string *idxs;

   for(res =0, i = sizeof(idxs = m_indices(submap)); i--;)
      if(mappingp(submap[idxs[i]]))
         res += do_change(submap[idxs[i]], old, new);
      else if(submap[idxs[i]] == old)
      {
         submap[idxs[i]] = new;
         res++;
      }
   return res;
}

int change_filename(string old, string new)
{
   if(!secure())
      return -10;
   if(!stringp(old) || !stringp(new))
      return -1;
   if(file_size(new+".c") < 1 && !touch(new, NO_LOG|NO_WRITE))
      return -2;
   if(do_change(teles, old, new))
   {
      save();
      update_info();
      return 0;
   }
   return -3;
}
void prepare_renewal()
{
    save();
}

void abort_renewal()
{
}

void finish_renewal(object neu)
{
}