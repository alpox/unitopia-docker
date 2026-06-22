// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/secure/simul_efun/deklin.c
// Description:	Deklination von Pronomina, Substantiven, Adjektiven
// Author:	Garthan (15.01.93)
// Modified by: Kurdel (05.02.97) string_parser fuer Argumentlisten erweitert
//              Mammi (25.01.99-09.05.99) Genitiv/Dativ/Akkusativ-Problematik
//                                        debuggt.
//              Freaky (07.06.2000) query_deklin_name: query_dekliniert...
//                  korrigiert
//              Gnomi  (04.01.2001) Mengenbezeichnungen eingefuehrt.

#pragma save_types
#pragma strict_types

/* =================== Defines && Includes ===================== */

#include "/sys/deklin.h"  /* ART_..., FALL_... und OBJ_... Konstanten */
#include "/sys/invis.h"
#include "/sys/parse_com.h"
#include "/sys/error.h"
#include "/sys/misc.h"
#include "/sys/apps.h"

#undef  DEKLIN_LOG
#undef  DEKLIN_DEBUG

#define PRON_DER              0
#define PRON_BEST             1
#define PRON_UNBEST           2
#define PRON_UNBEST_NACH_BEST 3
#define PRON_1_PERS           4
#define PRON_2_PERS           5
#define PRON_3_PERS           6

#undef QUERY
#define QUERY(x,ob) (mappingp(ob)								\
	?((closurep((ob)[x])&&get_type_info((ob)[x],1)==2)					\
	    ?funcall((ob)[x]):((member((x),':')>=0&&((ob)["v_item_master"]))			\
	        ?({mixed})(ob)["v_item_master"]->query_v_item_property((ob),(x)):funcall((ob)[x],ob)))	\
	:(({mixed})call_other(ob,"query_"+(x)) || ({mixed})call_other(ob, "query", (x))))

/* =================== Global Data ============================= */

private static string*** pronomen =  // Alle Pronomen
({
   ({ // PRON_DER
      ({ "das", "des", "dem", "das" }),
      ({ "der", "des", "dem", "den" }),
      ({ "die", "der", "der", "die" }),
      ({ "die", "der", "den", "die" }),
   }),
   ({ // PRON_BEST
      ({ "es","es", "em", "es" }),
      ({ "er","es", "em", "en" }),
      ({ "e", "er", "er", "e"  }),
      ({ "e", "er", "en", "e"  }),
   }),
   ({ // PRON_UNBEST
      ({ "" , "es", "em", ""   }),
      ({ "" , "es", "em", "en" }),
      ({ "e", "er", "er", "e"  }),
      ({ "e", "er", "en", "e"  }),
   }),
   ({ // PRON_UNBEST_NACH_BEST
      ({ "e", "en", "en", "e"  }),
      ({ "e", "en", "en", "en" }),
      ({ "e", "en", "en", "e"  }),
      ({ "en","en", "en", "en" }),
   }),
   ({ // PRON_1_PERS
      ({  "ich", "meiner", "mir", "mich" }),
      ({  "ich", "meiner", "mir", "mich" }),
      ({  "ich", "meiner", "mir", "mich" }),
      ({  "wir", "unser" , "uns", "uns"  })
   }),
   ({ // PRON_2_PERS
      ({  "du" , "deiner", "dir", "dich" }),
      ({  "du" , "deiner", "dir", "dich" }),
      ({  "du" , "deiner", "dir", "dich" }),
      ({  "ihr", "euer"  , "euch","euch" })
   }),
   ({ // PRON_3_PERS
      ({  "es",  "seiner", "ihm",  "es"  }),
      ({  "er",  "seiner", "ihm",  "ihn" }),
      ({  "sie", "ihrer" , "ihr",  "sie" }),
      ({  "sie", "ihrer" , "ihnen","sie" })
   })
});

private static string** endungen =  // Alle Adjektivendungen
({
   // bestimmt          | unbestimmt     | ohne Pronom     |
   // (schwach)         | (gemischt)     | (stark)
   // ------------------+----------------+-----------------+
   // der, dieser       | ein, mein, dein|                 |
   //                   | sein, kein     | Plural von ein  |
   // ------------------+----------------+-----------------+
   //    Nom,Gen,Dat,Akk| Nom,Gen,Dat,Akk| Nom,Gen,Dat,Akk |
   ({ 0 ,"" ,"n","n","" , "s","n","n","s", "s","n","m","s" }), // Sg: neutrum
   ({ 0 ,"" ,"n","n","n", "r","n","n","n", "r","n","m","n" }), // Sg: maskulin
   ({ 0 ,"" ,"n","n","" , "" ,"n","n","" , "" ,"r","r",""  }), // Sg: feminin
   ({ 0 ,"n","n","n","n", "n","n","n","n", "" ,"r","n",""  })  // Pl: alle drei
});

private static string * genders = ({ "maennlich", "weiblich" });
private static string * jemand  = ({ "", "ens", "em", "en" });

private static string dativ_plural_endungen = "ansi";
private static string genitiv_endungen = "szxSZX";

/* ============== Generic Utility Functions ==================== */

/*
FUNKTION: not_alone
DEKLARATION: int not_alone(object ob)
BESCHREIBUNG:
Liefert 1 zurueck, wenn sich weitere Objekte mit derselben Id wie ob
in der Umgebung von ob befinden.
(wird beispielsweise zur Automatischen Artikel Auswahl (ART_AAA) in
der Grammatik/Deklination verwendet.
GRUPPEN: simul_efun, grammatik
*/
int not_alone(object|mapping ob)
{
   string *ids;

   if(!objectp(ob))
      return 0;
   if(!(ids = ({string *})ob->query_id()) || !sizeof(ids) || !environment(ob))
      return 0;
   return present(ids[0]+" 2", environment(ob)) != 0;
}


/*
FUNKTION: auto_owner_search
DEKLARATION: object auto_owner_search(object|mapping ob)
BESCHREIBUNG:
Liefert das naechst-umgebende lebende Objekt von ob zurueck
oder 0 falls kein lebendes Objekt gefunden.

Mit anderen Worten: Es wird der Besitzer eines Objekts
zurueckgeliefert, sprich derjenige, der es mit sich rumtraegt.
VERWEISE: sein, seines, seinem, seinen, ihr, ihres, ihrem, ihren
GRUPPEN: simul_efun, grammatik
*/
object auto_owner_search(object|mapping ob)
{
   do
   {
      ob = mappingp(ob) ?
	 ob["environment"] :
	 environment(ob);
   }
   while(ob && !(objectp(ob) && living(ob)));
   return ob;
}

/* ============== STRING_PARSER FUNCTIONS ====================== */

/*
FUNKTION: string_parser
DEKLARATION: varargs mixed string_parser(string str, int tp_flag, int|symbol use_tp, mapping functions)
BESCHREIBUNG:
Der uebergebene String str wird nach Funktionen durchsucht und ein Array
aus Closures und Strings aufgebaut und zurueckgegeben, welches fuer die
Konstruktion einer lambda-Closure verwendet werden kann.
Enthaelt der String str Funktionen, so muessen sie in der Form "$fun(args)"
geschrieben werden. Dabei ist fun der Name der Funktion und args ist eine
Liste von durch Kommas getrennten Argumenten.
Argumente koennen Mappings, Arrays, Strings, Integerzahlen, Closures oder
Symbole sein.
In Mappings und Arrays koennen keine Closures oder Symbole vorkommen!
Die Strings koennen ohne "" geschrieben werden, Zahlen werden als Argumente
und Mappingeintraege automatisch als solche erkannt. Will man partout eine
Zahl als Stringargument, muss man sie in "" setzen, das Quoten mit \ in
LPC-Strings nicht vergessen!

Ist tp_flag gesetzt, wird Funktionen ohne Argument entweder 'tp (use_tp==1),
use_tp (falls use_tp ein Symbol ist) oder OBJ_TP uebergeben.

Ist use_tp == 1, so wird 'tp eingesetzt, wenn OBJ_TP im String vorkam.
Ist use_tp ein Symbol, so wird es statt OBJ_TP eingesetzt.

Im Mapping functions kann man noch zusaetzliche Funktionen angeben.
Dabei muss einem Funktionsname eine Closure zugeordnet werden.
(z.B. (["hinweis": #'hinweis]), dann wird bei $hinweis(OBJ_TP) hinweis(OBJ_TP)
aufgerufen.)

string_parser wird von mixed_to_closure verwendet, im Normalfall kommt man
damit vor allem bei msgs von v_items und Bewegungsmeldungen in Beruehrung.
Argumente sind vor allem praktisch, wenn man sein/ihr verwenden moechte
bei der Erwaehnung von Sinnen oder Koerperteilen des Spielers in den msgs,
man muss dann nicht mehr eigenhaendig Closures konstruieren.

Beispiele:

   Bewegungsmeldungen:

      "$Der() naehert sich $dir()."
      "$Der(OBJ_TP,besoffen) torkelt $dir() davon."
      "Du stolperst ueber $den(OBJ_TP,\"\")." (kein Adjektiv)

   v_item-msgs:

      "$Des(OBJ_TP,ungeschickt) Versuch schlaegt fehl."
      "$Sein(([name:haare,gender:saechlich,plural:1]),({lila,lilan}),
         OBJ_TP) stehen $ihm() gut."
      "$Ihr(([name:augen,plural:1,gender:saechlich]),mued,OBJ_TP,
         erschoepft) fallen zu."
      "$Ihr(([name:nase,gender:weiblich]),empfindlich,OBJ_TP
       ,herumschnueffelnd) kraeuselt sich."

VERWEISE: mixed_to_closure
GRUPPEN: simul_efun, grammatik
*/

