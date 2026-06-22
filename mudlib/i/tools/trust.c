// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/i/tools/trust.c
// Description: Trusten von Objekten
// Author:	Freaky (23.12.93)
// Modified by: Garthan (23.12.93)
//		Freaky (26.12.97) Security-Bugfix in trust_command
//		Freaky (19.11.1999) Security-Bugfix in trust_command

/* Truster */
/* Doku /doc/funktionsweisen/trust */

#pragma save_types

private functions inherit "/i/tools/security";

static private string trust_reason;
static private int trusted = init_security_for_actions() || 0;

void set_trust_reason(string str) { trust_reason = wrap(str); }
string query_trust_reason() { return trust_reason; }

string trusted()
{
   int ret;

   if (trusted)
       return geteuid();

   if(this_player() && (ret = this_player()->is_trusted_object()) == 1)
      return geteuid();

   if(ret)
      notify_fail("Dies ist "+ein()+" neueren Datums, wenn Du "+ihm()+
		  " weiterhin auf Dauer\nvertrauen willst, dann mittels "+
		  "'vertraue "+this_object()->query_name()+" immer'.\n");
   else
      notify_fail((trust_reason?trust_reason:"")+"Du musst "+dem()+
		  " mittels 'vertraue "+this_object()->query_name()+"' "+
		  "dein Vertrauen schenken.\n");
   return 0;
}

void init()
{
   add_action("trust_command", "vertraue",-7);
   add_action("trust_command", "trust");
}

int trust_command(string str)
{
   string rest;
   int mode;

   if(!check_security() || !this_player() ||
      this_player() != this_interactive() ||
      this_player() != environment())
      return 0;
   if(!(rest = this_object()->me(str)))
      return 0;
   if (!clonep())
   {
      notify_fail("Man kann nur Clones vertrauen...\n");
      return 0;
   }
   switch(rest)
   {
      case "ewig":
	 mode = 2; break;
      case "immer":
      case "+":
	 mode = 1; break;
      case "nicht":
      case "nicht mehr":
      case "-" :
	 mode = -1; break;
      default: 
	 mode = 0; break;
   }

   this_player()->trust(mode);

   return 1;
}

void setup_trust()
{
    if(playerp(previous_object()))
        seteuid(0);
}

void callback_trust(object ob, int mode)
{
#if __EFUN_DEFINED__(export_uid)
    if (playerp(previous_object()) && !geteuid() && ob==this_object())
    {
	seteuid(getuid());
#else
    if (playerp(previous_object()) && geteuid() == geteuid(previous_object()) && ob==this_object())
    {
#endif
        trusted=(mode>=0);
    }
}
