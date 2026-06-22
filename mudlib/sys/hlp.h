// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/sys/hlp.h
// Description:	Makro zum Test auf bestimmte Gabe eines HLP
// Author:	Garthan (vor 09.08.1996)

#ifndef HLP_H
#define HLP_H 1

#ifndef GABE
#  ifndef hlpp
#     include <level.h>
#  endif
#  define GABE(player,g)                  \
       ((hlpp(player) || wizp(player)) &&  \
       present("hlp#tool",(player)) &&      \
       present("hlp#tool",(player))->gabe(g))
#  define GABE2(player,g)                  \
       ((hlpp(player) || wizp(player)) &&  \
       present("hlp#tool",(player)) &&      \
       present("hlp#tool",(player))->gabe(g,1))
#endif

/*
FUNKTION: GABE
DEKLARATION: int GABE(object spieler, string gabe)
BESCHREIBUNG:
Liefert 1, wenn der Spieler die Gabe besitzt, sonst 0.
gabe ist dabei das zweibuchstabige Kuerzel fuer eine Gabe in Kleinbuchstaben.
Moerder, welche diese Gabe urspruenglich hatten, aber aufgrund des (M)'s
sie nicht mehr besitzen, erhalten eine entsprechende Meldung. Ebenfalls
wird eine Meldung ausgegeben, wenn man die Gabe zwar grundsaetzlich besitzt,
aber aus irgendwelchen Umstaenden sie derzeit nicht verfuegbar ist.
VERWEISE: hlpp, GABE2
GRUPPEN: level
*/

/*
FUNKTION: GABE2
DEKLARATION: int GABE2(object spieler, string gabe)
BESCHREIBUNG:
Liefert 1, wenn der Spieler die Gabe besitzt, sonst 0.
gabe ist dabei das zweibuchstabige Kuerzel fuer eine Gabe in Kleinbuchstaben.
Es wird keinerlei Meldung an den Spieler ausgegeben.
VERWEISE: hlpp, GABE
GRUPPEN: level
*/

#endif // HLP_H
