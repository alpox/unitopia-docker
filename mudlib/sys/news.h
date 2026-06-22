// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /sys/news.h
// Description: Headerfile fuer News
// Author:      Freaky (02.05.94)

#ifndef NEWS_H
#define NEWS_H 1

// Der News-Deamon
#define NEWSD "/apps/newsd"
#define NEWS_INDEX "/apps/news_index"
#define INEWSD "/apps/inewsd"

// Die Maximae Titellaenge
#define MAX_TITLE_LENGTH 65

#define NUMMER  2
#define SUBTREE 1
#define TITEL   0

// Neue Artikelstruktur
#define N_NUMBER  0
#define N_DATE    1
#define N_AUTHOR  2
#define N_TITLE   3
#define N_LINES   4
#define N_SUBTREE 5

// Erlaubnisse
#define PERM_OWNER 0
#define PERM_READ  1
#define PERM_WRITE 2

// Standardverhalten, wenn nix angegeben: 0: nicht erlaubt, 1: erlaubt
#define PERM_STANDARD ({ 0, 1, 1})

#define SOURCE_MUD	0
#define SOURCE_NNTP	1

#define GRP_ALLOW_NAME	1
#define GRP_ALL_BOARDS	2

#define MASTER_FILE	0
#define MASTER_FUN	1
#define MASTER_FLAGS	2

#endif // NEWS_H
