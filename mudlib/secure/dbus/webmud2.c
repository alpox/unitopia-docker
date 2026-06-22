// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /secure/dbus/webmud2.c
// Description: RPC-Funktionen fuers Webmud2
// Author:      Gnomi

#include <eyes.h>
#include <invis.h>
#include <level.h>
#include <tls.h>
#include <finger.h>
#include <interactive_info.h>

mapping players = ([:1]); // Port -> Player

private int get_port(object player)
{
    string ip = efun::interactive(player) && efun::interactive_info(player, II_IP_NUMBER);

    if(member(({"127.0.0.1", "::1", "::ffff:127.0.0.1"}), ip) >= 0)
        return efun::interactive_info(player, II_IP_PORT);
}

private object find_player_port(int port)
{
    if(players[port])
    {
        if(get_port(players[port]) != port)
            m_delete(players, port);
        else
            return players[port];
    }

    // Dann suchen wir mal:
    foreach(object user: users())
        if(get_port(user) == port)
        {
            m_add(players, port, user);
            return user;
        }
}

private nosave mapping htmlchars = (["&": "\\&amp;", "<": "\\&lt;", ">": "\\&gt;", "\n": "<br/>"]);
private string htmlify(string text)
{
    return regreplace(text, "[&<>\n]", function string(string t)
        {
            return htmlchars[t];
        }, 1);
}

private string show_inv(object pl, object* obs, int inv, string category, closure cond)
{
    string ret = "";

    foreach(int i: sizeof(obs))
    {
        object ob = obs[i];
        string obcat, obshort;

        if(!ob)
            continue;

        obcat = ob->query_inventory_category();
        if(obcat)
        {
            if(obcat != category)
                continue;
        }
        else if(!funcall(cond, ob))
            continue;

        obshort = ob->query_short(pl);
        if(obshort)
        {
            int obinv = ob->query_invis() & V_ATOM_NOLIST;
            obshort = htmlify(obshort);

            if(!obinv)
                ret += "<li class=\"invitem\" id=\"inv."+hash(TLS_HASH_MD5,object_name(ob))+"\">"+obshort+"</li>";
            else if(inv)
                ret += "<li class=\"invitem invisitem\" id=\"inv."+hash(TLS_HASH_MD5,object_name(ob))+"\">("+obshort+")</li>";
        }

        obs[i] = 0;
    }

    if(sizeof(ret))
        return "<li><span class=\"invcategoryname\">"+category+"</span><ul class=\"invcategory\">"+ret+"</ul></li>";
    return "";
}

