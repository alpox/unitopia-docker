// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/i/living/eyes.c
// Description: Augen, hauptsaechlich zum Durchblicken...
//		Garthan (06.03.1996) examine_object, reading: query_light
//		                     statt set_light(0)
//		Garthan (27.05.1996) query_contents in query_object_descr.
//		Freaky  (28.05.1996) this_object()->query_eye_level() damit
//				     man es shadowen kann
//		Freaky  (21.09.1997) can_see() eingefuehrt
//		Freaky  (10.03.1998) message auf send_message umgebaut.
//              Offler  (14.03.2000) examin_object: x in y
//              Sissi   (18.05.2000) visible_in_the_dark - vitems kann man
//                                   nun im Dunkeln angucken

#pragma save_types
#pragma strong_types

private functions inherit "/i/tools/kleidung";

// kann spaeter wieder raus (wetten, dass ichs vergess):
#include <level.h>

#include <parse_com.h>
#include <invis.h>
#include <room.h>
#include <eyes.h>
#include <more.h>
#include <message.h>
#include <landschaft.h>
#include <stats.h>
#include <time.h>
#include <term.h>
#include <control.h>
#include <config.h>
#include <notify_fail.h>
#include <strings.h>

private static int eye_level;

nomask int query_wiz_level();
varargs mixed query_eye_option(string option);
int count_shimmer(object room);


#ifdef FILTER_MSG_BY_ATTRIBUTES
varargs void notify_message(string msg, int type,mapping attributes)
{
    if(this_object())
        this_object()->receive_message(MT_NOTIFY,type,this_object(),
            msg,attributes);
}
#else
varargs void notify_message(string msg, int type)
{
    if(this_object())
        this_object()->receive_message(MT_NOTIFY,type,this_object(),msg);
}
#endif

/*
FUNKTION: query_eye_level
DEKLARATION: int query_eye_level()
BESCHREIBUNG:
query_eye_level liefert den Bonus eines Lebewesens (Monster/Spieler),
der auf alle Lichttests aufaddiert wird.

Bei normalen Spielern und Monstern ist der eye_level Null.
Eine Wert groesser Null bedeutet besser sehen, ein Wert kleiner Null
schlechter sehen.

VERWEISE: set_eye_level, describe_room
GRUPPEN: licht, monster, spieler, augen
*/

int query_eye_level()
{
   return eye_level;
}

/*
FUNKTION: set_eye_level
DEKLARATION: void set_eye_level(int level)
BESCHREIBUNG:
set_eye_level setzt den Bonus eines Lebewesens (Monster/Spieler),
der auf alle Lichttests aufaddiert wird.

Bei normalen Spielern und Monstern ist der eye_level Null.
Ein Wert groesser Null bedeutet besser sehen, ein Wert kleiner Null
schlechter sehen.

VERWEISE: query_eye_level, describe_room
GRUPPEN: licht, monster, spieler, augen
*/

void set_eye_level(int i)
{
   eye_level = i;
}

/*
FUNKTION: query_visible_in_the_dark
DEKLARATION: int query_visible_in_the_dark()
BESCHREIBUNG:
Wenn diese Funktion eines Objektes einen Wert != 0 liefert, so kann
man das Objekt betrachten, auch wenn es eigentlich zu dunkel dafuer ist.
VERWEISE: can_see, query_eye_level, query_light, query_obvious
GRUPPEN: licht, monster, spieler, augen
*/

/*
FUNKTION: can_see
DEKLARATION: int can_see(object ob)
BESCHREIBUNG:
Liefert 1, wenn das Living das Objekt 'ob' sehen kann.
Dies ist der Fall, wenn es fuer den Spieler hell genug ist.
Ansonsten wird 0 zurueckgeliefert.
VERWEISE: query_eye_level, query_light, query_obvious
GRUPPEN: licht, monster, spieler, augen
*/
int can_see(object ob)
{
    object env;

    while (env = environment(ob))
	ob = env;
    return ob->query_light() + this_object()->query_eye_level() > 0;
}

/*
FUNKTION: cannot_see
DEKLARATION: string cannot_see(object ob)
BESCHREIBUNG:
Liefert 0, falls der Spieler das Objekt 'ob' sehen kann.
Ansonsten wird ein umgebrochener String geliefert, der beschreibt,
warum der Spieler das Objekt nicht sehen kann.
VERWEISE: can_see
GRUPPEN: licht, monster, spieler, augen
*/
string cannot_see(object ob)
{
    if (this_object()->is_blind())
        return "Du bist aber blind.\n";
    if (!this_object()->can_see(ob))
	return "Es ist zu dunkel.\n";
    return 0;
}

/*
Liefert die Breite fuer die Umbrueche der Item- und
Raumbeschreibung. Zum Ueberlagern durch den Player.
*/
protected int query_eye_description_width()
{
    return 75;
}

private string eye_wrap(string str, int width)
{
    if (!sizeof(str))
        return "";
    if (str[<1] == '\n')
        return str;
    return wrap(str, width);
}

private int exit_mode;
void set_exit_mode(int i) { exit_mode=i; }
int query_exit_mode() { return exit_mode; }

#define EXIT_MODE_NORMALE_AUSGAENGE_GRAFISCH_ANZEIGEN \
    (query_exit_mode()&1)
#define EXIT_MODE_ANDERE_AUSGAENGE_GRAFISCH_ANZEIGEN \
    (query_exit_mode()&2)
#define EXIT_MODE_GRAFISCH_ANGEZEIGTE_AUSGAENGE_NICHT_AUFLISTEN \
    (query_exit_mode()&4)
#define EXIT_MODE_HOCH_RUNTER_GRAFISCH_ANZEIGEN \
    (query_exit_mode()&8)

