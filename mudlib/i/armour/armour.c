// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/i/armour/armour.c
// Description: Eine normale Ruestung
// Author:	FFG
// Modified By: Garthan, Freaky (22.11.94) (kein Armourshadow mehr)
// Modified by: Freaky (06.11.95) query_value() lieferte Werte < 0
//              Garthan (27.02.96) kaputte Ruestungen->no_store && min_val/2
//              Sissi (06.03.96) Reparatur aus Schmieden in Ruestung verlegt
//		Zap (27.09 96) query_extended_short()
//              Kurdel (24.02.97) Ruestung erbt die Kleidung

#pragma save_types
#pragma strong_types

inherit "/i/clothes/kleidung";

#include <attack.h>
#include <config.h>
#include <fight_options.h>
#include <object_stats.h>
#include <add_hp.h>
#include <apps.h>
#include <deklin.h>
#include <parse_com.h>
#include <message.h>
#include <error.h>
#include <description.h>
#include <simul_efuns.h>

private int protection, life, max_life, armour_min_val, armour_max_val;
private static int unbreakable;
private string armour_class, broken_message, broken_adjektiv;

#define REPAIR_RATE 67
#define REPAIR_MAX 90

//////////////////////////
// armour identifier

/*
FUNKTION: query_armour
DEKLARATION: int query_armour()
BESCHREIBUNG:
Liefert bei Ruestungen 1. Dient dazu, Ruestungen zu erkennen.
VERWEISE: query_weapon, query_cloth, query_armour_level
GRUPPEN: ruestung
*/
int query_armour() { return 1; }

//////////////////////////
// Adjektiv geschussel

//Dokumentiert in /i/weapon/weapon_logic.c
void set_broken_adjektiv(string str)
{
    broken_adjektiv = str;
    this_object()->add_setter_conservation("set_broken_adjektiv",({str}));
}

string query_broken_adjektiv() { return broken_adjektiv; }

//Dokumentiert in /i/clothes/kleidung.c
int query_worn()
{ 
   return environment() && environment()->armour_worn(this_object());
}

//////////////////////////
// action primitives

//Dokumentiert in /i/clothes/kleidung.c
void do_wear()
{
    if(environment() && !query_worn())
    {
        environment()->add_armour(this_object());
        if (!playerp(environment()))
            unbreakable = 1;
        ::do_wear();
    }
}

//Dokumentiert in /i/clothes/kleidung.c
void do_remove()
{
    if (query_worn())
    {
        environment()->delete_armour(this_object());
        unbreakable = 0;
        ::do_remove();
    }
}



////////////////////////////////////////
// life, reparatur und broken geschussel

//Dokumentiert in /i/weapon/weapon_logic.c
void do_break()
{
    if(broken_adjektiv && !this_object()->adjektiv(broken_adjektiv))
        this_object()->add_adjektiv(broken_adjektiv,1);
    this_object()->notify("armour_fail", this_object(), "broken");
    if(environment())
       environment()->notify("armour_fail", this_object(), "broken");
}

/*
FUNKTION: notify_armour_fail
DEKLARATION: void notify_armour_fail(object armour, string how)
BESCHREIBUNG:
Wenn die Ruestung unbrauchbar wird, dann wird sowowhl in der Waffe selber als
auch im environment ob der Ruestung (also meistens im Lebewesen)
armour->notify("armour_fail", armour, how) bzw.
ob->notify("armour_fail", armour, how) aufgerufen.

Die Funktion notify ruft dann in allen mit ob->add_controller(
"notify_armour_fail", other) angemeldeten Objekten other die Funktion
other->notify_armour_fail(armour, how) auf. Sowohl ob als auch other haben
dann die Moeglichkeit darauf zu reagieren. Zum Beispiel koennte ein Gegner im
Kampf die Ruestung wuetend auf den Gegner werfen.

Folgende Werte werden zur Zeit fuer how genutzt:
 "broken": Die Ruestung wurde beschaedigt (siehe do_break und set_life).

VERWEISE: do_break
GRUPPEN: ruestung
*/

//Dokumentiert in /i/weapon/weapon_logic.c
int query_repairable()
{
    return max_life > 10;
}

