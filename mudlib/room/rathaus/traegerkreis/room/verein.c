/* File: /d/Vaniorh/fizban/sitzung.c
   Autor: urspruenglich Monty
          ueberarbeitet von Fizban
   Datum: 30.10. 1996
   Bemerkung: Das Vereinsheim des Traegerkreises UNItopia
   sin 23.8.99 Spendometer umgestellt
//              Tmm        (15.01.02) - query_long
//                         (11.04.02) - Workomat wird getoucht, nicht geclont
   sin 25.5.02 spendomat ausgebaut
   sin 16.12.02 kaempfen_verboten raus
   Tmm 10.05.03 ein wenig verschoenert
   Ranger 24.11.03 Bankverbindung aktuallisiert
*/

inherit "%room";

#include <verein.h>
#include <level.h>
#include <move.h>
#include <invis.h>
#include <landschaft.h>

object tussi;


string satzung_read() {
    SATZUNG_TEXT
}


int query_sitzung() {
    return abs_path("sitzung")->query_logging();
}


void reset()
{
    object brett;
#if 0
   if (!present("spend-o-meter")) {
      object spend=clone_object(OBJ_PFAD"spende");
      spend->move(this_object());
      spend->set_no_move(1);
   }
#endif
    if(!brett=present("newsbrett",TO)) {
        brett=clone_object("/obj/brett");
        brett->set_adjektiv("wichtig");
        brett->set_brett_name("Trägerkreis");
        brett->move(TO);
    }
    brett->set_invis(V_NOLIST);

    if(!tussi=present("tussnelda",TO)) {
        tussi=touch(NPC_PFAD+"tussnelda");
        tussi->move(TO,([MOVE_FLAGS:MOVE_MAGIC]));
    }
#if 0
    if (!present("woerkomat", TO))
    {
      object work=touch(VEREIN_PFAD"touch/worko");
      work->move(TO);
      work->add_id("woerkomat");
    }
#endif
   if(!present("spendenschild",TO)){
      object o;
      o=clone_object("/obj/schatz");
      o->set_name("Spendenaufruf-Schild");
      o->set_gender("saechlich");
      o->set_id(({"spendenaufrufschild","spendenschild","aufrufschild",
         "schild"}));
      o->set_long("Für seine Schlichtheit sieht es sehr bedeutungsvoll aus.\n"
         "Du solltest es vielleicht mal lesen.");
      o->set_read("Für Erweiterung und Instandhaltung wird immer Geld "
                  "benötigt.\nUnser Spendenkonto lautet:\n\n"
        "  Trägerkreis UNItopia e.V.\n"
        "  IBAN: DE72 8306 5408 0006 8975 84\n"
        "  BIC: GENODEF1SLR\n"
        "  Bank: Deutsche Skatbank\n"
        "\n"
        "Nähere Infos zum Trägerkreis gibts am Trägerkreisbrett "
        "oder im WWW:\n"
        "http://www.UNItopia.de/traegerkreis\n\n");
      o->move(TO);
      o->set_no_move(1);
   }
}


string schild_long(mapping v,object who)
{
    string str;

    str="+----------------------------------------------------------------------+\n"
    "|                                                                      |\n"
    "|"+center("Sitzungszimmer des Vorstandes",70)+"|\n";

    if(query_sitzung())
        str+="|"+center("*** Ruhe Bitte, Sitzung ist im Gange! ***", 70)+"|\n";

    str+="|"+center("Eintritt nur für Vorstands- und Beiratsmitglieder",70)+"|\n"
    "|                                                                      |\n"
    "+----------------------------------------------------------------------+\n";

    return wrap(str);
}

string schild_read(string parse_rest, string text, mapping v, object who)
{
    return schild_long(v, who);
}