// Kleine Subroutine, um einen Wert zu bestimmen. Dies ist notwendig,
// um sich (vor allem bei Arrays/Mappings) rekursiv aufrufen zu koennen.
// Parameter:
//   str - Der zu parsende String
//   pos - An dieser Stelle muss geparst werden, wird per Referenz
//         uebergeben. -1 als Ergebnis bedeutet einen Fehler.
//   abbruch - Eine Regexp, die angibt, wo bei einem String (dem ja die
//             Begrenzungszeichen "" fehlen) abgebrochen werden soll
//   use_tp - Anstatt OBJ_TP wird 'tp genutzt
//   deep - 1, wenn dieser Wert sich innerhalb eines Arrays oder Mappings
//          befindet. (Dann gehen 'symbol und #'funktion nicht mehr.
//          Arrays werden dann auch nicht mehr gequotet.)
#if 1
varargs mixed string_parser(string str, int tp_flag, int|symbol use_tp, mapping functions)
{
    return ({mixed})CLOSURE_CONTAINER->string_parser(str, tp_flag, use_tp, functions);
}
#else
private mixed value_parser(string str, int pos, string abbruch, symbol use_tp, int deep)
{
   if(str[pos..pos+1]=="({")  // Ein Array
   {
      mixed tmp=({});
      pos+=2;
      while(str[pos..pos+1]!="})")
      {                                              // Naechste Element holen
         tmp+=({value_parser(str, &pos, "}\\)|,", use_tp, 1)});
         if(pos<0) return 0;                         // Bei Fehler -> Ende
         if(str[pos..pos]==",")                      // Komma uebergehen
            pos++;
         else if(str[pos..pos+1]!="})")              // Kein Komma und kein })
         {                                           // -> Feher
            pos=-1;
            return 0;
         }
      }
      pos+=2;                                        // Fuer "})"
      return deep?tmp:quote(tmp);                    // Quoten fuer die Lambda
   }
   else if(str[pos..pos+1]=="([")                    // Ein Mapping
   {
      mapping tmp=([]);
      pos+=2;
      while(str[pos..pos+1]!="])")
      {                                              // Erster Wert, der Schluessel
         string key = value_parser(str, &pos, ":|,|\\]\\)", use_tp, 1);
         if(pos<0) return 0;
         if(str[pos..pos]==":")                      // Ein Paar Key:Value
         {
            pos++;
	    tmp+=([key:value_parser(str, &pos, ",|\\]\\)", use_tp, 1) ]);
	    if(pos<0) return 0;
         }
	 else if(str[pos..pos]==",")                 // Nur ein Schluessel
	    pos++;                                   // ohne Wert
         else if(str[pos..pos+1]!="])")              // -> Fehler
         {
            pos=-1;
            return 0;
         }
      }
      pos+=2;                                        // Fuer "])"
      return tmp;
   }
   else if(str[pos..pos]=="(")                       // normale Klammerung
   {
      mixed tmp;                                     // Einfach rekursiv
      pos++;                                         // aufrufen,
      tmp=value_parser(str, &pos, "\\)", use_tp, deep);
      if(str[pos..pos]!=")")                         // um danach die
      {                                              // Klammerung testen
         pos=-1;                                     // zu koennen.
	 return 0;
      }
      pos++;
      return tmp;
   }
   else if(str[pos..pos]=="\"")                      // Ein String in ""
   {
      int i;
      string tmp;
      i=strstr(str,"\"",pos+1);
      if(i<0)
      {
         pos=-1;
	 return 0;
      }
      tmp=str[pos+1..i-1];
      pos=i+1;
      return tmp;
   }
   else                                              // Zahlen oder Strings
   {
      mixed tmp=regexplode(str[pos..<1],abbruch)[0]; // Ende ermitteln
      int k;
      pos+=sizeof(tmp);
      if(!deep && tmp[0..1]=="#'")                   // #'fun ?
         return ({#'call_other,previous_object(),tmp[2..<1]});
      if(to_string(k=to_int(tmp))==tmp)              // Eine Zahl?
         return k;
      if(!deep && tmp[0..0]=="'")                    // 'symbol?
         return quote(tmp[1..<1]);
      switch(tmp)
      {
         case "OBJ_TO":
           return previous_object();
	 case "OBJ_PO":
	   return OBJ_PO;
	 case "OBJ_TP":
	   return use_tp || OBJ_TP;
	 case "OBJ_OW":
	    return OBJ_OW;
	 case "OBJ_TI":
	    return OBJ_TI;
	 default:
	    return tmp;
      }
   }
   // Hierher kommt man nicht.
}

varargs mixed string_parser(string str, int tp_flag, int|symbol use_tp, mapping functions)
{
   string *parts, fun, *s, *e, element;
   int i,j,k,l, pos;
   mixed head, op, params;
   mapping m;

   if (!str)
      return 0;
   if (str=="")
      return "";

   if(!functions)
      functions = ([]);
   if(use_tp && !symbolp(use_tp))
      use_tp = 'tp;

   // Wir teilen nach $Funktion.
   parts = regexplode(str,"\\$[^\\(]*\\(");
   // Also haben wir abwechselnd "$fun(" und "par) Text"

   // Am Anfang steht reiner Text (oder "")
   if(parts[0]!="")
      head = parts[0];

   for (i = 1; i < sizeof(parts); i+=2)
   {
      fun = parts[i][1..<2]; // $ und ( abschneiden
      element = parts[i+1]; // spart staendige Array-Indizierungen
      
      if (fun == "dir" || fun == "Dir" || 
          function_exists(fun, this_object())==__FILE__[0..<3] ||
          member(functions, fun))
      {
         op = 0;
	 switch(fun)
	 {
	    case "dir" :
               if(!member(functions, "dir"))         // Das haette Vorrang
               {
	          j=strstr(element,")");             // schliessende Klammer
	          if(j<0) break;                     // suchen, ansonsten Fehler
	          j++;
	          op = 'richtung;
	          break;
               }
	    case "Dir" :
               if(!member(functions,fun))
               {
	          j=strstr(element,")");
	          if(j<0) break;
	          j++;
	          op = ({ #'capitalize, 'richtung });
	          break;
               }
	    default:
               j=0;
	       op = ({});
	       while(element[j..j]!=")")	     // Parameter parsen   
	       {
	          if(element[j..j]!=",")             // leer -> ignorieren
		     op+= ({ value_parser(element,&j,",|\\)",use_tp, 0) });
		  if(element[j..j]==",")
		     j++;
		  else if(element[j..j]!=")")
		     break;
	       }
	       if(j<0) break;
	       j++;
	       if(!sizeof(op))                       // Kein Parameter?
	       {
	          if(!tp_flag)                       // Explizit PO nehmen.
		     op=({ previous_object() });     // da die Closure meist
		  else if(use_tp)                    // durch den Closure-
		     op=({ use_tp });                // Container ausgewertet
		  else                               // wird, greift der
		     op=({ OBJ_TP });                // Defaultwert der Gram-
	       }                                     // matikfunktionen nicht.
               if(member(functions, fun))
                  op = ({#'funcall, functions[fun] }) + op;
               else
	          op = ({ symbol_function(fun) }) + op;
         }
	 if(j>=0)
	 {
	    if(head)
	       head = ({#'+,head,op});
	    else
	       head = op;
	 }
	 else // Fehler, die Funktion als String aufnehmen
	 {
	    element=parts[i]+element;
	    j=0;
	 }
	 if(sizeof(element[j..<1]))
	 {
	    if(head)
	       head = ({#'+,head,element[j..<1]});
	    else
	       head = element[j..<1];
	 }
      }
      else if((j=strstr(parts[i],"$",1))>0) // Funktion gibt es nicht.
      {                                     // Einfach nur ein einzelnes '$'?
          if(head)
              head = ({#'+, head, parts[i][0..j-1]});
          else
              head = parts[i][0..j-1];
          parts[i]=parts[i][j..<1];
          i-=2;
      }
      else if(head)                         // Dann Text einfach nur hinzufuegen.
          head = ({#'+,head,parts[i]+element});
      else
          head = parts[i]+element;
   }
   return head;
}
#endif

/*
FUNKTION: mixed_to_closure
DEKLARATION: varargs closure mixed_to_closure(closure|mixed*|string mix, symbol* symbols, int tp_flag, int|symbol use_tp, mapping functions)
BESCHREIBUNG:

Eine vollstaendige Dokumentation dazu ist /doc/funktionsweisen/pseudoclosures.

Diese Funktion verhaelt sich je nach mix unterschiedlich: 
 - Ist mix bereits eine Closure, so wird diese einfach zurueckgeliefert.
 - Ist mix ein Array, so wird angenommen, dass seine Elemente entweder
   Strings (welche dann als Pseudoclosures behandelt werden) oder
   Lambda-Ausdruecke (also die Sachen, die man lambda als 2. Parameter gibt)
   sind. Die Ergebnisclosure addiert dann im Endeffekt die Ergebnisse der
   einzelnen Array-Elemente zusammen und liefert den Gesamttext zurueck.
 - Ist mix ein String, so wird dieser als Pseudoclosure behandelt und von
   string_parser in einen Lambda-Ausdruck umgewandelt.
		  
Die Parameter tp_flag, use_tp und functions werden direkt an string_parser
weitergegeben. Der zurueckgelieferte Lambda-Ausdruck wird in eine ungebundene
Lambda-Closure mit symbols als Parameter der Closure in der angegebenen
Reihenfolge (symbols ist ein Array von Symbolen) umgewandelt und
zurueckgeliefert.
		  
VERWEISE: closure_to_string, string_parser
GRUPPEN: simul_efun, grammatik
*/
#if 1
varargs closure mixed_to_closure(closure|mixed*|string mix, symbol *symbols, int tp_flag, int|symbol use_tp, mapping functions)
{
    return ({closure})CLOSURE_CONTAINER->mixed_to_closure(mix, symbols, tp_flag, use_tp, functions);
}
#else
varargs closure mixed_to_closure(closure|mixed*|string mix, symbol *symbols, int tp_flag, int|symbol use_tp, mapping functions)
{
   int a;
   mixed ret;

   if(stringp(mix))
      ret = string_parser(mix, tp_flag, use_tp, functions);
   else if(pointerp(mix))
      for(a = sizeof(mix)-2,
          ret = (stringp(mix[<1])?string_parser(mix[<1], tp_flag, use_tp,
                  functions):mix[<1]);
          a >= 0;
          a--)
         ret = ({ #'+, stringp(mix[a])?string_parser(mix[a], tp_flag, use_tp,
             functions):mix[a], ret });
   else if(closurep(mix))
      return mix;
   else
      ret = "";

   return unbound_lambda(symbols?symbols:({}), ret);
}
#endif

/*
FUNKTION: convert_message
DEKLARATION: deprecated mixed convert_message(mixed str)
BESCHREIBUNG:

ACHTUNG: convert_message() führt keine Konvertierung mehr durch, sondern
         liefert den String im Original zurück.

Früher hat es Meldungen alten Typs in Pseudoclosures umgewandelt, in dem dort
"$Der(OBJ_TP) " vorne angehängt wurde, falls kein $ in str enthalten war.
VERWEISE: mixed_to_closure, string_parser, convert_message
GRUPPEN: simul_efun, grammatik
*/

#ifdef TestMUD
deprecated
#endif
mixed convert_message(mixed str)
{
   return str;
}

/* ============== END OF STRING_PARSER FUNCTIONS ====================== */

private string deklin_err(int err, object|mapping who, int art, int fall, string msg)
{
    object po; int i;
    while (po = previous_object(i++)) 
    { 
        if (object_name(po) != CLOSURE_CONTAINER) break; 
    } 
    if (!po) po = this_object();
    
#ifdef DEKLIN_LOG
   write_file("/log/DEKLIN",
      (this_player() ? ({string})this_player()->query_name() : " --- ")+": "+
      "err #"+err+" by "+object_name(po)+" "+
      (this_player() ? ({string})this_player()->query_command() : query_verb())+
      sprintf("\n   who: %O, art: %O, fall: %O\n", who, art, fall));
#endif
   do_error2(msg+sprintf("\n\t(who:%O)\n", who),
        object_name(po),(who && objectp(who) && object_name(who) !=
        CLOSURE_CONTAINER)? object_name(who): object_name(po),0);

   return this_interactive() && ({int})this_interactive()->query_wiz_level() ?
	  "'DeklinErr "+err+"'" : "irgend etwas";
}

// Funktion zur Rekonstruktion der urspruenglich aufgerufenen Funktion :(
// Nur fuer's Errhandling relevant.
private string deklin_function(int art, int fall)
{
   switch(art)
   {
      case ART_EIN:
	 switch(fall)
	 {
	    case FALL_GEN: return "eines()";
	    case FALL_DAT: return "einem()";
	    case FALL_AKK: return "einen()";
	    case FALL_NOM:
	    default:       return "ein()";
	 }
      case ART_DER:
	 switch(fall)
	 {
	    case FALL_GEN: return "des()";
	    case FALL_DAT: return "dem()";
	    case FALL_AKK: return "den()";
	    case FALL_NOM:
	    default:       return "der()";
	 }
      case ART_DIESER:
	 switch(fall)
	 {
	    case FALL_GEN: return "dieses()";
	    case FALL_DAT: return "diesem()";
	    case FALL_AKK: return "diesen()";
	    case FALL_NOM:
	    default:       return "dieser()";
	 }
      case ART_DEIN:
	 switch(fall)
	 {
	    case FALL_GEN: return "deines()";
	    case FALL_DAT: return "deinem()";
	    case FALL_AKK: return "deinen()";
	    case FALL_NOM:
	    default:       return "dein()";
	 }
      case ART_SEIN:
	 switch(fall)
	 {
	    case FALL_GEN: return "seines()";
	    case FALL_DAT: return "seinem()";
	    case FALL_AKK: return "seinen()";
	    case FALL_NOM:
	    default:       return "sein()";
	 }
      case ART_ER:
	 switch(fall)
	 {
	    case FALL_GEN: return "wessen()";
	    case FALL_DAT: return "ihm()";
	    case FALL_AKK: return "ihn()";
	    case FALL_NOM:
	    default:       return "er()";
	 }
      default:
	 switch(fall)
	 {
	    case FALL_GEN: return "wessen()";
	    case FALL_DAT: return "wem()";
	    case FALL_AKK: return "wen()";
	    case FALL_NOM:
	    default:       return "wer()";
	 }
   }
}


/* ============== Grammar Utility Functions ==================== */


   /* -------- Artikel/Pronomen ----------- */

/*
FUNKTION: query_pronom
DEKLARATION: varargs string query_pronom(object|mapping who, int art, int fall, object|mapping owner)
BESCHREIBUNG:
Diese Funktion liefert Artikel und Pronomen aller Art.

  who   Das Objekt dessen Pronomen oder Artikel gewuenscht ist.

	ODER eine virtuelles Objekt in der Form:
	   ([ "name" : "...", "gender" : "..." ])
	oder bei Plural
	   ([ "name" : "...", "gender" : "...", "plural": 1 ])

  art
     ART_EIN        unbest. Artikel    (ein)
     ART_DER        bestimmter Artikel (der)
     ART_DIESER     demonstrativ       (dieser)
     ART_MEIN       possesiv 1.Person  (mein)
     ART_DEIN       possesiv 2.Person  (dein)
     ART_SEIN       possesiv 3.Person  (sein)  Besitzerangabe moeglich
     ART_ICH        personal 1.Person  (ich)
     ART_DU         personal 2.Person  (du)
     ART_ER         personal 3.Person  (er)
     ART_KEIN       negiert            (kein)
     ART_JENER      \
     ART_MANCHER     > Selbsterklaerend :)
     ART_WELCHER    /

  fall
     FALL_DEF   0   Nominativ
     FALL_NOM   1   Nominativ
     FALL_GEN   2   Genitiv
     FALL_DAT   3   Dativ
     FALL_AKK   4   Akkusativ

  owner Besitzer des Objekts. Falls 0 wird es aus dem environment berechnet.
        (nur fuer art == ART_SEIN bzw. ART_NUR_SEIN verwendet)

Die Konstanten ART_..., FALL_... werden in deklin.h definiert.

Alle Parameter sind optional.
Defaultwerte sind dann
   who:      this_object()
   art:      0  (bestimmter Artikel)
   fall:     0  (Nominativ)
   owner:    0  (auto owner search: living environment object)

Die Verwendung von der, dem, den,... wird empfohlen!!! Siehe dort.
VERWEISE: query_deklin, wer, der
GRUPPEN: simul_efun, grammatik
*/

varargs string query_pronom(object|mapping who, int art, int fall, object|mapping owner)
{
   string gender_string, pronom;
   int plural, gender, flags;
   mapping | string | <string|int>* | <string|int>** menge;

   flags = art;
   art &= ART_MASK;
   
   if(!who)
      who = previous_object();
   if(objectp(who) || mappingp(who))
   {
      gender_string = QUERY("gender", who);
      if(!gender_string)
      {
        mapping|object ob = who;
	string name = QUERY("name", who);
	
	if(name)
	{
	    while(mappingp(ob)) ob=ob["environment"];
    	    do_warning2("Bei '"+QUERY("name", who)+"' wurde kein Geschlecht angegeben.\n",
		__FILE__, object_name(ob||previous_object()), __LINE__);
	}
      }
	    
      plural = QUERY("plural", who);
      menge = QUERY("menge", who);
      if(pointerp(menge))
      {
         if(art == ART_EIN)
	 {
	    if(pointerp(menge[0]))
	       menge=menge[0];
	 }
	 else if(pointerp(menge[1]))
	    menge=menge[1];
      }
      if(stringp(menge))
          menge = ({ menge, 0});
      else if(!pointerp(menge))
          menge = 0;
   }
   else
      return 0;

   if(!fall)
      fall = FALL_NOM;
   fall--;

   gender = plural ? 3 : 1 + member(genders, gender_string);

   switch(art)
   {
      case ART_EIN:
         if(menge)
	     return menge[0] + ((menge[1]&PRON_NICHT_DEKLIN)?"":
	         pronomen[PRON_UNBEST][gender][fall]);
         return "ein"+pronomen[PRON_UNBEST][gender][fall];
      case ART_DER:
         pronom = pronomen[PRON_DER][gender][fall];
	 break;
      case ART_SEIN:
         {
             object | mapping | string | <string|int>* | <string|int>** whose = QUERY("menge", owner);
	     if(!mappingp(whose))
	         whose = owner;

	     pronom = (
	         ((QUERY("plural",whose) || QUERY("gender",whose)=="weiblich") &&
	         !(!(flags&ART_VIS) && QUERY("invis",owner)&V_ATOM_INVIS))
	               ? "ihr" : "sein") +
	             pronomen[PRON_UNBEST][gender][fall];
	     break;
	 }
      case ART_MEIN:
         pronom = "mein"+pronomen[PRON_UNBEST][gender][fall];
	 break;
      case ART_KEIN:
         return "kein"+pronomen[PRON_UNBEST][gender][fall];
      case ART_DEIN:
         pronom = "dein"+pronomen[PRON_UNBEST][gender][fall];
	 break;
      case ART_ICH:
         return pronomen[PRON_1_PERS][gender][fall];
      case ART_DU:
         return pronomen[PRON_2_PERS][gender][fall];
      case ART_ER:
         return pronomen[PRON_3_PERS][gender][fall];
      case ART_DIESER:
         pronom = "dies"+pronomen[PRON_BEST][gender][fall];
	 break;
      case ART_JENER:
         pronom = "jen"+pronomen[PRON_BEST][gender][fall];
	 break;
      case ART_MANCHER:
         return "manch"+pronomen[PRON_BEST][gender][fall];
        case ART_WELCHER:
         return "welch"+pronomen[PRON_BEST][gender][fall];
      default:
         return 0;
   }
   if(!menge || (menge[1]&PRON_NICHT_NACH_BEST))
       return pronom;
   return pronom + " " + menge[0] + ((menge[1]&PRON_NICHT_DEKLIN)?"":
       pronomen[PRON_UNBEST_NACH_BEST][gender][fall]);
}


   /* -------- Adjektive ----------- */


/*
FUNKTION: query_deklin_ein_adjektiv
DEKLARATION: varargs string query_deklin_ein_adjektiv(string|<string|int>* adj, int fall, string gender_string, int plural)
BESCHREIBUNG:
Diese Funktion liefert ein dekliniertes Adjektiv.

adj
   hat die Syntax eines ELEMENTS der Adjektivliste:
      string        Grundform eines Adjektivs           "gruen", "blau",...
         ODER
      string *      Ein zweielementiges Feld:           ({"lila", "lilan"})
                    ({ grundform, unregelmaessiger_wortstamm })
         ODER
      string|int*   Ein zweielementiges Feld:           ({"rosa",
                                                          ADJ_NICHT_DEKLIN })
                    ({ grundform, art })
         ODER
      string *      Ein einelementiges Feld:            ({"blau"})

fall  (default 1)
      1            Nominativ (mit bestimmtem Artikel davor)
      2            Genitiv        "          "
      3            Dativ          "          "
      4            Akkusativ      "          "
      5            Nominativ (mit unbestimmtem Artikel davor)
      6            Genitiv        "          "
      7            Dativ          "          "
      8            Akkusativ      "          "
      9            Nominativ (ohne Artikel davor)
     10            Genitiv        "          "
     11            Dativ          "          "
     12            Akkusativ      "          "

gender_string   (default saechlich)
      "saechlich"
      "maennlich"
      "weiblich"

plural   (default 0)
      0            Singularform
      1            Pluralform

An sich braucht man diese Funktion nie direkt, da man alles auch mit
den Funltionen wer,wessen,wem,wen und dessen Spezialformen erreichen kann.

BEISPIEL:
   query_deklin_ein_adjektiv("gross", FALL_DAT, "maennlich")   --> "grossen"

VERWEISE: set_adjektiv, query_adjektiv, adjektiv,
          add_adjektiv, delete_adjektiv,
          query_deklin_adjektiv, query_deklin, wer, der
GRUPPEN: simul_efun, grammatik
*/

varargs string query_deklin_ein_adjektiv(string|<string|int>* adj, int fall, string gender_string, int plural)
{
   string stamm;
   int gender;
   int dontcheck;

   if(stringp(adj))
   {
      stamm = adj;
   }
   else if(pointerp(adj))
   {
      if(sizeof(adj) > 1 && intp(adj[1]))
      {
         if(adj[1] & ADJ_NICHT_DEKLIN)
	    return adj[0];
	else
	    stamm = adj[0];
      }
      else if(sizeof(adj) > 1 && stringp(adj[1]))
      {
         stamm = adj[1];
	 dontcheck = 1;
      }
      else if(sizeof(adj)==1 && stringp(adj[0]))
         stamm = adj[0];
   }
   else
      return 0;

   if(!dontcheck && stamm != "")
   {
      // Unveraenderlich: lila, rosa
      if(stamm[<1] == 'a')
         return stamm;
      // -e kuerzen: leise, beige
      else if(stamm[<1] == 'e')
         stamm = stamm[0..<2];
      // e kuerzen: edel, portabel
      else if(stamm[<2..<1] == "el" && member("aeioul",stamm[<3])<0)
         stamm = stamm[0..<3] + "l";
      // e kuerzen: teuer, sauer
      else if(stamm[<2..<1] == "er" && member(({"ei","eu","au"}),stamm[<4..<3])>=0)
         stamm = stamm[0..<3] + "r";
   }

   if(!fall)
      fall = FALL_NOM;

   if(fall<0 || stamm == "")
      return stamm; // Stamm abfragen

   gender = plural ? 3 : 1 + member(genders, gender_string);

   return stamm+"e"+endungen[gender][fall];
}


/*
FUNKTION: query_deklin_adjektiv
DEKLARATION: varargs string query_deklin_adjektiv(object|mapping who, string|<string|int|<string|int>*>*|int adj, int fall)
BESCHREIBUNG:
Diese Funktion liefert eine fertig deklinierte (gebeugte), komma
getrennte Liste von Adjektiven eines Objekts oder ein fremdes,
vorher uebergegebenes, dekliniertes Adjektiv.

who
   Das Objekt dessen Adjektive dekliniert werden.

adj
   hat ENTWEDER die Syntax eines ELEMENTS der Adjektivliste:
      string    Grundform eines Adjektivs  ("gruen", "blau",...)
      string *  Ein zweielementiges Feld:
                ({ grundform, unregelmaessiger_wortstamm })
		oder ein Feld aus mehreren Adjektiven:
		({ "jung", "gruen", "gut riechend" })
		(Bei 2 Adjektiven muss eines als Array 
		z.B. ({"jung"}) angegeben werden)
   ODER folgende Syntax zur AUSWAHL bestimmter Adjektive des Objekts
      int i :  i == 0 : ALLE Adjektive des Objekts
               i >  0 : die ERSTEN i Adjektive des Objekts
               i <  0 : das -i. Adjektiv des Objekts
      int *ip: ip == ({ a, b }) Alle Adjektive vom a. bis zum b.
               Ist a, b negativ oder 0 dann wird von hinten gezaehlt.
               ( 1 ist das erste Adjektiv,
                 0 ist das letzte Adjektiv,
                -1 das vorletzte,...)

fall  0,1   Nominativ
      2     Genitiv
      3     Dativ
      4     Akkusativ
      5     Nominativ unbestimmte Form
      6     Genitiv unbestimmte Form
      7     Dativ  unbestimmte Form
      8     Akkusativ unbestimmte Form

BEISPIEL:
   Das Objekt ob habe folgende Adjektivliste mit set_adjektiv definiert
   und sei maennlich.

   set_adjektiv( ({"offen", ({ "lila", "lilan" }), "voll"}) );

   Dann liefert         (wenn zweites Argument fehlt, dann Nominativ)
      query_deklin_adjektiv(ob)               "offene, lilane, volle"
      query_deklin_adjektiv(ob,0)             "offene, lilane, volle"
      query_deklin_adjektiv(ob,1)             "offene"
      query_deklin_adjektiv(ob,2)             "offene, lilane"
      query_deklin_adjektiv(ob,3,4)           "offenen, lilanen, vollen"
      query_deklin_adjektiv(ob,-1)            "offene"
      query_deklin_adjektiv(ob,-2,4)          "lilanen"
      query_deklin_adjektiv(ob,-3)            "vollen"
      query_deklin_adjektiv(ob,({1,2})        "offene, lilane"
      query_deklin_adjektiv(ob,({2,3})        "lilane, volle"
      query_deklin_adjektiv(ob,({2,2})        "lilane"
      query_deklin_adjektiv(ob,({2,0})        "lilane, volle"
      query_deklin_adjektiv(ob,({2,-1})       "lilane"

      query_deklin_adjektiv("rote",3)      "roten"
      query_deklin_adjektiv(({"rosa","rosan"}),3) "rosanen"

VERWEISE: set_adjektiv, query_adjektiv, adjektiv,
          add_adjektiv, delete_adjektiv,
          set_id, query_id, id, add_id, delete_id,
          query_deklin, wer, der
GRUPPEN: simul_efun, grammatik
*/

varargs string query_deklin_adjektiv(object|mapping who, string | <string|int|<string|int>*>* | int adj, int fall)
{
   int i, s, end;
   string res;
   string|<string|<string|int>*>* adjektiv;

   string gender;
   int plural;


   if(!who)
      who = previous_object();
   if(objectp(who) || mappingp(who))
   {
      gender = QUERY("gender", who);
      plural = QUERY("plural", who);
   }
   else
      return 0;


   /* Adjektiv explizit angegeben? */

   if(stringp(adj) ||
      pointerp(adj) && sizeof(adj)==2 && stringp(adj[0]) && (stringp(adj[1]) || intp(adj[1])))
         return query_deklin_ein_adjektiv(adj, fall, gender, plural);

   /* mehrere Adjektive explizit angegeben?*/
   
   if(pointerp(adj) &&
      (sizeof(adj) != 2 || !intp(adj[0]) || !intp(adj[1])))
   {
      adjektiv=adj;
      adj=0;
   }
   
   /* ansonsten die Adjektive des Objekts*/

   else if(!(adjektiv = QUERY("adjektiv", who)))
      return "";

   /* Ab hier Adjektivauswahl */
   if (stringp(adjektiv))
      adjektiv = ({adjektiv});

   i = 0;
   end = 0;
   s = sizeof(adjektiv);

   if(intp(adj))
   {
      if(adj < 0)    /* Ein bestimmtes Adjektiv gewaehlt? */
         return -adj <= sizeof(adjektiv) ?
            query_deklin_ein_adjektiv(adjektiv[-adj-1], fall, gender, plural) :
            0;

                     /* Alle Adjektive von 0 bis adj-1 */
      end = ( !adj ? s : (adj > s ? s : adj) )-1;
   }
   else if(pointerp(adj) && sizeof(adj) == 2 && intp(adj[0]) && intp(adj[1]))
   {
      /* Bereich gewaehlt */
      i = adj[0]-1;
      end = adj[1]-1;
   }

   if(s)
   {
      i   = (i   + s) % s;
      end = (end + s) % s;
   }

   res = "";
   for(; i <= end; i++)
   {
      res += query_deklin_ein_adjektiv(adjektiv[i], fall, gender, plural);
      if(i != end)
         res += ", ";
   }
   return res;
}



   /* -------- Substantiv/Hauptwort/Nomen ----- */


/*
FUNKTION: get_genitiv
DEKLARATION: string get_genitiv(string name)
BESCHREIBUNG:
Liefert die Genitiv-Form eines (Personal)-Namens zurueck.
Bsp: Francis  -> Francis'
     Detlef   -> Detlefs
VERWEISE: ihr, ihres, ihrem, ihren
GRUPPEN: simul_efun, grammatik
*/

string get_genitiv(string name)
{
   if(name && name != "")
   {
      if(member(genitiv_endungen, name[<1]) >= 0)
         return name+"'";
      return name+"s";
   }
   return name;
}

/*
FUNKTION: query_genitiv
DEKLARATION: string query_genitiv()
BESCHREIBUNG:
Ist diese Funktion in einem Objekt implementiert, so rufen die Grammatik-
Funktionen bei Genitiv-Generierung diese Funktion auf statt den Genitiv
generieren zu lassen.
Diese Funktion ist fuer Problemfaelle der Deklination gedacht.
VERWEISE: query_dekliniert, query_dativ, query_akkusativ, query_deklin_name
GRUPPEN: grammatik
*/

/*
FUNKTION: query_dativ
DEKLARATION: string query_dativ()
BESCHREIBUNG:
Ist diese Funktion in einem Objekt implementiert, so rufen die Grammatik-
Funktionen bei Dativ-Generierung diese Funktion auf statt den Dativ
generieren zu lassen.
Diese Funktion ist fuer Problemfaelle der Deklination gedacht.
VERWEISE: query_dekliniert, query_genitiv, query_akkusativ, query_deklin_name
GRUPPEN: grammatik
*/

/*
FUNKTION: query_akkusativ
DEKLARATION: string query_akkusativ()
BESCHREIBUNG:
Ist diese Funktion in einem Objekt implementiert, so rufen die Grammatik-
Funktionen bei Akkusativ-Generierung diese Funktion auf statt den Akkusativ
generieren zu lassen.
Diese Funktion ist fuer Problemfaelle der Deklination gedacht.
VERWEISE: query_dekliniert, query_dativ, query_genitiv, query_deklin_name
GRUPPEN: grammatik
*/

/*
FUNKTION: query_real_owner
DEKLARATION: object query_real_owner()
BESCHREIBUNG:
Diese Funktion wird aufgerufen, um den wahren Eigentuemer einer Sache
zu ermitteln. Ist dieser Eigentuemer ein anderer als der ermittelte
(d.h. der beim Parameter 'owner' der jeweiligen Grammatikfunktionen
uebergebene oder der, der das Objekt bei sich traegt), so
wird beim Aufruf von dein(), sein() oder mein() automatisch
der Eigentuemer wie bei ihr() angegeben.
VERWEISE: query_deklin, query_dekliniert, dein
GRUPPEN: grammatik
*/

/*
FUNKTION: query_deklin_name
DEKLARATION: string query_deklin_name(object|mapping who, int fall)
BESCHREIBUNG:
Diese Funktion liefert ein dekliniertes Nomen.

who       Das Objekt dessen Namen gewuenscht ist

fall (default ist 1)
      1            Nominativ (mit bestimmtem Artikel davor)
      2            Genitiv        "          "
      3            Dativ          "          "
      4            Akkusativ      "          "
      5            Nominativ (mit unbestimmtem Artikel davor)
      6            Genitiv        "          "
      7            Dativ          "          "
      8            Akkusativ      "          "
      9            Nominativ (ohne Artikel davor)
     10            Genitiv        "          "
     11            Dativ          "          "
     12            Akkusativ      "          "

Im Normalfalle liefert diese Funktion einfach who->query_cap_name(),
welches bei Genitiv gegebenenfalls ein "es", "s" oder "'" angehaengt
bekommt.
Man kann aber Ausnahmen definieren, indem man entweder eine Funktion
query_dekliniert  oder Funktionen query_genitiv, query_dativ und
query_akkusativ im Objekt implementiert.

VERWEISE: query_deklin, query_dekliniert, wer, der,
          query_genitiv, query_dativ, query_akkusativ
GRUPPEN: simul_efun, grammatik
*/

/*
FUNKTION: query_dekliniert
DEKLARATION: string query_dekliniert(int fall, object interessent, int flags)
BESCHREIBUNG:
Die Grammatik-Funktionen rufen bei der Deklination des Namens eines
Objektes in dem Objekt diese Funktion auf.

Ist der Rueckgabewert ungleich 0, so wird dieser Wert als deklinierte Form
des Namens verwendet; ist der Rueckgabewert 0, so wird auf herkoemmlichem
Wege versucht, den Namen zu deklinieren.

Diese Funktion hat Vorrang vor den Einzelfall-Funktionen query_genitiv(),
query_dativ und query_akkusativ.

Die Defines fuer fall stehen in /sys/deklin.h (sind 1-4 fuer
Nominativ-Akkusativ), das Objekt interessent ist das Objekt, welches die
Grammatikfunktion aufgerufen hat, die den Namen dekliniert haben will.

flags gibt an, in welchem Kontext der Name verwendet werden soll:
    DEKL_BESTIMMT       Dem Namen wird ein bestimmter Artikel vorangestellt
    DEKL_UNBESTIMMT     Dem Namen wird ein unbestimmter Artikel vorangestellt
    DEKL_BLANK          Dem Namen wird kein Artikel vorangestellt
Obige Defines sind keine Bitflags, d.h. eine Abfrage sollte aehnlich wie
(flags&DEKL_ART_MASK)==DEKL_BESTIMMT aussehen.

Diese Funktion ist fuer Problemfaelle der Deklination gedacht; im
Allgemeinen duerften die Einzelfall-Funktionen ausreichen.

Bei V-Items mit einer Closure als Eintrag fuer "dekliniert" wird als
1. Parameter das V-Item selber uebergeben und danach die Parameter
wie bei query_dekliniert.

BEISPIELE:

    string query_dekliniert(int fall, object interessent, int flags)
    {
        return
            ({ "Atlas",
               "Atlasses",
               "Atlasse",
               "Atlas",
            }) [fall-1];
    }

// Fuer adjektivaehnliche Namen (z.B. Reisender) kann man auf
// Grammatikfunktionen zurueckgreifen:
// (DEKL_2_ADJ_ART uebernimmt hierbei die Umrechung des Flags in den
// 2. Parameter von query_deklin. Es entspricht einem ART_NUR_ADJEKTIV
// mit ART_DER, ART_EIN bzw. ART_BLANK.)

    string query_dekliniert(int fall, object interessent, int flags)
    {
	return query_deklin(
	    ([
		"adjektiv": ({"Reisend"}),
		"gender":"maennlich"
	    ]), DEKL_2_ADJ_ART(flags), fall);
    }

VERWEISE: query_genitiv, query_dativ, query_akkusativ, query_deklin_name,
          query_dekliniert
GRUPPEN: grammatik
*/


string query_deklin_name(object|mapping who, int fall)
{
   string name, anfang, ende, gender;
   string tmp;
   int pos, personal;

   if (!(name = QUERY("cap_name",who)))
   {
      if(!(name = QUERY("name",who)))
        return 0;
      else
        name = capitalize(name);
   }
   if ((pos = strstr(name, " ")) != -1 || (pos=strstr(name, ",")) != -1)
   {
      anfang = name[0 .. pos - 1];
      ende = name[pos .. <1];
   }
   else {
      anfang = name;
      ende = "";
   }
   if(!fall)
       fall = 1;
   if (mappingp(who))
       tmp = funcall(who["dekliniert"], who, (fall&3)||4, previous_object(),
           ((fall-1)>>2)&DEKL_ART_MASK);
   else if (objectp(who))
       tmp = ({string})who->query_dekliniert( (fall&3)||4, previous_object(),
           ((fall-1)>>2)&DEKL_ART_MASK);
   if (tmp)
       return tmp;
   switch((fall&3)||4)
   {
      case FALL_NOM:
         return name;

      case FALL_DAT:
         if (tmp = QUERY("dativ",who))
             return capitalize(tmp);

         if (QUERY("plural", who))
         {
             if (member(dativ_plural_endungen, anfang[<1]) < 0)
                 return anfang + "n" + ende;
             return name;
         }

         // Da im Akkusativ-Fall auch QUERY("akkusativ") gecheckt wird,
         // kann kein fall-through mehr gemacht werden:
         if (QUERY("gender",who) == "maennlich" && !QUERY("personal",who))
         {
             // Rabe, Knabe, Alte, Kaese,...:
             if (anfang[<1] == 'e' && member("aeiou",anfang[<2]) < 0)
                 if (anfang[<4..<2] == "aes" || anfang[<3..<2] == "äs")
                     return name;
                 else
                     return anfang + "n" + ende;
             // Basilisk, Christ, Pazifist, Exorzist...:
             if (member(({"ist","isk"}),anfang[<3..]) != -1 &&
                 member("aeiou",anfang[<4]) == -1)
                 return anfang + "en" + ende;
         }
         return name;

      case FALL_AKK:
         if (tmp = QUERY("akkusativ",who))
             return capitalize(tmp);

         if (QUERY("plural",who))
             return name;

         if (QUERY("gender",who) == "maennlich" && !QUERY("personal",who))
         {
             // Rabe, Knabe, Alte, Kaese,...:
             if (anfang[<1] == 'e' && member("aeiou",anfang[<2]) < 0)
                 if (anfang[<4..<2] == "aes" || anfang[<3..<2] == "äs")
                     return name;
                 else
                     return anfang + "n" + ende;
             // Basilisk, Christ, Pazifist, Exorzist...:
             if (member(({"ist","isk"}),anfang[<3..]) != -1 &&
                 member("aeiou",anfang[<4]) == -1)
                 return anfang + "en" + ende;
         }
         return name;

      case FALL_GEN:
         if (tmp = QUERY("genitiv",who))
             return capitalize(tmp);

         if ((gender = QUERY("gender", who)) == "weiblich" ||
             QUERY("plural", who))
                 return name;

         if (!personal = QUERY("personal",who))
	 {
	    if(gender == "maennlich")
	    {
        	// Basilisk, Christ, Pazifist, Exorzist...:
        	if (member(({"ist","isk"}),anfang[<3..]) != -1 &&
            	    member("aeiou",anfang[<4]) == -1)
            	    return anfang + "en" + ende;

        	// Rabe, Knabe, Alte, Kaese,...:
        	if (anfang[<1]=='e' && member("aeiou",anfang[<2]) < 0)
            	    if (anfang[<2] == 's')
                	return anfang + "s" + ende;
            	    else
                	return anfang + "n" + ende;
	    }
	    if(gender != "weiblich")
	    {
		if(anfang[<2..<1] == "ss")
		    return anfang + "es" + ende;
	    }
	    
	    // maennlich: Glanz, Platz, Satz, Sturz, Witz, Reiz
	    // saechlich: Gesetz, Holz, Kreuz
	    if (anfang[<1]=='z')
		return anfang + "es" + ende;
	 }
	 // Eigennamen mit Artikel davor sind im Genitiv endungslos.
	 else if(fall != 10)
	    return name;
	 
         if (member(genitiv_endungen, anfang[<1]) >= 0)
             if (personal)
                 return anfang + "'" + ende;
             else
                 return name;

         return anfang + "s" + ende;
   }
}


/* ================= deklin kernel functions =================== */

/* parse_deklin_object expandiert Abkuerzungen fuer who und owner
 * in query_deklin und query_deklin_owner Aufrufen.
 */

private object|mapping parse_deklin_object(object|mapping|int who)
{
   if(intp(who))
      if(who < 0)
         who = previous_object(1-who);
      else switch(who)
      {
         case OBJ_TO : /* 0 */
            who = previous_object();  break;
         case OBJ_PO : /* 1 */
            who = previous_object(1); break;
         case OBJ_TP : /* 2 */
            who = this_player();      break;
         case OBJ_OW : /* 3 */
            who = auto_owner_search(previous_object());
            break;
         case OBJ_TI : /* 4 */
            who = ({object})funcall(symbol_function("this_interactive")); break;
         default:
            who = previous_object();
      }
   return who;
}


/*
FUNKTION: query_deklin
DEKLARATION: varargs string query_deklin(object|mapping|int who, int art, int fall, string | <string|int|<string|int>*>* | int adjektiv, object|mapping|int owner)
BESCHREIBUNG:
Liefert deklinierte Form eines Objektnamens. Dabei wird beruecksichtigt,
ob das Objekt ein personal_name hat (query_personal()==1).

  who   Das Objekt dessen deklinierte Form gewuenscht ist.

	ODER eine virtuelles Objekt in der Form:
	   ([ "name" : "...", "gender" : "..." ])
	oder bei Plural
	   ([ "name" : "...", "gender" : "...", "plural": 1 ])

        ODER eine Zahl:
           OBJ_TO  0   this_object()  (default)
           OBJ_PO  1   previous_object()
           OBJ_TP  2   this_player()
           OBJ_OW  3   auto_owner_search  (sonst this_player())
           OBJ_TI  4   this_interactive()


  art
     ART_EIN            unbest. Artikel    (ein) + Adjektive + Nomen
     ART_DER            bestimmter Artikel (der) + Adjektive + Nomen
     ART_DIESER         demonstrativ       (dieser)  + Adjektive + Nomen
     ART_MEIN           possesiv 1.Person  (mein) + Adjektive + Nomen
     ART_DEIN           possesiv 2.Person  (dein) + Adjektive + Nomen
     ART_SEIN           possesiv 3.Person  (sein) + Adjektive + Nomen
     ART_ICH            personal 1.Person  (ich)
     ART_DU             personal 2.Person  (du)
     ART_ER             personal 3.Person  (er)
     ART_KEIN           negiert            (kein)
     ART_JENER      \
     ART_MANCHER     > Selbsterklaerend :)
     ART_WELCHER    /

     ART_KEINS          Kein Artikel, nur die Adjektive und das Nomen.
     ART_KEINS_BEST     Kein Artikel, nur die Adjektive und das Nomen.
     ART_KEINS_UNBEST   Kein Artikel, nur Adjketive(unbestimmte Form) und
                        das Nomen.
     ART_NUR_DER        nur bestimmter Artikel   (keine Adj., kein Nomen)
			(ART_DER | ART_NO_ADJEKTIV | ART_NO_NOMEN)
     ART_NUR_EIN        nur unbest. Artikel      (keine Adj., kein Nomen)
			(ART_EIN | ART_NO_ADJEKTIV | ART_NO_NOMEN)
     ART_NUR_DEIN       nur Possesivpronomen 2.P.(keine Adj., kein Nomen)
     ART_NUR_SEIN       nur Possesivpronomen 3.P.(keine Adj., kein Nomen)
     ART_NUR_DIESER     nur demonstr. Artikel (keine Adj., kein Nomen)
     ART_NUR_KEIN       (ART_KEIN | ART_NO_ADJEKTIV | ART_NO_NOMEN)

     ART_AAA            Automatische Artikelauswahl (ein/der) + Adj + Nomen
     ART_NUR_AAA        Nur Artikel nach automatischer Auswahl (k.Adj.,k.Nomen)
                        (Je nach Anzahl gleicher Objekte in der Umgebung wird
                         ein oder der benutzt.)
     ART_CAPITALIZE     Im Falle eines Pronoms wird jenes grossgeschrieben,
                        im Falle einer pronomenlosen Konstruktion mit
                        Adjektiven wird das erste Adjektiv gross geschrieben.

  fall
     FALL_DEF   0   Nominativ
     FALL_NOM   1   Nominativ
     FALL_GEN   2   Genitiv
     FALL_DAT   3   Dativ
     FALL_AKK   4   Akkusativ


  adjektiv
     string  ->  Grundform eines Adjektivs
                 ""         KEINE Adjektive
     string* ->  ({ unregelm.Grundform, Deklinationsstamm }) eines Adjektivs
                 ({ adjektiv1, adjektiv2, ...}) mehrere Adjektive
     int i   ->  i == 0   ALLE Adjektive des Objekts who.
                 i >  0   Die ersten i Adjektive des Objekts who.
                 i <  0   Das -i. te Adjektiv des Objekts who.
     int*    ->  ({ a, b }) Die a bis b. Adjektive des Objekts who.
                 ({})       KEINE Adjektive


  owner Besitzer des Objekts. Falls 0 wird es aus dem environment berechnet.
        (nur fuer art == ART_SEIN bzw. ART_NUR_SEIN verwendet)

Die Konstanten ART_..., FALL_..., OBJ_... werden in deklin.h definiert.

Mit Hilfe von adjektiv koennen alle oder bestimmte Adjektive des Objekts
oder ein fremdes Adjektiv in den Ergebnisstring zwischen Artikel/Pronom und
Substantiv gestellt werden.
(Adjektive eines Objekts setzt man mit set_adjektiv, siehe dort.)

Alle Parameter sind optional.
Defaultwerte sind dann
   who:      this_object()
   art:      0  (bestimmter Artikel)
   fall:     0  (Nominativ)
   adjektiv: 0  (alle Adjektive des Objekts who)
   owner:    0  (auto owner search: living environment object)

BEISPIELE:
   who sei eine Person namens Anton
     query_deklin(who);                   liefert: "Anton"
   who sei ein Ork
     query_deklin(who,ART_EIN,3,"boesen");      liefert: "einem boesen Ork"
   what sei ein Auto und who eine weibliche Person
     query_deklin(what,ART_SEIN,4,0,who);        liefert: "ihr Auto"


query_deklin verwendet:
   auto_owner_search
   parse_deklin_object

   query_pronom
   query_deklin_adjektiv
   query_deklin_name

   query_personal
   query_plural
   query_eigen

Die Verwendung von der, dem, den, ... anstelle von query_deklin
wird empfohlen!!! Siehe dort.
VERWEISE: wer, der, ihr, query_pronom, set_adjektiv
GRUPPEN: simul_efun, grammatik
*/

varargs string query_deklin(object|mapping|int who, int art, int fall, string | <string|int|<string|int>*>* | int adjektiv, object|mapping|int owner)
{
   string satzteil, adj, pronom, title;
   string name;
   object|mapping virt_env;
   mapping | string | <string|int>* | <string|int>** menge;
   int is_pers_pron, is_pers_name, only_pron;
   int flags, oart, append_personal, efall;

#ifdef DEKLIN_DEBUG
   printf("query_deklin:: who:%O, art:%O, fall:%O, adjektiv:%O, owner:%O\n",
          who, art, fall, adjektiv, owner);
#endif
   if(!intp(art))
      return deklin_err(DEKLIN_ERR_ILLEGAL_ART,
	 who, art, fall,
	 "Bad type of argument 'art' to "+deklin_function(art, fall)+
	 " (must be int)");
   if(objectp(adjektiv))
      return deklin_err(DEKLIN_ERR_ILLEGAL_ADJ,
	 who, art, fall,
	 "Bad argument 'adjektiv' to "+deklin_function(art, fall)+
	 " (is object, must be adj)");

   /*  ==NEWSFLASH:   D O N ' T     P A N I C ! ! !== */

   /* --Objektumwandlungen-- */

   who = parse_deklin_object(who); /* Abkuerzungen expandieren */

   if(owner)  /* Wenn nicht auto_owner, dann Abkuerzungen expandieren */
      owner = parse_deklin_object(owner);

   if(!objectp(who) && !mappingp(who))
      return deklin_err(DEKLIN_ERR_ILLEGAL_WHO,
	 who, art, fall,
	 "Bad argument 'who' to "+deklin_function(art, fall)+
	 " (must be object or virtual object)");

   /* --Fallumwandlungen-- */
   
   if(!fall)                   /* Defaultfall: Nominativ */
      fall = FALL_NOM;

   /* --Artumwandlungen-- */

   flags = art & ~ART_MASK;
   art &= ART_MASK;


   is_pers_pron =         /* Ist ein Personalpronomen gewuenscht? */
      art == ART_ER || art == ART_DU || art == ART_ICH;     /* (er/du) */

   if(is_pers_pron)       // Die Art des Artikels nicht mehr aendern, da es
      flags &= ~ART_AUTO; // von nun an ein Personalpronomen bleiben muss.
      
   if(!is_pers_pron && !(flags & ART_NO_NOMEN) &&
      mappingp(who) && (mappingp(virt_env = who["environment"]) ||
      virt_env && !(objectp(virt_env) &&
         (living(virt_env) || !({string})virt_env->query_name() || !({string})virt_env->query_gender()))))
   {
      oart = (art == ART_BLANK ? ART_AUTO : art) | flags;
      art = ART_DER;
      oart &= ~(ART_CAPITALIZE|ART_NO_PRONOM);
      if(flags & ART_NO_NOMEN)
         oart = 0;
      else if(append_personal = (objectp(virt_env) &&
	 ({int})virt_env->query_personal() &&
	 !sizeof(({mixed*})virt_env->query_adjektiv())))
	 flags |= ART_NO_PRONOM;
   }

   if(!art)
      art = ART_DER;

   if((flags & ART_INVIS ||
      !(flags & ART_VIS) && (QUERY("invis", who) & V_ATOM_INVIS))
         &&
      !(art & ART_NO_NOMEN))  // Bei art: ***_NUR_*** nicht jemand setzen
	 return livingp(who) ? "Jemand"+jemand[fall-1] : "Etwas";

   if(art == ART_SEIN || art == ART_DEIN || art == ART_MEIN)
   {
      object r_owner = QUERY("real_owner", who);
      
      if(!owner)
         owner = auto_owner_search(who);
      
      if(r_owner && r_owner!=owner)
         return ({string})this_object()->query_deklin_owner(
	    who, flags&ART_DER, fall, adjektiv, r_owner);

      if(!owner)
         flags |= ART_AUTO;
   }

   if(flags & ART_AUTO)   /* Automatische Artikel Auswahl */
      art = not_alone(who) ? ART_EIN : ART_DER;

   if(art == ART_EIN &&
      (QUERY("eigen",who) || QUERY("personal",who)))
      art = ART_DER;

   /* --Mengenbehandlung-- */

   menge = QUERY("menge",who);

   if(art == ART_EIN && QUERY("plural", who) && !menge)
   {
      art = ART_BLANK;
      flags |= ART_NO_PRONOM;
   }

   satzteil = "";
   
   if(mappingp(menge) && !(flags&ART_NO_NOMEN))
   {
   /* Auszug aus dem Duden (alte Rechtschreibung):
      K 312: Steht das der Mass- oder Mengenangabe folgende Substantiv
             im  Singular und ohne ein Attribut, so hat es in der Regel
	     keine Deklinationsendung.
	     Steht das Substantiv im Plural oder mit einem Attribut,
	     so tritt es gewoehnlich in den gleichen Kasus wie die
	     Massbestimmung.
   */
      pronom = query_deklin(menge, art|(flags&ART_CAPITALIZE), fall, 0, owner);
      if(is_pers_pron)
         return pronom;
      art = ART_BLANK;
      flags = (flags | ART_NO_PRONOM) &~ART_CAPITALIZE;
   // Nach dem Adjektiv muessen wir gegebenenfalls den Fall auf
   // Nominativ setzen.
   }

   /* --Artumwandlungen enden hier-- */

   only_pron = flags & ART_NO_ADJEKTIV &&
	       flags & ART_NO_NOMEN &&
               !(flags & ART_NO_PRONOM);

   /* --Adjektivlogik-- */
   efall = ((art == ART_BLANK) || (art == ART_EIN && QUERY("plural", who)))
         ? fall + 8
	 : (art == ART_DER || art == ART_DIESER || art == ART_JENER || art == ART_MANCHER || art == ART_WELCHER
	 ? fall
	 : fall + 4);

   adj = is_pers_pron ||
	 only_pron ||
         stringp(adjektiv) && adjektiv == "" ||
         pointerp(adjektiv) && !sizeof(adjektiv) ||
	 flags & ART_NO_ADJEKTIV ?
      0 :
      query_deklin_adjektiv(who, adjektiv, efall);
   adj = adj == "" ? 0 : adj;

   if(!adj && sizeof(pronom) && !QUERY("plural",who))
   {
      // Mapping-Menge, siehe oben.
      efall -= fall-FALL_NOM;
      fall = FALL_NOM;
   }
      
   /* --Titellogik-- */

   is_pers_name = QUERY("personal", who);
   title = QUERY("personal_title", who);
   
   /* --Artikel/Pronomenlogik-- */


   /* Soll ein Pronom/Artikel vorangestellt werden?
      1  Personalnames bekommen keine Artikel,    Garthan
         (es sei denn, sie haben Adjektive)       der deklinierte Garthan
         wohl aber alle anderen Pronomina.        dieser Garthan
      2  wenn explizit kein Artikel/Pronom
         gewuenscht, dann gibt's auch keinen      bunter Hund
    */
   if((!is_pers_name || adj || !(art == ART_DER || art == ART_EIN)) && // 1
       !(flags & ART_NO_PRONOM)                                        // 2
         ||
       only_pron)
   {
      /* Pronom/Artikel nur holen wenn benoetigt */
      pronom = query_pronom(who, art|(flags&~ART_MASK), fall, owner);
      if (pronom && !(virt_env && oart && append_personal) && (flags & ART_CAPITALIZE))
         pronom = capitalize (pronom);
   }
   else
      efall = 8 + ((efall & 3)||4);

   /* --Nomenlogik-- */

   if(!(flags & ART_NO_NOMEN))   /* NAME holen */
   {
      if(!(name = query_deklin_name(who, efall)))
	 return deklin_err(DEKLIN_ERR_MISSING_NAME,
	    who, art, fall,
	    "Object passed to "+deklin_function(art, fall)+
	    " has no name");
      // Wenn Geschlecht weiblich, Singular, Fall=Genitiv, kein Pronomen
      // kommt, der Name ein einzelnes Wort ist: dann "s" anhaengen.
      if (fall==FALL_GEN && !pronom && !(flags & ART_NO_NOMEN) && 
          QUERY("gender",who)=="weiblich" && !QUERY("plural",who) &&
	  QUERY("personal",who) && strstr(name," ")<0 &&
	  member(genitiv_endungen, name[<1]) < 0)
             name += "s";
      if(title)
         name = title + " " + name;
   }


   /*  ==NEWSFLASH:   K E E P   C O O L  , it's nearly over.== */

   /* --Satzteil Zusammenbau-- */

   if(pronom)
      satzteil += pronom;
   if(!is_pers_pron)
   {
      if(pronom && (name || adj))
         satzteil += " ";
      if(adj)
      {
         if (!pronom && (flags & ART_CAPITALIZE))
            satzteil += capitalize (adj);
         else
            satzteil += adj;
         if(name)
            satzteil += " ";
      }
      if(name)
         satzteil += name;
   }

   /* Eventuell rekursiv ergaenzen */
   if(oart && !QUERY("noowner", who))
   {
      string prep;
      
      if(append_personal)
	 return get_genitiv(({string})virt_env->query_cap_name())+" "+ satzteil;
      else if(prep = QUERY("prep",who))
	 return satzteil+" "+prep+" "+
		query_deklin(virt_env, oart, FALL_DAT);
      else 
	 return satzteil+" "+
		query_deklin(virt_env, oart, FALL_GEN);
   }
   else
      return satzteil;

   /*  ==NEWSFLASH:   T H A T ' S  I T   , you've got the message? == */
}


/*
FUNKTION: query_deklin_owner
DEKLARATION: varargs string query_deklin_owner(object|mapping|int who, int art, int fall, string|<string|int|<string|int>*>*|int adjektiv, <object|mapping|int> owner, int oart, string|<string|int|<string|int>*>*|int oadjektiv)
BESCHREIBUNG:
Liefert die deklinierte Form eines Objektnamens. Dabei wird zusaetzlich
der Besitzer des Objektes mit ausgegeben. Wird der Besitzer nicht
explizit angegeben, so wird das naechst umgebende lebende Objekt
herangezogen.

Das Objekt selbst wird durch die Parameter: who, art, fall und adjektiv
beschrieben. Diese Parameter werden so an query_deklin() weitergegeben.
Siehe dort fuer die genaue Beschreibung der Parameter.

Der Besitzer wird durch die Parameter owner, oart und oadjektiv
beschrieben. query_deklin() wird mit diesen und FALL_GEN aufgerufen.

VERWEISE: query_deklin, ihr
GRUPPEN: simul_efun, grammatik
*/

/*
 * query_deklin_owner macht die Dreckarbeit fuer die Funktionen
 * ihr, ihren, ihrem.
 * Funktioniert aehnlich wie query_deklin. Frag Garthan...
 */

varargs string query_deklin_owner(
   <object|mapping|int> who,   int art, int fall, string | <string|int|<string|int>*>* | int adjektiv,
   <object|mapping|int> owner, int oart,          string | <string|int|<string|int>*>* | int oadjektiv)
{
   string owner_part;
   object|mapping virt_env;

   who = parse_deklin_object(who); /* Aus eventuellen Abk. Objekt machen */
   if(!objectp(who) && !objectp(owner) && !mappingp(who))
      return deklin_err(DEKLIN_ERR_2_AUTO_OWNER_WITH_VIRT_OBJ,
	 who, art, fall,
	 "Call to ihr/es/em/en() with virtual object and without explicit owner"
	 );
   if(!owner)
      owner = auto_owner_search(who);
   if(!owner) { /* ihr ohne Besitzer */
      if (art & ART_CAPITALIZE)
         return query_deklin(who, ART_AAA | ART_CAPITALIZE, fall, adjektiv);
      else
         return query_deklin(who, ART_AAA, fall, adjektiv);
   }
   
   owner = parse_deklin_object(owner); /* Aus eventuellen Abk. Objekt machen */

   if(mappingp(who) &&
      (mappingp(virt_env = who["environment"]) ||
       virt_env && !(objectp(virt_env) &&
           (living(virt_env) || !({string})virt_env->query_gender() || !({string})virt_env->query_name()))))
   {
      object|mapping virt_env_env = mappingp(virt_env) ? virt_env["environment"] : environment(virt_env);
      string prep;
      
      if(objectp(virt_env_env) &&
          (!({string})virt_env_env->query_name() || !({string})virt_env_env->query_gender()))
	      virt_env_env = 0;
	      
      if(prep = QUERY("prep",who))
	 return query_deklin(who, art, fall, adjektiv)+" "+
		prep+" "+
		query_deklin_owner(virt_env, ART_DER, FALL_DAT,0,
		    virt_env_env, oart, FALL_GEN, oadjektiv);
      else
	 return query_deklin(who, art, fall, adjektiv)+" "+
		query_deklin_owner(virt_env, ART_DER, FALL_GEN,0,
		    virt_env_env, oart, FALL_GEN, oadjektiv);
   }
   // TODO: Diese Berechnung wird eigentlich nur fuer die if-Abfrage
   // selber und den else-Zweig benoetigt. Die if-Abfrage sollte
   // man umstellen, dass sie das nicht mehr braucht.
   owner_part = query_deklin(owner, oart, FALL_GEN, oadjektiv);

   if(objectp(owner) &&
      ({int})owner->query_personal() &&
      // Sehr heuristisch, aber schneller...
      //	 eigentlich sollte man hier testen, ob kein artikel/adjektiv
      //	 da ist...
      (({string})owner->query_personal_title() ||
      sizeof(owner_part) > 0 && sizeof(explode(owner_part," ")) == 1)
      )
   {
      owner_part = query_deklin(owner, oart|(art&ART_CAPITALIZE), FALL_GEN, oadjektiv);
      if(({string})owner->query_gender() == "weiblich")
      {
         if(member(genitiv_endungen, owner_part[<1]) < 0)
	    owner_part += "s";
	 else if(member(genitiv_endungen, (QUERY("cap_name",owner)||QUERY("name",owner))[<1])>=0)
	    owner_part += "'";
      }
      return
	 owner_part
	 +" "+
	 query_deklin(who, ART_KEINS | (art&~(ART_MASK|ART_CAPITALIZE|ART_AUTO)), fall, adjektiv);
   }
   else                                          /* unpersoenlicher Besitzer */
      return query_deklin(who, art,              fall, adjektiv)
	     +" "+
	     owner_part;
}


/* ===== Zwischenschicht zwischen query_deklin und user ======== */


/*
FUNKTION: wer
DEKLARATION: varargs string wer(object|mapping|int who, int art, string|<string|int|<string|int>*>*|int adjektiv, object|mapping|int owner)
BESCHREIBUNG:
Liefert im Nominativ deklinierte Form eines Objektnamens.
Dabei wird beruecksichtigt, ob das Objekt ein personal_name hat
(query_personal()==1).

  who   Das Objekt dessen deklinierte Form im Nominativ gewuenscht ist.

	ODER eine virtuelles Objekt in der Form:
	   ([ "name" : "...", "gender" : "..." ])
	oder bei Plural
	   ([ "name" : "...", "gender" : "...", "plural": 1 ])

        ODER eine Zahl:
           OBJ_TO  0   this_object()  (default)
           OBJ_PO  1   previous_object()
           OBJ_TP  2   this_player()
           OBJ_OW  3   auto_owner_search  (sonst this_player())
           OBJ_TI  4   this_interactive()

  art
     ART_EIN            unbest. Artikel    (ein) + Adjektive + Nomen
     ART_DER            bestimmter Artikel (der) + Adjektive + Nomen
     ART_DIESER         demonstrativ       (dieser)  + Adjektive + Nomen
     ART_MEIN           possesiv 1.Person  (mein) + Adjektive + Nomen
     ART_DEIN           possesiv 2.Person  (dein) + Adjektive + Nomen
     ART_SEIN           possesiv 3.Person  (sein) + Adjektive + Nomen
     ART_ICH            personal 1.Person  (ich)
     ART_DU             personal 2.Person  (du)
     ART_ER             personal 3.Person  (er)
     ART_KEIN           negiert            (kein)
     ART_JENER      \
     ART_MANCHER     > Selbsterklaerend :)
     ART_WELCHER    /
     
     ART_VIS            Zeigt die sichtbare Deklination, unabhaengig,
                        ob das Objekt/Lebewsen sichtbar ist oder nicht.
     ART_INVIS          Zeigt die unsichtbare Deklination.

     ART_KEINS          Kein Artikel, nur die Adjektive und das Nomen.
     ART_KEINS_BEST     Kein Artikel, nur die Adjektive und das Nomen.
     ART_KEINS_UNBEST   Kein Artikel, nur Adjketive(unbestimmte Form) und
                        das Nomen.
     ART_NUR_EIN        nur unbest. Artikel      (keine Adj., kein Nomen)
     ART_NUR_DER        nur bestimmter Artikel   (keine Adj., kein Nomen)
     ART_NUR_DIESER     nur demonstr. Artikel (keine Adj., kein Nomen)
     ART_NUR_MEIN       nur Possesivpronomen 1.P.(keine Adj., kein Nomen)
     ART_NUR_DEIN       nur Possesivpronomen 2.P.(keine Adj., kein Nomen)
     ART_NUR_SEIN       nur Possesivpronomen 3.P.(keine Adj., kein Nomen)
     ART_NUR_KEIN       nur Negationspronomen kein (keine Adj., kein Nomen)
     ART_CAPITALIZE     Im Fall eines Pronoms wird dieses grossgeschrieben
                        im Falle einer pronomenlosen Konstruktion mit
                        Adjektiven wird das erste Adjektiv gross geschrieben.

     ART_AAA            Automatische Artikelauswahl (ein/der) + Adj + Nomen
     ART_NUR_AAA        Nur Artikel nach automatischer Auswahl (k.Adj.,k.Nomen)
                        (Je nach Anzahl gleicher Objekte in der Umgebung wird
                         ein oder der benutzt.)

  adjektiv
     string  ->  Grundform eines Adjektivs
                 ""         KEINE Adjektive
     string* ->  ({ unregelm.Grundform, Deklinationsstamm }) eines Adjektivs
                 ({ adjektiv1, adjektiv2, ...}) mehrere Adjektive
     int i   ->  i == 0     ALLE Adjektive des Objekts who.
                 i >  0     Die ersten i Adjektive des Objekts who.
                 i <  0     Das -i. te Adjektiv des Objekts who.
     int*    ->  ({ a, b }) Die a bis b. Adjektive des Objekts who.
                 ({})       KEINE Adjektive

  owner Besitzer des Objekts. Falls 0 wird es aus dem environment berechnet.
        (nur fuer art == ART_SEIN bzw ART_NUR_SEIN verwendet)

Die Konstanten ART_..., FALL_..., OBJ_... werden in deklin.h definiert.

Mit Hilfe von adjektiv koennen alle oder bestimmte Adjektive des Objekts
oder ein fremdes Adjektiv in den Ergebnisstring zwischen Artikel/Pronom und
Substantiv gestellt werden.
(Adjektive eines Objekts setzt man mit set_adjektiv, siehe dort.)

Alle Parameter sind optional.
Defaultwerte sind dann
   who:      this_object()
   art:      0  (bestimmter Artikel)
   adjektiv: 0  (alle Adjektive des Objekts who)
   owner:    0  (auto owner search: living environment object)


Beispiele fuer wer, wem, wen
   who  sei eine Person namens Anton
   ork  sei ein Ork ohne Namen

     Wer(who)+" zieht "+wem(ork,ART_EIN)+" einen Pullover an."
        liefert:
     "Anton zieht einem Ork einen Pullover an."

     Wer(ork,ART_DIESER,"boese")+
         " laesst sich das von "+wem(who)+" nicht gefallen."
        liefert:
     "Dieser boese Ork laesst sich das von Anton nicht gefallen."

     Wer(ork)+" haette "+wen(who,ART_ER)+" am liebsten davongejagt."
        liefert:
     "Der Ork haette ihn am liebsten davongejagt."

     Wer(ork)+" patscht "+wem(who)+" auf "+
     wen((["name"  :"finger",
	   "gender":"maennlich",
	   "plural":1]), ART_SEIN, 0, who)+"."
        liefert:
     "Der Ork patscht Anton auf seine Finger."


wer, wem, wen sind Spezialfaelle von query_deklin.

ANSTELLE VON wer, wen, wem SOLLTEN DIE BEFEHLE:
  der, dem, den, ein, einem, einen, dein, deinem, deinen,
  sein, seinem, seinen, dieser, diesem, diesen, er, ihm, ihn, ihr,
  Der, Dem, Den, Ein, Ihr, ...
BENUTZT WERDEN!!!!!  (siehe dort)
VERWEISE: der, query_pronom, query_deklin, set_adjektiv
GRUPPEN: simul_efun, grammatik
*/
varargs string wer(object|mapping|int who, int art, string|<string|int|<string|int>*>*|int adjektiv, object|mapping|int owner)
{
   return query_deklin(who, art, FALL_NOM, adjektiv, owner);
}
varargs string Wer(object|mapping|int who, int art, string|<string|int|<string|int>*>*|int adjektiv, object|mapping|int owner)
{
   return query_deklin(who, art | ART_CAPITALIZE, FALL_NOM, adjektiv, owner);
}

/*
FUNKTION: wessen
DEKLARATION: varargs string wessen(object|mapping|int who, int art, string|<string|int|<string|int>*>*|int adjektiv, object|mapping|int owner)
BESCHREIBUNG:
Liefert im Genitiv deklinierte Form eines Objektnamens.
Dabei wird beruecksichtigt, ob das Objekt ein personal_name hat.
Weitere Beschreibung ->wer
VERWEISE: wer, wen, wem
GRUPPEN: simul_efun, grammatik
*/
varargs string wessen(object|mapping|int who, int art, string|<string|int|<string|int>*>*|int adjektiv, object|mapping|int owner)
{
   return query_deklin(who, art, FALL_GEN, adjektiv, owner);
}
varargs string Wessen(object|mapping|int who, int art, string|<string|int|<string|int>*>*|int adjektiv, object|mapping|int owner)
{
   return query_deklin(who, art | ART_CAPITALIZE, FALL_GEN, adjektiv, owner);
}


/*
FUNKTION: wem
DEKLARATION: varargs string wem(object|mapping|int who, int art, string|<string|int|<string|int>*>*|int adjektiv, object|mapping|int owner)
BESCHREIBUNG:
Liefert im Dativ deklinierte Form eines Objektnamens.
Dabei wird beruecksichtigt, ob das Objekt ein personal_name hat.
Weitere Beschreibung ->wer
VERWEISE: wer, wen
GRUPPEN: simul_efun, grammatik
*/
varargs string wem(object|mapping|int who, int art, string|<string|int|<string|int>*>*|int adjektiv, object|mapping|int owner)
{
   return query_deklin(who, art, FALL_DAT, adjektiv, owner);
}
varargs string Wem(object|mapping|int who, int art, string|<string|int|<string|int>*>*|int adjektiv, object|mapping|int owner)
{
   return query_deklin(who, art | ART_CAPITALIZE, FALL_DAT, adjektiv, owner);
}


/*
FUNKTION: wen
DEKLARATION: varargs string wen(object|mapping|int who, int art, string|<string|int|<string|int>*>*|int adjektiv, object|mapping|int owner)
BESCHREIBUNG:
Liefert im Akkusativ deklinierte Form eines Objektnamens.
Dabei wird beruecksichtigt, ob das Objekt ein personal_name hat.
Weitere Beschreibung ->wer
VERWEISE: wer, wem
GRUPPEN: simul_efun, grammatik
*/
varargs string wen(object|mapping|int who, int art, string|<string|int|<string|int>*>*|int adjektiv, object|mapping|int owner)
{
   return query_deklin(who, art, FALL_AKK, adjektiv, owner);
}
varargs string Wen(object|mapping|int who, int art, string|<string|int|<string|int>*>*|int adjektiv, object|mapping|int owner)
{
   return query_deklin(who, art | ART_CAPITALIZE, FALL_AKK, adjektiv, owner);
}


/* ============= User Interface fuer query_deklin ============== */



/*
FUNKTION: der
DEKLARATION: varargs string der(object|mapping|int who, string|<string|int|<string|int>*>*|int adjektiv)
BESCHREIBUNG:
Liefert im Nominativ deklinierte Form eines Objektnamens mit bestimmten Artikel
Dabei wird beruecksichtigt, ob das Objekt ein personal_name hat
(query_personal()==1).

  who   Das Objekt dessen deklinierte Form im Nominativ gewuenscht ist.

	ODER eine virtuelles Objekt in der Form:
	   ([ "name" : "...", "gender" : "..." ])
	oder bei Plural
	   ([ "name" : "...", "gender" : "...", "plural": 1 ])

  adjektiv
     string  ->  Grundform eines Adjektivs
                 "" bedeutet kein Adjektiv
     string* ->  ({ unregelm.Grundform, Deklinationsstamm }) eines Adjektivs
                 ({ adjektiv1, adjektiv2, ...}) mehrere Adjektive
     int i   ->  i == 0   Alle Adjektive des Objekts who.
                 i >  0   Die ersten i Adjektive des Objekts who.
                 i <  0   Das -i. te Adjektiv des Objekts who.
     int*    ->  ({ a, b }) Die a bis b. Adjektive des Objekts who.

Mit Hilfe von adjektiv koennen alle oder bestimmte Adjektive des Objekts
oder ein fremdes Adjektiv in den Ergebnisstring zwischen Artikel und
Substantiv gestellt werden.
(Adjektive eines Objekts setzt man mit set_adjektiv, siehe dort.)

Alle Parameter sind optional.
Defaultwerte sind dann
   who:      this_object()
   adjektiv: 0  (alle Adjektive des Objekts who)

BEISPIEL:
   fuer  der, dem, den, einen, einem, einen, dein, deinem, deinen,
         sein, seinem, seinen, dieser, diesem, diesen, er, ihm, ihn

   object Spieler, Ork, Gegenstand;
   Spieler    sei eine Person NAMENS Laura  (also mit personal_name)
   Ork        sei ein Ork ohne Namen mit Adjektiv boese (ohne personal_name)
   Gegenstand sei eine Jacke, die Laura bei sich traegt.

   Der(Spieler)+" zieht "+einem(Ork)+" "+seinen(Gegenstand)+" an."
      liefert:
   "Laura zieht einem boesen Ork ihre Jacke an."

   Dieser(Ork,"frech")+" laesst sich das von "+dem(Spieler)+" nicht gefallen."
      liefert:
   "Dieser freche Ork laesst sich das von Laura nicht gefallen."

   Er(Ork)+" haette "+ihn(Spieler)+" am liebsten davongejagt."
      liefert:
   "Er haette sie am liebsten davongejagt."

   Ein(Ork)+" schaut sich neugierig "+
   seinen((["name":"ausruestung",
	    "gender":"weiblich"]),"gut",Spieler)+" an."
      liefert:
   "Ein Ork schaut sich neugierig ihre gute Ausruestung an."

HINWEIS:
   Saemtliche Funktionen existieren nur in MAENNLICHER Form im
   Singular.
   Die Funktionen erzeugen daraus abhaengig vom Geschlecht und
   Zahl des Objekts die RICHTIGE Form.
   Die Funktionen existieren jeweils in Gross- und Kleinschrift.
   ( der und Der, den und Den,... )
ANMERKUNG:
   Keine Diskriminierung beabsichtigt. Die maennlichen Deklinations-
   formen sind die einzigen die sich im Nominativ und Akkusativ
   voneinander unterscheiden, und nur darauf kommt's hier an. :-)

Saemtliche aufgefuehrte Funktionen sind Spezialfaelle von
   wer, wen, wem, Wer, Wen, Wem.


VERWEISE: der, dem, den, einen, einem, einen, dein, deinem, deinen,
          sein, seinem, seinen, dieser, diesem, diesen, er, ihn, ihm,
          Der, Dem, Den, Einen, Einem, Einen, Dein, Deinem, Deinen,
          Sein, Seinem, Seinen, Dieser, Diesem, Diesen, Er, Ihn, Ihm,
          wer, wen, wem, Wer, Wen, Wem, ihr, ihren, ihrem, Ihr, Ihren,
	  Ihrem, set_adjektiv
GRUPPEN: simul_efun, grammatik
*/
varargs string der(object|mapping|int who, string|<string|int|<string|int>*>*|int adjektiv)    { return wer(who, ART_DER, adjektiv); }
varargs string Der(object|mapping|int who, string|<string|int|<string|int>*>*|int adjektiv)    { return Wer(who, ART_DER, adjektiv); }

/*
FUNKTION: des
DEKLARATION: varargs string des(object|mapping|int who, string|<string|int|<string|int>*>*|int adjektiv)
BESCHREIBUNG:
Liefert deklinierte Form eines Objektnamens.   siehe der
VERWEISE: der
GRUPPEN: simul_efun, grammatik
*/
varargs string des(object|mapping|int who, string|<string|int|<string|int>*>*|int adjektiv)    { return wessen(who, ART_DER, adjektiv); }
varargs string Des(object|mapping|int who, string|<string|int|<string|int>*>*|int adjektiv)    { return Wessen(who, ART_DER, adjektiv); }

/*
FUNKTION: dem
DEKLARATION: varargs string dem(object|mapping|int who, string|<string|int|<string|int>*>*|int adjektiv)
BESCHREIBUNG:
Liefert deklinierte Form eines Objektnamens.   siehe der
VERWEISE: der
GRUPPEN: simul_efun, grammatik
*/
varargs string dem(object|mapping|int who, string|<string|int|<string|int>*>*|int adjektiv)    { return wem(who, ART_DER, adjektiv); }
varargs string Dem(object|mapping|int who, string|<string|int|<string|int>*>*|int adjektiv)    { return Wem(who, ART_DER, adjektiv); }

/*
FUNKTION: den
DEKLARATION: varargs string den(object|mapping|int who, string|<string|int|<string|int>*>*|int adjektiv)
BESCHREIBUNG:
Liefert deklinierte Form eines Objektnamens.   siehe der
VERWEISE: der
GRUPPEN: simul_efun, grammatik
*/
varargs string den(object|mapping|int who, string|<string|int|<string|int>*>*|int adjektiv)    { return wen(who, ART_DER, adjektiv); }
varargs string Den(object|mapping|int who, string|<string|int|<string|int>*>*|int adjektiv)    { return Wen(who, ART_DER, adjektiv); }

/*
FUNKTION: ein
DEKLARATION: varargs string ein(object|mapping|int who, string|<string|int|<string|int>*>*|int adjektiv)
BESCHREIBUNG:
Liefert deklinierte Form eines Objektnamens.   siehe der
VERWEISE: der
GRUPPEN: simul_efun, grammatik
*/
varargs string   ein(object|mapping|int who, string|<string|int|<string|int>*>*|int adjektiv)  { return wer(who, ART_EIN, adjektiv); }
varargs string   Ein(object|mapping|int who, string|<string|int|<string|int>*>*|int adjektiv)  { return Wer(who, ART_EIN, adjektiv); }

/*
FUNKTION: eines
DEKLARATION: varargs string eines(object|mapping|int who, string|<string|int|<string|int>*>*|int adjektiv)
BESCHREIBUNG:
Liefert deklinierte Form eines Objektnamens.   siehe der
VERWEISE: der
GRUPPEN: simul_efun, grammatik
*/
varargs string eines(object|mapping|int who, string|<string|int|<string|int>*>*|int adjektiv)  { return wessen(who, ART_EIN, adjektiv); }
varargs string Eines(object|mapping|int who, string|<string|int|<string|int>*>*|int adjektiv)  { return Wessen(who, ART_EIN, adjektiv); }

/*
FUNKTION: einem
DEKLARATION: varargs string einem(object|mapping|int who, string|<string|int|<string|int>*>*|int adjektiv)
BESCHREIBUNG:
Liefert deklinierte Form eines Objektnamens.   siehe der
VERWEISE: der
GRUPPEN: simul_efun, grammatik
*/
varargs string einem(object|mapping|int who, string|<string|int|<string|int>*>*|int adjektiv)  { return wem(who, ART_EIN, adjektiv); }
varargs string Einem(object|mapping|int who, string|<string|int|<string|int>*>*|int adjektiv)  { return Wem(who, ART_EIN, adjektiv); }

/*
FUNKTION: einen
DEKLARATION: varargs string einen(object|mapping|int who, string|<string|int|<string|int>*>*|int adjektiv)
BESCHREIBUNG:
Liefert deklinierte Form eines Objektnamens.   siehe der
VERWEISE: der
GRUPPEN: simul_efun, grammatik
*/
varargs string einen(object|mapping|int who, string|<string|int|<string|int>*>*|int adjektiv)  { return wen(who, ART_EIN, adjektiv); }
varargs string Einen(object|mapping|int who, string|<string|int|<string|int>*>*|int adjektiv)  { return Wen(who, ART_EIN, adjektiv); }

/*
FUNKTION: dein
DEKLARATION: varargs string dein(object|mapping|int who, string|<string|int|<string|int>*>*|int adjektiv, object|mapping|int owner)
BESCHREIBUNG:
Liefert deklinierte Form eines Objektnamens.   siehe der
Zusaetzlicher Parameter owner gibt den Besitzer von who an.
Fehlt owner, dann wird das naechste living environment von who genommen.
Hat who kein living environment, so wird auf automatische Artikelauswahl
(ein/der) umgeschaltet.
VERWEISE: der
GRUPPEN: simul_efun, grammatik
*/
varargs string   dein(object|mapping|int who, string|<string|int|<string|int>*>*|int adjektiv, object|mapping|int owner) { return wer(who,ART_DEIN, adjektiv, owner); }
varargs string   Dein(object|mapping|int who, string|<string|int|<string|int>*>*|int adjektiv, object|mapping|int owner) { return Wer(who,ART_DEIN, adjektiv, owner); }

/*
FUNKTION: deines
DEKLARATION: varargs string deines(object|mapping|int who, string|<string|int|<string|int>*>*|int adjektiv, object|mapping|int owner)
BESCHREIBUNG:
Liefert deklinierte Form eines Objektnamens.   siehe der
Zusaetzlicher Parameter owner gibt den Besitzer von who an.
Fehlt owner, dann wird das naechste living environment von who genommen.
Hat who kein living environment, so wird auf automatische Artikelauswahl
(ein/der) umgeschaltet.
VERWEISE: der
GRUPPEN: simul_efun, grammatik
*/
varargs string deines(object|mapping|int who, string|<string|int|<string|int>*>*|int adjektiv, object|mapping|int owner) {return wessen(who,ART_DEIN, adjektiv, owner);}
varargs string Deines(object|mapping|int who, string|<string|int|<string|int>*>*|int adjektiv, object|mapping|int owner) {return Wessen(who,ART_DEIN, adjektiv, owner);}

/*
FUNKTION: deinem
DEKLARATION: varargs string deinem(object|mapping|int who, string|<string|int|<string|int>*>*|int adjektiv, object|mapping|int owner)
BESCHREIBUNG:
Liefert deklinierte Form eines Objektnamens.   siehe der
Zusaetzlicher Parameter owner gibt den Besitzer von who an.
Fehlt owner, dann wird das naechste living environment von who genommen.
Hat who kein living environment, so wird auf automatische Artikelauswahl
(ein/der) umgeschaltet.
VERWEISE: der
GRUPPEN: simul_efun, grammatik
*/
varargs string deinem(object|mapping|int who, string|<string|int|<string|int>*>*|int adjektiv, object|mapping|int owner) { return wem(who,ART_DEIN, adjektiv, owner);}
varargs string Deinem(object|mapping|int who, string|<string|int|<string|int>*>*|int adjektiv, object|mapping|int owner) { return Wem(who,ART_DEIN, adjektiv, owner);}

/*
FUNKTION: deinen
DEKLARATION: varargs string deinen(object|mapping|int who, string|<string|int|<string|int>*>*|int adjektiv, object|mapping|int owner)
BESCHREIBUNG:
Liefert deklinierte Form eines Objektnamens.   siehe der
Zusaetzlicher Parameter owner gibt den Besitzer von who an.
Fehlt owner, dann wird das naechste living environment von who genommen.
Hat who kein living environment, so wird auf automatische Artikelauswahl
(ein/der) umgeschaltet.
VERWEISE: der
GRUPPEN: simul_efun, grammatik
*/
varargs string deinen(object|mapping|int who, string|<string|int|<string|int>*>*|int adjektiv, object|mapping|int owner) { return wen(who,ART_DEIN, adjektiv, owner); }
varargs string Deinen(object|mapping|int who, string|<string|int|<string|int>*>*|int adjektiv, object|mapping|int owner) { return Wen(who,ART_DEIN, adjektiv, owner); }

/*
FUNKTION: sein
DEKLARATION: varargs string sein(object|mapping|int who, string|<string|int|<string|int>*>*|int adjektiv, object|mapping|int owner)
BESCHREIBUNG:
Liefert deklinierte Form eines Objektnamens.   siehe der
Zusaetzlicher Parameter owner gibt den Besitzer von who an.
Fehlt owner, dann wird das naechste living environment von who genommen.
Hat who kein living environment, so wird auf automatische Artikelauswahl
(ein/der) umgeschaltet.
VERWEISE: der
GRUPPEN: simul_efun, grammatik
*/
varargs string sein(object|mapping|int who, string|<string|int|<string|int>*>*|int adjektiv, object|mapping|int owner) { return wer(who, ART_SEIN, adjektiv, owner); }
varargs string Sein(object|mapping|int who, string|<string|int|<string|int>*>*|int adjektiv, object|mapping|int owner) { return Wer(who, ART_SEIN, adjektiv, owner); }

/*
FUNKTION: seines
DEKLARATION: varargs string seines(object|mapping|int who, string|<string|int|<string|int>*>*|int adjektiv, object|mapping|int owner)
BESCHREIBUNG:
Liefert deklinierte Form eines Objektnamens.   siehe der
Zusaetzlicher Parameter owner gibt den Besitzer von who an.
Fehlt owner, dann wird das naechste living environment von who genommen.
Hat who kein living environment, so wird auf automatische Artikelauswahl
(eines/des) umgeschaltet.
VERWEISE: der
GRUPPEN: simul_efun, grammatik
*/
varargs string seines(object|mapping|int who, string|<string|int|<string|int>*>*|int adjektiv, object|mapping|int owner) {return wessen(who,ART_SEIN, adjektiv, owner);}
varargs string Seines(object|mapping|int who, string|<string|int|<string|int>*>*|int adjektiv, object|mapping|int owner) {return Wessen(who,ART_SEIN, adjektiv, owner);}

/*
FUNKTION: seinem
DEKLARATION: varargs string seinem(object|mapping|int who, string|<string|int|<string|int>*>*|int adjektiv, object|mapping|int owner)
BESCHREIBUNG:
Liefert deklinierte Form eines Objektnamens.   siehe der
Zusaetzlicher Parameter owner gibt den Besitzer von who an.
Fehlt owner, dann wird das naechste living environment von who genommen.
Hat who kein living environment, so wird auf automatische Artikelauswahl
(einem/dem) umgeschaltet.
VERWEISE: der
GRUPPEN: simul_efun, grammatik
*/
varargs string seinem(object|mapping|int who, string|<string|int|<string|int>*>*|int adjektiv, object|mapping|int owner) { return wem(who, ART_SEIN, adjektiv, owner); }
varargs string Seinem(object|mapping|int who, string|<string|int|<string|int>*>*|int adjektiv, object|mapping|int owner) { return Wem(who, ART_SEIN, adjektiv, owner); }

/*
FUNKTION: seinen
DEKLARATION: varargs string seinen(object|mapping|int who, string|<string|int|<string|int>*>*|int adjektiv, object|mapping|int owner)
BESCHREIBUNG:
Liefert deklinierte Form eines Objektnamens.   siehe der
Zusaetzlicher Parameter owner gibt den Besitzer von who an.
Fehlt owner, dann wird das naechste living environment von who genommen.
Hat who kein living environment, so wird auf automatische Artikelauswahl
(einen/den) umgeschaltet.
VERWEISE: der
GRUPPEN: simul_efun, grammatik
*/
varargs string seinen(object|mapping|int who, string|<string|int|<string|int>*>*|int adjektiv, object|mapping|int owner) { return wen(who, ART_SEIN, adjektiv, owner); }
varargs string Seinen(object|mapping|int who, string|<string|int|<string|int>*>*|int adjektiv, object|mapping|int owner) { return Wen(who, ART_SEIN, adjektiv, owner); }

/*
FUNKTION: dieser
DEKLARATION: varargs string dieser(object|mapping|int who, string|<string|int|<string|int>*>*|int adjektiv)
BESCHREIBUNG:
Liefert deklinierte Form eines Objektnamens.   siehe der
VERWEISE: der
GRUPPEN: simul_efun, grammatik
*/
varargs string dieser(object|mapping|int who, string|<string|int|<string|int>*>*|int adjektiv) { return wer(who, ART_DIESER, adjektiv); }
varargs string Dieser(object|mapping|int who, string|<string|int|<string|int>*>*|int adjektiv) { return Wer(who, ART_DIESER, adjektiv); }

/*
FUNKTION: dieses
DEKLARATION: varargs string dieses(object|mapping|int who, string|<string|int|<string|int>*>*|int adjektiv)
BESCHREIBUNG:
Liefert deklinierte Form eines Objektnamens.   siehe der
VERWEISE: der
GRUPPEN: simul_efun, grammatik
*/
varargs string dieses(object|mapping|int who, string|<string|int|<string|int>*>*|int adjektiv) { return wessen(who, ART_DIESER, adjektiv); }
varargs string Dieses(object|mapping|int who, string|<string|int|<string|int>*>*|int adjektiv) { return Wessen(who, ART_DIESER, adjektiv); }

/*
FUNKTION: diesem
DEKLARATION: varargs string diesem(object|mapping|int who, string|<string|int|<string|int>*>*|int adjektiv)
BESCHREIBUNG:
Liefert deklinierte Form eines Objektnamens.   siehe der
VERWEISE: der
GRUPPEN: simul_efun, grammatik
*/
varargs string diesem(object|mapping|int who, string|<string|int|<string|int>*>*|int adjektiv) { return wem(who, ART_DIESER, adjektiv); }
varargs string Diesem(object|mapping|int who, string|<string|int|<string|int>*>*|int adjektiv) { return Wem(who, ART_DIESER, adjektiv); }

/*
FUNKTION: diesen
DEKLARATION: varargs string diesen(object|mapping|int who, string|<string|int|<string|int>*>*|int adjektiv)
BESCHREIBUNG:
Liefert deklinierte Form eines Objektnamens.   siehe der
VERWEISE: der
GRUPPEN: simul_efun, grammatik
*/
varargs string diesen(object|mapping|int who, string|<string|int|<string|int>*>*|int adjektiv) { return wen(who, ART_DIESER, adjektiv); }
varargs string Diesen(object|mapping|int who, string|<string|int|<string|int>*>*|int adjektiv) { return Wen(who, ART_DIESER, adjektiv); }

/*
FUNKTION: er
DEKLARATION: varargs string er(object|mapping|int who)
BESCHREIBUNG:
Liefert deklinierte Form eines Objektnamens.   siehe der
Anders als bei der fehlt der Parameter adjektiv, Personalpronomina
haben keine Adjektive.
VERWEISE: der
GRUPPEN: simul_efun, grammatik
*/
varargs string  er(object|mapping|int who)            { return wer(who, ART_ER); }
varargs string  Er(object|mapping|int who)            { return Wer(who, ART_ER); }

/*
FUNKTION: ihm
DEKLARATION: varargs string ihm(object|mapping|int who)
BESCHREIBUNG:
Liefert deklinierte Form eines Objektnamens.   siehe der
Anders als bei der fehlt der Parameter adjektiv, Personalpronomina
haben keine Adjektive.
VERWEISE: der
GRUPPEN: simul_efun, grammatik
*/
varargs string ihm(object|mapping|int who)            { return wem(who, ART_ER); }
varargs string Ihm(object|mapping|int who)            { return Wem(who, ART_ER); }

/*
FUNKTION: ihn
DEKLARATION: varargs string ihn(object|mapping|int who)
BESCHREIBUNG:
Liefert deklinierte Form eines Objektnamens.   siehe der
Anders als bei der fehlt der Parameter adjektiv, Personalpronomina
haben keine Adjektive.
VERWEISE: der
GRUPPEN: simul_efun, grammatik
*/
varargs string ihn(object|mapping|int who)            { return wen(who, ART_ER); }
varargs string Ihn(object|mapping|int who)            { return Wen(who ,ART_ER); }



/* ========= User Interface fuer query_deklin_owner ============ */


/*
FUNKTION: ihr
DEKLARATION: varargs string ihr(<object|mapping|int> who, <string|string*|int|int*> who_adjektiv, <object|mapping|int> owner, <string|string*|int|int*> owner_adjektiv, int who_art, int owner_art)
BESCHREIBUNG:
Liefert deklinierte Form eines Objektnamens. Siehe der().

Der zusaetzlicher Parameter owner gibt den Besitzer von who an.
Fehlt owner, dann wird das naechste living environment von who genommen.
Hat who kein living environment, so wird auf automatische Artikelauswahl
(ein/der) umgeschaltet.

Zum eigentlichen Objekt who bzw. zum Besitzer owner kann man zusaetzlich
noch Adjektive (who_adjektiv bzw. owner_adjektiv) angeben, die statt der
tatsaechlich gesetzten Adjektive genommen werden. Ebenso kann man bei
beiden die Auswahl der Pronomen mit who_art bzw. owner_art bestimmen
(genaue Beschreibung siehe query_deklin).

***WICHTIG***:
ihr() liefert NICHT 'ihr Testobjekt' (das macht ja sein() schon automatisch),
SONDERN 'Stellas Testobjekt'. Also NICHT das POSSESIVPRONOMEN, sondern
den NAMEN des Besitzers selbst. Ist der Besitzer nicht 'personal', dann
liefert ihr() beispielsweise: 'das Testobjekt des Orks'.

VERWEISE: ihres, ihren, ihrem, der, sein, query_deklin
GRUPPEN: simul_efun, grammatik
*/
varargs string ihr(<object|mapping|int> who, <string|string*|int|int*> who_adjektiv, <object|mapping|int> owner, <string|string*|int|int*> owner_adjektiv, int who_art, int owner_art, int old_who_art)
{
   if(!owner_art)
      owner_art = ART_AAA;
   return query_deklin_owner(who, who_art || old_who_art, FALL_NOM, who_adjektiv, owner, owner_art, owner_adjektiv);
}
varargs string Ihr(<object|mapping|int> who, <string|string*|int|int*> who_adjektiv, <object|mapping|int> owner, <string|string*|int|int*> owner_adjektiv, int who_art, int owner_art, int old_who_art)
{
   if (!owner_art)
      owner_art = ART_AAA;
   return query_deklin_owner(who, (who_art || old_who_art) | ART_CAPITALIZE, FALL_NOM, who_adjektiv, owner, owner_art, owner_adjektiv);
}

/*
FUNKTION: ihres
DEKLARATION: varargs string ihres(<object|mapping|int> who, <string|string*|int|int*> who_adjektiv, <object|mapping|int> owner, <string|string*|int|int*> owner_adjektiv, int who_art, int owner_art)
BESCHREIBUNG:
Liefert deklinierte Form eines Objektnamens. Siehe ihr().
VERWEISE: ihr, sein, der
GRUPPEN: simul_efun, grammatik
*/
varargs string ihres(<object|mapping|int> who, <string|string*|int|int*> who_adjektiv, <object|mapping|int> owner, <string|string*|int|int*> owner_adjektiv, int who_art, int owner_art, int old_who_art)
{
   if(!owner_art)
      owner_art = ART_AAA;
   return query_deklin_owner(who, who_art || old_who_art, FALL_GEN, who_adjektiv, owner, owner_art, owner_adjektiv);
}
varargs string Ihres(<object|mapping|int> who, <string|string*|int|int*> who_adjektiv, <object|mapping|int> owner, <string|string*|int|int*> owner_adjektiv, int who_art, int owner_art, int old_who_art)
{
   if (!owner_art)
      owner_art = ART_AAA;
   return query_deklin_owner(who, (who_art || old_who_art) | ART_CAPITALIZE, FALL_GEN, who_adjektiv, owner, owner_art, owner_adjektiv);
}

/*
FUNKTION: ihrem
DEKLARATION: varargs string ihrem(<object|mapping|int> who, <string|string*|int|int*> who_adjektiv, <object|mapping|int> owner, <string|string*|int|int*> owner_adjektiv, int who_art, int owner_art)
BESCHREIBUNG:
Liefert deklinierte Form eines Objektnamens. Siehe ihr().
VERWEISE: ihr, sein, der
GRUPPEN: simul_efun, grammatik
*/
varargs string ihrem(<object|mapping|int> who, <string|string*|int|int*> who_adjektiv, <object|mapping|int> owner, <string|string*|int|int*> owner_adjektiv, int who_art, int owner_art, int old_who_art)
{
   if(!owner_art)
      owner_art = ART_AAA;
   return query_deklin_owner(who, who_art || old_who_art, FALL_DAT, who_adjektiv, owner, owner_art, owner_adjektiv);
}
varargs string Ihrem(<object|mapping|int> who, <string|string*|int|int*> who_adjektiv, <object|mapping|int> owner, <string|string*|int|int*> owner_adjektiv, int who_art, int owner_art, int old_who_art)
{
   if (!owner_art)
      owner_art = ART_AAA;
   return query_deklin_owner(who, (who_art || old_who_art) | ART_CAPITALIZE, FALL_DAT, who_adjektiv, owner, owner_art, owner_adjektiv);
}

/*
FUNKTION: ihren
DEKLARATION: varargs string ihren(<object|mapping|int> who, <string|string*|int|int*> who_adjektiv, <object|mapping|int> owner, <string|string*|int|int*> owner_adjektiv, int who_art, int owner_art)
BESCHREIBUNG:
Liefert deklinierte Form eines Objektnamens. Siehe ihr().
VERWEISE: ihr, sein, der
GRUPPEN: simul_efun, grammatik
*/
varargs string ihren(<object|mapping|int> who, <string|string*|int|int*> who_adjektiv, <object|mapping|int> owner, <string|string*|int|int*> owner_adjektiv, int who_art, int owner_art, int old_who_art)
{
   if(!owner_art)
      owner_art = ART_AAA;
   return query_deklin_owner(who, who_art || old_who_art, FALL_AKK, who_adjektiv, owner, owner_art, owner_adjektiv);
}
varargs string Ihren(<object|mapping|int> who, <string|string*|int|int*> who_adjektiv, <object|mapping|int> owner, <string|string*|int|int*> owner_adjektiv, int who_art, int owner_art, int old_who_art)
{
   if (!owner_art)
      owner_art = ART_AAA;
   return query_deklin_owner(who, (who_art || old_who_art) | ART_CAPITALIZE, FALL_AKK, who_adjektiv, owner, owner_art, owner_adjektiv);
}

/* ===================== end of deklin ========================= */

/*
FUNKTION: plural
DEKLARATION: varargs string plural(string einzahl_str, string plural_str, object|mapping|int ob, int art)
BESCHREIBUNG:
Liefert plural_str, wenn ob im Plural ist, sonst einzahl_str.
ob kann hierbei ein v-item, ein Objekt sein oder OBJ_TO/PO/TP/OW/TI sein.
Wenn ob nicht angegeben wird, wird this_object() angenommen.
art hat die gleiche Bedeutung wie der zweite Parameter von query_deklin.
BEISPIEL:
Der(ob) + plural(" braucht"," brauchen",ob) + " nichts."
VERWEISE: ist, query_deklin
GRUPPEN: simul_efun, grammatik
*/
varargs string plural(string einzahl, string plural_str, object|mapping|int ob, int art)
{
    int plural;
    
    ob = parse_deklin_object(ob);

    if((art & ART_INVIS) ||
      !(art & ART_VIS) && (QUERY("invis", ob) & V_ATOM_INVIS))
	plural = 0;
    else
    {
        mapping | string | <string|int>* | <string|int>** menge = QUERY("menge",ob);
	if(mappingp(menge))
	    plural = QUERY("plural",menge);
	else
	    plural = QUERY("plural",ob);
    }
    
    return plural ? plural_str : einzahl;
}

/*
FUNKTION: ist
DEKLARATION: varargs string ist(object|mapping|int ob, int spaces)
BESCHREIBUNG:
Liefert "sind", wenn ob im Plural ist, sonst "ist".
ob kann hierbei ein v-item oder ein Objekt sein.
Wenn ob nicht angegeben wird, wird this_object() angenommen.
Diese Funktion ist ein Spezialfall der Funktion 'plural'
Wenn spaces == IST_SPACE_BEFORE ist, wird ein Leerzeichen vorangestellt,
bei spaces == IST_SPACE_AFTER angehaengt, und bei
spaces == IST_SPACE_BEFORE|IST_SPACE_AFTER beides.
Diese Konstanten sind in simul_efuns.h definiert.

BEISPIEL:

    Der(ob) + ist(ob, IST_SPACE_BEFORE) + " nicht zu gebrauchen."

VERWEISE: plural
GRUPPEN: simul_efun, grammatik
*/
varargs string ist(object|mapping|int ob, int spaces)
{
    if (spaces == 1)
	return plural(" ist"," sind",ob);
    if (spaces == 0)
	return plural("ist","sind",ob);
    if (spaces == 2)
	return plural("ist ","sind ",ob);
    return plural(" ist "," sind ",ob);
}

/*
FUNKTION: choose_by_gender
DEKLARATION: mixed choose_by_gender(string gender|object ob|mapping vitem, mixed *values|mapping values|string str)
BESCHREIBUNG:
Diese Funktion waehlt entsprechend gender ("maennlich","weiblich" oder
"saechlich") bzw. dem Geschlecht des Objektes ob oder des V-Items vitem
einen Wert aus values aus. Man kann ebenfalls die Defines OBJ_TP und andere
OBJ-Defines aus deklin.h nutzen.
values kann entweder ein Array Groesse 3 sein (Reihenfolge
"saechlich", "maennlich", "weiblich"), ein Mapping (die Geschlechter als
Schluessel) oder einfach ein String (der wird dann nur zurueck gegeben).
Wenn values als Array ein viertes Element oder als Mapping einen weiteren
Eintrag "plural" hat, dann wird im Falle des Plurals dieser Eintrag genommen.

Beispiel:

       choose_by_gender(OBJ(croft), ({ "Erschaffendes",
           "Erschaffender", "Erschaffende" })) -> "Erschaffende"

       choose_by_gender((["gender":"weiblich"]), ([
           "saechlich" : "Testendes",
           "maennlich" : "Testender",
           "weiblich"  : "Testende" ])) -> "Testende"
	
Oder in einer Pseudoclosure:

	"Hallo, $choose_by_gender(OBJ_TP," // <- Achtung, kein Leerzeichen!
	"([maennlich:Meister,weiblich:Meisterin,saechlich:Meisterliches]))!"

VERWEISE: query_gender
GRUPPEN: simul_efun, grammatik
*/
mixed choose_by_gender(string|object|mapping|int what, mixed*|mapping|string values)
{
    string gender;
    int plural;
    if (!pointerp(values) && !stringp(values) && !mappingp(values))
    {
        do_error("choose_by_gender: Parameter 2 muss ein Array, Mapping oder String sein.\n");
        return 0;
    }
    if (pointerp(values) && sizeof(values) != 3 && sizeof(values) != 4)
    {
        do_error("get_gender_string: Parameter 2 als Array muss Größe 3 oder 4 haben.\n");
        return 0;
    }
    
    if (!stringp(what))
        what = parse_deklin_object(what);
    if (mappingp(what))
	gender = what["gender"];
    else if (objectp(what))
	gender = ({string})what->query_gender();
    else if (stringp(what))
	gender = ({string})what;
    else
    {
        do_error("get_gender_string: Parameter 1 muss Object, Mapping oder String sein.\n");
        return 0;
    }

    if(mappingp(what) || objectp(what))
    {
        mapping|string|<string|int>*|<string|int>** menge = QUERY("menge",what);
	if(mappingp(menge))
	{
	    plural = QUERY("plural",menge);
	    gender = QUERY("gender",menge);
	}
	else
	    plural = QUERY("plural",what);
    }
    else
	plural = 0;

    if (stringp(values))
        return values;
    if (mappingp(values))
    {
	if(plural && member(values,"plural"))
	    return values["plural"];
        return values[gender];
    }
    if (pointerp(values) && (sizeof(values)==3 || sizeof(values)==4))
    {
	if(plural && sizeof(values) == 4)
	    return values[3];
	
        switch (gender)
	{
    	    case "maennlich" : return values[1];
    	    case "weiblich" :  return values[2];
    	    default:           return values[0];
        }
    }
}