//Dokumentiert in /i/weapon/weapon_logic.c
varargs void do_repair(int percent)
{
    if (!query_repairable())
        return;

    if (percent <= 0)
        percent = REPAIR_RATE;
    else if (percent > REPAIR_MAX)
        percent = REPAIR_MAX;

    max_life = max_life * percent / 100;
    life = max_life;
    if(broken_adjektiv)
        this_object()->delete_adjektiv(broken_adjektiv);
    armour_max_val = armour_max_val * percent / 100;
    armour_min_val = armour_min_val * percent / 100;
    if (armour_max_val < 1)
        armour_max_val = 1;
    if (armour_min_val < 1)
        armour_min_val = 1;
    this_object()->delete_seq_conservation("life");
    this_object()->add_setter_conservation("set_life",({max_life}),"life");
    this_object()->add_setter_conservation("set_value",
        ({armour_min_val,armour_max_val}));
}

//Dokumentiert in /i/weapon/weapon_logic.c
varargs int query_repair_cost(int percent)
{
    if (percent <= 0)
        percent = REPAIR_RATE;
    else if (percent > REPAIR_MAX)
        percent = REPAIR_MAX;

    int tmp = (armour_max_val - armour_min_val) * percent / 100;
    if (tmp <= 0)
        return 1;
    return 2 * tmp;
}

//Dokumentiert in /i/weapon/weapon_logic.c
string query_broken_message()
{
    return broken_message;
}
void set_broken_message(string str)
{
    broken_message = str;
    this_object()->add_setter_conservation("set_broken_message",({str}));
}

/*
FUNKTION: set_life
DEKLARATION: void set_life(int life)
BESCHREIBUNG:
Mit set_life kann man den Schaden festlegen, die eine Ruestung einsteckt,
bevor sie kaputt geht. Fuer erlaubte Werte
->/doc/richtlinien/ruestungen/leben
VERWEISE: add_life, query_life, query_max_life
GRUPPEN: ruestung, kampf
*/
void set_life(int i)
{
    if(life <= 0 && i > 0)
        do_repair();
    life = max_life = i <= 0 ? 0 : i;
    this_object()->delete_seq_conservation("life");
    this_object()->add_setter_conservation("set_life",({max_life}),"life");
}

/*
FUNKTION: add_life
DEKLARATION: int add_life(int life)
BESCHREIBUNG:
Mit add_life kann man den Schaden erhoehen oder verringern (bei negativem
life), die eine Ruestung einsteckt, bevor sie kaputt geht.
Fuer erlaubte Werte ->/doc/richtlinien/ruestungen/leben
VERWEISE: set_life, query_life, query_max_life
GRUPPEN: ruestung, kampf
*/
int add_life(int a)
{ 
   if (!this_object()) 
   {
       return 0; // kann 0 sein, warum auch immer.
   }
   if (a < 0 && unbreakable)
      return life;
   if (life <= 0 && life + a > 0)
      do_repair();
   if (life > 0 && life + a <= 0)
   {
      do_break();
      if(environment()&&this_object())
#ifdef FILTER_MSG_BY_ATTRIBUTES
        this_object()->send_message_to(environment(),MT_LOOK|MT_NOISE,
            MA_UNKNOWN,wrap(query_broken_message()),
            ([MSG_RECEIVER_WHOM:AH_ATTACKER,
              FIM_ARMOUR:this_object(),FIM_BROKEN: 1]) );
#else
        this_object()->send_message_to(environment(),MT_LOOK|MT_NOISE,
            MA_UNKNOWN,wrap(query_broken_message()) );
#endif
   }
   life += a; // TODO und wozu gibt es max_life?
   if (this_object())
   {
        this_object()->delete_seq_conservation("life");
        this_object()->add_setter_conservation("set_life",({max_life}),"life");
        this_object()->add_setter_conservation("add_life",({life-max_life}),"life");
   }
   return life;
    
}

/*
FUNKTION: query_life
DEKLARATION: int query_life()
BESCHREIBUNG:
Liefert den Schaden, die eine Ruestung einsteckt, bevor sie kaputt geht.
VERWEISE: set_life, query_max_life
GRUPPEN: ruestung, kampf
*/
int query_life()     { return life; }

/*
FUNKTION: query_max_life
DEKLARATION: int query_max_life()
BESCHREIBUNG:
Liefert den Schaden, den die Ruestung im neuen Zustand (sprich nach dem
create()) einstecken konnte.
VERWEISE: query_life, set_life
GRUPPEN: waffen, kampf
*/
int query_max_life() { return max_life; }

//Dokumentiert in /i/weapon/weapon_logic.c
int query_broken()   { return life <= 0; }

