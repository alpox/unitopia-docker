// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/apps/town.c
// Description: Konvertiere Ip-Name in Stadt-Name
// Author:	Freaky

#undef LOGGING
#undef DEBUG

#define TOWN_FILE "/static/adm/TOWN"
#define TOWN_FILE_TONLINE "/static/adm/TOWN-TONLINE"

#ifdef LOGGING
#define ERR sys_log("TOWN","STR:" + str + ": STRN:" + strn + ":\n");
#define DO_ERR { ERR; return "Unbekannt"; }
#else
#define ERR {}
#define DO_ERR return "Unbekannt";
#endif

#define JUNK ({"uni","fh","tu","fu","hu","th","lrz","kfa","ba","fernuni", \
	"rwth","ku","htw","fht","tfh","unibw","fz","mu","hbi","dkfz"})

#include <level.h>
#if __EFUN_DEFINED__(interactive_info)
#include <interactive_info.h>
#endif

private mapping towns, alias, domains, tonline;

void create()
{
    string *lines, ip, str, descr, file;
    int i;

    towns = m_allocate(1,2);
    alias = ([]);
    tonline = ([]);
    domains = m_allocate(1,2);
    file = read_file(TOWN_FILE);
    if (!file)
	return;
    lines = explode(file,"\n") - ({""});
    for (i = 0; i < sizeof(lines); i++)
    {
	if (lines[i][0] == '#')
	    continue;
	if (sscanf(space(lines[i]),"%s %s",ip,str) != 2)
       	{
	    write("Error Line " + (i + 1) + ": '" + lines[i] + "'\n");
	    continue;
	}
	if (ip[0] == '.')
	{
	    sscanf(str,"%s %s",str,descr);
	    domains[ip[1..]] = str;
	    domains[ip[1..],1] = descr;
	}
	else if (member(ip,'.')>0)
       	{
	    sscanf(str,"%s %s",str,descr);
	    towns[ip] = str;
	    towns[ip,1] = descr;
	}
	else
	    alias[ip] = str;
    }
    file = read_file(TOWN_FILE_TONLINE);
    if (file) {
        lines = explode(file,"\n") - ({""});
        for (i = 0; i < sizeof (lines); i++) {
            if (lines[i][0] == '#')
                continue;
            if (sscanf (space (lines[i]),"%s %s",ip,str) != 2) {
                write("Error line "+ (i+1)+": '"+lines[i]+"'\n");
                continue;
            }
            tonline[lower_case(ip)] = str;
        }
    }
}

varargs string town(mixed ob, int flag)
{
    string str, strn, junk, tmp, town, *strs;
    int a, b, c;

    if (!adminp(this_interactive()) ||
	    geteuid(previous_object()) != geteuid(this_interactive()))
    {
	return "";
//	flag = 0;
    }

    if (stringp(ob))
    {
#ifdef UNItopia
        if ((ob == "sissi") || (ob == "Sissi")) return "Berlin";
#endif
	if (to_int(ob))
	    strn = ob;
	else
	    str = ob;
    }
    else if (objectp(ob) && efun::interactive(ob))
    {
#ifdef UNItopia
        if (ob->query_real_name() == "sissi") return "Berlin";
#endif
#if __EFUN_DEFINED__(query_ip_number)
	str = ob->query_real_ip_name(ob) || efun::query_ip_name(ob);
	strn = ob->query_real_ip_number(ob) || efun::query_ip_number(ob);
#else
	str = ob->query_real_ip_name(ob) || efun::interactive_info(ob, II_IP_NAME);
	strn = ob->query_real_ip_number(ob) || efun::interactive_info(ob, II_IP_NUMBER);
#endif
    }
    else
	return 0;

    if (str && ((str [<17..] == ".dip.t-dialin.net")
             || (str [<20..] == ".dip0.t-ipconnect.de"))) {
        string temp = lower_case(str [..<18]);
        for (int i = strlen(temp)-1; i >= 3; i--)
            if (town = tonline[temp[0..i]])
                return flag ? "Deutsche Telekom AG - "+town : town;
        return flag ? "Deutsche Telekom AG" : "Deutschland";
    }

    if (strn)
    {
	if (sscanf(strn,"%d.%d.%d.%~d",a,b,c) != 4)
	    DO_ERR;
	
	if (town = towns[strn,flag])
	    return town;
	if (town = towns[a + "." + b + "." + c,flag])
	    return town;
	if (town = towns[a + "." + b,flag])
	    return town;
    }

    if (str)
    {
	strs = explode(lower_case(str),".");
	if (sizeof(strs) == 1)
	    strs += explode(__DOMAIN_NAME__,".");

	if (strs[<1] == "de")
	{
	    if ((sscanf(strs[<2],"%s-%s",junk,town) == 2) &&
		    (member(JUNK,junk) >= 0))
	    {
		if (tmp = alias[town])
		    return tmp;
		return capitalize(town);
	    }
	    if (tmp = alias[strs[<2]])
		return tmp;
	}
    }

    ERR;

    if (str)
    {
	for (int e = 1; e < sizeof(strs); e++ )
	{
	    if (tmp = domains[implode(strs[e..],"."),flag])
	    {
		return tmp;
	    }
	}
    }
    return "Unbekannt";
}

string description(mixed ob)
{
    return town(ob,1);
}

#ifdef DEBUG
mapping query_towns() { return towns; }
mapping query_alias() { return alias; }
mapping query_domains() { return domains; }
#endif
