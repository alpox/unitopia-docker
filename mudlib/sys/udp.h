// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/sys/udp.h
// Description:	Defines fuer UDP
// Author:	

#ifndef _UDP_H_
#define _UDP_H_

// Hier muss direkt included werden, da udp.h vom Master included wird.
#include "/sys/config.h"

/* --- User Configuration. --- */

#define INETD		"/secure/udp/inetd"
#define UDP_CMD_DIR	"/secure/udp/"
#define HOST_FILE	"/static/adm/INETD_HOSTS"
#define HOST_SAVE_FILE	"/var/INETD_HOSTS"
#define INETD_LOG_FILE	"INETD"

#define REPLY_TIME_OUT	12
#define RETRIES		2

// 1 Woche Downtime -> Weg damit.
#define DOWN_TIME_FOR_REMOVAL 604800
#define PINGS_AT_ONCE		4

/* #define LOCAL_NAME	SECURITY->get_mud_name()	// CD */
#define LOCAL_NAME	MUD_NAME

/* #define LOCAL_UDP_PORT	SECURITY->do_debug("udp_port");  // CD */
#if __EFUN_DEFINED__(query_udp_port)
#define LOCAL_UDP_PORT  query_udp_port()
#else
#include "driver_info.h"
#define LOCAL_UDP_PORT  driver_info(DI_UDP_PORT)
#endif

#define INETD_DIAGNOSTICS

/* If you are running another intermud system concurrently and there is a
 * clash with the udp.h filename, rename the old udp.h file as appropriate
 * and include this line. */
/* #include <cdudp.h>	// CD */

/* Include these definitions for CD muds. */
/* #define CDLIB */
/* #define CD_UDP		"/d/Standard/obj/udp" */

/* --- End of config. --- */

#define INETD_VERSION	"0.61@UNItopia:1"

/* --- Standard header macros. --- */

#define RECIPIENT	"RCPNT"
#define REQUEST		"REQ"
#define SENDER		"SND"
/* The DATA field should be used to store the main body of any packet. */
#define DATA		"DATA"

/* These headers are reserved for system use only. */
#define HOST		"HST"
#define ID		"ID"
#define NAME		"NAME"
#define PACKET		"PKT"
#define UDP_PORT	"UDP"
#define SYSTEM		"SYS"
#define METHOD		"METHOD"

#ifdef INETD_DIAGNOSTICS
/* Reserved headers for diagnostics. */
#define PACKET_LOSS	"PKT_LOSS"
#define RESPONSE_TIME	"TIME"
#endif

/* --- Standard REQUEST macros. --- */

#define PING		"ping"
#define UDP_QUERY	"query"
#define REPLY		"reply"

/* --- Standard SYSTEM macros. --- */
#define TIME_OUT	"TO"

/* --- Index macros for host arrays. --- */

#define HL_HOST_NAME		0
#define HL_HOST_IP		1
#define HL_HOST_UDP_PORT	2
#define HL_LOCAL_COMMANDS	3
#define HL_HOST_COMMANDS	4
#define HL_HOST_STATUS		5
#define HL_HOST_ENCODING	6

#endif // _UDP_H_
