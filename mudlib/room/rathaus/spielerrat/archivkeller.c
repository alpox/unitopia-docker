// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/room/rathaus/spielerrat/archivkeller.c
// Description: Rathaus, Spielerratsraeume, Protokollarchiv.

inherit "%room";

#include <apps.h>
#include <invis.h>
#include <level.h>
#include <message.h>
#include <misc.h>
#include <move.h>

#define DEBUGGER "myonara"
#include <debug.h>

#define PR_INDEX "/obj/protokollindex"

void reset()
{
    "*"::reset();
}

string query_long(object viewer)
{
    string text = ::query_long(viewer);
    if (wizp(viewer) || spielerratp(viewer))
    {
        return text
              +wrap("An der einen Wand fällt dir ein Protokollindex auf.");
    }
    return text;
}

string take_index(mapping vitem)
{
    if (wizp(TP) || spielerratp(TP))
    {
        if (present_clone(PR_INDEX,TP))
            return wrap("Du besitzt bereits einen Protokollindex.");
        object ob = clone_object(PR_INDEX);
        if (ob->move(TP,([MOVE_FLAGS:MOVE_ERR_REMOVE])) == MOVE_OK)
            return wrap("Du nimmst dir einen Protokollindex, probiere "
                "pr:? aus.");
        else
            return wrap("Du hast keinen Protokollindex nehmen können.");
    }
    return 0; // Da gibt es nichts zu holen.
}
int invis_index(mapping vitem)
{
    if (wizp(TP) || spielerratp(TP))
    {
        return V_HIDDEN;
    }
    return V_INVIS;
}

void create()
{
    "*"::create();
    set_short("Archivkeller der Spielerratsräume");
    set_long(
      "Du befindest dich im Protokollarchiv des Spielerrates, das im Keller "
      "des Rathauses untergebracht ist. Hinter einem Gitter befinden sich "
      "lange Reihen von Regalen mit vielen Ordnern.");
    add_v_item ((["name":"gitter","gender":"saechlich","id":({"gitter","tür"}),
      "long":"Ein robustes Gitter ohne erkennbare Tür."
    ]));
    add_v_item((["name":"regale","gender":"saechlich","plural":1,
        "id" : ({ "regale","regal","ordner" }),
        "long":"Du kannst nur erkennen, dass es viele Ordner gibt, "
        "aber nicht, was da enthalten ist.",
        "look_msg": "$Der(OBJ_TP) schaut in die Weiten der Regalreihen.",
        "smell":"Die Regale riechen nach Staub, das kitzelt in der Nase.",
        "smell_msg":"$Der(OBJ_TP) muss kräftig niesen.",
    ]));
    add_v_item(([
        "name":"index","gender":"maennlich",
        "id":({"index","protokollindex"}),
        "long" : "Der Protokollindex hilft dem Spielerrat oder Gott, "
                 "die Protokolle anzusehen. Du kannst den Index nehmen.",
        "look_msg": "$Der(OBJ_TP) schaut in die Weiten der Regalreihen.",
        "take": #'take_index,
        "invis" : #'invis_index,
    ]));
    

    set_exits( ({ "gang1" }), ({ "hoch" }) );

    reset();
}
