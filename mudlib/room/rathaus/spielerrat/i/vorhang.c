// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/room/rathaus/spielerrat/i/room.c
// Description:	Inherit fuer die Spielerratsraeume

/* --- Includes & Defines: --- */
#include <level.h>
#include <message.h>
#include <move.h>
#include <notify_fail.h>
#include <parse_com.h>
#include <simul_efuns.h>
#include <misc.h>
#include <input_to.h>

#define AUSGANG "/room/rathaus/foyer"
#define SRSEC (TI && TP == TI && (spielerratp(TI) || adminp(TI)))


/* --- Globale Variablen: --- */
int vorhang_geschlossen;

/* --- Funktionen: --- */

void check_vorhang()
{
    if(!player_present(TO, PPRESENT_STATUES))
    {
        // Keine Spieler mehr da, Vorhang ist per default offen:
        vorhang_geschlossen = 0;
    }
}

int vorhang_spy()
{
    if(vorhang_geschlossen) return 1;
}

<int|string> vorhang_f_move_in(string message, mapping mv_infos)
{
    object wer = mv_infos[MOVE_OBJECT];
    check_vorhang();

    if(!objectp(wer) || !living(wer) || ENVR(wer) == this_object() ||
       spielerratp(wer) || adminp(wer))
    {
        return 0;
    }

    if(vorhang_geschlossen)
    {
        TO->send_message_to(wer, MT_NOTIFY, MA_SENSE, 
            wrap("Dieser Raum ist momentan geschlossen. Er kann momentan nicht "
                 "betreten werden. Bitte notfalls einen Admin oder einen Spielerrat "
                 "um Einlass."));
        return "Ein großer, schwerer Vorhang versperrt den Weg.";
    }

    if(sizeof(filter(all_inventory(), (: spielerratp($1) || adminp($1) :))) == 0)
    {
        TO->send_message_to(wer, MT_NOTIFY, MA_SENSE,
            wrap("Dieser Raum kann nur betreten werden, wenn sich darin gerade "
                 "ein Mitglied des Spielerrates oder ein Admin aufhält."));
        return "Eine unsichtbare Kraft versperrt den Weg.";
    }
}
//
// Vorhang-Action zum Oeffnen und Schliessen
//

int vorhang_auf(string str)
{
    if(!SRSEC)
    {
        return 0;
    }

    if(TO->here(str, "vorhang"))
    {
        if(vorhang_geschlossen)
        {
            vorhang_geschlossen = 0;
            TO->send_message(MT_LOOK, MA_CRAFT,
                wrap(Der(TP)+" öffnet den Vorhang, so dass der Raum wieder frei "
                     "betreten werden darf."),
                wrap("Du öffnest den Vorhang, so dass der Raum wieder frei "
                     "betreten werden darf."),
                TP);
            return 1;
        }

        else
        {
            return notify_fail(wrap("Der Vorhang ist doch bereits offen!\n"), FAIL_INTERNAL);
        }
    }

    return notify_fail("öffne was?\n", FAIL_NOT_OBJ);
}

int vorhang_zu(string str)
{
    if(!SRSEC)
    {
        return 0;
    }

    if(TO->here(str, "vorhang"))
    {
        if(!vorhang_geschlossen)
        {
            vorhang_geschlossen = 1;
            TO->send_message(MT_LOOK, MA_CRAFT,
                wrap(Der(TP)+" schließt den Vorhang. Der Raum kann nun nicht "
                     "mehr betreten werden."),
                wrap("Du schließt den Vorhang. Der Raum kann nun nicht "
                     "mehr betreten werden."),
                TP);
            return 1;
        }

        else 
        {
            return notify_fail(wrap("Der Vorhang ist doch bereits zu!\n"), FAIL_INTERNAL);
        }
    }

    return notify_fail("schließe was?\n", FAIL_NOT_OBJ);
}

int vorhang(string str)
{
    if(!SRSEC)
    {
        return 0;
    }

    if(strstr(str, "auf") != -1)
    {
        return vorhang_auf(str);
    }

    else if(strstr(str, "zu") != -1)
    {
        return vorhang_zu(str);
    }

    else if(TO->here(str, "vorhang"))
    {
        return notify_fail("ziehe vorhang auf oder zu?\n", FAIL_WRONG_ARG);
    }

    return notify_fail("ziehe was auf oder zu?\n", FAIL_NOT_OBJ);
}

//
// Rauswurf-Action um Unbefugte des Raumes zu verweisen
//

