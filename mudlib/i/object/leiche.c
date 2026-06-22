// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/i/object/leiche.c
// Description:
// Eine verwesende Leiche. Neue Features:
// 1. In eine Leiche kann man nix mehr reinlegen.
// 2. Unsichtbare Objekte in der Leiche werden beim vergammeln zerstoert.

#pragma save_types

inherit "/i/item";
inherit "/i/move";
inherit "/i/contain";

#include <config.h>
#include <deklin.h>
#include <message.h>
#include <move.h>
#include <commands.h>
#include <landschaft.h>
#include <fail.h>
#include <stats.h>

#define DECAY_TIME          180
#define KEINE_TOTENGRAEBER_LANDSCHAFT \
    (L_WASSER|L_UNTERWASSER|L_LUFT|L_MOOR|L_SUMPF|L_EIS|L_DSCHUNGEL)

private string decay_short = "Eine verwesende Leiche";
private string player_name;
private string decay_long;
private int todeszeit;
private int lass_nix_mehr_rein;
private mixed *resurrection_info;
private string no_resurrection;
private static object *armours = ({});
private mapping murderer;

void create()
{
    set_id( ({"leiche"}) );
    set_name("leiche");
    set_smell("Sie müfft schon etwas angemodert.");
    set_gender("weiblich");
    set_material("biologisch");
    set_content_message("Sie trägt bei sich:");
    set_weight(30);
    seteuid(getuid());
    set_max_internal_encumbrance(1);
    set_take_prepos("von");
    set_put_prepos("zu");
    set_transparent(1);
    call_out("lass_nix_mehr_rein",2);
}

void lass_nix_mehr_rein() { lass_nix_mehr_rein = 1; }

<int|string> let_not_in(mapping mv_infos)
{
    object opfer = mv_infos[MOVE_OBJECT];
    if (lass_nix_mehr_rein &&
        // Nur von es von ausserhalb kommt...
        member(all_environment(opfer)||({}),this_object())<0)
    {
        return 
            Der() + plural(" braucht "," brauchen ") + den(opfer) +
            " jetzt bestimmt nicht mehr; "
            + der() + plural(" ist"," sind") + " nämlich tot.";
    }
    if (living(opfer))
        return 1;
    return ::let_not_in(mv_infos);
}

void decay()
{
    set_short(decay_short);
    if (decay_long) set_long (decay_long);
    delete_id(({"leiche"}));
    add_id(({"überreste", "reste", "überrest", "überresten"}));
    set_name("überreste");
    set_gender("maennlich");
    set_material("bein");
    set_plural(1);
    set_smell("Sie müffeln stark vermodert.");
    set_content_message("Dabei liegen:");
    call_out("skelettiere",60);
}

void set_decay_short(string str) { decay_short = str; }
void set_decay_long(string str) { decay_long = str; }
string query_decay_short() { return decay_short; }

/*
FUNKTION: set_time_of_death
DEKLARATION: void set_time_of_death()
BESCHREIBUNG:
Diese Funktion setzt die Todeszeit einer Leiche auf "jetzt".
Mit Hilfe dieser Funktion ist es moeglich, start_decay zu ueberlagern.
VERWEISE: start_decay
GRUPPEN: spieler, monster
*/


void set_time_of_death()
{
    todeszeit = time();
}

varargs void start_decay(object ob)
{
    if (playerp(ob))
	player_name = ob->query_real_name();
    todeszeit = time();
    set_weight(ob->query_weight());
    set_max_internal_encumbrance(ob->query_max_internal_encumbrance());
    set_short(Ihr(OBJ_TO,0,ob,0,0,ART_EIN));
    set_decay_short(Ihr((["gender":"maennlich", "plural":1,
	"name":"überreste"]),0,ob,0,0,ART_EIN));
    if (random (333) == 17)
        call_out("totengraeber",random(20)+2);
    else
	call_out("decay",DECAY_TIME);
}

void skelettiere()
{
    object *obs, skelett;
    int a;

    if (!environment() || !player_name || !todeszeit ||
        !environment()->query_room())
    {
	remove();
	return;
    }
    obs = all_inventory(environment());
    for (a=0; a<sizeof(obs); a++)
	if (obs[a]->is_skelett())
    {
	    skelett = obs[a];
	    break;
    }
    if (!skelett)
    {
	skelett = clone_object("/obj/skelett");
	skelett->move(environment(),([MOVE_FLAGS:MOVE_FORCE]));
    }
    skelett->add_skelett(player_name,todeszeit);
    remove();
}

