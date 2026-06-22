// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/apps/store_master.c
// Description: Verwaltung der verschienden Lager der Laeden.
// Author:	Garthan	(25.07.94)

object *stores = ({});

void add_store(object ob)
{
   if(ob && ob->query_store())
   {
      stores -= ({ ob });
      stores += ({ ob });
   }
}

void delete_store(object ob)
{
   if(ob)
   {
      stores -= ({ ob });
   }
}

object *query_stores()
{
   stores -= ({ 0 });
   return ({})+stores;
}

object *query_import_stores()
{
   return filter_objects(query_stores(), "want_import");
}
