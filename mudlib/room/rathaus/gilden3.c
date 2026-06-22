// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File: /room/rathaus/gilden3.c
// Description: Rathaus, Gildenraeume #4
//              Tmm      11.06.00 - Anpassung an neues Vaniorh
//              Selmarin 27.07.02 - Tuer nach Norden, Beschreibung

#ifdef UNItopia
#include "/d/Vaniorh/sys/vaniorh_exports.h"
#endif

inherit "/i/room";

#ifdef UNItopia
object door;

void reset()
{
   if(!door)
   {
      door=clone_object("/obj/tuer");
      door->set_pass_cmd("norden");
      door->set_door_height(-1);
      door->set_door_exit("/z/Gilden/Diebesgilde/d/Vaniorh/Tadmor/praesentation");
      door->set_long("Eine Tür und ein wüstes Gewirr "
                     "an polizeilichen Absperrbändern. An der Tür "
                     "klebt ein Siegel und irgendjemand hat einen "
                     "Zettel an das alte Holz genagelt.");
      door->set_no_lock(1);
      door->set_door_is_closed_msgs("Das geht nicht, da dir eine "
           "Tür und ein wüstes Gewirr an polizeilichen "
           "Absperrbändern im Weg sind. An der Tür klebt ein "
           "Siegel und irgendjemand hat einen Zettel an das "
           "alte Holz genagelt.");
      door->set_door_opens_message("Mit einem kühlen Lächeln setzt "
           "du dich über die Anordnung der Stadtwache hinweg, "
           "stemmst dich mit der Schulter gegen die Tür und schiebst "
           "sie mühsam auf.",
           "Mit einem kühlen Lächeln setzt "
           "sich $der(OBJ_TP) über die Anordnung der Stadtwache "
           "hinweg, stemmt sich mit der Schulter gegen die Tür und "
           "schiebt sie mühsam auf.");
      door->move(this_object());
   }
}
#endif

void create() {
    add_type("kunstlicht", 1);
    add_type("kaempfen_verboten",1);
    add_type("landeplatz","treppe");
    set_own_light(1);
    set_short("Gang der Gilden");
    set_long(
      "In den Räumen links und rechts von diesem Gang präsentieren "
      "sich sämtliche, über Magyra verteilte Gilden. "
      "Nördlich von hier befindet sich die Tür zum Raum der "
      "Diebesgilde. Im Süden riecht es dagegen deutlich nach "
      "Chemikalien.");
    set_exits( ({ "gilden2",
        "gilden4"
#ifdef UNItopia
        , ALCHEMISTEN_PRAESENTATION
#endif
        }),
          ({ "osten", "westen"
#ifdef UNItopia
          , "süden"
#endif
          }) );

#ifdef UNItopia
   add_v_item(([
      "name": "absperrbänder",
      "gender": "saechlich",
      "plural": 1,
      "id": ({"band", "bänder", "absperrband", "absperrbänder"}),
      "long": "Die Absperrbänder sind schwarz-gelb gestreift und "
              "tragen die Aufschrift:\n"
              "POLIZEILICHE ABSPERRUNG - BETRETEN VERBOTEN\n"
              "Sie kleben kreuz und quer vor der Tür. Allerdings "
              "hängen einige auch schon auf den Boden hinab."]));
   add_v_item(([
      "name": "siegel",
      "gender": "saechlich",
      "id": ({"siegel", "wachs"}),
      "long": "Das Siegel ist aus Wachs, trägt das Stadtwappen "
              "Tadmors und verklebt die Tür mit dem Rahmen. "
              "Allerdings sieht es aus, als habe es vor kurzem "
              "jemand zerbrochen."]));
   add_v_item(([
      "name": "bekanntmachung",
      "adjektiv":({"amtlich"}),
      "gender": "weiblich",
      "id": ({"zettel","bekanntmachung","papier","brief"}),
      "long": "Dieser Zettel sieht aus wie eine offizielle "
              "Bekanntmachung der Stadt Tadmor - inklusive "
              "Amtsstempel und Wasserzeichen. Vielleicht solltest "
              "du ihn ja mal lesen?",
      "read": "Tadmor 4. Oktober 91\n\n"
              "Wegen polizeilicher Ermittlungsarbeiten "
              "geschlossen.\n\n"
              "Dieser Raum wurde für den Publikumsverkehr gesperrt, "
              "da er offensichtlich als Stützpunkt für eine "
              "illegale Organisation diente. Deshalb wurde er durch "
              "offizielle Vertreter der Stadtwache auf Veranlassung "
              "des Stadtrates versiegelt. Die Ermittlungsarbeiten "
              "dauern an.\n\n"
              "Das Betreten des Tatortes und Mitnahme von "
              "Beweismitteln ist STRENGSTENS untersagt!\n\n"
              "gez. Hauptmann Bogus"]));
    reset();
#endif
}