int query_enable_cleanup()
{
    if(this_object()->query_prevent_cleanup())
        return 0;
    if(this_object()->query_broken())
        return 1;
    return ::query_enable_cleanup();
}
			    
//////////////////////////
// armour class

/*
FUNKTION: set_armour_class
DEKLARATION: void set_armour_class(string armour_class)
BESCHREIBUNG:
Damit setzt man die Klasse der Ruestung. Von einer Klasse kann man immer nur
eine Ruestung anhaben. Erlaubte Klassen stehen in
/doc/richtlinien/ruestungen/klassen
VERWEISE: query_armour_class, armour_class
GRUPPEN: ruestung
*/
void set_armour_class(string str)
{
   armour_class = !str || str == "" ? "oberkoerper" : lower_case(convert_umlaute(str));
   // An die Fuesse und auf den Kopf passt jeweils nur eine Kleidung/Ruestung.
   // Ruestung fuer den Kopf wird auch wie eine Muetze "aufgesetzt".
   if (armour_class == "fuesse")
       set_typ("schuhe"); // Kleidungstyp
   if (armour_class == "kopf") 
   {
       set_typ("muetze");
       set_worn_adjektiv("aufgesetzt");
   }
   this_object()->add_setter_conservation("set_armour_class",({armour_class}));
}

/*
FUNKTION: query_armour_class
DEKLARATION: string query_armour_class()
BESCHREIBUNG:
Liefert die Klasse der Ruestung. Von einer Klasse kann man immer nur
eine Ruestung anhaben. Erlaubte Klassen stehen in
/doc/richtlinien/ruestungen/klassen
VERWEISE: set_armour_class, armour_class
GRUPPEN: ruestung
*/
string query_armour_class() { return armour_class; }

/*
FUNKTION: armour_class
DEKLARATION: int armour_class(string str)
BESCHREIBUNG:
Liefert einen Wert !=0, wenn die uebergebene Klasse mit der Klasse der
Ruestung uebereinstimmt.
VERWEISE: set_armour_class, query_armour_class
GRUPPEN: ruestung
*/
int armour_class(string str) { return str && (lower_case(str)==armour_class); }

//////////////////////////
// armour protection

/*
FUNKTION: set_armour_protection
DEKLARATION: void set_armour_protection(int protection)
BESCHREIBUNG:
Damit wird die Schutzkraft der Ruestung gesetzt. Eine Ruestung haelt bis zu
protection APs pro Schlag ab. Erlaubte Werte fuer protection stehen in
/doc/richtlinien/ruestungen/schutzkraft
VERWEISE: query_armour_protection
GRUPPEN: ruestung
*/
void set_armour_protection(int prot)
{
    protection = prot <= 0 ? 0 : prot;
    this_object()->add_setter_conservation("set_armour_protection",
        ({protection}));
}

/*
FUNKTION: query_armour_protection
DEKLARATION: int query_armour_protection()
BESCHREIBUNG:
Liefert die Schutzkraft der Ruestung. Erlaubte Werte fuer die Schutzkraft
stehen in /doc/richtlinien/ruestungen/schutzkraft
VERWEISE: set_armour_protection
GRUPPEN: ruestung
*/
int query_armour_protection() { return protection; }


/*
FUNKTION: query_extra_armour_protection
DEKLARATION: int query_extra_armour_protection(int ahps, mapping info)
BESCHREIBUNG:
Diese Funktion wird in der Ruestung aufgerufen, wenn die Schutzwirkung
gegen einen bestimmten Schlag berechnet wird. Der Wert wird zur
normalen Schutzkraft dazuaddiert. Negative Werte sind moeglich.
Die beiden Parameter entsprechen denen von add_hp.

Die Summe aus dieser und der normalen Schutzwirkung darf die Werte
nach /doc/richtlinien/ruestungen/schutzkraft nicht ueberschreiten.
VERWEISE: query_armour_protection, add_hp
GRUPPEN: ruestung
*/
int hit_action(object attacker, int ahps, mapping info)
{
    if(!info)
	info = ([ AH_ATTACKER: attacker ]);

    return random(this_object()->query_armour_protection() +
	    this_object()->query_extra_armour_protection(ahps, info) + 1);
}

//////////////////////////
// armour value

