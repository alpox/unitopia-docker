// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/i/domain/map_room.c
// Description:	Inherit-File fuer Map-Raeume, die gecloned werden
// Author:	Freaky, Garthan (04.04.96)

#pragma save_types

virtual inherit "/i/room";

nomask void create()
{
    ::create();
}

/*
FUNKTION: map_create
DEKLARATION: void map_create()
BESCHREIBUNG:
Diese Funktion wird anstelle von create() in geclonten Map-Raeumen aufgerufen.
Beim Ueberlagern muss sie zwingend mit ::map_create() aufgerufen werden!
GRUPPEN: map, raum
VERWEISE: query_room_file, query_map_pos
*/
void map_create()
{
}