void totengraeber()
{
    if (!environment() || !todeszeit)
    {
	remove();
	return;
    }

    if (!environment()->query_room() ||
        (environment()->query_type(LANDSCHAFT)&KEINE_TOTENGRAEBER_LANDSCHAFT) ||
	environment()->query_type("kein_totengraeber"))
    {
    	call_out("decay",DECAY_TIME);
        return;
    }

    /*
    object* obs = all_inventory(this_object());
    for (int a=0; a<sizeof(obs); a++)
    {
	if (obs[a])
	    obs[a]->remove();
        if (obs[a])
	    destruct(obs[a]);
    }
    */
    this_object()->send_message(MT_LOOK,MA_UNKNOWN,
    	wrap ("Ein Totengräber kommt des Weges, "
        "lädt "+den()+" auf sein Wägelchen und verschwindet wieder."));
    close_con(); // Inhalt wird dann zerstoert.
    remove();
}

void init()
{
    add_action("begrabe_command","begrabe",-6);
    add_action("begrabe_command","vergrabe",-7);
    add_action("begrabe_command","verbudd",AA_SHORT);
}

string query_player_name() { return player_name; }
int query_todeszeit() { return todeszeit; }

int begrabe_command(string str)
{
    mixed t;

    if (!me(str))
    {
        notify_fail("Was willst Du begraben?\n");
        return 0;
    }
    FAIL_GHOST("Wie willst Du das als Geist bewerkstelligen?");
    if (!environment() || t=environment()->query_type("graben_verboten"))
    {
	notify_fail(stringp (t) ? wrap(t) : "Das geht hier nicht.\n");
	return 0;
    }
    if (environment(environment()))
    {
        notify_fail (wrap (Der()+" sollte schon zu Deinen Füßen liegen, "
            "wenn Du "+ihn()+" begraben willst."));
        return 0;
    }       
    if (this_player()->free_hand() == -1)
    {
        write ("Du hast keine Hand mehr frei.\n");
        return 1;
    }
    write(wrap("Du begräbst "+den()+"."));
    say(wrap(Der(OBJ_TP)+" begräbt "+den()+"."));
    /*
    object *inv;
    if (inv = all_inventory(this_object()))
	for (int a=0; a<sizeof(inv); a++)
	{
	    if (inv[a])
		inv[a]->remove();
	    if (inv[a])
		destruct(inv[a]);
	}
    */
    close_con();
    remove();
    return 1;
}

// Braucht man, da Unsichtbare Objekte nicht mehr in der Leiche bleiben
// sollen, wenn die Leiche verwest ist.
int remove_invis_object(object ob) 
{
   if (ob && ob->query_invis())
       return ob->remove();
}

int remove() 
{
   object *inv;
   inv = all_inventory();
   filter(inv, #'remove_invis_object);
   destruct(this_object());
   return 1;
}

/*
FUNKTION: query_murderer
DEKLARATION: mapping query_murderer()
BESCHREIBUNG:
Liefert den Moerder (sofern bekannt) dieser Leiche aus.
VERWEISE: set_murderer, query_leiche
GRUPPEN: spieler, monster
*/
mapping query_murderer() {return murderer;}

/*
FUNKTION: set_murderer
DEKLARATION: void set_murderer(mapping murderer)
BESCHREIBUNG:
Damit verraet man den Moerder dieser Leiche.
VERWEISE: query_murderer, query_lieche
GRUPPEN: spieler, monster
*/
void set_murderer(mapping vitem) {murderer=vitem;}

/*
FUNKTION: query_resurrection_info
DEKLARATION: mixed *query_resurrection_info()
BESCHREIBUNG:
Liefert die fuer die Wiederbelebung einer Leiche noetigen Werte.
Wird beispielsweise von der Finsterlinge- und der Sehergilde verwendet.
VERWEISE: set_no_resurrection, query_no_resurrection
GRUPPEN: spieler, monster
*/
mixed *query_resurrection_info ()
{
    return resurrection_info;
}

void set_resurrection_info (mixed *x)
{
    resurrection_info = x;
}

void set_no_resurrection (string warum)
{
    no_resurrection = warum;
}

string query_no_resurrection ()
{
    return no_resurrection;
}
    
int query_leiche() { return 1; }

int add_armour(object armour)
{
   if(objectp(armour) &&
      environment(armour) == this_object() &&
      armour->query_armour())
   {
      armours -= ({ armour, 0 });
      armours += ({ armour });
      return 1;
   }
}

int delete_armour(object armour)
{
   armours -= ({ armour });
   return 1;
}

int armour_worn(object armour)
{
   return member(armours, armour) >= 0;
}

string query_info()
{
    string tmp="";
    mixed info = query_resurrection_info();
    if(info)
	tmp += sprintf("Opfer:      %-=67s\n"
		       "Stats:      Stärke: %d, Intelligenz: %d, Ausdauer: %d, Geschick: %d\n",
		       capitalize(info[1])+" ("+(info[0]?"Spieler":"NPC")+")",
		       info[3][STAT_STR],info[3][STAT_INT],
		       info[3][STAT_CON],info[3][STAT_DEX]);
    if(query_murderer())
	tmp += sprintf("Mörder:    %-=67s\n", Der(query_murderer()));
    return strlen(tmp) && tmp[0..<2];
}