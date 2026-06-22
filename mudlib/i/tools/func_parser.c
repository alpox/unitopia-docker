// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/i/tools/func_parser.c
// Description: Umwandeln von Format-Strings in Closures
// Author:      Garthan (23.12.93)
// Modified by:	Freaky (22.05.2000) do_fun_call() verbessert

/*
 * der func_parser parst Strings nach Funktionsaufrufen, fuehrt selbige
 * durch, setzt das Ergebnis in den String anstelle des Aufrufs ein und
 * gibt ihn zurueck.
 *
 * Funktionsaufrufe werden an einem fuehrenden $ und einer abschliessenden )
 * erkannt.
 *
 * zB:      "$Der()", "$seinem()",
 *          "$/d/Vaniorh/Tadmor/Strassen/npc/bettler->query_name()"
 *          "$/d/Vaniorh/Tadmor/Laeden/npc/connor->set_title(der Schotte)"
 */
 
#pragma save_types

#include <apps.h>

/*
 * do_fun_call und do_efun_call koennen durch eigene Routinen ueberlagert
 * werden (/i/tools/rollen.c macht es zB)
 */
mixed do_fun_call(mixed ob, string funktion, varargs mixed *parameter)
{
    if ((sizeof(parameter) == 1) && pointerp(parameter[0]))
	parameter = parameter[0];
    return apply(#'call_other,CLOSURE_CONTAINER,"do_call",
	    ob,funktion,parameter);
}

mixed do_efun_call(string funktion, varargs mixed *parameter)
{
    object ob;

    if ((sizeof(parameter) == 1) && pointerp(parameter[0]))
	parameter = parameter[0];
    ob = find_object("/secure/simul_efun/simul_efun") || find_object("/secure/simul_efun/backup/simul_efun");
    return apply(#'call_other,ob,funktion,parameter);
}

string func_parser(string str)
{
    string *parts, param, fun, ob, out;
    int a;
    mixed ret;

    if (!str)
	return 0;
    parts = regexplode(str,"\\$[^\\(]*\\([^\\)]*\\)");
    out = "";

    for (a = 0; a < sizeof(parts); a++)
    {
	param = fun = ob = 0; 
	if (sscanf(parts[a],"$%s(%s)",fun,param))
	{
	    if (sscanf(fun,"%s->%s",ob,fun) == 2)
		ret = do_fun_call(ob,fun,strlen(param) && param);
	    else
		ret = do_efun_call(fun,strlen(param) && param);
	}
	else
	    ret = 0;
	if (ret && stringp(ret))
	    out += ret;
	else
	    out += parts[a];
    }
    return out;
}