int rauswurf(string str)
{
    mixed * parse;
    object ob;

    if(!SRSEC)
    {
        return 0;
    }

    parse = parse_com(str, TO, 0, PARSE_NO_V_ITEMS);

    if(parse_com_error(parse, "rauswurf <name>\n", 
           "Du kannst nur einen auf einmal rauswerfen.\n"))
    {
        return 0;
    }

    ob = parse[PARSE_OBS][0];

    if(spielerratp(ob) || adminp(ob))
    {
        return notify_fail(wrap("Es macht überhaupt keinen Sinn, "
            "Spielerräte oder Admins rauszuwerfen, die kommen eh "
            "wieder."), FAIL_INTERNAL);
    }

    else if(living(ob))
    {
        sys_log("SR_RAUSWURF", sprintf("%s | %10-s rauswurf %#Q\n",
            shorttimestr(time()), TI->query_real_name(), 
            ob->query_real_name() || 
            ({ob, ob->query_name(), ob->query_short()})));

        ob->move(AUSGANG,([MOVE_FLAGS:MOVE_FORCE]));

        TO->send_message(MT_NOTIFY, MA_MOVE_OUT,
            wrap(Der(ob)+" wurde aus dem Raum geworfen."),
            "Du wurdest aus dem Raum geworfen.\n",
            ob);

        return 1;
    }

    else
    {
        TO->send_message_to(TI, MT_NOTIFY, MA_SENSE,
            wrap(Der(ob)+" ist nicht lebendig! "+Er(ob)+" kann "
            "nicht rausgeworfen, sondern höchstens zerstört werden.\n"
            "Bist du wirklich SICHER, dass du dieses Objekt SAMT INHALT zerstören willst? "
            "Dies darf nur in besonders begründeten Fällen geschehen und wird "
            "geloggt!"));
        input_to("rauswurf_remove", INPUT_PROMPT,
            "Antworte deutlich mit 'ja' oder 'nein' > ",
	    ob);
        return 1;
    }

    return notify_fail("rauswurf <wer oder was>\n");
}

void rauswurf_remove(string str, object ob)
{
    if(!SRSEC)
    {
        return 0;
    }

    if(str != "ja")
    {
        TO->send_message_to(TI, MT_NOTIFY, MA_SENSE,
            wrap(sprintf("Deine Antwort lautete '%s' und nicht 'ja'. Abbruch.",
                         str)));
    }

    else if(objectp(ob) && environment(ob) == TO)
    {
        str = sprintf("%#Q (%#Q %#Q)", ob, ob->query_name(), ob->query_short());

        catch(ob->close_con(), ob->remove(), destruct(ob); publish);

        if(ob)
        {
            TO->send_message_to(TI, MT_NOTIFY, MA_SENSE,
                "ACHTUNG! Objekt ließ sich nicht zerstören. Abbruch.\n");
        }

        else
        {
            sys_log("SR_RAUSWURF", sprintf("%s | %10-s remove %s\n",
                shorttimestr(time()), TI->query_real_name(), str));

            TO->send_message_to(TI, MT_NOTIFY, MA_SENSE,
                "Das Objekt wurde zerstört. Der Vorgang wurde geloggt.\n");
        }
    }

    else
    {
        TO->send_message_to(TI, MT_NOTIFY, MA_SENSE,
            wrap("ACHTUNG! Objekt befindet sich nicht mehr im Raum oder "
                 "existiert nicht mehr. Abbruch."));
    }
}

//
// Applied LFuns
//

void create()
{
    TO->add_controller("forbidden_move_in", #'vorhang_f_move_in);
    TO->add_controller("notify_moved_out", #'check_vorhang);
    TO->add_controller("forbidden_spy_here", #'vorhang_spy);

    TO->add_v_item(
    ([
        "name"   : "vorhang",
        "gender" : "maennlich",
        "id"     : ({"vorhang", "wandvorhang", "tuervorhang"}),
        "long"   : "Ein großer, schwerer Vorhang. Er kann vor die Tür "
                   "gezogen werden. Dadurch kann man den Raum zwar noch "
                   "verlassen, aber keinesfalls mehr betreten. Auch die "
                   "Sicht in den Raum wird durch ihn eingeschränkt.\n"
                   "Er kann nur von Mitgliedern des Spielerrats und von "
                   "Admins geöffnet oder geschlossen werden.",
    ]) );
}

void init()
{
    add_action("vorhang", "ziehe");
    add_action("vorhang_zu", "schließe", -3);
    add_action("vorhang_auf", "öffne", -3);

    add_action("rauswurf", "rauswurf", -4);
    add_action("rauswurf", "werfe", -4);
    add_action("rauswurf", "rausschmiss", -4);
}

void reset()
{
    check_vorhang();
}

/* --- End of file. --- */
