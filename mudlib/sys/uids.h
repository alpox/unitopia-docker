// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/sys/uids.h
// Description: User-IDs mit verschiedenen Zugriffsrechten

#ifndef UIDS_H
#define UIDS_H 1

// Root-UID darf ueberall lesen und schreiben
#define ROOT_UID	"Root"

// Apps-UID darf unter /var schreiben und lesen
#define APPS_UID	"Apps"

// Objekte, die diese UID bekommen, bekommen sofort die UID des
// clonenden Objektes
#define BACKBONE_UID	"Backbone"

// Die Map (Schreibrecht unter /map)
#define MAP_UID		"Map"

// Keine Schreibrechte
#define GENERAL_UID	"Magyra"

// Keine Schreibrechte
#define NOBODY_UID	(!strstr(getuid(),"n:") ? getuid() : "n:" + getuid())

// UID des Spielerobjektes
#define PLAYER_UID	"Player"

// UID der Spieler, die schon Gott sind, aber noch nicht schreiben duerfen
#define LEARNER_UID	"Learner"

#endif // UIDS_H
