// This file is part of UNItopia Mudlib.
// ---------------------------------------------------------------------
// File:	/apps/top.c
// Description:	Liefert Informationen zur Aktivität von UIDs
// Author:	Gnomi

#pragma strong_types

private inherit "/i/tools/top";

#include <wizlist.h>

int last_time;
struct top_uid_info* last_info;

struct top_uid_info* get_top_info()
{
    if (time() != last_time)
    {
        mapping callouts = ([:1]);

        foreach(mixed *callout: efun::call_out_info())
            callouts[getuid(callout[0])]++;

        last_info = map(efun::wizlist_info(), function struct top_uid_info(mixed* info)
        {
            return (<top_uid_info>
                uid:      info[WL_NAME],
                evals:    info[WL_TOTAL_GIGACOST] * 1000000000 + info[WL_TOTAL_COST],
                data:     info[WL_ARRAY_TOTAL] + info[WL_MAPPING_TOTAL] + info[WL_STRUCT_TOTAL],
                callouts: callouts[info[WL_NAME]],
            );
        });
        last_time = time();
    }

    return last_info;
}
