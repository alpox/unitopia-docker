// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/i/tools/update_actions
// Description:	Braucht man, wenn man add_actions aus Shadows macht :(
// Author:	Freaky (24.01.96)

#pragma save_types

/*
FUNKTION: update_actions
DEKLARATION: void update_actions(object ob)
BESCHREIBUNG:
Wenn ein shadow Actions fuer Umstehende (typischerweise im ueberlagerten
init()) anmeldet, so muss es /i/tools/update_actions inheriten, damit
beim Ueberwerfen und Entfernen diese Actions ordentlich an- bzw. abgemeldet
werden. Ein Aufruf dieser Funktion ist aber nicht notwendig.
VERWEISE: init_shadow, init
GRUPPEN: shadow
*/
void update_actions(object ob)
{
    if (objectp(ob) && environment(ob))
	efun::move_object(ob,environment(ob));
}
