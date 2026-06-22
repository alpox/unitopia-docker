// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /i/player/mxp.c
// Description: Unterstuetzung fuer MXP

#include <invis.h>
#include <message.h>
#include <telnet.h>
#include <term.h>

#define CLIENT_MXP_WORKAROUND

#ifdef CLIENT_MXP_WORKAROUND
nosave int needs_mxp_workaround;
#endif

static int has_mxp()
{
    return this_object()->has_telnet_option(TELOPT_MXP, 1);
}

#ifdef CLIENT_MXP_WORKAROUND
static int mxp_needs_workaround()
{
    return needs_mxp_workaround;
}
#endif

protected void update_points_display()
{
    if (!interactive() || !has_mxp())
        return;

    this_object()->receive_message_low(
        sprintf(
            VT_MXP_TEMP_SECURE_MODE "<!ENTITY ap    \"%d\" DESC=\"Ausdauerpunkte\"          PUBLISH>"
            VT_MXP_TEMP_SECURE_MODE "<!ENTITY maxap \"%d\" DESC=\"maximale Ausdauerpunkte\" PUBLISH>"
            VT_MXP_TEMP_SECURE_MODE "<!ENTITY zp    \"%d\" DESC=\"%s\"                      PUBLISH>"
            VT_MXP_TEMP_SECURE_MODE "<!ENTITY maxzp \"%d\" DESC=\"maximale %s\"             PUBLISH>"
#ifdef CLIENT_MXP_WORKAROUND
            "%s"
#endif
            ,
            this_object()->query_hp(),
            this_object()->query_max_hp(),
            this_object()->query_sp(),
            this_object()->query_sp_name(),
            this_object()->query_max_sp(),
            this_object()->query_sp_name()
#ifdef CLIENT_MXP_WORKAROUND
            , (needs_mxp_workaround ? VT_MXP_LOCK_LOCKED_MODE : "")
#endif
            ));
}

static void init_mxp()
{
    if (!has_mxp())
        return;

    update_points_display();
    this_object()->receive_message_low(
        VT_MXP_LINE_SECURE_MODE         // Activate secure mode until newline
        "<!ELEMENT rlong      ''                        FLAG=RoomDesc>"
        "<!ELEMENT rshort     ''                        FLAG=RoomName>"
        "<!ELEMENT rexit      '<send expire=\"room\">'  FLAG=RoomExit>"
        "<!ELEMENT rexpire    '<expire name=\"room\">'>"
        "<!ELEMENT ircontent  '<send href=\"betrachte &id;|fuehle &id;|horche &id;|rieche &id;|nimm &id;\" expire=\"room\">' ATT='id'>"
        "<!ELEMENT lrcontent  '<send href=\"betrachte &id;|mustere &id;|begruesse &id;|sage zu &id; Hallo!\">' ATT='id'>"
        "<!ELEMENT iinventory '<send href=\"betrachte &id;|lege &id; hin|fuehre &id;|ziehe &id; an\">' ATT='id'>"
        "<!ELEMENT enzyfun    '<send href=\"? &text;\">'>"
        "<!ELEMENT enzybsp    '<send href=\"bsp? &text;\">'>"
        "<stat ap max=maxap caption=\"AP:\">"
        "<stat zp max=maxzp caption=\"" + this_object()->query_sp_short_name() + ":\">"
        "<sound Off U=\"https://www.unitopia.de/sound\">"
        VT_MXP_LOCK_LOCKED_MODE         // Lock to locked mode.
    );

#ifdef CLIENT_MXP_WORKAROUND
    {
        mixed client;
        if (this_object()->query_telnet(TELOPT_TTYPE, &client) &&
            sizeof(client)>1 && sizeof(client[1]) &&
            (!strstr(client[1][0], "mudlet")||
            (!strstr(client[1][0], "webmud3"))))
                needs_mxp_workaround = 1;
    }
#endif
}

