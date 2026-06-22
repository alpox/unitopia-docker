// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/apps/description_container.c
// Description: Der Closure-Container fuer Descriptions
// Author:	Gnomi (15.03.2013)

inherit "/i/tools/description";

#include <description.h>
#include <v_item.h>

nomask void create()
{
}

public string|closure compile_description(mixed desc, mapping tags)
{
    return compile_desc(desc, &tags);
}

protected int|closure compile_condition(mixed cond, mapping tags)
{
    return compile_cond(cond, &tags);
}

nomask closure do_bind_with_args_ignore_n(closure c, int num, varargs mixed* args)
{
    closure cl = bind_lambda(c);
    return function mixed (varargs mixed* vargs)
    {
        return funcall(cl, args..., vargs[num..]...);
    };
}

/* Fuer die Descriptions muessen wir die desc_* - Aufrufe fuer eventuelle
 * Ueberlagerungen auch an das jeweilige Objekt weiterleiten.
 */
private object desc_object(mapping info)
{
    object|mapping item = info[TI_ITEM]||info[TI_OBJECT]||info[TI_ROOM];

    if (objectp(item))
        return item;
    if (mappingp(item))
        return item[VI_MASTER]||item[VI_OBJECT];
    return 0;
}

protected mixed desc_condition(string name, mixed info, mixed* par)
{
    mixed res = ::desc_condition(name, info, par);
    if (res)
        return res;
    else
    {
        object ob = desc_object(info);

        return ob && call_other(ob, "desc_condition_"+name, info, par...);
    }
}

protected mixed desc_filter(string name, mixed info, mixed orig, mixed* par)
{
    mixed res = ::desc_filter(name, info, orig, par);
    if (res)
        return res;
    else
    {
        object ob = desc_object(info);

        return ob && call_other(ob, "desc_filter_"+name, info, orig, par...);
    }
}

protected mixed desc_text(string name, mapping info, mixed* par)
{
    mixed res = ::desc_text(name, info, par);
    if (res)
        return res;
    else
    {
        object ob = desc_object(info);

        return ob ? call_other(ob, "desc_text_"+name, info, par...) : "";
    }
}

protected int desc_number(string name, mixed info, mixed* par)
{
    int res = ::desc_number(name, info, par);
    if (res)
        return res;
    else
    {
        object ob = desc_object(info);

        return ob && call_other(ob, "desc_number_"+name, info, par...);
    }
}
