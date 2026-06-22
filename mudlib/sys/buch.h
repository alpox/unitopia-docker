// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /sys/buch.h
// Description: Definitionen fuer /i/object/buch.c
// Author:      Ohgott (24.10.2018)

#ifndef BUCH_H
#define BUCH_H

// Keys fürs set_seiten_desc Mapping
#define PAGE_DESC_NAME                "name"
#define PAGE_DESC_GENDER              "gender"
#define PAGE_DESC_IDS                 "ids"
#define PAGE_DESC_NO_PAGES            "no_pages"
#define PAGE_DESC_WHICH               "which_page"
#define PAGE_DESC_INVALID             "invalid_page"
#define PAGE_DESC_OVERFLOW            "page_overflow"
#define PAGE_DESC_NO_PREV             "no_previous_page"
#define PAGE_DESC_NO_NEXT             "no_next_page"
#define PAGE_DESC_READ_MSG            "read_page_msg"
#define PAGE_DESC_READ_TOC_MSG        "read_toc_msg"
#define PAGE_DESC_STOP                "stop_reading"
#define PAGE_DESC_STOP_MSG            "stop_reading_msg"
#define PAGE_DESC_NOT_ALLOWED         "not_allowed"
#define PAGE_DESC_NOT_SEARCHABLE      "not_searchable"
#define PAGE_DESC_EMPTY_SEARCH        "empty_search"
#define PAGE_DESC_SEARCH_BEYOND       "search_beyond_end"
#define PAGE_DESC_SEARCH_TLE          "search_too_long_evaluation"
#define PAGE_DESC_SEARCH_REACHED_LAST "search_reached_last_page"
#define PAGE_DESC_OPEN                "open_book"
#define PAGE_DESC_OPEN_MSG            "open_book_msg"
#define PAGE_DESC_CLOSE               "close_book"
#define PAGE_DESC_CLOSE_MSG           "close_book_msg"

// Defines für set_page_mode
#define PAGE_MODE_MORE     "more"
#define PAGE_MODE_PAGE     "page"
#define PAGE_MODE_MOREFILE "morefile"

#endif // BUCH_H
