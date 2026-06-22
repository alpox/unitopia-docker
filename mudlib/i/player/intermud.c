// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /i/player/intermud.c
// Author:      Gnomi
// Description: Support fuer InterMUD-Portale

// Diese Datei hat zwei Teile.
// Die erste Haelfte (Server) beschaeftigt sich mit diesem
// Objekt als fremdes Wesen aus einer anderen Welt.
// Die zweite Haelfte (Client) hilft diesem Objekt, in eine
// andere Welt zu gehen.

#include <message.h>
#include <portal.h>
#include <functionlist.h>

#define SECURE_IM if(object_name(previous_object()) != PORTAL_SERVER) return;
  /*************************/
  /*  Server - Funktionen  */
  /*************************/

#pragma share_variables
nosave mapping save_vars = ([
        "material", "smell", "noise", "feel", "adjektiv",
        "personal", "personal_title", "alignment", "title",
        "msg_out", "mmsg_out", "msg_in", "mmsg_in", "msg_invis", "msg_vis",
        "exit_mode", "reattack", "likes_attacks", "whimpy",
        "hp_view", "hp", "max_hp", "sum_hp", "max_sp", "sp", "sum_sp",
        "short_combat_msg", "probability_for_guardian_angel",
        "auto_load_files", "auto_load_parameter", "hlp_data",
        "stats", "level", "mail_address", "value_of_inventory", "www_page",
        "gilde", "rang", "gilden_data", "letzte_gilden2", "level_dates",
        "eye_option", "editor_options", "ed_setup", "chunk",
        "tue_trenner", "aliases", "prompt",
        "wiz_soul_flags", "new_wiz_errors", "zauberstab_info",
        "headache", "wp", "fp", "besoffen", "kein_verbrauch",
        "skill_version", "skill_structure", "skill","other_skills",
        "erforscht", "reise", "gesehen", "handeln",
        "kill_list_races", "kill_list_name_level",
        "sum_comm", "sum_feel", "sum_move", "sum_weight",
        "events", "onoff", "edpuffer", "echomode", "pufferoptionen",
        "global_eventmode", "global_colourmode", "ignored_players",
        "keine_gummigoettchen", "colours", "trigger", "wizard_appreciations",
        "option_flags",
        "player_age", "ghost", "kirche", "konto", "konto_age",
        "scar", "no_wer", "no_tips", "no_ascii_art", "player_flags",
        "other_finger_flags", "opfer", "erf_gestorben",
        "artikel", "newsreader_settings",
        "descr_source", "clone_msg", "home_msg", "destruct_msg",
        "descr", "no_airport", "version", "no_retain_messages",
        "saved_critters", "opponents",
    ]);

nosave string src_mud;
nosave string src_name;
nosave int aborted; /* 0: Live, 1: We try to quit, 2: We are silent */
nosave mixed cmd_prompt;
nosave int sent_prompt;
nosave int last_active;
nosave int unicode_capable;


protected int execute_input_to(string cmd);
protected mixed query_pending_input_to_prompt();
void print_prompt(mixed prompt);


private varargs void restore_data(string var, mapping data, string key)
{
    if(member(data, key || var))
    {
        string saved = save_value(data[key || var]);
        int pos = strstr(saved,"\n");

        saved[pos..pos] = "\n" + var + " ";

        restore_object(saved);
    }
}

nomask void setup_intermud_player(string mud, string name, mixed chardata, mixed savedata, int unicode)
{
    SECURE_IM;

    unicode_capable = unicode;
    src_mud = mud;
    src_name = name;

    if(!mappingp(savedata))
    {
        savedata = ([
            "whimpy":   30,
            "msg_in":   "$Ein() nähert sich $dir().",
            "msg_out":  "$Der() entfernt sich $dir().",
            "mmsg_in":  "$Ein() erscheint in einer Rauchwolke.",
            "mmsg_out": "$Der() verschwindet in einer Rauchwolke.",
            "fp":       100,
            "wp":       100,
        ]);
        this_object()->add_skill_points(({"skill","offensiv","haende"}),0);
        this_object()->add_skill_points(({"skill","defensiv","schild","klein"}),0);
        this_object()->add_skill_points(({"skill","spiel"}),0);
        this_object()->add_skill_points(({"skill","raetsel"}),0);
    }

    this_object()->set_weight(30);

#ifdef Orbit
    if(src_mud == "UNItopia")
        savedata["level"] = chardata[P_CHAR_LEVEL];
    else
#endif
    if(!savedata["level"] || savedata["level"] > 10)
        savedata["level"] = 1;

    foreach(string var: save_vars)
        restore_data(var, savedata);

    this_object()->update_sum_skill();
    this_object()->compute_quest_count();
    this_object()->compute_game_count();

    this_object()->setup_player();
    if(lower_case(chardata[P_CHAR_NAME]) == lower_case(name))
        this_object()->set_real_cap_name(chardata[P_CHAR_NAME] + "@" + src_mud);
    else
        this_object()->set_real_cap_name(name + "@" + src_mud);
    this_object()->set_name(lower_case(chardata[P_CHAR_NAME]));
    this_object()->set_cap_name(chardata[P_CHAR_NAME]);
    switch(chardata[P_CHAR_GENDER])
    {
        case "maennlich":
        case "m":
        case "male":
            this_object()->set_gender("maennlich");
            break;

        case "weiblich":
        case "w":
        case "female":
            this_object()->set_gender("weiblich");
            break;

        default:
            this_object()->set_gender("saechlich");
            break;
    }
    this_object()->set_short(chardata[P_CHAR_NAME] + " aus " + src_mud);
    this_object()->add_id(src_name);

    last_active = time();
}

