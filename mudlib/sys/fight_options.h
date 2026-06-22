// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /sys/fight_options.h
// Description: Konstanten fuer fight_options und Kampfmeldungsfilter
// Author:      Myonara (11.feb.2015)

#ifndef __FIGHT_OPTIONS_H
#define __FIGHT_OPTIONS_H 1

#include "/sys/attack.h"

#define FIO_BROKEN_WEAPON           "fight:options:broken:weapon" // 0,1,2
#define FIO_PREVENT_ONLY_DEFENSIVE  "fight:options:prevent:only:defensive"
#define FIO_WIELD_WEAPON            "fight:options:wield:weapon"
#define FIO_TAKE_WEAPON             "fight:options:take:weapon"
#define FIO_WIELD_RETRY             "fight:options:wield:retry"
#define FIO_USE_FAR_WEAPON          "fight:options:use:far:weapon"
#define FIU_CL_ARROW_FACTORY        "fight:options:closure:arrow:factory"

#define FIM_WHO_SELF                "fight:messages:who:self"
#define FIM_WHO_ENEMY               "fight:messages:who:enemy"
#define FIM_WHO_OTHERS              "fight:messages:who:others"
#define FIM_MAX_COUNT               "fight:messages:max:count"
#define FIM_WEAPON                  "fight:messages:weapon"
#define FIM_ARMOUR                  "fight:messages:armour"
#define FIM_BROKEN                  "fight:messages:broken"

#define FIM_FILTER_FIGHT_FIRST_MSG  0x0001
#define FIM_FILTER_FIGHT_CRITICAL   0x0002
#define FIM_FILTER_WEAPON_BROKEN    0x0004
#define FIM_FILTER_ARMOUR_BROKEN    0x0008
#define FIM_FILTER_FIGHT_LAST_MSG   0x0010
#define FIM_FILTER_CATEGORY_0NULL     0x0100
#define FIM_FILTER_CATEGORY_1WEAK     0x0200
#define FIM_FILTER_CATEGORY_2MEDIUM   0x0400
#define FIM_FILTER_CATEGORY_3STRONG   0x0800
#define FIM_FILTER_CATEGORY_4HIGH     0x1000
#define FIM_FILTER_CATEGORY_5FRACTAL  0x2000
#define FIM_FILTER_ALL_CATEGORIES     0x3F00


#endif // __FIGHT_OPTIONS_H
