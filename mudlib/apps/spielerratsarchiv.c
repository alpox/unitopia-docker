// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /apps/spielerratsarchiv.c
// Description: Archiv des Spielerrats
// Author:      Gnomi

// UID: Apps

#include <level.h>

#define PAGE_DIR  "/var/adm/spielerratsarchiv/"
#define SAVE_FILE "/var/adm/spielerratsarchiv/archiv"

// Format: ([ Buchtitel: Array aus den Einzelnen Seiten, ... ])
// Jede Seite hat den Aufbau:
//   ({ string Titel, string Datum, string Dateiname, int Zeilenanzahl })

mapping pages=([]);

void add_book(string id)
{
   if(strstr(object_name(previous_object()),
       "/room/rathaus/spielerrat/raum3"))
           return;
    if(!member(pages, id))
        m_add(pages, id, ({}));
    save_object(SAVE_FILE);
    
}

void set_pages(string id, mixed p)
{
   if((strstr(object_name(previous_object()),
       "/room/rathaus/spielerrat/obj/buch#") || !pages[id]) &&
       !adminp(this_interactive()))
           return;
   if(!p) pages-=([id]);
   else pages[id]=p;
   save_object(SAVE_FILE);
}

mixed get_pages(string id)
{
   if(strstr(object_name(previous_object()),
       "/room/rathaus/spielerrat/obj/buch#"))
           return 0;
   return pages[id];
}

string get_page(string file)
{
   if(strstr(object_name(previous_object()),
       "/room/rathaus/spielerrat/obj/buch#"))
           return 0;
    return read_file(PAGE_DIR + file);
}

void remove_page(string file)
{
   if(strstr(object_name(previous_object()),
       "/room/rathaus/spielerrat/obj/buch#"))
           return 0;
    rm(PAGE_DIR + file);
}

string create_page(string text)
{
    string file = get_unique_string();
    write_file(PAGE_DIR + file, text);
    return file;
}

void create()
{
  restore_object(SAVE_FILE);
}
      
int clean_up(int arg)
{
    destruct(this_object());
    return 1;
}