nomask void abort_intermud(int silent)
{
    SECURE_IM;

    aborted = silent ? 2 : 1;
    this_object()->quit();
}

nomask int is_intermud_guest()
{
    return src_mud != 0;
}

nomask string get_intermud_real_name()
{
    if(src_mud)
        return lower_case(src_name + "@" + src_mud);
}

nomask string get_intermud_src_name()
{
    return src_name;
}

nomask string get_intermud_src_mud()
{
    return src_mud;
}

static nomask void send_intermud_msg(string orig, string text, int mt, int ma)
{
    if (aborted == 2)
        return;

    if (!unicode_capable)
        text = to_text(to_bytes(text, "ASCII//TRANSLIT"), "ASCII");

    PORTAL_SERVER->send_message(([
        P_MSG_DATA: text,
        P_MSG_TEXTORIG: orig,
        P_MSG_TEXTTYPE: mt,
        P_MSG_TEXTACTION: ma
    ]));
}

static nomask void send_intermud_prompt(mixed prompt)
{
    string text = funcall(prompt) || "";

    if (aborted == 2)
        return;

    if (!unicode_capable)
        text = to_text(to_bytes(text, "ASCII//TRANSLIT"), "ASCII");

    sent_prompt = 1;
    PORTAL_SERVER->send_message(([
        P_MSG_DATA: text,
        P_MSG_TEXTTYPE: "Prompt",
    ]));
}

static nomask void send_intermud_data()
{
    mixed* variables = efun::variable_list(this_object(), RETURN_FUNCTION_NAME|RETURN_VARIABLE_VALUE);
    mapping result = ([]);

    if (aborted == 2)
        return;

    for(int i=0; i<sizeof(variables); i+=2)
    {
        if(member(save_vars, variables[i]))
            m_add(result, variables[i], variables[i+1]);
    }

    PORTAL_SERVER->send_savedata(result);
}

nomask void do_intermud_cmd(string cmd)
{
    SECURE_IM;

    sent_prompt = 0;
    last_active = time();

    if(sizeof(cmd) && cmd[0]=='!')
    {
        cmd = cmd[1..<1];
        efun::command(cmd);
    }
    else if(!call_with_this_player(#'execute_input_to, cmd))
        efun::command(cmd);

    if(!sent_prompt && this_object())
        send_intermud_prompt(query_pending_input_to_prompt() || cmd_prompt || this_object()->query_prompt_string());
}

protected void quit_intermud_guest()
{
    if(!aborted)
        PORTAL_SERVER->send_quit();
}

int query_intermud_idle()
{
    return time() - last_active;
}

mixed set_intermud_prompt(mixed prompt)
{
    if(!prompt)
        return cmd_prompt;
    else
    {
        mixed last = cmd_prompt;
        cmd_prompt = prompt;
        return last;
    }
}


  /*************************/
  /*  Client - Funktionen  */
  /*************************/

mapping intermud_data = ([]);

void call_save();

nomask void receive_intermud_msg(string mud, mapping data)
{
    SECURE_IM;

    if(data[P_MSG_TEXTTYPE] == "Prompt")
        print_prompt(data[P_MSG_DATA]);
    else if(data[P_MSG_TEXTORIG])
        this_object()->receive_message(data[P_MSG_TEXTTYPE], data[P_MSG_TEXTACTION], this_object(), data[P_MSG_TEXTORIG]);
    else if(data[P_MSG_DATA])
        this_object()->receive_message(MT_UNKNOWN | MT_NO_WRAP, MA_UNKNOWN, this_object(), data[P_MSG_DATA]);
}

nomask void save_intermud_data(string mud, mixed data)
{
    SECURE_IM;

    intermud_data[lower_case(mud)] = data;
    call_save();
}

nomask mixed get_intermud_data(string mud)
{
    SECURE_IM;

    return intermud_data[lower_case(mud)];
}
