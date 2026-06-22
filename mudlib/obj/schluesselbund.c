// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/obj/schluesselbund.c
// Description: Schluesselbund
// Author:	Sissi (03.04.98)

inherit "/i/item";
inherit "/i/move";
inherit "/i/value";
inherit "/i/contain";
#ifdef UNItopia
inherit "/p/Item/i/anbinden";	// Damit man's an einen Guertel haengen kann
#endif

#include <message.h>

void create()
{
   set_name("schlüsselring");
   set_gender("maennlich");
   set_id(({"schlüsselring","schlüsselbund","ring","bund"}));
   set_material("metall");
   set_long("Ein leicht angerosteter und schon etwas verbogener "
      "metallener Ring, welcher Dich dazu einlädt, Schlüssel "
      "an ihn zu hängen. Und dann mit ihm zu klimpern.");
   set_content_message("Am Schlüsselring hängen:");
   set_put_prepos("an");
   set_put_verb("häng");
   set_take_prepos("von");
   allow_only(({"schlüssel","darf # an # schlüsselbund"}),
      "An "+den()+" kannst Du nur Schlüssel hängen.");
   set_weight(1);
   set_value(5);
   set_no_store(1);
   set_max_internal_encumbrance(15);
   set_min_weight(1); set_max_weight(2);
   set_transparent(1);
}

#ifdef UNItopia
string query_long(object betrachter)
{
    return item::query_long(betrachter);
}

// query_anbinden_long ans long anhaengen lassen:
protected string query_long_postprocess(string msg, mapping info)
{
    return item::query_long_postprocess(
          anbinden::query_long_postprocess(msg, info), info);
}

// query_anbinden_short statt normalem short, falls angebunden:
string query_short(object viewer)
{
    return query_anbinden_short(viewer) || item::query_short(viewer);
}
#endif

void init()
{
   "*"::init();
   add_action("klimper","klimpere",-7);
   add_action("klimper","klimpre");
}

int klimper(string s)
{
   int anz_schluessel;
   if(!s) {
      notify_fail("Womit willst Du klimpern?\n");
      return 0;
   }
   if (s[0..3]=="mit ") s = s[4..];
   if (!me (s)) {
      notify_fail("Damit kannst Du nicht klimpern.\n");
      return 0;
   }
   if(!(anz_schluessel=sizeof(all_inventory()))) {
      notify_fail("Ohne Schlüssel? Wie soll das gehen?\n");
      return 0;
   }
   if(anz_schluessel==1) {
      send_message(MT_NOTIFY,MA_CRAFT,wrap(
         Der(this_player())+" klimpert mit "+dem()+" herum. "
         "Da allerdings nur ein Schlüssel dranhängt hört sich "
         "das arg kümmerlich an."),wrap(
         "Du klimperst mit "+deinem()+" herum. "
         "Da allerdings nur ein Schlüssel dranhängt hört sich "
         "das arg kümmerlich an."),this_player());
      return 1;
   }
   if(anz_schluessel<5) {
      send_message(MT_NOTIFY,MA_CRAFT,wrap(
         Der(this_player())+" klimpert mit "+dem()+" herum. "
         "Das klimpert schon ganz schön."),wrap(
         "Du klimperst mit "+deinem()+" herum. "
         "Das klimpert schon ganz schön."),this_player());
      return 1;
   }
   send_message(MT_NOTIFY,MA_CRAFT,wrap(
      Der(this_player())+" klimpert mit "+dem()+" herum. "
      "Das klimpert gar fürchterlich laut! Klimper klimper klimper klimp."),
      wrap("Du klimperst mit "+deinem()+" herum. "
      "Das klimpert gar fürchterlich laut! Klimper klimper klimper klimp."),
      this_player());
   return 1;
}
 
int query_schluesselbund()
{
   return 1;
}

int query_collapsible()
{
    return 1;
}
