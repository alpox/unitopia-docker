// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /secure/dbus/mail.c
// Description: RPC-Funktionen fuer E-Mail
// Author:      Gnomi

#include <apps.h>
#include <level.h>
#include <mail.h>

private mapping ignore_header = ([ "from", "to", "cc", "subject" ]);

static void mail_receivemail(string player, string from_addr, string* cc,
    string subject, int date, string text, mixed* header)
{
    MAILD->deliver_mail(
        player,					/* To */
        from_addr,				/* From */
        cc[1..],				/* Cc */
        subject,				/* Subject */
        text,					/* Mail Body */
        sizeof(cc) && cc[0],			/* real-to */
        implode(map(header, function string(string* entry)
        {
            if(member(ignore_header, lower_case(entry[0])))
                return "";
            return implode(entry, ": ") + "\n";
        }), "")					/* header */
    );
}

static int mail_is_allowed_sender(string realname, string sendername)
{
    string wiz;

    if(realname == sendername)
        return 1;

    wiz = SECOND->is_special(sendername);
    if(wiz)
        return GROUP_MASTER->is_group_member(realname, wiz);

    wiz = testplayerp(sendername);
    if(wiz)
        return wiz == realname;
}

/* Aufrufe an externe Objekte */
void send_email(closure cb_success, closure cb_error, string from_addr, string to_addr, string header, int date, string* refs, string text)
{
    if(object_name(previous_object()) != INTERNET_MAILD)
        return;

    dbus_call_method(function void(string errname, varargs mixed* args)
    {
        if (!errname)
            funcall(cb_success);
        else
            funcall(cb_error, sprintf("%s: %Q\n", errname, args));
    }, "de.system", "/de/system/exim4", "de.unitopia.Exim4", "sendmail", "sssxass",
    from_addr, to_addr, header || "", date, refs || ({}), text);
}
