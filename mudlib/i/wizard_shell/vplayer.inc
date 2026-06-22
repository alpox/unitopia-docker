// This file is part of Avalon Mudlib
// ----------------------------------------------------------------
// File:        /i/wizard_shell/vplayer.inc
// Author:      Avatar (10/96)
// Description: Hier befinden sich die wichtigsten Funktionen, die noetig
//              sind, um den Wiz-Tools einen Player vorzutaeuschen

#include <telnet.h>

private object update_master()
{
   if( !master_name )
      return 0;
   return master || (master=find_player(master_name));
}

nomask string query_real_name()       { return master_name; }
nomask string query_real_cap_name()   { return capitalize(master_name); }

nomask int query_level()
{
   if( update_master() )
      return master->query_level();
}
nomask int query_wiz_level()
{
   int level;
   
   return (level=query_level()) >= LVL_LEARNER ? level : 0;
}

// Fuer /i/player/tippse.c
int query_f_casting()      { return 0; }

// Fuer den Standard-Editor
int query_ed_setup()
{
   if( update_master() ) 
      return master->query_ed_setup(); 
}
void set_ed_setup(int i)
{
   if( update_master() )
      master->set_ed_setup(i);
}

int query_player_flag(int bit)
{
   if( update_master() )
      return master->query_player_flag(bit);
}

void net_dead()  
{
   // Editor-Inhalt sichern
   xeditor::editor_save_current_file();
   destruct(this_object());
}

mixed query_read_artikel()
{
   if( update_master() )
      return master->query_read_artikel();
}

void set_read_artikel(mixed struc)
{
   if( update_master() )
      master->set_read_artikel(struc);
}

static string query_prompt_msg()
{
   return files::query_current_path();
}

void set_new_wiz_errors(mixed *errs)
{
    if( update_master() )
        master->set_new_wiz_errors(errs);
}

mixed *query_new_wiz_errors()
{
    if( update_master() )
        return master->query_new_wiz_errors();
    else
	return ({ MAX_INT, ({ }) });
}

varargs mixed query_eye_option(string option)
{
    if (update_master())
	return master->query_eye_option(option);
    else if(option)
	return 0;
    else
	return ([]);
}

// Fuer /i/tools/xeditor.c
mapping query_editor_options()
{
    if (geteuid(previous_object()) == geteuid() && update_master())
    {
	mapping opts = master->query_editor_options();
	if(opts)
	    return opts-(["split_y"]);
    }
    return 0;
}
void set_editor_options(mapping opts)
{
    if (geteuid(previous_object()) == geteuid() && update_master())
	master->set_editor_options(opts);
}

// Fuer /i/player/wiz_soul.c
varargs void notify_message(string msg, int type)
{
    if(this_object())
        this_object()->receive_message(MT_NOTIFY,type,this_object(),msg);
}

// Wird vom Zauberstab aufgerufen.
void notify_ed_exit(int suspend_client)
{
    if(suspend_client)
	this_object()->restart_mudclient();
}

void notify_ed_enter(int suspend_client)
{
    if(suspend_client)
	this_object()->suspend_mudclient();
}

// Wird von more verwendet.
#define MIN_WIDTH 79
int query_client_width()
{
    mixed sb;
    int width;

    if(update_master())
        width = master->query_set_client_width();

    if(width)
        return width;

    if(this_object()->query_telnet(TELOPT_NAWS,&sb) && sizeof(sb) > 1 && sb[0] > MIN_WIDTH)
        return sb[0];

    return MIN_WIDTH;
}
