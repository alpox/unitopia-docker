// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/apps/news_ownerd.c
// Description:	Objekt zum Speichern der Ownerdaten des News-Systems
// Author:	Freaky '95
// Modified by:	Freaky (05.12.95) nach /apps geschoben.

// UID: Apps

#include <news.h>

#define OWNERS "/var/news_owners"

mapping owners=([]);
mapping masters=([]);

void create() {
}

void save_owners(mapping own, mapping m) {
    if (previous_object() && object_name(previous_object())==NEWSD) {
	owners=own;
	if(m) masters=m;
	save_object(OWNERS);
	}
    destruct(this_object());
}

mapping *query_owners() {
    if (previous_object() && object_name(previous_object())==NEWSD) {
	restore_object(OWNERS);
	destruct(this_object());
	return ({owners, masters});
	}
    destruct(this_object());
}

