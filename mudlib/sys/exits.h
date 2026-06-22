// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /sys/exits.h
// Description: All defines related to exits
// Author:      Myonara, 10.05.2026

#ifndef __SYS_EXITS_H
#define __SYS_EXITS_H

#ifdef TestMUD
#define NEW_EXIT_OBJECTS
#endif

#define EXIT  "/lwo/exit"
#define EXITS "/lwo/exits"




#define M_EXIT_COMMAND              "m:exit:command"
#define M_EXIT_COMMAND_ASCII        "m:exit:command:ascii"
#define M_EXIT_ROOM                 "m:exit:room"
#define M_EXIT_CATEGORY             "m:exit:category"
#define M_EXIT_FLAGS                "m:exit:flags"
#define M_EXIT_DIR_RAUS             "m:exit:dir:raus"
#define M_EXIT_DIR_REIN             "m:exit:dir:rein"
#define M_EXIT_LOCK_REASON          "m:exit:lock:reason"
#define M_EXIT_LOCK_REASON_OTHER    "m:exit:lock:reason:other"

#endif