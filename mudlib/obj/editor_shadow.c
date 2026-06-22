// This file is part of Avalon Mudlib
// ----------------------------------------------------------------
// File:        /obj/editor_shadow.c
// Author:      Avatar
// Description: Ein Player-Shadow fuer den zeichenorientierten Editor

inherit "/i/shadow";

#include <shadow.h>
#include <more.h>
#include <apps.h>
#include <config.h>
#include <message.h>

private static int split_y;
private static object owner;
private static string msg_buffer = "";

varargs void destruct_editor_shadow(int flag);
 
void init_editor_shadow(object _owner, int _split_y)
{
   if( owner || !(_owner) )
      return;

   owner=_owner;
   split_y=_split_y;
   if( split_y )
      tell_object(owner,sprintf(
      "%c[2J"+                   // ClrScr
      "%c[H"+                    // Home
      "%c[s"+                   // SaveCursor
      "%c[1;"+(split_y)+"r"      // Split
      ,27,27,27,27));
   init_shadow(owner,REPLACE_OLD_SHADOW);
// ret printf("%c[2J%c[H%c[s%c[1;13r%c[13;1H",27,27,27,27,27);
}

object has_editor_shadow() { return this_object(); }

string query_away()
{
   return Er(owner)+" befindet sich gerade im Editor.";
}

// Fuer den Standard-Msg-Buffer
int query_receive_no_msg()
{
   return 0;
}

void receive_message(int msg_type, int msg_action, object who, string msg)
{
   if( !interactive(owner) )
   {
      destruct_editor_shadow();
      return;
   }

   if( this_interactive() != owner )
   {
      if( split_y )
      {
         owner->receive_message(msg_type,msg_action,who,
		 sprintf("%c[u%s%c[s",27,msg,27));
         return;
      }

      if( object_name(previous_object(1)||this_object()) != EVENT_MASTER )
         owner->receive_message(msg_type|MT_NO_WRAP,msg_action,who,"\a");
      msg_buffer += msg;
   }
   else
      owner->receive_message(msg_type,msg_action,who,msg);
}

varargs int flush_buffer(int no_void_msg)
{
   if( strlen(msg_buffer) )
   {
      if( present("avatars#xed",owner) )
         tell_object(owner,"\n-----\n"+msg_buffer+"-----\n");
      else
         this_object()->more(
            ({"~~~~~"})+
            (explode(msg_buffer,"\n")-({""}))+
            ({"~~~~~"}),
            "--Mehr--", 0, M_AUTO_END);

      msg_buffer="";
      return 1;
   }
   else
      if( !no_void_msg )
         tell_object(owner,(query_input_pending(owner) ? "\n" : "")+
            "Während der Editor-Session sind keine Meldungen eingegangen.\n");
}

varargs void destruct_editor_shadow(int flag)
{
   if( owner && !flag && interactive(owner) )
      flush_buffer();
   destruct_shadow();
}

void wieder_belebung(int was_non_interactive)
{
    query_shadow_owner()->wieder_belebung(was_non_interactive);
    destruct_editor_shadow(0);
}
