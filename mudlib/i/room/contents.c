// This file is Part of the Unitopia Mudlib
// ----------------------------------------
// Autor:   Jesaia
// Datum:   30.11.97
// file :   /i/room/contents.c
//
// Beschreibung:
// contents.c ist ein inherit, das einem Container oder Raum eingefuegt 
// werden kann und dient dazu, verschiedene Objekte in einem Raum/Container
// darzustellen. In der reset-Routine wird zufaellig entschieden,
// welches Objekt in den Raum kommt, und welches Objekt zerstoert wird.
// Jedes Objekt wird ueber ein folgendes Mapping gesteuert.
//      ([ "name": string,         Name des Objektes, fuer das interne Handling
//         "object":mixed,         Entweder ist der Eintrag ein String, der
//                                 den Filenamen des Objektes angibt oder eine
//                                 closure (#'fun) die vom Typ object ist oder
//                                 ein Array ({object,"funktion"}) fuer einen
//                                 call_other-Aufruf. Die Funktion muss auch 
//                                 vom Typ object sein.
//         "hidden":int,           Das Objekt ist verborgen bis zur naechsten
//                                 Bewegung (hidden_until_next_move)
//         "clone_later":int,      Wenn das Objekt erst nach dem Betrachten
//                                 eines v_items in den Raum geclont werden
//                                 soll (wird dann durch do_clone_now(name)
//                                 ausgefuehrt)
//         "min":int,              wieviele Objekte von dem Typ soll es
//                                 mindestens im Raum geben
//         "max":int,              wieviele Objekte von dem Typ soll es
//                                 maximal im Raum geben
//         "clone_message":string, Message, die alle beim Erstellen erhalten
//                                 werden
//         "remove_message":string, Message beim Zerstoeren
//         "chance":int            Risiko (1-100), dass max. Objekte entstehen
//           ])
// Das ganze wird ueber folgende Funktionen geregelt:
//    void set_max_objects(int max)
//    void set_min_objects(int min)
// Die beiden Funktionen steuern, wieviele Objekte im Raum sind, die von
// contents gesteuert werden.
//
//    void add_object(mapping obj)
//    void delete_object(string name)
//    mapping query_object(string name)
//    mixed *query_all_objects()
// Diese Funktionen sind die Schnittstellen zum Mappinghandling:
//
//    mapping query_present_objects()
//    mapping query_later_objects()
//    void  do_clone_now(string name)
// Diese Funktionen sind die Schnittstellen zum Objecthandling und liefern
// die noetigen returns bzw loest das Clonen eines Objekts aus.
// 
// Damit das ganze laeuft, muss in der Resetfunktion des Raumes oder Containers
// contents::reset() stehen.


// Precompilerkrempel
#include <error.h>
#include <move.h>
#include <message.h>
#pragma save_types

#define ERROR(x) do_error2(x,__FILE__,object_name(),__LINE__)
// Globale Variablen

private mapping all_objects = ([]);
private mapping cl_objects = ([]);
private int max_obj =-1;
private int min_obj =-1;
private int counted_ob =0;
private int reset_mode =0;

mapping get_present_objects();

// Hilfsfunktionen
/*
+-------------------------------------------------------------------------+
| Funktion: int check_name(string name)                                   |
| Beschreibung: Testet, ob der Name schon vorhanden ist                   |
| RC: Integer (True/False) 1/0                                            |
+-------------------------------------------------------------------------+
*/
int check_name(string name)
{
    return member(all_objects,name);
}

/*
FUNKTION: set_random_clone_mode
DEKLARATION: void set_random_clone_mode(integer mode)
BESCHREIBUNG:
Mit dieser Funktion wird das Verhalten bei einem Reset massgeblich veraendert.
Sobald mit set_random_clone_mode(1), das zufallsgesteuerte cloenen von 
Objekten eingeschaltet ist, werden die Eigenschaften, min, chance und
clone_later ausgwertet.
VERWEISE:
GRUPPEN: contents
*/
void set_random_clone_mode(int mode)
{
    if (mode != 1)
	reset_mode=0;
    else
	reset_mode=1;
}

