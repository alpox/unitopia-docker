// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /i/object/key.c
// Description: Schluessellogik
// Author:      Garthan (10.02.94)

#pragma save_types

int id(string str);

int fit(object door)
{
   int i;
   string *keys;

   if(door && pointerp(keys = door->query_keys()))
      for(i = sizeof(keys); i--;)
	  if(id(keys[i]))
             return 1;
}

/*
FUNKTION: query_key
DEKLARATION: int query_key()
BESCHREIBUNG:
Liefert bei Schluesseln einen Wert != 0.
VERWEISE: query_keys
GRUPPEN: taschen
*/
int query_key()
{
    return 1;
}
