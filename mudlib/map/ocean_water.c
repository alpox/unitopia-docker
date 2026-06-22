// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/map/ocean_water.c
// Description:	V-Item-Master fuer /obj/ocean_room.c
// Author:	Freaky (11.09.2000)

#include <deklin.h>

inherit "/i/item/v_item";

void create()
{
    closure smsg,osmsg;
    
    add_v_item(([
	"name": "meer",
	"gender": "saechlich",
	"id": ({"meer","ozean","# well #"}),
	"water_id": "salzwasser",
	"long": "Das Meer scheint unendlich weit.",
	"look_msg": "$Der(OBJ_TP) schaut sinnierend und "
		"traumversunken über das weite Meer",
	"smell": "Die Luft riecht salzig.",
	"noise": "Das Rauschen des Meeres beruhigt Dich.",
	"feel": "Nass.",
	"feel_msg": "$Der() taucht eine Hand ins Meerwasser.",
	"well_success_msg": smsg= (: 
	    (this_player()->query_koerperform()=="humanoid")
	    ?("Du schöpfst mit der Hand etwas Salzwasser "
	      "aus dem Meer und trinkst "+ihn($1)+".")
	    :("Du trinkst etwas "+wen($1,ART_KEINS)+" aus dem Meer.") :),
	"well_other_success_msg": osmsg= (: 
	    (this_player()->query_koerperform()=="humanoid")
	    ?(Der(this_player())+" schöpft mit der Hand etwas Salzwasser "
	      "aus dem Meer und trinkt "+ihn($1)+".")
	    :(Der(this_player())+" trinkt etwas "+wen($1,ART_KEINS)+" aus dem Meer.") :),
	"well_failure_msg": smsg,
	"well_other_failure_msg": osmsg,
	"well_fill_msg": "Du hältst $deinen('bottle) ins Meer und "
		"$er('bottle) füllt sich mit $dem('water).",
	"well_other_fill_msg": "$Der(OBJ_TP) hält $seinen('bottle) ins "
	    	"Meer und $er('bottle) füllt sich mit $dem('water)."
    ]));

    add_v_item(([
	"name": "meerwasser",
	"gender": "saechlich",
	"id": ({"meerwasser","salzwasser","wasser","# water #"}),
	"well_id": "meer",
	"long": "Das Wasser ist blau.",
	"look_msg": "$Der(OBJ_TP) schaut über das Meer.",
	"smell": "Riecht salzig, wie es sich für Meerwasser gehört.",
	"feel": "Nass.",
	"feel_msg": "$Der() taucht eine Hand ins Meerwasser.",
	"water_amount": -2,
	"water_success_msg": "Da das Wasser sehr salzig ist, macht Dich "
		"das eher durstiger.",
	"water_other_success_msg": "$Der(OBJ_TP) verzieht ziemlich das "
		"Gesicht.",
	"water_failure_msg": "Du spuckst das salzige Zeug gleich wieder aus.",
	"water_other_failure_msg": "$Der(OBJ_TP) spuckt das Salzwasser "
		"gleich wieder aus."
    ]));
}