/*
FUNKTION: query_random_clone_mode
DEKLARATION: int query_random_clone_mode()
BESCHREIBUNG:
Liefert den Status des Clone_mode
VERWEISE:
GRUPPEN: contents
*/

int query_random_clone_mode()
{
    return reset_mode;
}
/*
FUNKTION: add_object
DEKLARATION: void add_object(mixed obj)
BESCHREIBUNG:
Fuegt dem Container die Information des Objekts zu. Das Mapping dazu sieht
wie folgt aus:
      ([ "name": string,         Name des Objektes, fuer das interne Handling
         "object":mixed,         Entweder ist der Eintrag ein String, der
                                 den Filenamen des Objektes angibt oder eine
                                 closure (#'fun) die vom Typ object ist oder
                                 ein Array ({object,"funktion"}) fuer einen
                                 call_other-Aufruf. Die Funktion muss auch 
                                 vom Typ object sein.
         "hidden":int,           Das Objekt ist verborgen bis zur naechsten
                                 Bewegung (hidden_until_next_move)
         "clone_later":int,      Wenn das Objekt erst nach dem Betrachten
                                 eines v_items in den Raum geclont werden
                                 soll (wird dann durch do_clone_now(name)
                                 ausgefuehrt)
         "min":int,              wieviele Objekte von dem Typ soll es
                                 mindestens im Raum geben
         "max":int,              wieviele Objekte von dem Typ soll es
                                 maximal im Raum geben
         "clone_message":string, Message, die alle beim Erstellen erhalten
                                 werden.
         "remove_message":string, Message beim Zerstoeren
         "chance":int            Risiko (1-100), dass max. Objekte entstehen
                                 (nur bei set_random_clone_mode(1))
         "no_clone_delay":int    Gibt an, ob beim clonen eine Verzoegerung
                                 verwendet werden sollte. Default = 0
				 ])

Alternative zu dem Mapping kann man einfach nur den absoluten Pfad angeben,
dabei wird das Obj aber bei Reset, sofern nicht vorhanden kreiert.

Bei Meldungen duerfen allgemein Pseudoclosures verwendet werden.
Das betroffene Objekt ist hierbei 'obj.

Beispiel: "$Ein('obj) faellt vom Himmel."

VERWEISE: delete_object, query_object, query_all_objects, set_random_clone_mode
GRUPPEN: contents
*/
void add_object(mixed obj)
{
  if (mappingp(obj))
  {
   if (!member(obj,"name") ||
       !member(obj,"object"))
       {
         ERROR("Die Schlüsselwörter name und "+
               "object müssen definiert sein.\n");
         return;
       }
   if (check_name(obj["name"]) == 1)
      {
        ERROR("Der Name ist schon vergeben.\n");
        return;
      }
   if (!member(obj,"min"))
       obj["min"]=0;
   if (!member(obj,"max"))
       obj["max"]=obj["min"]+1;
   if (obj["max"]<obj["min"])
      {
        ERROR("Min ist größer als Max.\n");
        return;
      }
   if (!member(obj,"chance"))
      obj["chance"]=100; 
   if (!member(obj,"hidden"))
      obj["hidden"]=0;
   if (!member(obj,"clone_later"))
     obj["clone_later"]=0;
   if (!member(obj,"clone_message"))
     obj["clone_message"]=0;
   if (!member(obj,"remove_message"))
     obj["remove_message"]=0;
   if (!member(obj,"no_clone_delay"))
       obj["no_clone_delay"]=1;
   all_objects += ([obj["name"]:obj
                  ]);
   return;
  }
  if (stringp(obj))
  {
     if (check_name(obj) == 1)
     {
        ERROR("Der Name ist schon vergeben.\n");
        return;
     } 
    all_objects +=([obj:([
                  "name":             obj,
                  "object":           obj,
                  "hidden":           0,
                  "clone_later":      0,
                  "min":              0,
                  "max":              1,
                  "clone_message":    "",
                  "remove_message":   "",
                  "chance":           100,
		  "no_clone_delay":   1
          ])]);
    return;
  }
}
/*
FUNKTION: delete_object
DEKLARATION: void delete_object(string name)
BESCHREIBUNG:
Die Beschreibung zum Objekt name wird wieder geloescht, beim naechsten Reset
werden keine Objekte name erstellt.
VERWEISE: add_object, query_object, query_all_objects
GRUPPEN: contents
*/
void delete_object(string name)
{
   if (!check_name(name))
   {
       ERROR("Dieses Object gibt es nicht.\n");
       return;
   }
   m_delete(all_objects,name);
}
/*
FUNKTION: query_object
DEKLARATION: mapping query_object(string name)
BESCHREIBUNG:
Liefert ein Mapping, welches das Objekt name beschreibt
Mappingstruktur siehe add_object.
VERWEISE: add_object,delete_obj_des, query_all_objects
GRUPPEN: contents
*/
mapping query_object(string name)
{
  if (!check_name(name))
     return ([]);
  return copy(all_objects[name]);
}
/*
FUNKTION: query_all_objects
DEKLARATION: mixed *query_all_objects()
BESCHREIBUNG:
Diese Funktion liefert ein Array mit den Mappings von jedem Objekt als
Rueckgabewert.
Mappingstruktur siehe add_object.
VERWEISE: add_object,delete_object, query_object
GRUPPEN: contents
*/
mixed *query_all_objects()
{
  mixed RC,names;
  int x;
  names = m_indices(all_objects);
  RC = ({});
  for (x=sizeof(names);x--;)
  {
      RC += ({copy(all_objects[names[x]]) });
  }
  return ({})+RC;
}
/*
FUNKTION: query_present_objects
DEKLARATION: mapping query_present_objects()
BESCHREIBUNG:
Diese Funktion liefert ein Mapping mit allen Objektnamen,
die im Container sind und von contents verwaltet werden.
Das Mapping sieht wie folgt aus:
         RC = ([ name1: anzahl1,
                 name2: anzahl2,
                 ....
              ])
Anzahl sagt, wieviel mal das obj name im Raum ist.
VERWEISE: query_later_objects, do_clone_now
GRUPPEN: contents
*/
mapping query_present_objects()
{
    mapping RC;
    RC = get_present_objects();
    return ([])+RC;
}
/*
FUNKTION: query_later_objects
DEKLARATION: mapping query_later_objects()
BESCHREIBUNG:
Diese Funktion liefert alle Objektnamenn die noch geclont werden koennen mit
do_clone_now().
Das Mapping sieht wie folgt aus:
         RC = ([ name1: anzahl1,
                 name2: anzahl2,
                 ....
              ])
Anzahl sagt, wieviele Male das Objekt name im Raum ist.
VERWEISE: query_present_objects, do_clone_now
GRUPPEN: contents
*/
mapping query_later_objects()
{
     return ([])+cl_objects;
}
/*
FUNKTION: set_max_objects
DEKLARATION: void set_max_objects(int max)
BESCHREIBUNG:
Diese Funktion setzt die Anzahl Objekte, die von contents 
verwaltet werden, die maximal im Container sein duerfen.
VERWEISE: set_min_objects
GRUPPEN: contents
*/
void set_max_objects(int max)
{
    max_obj =max;
}
/*
FUNKTION: set_min_objects
DEKLARATION: void set_min_objects(int max)
BESCHREIBUNG:
Diese Funktion setzt die Anzahl Objekte, die von contents 
verwaltet werden, die minimal im Container sein muessen.
VERWEISE: set_max_objects
GRUPPEN: contents
*/
void set_min_objects(int min)
{
    min_obj =min;
}
/*
+-------------------------------------------------------------------------+
| Funktion: int id_such(mixed str)                                        |
| Beschreibung: Testfunktion fuer get_present, damit die richtige ID      |
|               behandelt und anylsiert wird                              |
| RC: Integer (True/False) 1/0                                            |
+-------------------------------------------------------------------------+
*/
int id_such(mixed str)
{
      if (!stringp(str))
         return 0;
      if (strstr(str,"#Doicname#")>-1)
         return 1;
}
/*
+-------------------------------------------------------------------------+
| Funktion: mapping get_present_objects()                                  |
| Beschreibung: Liefert ein Mapping von allen contents Obj. im Container  |
| RC: mapping ([name1:anzahl1, name2:anzahl2 ... ])                       |
+-------------------------------------------------------------------------+
*/
mapping get_present_objects()
{
     object *all_obj;
     int x;
     mapping RC;
     mixed temp;
     string str;
     RC =([]);

     all_obj=filter_objects(all_inventory(this_object()),"id","#Doic#id#");
     for (x=sizeof(all_obj);x--;)
       {
          temp = filter(all_obj[x]->query_id(),"id_such");
          if (!sizeof(temp)) continue;
          if(pointerp(temp))
            temp = temp[0];
            sscanf(temp,"#Doicname#%s",str);                    
            if (member(RC,str))
               RC[str]+= 1;
            else
	      RC+= ([str:1]);
       }
      return RC;
}
/*
+-------------------------------------------------------------------------+
| Funktion: mapping new_calc                                              |
| Beschreibung: Rechnet eine neue Konstellation der Objekte aus           |
| RC: mapping ([ name1:anzahl1.... ])                                     |
+-------------------------------------------------------------------------+
*/
mapping new_cal()
{
    mixed *index;
    mapping RC;
    int x,y,anzahl;
    
    counted_ob =0;
    RC =([]);
    index = m_indices(all_objects);
    for (x=sizeof(index);x--;)
      {
        anzahl = all_objects[index[x]]["min"];
        for (y=all_objects[index[x]]["min"];y<all_objects[index[x]]["max"];y++)
	  {
      //        if((diff=all_objects[index[x]]["max"]-all_objects[index[x]]["min"]) > 0)
                 if (random(100)<(all_objects[index[x]]["chance"])) // /diff)
                     anzahl ++;
          }
        RC += ([index[x]:anzahl]);
      }
    return RC;
}
/*
+-------------------------------------------------------------------------+
| Funktion: void handle_cl_obj                                            |
| Beschreibung: Verwaltet die clone_later Objekte.                        |
| RC: --                                                                  |
+-------------------------------------------------------------------------+
*/
void handle_cl_obj(string name, int flag)
{
      if (flag)
        {
           if (member(cl_objects,name))
              cl_objects[name] ++;
           else
	     cl_objects+=([name: 1]);
        }
      else
           if (member(cl_objects,name))
              cl_objects[name] --;
} 
/*
+-------------------------------------------------------------------------+
| Funktion: void obj_remover                                              |
| Beschreibung: Verwaltet das Zerstoeren von contents Objekten            |
| RC: --                                                                  |
+-------------------------------------------------------------------------+
*/
void obj_remover(string name,mixed message)
{
      object ob;
      object *inv;
      closure cl_msg; 
      if(ob=present ("#Doicname#"+name,this_object())) 
      {
        if (ob->query_enable_cleanup())
	{
       	  if (stringp(message) && message !="" && environment(ob)) 
	  {
	     cl_msg = mixed_to_closure(message, ({'obj}));
	     ob->send_message(MT_LOOK,MA_LOOK,wrap(
			 ob->closure_to_string(cl_msg, ({ob}) ))); 
	  }
	  inv = deep_inventory(ob);
	  if(inv)
	      inv->remove();
	  ob->remove();
         }
       }

      // else 
//	  if (stringp(message) && message != "")
//             this_object()->send_message(MT_LOOK,MA_LOOK,wrap(message));
}
/*
+-------------------------------------------------------------------------+
| Funktion: void obj_creator                                              |
| Beschreibung: Verwaltet das create von contents Objekten                |
| RC: --                                                                  |
+-------------------------------------------------------------------------+
*/
void obj_creator(string name, mixed message)
{
     object ob;
     closure cl_msg;
     if (all_objects[name]["clone_later"])
     {
        handle_cl_obj(name,1);
        if (stringp(message) && message != "")
            this_object()->send_message(MT_LOOK,MA_MOVE_IN,wrap(message));
        return;
     }
     if (closurep(all_objects[name]["object"]))
        ob = funcall(all_objects[name]["object"]);
     else 
        if (pointerp(all_objects[name]["object"]))
              ob = call_other(all_objects[name]["object"][0],all_objects[name]["object"][1]);
        else
              ob = clone_object(all_objects[name]["object"]);
     if (!objectp(ob))
       {
          ERROR("Fehler beim Clonen von "+name+".\n");
          return;
       }
     ob->add_id("#Doic#id#");
     ob->add_id("#Doicname#"+name );
    
     if (MOVE_OK == ob->move(this_object()))
       {
         if (all_objects[name]["hidden"])
            ob->set_hidden_until_next_move();
         if (stringp(message) && message !="" && environment(ob))
	 {
	     cl_msg=mixed_to_closure(message, ({'obj}));
	     ob->send_message(MT_LOOK,MA_MOVE_IN,wrap(
			 ob->closure_to_string(cl_msg, ({ob}) )));
	 }
	 return;
       }
     ERROR("Fehler beim move von "+ob->query_name()+" in container.\n");
}
/*
FUNKTION: do_clone_now
DEKLARATION: void do_clone_now(string name)
BESCHREIBUNG:
Clont das contents-Objekt name. Diese Objekt muss den status "clone_later":1
haben und in query_later_objects enthalten sein. Das Objekt wird in den Raum
bewegt.
VERWEISE:
GRUPPEN: contents
*/
void do_clone_now(string name)
{
    object ob;
    if (!member(cl_objects,name))
      {
         ERROR(name +"ist nicht zu clonen jetzt.\n");
         return;
      }
   if(cl_objects[name] == 0)
     {
        ERROR(name+" ist gleich null.\n");
        return;
     }
     cl_objects[name] --;
     if (closurep(all_objects[name]["object"]))
        ob = funcall(all_objects[name]["object"]);
     else 
        if (pointerp(all_objects[name]["object"]))
              ob = call_other(all_objects[name]["object"][0],all_objects[name]["object"][1]);
        else
              ob = clone_object(all_objects[name]["object"]);
     if (!objectp(ob))
       {
          ERROR("Fehler beim Clonen von "+name+".\n");
          return;
       }
     ob->add_id("#Doic#id#");
     ob->add_id("#Doicname#"+name );
    
     if (MOVE_OK == ob->move(this_object()))
       {
         if (all_objects[name]["hidden"])
            ob->set_hidden_until_next_move();
         return;
       }
     ERROR("Fehler beim move von "+ob->query_name()+" in container.\n");
}  

void handlefastreset()
{
     int x,y;
     mixed tmpObj;
     object ob, *all_obj;
     string message,tmpKey;
     closure cl_msg;

     foreach (tmpKey, tmpObj:all_objects)
     {
       all_obj=filter_objects(all_inventory(this_object()),"id","#Doicname#"+
               tmpKey);
      x = tmpObj["max"] - sizeof(all_obj);
       message = all_objects[tmpKey]["clone_message"];
       if (x > 0 )
          for (y=x;y--;)
          {
              if (closurep(all_objects[tmpKey]["object"]))
                  ob = funcall(all_objects[tmpKey]["object"]);
              else
                  if (pointerp(all_objects[tmpKey]["object"]))
                   ob = call_other(all_objects[tmpKey]["object"][0],
                        all_objects[tmpKey]["object"][1]);
                  else
                   ob = clone_object(all_objects[tmpKey]["object"]);
              if (!objectp(ob))
              {
		   // Entweder es gibt sowieso einen Fehler (z.B.
                   // vom clone_object) oder es ist gewollt, dass
		   // jetzt gerade kein Objekt erscheinen soll.
         	   //ERROR("Fehler beim Clonen von "+tmpKey+".\n");
                   continue;
              }
              ob->add_id("#Doic#id#");
              ob->add_id("#Doicname#"+tmpKey );

             if (MOVE_OK == ob->move(this_object()))
            {
                if (all_objects[tmpKey]["hidden"])
                   ob->set_hidden_until_next_move();
                if (stringp(message) && message !="" && environment(ob))
		{
		    cl_msg= mixed_to_closure(message, ({'obj}));
       		    ob->send_message(MT_LOOK,MA_MOVE_IN,
			    wrap(ob->closure_to_string(cl_msg,({ob}) )));
		}
                continue;
            }
            ERROR("Fehler beim move von "+ob->query_name()+
                    " in container.\n");
          }
    }
}

void reset()
{
    int dif_x,x,y,z;
    mixed *index,index2;
    mapping new_calc,present_objs;
    string name = object_name(this_object());

    if  (strstr(name, "/i/") >= 0)
        return; // ist ein inherit

    if  (!clonep() && (strstr(name, "/obj/") >= 0))
        return; // ein nicht-clone in /obj/ -> blueprint
     
    if (reset_mode == 0)
    {
       handlefastreset();
       return;
    }       
    present_objs = get_present_objects();
    new_calc = new_cal();
    cl_objects=([]);    
    // So checken wir mal die Umgebungsvariabeln ab

    if (min_obj> 0 && counted_ob < min_obj)    
       if (counted_ob != 0)
	 {
          dif_x = min_obj - counted_ob;
          index2 = m_indices(new_calc);
          y= sizeof(index2);
          for(x=dif_x;x--;)
            {
              new_calc[index2[random(y)]]++;
            }
         }
       else
	 {
          x =0;
          do
            {
              new_calc =new_cal();
              x++;
            }
          while (counted_ob == 0 || x < 5);
          if (counted_ob != 0 && counted_ob < min_obj )
           {
              dif_x = min_obj - counted_ob;
              index2 = m_indices(new_calc);
              y= sizeof(index2);
              for(x=dif_x;x--;)
              {
                 new_calc[index2[random(y)]]++;
              }
            }
          else
           if (x >4)
             ERROR("Der Parameter Min_object_in_container ist nicht ideal gewählt.\n");
         }
     if (max_obj>0 && counted_ob > max_obj)
        {
          dif_x = counted_ob - max_obj;
          index2 = m_indices(new_calc);
          y= sizeof(index2);
          for(x=dif_x;x--;)
            {
              for( z=y; z--;)
	       {
                 if (new_calc[index2[z]] < 1)
		  {
                   index2 -=({index2[z]});
                   y --;
                  }
	       }
              new_calc[index2[random(y)]]--;
            }
         }
     // So dann gucken wir mal was wir alles Rauswerfen muessen...
     index = m_indices(all_objects);
     for(x=sizeof(index);x--;)
       {
          if(member(present_objs,index[x]) && member(new_calc,index[x]))
            if (present_objs[index[x]] > new_calc[index[x]])
	      {
                for(y=present_objs[index[x]]-new_calc[index[x]];y--;)
		{
		    if(all_objects[index[x]]["no_clone_delay"]== 1)
		    {
                    call_out("obj_remover",0,index[x],
			    all_objects[index[x]]["remove_message"]);
		    }
		    else
		    {
	  	    call_out("obj_remover",x*20+random(15),index[x],
                             all_objects[index[x]]["remove_message"]);
		    }
	       	}
	      }
           else
	     {
	     if (present_objs[index[x]] < new_calc[index[x]])
	      {
                for(y=new_calc[index[x]]-present_objs[index[x]];y--;)
		  {
                    if (all_objects[index[x]]["no_clone_delay"]==1)
		    {
			call_out("obj_creator",0,index[x],
	                     all_objects[index[x]]["clone_message"]);
		    }
	    	    else
	            {
			    call_out("obj_creator",x*20+random(15),index[x],
                              all_objects[index[x]]["clone_message"]);
		    }
		  }
	      }
             }
         else
	 {
          if(member(present_objs,index[x]))
	    {
              for(y=present_objs[index[x]];y--;)
		  {
                    if(all_objects[index[x]]["no_clone_delay"]==1)
		    {
			call_out("obj_remover",0,index[x],
                             all_objects[index[x]]["remove_message"]);
		    }
		    else
		    {
			call_out("obj_remover",x*20+random(15),index[x],
	                     all_objects[index[x]]["remove_message"]);
		    }
                  }
	     }
          
          if (member(new_calc,index[x]))
             {
                for(y=new_calc[index[x]];y--;)
		  {
                   if(all_objects[index[x]]["no_clone_delay"]==1)
		   {
		       call_out("obj_creator",0,index[x],
                            all_objects[index[x]]["clone_message"]);
		   }
		   else
		   {
		       call_out("obj_creator",y*20+random(15),index[x],
		            all_objects[index[x]]["clone_message"]);
		   }
		  }
	     }
         }
 }
}










