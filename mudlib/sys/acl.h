// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/sys/acl.h
// Description:	Defines fuer die ACLs
// Author:	Sissi (02.10.98)

#ifndef ACL_H
#define ACL_H 1

#define ACL_ADMIN	0x001
#define ACL_WRITE	0x002
#define ACL_CREATE	0x004
#define ACL_REMOVE	0x008
#define ACL_MKDIR	0x010
#define ACL_RMDIR	0x020
#define ACL_SAVE	0x040
#define ACL_RESTORE	0x080
#define ACL_DEBUG       0x100
#define ACL_READ	0x200

/*
FUNKTION: MAY_WRITE
DEKLARATION: int MAY_WRITE(string filename, object wiz_or_master)
BESCHREIBUNG:
Liefert einen Wert != 0, wenn der angegebene Gott oder das angegebene
Objekt in die angegebene Datei schreiben darf.
VERWEISE: query_access
GRUPPEN: master, acl
*/
#define MAY_WRITE(fname, wiz)	(({string|int})__MASTER_OBJECT__->valid_write(fname, \
				    geteuid(wiz), "write_test",  wiz) && 1)
#endif // ACL_H
