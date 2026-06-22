// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /sys/notifier.h
// Description: Defines zum Mail-Notifier
// Author:      Myonara (28.10.2017)

#ifndef __NOTIFIER__H
#define __NOTIFIER__H

#define NOTIFIER_MASTER     "/apps/notifier"
#define NOTIFIER_HELP(x) ("/doc/hilfe/goetter/zauberstab/zpost_"+(x))

#define NOTIFIER_ABO            "notifier:abonnement"
#define NOTIFIER_PLANNED_TIME   "notifier:planned:time"
#define NOTIFIER_EDITOR_CB      "notifier:editor:callback"
#define NOTIFIER_WIZNAME        "notifier:wizname"
#define NOTIFIER_MAIL_FORMAT    "notifier:mail:format"
#define NOTIFIER_CFG_D_HOUR     "notifier:config:daily:hour"
#define NOTIFIER_CFG_D_MIN      "notifier:config:daily:minute"
#define NOTIFIER_CFG_W_DAY      "notifier:config:weekly:day"
#define NOTIFIER_CFG_W_HOUR     "notifier:config:weekly:hour"
#define NOTIFIER_CFG_W_MIN      "notifier:config:weekly:minute"
#define NOTIFIER_CFG_FREQUENCY  "notifier:config:frequency"
#define NOTIFIER_CFG_FLAGS      "notifier:config:flags"
#define NOTIFIER_ABO_TYPE       "notifier:abonnement"
#define NOTIFIER_FDB_LISTID     "notifier:fdb:list:id"
#define NOTIFIER_EVT_EVENTID    "notifier:event:eventid"
#define NOTIFIER_EVT_SUMMARY    "notifier:event:summary"
#define NOTIFIER_EVT_BODY       "notifier:event:body"
#define NOTIFIER_EVT_CREATED_ON "notifier:event:created:on"
#define NOTIFIER_EVT_OBSOLETE_ON "notifier:event:obsolete:on"
#define NOTIFIER_EVT_NOTIFIED_ON "notifier:event:notified:on"
#define NOTIFIER_EVT_SENT_ON    "notifier:event:sent:on"

#define NOTIFIER_FQ_NEVER       0x00
#define NOTIFIER_FQ_AT_ONCE     0x01
#define NOTIFIER_FQ_DAILY       0x02
#define NOTIFIER_FQ_WEEKLY      0x04
#define NOTIFIER_FQ_MONTHLY     0x08
#define NOTIFIER_FQ_ALL         0x0F

#define NOTIFIER_WF_ONLINE_N    0x01 // Wenn online, dann benachrichtigen
#define NOTIFIER_WF_ONLINE_M    0x02 // Wenn online, trotzdem Mail.

#endif // __W_MOVE_STATISTICS__H