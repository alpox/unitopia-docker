// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /i/tools/move_msg_utils.c
// Description: Utility-Funktionen fuer move_msg
// Author:      Freaky (08.04.2001)

#pragma save_types

#define DIRS ({ "norden","nordwesten","westen","suedwesten", \
                "sueden","suedosten","osten","nordosten" })
#define DIRS_UL ({ "norden","nordwesten","westen","südwesten", \
                   "süden","südosten","osten","nordosten" })

string direction_string(string prep, string dir, int rein)
{
    int i;

    i = member(DIRS, dir);
    if (i < 0)
        i = member(DIRS_UL, dir);
    if (i >= 0)
        return prep + " " + capitalize(DIRS_UL[(i+4*rein)%8]);

    switch(dir)
    {
        case "runter":
        case "unten":
           return prep + (rein ? " oben" : " unten");
        case "hoch":
        case "rauf":
        case "oben":
           return prep + (rein ? " unten": " oben");
        case "links":
           return prep + " " + (rein ? "rechts" : "links");
        case "rechts":
           return prep + " " + (rein ? "links" : "rechts");
    }
    return 0;
}

/*
 * message_expansion wandelt die uebergegebene closure in
 * die Bewegunsgmeldung um
 */
string message_expansion(closure rule, mixed who, string dir_str, varargs <object|mapping>* args)
{
   string exp;

   if(!rule)
      return 0;
   if(playerp(this_object()) &&
      this_object()->query_alc()*3>this_object()->query_max_alc()*2)
   {
      exp = funcall(rule, who, "\1");
      exp = regreplace(exp, "nähert sich \1", "torkelt \1 herbei", 0);
      exp = regreplace(exp, "entfernt sich \1", "torkelt \1 davon", 0);
      if(dir_str)
        exp = regreplace(exp, "\1", dir_str, 1);
   }
   else
   {
      if(!(exp = funcall(rule,who,dir_str||"\1", args...)))
         return 0; 
   }
   exp=regreplace(exp,"( |)\1","",1);
   if(sizeof(exp) && exp[0] == ' ')
      exp = exp[1..];
   if(sizeof(exp) && exp[<1] == ' ')
      exp = exp[0..<2];
   return exp;
}
