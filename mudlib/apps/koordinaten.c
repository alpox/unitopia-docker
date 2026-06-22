// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/apps/koordinaten.c
// Description:	Koordinatenpuffer und KOORDINATEN-Datei-Verwalter
// Author:	Parsec@UNItopia (9/00)

// UID: Apps

#pragma strict_types

inherit "/i/tools/room_types";

#include <apps.h>
#include <config.h>
#include <error.h>


private mapping paths = ([]);


mapping query_paths() {
  return deep_copy(paths);
}

void clear_paths(string path) {
  if (path)
    m_delete(paths,path);
  else
    paths = ([]);
  return;
}

private int *get_koordinaten(string name) {
  string path, *unknown_path;
  int *koord;

  path = name[1..] ;
  unknown_path = ({});

  while (strstr(path = regreplace(path, "/[^/]*$", "", 1), "/") != -1) {
                           //  Hinten ein Stueck Pfad abschnippseln
    if (member(paths, path)) {
      // Den Pfad kenn ich also schon
      koord = paths[path];
      break;
    }

    // Den Pfad kenn ich noch nicht
    unknown_path += ({ path });

    if (file_size("/"+path+"/KOORDINATEN") > 0) {
      // Es gibt ne KOORDINATEN-Datei unter dem Pfad
      koord = ({0, 0,});
      if (sscanf(regreplace(read_file("/"+path+"/KOORDINATEN"), 
                            "[^-+0-9][^-+0-9]*", " ", 1),
                 "%d %d", koord[0], koord[1]) == 2)
        // ... und in der Datei steht was Vernuenftiges drinnen
        break;
      else
        // ... und in der Datei steht nix Vernuenftiges drinnen
        koord = 0;
    }
  }

  // Alle besuchten Pfade mit gefundenen Koordinaten markieren
  foreach(string p : unknown_path)
    paths[p] = koord;

  return copy(koord);
}


// das Peile-System aus der compass_logic
// war:  /z/Schiffe/Linienschiffe/i/compass_logic.c
// 1995-2001 Yellow, Milf, Parsec, Gnomi

/*
FUNKTION: get_cartesian_coordinates
DEKLARATION: varargs int *get_cartesian_coordinates(object ob, int supress_error)
BESCHREIBUNG:
Gibt die Differenz der Koordinates des Objekts ob zum Map-Raum m0_0 in 
kartesischen Koordinaten zurueck. 
ob kann dabei ein Objekt in einem Raum oder ein Raum selbst sein. 
Wird ob nicht angegeben, wird das aufrufende Objekt verwendet.

Aufruf: #include <apps.h>
        int *koordinaten=KOORDINATEN_MASTER->get_cartesian_coordinates(ob)

     koordinaten[0]:     x-Koordinate
     koordinaten[1]:     y-Koordinate

Sind in einem Raum die Typen
                     x_koordinate      | im Raum setzen mit:
             und     y_koordinate      |  add_type("x_koordinate",-142);
gesetzt, werden diese zurueckgegeben.

Befindet die das Object ob in keinem Map-Raum, so wird der Mittelpunkt
der Domain genommen, in der der Raum liegt.

Wenn keine Koordinaten ermittelt werden koennen, wird 0 zurueckgeliefert.
supress_error!=0 unterdrueckt in dem Fall das do_error...

VERWEISE: get_polar_coordinates
GRUPPEN: kompass, raum, map
*/
varargs int *get_cartesian_coordinates(object ob, int supress_error) {
  string  name, domain, *path;
  int *koord;
  object env;

  // Kein Objekt
  if (!ob && !(ob=previous_object()))
    raise_error("Kein Objekt übergeben und kein previous_object() "
                "vorhanden!\n");

  // Aeussersten Raum nehmen zum Koordinatenermitteln.
  env = get_environment(ob);
  if (!env) {
      if (!supress_error)
        do_error(wrap(sprintf("%O: ",ob)
            +"Es konnte kein Environment ermittelt werden."));
    return 0;
  }

  koord = ({({int})env->query_type("x_koordinate"),
            ({int})env->query_type("y_koordinate"),});
  if ((koord[0] || koord[1]) ||
      (koord = ({int*})env->query_map_pos()))
    return koord;

  name = object_name(env);
  if (koord = get_koordinaten(name))
    return koord;

  path = explode(name,"/");
  if (path[1] == "d")
    domain = path[2];
  else if (sizeof(path) > 4 && path[1] == "z" && path[4] == "d")
    domain = path[5];
  if (koord = ({int*})MAP_OB->query_domain_center(domain))
    return koord;
  if (({int})DOMAIN_INFOS->query_domain_entry(domain, "NotInMap"))
    return ({0, 0,});
  
  if (!supress_error)
    do_error(wrap(sprintf("%O: ",env)+"Das Objekt hat keine Koordinaten.\n"));
  return 0;
}

/*
FUNKTION: get_polar_coordinates
DEKLARATION: varargs int *get_polar_coordinates(object ob, int supress_error)
BESCHREIBUNG:
Gibt die Richtung und die Distanz vom Standort des Objekts ob
in der Map zum Map-Raum m0_0 zurueck.
ob kann dabei ein Objekt in einem Raum oder ein Raum selbst sein.
Wird ob nicht angegeben, wird das aufrufende Objekt verwendet.

Aufruf: #include <apps.h>
        int *koordinaten=KOORDINATEN_MASTER->get_polar_coordinates(ob)

     koordinaten[0]:     Richtung
     koordinaten[1]:     Distanz

Sind in einem Raum die Typen
		     x_koordinate      | im Raum setzen mit:
	     und     y_koordinate      |  add_type("x_koordinate",-142);
gesetzt, werden diese fuer die Berechnung verwendet.

Befindet die das Object ob in keinem Map-Raum, so wird der Mittelpunkt
der Domain genommen, in der der Raum liegt.

Wenn keine Koordinaten ermittelt werden koennen, wird 0 zurueckgeliefert.
supress_error!=0 unterdrueckt in dem Fall das do_error...

VERWEISE: get_cartesian_coordinates
GRUPPEN: kompass, raum, map
*/
varargs int *get_polar_coordinates(object ob, int supress_error) {
  int *pol;

  pol = get_cartesian_coordinates(ob,supress_error);
  if (!pol)
      return pol;

  pol = xy2pol(pol[0], pol[1]);
  pol[0] += 180;
  pol[0] %= 360;

  return pol;
}
