// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/room/rathaus/div/recycl-o-mat
// Description: Hilft Junggoettern, sinnlose Tools wieder loszuwerden.
// Idee:        Anin.
// Author:      Sissi.

inherit "/i/monster/monster";

#include <message.h>

void schluck(object what, object woher)
{
    if (!what || !present(what,this_object()))
	return;
    send_message(MT_LOOK,MA_MOVE,wrap(Der()+" wirft "+den(what)+
	" hoch in die Luft, fängt "+ihn(what)+" mit dem Mund wieder "
	"auf und schluckt "+ihn(what)+" runter."),
	wrap(Der()+" wirft "+Deinen(what)+
	" hoch in die Luft, fängt "+ihn(what)+" mit dem Mund wieder "
	"auf und schluckt "+ihn(what)+" runter."), woher);
    what->remove();
    if (what) destruct(what);
}

void create()
{
    ::create();
    initialize("ork",100);
    set_name("recycl-o-mat");
    set_id(({"recycl-o-mat","recyclomat"}));
    set_gender("maennlich");
    add_controller("notify_show_whom");
    clone_object("/obj/soul")->move(this_object());
    set_accept_objects(({#'accept_from_void, #'schluck}));
}

string query_long(object wer)
{
    if(present(this_player(),environment()))
        call_out("hallo",2,wer);
    return wrap("Ein kleiner, niedlicher Recycl-O-Mat steht freudestrahlend "
        "vor Dir und wartet nur drauf, dass er ein sinnloses Tool aufessen "
        "darf.");
}

void hallo(object wer)
{
    if (!wer || !present(wer,environment())) return;
    do_command("sage Hallo, ich bin "+der()+" und habe fürchterlichen "
        "Hunger. Wenn Du ein Tool mit Dir rumträgst, das Du nicht mehr "
        "brauchst, fresse ich es sehr gerne auf. Zeige es mir einfach.");
}

void notify_show_whom(object who, mixed what,object whom)
{
    if(mappingp(what))
    {
	call_out("do_command",2,"seufz");
	call_out("do_command",4,"sage "+
	    ((what["environment"]==who)?Deinen(what,0,who):Den(what))+
	    " kann ich nicht fressen. "
	    "Davon wird mir immer fürchterbar übelig.");
	return;
    }
    if(!present(what,who))
    {
        call_out("do_command",2,"schmolle");
        call_out("do_command",4,
            "sage Och, "+den(what)+" darf ich nicht fressen, sonst werde "
            "ich wieder ausgeschimpft.");
        return;
    }
    call_out("do_command",2,"freue");
    call_out("do_command",4,"sage Au ja, "+den(what)+" fress ich gerne auf.");
    call_out("friss",6,what);
}

void friss (object what)
{
    if (!this_player()||!present(this_player(),environment())||!what
        ||!present(what,this_player())) return;
    send_message(MT_LOOK,MA_MOVE,wrap(Der()+" hüpft an "+dem(this_player())
        +" hoch und frisst "+den(what)+" mit lautem Schmatzen auf."),
        wrap(Der()+" hüpft an Dir hoch und frisst "
        +deinen(what,0,this_player())+" mit lautem Schmatzen auf."),
        this_player());
    what->remove();
    if (what) destruct(what);
}
