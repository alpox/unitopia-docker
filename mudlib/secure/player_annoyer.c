// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /secure/player_annoyer.c
// Description: Ein paar Gemeinheiten
// Author:      Gnomi

#define CONFIG_FILE "/static/adm/ANNOYANCES"

#include <level.h>
#include <message.h>
#include <stats.h>

private inherit "%tools/config";

private mapping config;

mixed query(string what, string wen, varargs mixed* data)
{
    if(member(config, what) == 0)
        return;

    switch(what)
    {
	case "Schreibverbot":	// ddata = ({})
        case "Fehlermeldungen":
        case "Konto":
	    return member(config[what],wen);
	case "Regenerationsbeschraenkung": // data = ({})
	    return member(config[what],wen)?config[what][wen]:__INT_MAX__;
	case "Gabensperre": // data=({string gabenkuerzel})
        case "MAPIL-Verbot": // data=({string mapilname})
	    return member(config[what],wen)?
		(member(config[what][wen], data[0])>=0):0;
	case "Stats":
	    return config[what][wen];
    }
}

private int mapil_controller(string contr, int magic_type, int flag,
    string name, object caster, mixed victim, int chance, mapping extra)
{
    if(playerp(caster) && query("MAPIL-Verbot",
	caster->query_real_name(), name))
    {
	caster->send_message_to(caster, MT_NOTIFY, MA_MAGIC,
	    "Die Götter haben dir diese Fähigkeit genommen.\n");
	return 1;
    }
}

void setup_player(object ob)
{
    string name;
    
    if(!playerp(ob))
	return;
    if(ob!=previous_object() && (
	!adminp(this_player()) ||
	geteuid(this_player()) != geteuid(previous_object())))
	    return;

    name = ob->query_real_name();
    
    if(config["MAPIL-Verbot"] && sizeof(config["MAPIL-Verbot"][name]))
	ob->add_controller(({"forbidden_magie","forbidden_handwerk"}),
	    #'mapil_controller);
}

void create()
{
    config = read_config(CONFIG_FILE,
	(["Regenerationsbeschraenkung:": (: to_int($3) :),
	  "MAPIL-Verbot:": (: map(explode($3,","), #'trim) :),
          "Gabensperre:": (: map(explode($3,","), #'trim) :),
	  "Stats:": 
	    (: 
		float *diff = ({0})*STAT_NUMBER;
		foreach(string str: explode($3,","))
		{
		    str=trim(str);
		    for(int i=0;i<STAT_NUMBER;i++)
			if(!strstr(str,STAT_NAMES[i]))
			    diff[i] = to_float(str[strlen(STAT_NAMES[i])..<1]);
		}
		return diff;
	    :),
	]));

    if(!mappingp(config))
    {
        config = ([:1]);
    }
    
    if(config["MAPIL-Verbot"])
	foreach(string name: config["MAPIL-Verbot"])
	    if(find_player(name))
		setup_player(find_player(name));
}
