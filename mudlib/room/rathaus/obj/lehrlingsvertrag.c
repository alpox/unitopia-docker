// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /room/rathaus/obj/lehrlingsvertrag.c
// Description: Der Lehrlingsvertrag

inherit "/i/item";
inherit "/i/move";
inherit "/i/tools/security";

#include <config.h>
#include <level.h>
#include <more.h>
#include <apps.h>

#define CAP(x) capitalize(x)
#define FAIL(x) { notify_fail(x); return 0; }
#define CHECK_LEO if (!previous_object() || object_name(previous_object())!= \
	"/room/rathaus/div/leo") return

string lehrling, vogt, erstie, admin;
int signed;

void create() {
    set_name("lehrlingsvertrag");
    set_short("Der Lehrlingsvertrag");
    set_gender("maennlich");
    set_id(({"vertrag", "vertrach","lehrlingsvertrag"}));
    set_weight(0);
    init_security_for_actions();
}

int query_prevent_shadow(object shadow) { return 1; }

void init() {
    add_action("unterschreibe","unterschreibe",-12);
    add_action("unterschreibe","unterzeichne");
    add_action("zerreisse","zerreiße",-7);
}

int query_signed() { return signed; }

string query_long(object who)
{
    string tmp;

    switch(signed) {
      case 0:
	tmp="Bisher hat noch niemand unterschrieben.";
	break;
      case 1:
	tmp=CAP(lehrling)+" hat schon unterschrieben.";
	break;
      case 2:
	tmp=CAP(vogt)+" hat schon unterschrieben.";
	break;
      case 3:
	tmp="Beide haben unterschrieben.";
	break;
      default:
	tmp="Irgendwas ist schiefgelaufen (Freaky)";
      }
   return wrap("Der Lehrlingsvertrag ist eine Pergamentrolle, an der ein "
	"großes rotes Siegel hängt. Mit diesem Lehrlingsvertrag bestätigt "
	"jeder Gott, sich an den Götterkodex zu halten.\n"+
	(lehrling && vogt?
	"Dieser Vertrag besagt, dass "+CAP(lehrling)+" als Lehrling bei "+
	CAP(vogt)+" in Lehre gehen möchte.\n"+tmp:" "));
}

varargs string query_read(string rest, string str)
{
    string text;

    text = read_file("/static/goetter/vertrag.lehrling");

    if(erstie)
        text += read_file("/static/goetter/vertrag.zweitie");
    if(lehrling && vogt)
       text = implode(explode(implode(explode(text, "$L"), CAP(lehrling)),
						    "$V"), CAP(vogt));
    text += "   "+implode(explode(vtimestr(vtime())," ")[0..<2]," ")+"\n\n";
    text += (erstie ? "    Zweitie " : "   Engel ");
    text += (query_signed() & 1 ? CAP(lehrling) : "..........");
    text += "\t\t"+"Vogt "+(query_signed()&2? CAP(vogt)     : "..........");
    if(admin)
        text += "\t\tAdmin "+capitalize(admin);
    text += "\n";

    this_player()->more(explode(text,"\n"),"   --Mehr--",0,M_AUTO_END);
    return "";
}

int unterschreibe(string str)
{
    string name;

    if (!check_security())
	return 0;

    if (!me(str))
	FAIL("Was willst du "+query_verb()+"n ?\n");

    if (!playerp(this_interactive()))
	FAIL(Den()+" können nur Spieler unterschreiben.\n");

    name = this_interactive()->query_real_name();
    if (!name)
	FAIL("Huch!!! Du hast ja gar keinen Namen.\n");

    if (!lehrling || !vogt)
	FAIL(Dieser()+" ist noch nicht zum "+CAP(query_verb())+
		"n vorbereitet.\n");

    if (name != lehrling && name != vogt)
	FAIL(Diesen()+" dürfen nur "+CAP(lehrling)+" und "+CAP(vogt)+" "+
		query_verb()+"n\n");

    if (name != lehrling && !GOETTER_REGISTER->query_lehrerlaubnis(name) &&
	!adminp(this_interactive()))
       FAIL(
          "Du musst Dir erst die Lehrerlaubnis von einem Domainlord holen.\n");


    if (name==lehrling)
    {
	if (signed & 1)
	    FAIL("Du hast "+diesen()+" schon unterzeichnet.\n");
	signed |= 1;
    }
    else if (name == vogt)
    {
	if (signed & 2)
	    FAIL("Du hast "+diesen()+" schon unterzeichnet.\n");
	if (!wizp(this_interactive()))
	   FAIL("Nur ein Gott darf hier unterzeichnen.\n");
	signed |= 2;
    }
    say(wrap(Der(this_player())+
	" unterzeichnet mit schnörkeligen Buchstaben "+den()+"."));
    write("Du unterschreibst "+den()+".\n");
    return 1;
}

int zerreisse(string str)
{
   if(!me(str))
      FAIL("Zerreiße was?\n");
   write("Du zerreist "+deinen()+".\n");
   say(Der(this_player())+" zerreißt "+seinen()+".\n");
   remove();
   return 1;
}

void set_vogt(string str)
{
    if (!stringp(str) || str=="")
	return;

    CHECK_LEO;
    vogt = str;
}

void set_lehrling(string str)
{
    if (!stringp(str) || str=="")
	return;

    CHECK_LEO;
    lehrling = str;
}

void set_admin(string str)
{
    // Wird nur bei Zweitievertraegen aufgerufen
    if(!stringp(str) || str=="")
        return;

    CHECK_LEO;
    admin = str;
}

void set_erstie(string str)
{
    // Wird nur bei Zweitievertraegen aufgerufen
    if(!stringp(str) || str=="")
        return;

    CHECK_LEO;
    erstie = str;
}

string query_vogt() { return vogt; }

string query_lehrling() { return lehrling; }

string query_admin() { CHECK_LEO (0); return admin; }

string query_erstie() { CHECK_LEO (0); return erstie; }

string query_look_msg()
{
   object ll;
   return lehrling && (ll = present(lehrling, environment(this_player()))) ?
      Der(this_player())+" schaut "+
      (ll == this_player() ? seinen() : ihren(0,0,ll)) + " an."
     : 0;
}

string query_read_msg()
{
   object ll;
   return lehrling && (ll = present(lehrling, environment(this_player()))) ?
      Der(this_player())+" liest "+
      (ll == this_player() ? seinen() : ihren(0,0,ll)) + "."
     : 0;
}
