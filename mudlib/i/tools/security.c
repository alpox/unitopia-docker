// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/i/tools/security.c
// Description:	Sicherheitsabfragen (Trusted Path)
// Author:	Gnomi

#include <security.h>
#include <uids.h>
#include <config.h>

static private closure *conditions=({});
static private <object|lwobject> *objects=({});
static private string *dirs=({});
static private mapping euids=([ROOT_UID]), obj_names=([:0]);

/*
FUNKTION: add_security_condition
DEKLARATION: protected void add_security_condition(closure condition|object ob|lwobject lwo|string obj_name|string euid)
BESCHREIBUNG:
Hiermit gibt man an, welchen Objekten man vertraut.
Folgende Angaben sind moeglich:

    Eine Closure:	 Diese Closure wird mit dem Objekt als Parameter
                         aufgerufen. Liefert sie 1, wird dem Objekt vertraut.
    Eine EUID:    	 Jedem Objekt mit dieser EUID wird vertraut.
    Ein Objekt:		 Diesem Objekt wird vertraut (bis es zerstoert wird).
    Ein LW-Objekt:   Diesem Lightwight-Objekt wird vertraut.
    Eine Objektname:	 Diesem Objekt wird vertraut. Endet der String
			 mit einem "#" so wird den Clones vertraut.
    Ein Verzeichnisname: Dieser String muss mit einem "/" enden. Alle Objekte
			 in diesem Verzeichnis und seine Unterverzeichnisse
			 werden als vertrauenswuerdig angesehen.

Folgende Bedingungen sind vordefiniert:
    
    #'sc_euid_as_ti	 Das Objekt muss dieselbe EUID wie TI haben.
    #'sc_euid_as_ti	 Das Objekt muss dieselbe EUID wie TP haben.
    #'sc_ti		 Das Objekt ist TI.
    #'sc_tp		 Das Objekt ist TP.

VERWEISE: check_security, init_security_for_actions, init_security_trust_mudlib
GRUPPEN: Sicherheit
*/
protected void add_security_condition(mixed which)
{
    if(closurep(which))
        conditions+=({which});
    else if(lwobjectp(which))
        objects+=({which});
    else if(objectp(which))
        objects+=({which});
    else if(stringp(which))
    {
        if(member(which,'/')<0)
            m_add(euids,which);
        else if(which[<1]=='/' || which[<1]=='#')
            dirs+=({which});
        else
            m_add(obj_names, domain2map(which)||which);
    }
}
/*
FUNKTION: check_security
DEKLARATION: varargs int check_security(int flags)
BESCHREIBUNG:
Ueberprueft, ob diese Funktion nur ueber vertrauenswuerdige Objekte aufgerufen
wurde. (Es wird der caller_stack inkl. this_interactive ueberprueft.)
Folgende Flags (definiert in security.h) kann man dabei angeben:

    CHECK_LAST_OBJECT	  Es wird nur das letzte Objekt getestet (welches
			  nicht this_object() ist).
    CHECK_ERROR		  Im Falle eines nicht vertrauenswuerdigen Objektes
			  breche die Ausfuehrung mit einem Fehler
			  (mit raise_error) ab.

Die Funktion liefert 1, wenn es keine Probleme gibt, anderenfalls 0.
Diese Funktion sollte aus Sicherheitsgruenden (wenn man Wert auf ein
korrektes Ergebnis legt...) nicht per call_other (->) aufgerufen werden.
VERWEISE: add_security_condition, init_security_for_actions,
	  init_security_trust_mudlib
GRUPPEN: Sicherheit
*/
varargs int check_security(int flags)
{
    <object|lwobject> *stack = caller_stack(1)-({this_object(),0});

    if(!sizeof(stack))
        return 1;

    if(flags & CHECK_LAST_OBJECT)
	stack = stack[0..0];
	
    stack-=objects;
    
    foreach(<object|lwobject> ob: stack)
    {
	int okay=0;
	if(member(euids, geteuid(ob)))
	    continue;
	string ob_name = lwobjectp(ob)?load_name(ob):object_name(ob);
	if(member(obj_names, ob_name))
	    continue;
	    
	foreach(string dir: dirs)
	    if(!strstr(ob_name,dir))
	    {
		okay = 1;
		break;
	    }
	if(okay)
	    continue;
	    
	foreach(closure cl: conditions)
	    if(funcall(cl, ob))
	    {
		okay = 1;
		break;
	    }
	if(okay)
	    continue;
	
	if(flags&CHECK_ERROR)
	    raise_error(sprintf(
		"Aufruf durch ein nicht vertrauenswürdiges Objekt:\n%O\n",ob));
	return 0;
    }
    
    return 1;
}

// Einige Bedingungen, Beschreibung siehe add_secure_condition
int sc_euid_as_ti(<object|lwobject> ob)
{
    return this_interactive() && geteuid(ob)==geteuid(this_interactive());
}

int sc_euid_as_tp(<object|lwobject> ob)
{
    return this_player() && geteuid(ob)==geteuid(this_player());
}

int sc_wiz_euid_as_tp(<object|lwobject> ob)
{
    if(!this_player())
        return 0;
    else
    {
        string tp_euid = geteuid(this_player());
        
        if(tp_euid == PLAYER_UID || tp_euid == LEARNER_UID)
            return ob == this_player();
        else
            return geteuid(ob) == tp_euid;
    }
}

int sc_ti(<object|lwobject> ob)
{
    return ob==this_interactive();
}

int sc_tp(<object|lwobject> ob)
{
    return ob==this_player();
}

/*
FUNKTION: init_security_for_actions
DEKLARATION: protected void init_security_for_actions()
BESCHREIBUNG:
Damit wird this_player() und - sofern this_player() ein Gott ist - auch
allen Objekten mit gleicher EUID vertraut. (Falls TI!=0 ist, wird damit
auch TI==TP verlangt.)

Diese Einstellung kann statt 'static add_actions' genutzt werden.
Achtung: Es ist dazu aber noch sicherzustellen, dass this_player()
wirklich jemand ist, der dieser Action nutzen darf.

BEISPIEL:

inherit "/i/tools/security";

int my_action(string str)
{
    if(!this_player_darf_das() || !check_security())
	return 0;

    ...
}

void init()
{
    // Hier waere frueher das if(this_player_darf_das()) gewesen.
    add_action("my_action","trala");
    
    ...
}

void create()
{
    init_security_for_actions();

    ...
}

VERWEISE: add_security_condition, check_security, init_security_trust_mudlib
GRUPPEN: Sicherheit
*/
protected void init_security_for_actions()
{
    add_security_condition(#'sc_wiz_euid_as_tp);
}

/*
FUNKTION: init_security_trust_mudlib
DEKLARATION: protected void init_security_trust_mudlib()
BESCHREIBUNG:
Fuegt als sicher angesehene Mudlib-Objekte hinzu. (Derzeit Objekte aus /apps,
/map/map, Spieler-Objekte, Objekte aus /room/rathaus und Admin-Tools.)
Dies kann man als Ausgangspunkt fuer eigene Anpassungen (z.B. fuer Master)
nehmen.
VERWEISE: add_security_condition, check_security, init_security_for_actions
GRUPPEN: Sicherheit
*/
protected void init_security_trust_mudlib()
{
    add_security_condition(APPS_UID);	// /apps ohne closure_container
    add_security_condition(MAP_UID);	// /map
    add_security_condition(#'playerp);
    add_security_condition("/room/rathaus/");
    foreach(string adm: ADMINS)
	add_security_condition(adm);
}
