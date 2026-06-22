// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/sys/add_hp.h
// Description:	Konstanten fuer Modifierflag von add_hp
// Author:	Garthan (19.03.96)

// Doku siehe /i/living/hands::add_hp()

#ifndef ADD_HP_H
#define ADD_HP_H 1

#define AH_NO_ARMOUR		0x01
#define AH_ARMOUR		0x02
#define AH_NO_MESSAGE		0x04
#define AH_CRITICAL		0x08
#define AH_CRITICAL_MESSAGE	0x08	// Veraltet
#define AH_NO_AGGRESSION	0x10
#define AH_NO_GUARDIAN_ANGEL	0x20
#define AH_DIE			0x40
#define AH_DONT_DIE		0x80
#define AH_NO_SKILL		0x0100

// Flags fuer become_aggression_victim()
#define AG_SILENT		0x01

// Werte bei set/query_reattack
#define REATTACK_DONT			0
#define REATTACK_ONLY_SELF_DEFENSE	1
#define REATTACK_ALWAYS			2

// Mapping-Eintraege fuer add_hp
#define AH_ATTACKER		"attacker"
#define AH_WEAPON		"weapon"
#define AH_PROJECTILE		"projectile"
#define AH_CAUSE		"cause"
#define AH_ORIGINATOR		"originator"
#define AH_FLAGS		"flags"
#define	AH_MESSAGE		"message"
#define AH_ERF_TOD		"erf_tod"
#define AH_ERF_TOD_OTHER	"erf_tod_other"
#define AH_ERF_RETTUNG		"erf_rettung"
#define AH_HEAL_TYPE		"heal_type"
#define AH_DAMAGE_TYPE		"damage_type"
#define AH_COMBAT_LOG		"combat_log"
#define AH_MAPI			"mapi"

// Zusaetzlicher Eintrag beim modify_damage
#define AH_DAMAGE		"damage"
// Zusaetzlicher Eintrag beim modify_healing
#define AH_HEALING		"healing"
// Zusaetzlicher Eintrag beim modify_hit und do_hit
#define AH_VICTIM		"victim"
#define AH_HANDNR		"handnr"

// Werte fuer AH_HEAL_TYPE
#define AH_HEAL_NORMAL		0
#define AH_HEAL_MAGIC		1
#define AH_HEAL_MEDIC		2

// Indizes ins Meldungsarray
#define AH_MESG_VICTIM		0
#define AH_MESG_OTHER		1
#define AH_MESG_ATTACKER	2

#define AH_MESG_SIZE		3

#define AHD_STICH		"stich"
#define AHD_SCHNITT		"schnitt"
#define AHD_STUMPF		"stumpf"
#define AHD_HEILIG		"heilig"
#define AHD_DAEMONISCH		"daemonisch"
#define AHD_TOD			"tod"
#define AHD_LEBEN		"leben"
#define AHD_FEUER		"feuer"
#define AHD_WASSER		"wasser"
#define AHD_ERDE		"erde"
#define AHD_LUFT		"luft"
#define AHD_MAGIE		"magie"
#define AHD_ANSTRENGUNG		"anstrengung"
#define AHD_ERSTICKEN           "ersticken"
#define AHD_KAELTE		"kaelte"
#define AHD_WAERME		"waerme"
#define AHD_EXPLOSION		"explosion"
#define AHD_SAEURE		"saeure"
#define AHD_GIFT		"gift"
#define AHD_ELEKTRIZITAET	"elektrizitaet"
#define AHD_LAERM		"laerm"

/*
FUNKTION: AHD_LISTE
DEKLARATION: Liste der Schadenstypen
BESCHREIBUNG:

Der Eintrag AH_DAMAGE_TYPE bei add_hp gibt an, welcher Art
der angerichtete Schaden ist. Dabei werden die Typen in einem Array
angegeben. Daher ist eine Kombination mehrerer Typen moeglich.

  AHD_STICH		Stichverletzung: Dolchstich, Nadelstich, Pfeil
  AHD_SCHNITT		Schnittverletzung: Schwerthieb
  AHD_STUMPF		Verletzungen ohne Spitzen oder scharfen Kanten.
  AHD_HEILIG		Magischer Schaden einer guten Kraft
  AHD_DAEMONISCH	Magischer Schaden einer boesen Kraft
  AHD_TOD		Schaden wirkt besonders auf Lebendige
  AHD_LEBEN		Schaden wirkt besonders auf (Un)tote
  AHD_FEUER		Schaden durch Feuer
  AHD_WASSER		Schaden durch Wasser
  AHD_ERDE		Schaden durch Erde
  AHD_LUFT		Schaden durch Luft/Wind
  AHD_MAGIE		Magischer Schaden
  AHD_ANSTRENGUNG	Schaden durch besondere Anstrengung
  AHD_ERSTICKEN         Schaden durch mangelnde Sauerstoffzufuhr
  AHD_KAELTE		Schaden durch Kaelte
  AHD_WAERME		Schaden durch Waerme
  AHD_EXPLOSION		Schaden aufgrund einer Explosion
  AHD_SAEURE		Schaden durch Saeure
  AHD_GIFT		Schaden durch Gift (vergiftetes Wasser, Lebensmittel)
  AHD_ELEKTRIZITAET	Schaden von Elektrizitaet
  AHD_LAERM		Schaden durch Laerm

VERWEISE: add_hp
GRUPPEN: monster, spieler, kampf
*/
#endif // ADD_HP_H
