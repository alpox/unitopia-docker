// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/i/tools/passwd
// Description:	Funktionen zur Pruefung des Passworts
// Author:	Garthan	(20.04.95)

#pragma save_types

// nur static globals.

#include <passwd.h>

static int insecure_passwd(string passwd, string user)
{
   int res, i;
   string *pws;
   if(!stringp(passwd) || passwd == "")
      return PASSWD_NONE;
   if(strlen(passwd) < 5)
      res |= PASSWD_SHORT;
   if(strstr(passwd, " ") >= 0)
      res |= PASSWD_SPACE;
   passwd = lower_case(passwd);
   if(stringp(user) && strstr(passwd, user) >= 0)
      res |= PASSWD_NAME;
   for(i = sizeof(pws = UNSECURE_PASSWDS); i--;)
      if(strstr(passwd, pws[i]) >= 0)
      {
	 res |= PASSWD_EASY;
	 break;
      }
   return res;
}

static string passwd_msg(int res, int want)
{
   int shft, i;
   string msg;
   res &= want;
   
   for(msg = "", i = 0, shft = res; shft; i++, shft >>= 1)
      if(res & (1<<i))
	 switch(1<<i)
	 {
	    case PASSWD_SHORT:
	       msg += "Das Passwort muss mindestens 5 Zeichen enthalten.\n";
	       break;
	    case PASSWD_NAME:
	       msg += "Das Passwort darf nicht Deinen Namen enthalten.\n";
	       break;
            case PASSWD_SPACE:
	       msg += "Dein Passwort darf kein Leerzeichen enthalten.\n";
	       break;
	    case PASSWD_NONE:
	       msg += "Du musst ein Passwort setzen.\n";
	       break;
	    case PASSWD_EASY:
	       msg += "Dein Passwort lässt sich zu leicht erraten!\n";
	       break;
	 }
   return msg == "" ? 0 : msg;
}

#if __VERSION__ < "3.6.0"
static varargs string extended_crypt(string str, string seed)
#else
static varargs string extended_crypt(string|bytes str, string seed)
#endif
{
    if (!seed)
    {
        string set = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789./";
        seed = "$5$";
        foreach (int i: 16)
        {
            int n = random(sizeof(set));
            seed += set[n..n];
        }
    }

    return efun::crypt(str, seed);
}