private string calc_item_id(int arg, mapping attributes)
{
    if (!attributes)
        return 0;

    object* obs = attributes[MSG_MXP_ITEMS];
    if (sizeof(obs) <= arg)
        return 0;

    object ob = obs[arg];
    string id;
    foreach(string ob_id: ob->query_id())
    {
        if (!sizeof(ob_id) || sizeof(ob_id - "abcdefghijklmnopqrstuvwxyz1234567890-_"))
            continue;
        id = ob_id;
        break;
    }

    if (!id)
        return 0;

    if (!environment(ob))
        return " id=\"" + id + "\"";

    int num;
    foreach(object inv: all_inventory(environment(ob)))
    {
        if (inv == ob)
            break;
        if (inv->query_invis() & V_ATOM_NOSHIMMER)
            continue;
        if (!inv->id(id))
            continue;
        num++;
    }

    if (!num)
        return " id=\"" + id + "\"";
    return sprintf(" id=\"%d. %s\"", num+1, id);
}

private nosave mapping mxp_tag_names =
([
    VT_MXP_RSHORT: "rshort"; 0,                // Raum-Short
    VT_MXP_RLONG: "rlong"; 0,                  // Raum-Long
    VT_MXP_REXIT: "rexit"; 0,                  // Raum-Ausgang
    VT_MXP_REXPIRE: "rexpire"; 0,              // Raum-Expire
    VT_MXP_IROOMCONTENT: "ircontent"; 0,       // Gegenstand im Raum
    VT_MXP_LROOMCONTENT: "lrcontent"; 0,       // Lebewesen im Raum
    VT_MXP_IINVENTORY: "iinventory"; 0,        // Gegenstand im Spieler
    VT_MXP_ENZYFUN: "enzyfun"; 0,              // Lexikon: Funktion (? <name>)
    VT_MXP_ENZYBSP: "enzybsp"; 0,              // Lexikon: Beispiel (bsp? <name>)
]);

protected string process_mxp(string msg, mapping attributes)
{
    int mxp_enabled = has_mxp();

    msg = regreplace(msg, VT_ESC "\\[![0-9]+(;[0-9]+)*[st]",
        function string(string sub)
        {
            if (!mxp_enabled)
                return "";

            int typ_char = sub[<1];
            int is_close;
            if (typ_char == VT_MXP_OPEN_CHAR)
                is_close = 0;
            else if (typ_char == VT_MXP_CLOSE_CHAR)
                is_close = 1;
            else
                return sub; // das solle eigentlich nie passieren, 
                            // wenn die regexp korrekt ist

            int *args = map(explode(sub[3..<2], ";"), #'to_int);//');
            int code = args[0];
            string tag_name = mxp_tag_names[code];
            closure tag_arg_fun = mxp_tag_names[code, 1];

            if (!tag_name)
                return ""; // unbekannt fliegt raus
                           // oder vielleicht besser drin lassen?


            string tag_arg = "";
            if (!is_close && sizeof(args) > 1)
            {
                tag_arg = funcall(tag_arg_fun, args[1], attributes);
                if (!tag_arg)
                    return "";
            }

            string ret = VT_MXP_TEMP_SECURE_MODE
                + (is_close ? "</" : "<")
                + tag_name
                + tag_arg
                + ">";

#ifdef CLIENT_MXP_WORKAROUND
            if (needs_mxp_workaround)
                ret += VT_MXP_LOCK_LOCKED_MODE;
#endif

            return ret;
        },
    1);

    if (mxp_enabled)
    {
        string sound = attributes && attributes[MSG_SOUND];
        if (sound)
        {
            msg = VT_MXP_TEMP_SECURE_MODE "<sound \"" + sound + "\">"
#ifdef CLIENT_MXP_WORKAROUND
                + (needs_mxp_workaround ? VT_MXP_LOCK_LOCKED_MODE : "")
#endif
                + msg;
        }
    }

    return msg;
}

void create()
{
    // Machen wir spaeter wegen replace_program.
    mxp_tag_names[VT_MXP_IROOMCONTENT,1] = #'calc_item_id;
    mxp_tag_names[VT_MXP_LROOMCONTENT,1] = #'calc_item_id;
    mxp_tag_names[VT_MXP_IINVENTORY,1] = #'calc_item_id;
}
