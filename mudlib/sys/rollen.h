// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/sys/rollen.h
// Description: Defines fuer /i/monster/rollen.c
// Author:      Francis

#ifndef ROLLEN_H
#define ROLLEN_H 1

/*
 * Fuer das Rollenspiel der Monster.
 *  Naeheres siehe /i/tools/rollen.c
 */

/*
 * Rollen-Flags des Monsters. 
 *
 * Return-Codes von query_rollen_modus()
 *
 */
#define R_AUS		0
#define R_SOUFFLEUR	1
#define R_PASSIV	2

/*
 * Zustand des Monsters.
 * (Returncodes der Routine query_zustand())
 */
#define R_STANDBY	0
#define R_RUNNING	1
#define R_STOPPED	2

#endif // ROLLEN_H
