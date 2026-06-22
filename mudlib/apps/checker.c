// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /apps/checker.c
// Description: Prüft Objekte im MUD (im Hintergrund)
// Author:      Gnomi

#include <message.h>

#define RESET_CHECK_SIZE 100 /* Ab dieser Anzahl rufen wir mal testweise reset() auf. */
#define WARN_SIZE        500 /* Ab dieser Anzahl geben wir eine Warnung aus. */

nosave object* last_objects = ({});

void warn_message(object ob, string msg, varargs mixed* args)
{
    object pl = find_player("gnomi");
    if (pl)
        pl.send_message_to(pl, MT_DEBUG, MA_UNKNOWN, sprintf("%Q: " + msg, ob, args...));
}

void check_object(object ob)
{
    /* Wir prüfen die Anzahl an Gegenständen im Inventar. */
    object * content = all_inventory(ob);
    if (sizeof(content) >= WARN_SIZE)
    {
        warn_message(ob, "Zuviele Gegenstände im Inventar: %d", sizeof(content));
    }
    else if (sizeof(content) >= RESET_CHECK_SIZE
          && !check_type(ob, [object "/i/money/lager"]))
    {
        object *new_content;

        ob->reset();
        new_content = all_inventory(ob);
        if (sizeof(new_content) > sizeof(content))
            warn_message(ob, "Inventar wächst durch reset(), enthält bereits: %d", sizeof(new_content));
    }
}

void heart_beat()
{
    object *last = last_objects - ({0});
    object next;

    if (!sizeof(last))
    {
        next = objects(0, 1)[0];
        last_objects += ({next});
    }
    else
    {
        object *next_objects = objects(last[<1], 1);
        if (!sizeof(next_objects))
            next_objects = objects(0,1);
        next = next_objects[0];
        last_objects = last_objects[1..] + ({next});
    }

    catch(check_object(next); publish);
}

void create()
{
    set_heart_beat(1);
}
