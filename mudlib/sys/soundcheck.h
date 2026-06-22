// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /sys/soundcheck.h
// Description: Defines fuer den SoundCheck==Tondateiverwaltung.

#ifndef __SYS_SOUNDCHECK__H
#define __SYS_SOUNDCHECK__H

#define SC_SOUNDCHECKER "/room/rathaus/obj/soundchecker"
#define SOUNDCHECK      "/apps/soundcheck"

#define SOUND_OPT_FILE      "sound:options:filename"
#define SOUND_OPT_STATUS    "sound:options:status"
#define SOUND_OPT_STATUS_BM "sound:options:status:bitmap"
#define SOUND_OPT_SOURCE    "sound:options:source"
#define SOUND_OPT_COPYRIGHT "sound:options:copyright"
#define SOUND_OPT_SUM       "sound:options:sum"
#define SOUND_OPT_WL_ID     "sound:options:wishlist:id"
#define SOUND_OPT_PR_ID     "sound:options:proposal:id"
#define SOUND_OPT_FROM      "sound:options:from"
#define SOUND_OPT_PRIORITY  "sound:options:priority"
#define SOUND_OPT_WISH      "sound:options:wish"
#define SOUND_OPT_DESCR     "sound:options:description"
#define SOUND_OPT_SELECTION "sound:options:selection"

#define SC_STATUS_DELETED          0x00001
#define SC_STATUS_ACTIVE           0x00002
#define SC_STATUS_SUM_BASIS        0x00003
#define SC_SOUNDFILE_TARGET        0x00010
#define SC_SOUNDFILE_PLAY          0x00020
#define SC_SOUNDFILE_PROP          0x00040
#define SC_SOUNDFILE_SUM           0x00070
#define SC_WISHLIST_OPEN           0x01000
#define SC_WISHLIST_FULFILLED      0x02000
#define SC_WISHLIST_SUM            0x03000
#define SC_PROPOSAL_OPEN           0x10000
#define SC_PROPOSAL_FULFILLED      0x20000
#define SC_PROPOSAL_SUM            0x30000

#define SC_LVL_ADMIN    50
#define SC_LVL_WIZP     25
#define SC_LVL_HLP      10
#define SC_LVL_GUEST    1
#define SC_LVL_NEWBIE   2
#define SC_LVL_PLAYER   5

#endif
