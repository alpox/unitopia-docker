//              Tmm        (15.01.02) - query_long
//                         (10.05.03) - member_array durch member ersetzt
//

nosave variables inherit"/i/item.c";
nosave variables inherit"/i/move.c";

#include <config.h>
#include <misc.h>

#define SAVEFILE "/var/spende"

string betrag,datum;

void create()
{

    ::create();
    restore_object(SAVEFILE);
    set_id( ({ "meter", "spend-o-meter","spend","spendometer","spende" }) );
    set_name("Spend-o-meter");
    set_gender("saechlich");
    set_material( ({"metall"}) );
    set_short("Ein Spend-o-meter");
} 

void init() 
{
    add_action("geld","setze",-4);
    add_action("datum","datum",1);
}

string query_long(object b)
{
    return(wrap("Dies ist ein Messgerät, das die Spendierfreudigkeit der "
        "UNItopianer misst. Einen eigenen Rechner haben wir ja schon. "
        "Jedoch wird immer wieder Geld für Erweiterung und "
        "Instandhaltung benötigt. Unser "
        "Spendenkonto ist :\n\n"
        "  Trägerkreis UNItopia e.V.\n"
	"  Kontonummer: 13 00 34 5\n"
	"  Bank: Landesbank Baden-Württemberg\n"
	"  Bankleitzahl: 600 501 01\n\n"
 "Nähere Infos zum Trägerkreis gibts am Trägerkreisbrett, im ersten Stock "
 "des Rathauses zu Tadmor, oder unter WWW :\n"
"http://www.UNItopia.de/traegerkreis\n\n"
 "Das Spend-o-meter zeigt "+betrag+" DM an.\n"
 "Der momentane Stand ist vom "+datum));
}

int geld(string str)
{
    string name;

    name=this_player()->query_real_name();
    if(playerp(this_player()) && (member(VORSTAND,name)+1))
    {
	betrag=str;
	tell_object(this_player(), "Betrag: "+betrag+" wurde gesetzt.\n");
	save_object(SAVEFILE);
	return 1;
    }
    else
	return notify_fail("Du darfst den Spendenbetrag nicht setzen.\n");
}

int datum(string str) 
{
    string name;

    name=this_player()->query_real_name();
    if(playerp(this_player()) && (member(VORSTAND, name)+1))
    {
	datum=str;
	tell_object(this_player(),"Datum: "+datum +" wurde gesetzt.\n");
	save_object(SAVEFILE);
	return 1;
    }
    else
    {
	write("DU darfst das nicht.\n");
	return 1;
    }
}