string query_long(object o)
{
    string str;
    object ob;

    str="Hier ist das Vereinsheim des Trägerkreises UNItopia e.V. "
    "Noch ist es hier ein wenig trostlos. An der Wand hängen einige "
    "eingerahmte Schriftstücke. Im Norden ist ein Durchgang, der zum "
    "Sitzungszimmer des Vorstandes führt. Ein Schild hängt daran. "
    "Eine Treppe führt nach unten in das Foyer. ";

    if(ob=present("brett",TO))
        str+=Ein(ob)+" hängt an der Wand. ";
    else str+="Irgendwer hat das schwarze Brett geklaut! ";

    if(!ob=present("sekretaerin",TO))
        str+="Die Sekretärin hat wohl gerade Ausgang.";

    return wrap(str);
}


void create()
{
    ::create();

    set_own_light(1);
    add_type("kunstlicht",1);
    add_type(LANDSCHAFT,L_DRINNEN|L_HAUS|L_SIEDLUNG);
    add_type("stehlen_verboten","Willst Du etwa wirklich HIER stehlen???\n");
    add_type("keine_magie","Die magische Aura dieses Raumes verhindert jede Magie!\n");
    add_type("graben_verboten",1);

    set_short("Das Vereinsheim");

    add_v_item(([
        "name":"schriftstücke",
        "gender":"saechlich",
        "plural":1,
        "id":({"stücke","schriftstücke","stück","schriftstück"}),
        "adjektiv":"eingerahmt",
        "long":"Dort hängt eine Satzung, eine Vorstandsliste und eine "
        "Beiratsliste. Die Listen kann man alle lesen.",
        "read":"Du musst das Schriftstück, dass Du lesen willst, schon "
        "genauer benennen."
      ]));

    add_v_item(([
        "name":"satzung",
        "gender":"weiblich",
        "long":"Ein ellenlanges Stück Pergament hinter Glas. Die Satzung "
        "sieht reichlich amtlich aus. Darunter stehen die Unterschriften "
        "der Gründungsmitglieder. Man kann die Satzung lesen.",
        "read":#'satzung_read
      ]));

    add_v_item(([
        "name":"unterschriften",
        "gender":"weiblich",
        "plural":1,
        "id":({"unterschriften","unterschrift","mitglieder","gründungsmitglieder"}),
        "long":sizeof(GRUENDER)+" Namen, teilweise fast unleserlich gekrakelt. "
        "Mit ein wenig Mühe kann man sie lesen.",
        "read":"\n"+GRUENDER_NAMEN+"."
      ]));

    add_v_item(([
        "name":"vorstandsliste",
        "gender":"weiblich",
        "id":({"vorstandsliste","liste","vorstand"}),
        "long":"Eine Liste mit "+sizeof(VORSTAND)+" Namen. "
        "Man kann sie bei Bedarf auch lesen.",
        "read":"\n"+CHEF_NAMEN+"."
      ]));

    add_v_item(([
        "name":"beiratsliste",
        "gender":"weiblich",
        "id":({"beiratsliste","liste","beirat"}),
        "long":"Eine Liste mit "+sizeof(ADMINS)+" Namen. Lesen könnte hier "
        "einiges an Erleuchtung bringen.",
        "read":"\n"+FUFFI_NAMEN+"."
      ]));

    add_v_item(([
        "name":"schild",
        "gender":"saechlich",
        "long":#'schild_long,
        "read":#'schild_read
      ]));

    set_exits(({AUSGANG, "sitzung", "kueche"}), ({"runter", "norden", "süden"}));
    delete_exit("süden");  // ist noch nicht fertig

    reset();
}


int filter_norden(object who)
{
    if(!who || !living(who))
        return 0;

    if(playerp(who) && (CHEF(who)))
        return 0;

    if(!tussi=present("tussnelda",TO))
        return 0;

    tell_object(who,wrap(Der(tussi)+" versperrt Dir den Weg."));
    tell_room(TO,wrap(Der(tussi)+" versperrt "+dem(who)+" den Weg."),({who,tussi}));
    tussi->do_command("sage Da dürfen nur Mitglieder des Vorstandes oder des "
      "Beirates hinein.");
    return 1;
}