varargs void set_value(int min, int max)
{
   armour_min_val = min < 1 ? 1 : min;
   if(max < 1)
   {
      armour_max_val = armour_min_val;
      armour_min_val /= 3 ;
   } 
   else
      armour_max_val = max;
   this_object()->add_setter_conservation("set_value",
        ({armour_min_val,armour_max_val}));
}

int query_value()
{
   if(max_life && (life > 0))    /* Division durch Null !! */
      return armour_min_val + life*(armour_max_val-armour_min_val) / max_life;
   else
      return armour_min_val / 2;
}

void just_sold()
{
   if(query_broken())
      this_object()->remove();
   else
      ::just_sold();
}


/*
FUNKTION: query_min_value
DEKLARATION: int query_min_value()
BESCHREIBUNG:
Liefert den Mindestwert der intakten Ruestung (also den Wert, den die Ruestung
besitzt, wenn sie noch einen Schlag aushaelt).
VERWEISE: query_max_value, query_value, set_life
GRUPPEN: ruestung
*/
int query_min_value() { return armour_min_val; }
/*
FUNKTION: query_max_value
DEKLARATION: int query_max_value()
BESCHREIBUNG:
Liefert den Maximalwert der intakten Ruestung (also den Wert der Ruestung
im nagelneuen Zustand).
VERWEISE: query_min_value, query_value, set_life
GRUPPEN: ruestung
*/
int query_max_value() { return armour_max_val; }

//////////////////////////
// comfort functions

/*
FUNKTION: init_armour_data
DEKLARATION: void init_armour_data(int protection, string class, int life, string broken)
BESCHREIBUNG:
Damit wird die Schutzkraft protection, die Klasse class, Lebensdauer life
und die Kaputt-Geh-Meldung broken der Ruestung gesetzt.
Erlaubte Werte fuer protection, class und life stehen in
/doc/richtlinien/ruestungen/schutzkraft, 
/doc/richtlinien/ruestungen/klassen und
/doc/richtlinien/ruestungen/leben.
VERWEISE: set_armour_protection, set_armour_class, set_life, set_broken_message
GRUPPEN: ruestung
*/
void init_armour_data(int prot, string class, int l, string broken)
{
   set_armour_protection(prot);
   set_armour_class(class);
   set_life(l);
   set_broken_message(broken);
}

//////////////////////////
// user interface

void create()
{
   ::create();
   seteuid(getuid());
   set_id("rüstung");
   set_class_id("rüstung");
   set_plural_id("rüstungen");
   set_name("rüstung");
   set_gender("weiblich");
   set_long("Du siehst nichts Besonderes.\n");
   set_value(5, 10);
   set_weight(1);
   set_schutz(2);
   init_armour_data(1, "oberkoerper", 1, "Ritsch!!\n");
   set_broken_adjektiv("beschädigt");
   if (program_name(this_object())==__FILE__) // fuer replace_program-Objekte
        clear_initial_conservation_data();
}

//////////////////////////
// ueberlagerte Funktionen

string query_short(object betrachter)
{
   string short;

   short = ::query_short(betrachter);
   if (!short)
      return short;
   if (query_broken())
      short += additional_string(broken_adjektiv,"beschädigt");
   return short;
}

protected string query_long_postprocess(string msg, mapping info)
{
    msg = ::query_long_postprocess(msg, info);
    
    if(query_broken() && !query_long_has_tag(T_ATOM_TAG_BROKEN_TEXT))
        msg += wrap(desc_text(T_ATOM_BROKEN_TEXT, info, ({})));

    return msg;
}
    
protected mixed desc_condition(string name, mixed info, mixed* par)
{
    object ob = info[TI_OBJECT] || (objectp(info[TI_ITEM]) && info[TI_ITEM]) || this_object();

    switch(name)
    {
        case T_ATOM_BROKEN:
            return ob && ob->query_broken();
    }
    
    return ::desc_condition(name, info, par);
}

protected mixed desc_text(string name, mixed info, mixed* par)
{
    object ob = info[TI_OBJECT] || (objectp(info[TI_ITEM]) && info[TI_ITEM]) || this_object();

    switch(name)
    {
        case T_ATOM_BROKEN_TEXT:
            if(ob)
            {
                mixed adj = ob->query_broken_adjektiv() || "beschädigt";
                return Der(ob,"") + ist(ob,IST_SPACE_BEFORE|IST_SPACE_AFTER) +
                    (pointerp(adj)?adj[0]:adj) + ".";
            }
    }
    
    return ::desc_text(name, info, par);
}

