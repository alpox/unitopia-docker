// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/room/rathaus/obj/vogtvertrag.c
// Description: Vertrag fuer Voegte
// Author:	Garthan
// Modified:    Sissi, 11.5.98: Dritte Lehrmeister braucht keine Lehrerlaubnis

inherit "/i/item";
inherit "/i/move";
inherit "/i/tools/security";

#include <config.h>
#include <level.h>
#include <apps.h>
#include <more.h>

#define CAP(x) capitalize(x)
#define FAIL(x) { notify_fail(x); return 0; }
#define CHECK_LEO if (!previous_object() || object_name(previous_object())!= \
	"/room/rathaus/div/leo") return;

string vogt, vogt1, vogt2, lord;
int signed;

void create() {
    set_name("zeugnis");
    set_gender("saechlich");
    set_id(({"zeugnis", "vogtvertrag"}));
    set_weight(1);
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

    if (!signed)
	tmp="Bisher hat noch niemand unterschrieben.";
    else if (signed==15)
	tmp="Alle haben unterschrieben.";
    else {
	tmp="";
	if (signed & 1)
	    tmp+=CAP(vogt)+" und ";
	if (signed & 2)
	    tmp+=CAP(vogt1)+" und ";
	if (signed & 4)
	    tmp+=CAP(vogt2)+" und ";
	if (signed & 8)
	    tmp+=CAP(lord)+" und ";
	tmp=tmp[0..<5];
	if (signed==1 || signed==2 || signed==4 || signed==8)
	    tmp+="hat schon unterschrieben.";
	else
	    tmp+="haben schon unterschrieben.";
	}
   return wrap(
      "Dieses Zeugnis ermöglicht dem Inhaber, Vogt zu werden und in "
      "eine Domain einzutreten. Es besteht aus einer vergilbten "
      "Pergamentrolle und einem dicken roten Siegel, das an der Rolle "
      "hängt.\n"+
      (vogt ?
      "Das Zeugnis wurde für "+CAP(vogt)+" ausgestellt und "
      "benötigt, um gültig zu sein, die Unterschrift von "+
      CAP(vogt1)+", "+CAP(vogt2)+" und einem beliebigen Domainlord.\n"+tmp :
      "Das Zeugnis ist ungültig."));
}

varargs string query_read(string rest, string str)
{
    string text;

    text = read_file("/static/goetter/vertrag.vogt");
    if(vogt && vogt1 && vogt2)
       text = implode(explode(implode(explode(implode(explode(text,
		"$NV"), CAP(vogt)),
		"$V1"), CAP(vogt1)),
		"$V2"), CAP(vogt2));
    text += "   "+implode(explode(vtimestr(vtime())," ")[0..<2]," ")+"\n\n";
    text += "   Geselle "+(query_signed() & 1 ? CAP(vogt) : "..........")+
	    "\t"+"Lehrmeister "+(query_signed()&2? CAP(vogt1) : "..........")+
	    "\t"+"Lehrmeister "+(query_signed()&4? CAP(vogt2) : "..........")+
	    "\n"+"   Lord "+(query_signed() & 8? CAP(lord) : "..........");
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
	FAIL("Was willst du unterschreiben ?\n");

    if (!wizp(this_interactive()))
	FAIL(Den()+" können nur Götter unterschreiben.\n");

    name=this_interactive()->query_real_name();
    if (!name)
	FAIL("Huch!!! Du hast ja gar keinen Namen.\n");

    if (!vogt)
	FAIL(Dieser()+" ist noch nicht zum Unterschreiben vorbereitet.\n");

/*
    Sissi, 11.5.98: Dritte Lehrmeister braucht keine Lehrerlaubnis
    
    if (name != vogt && name != vogt1 && name != vogt2 &&
    	    !GOETTER_REGISTER->query_lehrerlaubnis(name))
	FAIL("Du musst Dir erst die Lehrerlaubnis von einem "
		"Domainlord holen.\n");
*/

    if (name == vogt)
    {
	if (signed & 1)
	    FAIL("Du hast "+diesen()+" schon unterzeichnet.\n");
	signed |= 1;
    }
    else if (name == vogt1)
    {
	if (signed & 2)
	    FAIL("Du hast "+diesen()+" schon unterzeichnet.\n");
	signed |= 2;
    }
    else if (name == vogt2)
    {
	if (signed & 4)
	    FAIL("Du hast "+diesen()+" schon unterzeichnet.\n");
	signed |= 4;
    }
    else
    {
	if (!this_interactive() || this_player()!=this_interactive() ||
		!playerp(this_player()))
	    FAIL("So nicht!\n");
	lord = this_player()->query_real_name();
	if(!lordp(this_player()))
	    FAIL("Nur ein Domainlord darf hier unterzeichnen!\n");
	if (signed & 8)
	    FAIL(Dieser()+" ist schon unterzeichnet.\n");
	signed |= 8;
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

void set_vogt(string str) {
    string tmp_vogt1, tmp_vogt2;

    if (!stringp(str) || str=="")
	return;

    CHECK_LEO;
    tmp_vogt1=GOETTER_REGISTER->query_lehrmeister(str);
    tmp_vogt2=GOETTER_REGISTER->query_lehrmeister(str,1);
    if (!tmp_vogt1 || !tmp_vogt2 || GOETTER_REGISTER->query_lehrmeister(str,2))
	return;
    vogt=str;
    vogt1=tmp_vogt1;
    vogt2=tmp_vogt2;
    if(vogt1 == "leo")
       signed |= 2;
    if(vogt2 == "leo")
       signed |= 4;
}

string query_vogt1() { return vogt1; }
string query_vogt2() { return vogt2; }
string query_lord() { return lord; }


string query_vogt() { return vogt; }

string query_look_msg()
{
   object vgt;
   return vogt && (vgt = present(vogt, environment(this_player()))) ?
      Der(this_player())+" schaut "+
      (vgt == this_player() ? seinen() : ihren(0,0,vgt)) + " an."
     : 0;
}

string query_read_msg()
{
   object vgt;
   return vogt && (vgt = present(vogt, environment(this_player()))) ?
      Der(this_player())+ " liest "+
      (vgt == this_player() ? seinen() : ihren(0,0,vgt)) +"."
     : 0;
}

mixed *query_auto_load()
{
   if(gesellep(environment()) && vogt == environment()->query_real_name())
   {
      if(playerp(previous_object()))
         call_out("do_remove", 0);
      return ({ vogt1, vogt2, lord, signed });
   }
}

void do_remove()
{
   if(vogt && !find_player(vogt))
      remove();
}

void init_arg(mixed *strs)
{
   if(gesellep(previous_object()) && sizeof(strs) >= 4)
   {
      vogt = previous_object()->query_real_name();
      vogt1 = strs[0];
      vogt2 = strs[1];
      lord = strs[2];
      signed = strs[3];
   }
}


