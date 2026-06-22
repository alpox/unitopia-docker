//
// Das File zu einem Verkaeufer, der (fast) vollstaendig von einer Rolle
// gesteuert wird.
//
inherit "/i/monster/monster";

#include <lpc_parser.h>
LPC_PARSER_VARS

string ware, neue_ware;
object kunde, neuer_kunde, ware_ob;
int geld_uebergeben;

mapping preise = ([ "fackel": 20, "schaufel": 80, "rucksack": 160 ]);
string valuta = "taler";

void kunde_moechte_etwas(string was)
{
   if( strstr(was,"fackel") >= 0 )
      neue_ware="fackel";
   else
   if( strstr(was,"schaufel") >= 0 )
      neue_ware="schaufel";
   else
   if( strstr(was,"rucksack") >= 0 )
      neue_ware="rucksack";
   else
      return;

   neuer_kunde=this_player();
   starte_rolle("/doc/funktionsweisen/monster/rollen/beispiele/v_rolle");
}
 
void rolle_beendet(string rolle)
{
   kunde=ware=0;
}

int kunde_uebergibt_geld(object what)
{
   if( what && this_player() && what->query_money() && this_player() == kunde )
      if( what->query_valuta() != valuta )
         do_command("sage Ich hätte aber lieber "
            +capitalize(valuta)+" von Dir.");
      else
         if( what->query_money() != preise[ware] )
            do_command("sage Ich bekomme von Dir "+preise[ware]+" "
               +capitalize(valuta)+"!");
         else
         {
            what->remove();
            geld_uebergeben=1;
            return 1;
         }
}

void create()
{
   ::create();
   initialize("verkäufer",20);
   set_accept_objects(({ #'kunde_uebergibt_geld,"geld",
                         #'refuse }));
   set_parse_conversation(this_object(),
      ({ "kunde_moechte_etwas: sagt && möchte || sagt && will || "+
                             " sagt && haben || sagt && hast" }));
}
