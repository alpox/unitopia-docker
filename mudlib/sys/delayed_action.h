// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/sys/delayed_action.h
// Description:	Defines fuer do_delayed_action()
// Author:	Freaky (05.06.96)

#ifndef DELAYED_ACTION_H
#define DELAYED_ACTION_H 1

// Returnwert von do_delayed_action()
#define DA_OK		1
#define DA_BUSY		2

// Returnwert fuer den Action-Handler
#define DA_COMMAND_ALLOWED	1

// Returnwert der Aktion
#define DA_CONTINUE	0
#define DA_STOP		1

// Flag fuer die callback-Closure
#define DA_DONE		1
#define DA_STOPPED	2
#define DA_BREAK	3
#define DA_NEW_ACTION	4

// Flag fuer do_delayed_action()
#define DA_OK_ACTION	1
#define DA_OK_MOVE	2
#define DA_OK_COMMAND	4
#define DA_OK_FIGHT	8
#define DA_VALID_OBJECT 16
#define DA_NO_HALT	32

#define DA_NO_ACTION   -1

#define DA_STOP_ACTION	"halt"

#endif // DELAYED_ACTION_H
