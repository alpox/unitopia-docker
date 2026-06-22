// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/room/portal/obj/room.c
// Description:	Aufbewahrungsraum fuer einen Reisenden
// Author:	Gnomi
//

inherit "%room";

#include <config.h>
#include <commands.h>
#include <input_to.h>
#include <invis.h>
#include <move.h>
#include <portal.h>
#include <room_types.h>

object player;          /* Unseren Player-Ob.                     */
mapping portal;         /* Portal-Infos.                          */
string src_room;        /* Dateiname des Ursprungs-Raumes.        */
#ifdef UNItopia
string target_room;     /* Dateiname des Ziel-Raumes (zg-Befehl). */
#endif

void check_room();

/* Funktionen fuer den Portal-Master *
 * --------------------------------- */

/* Bei Betreten eines Portals, wird vom Portal-Master aufgerufen. */
#ifdef UNItopia
void set_traveller(object pl, mapping info, string target = 0)
#else
void set_traveller(object pl, mapping info)
#endif
{
    if (!playerp(pl))
        return;
    if (player)
        return;

    player = pl;
    portal = info;
    src_room = object_name(environment(pl));
#ifdef UNItopia
    target_room = target;
#endif
}

/* Der Reisende hat das MUD gewechselt. */
void move_traveller(object pl, mapping info)
{
    if (!player)
        set_traveller(pl, info);
    else if (player == pl)
        portal = info;
}

/* Der Reisende kehrt zurueck. */
#ifdef UNItopia
void leave(object pl, mapping info, string target = 0)
#else
void leave(object pl, mapping info)
#endif
{
    if (player != pl)
    {
#ifdef UNItopia
        if(present(pl) && target)
            pl.move(target, ([MOVE_FLAGS: MOVE_MAGIC]) );
#endif
        if(present(pl))
            pl.move(DEFAULT_NIRVANA_EXIT, ([MOVE_FLAGS: MOVE_MAGIC]) );
    }
    else if(info)
    {
        object room, portal;
        mixed msg;

#ifdef UNItopia
        if (target)
            room = touch(target);
#endif
        if (!room)
            room = touch(info[P_CONF_ROOM]);
        if(!room)
            room = touch(DEFAULT_START_ROOM);
        portal = present(ID_PORTAL(info[P_CONF_SRC_PORTAL_NAME]), room);
        if(portal)
        {
            msg = portal->query_enter_messages(pl);
            msg = pointerp(msg) && msg[1];
        }
        pl->move(room, ([MOVE_FLAGS: portal ? MOVE_NORMAL : MOVE_MAGIC, MOVE_MSG_ENTER: msg]));
    }
    else
    {
#ifdef UNItopia
        if(present(pl) && target)
            pl.move(target, ([MOVE_FLAGS: MOVE_MAGIC]) );
#endif
        if(present(pl))
            pl.move(src_room, ([MOVE_FLAGS: MOVE_NORMAL,
                MOVE_MSG_ENTER:"$Der() kommt aus dem Portal." ]));
    }

    player = 0;
    portal = 0;

    if(!pl->uses_vt100client())
        remove_input_to(pl);

    call_out(#'check_room, 2);
}

/* Die Verbindung zu einem MUD ist abgebrochen. */
object rescue(string mud)
{
    if (player && lower_case(portal[P_CONF_DEST_MUD]) == lower_case(mud))
    {
        object pl = player;
        leave(pl, 0);
        return pl;
    }

    return 0;
}

mapping get_traveller_info()
{
    return portal;
}

object has_traveller(string mud)
{
    if (player && lower_case(portal[P_CONF_DEST_MUD]) == lower_case(mud))
        return player;
    return 0;
}

/* Raum-Controller *
 * --------------- */

<int|string> let_not_in(mapping mv_infos)
{
    if(player != mv_infos[MOVE_OBJECT])
        return 1;
    return ::let_not_in(mv_infos);
}

void moved_in(mapping mv_infos)
{
    object wer = mv_infos[MOVE_OBJECT];
    if(player != wer)
    {
        wer->move(DEFAULT_NIRVANA_EXIT, ([MOVE_FLAGS:MOVE_MAGIC]));
        return;
    }

    // Damit man in den Raum reinsehen kann, ohne den Spieler zu sehen.
    wer->set_invis(V_NOLIST);
#ifdef UNItopia
    PORTAL_SERVER->enter_portal(wer, portal, target_room);
#else
    PORTAL_SERVER->enter_portal(wer, portal);
#endif

    if(wer)
        ::moved_in(mv_infos);
}

void moved_out(mapping mv_infos)
{
    mv_infos[MOVE_OBJECT]->set_invis(V_VIS);
    ::moved_out(mv_infos);
}

/* Eingabeschleife *
 * --------------- */

int cmd(string msg)
{
    if(player != this_player())
    {
        remove_input_to(this_player());
        this_player()->move(DEFAULT_NIRVANA_EXIT, ([MOVE_FLAGS:MOVE_MAGIC]));
        return 1;
    }

    if (!portal[P_CONF_UNICODE])
        msg = to_text(to_bytes(msg, "ASCII//TRANSLIT"), "ASCII");

    PORTAL_SERVER->send_command(this_player(), portal, msg);
    return 1;
}

void cmd_loop(string str)
{
    if(present(this_player()))
        input_to("cmd_loop", INPUT_IGNORE_BANG);
    cmd(str || "");
}

/* Standardfunktionen fuer Raeume. *
 * ------------------------------- */
string query_long(object viewer)
{
    if (viewer == player || !player || !portal)
        return "";

    switch (lower_case(portal[P_CONF_DEST_MUD]))
    {
        case "orbit":
            return Er(player) + " befindet sich im Orbit um UNItopia.";

        case "lp245":
            return Er(player) + " befindet sich in der Vergangenheit.";

        case "seifenblase":
            return Er(player) + " träumt gerade.";
    }
}

string query_long_dark(object viewer)
{
    return query_long(viewer);
}

string query_short(object viewer)
{
    if (viewer == player || !player || !portal)
        return "";

    switch (lower_case(portal[P_CONF_DEST_MUD]))
    {
        case "orbit":
            return "Im Orbit";

        case "lp245":
            return "In der Vergangenheit";

        case "seifenblase":
            return "Im Traum";
    }
}

void create()
{
    "*"::create();
    add_type(({RT_MAGIE_VERBOTEN, RT_HANDWERK_VERBOTEN, RT_KAEMPFEN_VERBOTEN, RT_STEHLEN_VERBOTEN,
               RT_TELEPORT_REIN_VERBOTEN, RT_TELEPORT_RAUS_VERBOTEN,
               RT_VERSAND_REIN_VERBOTEN, RT_VERSAND_RAUS_VERBOTEN,
               RT_KEIN_VERBRAUCH, RT_KUNSTLICHT}), 1);

    call_out(#'check_room, 2);
}

void init()
{
    "*"::init();
    if(!this_player()->uses_vt100client())
    {
        add_action("cmd", "", AA_NOSPACE);
        input_to("cmd_loop", INPUT_IGNORE_BANG);
    }
}

void check_room()
{
    if (!first_inventory())
        remove();
}

void abort_renewal() {}

void init_from_renewal(object orig_player, mapping orig_portal, string orig_src_room)
{
    if(strstr(object_name(previous_object()), "/room/portal/"))
        return;

    player = orig_player;
    portal = orig_portal;
    src_room = orig_src_room;
}

void finish_renewal(object neu)
{
    neu->init_from_renewal(player, portal, src_room);
}

void prepare_renewal() {}