/*
BESCHREIBUNG:
Liefert eine formatierte, gegebenenfalls grafische Darstellung (Liste) der
Ausgaenge in exits mit der Ueberschrift prefix zurueck.

Ist flag&1==0, so handelt es sich um normale Ausgaenge (EXIT_VISIBLE),
ansonsten um nur fuer Goetter sichtbare Ausgaenge (EXIT_NOLIST, EXIT_HIDDEN,
EXIT_LOCKED).
*/
private string formatted_graphical_exits(string * exits, int flag)
{
    exits||=({}); // RTE verhindern.
    exits-=({0}); // Aus Kompatibilitaetsgruenden.

    if(
        !(flag&1) && EXIT_MODE_NORMALE_AUSGAENGE_GRAFISCH_ANZEIGEN ||
        flag&1 && EXIT_MODE_ANDERE_AUSGAENGE_GRAFISCH_ANZEIGEN
        )
    {
        mixed grafik=({
            "     ",
            " O   ",
            "     ",
            });

        mixed richtungen=({
            ({"nordwesten", 0, 0, '\\'}),       // halblinks
            ({"norden"    , 0, 1, '|' }),       // geradeaus
            ({"nordosten" , 0, 2, '/' }),       // halbrechts
            ({"westen"    , 1, 0, '-' }),       // links
            ({"osten"     , 1, 2, '-' }),       // rechts
            ({"suedwesten", 2, 0, '/' }),       // scharflinks
            ({"sueden"    , 2, 1, '|' }),       // zurueck
            ({"süden"     , 2, 1, '|' }),       // zurück
            ({"suedosten" , 2, 2, '\\'}),       // scharfrechts
            ({"südosten"  , 2, 2, '\\'}),
            ({"südwesten" , 2, 0, '/' }),
            ({"hoch"      , 0, 4, '^' }),
            ({"runter"    , 2, 4, 'v' }),
            ({"oben"      , 0, 4, '^' }),
            ({"unten"     , 2, 4, 'v' }),
            });
        string *altrichtungen = ({
            "halblinks", "geradeaus", "halbrechts", "links",
            "rechts", "scharflinks", "zurueck", "zurück", "scharfrechts" });

        if(sizeof(exits) && sizeof(exits & altrichtungen) && !sizeof(exits & map(richtungen[0..<5], #'[, 0)))
            foreach(int i: sizeof(altrichtungen))
                richtungen[i][0] = altrichtungen[i];

        int anz_richtungen_zu_untersuchen;
        
        if (EXIT_MODE_HOCH_RUNTER_GRAFISCH_ANZEIGEN)
            anz_richtungen_zu_untersuchen = sizeof (richtungen);
        else
            anz_richtungen_zu_untersuchen = sizeof (richtungen)-4;

        for(int i=0; i<anz_richtungen_zu_untersuchen; i+=1)
            if(member(exits, richtungen[i][0])!=-1)
            {
                grafik[richtungen[i][1]][richtungen[i][2]]=richtungen[i][3];
                if(EXIT_MODE_GRAFISCH_ANGEZEIGTE_AUSGAENGE_NICHT_AUFLISTEN)
                    exits-=({richtungen[i][0]});
            }

        if(grafik[0][4]!=' ' || grafik[2][4]!=' ')
            grafik[1][4]='|';

        return implode(grafik, "\n")+"\n";
    }

    return 0;
}

private string formatted_exits(string prefix, string * exits, int prwidth, int width)
{
    exits-=({0}); // Aus Kompatibilitaetsgruenden.

    if(!sizeof(exits))
        return prefix+".\n";

    string * exit_lst = map(exits, (: MSG_REXIT(capitalize($1)) :) );
    return wrap_say(right(prefix + ":", prwidth + 1),
         liste(exit_lst)+".", width, prwidth + 2);
    /* return sprintf(
        "%*s: %=-*s\n",
        prwidth,
        prefix,
        width - prwidth - 2,
        liste(exit_lst)+".");*/
}

private string query_room_description_exit_view(object room, int flag)
{
    string * view_exits = room->query_command_list(EXIT_VIEW);
    if (!sizeof(view_exits))
        return 0;

    string ret = 0;
    foreach(string exit: view_exits)
    {
        object nroom = touch(room->query_one_exit(exit));
        object * visible_objs = filter(all_inventory(nroom),
            (:  living($1) && !$1->query_invis() :) );
        int obj_count = sizeof(visible_objs);

        if (!obj_count)
            continue;

        string tmp = exit=="hoch" ? "Über dir":
              exit=="runter" ? "Unter dir":
              "Im "+capitalize(exit);
        tmp += ({ " erkennst", " siehst", " bemerkst" })[random(3)] +
            " du ";

        if (this_object()->can_see(nroom))
        {
            tmp += liste(map(visible_objs, #'einen)) + ".";
        }
        else
        {
            tmp += "im Dunkeln "+ (obj_count > 1 
                ? obj_count+" Gestalten.": "eine Gestalt.");
        }

        if (ret)
            ret += " " + tmp;
        else
            ret = tmp;
    }
    return ret;
}

private string query_room_description_exits(object room, int flag, int width)
{
    string * exits = room->query_command_list(EXIT_VISIBLE);
    if (!sizeof(exits))
        return 0;

    int dark = flag & EYE_DARK;
    if(dark)
    {
        return eye_wrap("Undeutlich erkennst du in der Dunkelheit "+
            (sizeof(exits) == 1
                ? "einen Ausgang."
                : sizeof(exits) + " Ausgänge."), width);
    }

    string *categories = room->query_category_list(EXIT_VISIBLE);
    if (categories && sizeof(categories-({0})) 
        && sizeof(exits) == sizeof(categories))
    {
        // exits merken, da formatted_graphical_exits (je nach 
        // einstellung) die grafisch angezeigten exits rausloescht
        string * orig_exits = exits;
        string graph = formatted_graphical_exits(&exits, 0);

        // paare ({ exit, category }) rausfiltern, die jetzt noch 
        // angezeigt werden muessen
        mixed* pairs = filter(
            transpose_array(({orig_exits, categories})), 
            (: member($2, $1[0])>=0 :), exits);

        // restliche categories bestimmen
        categories = map(pairs, #'[, 1);
        // doppelte eintraege entfernen und rest sortieren
        string * cats = sort_array(m_indices(
            mkmapping(categories - ({0}))),#'>);
             
        int cwidth; // breite der category-spalte
        if(graph)
        {
            cwidth = member(categories, 0)>=0?6:0; // sizeof("Weiter")
            cwidth = foldl(cats, cwidth, (: max($1, sizeof($2)) :));
        }
        else
        {
            cwidth = max(map(cats, #'sizeof)+({12,}));
        }
        
        // eine 0 ans array haengen, wenn es auch exits ohne category 
        // gibt (fuer das "Weiter" spaeter)
        string* all_cats = cats;
        if (member(categories, 0)>=0)
            all_cats += ({0});

        string text = "";
        foreach(string category: all_cats)
        {
            // alles exits dieser category holen
            string *cexits = map(filter(pairs, 
                (: $1[1]==$2 :), category), #'[, 0);

            if (sizeof(cexits))
                text += formatted_exits(category || "Weiter", 
                    cexits, cwidth, width - (graph?6:0));
        }
        
        if(graph)
            return sprintf("%-=5s %-=*s\n", graph, width - 6, text);
        else
            return text;
    }
    else
    {
        string graph = formatted_graphical_exits(&exits, 0);
        string text = sizeof(exits) ? 
            formatted_exits("Weiter", exits, graph?6:12, width - (graph?6:0))
            : "";

        if(graph)
            return sprintf("%-=5s %-=*s\n", graph, width - 6, text);
        else
            return text;
    }
}

private string query_room_description_wiz_exits(object room, int flag, int width)
{
    if (!query_wiz_level()) 
        return 0;

    string ret = "";
    string *exits;

    if(query_eye_option(EYE_HIDDEN_EXITS) &&
        sizeof(exits = room->query_command_list(EXIT_ATOM_HIDDEN)))
        ret += formatted_exits("Versteckt", exits, 12, width);

    if(query_eye_option(EYE_LOCKED_EXITS) &&
        sizeof(exits = room->query_command_list(EXIT_ATOM_LOCKED)))
        ret += formatted_exits("Gesperrt", exits, 12, width);

    if(query_eye_option(EYE_NOLIST_EXITS) &&
        sizeof(exits = room->query_command_list(EXIT_ATOM_NOLIST)))
        ret += formatted_exits("NoList", exits, 12, width);

    return ret;
}

private string query_room_description_inventory_visible
                        (object room, int flag, mapping attributes)
{
#ifdef AUTO_COUNTOB
    all_inventory(room)->undo_split();
#endif /* AUTO_COUNTOB */

    object * ob_list = all_inventory(room);
    if (!sizeof(ob_list))
        return 0;

    int displayed = 0; // Anzahl der angezeigten Items, wird auch als
                       // Flag (-1) verwendet, wenn es zu viel ist.

    object* items = attributes && attributes[MSG_MXP_ITEMS] || ({});
    int see_invis = query_wiz_level() && query_eye_option(EYE_ROOM_INVIS);
    int width = query_eye_description_width();
    string ret = "";

    for (int i = 0; i < sizeof(ob_list); i++)
    {
        object ob = ob_list[i];
        if ((ob == this_object()) && !(flag&EYE_SHOW_ME))
            continue; // Mich selber nicht mitzählen

        string short = ob->query_short(this_object());
        if (!short || (short == ""))
            continue; // Kein Short? Überspringen wir auch

        if (ob->query_invis() & V_ATOM_NOLIST)
        {
            if (!see_invis)
                continue;

            string name;
            if (playerp(ob))
                name = ob->query_real_cap_name();
            else
                name = ob->query_cap_name();

            if (stringp(name) && (name != ""))
                ret += wrap_say("("+name+")","", width);
            else
                ret += wrap_say("("+short+")","", width);
        }
        else
        {
            string txt = wrap_say(add_dot_to_msg(short),"",width);

            // Text noch zeilenweise mit MXP verzieren
            string * lines  = explode(txt, "\n")[0..<2];
            if (living(ob))
                lines = map(lines, (: MSG_LRCONTENT(sizeof(items), $1) :));
            else
                lines = map(lines, (: MSG_IRCONTENT(sizeof(items), $1) :));
            ret += implode(lines, "\n") + "\n";

            items += ({ ob_list[i] });
        }

        if (++displayed >= MAX_ROOM_INVENTORY_LIST)
        {
            // Abbruch bei zu viel Inventar
            displayed = -1;
            break;
        }
    }

    if (displayed == -1)
    {
        ret += eye_wrap("Bei diesem ganzen Chaos hier verlierst du "
            "doch glatt den Überblick!"+
            (query_wiz_level() ? " ("+sizeof(ob_list)+")":""), width);
    }
    if (attributes)
        attributes[MSG_MXP_ITEMS] = items;

    return ret;
}

private string query_room_description_inventory_dark(object room)
{
    // Lebewesen und Gegenstände zählen
    int people = 0;
    int things = 0;
    object * ob_list = all_inventory(room);
    foreach (object ob : ob_list)
    {
        if (ob == this_object())
            continue; // Mich selber nicht mitzählen

        if (ob->query_invis() & V_ATOM_NOLIST)
            continue; // No-List-Objekte überspringen

        string short = ob->query_short(this_object());
        if (!short || (short == ""))
            continue; // Kein Short? Überspringen wir auch

        if(living(ob))
            people++;
        else
            things++;
    }

    string ret = "";
    int width = query_eye_description_width();
    if(people)
    {
        ret += eye_wrap("In der dich umgebenden Finsternis erkennst du "+
            (people == 1 ? "eine Gestalt" : people+" Gestalten")+".", 
            width);
    }
    if(things)
    {
        if(things == 1)
            ret += eye_wrap("Hier befindet sich etwas.", width);
        else if(things <= 3)
            ret += eye_wrap("Du meinst hier "+things+
                " Gegenstände auszumachen.", width);
        else
            ret += eye_wrap("Hier befinden sich mehrere Dinge.", width);
    }
    return ret;
}

private string query_room_description_inventory
                        (object room, int flag, mapping attributes)
{
    if (flag & EYE_NO_CONTENTS)
        return 0;

    int dark = flag & EYE_DARK;
    if(dark)
    {
        // Nur eine vage Andeutung, dass hier was ist
        return query_room_description_inventory_dark(room);
    }
    else
    {
        // Komplette Auflistung des Rauminhalts
        return query_room_description_inventory_visible(
            room, flag, attributes);
    }
}

/*
FUNKTION: query_room_description
DEKLARATION: varargs string query_room_description(object room, int flag, mapping attributes)
BESCHREIBUNG:
query_room_description liefert eine volle Beschreibung des Raumes room, also
mit Auflistung aller Sichtbaren Ausgaenge und aller sichtbaren Objekte im
Raum. Dabei wird NICHT beachtet, ob es in dem Raum hell ist oder nicht. 
Ausserdem wird auch die Sehkraft des Monsters oder Spielers, in dem 
query_room_description aufgerufen wird, nicht beachtet. Sollen Licht- und
Sichtverhaeltnisse beruecksichtigt werden, muss describe_room oder
describe_current_room verwendet werden.

Mit Flag kann man festlegen, was genau gesehen wird. Mit den in eyes.h 
definierten defines, die verodert werden koennen, sieht das so aus:

EYE_NO_EXITS:        die Liste der Ausgaenge soll fehlen,
EYE_NO_EXIT_VIEW:    nicht in EXIT_VIEW-Ausgaenge schauen,
EYE_NO_CONTENTS:     die Liste des Inhalts soll fehlen,
EYE_NO_DESCRIPTION:  die Raumbeschreibung soll fehlen,
EYE_FORCE_LONG:      trotz gesetzter Kurzbeschreibung in den Augen soll die
                     Lange Beschreibung des Raumes geliefert werden,
EYE_DARK:
EYE_FORCE_DARK:      erzwinge Beschreibung, als ob es dunkel waere,
EYE_FORCE_LIGHT:     erzwinge Beschreibung, als ob es hell waere
                     (sollte nicht mit EYE_FORCE_DARK kombiniert werden)
EYE_SHOW_ME:	     auch das Lebewesen, welches schaut, selber mit auflisten.

Um mehrere Optionen gleichzeitig zu verwenden, muss sie logisch oder 
verknuepfen. Flag kann weggelassen werden.

In attributes (kann 0 sein) kann man Attribute fuer send_message uebergeben.

Beispiel: 
    Wenn monty ein Objektpointer auf Monty ist, liefert
    
    monty->query_room_description(touch("/w/monty/workroom"),
                EYE_FORCE_LONG|EYE_NO_CONTENTS|EYE_NO_EXITS)
    
    den String 
    "Du befindest Dich in Montys Arbeitszimmer. Die gesamte "+
    "Nordwand und\nund ein Teil der Westwand werden von einem Deckenhohen"+
		      .....
    "nach westen gelangt man\nin den Rest von Montys Wohnung. Nach oben "+
    "kann man in Montys Waffenkammer gehen.\n" 
    zurueck, also ohne Ausgaenge und ohne enthaltene Objekte, egal, ob Monty
    seine Augen auf Kurzbeschreibungen eingestellt hat oder nicht.

VERWEISE: describe_room, describe_current_room,
	  query_additional_room_description
GRUPPEN: licht, monster, spieler, augen
*/
varargs string query_room_description(object room, int flag, mapping attributes)
{
    int raumtyp, dark, me_shimmer;
    string tmp, ret, longtxt;
    int width = query_eye_description_width();
    int room_order = query_eye_option(EYE_ROOM_ORDER);

    ret=MSG_REXPIRE;

    dark = flag & EYE_DARK;

    if (room_order != 0)
    {
        tmp = query_room_description_inventory(room, flag, attributes);
        if (sizeof(tmp))
            ret += tmp;
    }

    /* List description of room */

    raumtyp = room->query_type (LANDSCHAFT);

    if(!(flag & EYE_NO_DESCRIPTION))
    {
       if(dark)
       {
          longtxt = "";
          if (tmp = room->query_long_dark(this_object()))
             longtxt += eye_wrap(tmp, width);
          else if (raumtyp &&
                ((raumtyp & (L_WASSER | L_FLIESSEND | L_UNTERWASSER)) == raumtyp))
             longtxt += eye_wrap(
                ({ "Du schwimmst in einer finsteren Brühe.",
                   "Mutig schwimmst du deines Weges, bei totaler Finsternis.",
                   "Etwas ratlos paddelst du in der Dunkelheit herum." })
                   [random(3)], width);
          else
             longtxt += eye_wrap(
                ({ "Ein schaurig dunkles Plätzchen.",
                   "Ein grausig finsterer Ort.",
                   "Eine bei Helligkeit viel angenehmere Stelle." })
                   [random(3)], width);
        
          if(tmp = this_object()->query_additional_room_description(room, flag))
             longtxt += eye_wrap(tmp, width);

          ret += MSG_RLONG(longtxt);
       }
       else if ((query_eye_option(EYE_KURZ)==1) && !(flag & EYE_FORCE_LONG))
       {
           if (tmp = room->query_short(this_object()))
               ret += wrap_say(MSG_RSHORT(add_dot_to_msg(tmp)),"", width);
       }
       else
       {
           longtxt = "";
           if ((query_eye_option(EYE_KURZ)==2) && 
               (tmp = room->query_short(this_object())))
               ret += wrap_say(MSG_RSHORT(add_dot_to_msg(tmp)),"", width);
           if ((IS_NIGHT) && (tmp = room->query_long_night(this_object())))
               longtxt += eye_wrap(tmp, width);
           else if (tmp = room->query_long(this_object()))
               longtxt += eye_wrap(tmp, width);
           if(tmp = this_object()->query_additional_room_description(room, flag))
               longtxt += eye_wrap(tmp, width);

           ret += MSG_RLONG(longtxt);
       }
    } else if (flag & EYE_FORCE_SHORT) {
        if (tmp = room->query_short(this_object()))
               ret += wrap_say(MSG_RSHORT(add_dot_to_msg(tmp)),"", width);
    }

    me_shimmer = (this_object()->query_invis() & V_ATOM_INVIS) &&
                !(this_object()->query_invis() & V_ATOM_NOSHIMMER) &&
                environment() == room ? 1 : 0;
    if(count_shimmer(room) - me_shimmer)
     ret+=eye_wrap((room->query_type(LANDSCHAFT)&L_UNTERWASSER)?
          "Das Wasser schimmert hier seltsam.":
          "Die Luft flimmert hier seltsam.", width);

    /* View in Nachbarraeume */

    if(!dark && !(flag & EYE_NO_EXIT_VIEW))
    {
        tmp = query_room_description_exit_view(room, flag);
        if (sizeof(tmp))
            ret += eye_wrap(tmp, width);
    }

    /* List exits of room */


    if(!(flag & EYE_NO_EXITS))
    {
        tmp = query_room_description_exits(room, flag, width);
        if (sizeof(tmp))
            ret += tmp;

        tmp = query_room_description_wiz_exits(room, flag, width);
        if (sizeof(tmp))
            ret += tmp;
    }

    /* List inventory of room */
    if (room_order == 0)
    {
        tmp = query_room_description_inventory(room, flag, attributes);
        if (sizeof(tmp))
            ret += tmp;
    }

    return ret;
}

/*
FUNKTION: query_additional_room_description
DEKLARATION: string query_additional_room_description(object room, int flag)
BESCHREIBUNG:
Diese Funktion wird im Lebewesen selber aufgerufen, wenn dieses den
Raum 'room' betrachtet, um zum Beispiel Shadows die Moeglichkeit zu geben,
der aktuellen Raumbeschreibung etwas hinzuzufuegen. 'flag' kann
die gleichen Werte wie bei query_room_description annehmen.
VERWEISE: query_room_description, describe_room
GRUPPEN: spieler, monster, augen
*/

/*
FUNKTION: describe_room
DEKLARATION: varargs string describe_room(object room, int flag, mapping attributes)
BESCHREIBUNG:
Verwendet query_room_description (siehe dort), um die Beschreibung des Raumes
room zurueckzuliefern, beachtet dabei aber Licht- und Sichtverhaeltnisse.
Flag kann weggelassen werden, Verwendung von Flag und Beispiel siehe bei
query_room_description.
In attributes (kann 0 sein) kann man Attribute fuer send_message uebergeben.
VERWEISE: query_room_description, describe_current_room
GRUPPEN: licht, spieler, monster, augen
*/
varargs string describe_room(object room, int flag, mapping attributes)
{
    /* Test light present */

    if ((flag & EYE_FORCE_LIGHT) || this_object()->can_see(room))
	return this_object()->query_room_description(room, flag, attributes);
    return this_object()->query_room_description(room, flag | EYE_DARK, attributes);
}

/*
FUNKTION: describe_current_room
DEKLARATION: varargs void describe_current_room(int flag)
BESCHREIBUNG:
Verwendet query_room_description (siehe dort), um die Beschreibung des Raumes
zurueckzuliefern, in dem sich der Spieler (oder das Monster) im Moment
befindet. Licht- und Sichtverhaeltnisse werden beachtet.
Flag kann weggelassen werden, Verwendung von Flag und Beispiel siehe bei
query_room_description.
VERWEISE: query_room_description, describe_room
GRUPPEN: licht, spieler, monster, augen
*/
varargs void describe_current_room(int flag)
{
   object room;
   string tmp, fname, out;
   string *res;

   if(!(room = environment()))
      notify_message("Im Nirwana gibt's nichts anzuschauen.\n",MA_LOOK);
   else
   {
      mapping attributes = ([:1]);
      if(this_object()->do_forbiddens(C_RESORT, "look",
	    ({"", "_me"}), ({this_object(), room})))
		return;

      if(query_wiz_level())
      {
         int width = query_eye_description_width();
	 if(query_eye_option(EYE_FILE))
	 {
	    if((tmp = map2domain((fname = object_name(room)),1)) &&
	       strstr(tmp,"/map/m"))
	       out = tmp[0..<3]+(file_size(tmp) > 0?"*":"")+" ("+fname+")";
	    else if((tmp = room->query_file_name()) && stringp(tmp))
		out = tmp[0..<3]+(file_size(tmp)> 0?"*":"")+" ("+fname+")";
	    else
	       out = fname;
	    notify_message(out+"\n",MA_LOOK);
	 }
	 if(query_eye_option(EYE_SHADOWS))
	 {
	    object ob = room;
	    int first = 1;
	    
	    out = "";
	    while(ob = shadow(ob, 0))
		out += sprintf("%-8s %-=*s\n", first?(first=0,"Shadows:"):"",
		    width - 9, object_name(ob));
	
	    if(strlen(out))
		notify_message(out,MA_LOOK);
	 }
	 if(query_eye_option(EYE_V_ITEMS))
	 {
	    res = map(room->query_all_v_items()||({}), #'[, "name") - ({0});
	    if(sizeof(res))
	       notify_message(sprintf("v_items: %=-*s\n", width - 9, implode(res,", ")),
	       	MA_LOOK);
	 }
      }
      this_object()->send_message_to(this_object(), MT_LOOK,MA_LOOK,
	    this_object()->describe_room(room, flag, attributes), attributes);

      this_object()->do_notifies(C_RESORT, "look",
	    ({"", "_me"}), ({this_object(), room}));
   }
}

/*
FUNKTION: query_contents
DEKLARATION: string query_contents(int flags, object viewer)
BESCHREIBUNG:
Diese Funktion kann definiert werden, um eine alternative Beschreibung
des Inhalts auszugeben. Sie wird auch bei nicht-Containern ausgewertet.

flags ist eine Bitmaske mit derzeit folgenden Bits (Defines in eyes.h):
    CONTENTS_SHOW_INVIS	    Unsichtbare Gegenstaende sollen angezeigt werden.
    CONTENTS_SHOW_CONTENTS  Es sollte eine tiefergehende Anzeige erfolgen.
                            (Wenn 'mustere' statt 'betrachte' verwendet wurde.)

Diese Funktion sollte einen bereits umgebrochenen Text zurueckliefern.
Liefert diese Funktion 0 zurueck, so wird die Standardanzeige ausgegeben.
Es muss damit gerechnet werden, dass viewer null ist.
VERWEISE: query_long, extra_look, query_object_description, query_obvious
GRUPPEN: spieler, monster, augen
*/

/*
FUNKTION: query_object_description
DEKLARATION: string query_object_description(mixed ob, int flags)
BESCHREIBUNG:
Liefert die Beschreibung eines Objektes zurueck, so wie der Spieler oder das
Monster das Objekt sieht. Dabei wird beruecksichtigt, ob das Objekt ein
offener Container ist (und in diesem Fall, ob was drin ist), wenn es lebt,
ob ein extra_look gesetzt ist, wenn es nicht lebt, ob es schwer ist usw...
Das angegeben Objekt darf ein object, ein mapping (v_item!), oder alles
andere sein, Erfolg ist allerdings dann nicht zu erwarten :-).
Licht- und Sichtverhaeltnisse werden nicht beachtet.

flags ist eine Bitmaske mit derzeit folgenden Bits (Defines in eyes.h):
    CONTENTS_SHOW_INVIS	    Unsichtbare Gegenstaende sollen angezeigt werden.
    CONTENTS_SHOW_CONTENTS  Es sollte eine tiefergehende Anzeige erfolgen.
                            Also z.B. auch der Inhalt.
                            (Wenn 'mustere' statt 'betrachte' verwendet wurde.)
VERWEISE: query_long, extra_look, query_transparent, query_contents,
          query_obvious
GRUPPEN: spieler, monster, augen
*/
string query_object_description(mixed ob, int flags)
{
   string ret, tmp, desc, content;
   int i, weight, obs, see_invis;
   object *ob_list;
   int is_plural;
   int width = query_eye_description_width();

   if ((IS_DAY) || (!(ret=QUERY_PARS("long_night",ob,({this_object()})))))
      if (!(ret=QUERY_PARS("long",ob,({this_object()}))))
         ret="Du siehst nichts Besonderes.\n";
   ret = eye_wrap(ret, width);

   /* List description of object */

   if(objectp(ob))
   {
      /* Kein virtuelles Objekt */

      is_plural = (plural("nein","ja",ob)=="ja");
      /* List weight classification */

      if(!living(ob))
	 if (stringp(tmp = ob->query_weight_description()))
		ret += eye_wrap(tmp, width); // brauchbar fuer countobs
	 else if (!ob->query_no_move() &&
		(weight = ob->query_weight()) < 1000 && weight > 2)
		ret += eye_wrap(Er(ob)+
		    (is_plural ? " scheinen " : " scheint ")+
		    (weight > 4 ? "sehr " : "")+"schwer zu sein.", width);

      /* List Extralook if living Object */

      if(living(ob) && stringp(tmp = ob->query_extra_look()))
	 ret += eye_wrap(tmp, width);

      /* List Content of Object */
      see_invis = query_wiz_level() && query_eye_option("otherinv") && CONTENTS_SHOW_INVIS;

#ifdef AUTO_COUNTOB
      all_inventory(ob)->undo_split(); // countobs in ordentlichen Zustand bringen
#endif /* AUTO_COUNTOB */

      if(stringp(content = ob->query_contents(see_invis|flags, this_object())))
         ret += eye_wrap(content, width);
      else if (ob->query_container())
      {
	 if((tmp = ob->query_content_message()) && tmp != "")
	    content = eye_wrap(tmp, width);
	 else
	    content = eye_wrap(living(ob) ?
		   "\t"+Er(ob)+" "+(is_plural?"tragen":"trägt")+" bei sich:" :
		   "\t"+Er(ob)+" "+(is_plural?"enthalten:":"enthält:"), width);


	 if ((!ob->query_con_close()) || ob->query_transparent())
         {
	   if((!ob->query_worn()) || ob->query_content_visible_when_worn())
	   {
	     if (living (ob) && !(flags & CONTENTS_SHOW_CONTENTS))
	     {
	        ob_list = filter_objects (all_inventory (ob),"query_wield");
		ob_list += determine_obvious_clothes(filter_objects(all_inventory(ob),"query_worn")) - ob_list;
	        ob_list += (filter_objects (all_inventory (ob),"query_obvious") - ob_list);
	     } else {
		ob_list = all_inventory(ob);
	     }
	     for(i = 0; i < sizeof(ob_list); i++)
	   	if(stringp(desc = ob_list[i]->query_short(this_object())))
		    if(ob_list[i]->query_invis() & V_ATOM_NOLIST)
		    {
		        if(see_invis)
			{
			    content += wrap_say("("+desc+")","",width);
			    obs++;
		 	}
		    }
		    else
		    {
		        content += wrap_say(add_dot_to_msg(desc),"",width);
		        obs++;
		    }
	   } else {
	     mixed woa;
	     woa = ob->query_worn_adjektiv ();
	     if (pointerp (woa)) woa = woa [0];
	     if (!woa) woa = "angezogen";
	     ret += eye_wrap("Da "+er(ob)+" "+woa+" "+(ob->query_plural()?"sind":"ist")
	        +", siehst du nicht, ob bzw. was "+er(ob)+" "+
	        (ob->query_plural()?"enthalten":"enthält")+".", width);
	   }
	 }
	 if(obs)
	    ret += content;
      }
   }
   return ret;
}

/*
FUNKTION: examine_object
DEKLARATION: varargs int examine_object(string str, int flags, closure message)
BESCHREIBUNG:
Diese Funktion implementiert den Betrachte-Befehl. Sie uebernimmt das Parsen
des Objekts der Begierde <str> und fuehrt ein Betrachten anhand von <flags>
aus. Flags kann dabei folgende Werte (Oder-verknuepft) haben:
    CONTENTS_SHOW_INVIS:    Unsichtbare Gegenstaende im Inventar sollen gezeigt
                            werden. (Nur fuer Goettertools)

    CONTENTS_SHOW_CONTENTS: Nicht offensichtliche Gegenstaende im Inventar
                            sollen gezeigt werden (entspricht dem Mustern).

    LOOK_SHORT:             Eine Kurzbeschreibung (ohne Long und Inventar)
                            soll ausgegeben werden.

    LOOK_COST:              Die ZP-Kosten fuer das Mustern sollen abgezogen
                            werden.

Als dritter Parameter kann optional eine Closure zur Meldungsausgabe angegeben
werden. Die Closure erhaelt als den Spieler als 1. Parameter und den betrachteten
Gegenstand (Object oder V-Item) als 2. Parameter und soll alle Meldungen ausgeben.
Falls die Closure nicht angegeben wurde, werden die Standardmeldungen erzeugt.

Die Funktion liefert 1, wenn das Befehl erfolgreich geparst werden konnte.
Ansonsten 0, dann wird eine notify_fail-Meldung gesetzt.
VERWEISE: query_object_description, query_long, query_contents, query_obvious
GRUPPEN: spieler, monster, augen
*/
varargs int examine_object(string str, int flags, closure message)
{
    string tmp, *res;
    mixed *parsed, ob, *v_items;
    object owner;
    int i, must;

#if 0
    string env;

    if (sscanf(str, "%s in %s", tmp, env))
    {
        parsed = parse_com(env);
        if (parse_com_error(parsed,"Was soll angeschaut werden?\n",1))
	    return 0;
        ob = parsed[PARSE_OBS][0];
        if (ob->query_con_closed() && !ob->query_transparent())
       	{
            notify_message(wrap("In " + den(ob) +
                " kannst du nicht reinschauen."));
            return 1;
        }
        parsed = parse_com(tmp, ob);
    }
    else
#endif

    parsed = parse_com(str);

    if (parse_com_error(parsed,"Was soll angeschaut werden?\n",1))
	return 0;

    if (sizeof(parsed[PARSE_REST]))
        return notify_fail("Was soll angeschaut werden?\n", FAIL_WRONG_ARG);
    
    ob = parsed[PARSE_OBS][0];

    if (!QUERY("visible_in_the_dark",ob))
        if (environment() && !this_object()->can_see(environment()))
        {
	    notify_message("Es ist zu dunkel.\n",MA_LOOK);
	    return 1;
        }

    if (objectp(ob) && ob->query_invis() & V_ATOM_INVIS && ob != this_object())
    {
	notify_message(capitalize(parsed[PARSE_ID])+" nicht gefunden.\n",
		MA_LOOK);
	return 1;
    }

    if (flags & CONTENTS_SHOW_CONTENTS)
    {
	must = 1;
    }

    if ((flags & LOOK_COST) && objectp(ob) && living(ob) && ob != this_object())
    {
       if (this_object()->query_sp() < MUSTER_COST) {
           notify_message("Du hast nicht genug "+this_object()->query_sp_name()+".\n");
           return 1;
       }
       this_object()->add_sp(-MUSTER_COST);
    }

    /* MELDUNG fuer OTHERS */

    if(this_object()->do_forbiddens(C_RESORT, "look",
	({"", "_me"}), ({this_object(), ob})))
	    return 1;

    if (!IS_HIDDEN(this_object()))
        if (message)
            funcall(message, this_object(), ob);
	else if (ob == this_object())
            this_object()->send_message(MT_LOOK,MA_LOOK,wrap(Der()+" "
            +(must?"mustert":"betrachtet")+" sich."));
        else if ((IS_NIGHT) && (tmp=QUERY("look_msg_night",ob)))
        {
            if (tmp != "")
                this_object()->send_message(MT_LOOK,MA_LOOK,wrap(tmp));
        }
        else if (tmp=QUERY("look_msg",ob))
	{
	    if (tmp != "")
		this_object()->send_message(MT_LOOK,MA_LOOK,wrap(tmp));
	}
	else if (must)
	    this_object()->send_message(MT_LOOK,MA_LOOK,
		    (QUERY("invis",ob) & V_ATOM_HIDDEN) ?
		    wrap(Der()+" schaut etwas an.") :
		    wrap(Der()+" mustert "+seinen(ob)+"."),
		    wrap(Der()+" mustert dich."), objectp(ob) && ob);
	else if (QUERY("invis",ob) & V_ATOM_HIDDEN)
	    this_object()->send_message(MT_LOOK,MA_LOOK,
		    wrap(Der()+" schaut etwas an."),
		    wrap(Der()+" schaut dich an."), objectp(ob) && ob);
	else if ((owner = auto_owner_search(ob)) && owner && owner != this_object())
	     this_object()->send_message(MT_LOOK,MA_LOOK,
		    wrap(Der()+" schaut "+ihren(ob)+" an."),
		    wrap(Der()+" schaut "+deinen(ob)+" an."),
		    owner);
	else  if(objectp(ob))
	    this_object()->send_message(MT_LOOK,MA_LOOK,
		    wrap(Der() +" schaut " + seinen(ob) + " an."),
		    wrap(Der() +" schaut dich an."), ob);
	else
	    this_object()->send_message(MT_LOOK,MA_LOOK,
		    wrap(Der() +" schaut " + seinen(ob) + " an."));

    /* MELDUNGEN fuer THIS_PLAYER() */

    if (flags & LOOK_SHORT) 
        this_object()->send_message_to(this_object(),MT_LOOK,MA_LOOK,
            add_dot_to_msg(QUERY("short", ob) || Ein(ob))+"\n"+
            (QUERY("hp_string", ob) || ""));
    else
    {
        if (objectp(ob) && query_wiz_level())
        {
            if (query_eye_option("file"))
                notify_message(object_name(ob)+"\n",MA_LOOK);
	    if(query_eye_option("shadows"))
	    {
	        object sh = ob;
	        int first = 1;
	    
	        tmp="";
	    
	        while(sh = shadow(sh, 0))
		    tmp+=sprintf("%-8s %-=70s\n", first?(first=0,"Shadows:"):
                                 "", object_name(sh));
	        if(strlen(tmp))
		    notify_message(tmp, MA_LOOK);
	    }
            if (query_eye_option("v_items"))
       	    {
	        for (i = sizeof(v_items=ob->query_all_v_items()),res=({}); 
                                                                        i--;)
		    if (tmp = v_items[i]["name"])
		        res += ({ tmp });
	        if (sizeof(res))
		    notify_message(sprintf("v_items: %=-64s\n",
                                           implode(res,", ")), MA_LOOK);
            }
        }

        if (objectp(ob) && playerp(this_object()))
	    this_object()->more( ({this_object()->query_object_description(ob,flags) }),
		0,0,M_AUTO_END|M_TYPE(MT_LOOK)|M_ACTION(MA_LOOK));
        else
	    this_object()->send_message_to(this_object(),MT_LOOK,MA_LOOK,
		this_object()->query_object_description(ob,flags));
    }

    this_object()->do_notifies(C_RESORT, "look",
	({"", "_me"}), ({this_object(), ob}));

    return 1;
}

int look(string str) {
    int kurz;
    string tmp;

    kurz=(query_verb_ascii(1) == "beaeuge") && LOOK_SHORT;

    if (!str && kurz) {
        describe_current_room(
            EYE_NO_EXIT_VIEW|
            EYE_NO_DESCRIPTION|
            EYE_FORCE_SHORT);
        return 1;
    }

    if (!str && !kurz) {
	describe_current_room(EYE_FORCE_LONG);
	return 1;
	}
    if (query_verb()[0..4] != "schau")
        return this_object()->examine_object(str,kurz);

    if (sscanf(str, "auf %s", tmp)   ||
	sscanf(str, "in %s", tmp)    ||
        sscanf(str, "durch %s", tmp) ||
	sscanf(str+"\n", "%s an\n",tmp))
	return this_object()->examine_object(tmp,kurz);

    notify_fail("Schau was an? (oder betrachte was?)\n");
    return 0;
}

int examine(string str) {
    return this_object()->examine_object(str,CONTENTS_SHOW_CONTENTS | LOOK_COST);
}

int reading(string str)
{
    string text;
    mixed *parsed, ob,owner;

    if (environment() && !this_object()->can_see(environment()))
    {
	notify_message("Es ist zu dunkel.\n",MA_LOOK);
	return 1;
    }
    
#ifdef NEW_STATS
    if (this_object()->query_stat(STAT_INT) < MIN_INT_READ)
    {
        notify_fail("Dazu bist du nicht intelligent genug.\n");
        return 0;
    }
#endif
    
    parsed = parse_com(str);
    if (parse_com_error(parsed,"Lies was?\n",1))
	return 0;

    ob = parsed[PARSE_OBS][0];

    if(this_object()->do_forbiddens(C_RESORT, "read",
	({"", "_me"}), ({this_object(), ob})))
	    return 1;

    if (mappingp(ob))
	if (closurep(ob["read"]))
	    text = funcall(ob["read"],parsed[PARSE_REST],str,ob,this_object());
	else
	    text = ob["read"];
    else
	text = ob->query_read(parsed[PARSE_REST],str, this_object());

    if (!text)
    {
	this_object()->send_message_to(this_object(),MT_LOOK,MA_READ,
		"Da gibt es nichts zu lesen.\n");
	return 1;
    }
    this_object()->send_message_to(this_object(),MT_LOOK,MA_READ,text);

    if (!IS_HIDDEN(this_object()))
    {
        if (text = QUERY("read_msg",ob))
        {
            if (text != "")
                this_object()->send_message(MT_LOOK,MA_READ,wrap(text));
        }
        else if (!objectp(ob) || !ob->query_book())
        {
            // Das Buch gibt selber Meldungen aus.
            if ((owner = auto_owner_search(ob))
                && owner && owner != this_object())
                this_object()->send_message(MT_LOOK,MA_LOOK,
                wrap(Der()+" liest "+ihren(ob)+"."),
                wrap(Der()+" liest "+deinen(ob)+"."),
                owner);
        else 
            this_object()->send_message(MT_LOOK,MA_READ,
                wrap(Der() + " liest " + seinen(ob) + "."));
        }
    }

    this_object()->do_notifies(C_RESORT, "read",
	({"", "_me"}), ({this_object(), ob}));

    return 1;
}

void just_moved() { describe_current_room(); }

/*
FUNKTION: query_read_msg
DEKLARATION: string query_read_msg()
BESCHREIBUNG:
Diese Funktion wird in einem Objekt aufgerufen, wenn this_player() das
Objekt zu lesen beginnt. Liefert sie 0 zurueck, wird eine Standardmeldung
generiert, ansonsten wird diese Meldung an alle anderen im Raum
gesendet.
VERWEISE: set_read, query_read
GRUPPEN: grundlegendes
*/

/*
FUNKTION: query_look_msg
DEKLARATION: string query_look_msg()
BESCHREIBUNG:
Diese Funktion wird in einem Objekt aufgerufen, wenn this_player() das
Objekt ansieht. Falls es Nacht ist, wird sie nur aufgerufen, wenn
query_look_msg_night() 0 lieferte (z.B. weil sie nicht existiert).
Liefert diese Funktion 0 zurueck, wird die Standardmeldung
"<TP> schaut <Objekt> an." generiert.
VERWEISE: query_look_msg_night, set_long, query_long
GRUPPEN: grundlegendes
*/

/*
FUNKTION: query_look_msg_night
DEKLARATION: string query_look_msg_night()
BESCHREIBUNG:
Diese Funktion wird in einem Objekt aufgerufen, wenn this_player() das
Objekt Nachts ansieht. Liefert es 0, so wird die Meldung von query_look_msg
oder, falls diese auch 0 ist, die Standardmeldung ausgegeben.
Anderenfalls erhalten alle im Raum die zurueckgelieferte Meldung.
VERWEISE: query_look_msg, set_long, query_long
GRUPPEN: grundlegendes
*/

/*
FUNKTION: query_obvious
DEKLARATION: int query_obvious()
BESCHREIBUNG:
Liefert ein Gegenstand beim Aufruf von query_obvious einen von 0 verschiedenen
Wert so wird der Gegenstand beim Betrachten eines Lebewesens angezeigt, auch
wenn er weder angezogen noch gefuehrt ist.
Sinnvoll ist dies bei allen Gegenstaenden, die so auffaellig sind, dass
man sie beim Betrachten sieht wie Leitern oder grosse Kisten, die man mit
sich rumtraegt.
VERWEISE: query_contents, query_object_description
GRUPPEN: monster, spieler, augen
*/

/*
FUNKTION: notify_look
DEKLARATION: void notify_look(mixed what, object who)
BESCHREIBUNG:
Nachdem das Lebewesen 'who' das Objekt, V-item oder Raum(!) what
betrachtet hat, wird notify_look(what, who) an allen bei who
angemeldeten Controllern aufgerufen.
VERWEISE: notify, notify_look_me, notify_look_here
GRUPPEN: spieler, monster, augen
*/
/*
FUNKTION: notify_look_me
DEKLARATION: void notify_look_me(object who, mixed what)
BESCHREIBUNG:
Nachdem das Lebewesen 'who' das Objekt, V-item oder Raum(!) what
betrachtet hat, wird notify_look_me(who, what) an allen bei what
angemeldeten Controllern aufgerufen.
VERWEISE: notify, notify_look, notify_look_here
GRUPPEN: spieler, monster, augen
*/
/*
FUNKTION: notify_look_here
DEKLARATION: void notify_look_here(object who, mixed what)
BESCHREIBUNG:
Nachdem das Lebewesen 'who' das Objekt, V-item oder Raum(!) what
betrachtet hat, wird notify_look_here(who, what) an allen im
Raum von who angemeldeten Controllern aufgerufen.
VERWEISE: notify, notify_look, notify_look_me
GRUPPEN: spieler, monster, augen
*/

/*
FUNKTION: notify_read
DEKLARATION: void notify_read(mixed what, object who)
BESCHREIBUNG:
Nachdem das Lebewesen 'who' das Objekt oder V-item what zu lesen
begonnen hat, wird notify_read(what, who) an allen bei who
angemeldeten Controllern aufgerufen.
VERWEISE: notify, notify_read_me, notify_read_here
GRUPPEN: spieler, monster, augen
*/
/*
FUNKTION: notify_read_me
DEKLARATION: void notify_read_me(object who, mixed what)
BESCHREIBUNG:
Nachdem das Lebewesen 'who' das Objekt oder V-item what zu lesen
begonnen hat, wird notify_read_me(who, what) an allen bei what
angemeldeten Controllern aufgerufen.
VERWEISE: notify, notify_read, notify_read_here
GRUPPEN: spieler, monster, augen
*/
/*
FUNKTION: notify_read_here
DEKLARATION: void notify_read_here(object who, mixed what)
BESCHREIBUNG:
Nachdem das Lebewesen 'who' das Objekt oder V-item what zu lesen
begonnen hat, wird notify_read_here(who, what) an allen im
Raum von who angemeldeten Controllern aufgerufen.
VERWEISE: notify, notify_read, notify_read_me
GRUPPEN: spieler, monster, augen
*/

/*
FUNKTION: forbidden_look
DEKLARATION: int forbidden_look(mixed what, object who)
BESCHREIBUNG:
Bevor das Lebewesen 'who' das Objekt, V-item oder Raum(!) what
betrachten will, wird forbidden_look(what, who) an allen bei who
angemeldeten Controllern abgefragt.

Sobald einer der aufgerufenen Funktionen einen Wert ungleich 0
zurueckliefert, kann what nicht betrachtet werden. Fuer die
Ausgabe einer Meldung an das Lebewesen who hat der jeweilige
Controller zu sorgen.
VERWEISE: forbidden, forbidden_look_me, forbidden_look_here
GRUPPEN: spieler, monster, augen
*/
/*
FUNKTION: forbidden_look_me
DEKLARATION: int forbidden_look_me(object who, mixed what)
BESCHREIBUNG:
Bevor das Lebewesen 'who' das Objekt, V-item oder Raum(!) what
betrachten will, wird forbidden_look_me(who, what) an allen bei what
angemeldeten Controllern abgefragt.

Sobald einer der aufgerufenen Funktionen einen Wert ungleich 0
zurueckliefert, kann what nicht betrachtet werden. Fuer die
Ausgabe einer Meldung an das Lebewesen who hat der jeweilige
Controller zu sorgen.
VERWEISE: forbidden, forbidden_look, forbidden_look_here
GRUPPEN: spieler, monster, augen
*/
/*
FUNKTION: forbidden_look_here
DEKLARATION: int forbidden_look_here(object who, mixed what)
BESCHREIBUNG:
Bevor das Lebewesen 'who' das Objekt, V-item oder Raum(!) what
betrachten will, wird forbidden_look_here(who, what) an allen im
Raum von who angemeldeten Controllern abgefragt.

Sobald einer der aufgerufenen Funktionen einen Wert ungleich 0
zurueckliefert, kann what nicht betrachtet werden. Fuer die
Ausgabe einer Meldung an das Lebewesen who hat der jeweilige
Controller zu sorgen.
VERWEISE: forbidden, forbidden_look, forbidden_look_me
GRUPPEN: spieler, monster, augen
*/

/*
FUNKTION: forbidden_read
DEKLARATION: int forbidden_read(mixed what, object who)
BESCHREIBUNG:
Bevor das Lebewesen 'who' das Objekt oder V-item what lesen will,
wird forbidden_read(what, who) an allen bei who angemeldeten
Controllern abgefragt.

Sobald einer der aufgerufenen Funktionen einen Wert ungleich 0
zurueckliefert, kann what nicht gelesen werden. Fuer die
Ausgabe einer Meldung an das Lebewesen who hat der jeweilige
Controller zu sorgen.
VERWEISE: forbidden, forbidden_read_me, forbidden_read_here
GRUPPEN: spieler, monster, augen
*/
/*
FUNKTION: forbidden_read_me
DEKLARATION: int forbidden_read_me(object who, mixed what)
BESCHREIBUNG:
Bevor das Lebewesen 'who' das Objekt oder V-item what lesen will,
wird forbidden_read_me(who, what) an allen bei what angemeldeten
Controllern abgefragt.

Sobald einer der aufgerufenen Funktionen einen Wert ungleich 0
zurueckliefert, kann what nicht gelesen werden. Fuer die
Ausgabe einer Meldung an das Lebewesen who hat der jeweilige
Controller zu sorgen.
VERWEISE: forbidden, forbidden_read, forbidden_read_here
GRUPPEN: spieler, monster, augen
*/
/*
FUNKTION: forbidden_read_here
DEKLARATION: int forbidden_read_here(object who, mixed what)
BESCHREIBUNG:
Bevor das Lebewesen 'who' das Objekt oder V-item what lesen will,
wird forbidden_read_here(who, what) an allen im Raum von who
angemeldeten Controllern abgefragt.

Sobald einer der aufgerufenen Funktionen einen Wert ungleich 0
zurueckliefert, kann what nicht gelesen werden. Fuer die
Ausgabe einer Meldung an das Lebewesen who hat der jeweilige
Controller zu sorgen.
VERWEISE: forbidden, forbidden_read, forbidden_read_me
GRUPPEN: spieler, monster, augen
*/

/* --- add_actions: --- */

protected void add_actions()
{
    add_action("reading",     "lese",           -3);
    add_action("reading",     "lies");
    add_action("look",        "schaue",         -5);
    add_action("look",        "betrachte");
    add_action("look",        "beäuge",         -5);
    add_action("examine",     "mustere",        -6);
    add_action("examine",     "mustre");
}

/* --- End of file. --- */