protected int desc_number(string name, mixed info, mixed* par)
{
    switch(name)
    {
        case TN_LIFE:
            return query_life();
    }
    
    return ::desc_number(name, info, par);
}

varargs mixed already_worn(int all)
{
    return this_player()->armour_class(armour_class) || ::already_worn(all);
}

#ifdef LOG_OBJECT_STATS
void log_object_stats()
{
   OBJECT_STATS->add_object_stats(OS_ARMOUR, this_object(),
      ({
         query_name(),
         query_weight(),
         query_min_value(),
         query_max_value(),
         query_armour_protection(),
         query_life(),
         query_armour_class(),
      }));
}
#endif

/*
FUNKTION: init_armour
DEKLARATION: varargs void init_armour(string kategorie[, int schutz_in_prozent[, int leben_in_prozent[, int gewicht_in_prozent]]])
BESCHREIBUNG:
Setzt der Kategorie entsprechend Richtlinien-konform folgende Werte:

    Ruestungsklasse     set_armour_class()
    Schutz              set_armour_protection()
    Leben               set_life()
    Gewicht             set_weight()
    Wert                set_value()

Parameter:

    Schutz  in %: -100..100, 100=maximaler Schutz,
                               0=minimaler Schutz.
                             Negative Werte liefern denselben Schutz wie
                             positive, es werden aber zusaetzlich die
                             Richtlinien fuer verkaufbare Ruestungen
                             eingehalten (Schutz maximal 3).
    Leben   in %:    0..100, 100=maximales Leben fuer den gesetzten Schutz,
                             lineare Skalierung,
                             niedrigeres Leben bedeutet niedrigeren Wert.
    Gewicht in %:    0..200, 200=maximales Gewicht,
                             100=Standard-Gewicht (=Schutz),
                               0=minimales Gewicht,
                             niedrigeres Gewicht bedeutet hoeheren Wert.

    Minimal und maximal beziehen sich auf die Richtlinien.

    Ist ein Parameter nicht angegeben, so wird dafuer 100 genommen.

Schutz in Abhaengigkeit von Kategorie und Schutz in %:
(Dies sind die aktuellen Richtlinien.)

    Kategorie    Schutz
                 0%  100%
    ---------------------
    beine        1     2
    fuesse       1     2
    haende       1     2
    kopf         1     2
    magie        1     2
    oberkoerper  1     5

BEISPIEL:

    // Ein 80%-Kettenhemd,
    // das nur 60% der normalen Lebensdauer und
    // dafuer auch nur 60% des normalen Wertes hat.

    inherit "/i/armour/armour";
    
    void create()
    {
        "*"::create();
        init_armour("oberkoerper", 80, 60);
        set_name("kettenhemd");
        set_gender("saechlich");
        set_id(({"kettenhemd", "hemd"}));
    }

VERWEISE: set_armour_class, set_armour_protection, set_life, set_weight, set_value
GRUPPEN: ruestung
*/

private float prozent_to_float(mixed prozent, mixed min, mixed max)
{
    return min+(to_float(max)-to_float(min))*to_float(prozent)/100.0;
}

private int prozent_to_int(mixed prozent, mixed min, mixed max)
{
    return round(prozent_to_float(prozent, min, max));
}

private int schutz_to_leben(int schutz)
{
    return ({1500, 1000, 750, 600, 500})[schutz-1];
}

private int schutz_to_gewicht_min(int schutz)
{
    return ({1, 2, 2, 3, 4})[schutz-1];
}

private int schutz_to_gewicht_standard(int schutz)
{
    return ({1, 2, 3, 4, 5})[schutz-1];
}

private int schutz_to_gewicht_max(int schutz)
{
    return ({2, 3, 3, 4, 6})[schutz-1];
}

private int schutz_to_wert_min(int schutz)
{
    return ({100, 250, 650, 1300, 3500})[schutz-1];
}

private int schutz_to_wert_standard(int schutz)
{
    return ({100, 250, 500, 1000, 2200})[schutz-1];
}

private int schutz_to_wert_max(int schutz)
{
    return ({50, 150, 500, 1000, 1500})[schutz-1];
}

