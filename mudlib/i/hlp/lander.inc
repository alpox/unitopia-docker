// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /i/hlp/lander.inc
// Description: 

#pragma save_types
#pragma strong_types

#include <config.h>
#include <level.h>
#include <move.h>

private string map2domain_expansion(string name)
{
    int x, y;

    if(sscanf(name,"/map/m%d_%d",x,y) == 2 &&
       stringp(name = MAP_OB->query_map_file_name(x,y)) &&
        name[<2..] == ".c")
       name = name[0..<3];
    return name;
}

private void hlp_moved_in(string notify_moved_in, mapping mv_infos)
{
   object kugel, who = mv_infos[MOVE_OBJECT];
   string ort;

   if(who &&
      (hlpp(who) || wizp(who)) &&
      (kugel = present("hlp#tool", who)) &&
      kugel->gabe("pf") &&
      !(kugel->query_ort(ort = map2domain_expansion(object_name()))))
      kugel->add_ort(ort);
}

#pragma no_warn_unused_variables
private int __lander_inc_init__ = this_object()->add_controller(
    "notify_moved_in", #'hlp_moved_in);
