// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/apps/plugin.c
// Description:	Ein Plugin-Manager
// Author:	Freaky (12.01.2000)

// UID: Apps

#include <apps.h>
#include <error.h>
#include <rtlimits.h>

#define SAVE_FILE "/var/plugins"
#define PL_REG_DATE 0
#define PL_LAST_DATE 1
#define PL_SIZE 2

#define PL_LOAD_EVALS 500000

mapping plugins;

private void do_load_plugin(string pl)
{
    string msg;
    int ev;

    ev = get_eval_cost();
    if (msg = catch(load_object(pl)))
    {
	do_error2("Error loading plugin " + pl + ": " + msg,
		__FILE__, pl, __LINE__);
    }
    debug_message(sprintf("\n  Plugin: %-50s %8d",pl,ev-get_eval_cost()));
}

private void load_plugins(string pl)
{
    // Umweg ueber LIMIT_UNLIMITED gehen, da
    // PL_LOAD_EVALS kleiner als normale Evals ist.

    limited( (:
        limited(#'do_load_plugin,({ PL_LOAD_EVALS }), $1)
    :), ({ LIMIT_UNLIMITED }), pl);

}

void create()
{
    restore_object(SAVE_FILE);
    if (plugins)
	map(plugins,#'load_plugins);
    else
	plugins = m_allocate(0,PL_SIZE);
    debug_message("\n    ");
}

void save_plugins()
{
    save_object(SAVE_FILE);
}

void prepare_renewal()          {}
void abort_renewal()            {}
void finish_renewal(object neu) {}

string invalid_plugin(object plugin)
{
    if (member(inherit_list(plugin),"/i/tools/plugin.c") < 0)
	return "Ein Plugin muss '/i/tools/plugin' inheriten.\n";
    if (clonep(plugin))
	return "Ein Plugin darf kein Clone sein.\n";
    return 0;
}

int plugin_registered(string plugin)
{
    return member(plugins,plugin);
}

int plugin_register_date(string plugin)
{
    if (plugin_registered(plugin))
	return plugins[plugin,PL_REG_DATE];
    return -1;
}

int plugin_last_date(string plugin)
{
    if (plugin_registered(plugin))
	return plugins[plugin,PL_LAST_DATE];
    return -1;
}

int register_plugin()
{
    string plugin;

    if (invalid_plugin(previous_object()))
	return -1;
    
    plugin = object_name(previous_object());

    if (plugin_registered(plugin))
    {
	plugins[plugin,PL_LAST_DATE] = time();
	save_plugins();
	return 1;
    }
    
    plugins[plugin,PL_REG_DATE]  = time();
    plugins[plugin,PL_LAST_DATE] = time();
    save_plugins();
    return 2;
}

private void check_plugin(string plugin, int reg_date, int last_date)
{
    object ob;
    string err, tmp;

    ob = find_object(plugin);
    err = "";
    if (!ob)
    {
	if (file_size(plugin + ".c") <= 0)
	    err = "Plugin (File) ist nicht vorhanden.\n";
	else
	{
	    err = catch(ob = touch(plugin));
	    if (!ob && !err)
		err = "Plugin konnte nicht geladen werden.\n";
	}
    }
    if (ob)
    {
	tmp = invalid_plugin(ob);
	if (tmp)
	    err += tmp;
    }
    if (strlen(err))
    {
	m_delete(plugins,plugin);
	do_error2("Plugin wurde ausgetragen.\nGrund: "+err,
	    __FILE__, plugin, __LINE__);
    }
}

static int last_check;

int clean_plugins()
{
    walk_mapping(plugins,#'check_plugin);
    last_check = time();
    save_plugins();
    return 1;
}

mapping query_plugins()
{
    if(last_check<time()) // Hoechstens einmal pro Backend-Zyklus suchen
	clean_plugins();
    return deep_copy(plugins);
}

string query_plugin_replacement(string oldfile)
{
    return FILED->query_plugin_replacement(oldfile);
}
