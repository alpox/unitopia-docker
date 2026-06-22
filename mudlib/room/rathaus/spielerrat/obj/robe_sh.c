// Dieses File ist Teil der UNItopia MUDlib
// ------------------------------------------------------------------
// File:        /room/rathaus/spielerrat/obj/robe_sh.c
// Author:      Myonara (aus dem gericht_sh als Vorlage)


inherit "/i/shadow";

#include <shadow.h>
#include <invis.h>

#define ICH      query_shadow_owner()
#define ENV      environment(ICH)

/* ----------------------------------------------------------------- */

int do_shadow(object ob)
{
  return ( init_shadow ( ob ? ob : this_player(), NO_MULTI_SHADOW ) ); 
}

int no_sr_shadow()
{
  destruct_shadow(); 
  return 1; 
 }

/* ----------------------------------------------------------------- */

string query_gender()
{
  return ( ICH && ICH->query_real_gender() );
}

/* ----------------------------------------------------------------- */

string query_personal_title()
{
    string g = query_gender();
    switch (g[0..0])
    {
        case "m": return "Spielerrat";
        case "w": return "Spielerrätin";
        default: return "Spielerratendes";
    }
}


/* ----------------------------------------------------------------- */

string query_name()
{
    return ICH && ICH->query_real_name();
}

/* ----------------------------------------------------------------- */

string query_cap_name()
{
    return ICH && ICH->query_real_cap_name();
}

/* ----------------------------------------------------------------- */

mixed *query_adjektiv()
{
  return ( ({}) );
}

/* ----------------------------------------------------------------- */

int query_invis()
{
  return ( V_VIS );
}

/* ----------------------------------------------------------------- */

int query_personal()
{
  return ( 1 );
}

/* ----------------------------------------------------------------- */

int id(string id)
{
  if ( !ICH)
    return 0;

  if (id == "spielerrat" || id == "spielerrätin" 
        || id == "spieleratendes") return 1;
  
  return ( ICH->id(id) );
}

// string query_long(object betrachter)
// {
    // return this_object()->query_short(betrachter);
// }

/* ----------------------------------------------------------------- */


