// This file is part of Avalon Mudlib
// ----------------------------------------------------------------
// File:        /i/wizard_shell/vcommands.inc
// Author:      Avatar (10/96)
// Description: Hier befinden sich die staendig verfuegbaren Kommandos

// Folgende Variablen sind Dummys, damit man die Zepter-Files includen kann :/
private static string ed_func;
private static mapping params, vars = ([]);
private static closure c_clone_msg, c_destruct_msg;

void use_dummy_variables() { funcall(0, c_clone_msg, c_destruct_msg); }

#include "/i/zauberstab/zdefs.h"
#include "/i/zauberstab/zlib.inc"
// #include "/i/zauberstab/zbrowse.inc"
#include "/i/zauberstab/files.inc"
#include "/i/zauberstab/zfehler.inc"
#include "/i/zauberstab/find.inc"

#include <acl.h>
#include "/i/zauberstab/zacl.inc"

#include <driver_info.h>
#include <term.h>
#include "/i/zauberstab/zsysinfo.inc"
#pragma strong_types

#include <apps.h>

#undef MY_NAME
#define MY_NAME "/obj/wizard_shell"

int who_command(string str)
{
   int i;
   mixed *usrs;
   object ob;
 
   BEGIN_PIPE(str);

   usrs=filter(efun::users(),lambda(({'ob}),
       ({ #'==, ({
         #'strstr, ({ #'object_name, 'ob }), WIZARD_SHELL_OB }), 0 }) ));

   if( !i=sizeof(usrs) )
   {
      notify_fail("Momentan ist keine Wizard-Shell geöffnet.\n");
      return 0;
   }
 
   WLN(sprintf("%-10s   %6s  %8s  %8s  %:-28s",
      "Benutzer", "OID", "Zeit", "Idle", "eingeloggt von" ));
 
   for(i=sizeof(usrs);i--;)
   {
      ob=usrs[i];
 
      WLN(sprintf("%-10s  %6s  %-8s  %8d  %:-28s",
         capitalize(ob->query_master_name()),
         explode(object_name(ob),"#")[<1],
         explode(shorttimestr(ob->query_login_time())," ")[<1],
         query_idle(ob),
	 TOWN_MASTER->town(ob)));
   }
   WLN("Zur Zeit "+((i=sizeof(usrs)) == 1 ? "ist " : "sind ")+i+
      " Benutzer eingeloggt.");
 
   PRINT;
 
   return 1;
}
 
int kill_command(string str)
{
   string oid, oname;
   object wizshell;

   if( !to_int(str||"") )
   {
      notify_fail("Syntax: kill <OID>\n");
      return 0;
   }
   oid=to_string(to_int(str));
   oname=WIZARD_SHELL_OB+"#"+oid;
   if( !wizshell=find_object(oname) )
   {
      notify_fail("Die OID '"+oid+"' gibt es nicht.\n"+
         "Die existierenden OIDs erhältst Du mit 'who'.\n");
      return 0;
   }
   if(wizshell->query_master_name() 
      && (wizshell->query_master_name() != master_name))
      if(!adminp(this_player()))
      {
         notify_fail("Du darfst nur Deine eigenen Shells 'kill'en.\n");
         return 0;
      }
      else
         write("Ok, weil Du ein Admin bist, darfst Du auch fremde Shells "+
            "'kill'en.\n");

   wizshell->remove();
   if( wizshell )
      destruct(wizshell);

   if( this_object())
      write("'"+oid+"' zerstört.\n");
   return 1;
}

int prompt_command(string str)
{
   if( !str || str == "" )
   {
      write("Aktueller Prompt:\n"+
         "'"+this_player()->query_prompt_string()+"'\n");
      return 1;
   }
   this_player()->set_prompt_string(str);
   write("Neuer Prompt:\n"+
      "'"+this_player()->query_prompt_string()+"'\n");
   return 1;
}
