// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/room/rathaus/obj/hinweis_srwahl.c
// Description:

inherit "/i/item";
inherit "/i/install";

#define SR_KANDIDATENLISTE "/apps/spielerrat_kandidatenliste"

string query_long(object beo)
{
    switch (SR_KANDIDATENLISTE->get_phase())
    {
        default:
        case 0:
            call_out("remove",0);
            return wrap("Das Hinweisschild löst sich auf.");
        case 1:
            return wrap("Die Meldephase der Kandidaten zur Spielerratswahl "
             "hat begonnen und findet im Tadmorer Rathaus bei Leo statt. "
             "Die Wahlplakate sind auf der Treppe vor dem Rathaus zu sehen.");
        case 2:
            return wrap("Die Spielerratswahl läuft auch Hochtouren bei "
                "Leo im Rathaus zu Tadmor. Die Wahlplakate sind auf der "
                "Treppe vor dem Rathaus zu sehen.");
        case 3:
            return wrap("Die Ergebnisse der Spielerratswahl sind bei Leo "
                "im Rathaus zu Tadmor einzusehen.");
    }
}

varargs string query_read(string parse_rest, string str,object leser)
{
    return query_long(leser);
}

void create() {
    set_id(({"hinweisschild","hinweis","schild","wahlhinweis","wahl # hinweis"}));
    set_name("hinweisschild");
    set_gender("saechlich");
    set_short("HINWEISSCHILD ZUR SPIELERRATSWAHL");
}
