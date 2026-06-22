// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/i/item.c
// Description: Basis ALLER Objekte.

/*
 * Basis ALLER Objekte.
 */
#pragma save_types

inherit "/i/item/conservation";
inherit "/i/item/material";
inherit "/i/item/smell";
inherit "/i/item/noise";
inherit "/i/item/feel";
inherit "/i/item/adjektiv";
inherit "/i/item/description";
inherit "/i/item/name";
inherit "/i/item/gender";
inherit "/i/item/id";
inherit "/i/item/read";
inherit "/i/item/v_item";
inherit "/i/item/messages";
inherit "/i/item/control";
inherit "/i/item/message";
inherit "/i/item/licht";
inherit "/i/item/sounds";

#ifndef TestMUD
virtual
#endif
inherit "/i/item/properties";

void create() {}
void init() {}