//varargs void init_armour(
//    string kategorie,
//    int    schutz_in_prozent,
//    int    leben_in_prozent,
//    int    gewicht_in_prozent
//    )
varargs void init_armour(string kategorie, varargs int *in_prozent)
{
    int schutz_in_prozent =(sizeof(in_prozent)>=1 ? in_prozent[0] : 100);
    int leben_in_prozent  =(sizeof(in_prozent)>=2 ? in_prozent[1] : 100);
    int gewicht_in_prozent=(sizeof(in_prozent)>=3 ? in_prozent[2] : 100);
    int schutz, leben, gewicht;
    float wert; // Wegen Rundungsfehlern als Fliesskommazahl berechnet.
    int flag_verkaufsware;

    // Kategorie ueberpruefen:
    if(kategorie!=lower_case(kategorie))
    {
        do_error(wrap(
            "Rüstungskategorie \""+kategorie+"\" bitte klein schreiben!"
            ));
        return;
    }
    kategorie = convert_umlaute(kategorie);
    if(member(({
        "kopf",
        "oberkoerper",
        "haende",
        "beine",
        "fuesse",
        "magie",
        }), kategorie)==-1)
    {
        do_error(wrap("Unbekannte Rüstungskategorie \""+kategorie+"\"!"));
        return;
    }

    // schutz_in_prozent ueberpruefen:
    if(schutz_in_prozent>100 || schutz_in_prozent<-100)
    {
        do_error(wrap(
            "Angegebener Schaden "+schutz_in_prozent+" in % "
            "ist nicht zulässig.\n"
            "Zulässige Werte: -100..100."
            ));
        return;
    }
    if(schutz_in_prozent<0)
    {
        schutz_in_prozent=-schutz_in_prozent;
        flag_verkaufsware=1;
    }

    // leben_in_prozent ueberpruefen:
    if(leben_in_prozent>100 || leben_in_prozent<0)
    {
        do_error(wrap(
            "Angegebenes Leben "+leben_in_prozent+" in % "
            "ist nicht zulässig.\n"
            "Zulässige Werte: 0..100."
            ));
        return;
    }

    // gewicht_in_prozent ueberpruefen:
    if(gewicht_in_prozent>200 || gewicht_in_prozent<0)
    {
        do_error(wrap(
            "Angegebenes Gewicht "+gewicht_in_prozent+" in % "
            "ist nicht zulässig.\n"
            "Zulässige Werte: 0..200."
            ));
        return;
    }

    // Ruestungsklasse:
    set_armour_class(kategorie);

    // Schaden:
    schutz=prozent_to_int(
        schutz_in_prozent,
        1,
        (kategorie=="oberkoerper" ? 5 : 2)
        );
    if(schutz>3)
    {
        if(flag_verkaufsware)
            schutz=3;
        else
            set_no_store(1);
    }
    set_armour_protection(schutz);

    // Leben:
    leben=prozent_to_int(
        leben_in_prozent,
        1,
        schutz_to_leben(schutz)
        );
    set_life(leben);
    
    // Gewicht:
    if(gewicht_in_prozent<=100)
    {
        gewicht=prozent_to_int(
            gewicht_in_prozent,
            schutz_to_gewicht_min(schutz),
            schutz_to_gewicht_standard(schutz)
            );
    }
    else
    {
        gewicht=prozent_to_int(
            gewicht_in_prozent-100,
            schutz_to_gewicht_standard(schutz),
            schutz_to_gewicht_max(schutz)
            );
    }
    set_weight(gewicht);

    // Wert aus Schutz und Gewicht berechnen:
    if(gewicht<schutz_to_gewicht_standard(schutz))
    {
        wert=prozent_to_float(
            100.0*(gewicht-schutz_to_gewicht_min(schutz))/
            (schutz_to_gewicht_standard(schutz)-schutz_to_gewicht_min(schutz)),
            schutz_to_wert_min(schutz),
            schutz_to_wert_standard(schutz)
            );
    }
    else
    // Sonderfall, da es sonst zur Division durch 0 kommen kann:
    if(gewicht==schutz_to_gewicht_standard(schutz))
    {
        wert=to_float(schutz_to_wert_standard(schutz));
    }
    else
    {
        wert=prozent_to_float(
            100.0*(gewicht-schutz_to_gewicht_standard(schutz))/
            (schutz_to_gewicht_max(schutz)-schutz_to_gewicht_standard(schutz)),
            schutz_to_wert_standard(schutz),
            schutz_to_wert_max(schutz)
            );
    }

    // Wert wird linear durch das Leben korrigiert:
    wert=wert*leben/schutz_to_leben(schutz);

    set_value(round(wert));
}
