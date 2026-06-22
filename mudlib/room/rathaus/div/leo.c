// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /room/rathaus/div/leo.c
// Description: Leo, der Goetterbote, macht Goetter froh.

inherit "/i/monster/monster";
inherit "/room/rathaus/i/wizinfo";

#include <config.h>
#include <apps.h>
#include <stats.h>
#include <quest.h>
#include <game.h>
#include <gilden.h>
#include <deklin.h>
#include <level.h>
#include <parse_com.h>
#include <invis.h>
#include <soul.h>
#include <message.h>
#include <input_to.h>
#include <monster.h>
#include <move.h>

#define HOUR 3600
#define ANGEL_TIME_TO_WIZ (3*HOUR)
#define LEARNER_TIME (12*HOUR)
#define REJECT(x) { do_command(x); return 0; }

// learner
int lehrmeister;
string lehrling;

#include "rollen_weg.inc"
#include "leo_hlp.inc"
#include "leo_misc.inc"
#include "leo_lehrling.inc"
#include "leo_geselle.inc"
#include "leo_vogt.inc"

int query_prevent_shadow(object shadow) { return 1; }

int nein_danke(object ob)
{
    if (!present(ob,this_object()) || ob->id("seele"))
        return 1;
    do_command("sage Nein danke. Das will ich nicht.");
}

int gummigoettchen(object ob)
{
    if (!ob)
        return 0;
    if (!present(ob, this_object()))
        return 1;
    if (ob->id("gummigöttchen"))
    {
        if (sizeof(filter(all_inventory(),(: $1->id("gummigöttchen") :)))>1)
            do_command("iss gummigöttchen");
        else
            do_command("knuddel gummigöttchen");
        return 1;
    }
    return 0;
}

int orkmist(object ob, object woher)
{
    if (!woher || !ob)
        return 0;
    if (!present(ob,this_object()))
        return 1;
    if (ob->query_orkmist()) 
    {
        do_command("sage Ach ist das ein niedlicher Schreihals. Der will "
            "wohl erst noch ein Orkschlächter werden? Ist allerdings schon "
            "etwas arg rostig dafür...");
        exec_command("gib",woher,ob);
        return 1;
    }
    return nein_danke(ob);
}

int goetterbuch(object ob, object woher) {
    if(!woher)
        return 0;
    if (!present(ob,this_object()))
        return 1;
    if (!wizp(this_player()))
        do_command("sage Soso, Du willst also nicht den Weg der Götter "
            "einschlagen. Dann eben viel Spaß als Engel.");
    else
        do_command("sage Danke.");
    ob->remove();
    return 1;
}

void reset()
{
    if (!present("gummigöttchen"))
    {
        object bonbon=clone_object("/room/rathaus/obj/gummigott");
        bonbon->move(this_object(),([ MOVE_FLAGS: MOVE_ERR_REMOVE ]) );
    }
}
 
void create()
{
    monster::create();
    clone_object("/obj/soul")->move(this_object());
    initialize("mensch",100);
    set_name("leo");
    set_npc_name("leo");
    set_id(({"leo","bote", "götterbote"}));
    set_short("Leo, der Götterbote");
    set_long("Leo, der Götterbote. Er ist ein alter, betagter Mann, der "
             "sich sehr sorgsam um die Bedürfnisse werdender Engel und "
             "Götter kümmert und deshalb ein gutes Verhältnis zum "
             "Obersten Rat des Pantheons unterhält. Er trägt einen langen "
             "Bart und einen noch viel längeren weißen Umhang, welcher sein "
             "weiteres Äußeres verbirgt. Dieser Umstand verleiht ihm "
             "eine geheimnisvolle Aura.");
    set_personal(1);
    set_gender("maennlich");
    set_align(1000);
    ::load_chat(1,({"Es freut mich, Euch hier zu sehen."}));
    ::add_chats(({"Es freut mich, Euch hier zu sehen."}));

    set_parse_conversation(this_object(), ({
"engel:    engel && werden || will,möchte && engel ||"
        " [mach] && engel ", PARSE_SAY,
"keingott: kein && gott || nicht && gott ||nicht && götter", PARSE_SAY,
"lehrling: gott && werden || will,möchte && gott ||"
        " [mach] && gott ||"
        " götter && werden || nimm,nehm,nehme && götter ||"
        " nimm,nehm,nehme && auf", PARSE_SAY,
"lehrling: göttin && werden || will && göttin ||"
        "  [mach] && göttin ", PARSE_SAY,
"lehrling: lehrling && werden || will && lehrling ||"
        "  lehre && gehen ", PARSE_SAY,
"zweitie:  <fluester> && [zweitie] ", PARSE_SAY,
"geselle:  geselle && werden || will && geselle ||"
        "  gesellin && werden || will && gesellin ||"
        "  [mach] && geselle,gesellen,gesellin", PARSE_SAY,
"vogt:     vogt && werden || will && vogt ||"
        "  [mach] && vogt", PARSE_SAY,
"admin:    admin && werden || will && admin ||"
        "  [mach] && admin", PARSE_SAY,
"helfen:   will,möchte && helfen", PARSE_SAY,
"gummiaus: will,möchte && [kein,nicht] && gummigöttchen || "
"          will,möchte && gummigöttchen && [nicht]", PARSE_SAY,
"gummian:  will,möchte && gummigöttchen", PARSE_SAY,
"wiegehts: wie && [geht] && dir", PARSE_SAY,
"werbistdu: wer && bist && du", PARSE_SAY,
"wasengel: wer,was && engel", PARSE_SAY,
"wasgott: wer,was && gott,götter", PARSE_SAY,
"kobold: kobold,ecke", PARSE_SAY,
"neuer_tip: habe,hab && tip,tipp ||"
           "möchte,will && formblatt || "
           "<frag> && formblatt || tip,tipp && abgeben || "
           "möchte,will && tip,tipp && geben", PARSE_SAY,
"gruss:    hallo || hi || moin || moinmoin", PARSE_SAY,
"response: .*", PARSE_SAY|PARSE_RE_TRADITIONAL //sonst
        }));
    reset();
    set_accept_objects(({#'accept_from_void,
             #'orkschlaechter,   "ork#schlaechter",
             #'orkmist,          "kurzschwert",
             #'lehrlingsvertrag, "lehrlingsvertrag",
             #'gesellenvertrag,  "gesellenvertrag",
             #'vogtvertrag,      "vogtvertrag",
             #'goetterbuch,      "götterbuch",
             #'gummigoettchen,   "gummigöttchen",
             #'accept_invis,
             #'nein_danke,
             #'refuse}));
    init_misc();
}

void response(string str)
{
   if(!str)
      return;
   if(lehrmeister && lehrling)
      response_lehrling(str);
}
