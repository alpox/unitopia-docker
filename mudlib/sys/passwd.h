// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/sys/passwd.h
// Description:	Defines fuer /i/tools/passwd.c
// Author:	Garthan	(20.04.95)

#ifndef PASSWD_H
#define PASSWD_H 1

#include <tls.h>

#define PASSWD_OK	0x000
#define PASSWD_SHORT	0x001
#define PASSWD_NAME	0x002
#define PASSWD_SPACE	0x004
#define PASSWD_NONE	0x008
#define PASSWD_LONG	0x010
#define PASSWD_EASY	0x020
#define PASSWD_ALL	(~0)

#define PASSWD_WARNINGS PASSWD_LONG
#define PASSWD_ERRORS (PASSWD_SHORT|PASSWD_SPACE|PASSWD_NONE|\
		       PASSWD_NAME|PASSWD_EASY)

#define UNSECURE_PASSWDS \
({ "abcd", "qwert", "hallo", "hello", "passwor", "1234", "zxcv", "yxcv", \
   "asdf", "guest", "gast", "super", "master", "death", "killer", \
   "susanne", "detle", "gotcha", "games" })

// Makros zur Passwortueberpruefung:
// (str = unverschluesselte Eingabe, pass = verschluesseltes Passwort)
#define PASSWD_CONVERT(str, pass) ((strlen(pass) <= 32 && extended_crypt(str)) || pass)
#define PASSWD_CRYPT(str)         extended_crypt(str)
#define PASSWD_CHECK(str, pass)   (str && ((extended_crypt(str, pass) == pass) || \
                                  (hash(TLS_HASH_MD5, str) == pass) || hash(TLS_HASH_MD5, str[..7]) == pass))

#endif // PASSWD_H
