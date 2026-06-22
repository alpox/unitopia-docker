// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /i/tools/room_types.c
// Description: Hilfsfunktionen beim abfragen von Raumtypen
// Created by : Parsec  24.11.98

#pragma strong_types
#pragma save_types

#include <apps.h>
#include <landschaft.h>


/*
FUNKTION: query_innenraum
DEKLARATION: varargs int query_innenraum(object raum)
BESCHREIBUNG:
Gibt an ob es sich bei einem Raum um einen Innenraum handelt
(unter Beruecksichtigung vom gesetzten Landschaftstyp und, falls dieser
 nicht gesetzt ist, Kunstlicht).
Default fuer  raum  ist this_object().
VERWEISE: query_im_wasser
GRUPPEN: raum
*/

varargs int query_innenraum(object raum)
{
    int landschaft;

    if (!raum)
        raum = this_object();
    
    return
        raum &&
        (((landschaft = raum->query_type (LANDSCHAFT)) & L_DRINNEN) ||
          !landschaft && raum->query_type ("kunstlicht"));
}


/*
FUNKTION: query_im_wasser
DEKLARATION: varargs int query_im_wasser(object raum)
BESCHREIBUNG:
Gibt an, ob jemand, der sich in dem Raum aufhaelt, im Wasser steht oder schwimmt
(unter Beruecksichtigung von Stegen und Haefen).
Default fuer  raum  ist this_object().
VERWEISE: query_innenraum, query_hat_boden
GRUPPEN: raum
*/
varargs int query_im_wasser(object raum)
{
    int landschaft;

    if (!raum)
        raum = this_object();
    
    return
        raum &&
        ((landschaft = raum->query_type(LANDSCHAFT)) &
          (L_WASSER | L_FLIESSEND | L_FLACH | L_UNTERWASSER)) &&
        !(landschaft & (L_STEG|L_STRAND)) &&
        !raum->query_type("hafen");
}


/*
FUNKTION: query_hat_boden
DEKLARATION: varargs int query_hat_boden(object raum)
BESCHREIBUNG:
Gibt an, ob jemand, der sich in dem Raum aufhaelt,
Boden unter seinen Fuessen hat.
Default fuer  raum  ist this_object().
VERWEISE: query_innenraum, query_im_wasser
GRUPPEN: raum
*/
varargs int query_hat_boden(object raum)
{
    int landschaft;

    if (!raum)
        raum = this_object();
    
    return
        raum &&
        (!((landschaft = raum->query_type(LANDSCHAFT)) &
          (L_WASSER | L_FLIESSEND | L_FLACH | L_UNTERWASSER)) ||
          (landschaft & (L_STEG|L_STRAND|L_MEERESGRUND)) ||
          raum->query_type("hafen")) &&
        (!(landschaft & L_LUFT));
}


/*
FUNKTION: get_environment
DEKLARATION: object get_environment(object ob)
BESCHREIBUNG:
Liefert den logisch aeussersten Raum von ob.
Dabei werden Schiffe und andere Objekte mit den Typen "repraesentant" oder
"umgebung" beachtet.
GRUPPEN: raum
*/
object get_environment(object ob) {
  object env=ob;
  object new_env;

  while (env) {
    if (new_env = environment(env))
      env = new_env;
    else if (new_env = touch(env->query_ship()))
      env = environment(new_env);
    else if(objectp(new_env = env->query_type("repraesentant")))
      env = environment(new_env);
    else if(objectp(new_env = env->query_type("umgebung")))
      env = new_env;
    else
      break;
  }
  return env;
}

/*
FUNKTION: get_displayed_domain
DEKLARATION: string get_displayed_domain(object ob)
BESCHREIBUNG:
Liefert den Namen der Domain, in der sich ob aufhaelt,
der auch Spielern angezeigt werden kann.
GRUPPEN: raum
*/
string get_displayed_domain(object ob)
{
    object env = get_environment(ob);
    string dom;
    if (!env)
        return 0;

    dom = env->query_room_domain();
    if (member(({"Ozean","Pantheon","Himmel","Nirwana"}),dom)>=0)
        return dom;

    return DOMAIN_INFOS.query_entry(env, "Name");
}
