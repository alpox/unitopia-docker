// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/secure/master.c
// Description: master object

#pragma strict_types
#pragma no_inherit
#if __VERSION__ > "3.3"
#pragma no_warn_missing_return
#endif
#if __VERSION__ > "3.6.1-U048"
#pragma warn_unused_variables
#endif
#if __VERSION__ > "3.6.4-U056"
#pragma warn_dead_code
#endif
#if __VERSION__ > "3.6.4-U059"
#pragma warn_applied_functions
#endif
#ifdef __LPC_COROUTINES__
#pragma warn_unused_values
#endif

#include "/sys/config.h"
#include "/sys/move.h"
#include "/sys/udp.h"
#include "/sys/quest.h"
#include "/sys/game.h"
#include "/sys/level.h"
#include "/sys/driver_hook.h"
#include "/sys/apps.h"
#include "/sys/news.h"
#include "/sys/mail.h"
#include "/sys/uids.h"
#include "/sys/acl.h"
#include "/sys/files.h"
#include "/sys/gilden.h"
#include "/sys/debug_message.h"
#include "/sys/error_db.h"
#include "/sys/properties.h"
#include "/sys/portal.h"
#include "/sys/rtlimits.h"
#include "/sys/signals.h"
#include "/sys/time.h"
#include "/sys/inherit_list.h"
#include "/sys/functionlist.h"
#include "/sys/configuration.h"
#if __EFUN_DEFINED__(driver_info)
#include "/sys/object_info.h"
#include "/sys/driver_info.h"
#else
#include "/sys/debug_info.h"
#endif
#if __EFUN_DEFINED__(interactive_info)
#include "/sys/interactive_info.h"
#endif
#include "/secure/simul_efun.h"

// Fuer das Laden der simul-efun.
static private int loading_simul_efun;

// Freaky: Solange mir nix besseres einfaellt:
private object find_player(string str)
{
    return funcall(symbol_function('find_player),str);
}
private void tell_object(object ob, string str)
{
    mixed dummy;

    if(({int})ob->is_intermud_guest())
        ob->receive_message(0, 0, previous_object(), str);
    else if(!call_resolved(&dummy, ob, "receive_message_low", str))
	efun::tell_object(ob,str);
}

#define MASTER_SAVE_FILE "/var/master"

void save_master()
{
    save_object(MASTER_SAVE_FILE);
}

// Prototypen:
int valid_file_name(string name);
int no_load_file(string file);
void do_log_error(string err, string prg, string curobj, int line,
        string *debugger, int no_msg);

// Module:
#include "/secure/master/check_level.inc"
// Freaky: Braucht man bei LDmud-3.2.8 nicht mehr
// #include "/secure/master/query_player_level.inc"
#include "/secure/master/get_uids.inc"
#include "/secure/master/domain_compat.inc"
// #include "/secure/master/access.inc"
#include "/secure/master/create_wiz.inc"
#include "/secure/master/creator_file.inc"
#include "/secure/master/ed.inc"
#include "/secure/master/epilog.inc"
// Freaky: Braucht man bei uns nicht, da wir den Driver nie mit -f starten
// #include "/secure/master/flag.inc"
#include "/secure/master/get_wiz_name.inc"
#include "/secure/master/log_error.inc"
// Freaky: Braucht man bei uns nicht, da wir kein parse_command() haben
// #include "/secure/master/parse.inc"
#include "/secure/master/preload.inc"
#include "/secure/master/shadow.inc"
#include "/secure/master/valid_trace.inc"
#include "/secure/master/valid_exec.inc"
#include "/secure/master/valid_seteuid.inc"
#include "/secure/master/valid_snoop.inc"
#include "/secure/master/acl.inc"
#include "/secure/master/git.inc"
#include "/secure/master/valid_write.inc"
#include "/secure/master/valid_read.inc"
#include "/secure/master/connections.inc"
#include "/secure/master/handle_light.inc"
#include "/secure/master/skill_handling.inc"
#include "/secure/master/auto_include.inc"
#include "/secure/master/driver_hooks.inc"
#include "/secure/master/compiler_control.inc"
#include "/secure/master/privilege_violations.inc"
#include "/secure/master/master_control.inc"
#include "/secure/master/destruct.inc"
#include "/secure/master/log_command.inc"
#include "/secure/master/include_file.inc"
#include "/secure/master/inherit_file.inc"

void heart_beat()
{
    /* Update the debug file offset to ignore non-error messages there. */
    get_debugfile_offset();
}