nomask static string webmud2_inventory(int port)
{
    object pl = find_player_port(port);
    int inv;
    object *obs;

    if(!pl)
        return "";

    obs = all_inventory(pl);
    obs->undo_split();
    obs -= ({0});

    inv = wizp(pl);

    return "<div id=\"inventory\"><ul class=\"inventory\">"
      + show_inv(pl, obs, inv, IC_LIVING,     #'living)
      + show_inv(pl, obs, inv, IC_WEAPON,     function int(object ob) { return ob->query_weapon(); })
      + show_inv(pl, obs, inv, IC_ARMOUR,     function int(object ob) { return ob->query_armour(); })
      + show_inv(pl, obs, inv, IC_CONTAINER,  function int(object ob) { return ob->query_container(); })
      + show_inv(pl, obs, inv, IC_CLOTHES,    function int(object ob) { return ob->query_cloth(); })
      + show_inv(pl, obs, inv, IC_FOOD,       function int(object ob) { return ob->material("wasser") || ob->material("nahrung"); })
      + show_inv(pl, obs, inv, IC_MONEY,      function int(object ob) { return ob->query_money(); })
      + show_inv(pl, obs, inv, IC_VALUEABLES, function int(object ob) { return ob->query_value(); })
      + show_inv(pl, obs, inv, IC_OTHER,      function int(object ob) { return 1; })
      + "</ul></div>";
}

nomask static string webmud2_inv_description(int port, string id)
{
    object pl = find_player_port(port);
    object *obs;

    if(!pl)
        return "";

    obs = all_inventory(pl);
    obs->undo_split();
    obs -= ({0});

    if(id[0..3] == "inv.")
        id = id[4..<1];

    foreach(object ob: obs)
        if(hash(TLS_HASH_MD5,object_name(ob)) == id)
            return "<div id=\"invdesc\" class=\"invdesc\">"+htmlify(pl->query_object_description(ob,CONTENTS_SHOW_CONTENTS|(wizp(pl) && CONTENTS_SHOW_INVIS)))+"</div>";
}

nomask static string webmud2_who(int port)
{
    object pl = find_player_port(port);
    object *obs;
    string ret;

    if(!pl)
        return "";

    obs = users();
    if(!wizp(pl))
        obs = filter(obs, function int(object ob)
            {
                return !IS_INVIS(ob) &&
                       !(ob->query_no_wer() && (wizp(ob) || testplayerp(ob)));
            });

    obs = sort_array(obs, function int(object a, object b)
        {
            return a->query_name() > b->query_name();
        });


    ret = "<div id=\"who\"><ul>";

    foreach(object ob: obs)
    {
        string obshort = ob->query_short(pl);
        if(!obshort)
            obshort = ob->query_cap_name();

        if(obshort)
        {
            obshort = htmlify(obshort);
            ret += "<li class=\"whoitem\" id=\"who."+hash(TLS_HASH_MD5,lower_case(ob->query_name()))+"\">"+obshort+"</li>";
        }
    }

    ret += "</ul></div>";

    return ret;
}

nomask static string webmud2_who_description(int port, string id)
{
    object pl = find_player_port(port);

    if(!pl)
        return "";

    if(id[0..3] == "who.")
        id = id[4..<1];

    foreach(object ob: users())
        if(hash(TLS_HASH_MD5,ob->query_real_name()) == id)
        {
            int flags = pl->query_other_finger_flags();
            if(!(flags & FINGER_FLAG_VALID))
                flags = FINGER_FLAG_OTHER_DEFAULT;
            return "<div id=\"whodesc\" class=\"whodesc\">"
                + htmlify(FINGER_OB->do_local_finger(ob->query_real_name(), 1, flags))
                + "</div>";
        }

    return "";
}

nomask static string webmud2_listdir(int port)
{
    object pl = find_player_port(port);

    if(!wizp(pl))
        return "";

    // Wegen Zugriffsrechten duerfen wir das nicht machen.
    return pl->webmud_dir();
}

nomask static void webmud2_chdir(int port, string dir)
{
    object pl = find_player_port(port);

    if(!wizp(pl))
        return;

    pl->webmud_chdir(dir);
}

nomask static string webmud2_edit_file(int port, string id, string fname)
{
    object pl = find_player_port(port);

    if(!wizp(pl))
        raise_error("Invalid id.\n");

    return pl->webmud_start_edit(id, fname) || raise_error("Invalid id.\n");
}

nomask static string webmud2_edit_loadfile(int port, string id)
{
    object pl = find_player_port(port);

    if(!pl)
        raise_error("Invalid id.\n");

    return pl->webmud_edit_loadfile(id) || raise_error("Invalid id.\n");
}

nomask static int webmud2_edit_savefile(int port, string id, string txt)
{
    object pl = find_player_port(port);

    if(!pl)
        return 0;

    return pl->webmud_edit_savefile(id, txt);
}

nomask static int webmud2_edit_abort(int port, string id)
{
    object pl = find_player_port(port);

    if(!pl)
        return 0;

    return pl->webmud_edit_savefile(id, 0);
}

void reset()
{
    players = filter(players, (: $2 :));
}

void get_addr(closure cb, int port)
{
    if(!playerp(previous_object()))
        return;

    dbus_call_method(function void(string errname, varargs mixed* args)
    {
        if (!errname)
            funcall(cb, args...);
    }, "de.webmud2", "/de/webmud2/info", "de.unitopia.WebMUD2Info", "get_addr", "i", port);
}
