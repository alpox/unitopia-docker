// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /apps/properties.c
// Description: Property-Verwaltung
// Author:      Gnomi

inherit "/i/tools/property_master";
inherit "/i/item/control";

#include <property_master.h>
#include <properties.h>
#include <description.h>
#include <apps.h>
#include <soundcheck.h>

void create()
{
    set_property_info(P_LOOK_MSG, ([
        PI_VALUE:      0,
        PI_FLAGS:      0,

        PI_TYPES:      PT_STRING|PT_CLOSURE,
        PI_USES:       PU_MOVABLE|PU_VITEM,

        PI_SHORTNAME: "look_msg",

        PI_WIZ_SHORT: "Meldung beim Betrachten für alle anderen.",
        PI_WIZ_LONG:  "Die Meldung, die alle anderen im Raum erhalten, "
                      "wenn ein Lebewesen diesen Gegenstand betrachtet. "
                      "Erlaubt sind Strings, Closures oder Pseudoclosures. "
                      "Closures erhalten den Gegenstand als ersten Parameter, "
                      "Pseudoclosures erhalten ihn als OBJ_TO. Das Lebewesen "
                      "ist this_player() bzw. OBJ_TP. "
                      "Bei V-Items wird das virtuelle Objekt als erster, "
                      "und der Gegenstand als zweiter Parameter übergeben. "
                      "Pseudoclosures erhalten das V-Item als 'vitem.",

        PI_SET_FUN:   "set_pseudoclosure",
        PI_GET_FUN:   "get_pseudoclosure",
        PI_ADD_FUN:   "invalid_add",
        PI_DEL_FUN:   "invalid_del",

        PI_VITEM_SET_FUN: "set_vitem_pseudoclosure",
        PI_VITEM_GET_FUN: "get_vitem_pseudoclosure",
        PI_VITEM_ADD_FUN: "invalid_add",
        PI_VITEM_DEL_FUN: "invalid_del",
    ]));

    set_property_info(P_READ_MSG, ([
        PI_VALUE:      0,
        PI_FLAGS:      0,

        PI_TYPES:      PT_STRING|PT_CLOSURE,
        PI_USES:       PU_MOVABLE|PU_VITEM,

        PI_SHORTNAME: "read_msg",

        PI_WIZ_SHORT: "Meldung beim Lesen für alle anderen.",
        PI_WIZ_LONG:  "Die Meldung, die alle anderen im Raum erhalten, "
                      "wenn ein Lebewesen diesen Gegenstand zu lesen beginnt. "
                      "Erlaubt sind Strings, Closures oder Pseudoclosures. "
                      "Closures erhalten den Gegenstand als ersten Parameter, "
                      "Pseudoclosures erhalten ihn als OBJ_TO. Das Lebewesen "
                      "ist this_player() bzw. OBJ_TP. "
                      "Bei V-Items wird das virtuelle Objekt als erster, "
                      "und der Gegenstand als zweiter Parameter übergeben. "
                      "Pseudoclosures erhalten das V-Item als 'vitem.",

        PI_SET_FUN:   "set_pseudoclosure",
        PI_GET_FUN:   "get_pseudoclosure",
        PI_ADD_FUN:   "invalid_add",
        PI_DEL_FUN:   "invalid_del",

        PI_VITEM_SET_FUN: "set_vitem_pseudoclosure",
        PI_VITEM_GET_FUN: "get_vitem_pseudoclosure",
        PI_VITEM_ADD_FUN: "invalid_add",
        PI_VITEM_DEL_FUN: "invalid_del",
    ]));

    set_property_info(P_SMELL_MSG, ([
        PI_VALUE:      0,
        PI_FLAGS:      0,

        PI_TYPES:      PT_STRING|PT_CLOSURE,
        PI_USES:       PU_MOVABLE|PU_VITEM|PU_ROOM,

        PI_SHORTNAME: "smell_msg",

        PI_WIZ_SHORT: "Meldung beim Riechen für alle anderen.",
        PI_WIZ_LONG:  "Die Meldung, die alle anderen im Raum erhalten, wenn "
                      "ein Lebewesen an diesem Gegenstand oder im Raum riecht. "
                      "Erlaubt sind Strings, Closures oder Pseudoclosures. "
                      "Closures erhalten den Gegenstand als ersten Parameter, "
                      "Pseudoclosures erhalten ihn als OBJ_TO. Das Lebewesen "
                      "ist this_player() bzw. OBJ_TP. "
                      "Bei V-Items wird das virtuelle Objekt als erster, "
                      "und der Gegenstand als zweiter Parameter übergeben. "
                      "Pseudoclosures erhalten das V-Item als 'vitem.",

        PI_SET_FUN:   "set_pseudoclosure",
        PI_GET_FUN:   "get_pseudoclosure",
        PI_ADD_FUN:   "invalid_add",
        PI_DEL_FUN:   "invalid_del",

        PI_VITEM_SET_FUN: "set_vitem_pseudoclosure",
        PI_VITEM_GET_FUN: "get_vitem_pseudoclosure",
        PI_VITEM_ADD_FUN: "invalid_add",
        PI_VITEM_DEL_FUN: "invalid_del",
    ]));

    set_property_info(P_HEAR_MSG, ([
        PI_VALUE:      0,
        PI_FLAGS:      0,

        PI_TYPES:      PT_STRING|PT_CLOSURE,
        PI_USES:       PU_MOVABLE|PU_VITEM|PU_ROOM,

        PI_SHORTNAME: "hear_msg",

        PI_WIZ_SHORT: "Meldung beim Horchen für alle anderen.",
        PI_WIZ_LONG:  "Die Meldung, die alle anderen im Raum erhalten, wenn "
                      "ein Lebewesen an diesem Gegenstand oder im Raum horcht. "
                      "Erlaubt sind Strings, Closures oder Pseudoclosures. "
                      "Closures erhalten den Gegenstand als ersten Parameter, "
                      "Pseudoclosures erhalten ihn als OBJ_TO. Das Lebewesen "
                      "ist this_player() bzw. OBJ_TP. "
                      "Bei V-Items wird das virtuelle Objekt als erster, "
                      "und der Gegenstand als zweiter Parameter übergeben. "
                      "Pseudoclosures erhalten das V-Item als 'vitem.",

        PI_SET_FUN:   "set_pseudoclosure",
        PI_GET_FUN:   "get_pseudoclosure",
        PI_ADD_FUN:   "invalid_add",
        PI_DEL_FUN:   "invalid_del",

        PI_VITEM_SET_FUN: "set_vitem_pseudoclosure",
        PI_VITEM_GET_FUN: "get_vitem_pseudoclosure",
        PI_VITEM_ADD_FUN: "invalid_add",
        PI_VITEM_DEL_FUN: "invalid_del",
    ]));

    set_property_info(P_ATTACK_MSG, ([
        PI_VALUE:      0,
        PI_FLAGS:      0,

        PI_TYPES:      PT_STRING|PT_CLOSURE,
        PI_USES:       PU_MOVABLE|PU_VITEM,

        PI_SHORTNAME: "attack_msg",

        PI_WIZ_SHORT: "Meldung beim Angriff eines Gegenstands für alle anderen.",
        PI_WIZ_LONG:  "Die Meldung, die alle anderen im Raum erhalten, "
                      "wenn ein Lebewesen diesen Gegenstand angreifen versucht. "
                      "Diese Eigenschaft wird nur bei Gegenständen, "
                      "nicht bei Lebewesen abgefragt. "
                      "Erlaubt sind Strings, Closures oder Pseudoclosures. "
                      "Closures erhalten den Gegenstand als ersten Parameter, "
                      "Pseudoclosures erhalten ihn als OBJ_TO. Das Lebewesen "
                      "ist this_player() bzw. OBJ_TP. "
                      "Bei V-Items wird das virtuelle Objekt als erster, "
                      "und der Gegenstand als zweiter Parameter übergeben. "
                      "Pseudoclosures erhalten das V-Item als 'vitem.",

        PI_SET_FUN:   "set_pseudoclosure",
        PI_GET_FUN:   "get_pseudoclosure",
        PI_ADD_FUN:   "invalid_add",
        PI_DEL_FUN:   "invalid_del",

        PI_VITEM_SET_FUN: "set_vitem_pseudoclosure",
        PI_VITEM_GET_FUN: "get_vitem_pseudoclosure",
        PI_VITEM_ADD_FUN: "invalid_add",
        PI_VITEM_DEL_FUN: "invalid_del",
    ]));

    set_property_info(P_FEEL_MSG, ([
        PI_VALUE:      0,
        PI_FLAGS:      0,

        PI_TYPES:      PT_STRING|PT_CLOSURE,
        PI_USES:       PU_MOVABLE|PU_VITEM|PU_ROOM,

        PI_SHORTNAME: "feel_msg",

        PI_WIZ_SHORT: "Meldung beim Fühlen für alle anderen.",
        PI_WIZ_LONG:  "Die Meldung, die alle anderen im Raum erhalten, wenn "
                      "ein Lebewesen diesen Gegenstand oder im Raum fühlt. "
                      "Erlaubt sind Strings, Closures oder Pseudoclosures. "
                      "Closures erhalten den Gegenstand als ersten Parameter, "
                      "Pseudoclosures erhalten ihn als OBJ_TO. Das Lebewesen "
                      "ist this_player() bzw. OBJ_TP. "
                      "Bei V-Items wird das virtuelle Objekt als erster, "
                      "und der Gegenstand als zweiter Parameter übergeben. "
                      "Pseudoclosures erhalten das V-Item als 'vitem.",

        PI_SET_FUN:   "set_pseudoclosure",
        PI_GET_FUN:   "get_pseudoclosure",
        PI_ADD_FUN:   "invalid_add",
        PI_DEL_FUN:   "invalid_del",

        PI_VITEM_SET_FUN: "set_vitem_pseudoclosure",
        PI_VITEM_GET_FUN: "get_vitem_pseudoclosure",
        PI_VITEM_ADD_FUN: "invalid_add",
        PI_VITEM_DEL_FUN: "invalid_del",
    ]));

    set_property_info(P_TAKE_MSG, ([
        PI_VALUE:      0,
        PI_FLAGS:      0,

        PI_TYPES:      PT_STRING|PT_CLOSURE,
        PI_USES:       PU_VITEM,

        PI_SHORTNAME: "take_msg",

        PI_WIZ_SHORT: "Meldung beim Nehmen für alle anderen.",
        PI_WIZ_LONG:  "Die Meldung, die alle anderen im Raum erhalten, "
                      "wenn ein Lebewesen dieses V-Item aufnehmen will. "
                      "Erlaubt sind Strings, Closures oder Pseudoclosures. "
                      "Closures erhalten das virtuelle Objekt als ersten "
                      "und das Objekt als zweiten Parameter, "
                      "Pseudoclosures erhalten das V-Item als 'vitem'. "
                      "Das Lebewesen ist this_player() bzw. OBJ_TP.",

        PI_VITEM_SET_FUN: "set_vitem_pseudoclosure",
        PI_VITEM_GET_FUN: "get_vitem_pseudoclosure",
        PI_VITEM_ADD_FUN: "invalid_add",
        PI_VITEM_DEL_FUN: "invalid_del",
    ]));

    set_property_info(P_NAHRUNG, ([
        PI_VALUE:     0,
        PI_FLAGS:     PF_GET_NOMASK|PF_NO_CLOSURE,

        PI_TYPES:     PT_INT,

        PI_WIZ_SHORT: "Macht den Gegenstand essbar.",
        PI_WIZ_LONG:  "Wird ein Wert != 0 gesetzt, so wird dieser "
                      "Gegenstand essbar. "
                      "Dies dient nur Demonstrationszwecken!",

        PI_SET_FUN:   "make_food",
        PI_ADD_FUN:   "invalid_add",
        PI_DEL_FUN:   "invalid_del",
    ]));

    set_property_info(P_KEEP_OR_SELL, ([
        PI_VALUE:     0,
        PI_FLAGS:     PF_NO_CLOSURE|PF_PERSISTANT,

        PI_TYPES:     PT_INT,
        PI_USES:      PU_ITEM,

        PI_WIZ_SHORT: "Behalten eines verkaufbaren Gegenstandes.",
        PI_WIZ_LONG:  "Wird ein Wert != 0 gesetzt, so wird dieser "
                      "Gegenstand nicht mitverkauft.",

        PI_ADD_FUN:   "invalid_add",
        PI_DEL_FUN:   "invalid_del",
    ]));

    set_property_info(P_DONT_SELL_CONTENT, ([
        PI_VALUE:     0,
        PI_FLAGS:     PF_NO_CLOSURE|PF_PERSISTANT,

        PI_TYPES:     PT_INT,
        PI_USES:      PU_ITEM,

        PI_WIZ_SHORT: "Inhalt des Containers nicht verkaufen.",
        PI_WIZ_LONG:  "Wird ein Wert != 0 gesetzt, so wird der "
                      "Inhalt beim Verkauf nicht bewertet und nicht "
                      "vom Container getrennt.",

        PI_ADD_FUN:   "invalid_add",
        PI_DEL_FUN:   "invalid_del",
    ]));

    set_property_info(P_CRACK_CHANCES, ([
        PI_VALUE:     ([]),
        PI_FLAGS:     PF_NO_CLOSURE,

        PI_TYPES:     PT_MAPPING,
        PI_USES:      PU_ITEM,
        
        PI_WIZ_SHORT: "Speichert die Crack-Wahrscheinlichkeiten.",
        PI_WIZ_LONG:  "Speichert die Wahrscheinlichkeiten der Kategorien "
                      "CRACK_BOMB, CRACK_LOCK, CRACK_MAGIC, CRACK_DEFAULT. "
                      "Ist kein CRACK_DEFAULT angegeben, so wird query_crack "
                      "genutzt.",

        PI_SET_FUN:   "set_crack_chances",
        PI_GET_FUN:   "get_crack_chances",
        PI_ADD_FUN:   "add_crack_chances",
        PI_DEL_FUN:   "del_crack_chances",
    ]));

    set_property_info(P_DEBUG_INFO, ([
        PI_VALUE:     ([]),
        PI_FLAGS:     PF_NO_CLOSURE,

        PI_TYPES:     PT_MAPPING,
        PI_USES:      PU_OBJECT|PU_VITEM,
        
        PI_WIZ_SHORT: "Speichert debug infos für RTEs/fehler/ideen usw.",
        PI_WIZ_LONG:  "Speichert die debuginfos (Mapping) zusätzlich zu "
                      "dem query_debug_info. Als Schlüssel werden Strings, "
                      "als Werte Strings oder Zahlen akzeptiert.",

        PI_SET_FUN:   "set_debug_info",
        PI_GET_FUN:   "get_debug_info",
        PI_ADD_FUN:   "add_debug_info",
        PI_DEL_FUN:   "del_debug_info",
    ]));

    set_property_info(P_DEBUG_GROUP, ([
        PI_VALUE:     ({}),
        PI_FLAGS:     PF_NO_CLOSURE,

        PI_TYPES:     PT_ARRAY,
        PI_USES:      PU_OBJECT|PU_VITEM,
        
        PI_WIZ_SHORT: "Speichert zus.Debuggruppen für RTEs/fehler/ideen usw.",
        PI_WIZ_LONG:  "Speichert zusätzliche (bzgl ACLs) Debuggruppen "
                      "als Stringarray.",

        PI_SET_FUN:   "set_debug_group",
        PI_GET_FUN:   "get_debug_group",
        PI_ADD_FUN:   "add_debug_group",
        PI_DEL_FUN:   "del_debug_group",
    ]));
    set_property_info(P_CONSERVATION, ([
        PI_VALUE:     ([]),
        PI_FLAGS:     PF_NO_CLOSURE,

        PI_TYPES:     PT_MAPPING,
        PI_USES:      PU_ITEM|PU_MONSTER,
        
        PI_WIZ_SHORT: "Speichert Daten zur Armageddon-sicheren Einlagerung.",
        PI_WIZ_LONG:  "Folgende Schlüssel sind verfügbar: "
                      "P_CONSERVATION_PRECHECK(string) Überlagerung des "
                      "Standardtextes, wenn es nicht eingelagert werden kann. "
                      "P_CONSERVATION_FACTORY(string) Datei, die das Objekt "
                      "erzeugt, falls es selbst die Funktion hat. "
                      "P_CONSERVATION_IDENTIFIER(string) Identifier für die "
                      "Factory, um das Objekt zu erzeugen. "
                      "P_CONSERVATION_ITEM_TARIFF (float,taler) Preis des "
                      "Gegenstandes, wenne r abweicht vom Standard (10%). "
                      "P_CONSERVATION_TROPHEY (0/1): wenn 1 wird der "
                      "Sondertarif für Trophaene gezogen.",

        PI_SET_FUN:   "set_conservation",
        PI_GET_FUN:   "get_conservation",
        PI_ADD_FUN:   "add_conservation",
        PI_DEL_FUN:   "del_conservation",
    ]));
    set_property_info(P_ORIGIN, ([
        PI_VALUE:     ([]),
        PI_FLAGS:     PF_NO_CLOSURE|PF_GET_NOMASK|PF_SET_NOMASK|PF_SINGLE_SHOT,

        PI_TYPES:     PT_MAPPING,
        PI_USES:      PU_ITEM|PU_MONSTER,
        
        PI_WIZ_SHORT: "Speichert First_room/player usw. bei den Schließfächern.",
        PI_WIZ_LONG:  "Folgende Schlüssel sind verfügbar:\n"
                      "P_ORIGIN_ROOM(string) Speichert den ersten first_room\n"
                      "P_ORIGIN_PLAYER(string) Speichert den first_player\n"
                      "P_ORIGIN_CREATED_ON(int) Speichert die object_time\n"
                      "P_ORIGIN_CREATOR (string) Speichert den Creator\n"
                      "P_ORIGIN_UNIQUE_ID (string) Eine eindeutige ID.\n"
                      "Alle Schlüssel werden beim ersten Einlagern gespeichert "
                      "und danach erhalten.",

        PI_SET_FUN:   "set_origin",
        PI_GET_FUN:   "get_origin",
    ]));
    add_controller(FORBIDDEN_SET(P_ORIGIN),this_object());
    
    set_property_info(P_WATER_ORIGIN, ([
        PI_VALUE:     ([]),
        PI_FLAGS:     PF_NO_CLOSURE,

        PI_TYPES:     PT_MAPPING,
        PI_USES:      PU_ITEM,
        
        PI_WIZ_SHORT: "Speichert die Herkunft des Wassers.",
        PI_WIZ_LONG:  "Speichert die Herkunft des Wassers, siehe "
                      "/i/wasser/flasche.c ",

        PI_SET_FUN:   "set_water_origin",
        PI_GET_FUN:   "get_water_origin",
        PI_ADD_FUN:   "invalid_add",
        PI_DEL_FUN:   "invalid_del",
    ]));

    set_property_info(P_SOUND_ACTIONS, ([
        PI_VALUE:     ([]),
        PI_FLAGS:     PF_PERSISTANT,

        PI_TYPES:     PT_MAPPING,
        PI_USES:      PU_OBJECT | PU_VITEM,
        
        PI_WIZ_SHORT: "Speichert Sounddateien zu den Aktionen eines Objektes.",
        PI_WIZ_LONG:  
"Mit tuer->add(P_SOUND_ACTIONS,\"klopfen\",\"Basis/tuer_anklopfen.wav\") "
"wird der Standardsound gesetzt, jede andere Datei würde für die aktuelle "
"Tür den Sound ändern. Mit 0 statt dem Dateinamen wird ein Soudn unterdrückt. "
"Mit tuer->set(P_SOUND_ACTIONS,([\"klopfen\":\"<datei1>\",\"oeffnen\": usw...]) )"
"können alle Aktionen des Objektes tür gesetzt werden. Die verfügbaren "
"Aktionen werden über ? P_SOUND_ACTIONS dokumentiert.",
        PI_SET_FUN:   "set_sound_actions",
        PI_GET_FUN:   "get_sound_actions",
        PI_ADD_FUN:   "add_sound_actions",
        PI_DEL_FUN:   "del_sound_actions",
    ]));

}

mixed set_sound_actions(mapping property, mixed value, object caller, object item)
{
    mapping oldval = property[PI_VALUE];
    if (mappingp(value))
    {
        oldval = filter(({mapping}) value,
            function int (mixed action, mixed file) {
                if (stringp(action) && stringp(file))
                {
                    SOUNDCHECK->register_soundfile(file,SC_SOUNDFILE_PROP);
                    return 1;
                }
                else
                    return 0;
            } );
    }
    property[PI_VALUE] = oldval;
    return deep_copy(oldval);
}

mixed get_sound_actions(mapping property, mixed info, object caller, object item)
{
    mapping oldval = deep_copy(property[PI_VALUE]);
    if (info == 0 || !stringp(info))
    {
        return oldval;
    }
    if (member(oldval,info))
    {
        return oldval[info];
    }
    else
    {
        return 0;
    }
}

mixed add_sound_actions(mapping property, mixed key, mixed value, 
                        object caller, object item)
{
    mapping oldval = property[PI_VALUE];
    if (stringp(key) && stringp(value))
    {
        SOUNDCHECK->register_soundfile(value,SC_SOUNDFILE_PROP);
        oldval[key] = value;
    }
    property[PI_VALUE] = oldval;
    return deep_copy(oldval);
}

mixed del_sound_actions(mapping property, mixed key, object caller, object item)
{
    mapping oldval = property[PI_VALUE];
    if (stringp(key))
    {
        m_delete(oldval,key);
    }
    property[PI_VALUE] = oldval;
    return deep_copy(oldval);
}

mixed get_water_origin(mapping property, mixed info, object caller, object item)
{
    mapping oldval = deep_copy(property[PI_VALUE]);
    return oldval;
}

mixed set_water_origin(mapping property, mixed value, object caller, object item)
{
    mapping oldval = property[PI_VALUE];
    if (mappingp(value) && !sizeof(oldval)) // einmalig nur setzen
    {
        oldval = deep_copy(value);
        property[PI_VALUE] = oldval;
    }
    return deep_copy(oldval);
}

int forbidden_set_Root__origin(string property_name, mixed neuer_wert,
                 mixed alter_wert, object aufrufer, object item)
{
    return ("/apps/zentralbank"->is_valid_serializer(aufrufer)==0);
}

mixed set_origin(mapping property, mixed value, object caller, object item)
{
    mapping oldval = property[PI_VALUE];
    if (mappingp(value) && stringp(value[P_ORIGIN_ROOM])
        && stringp(value[P_ORIGIN_PLAYER]) && intp(value[P_ORIGIN_CREATED_ON])
        && value[P_ORIGIN_CREATED_ON] > 0 
        && sizeof(value)>=3 && sizeof(value)<=5)
    {
        oldval = deep_copy(value);
        property[PI_VALUE] = oldval;
    }
    return deep_copy(oldval);
}

mixed get_origin(mapping property, mixed info, object caller, object item)
{
    mapping oldval = deep_copy(property[PI_VALUE]);
    return oldval;
}


mixed set_debug_group(mapping property, mixed value, object caller, object item)
{
    string * oldval = property[PI_VALUE];
    if (pointerp(value))
    {
        // Unique machen die Gruppen...
        oldval = unmkmapping(mkmapping(filter(value, (: stringp($1) :)) ))[0];
    }
    property[PI_VALUE] = oldval;
    return oldval;
}

mixed get_debug_group(mapping property, mixed info, object caller, object item)
{
    string* oldval = deep_copy(property[PI_VALUE]);
    return oldval;
}

mixed add_debug_group(mapping property, mixed key, mixed value, 
                        object caller, object item)
{
    string* oldval = property[PI_VALUE];
    if (stringp(value))
    {
        oldval -= ({ value });
        oldval += ({ value });
    }
    else if (stringp(key)) // Falls mappingsyntax verwendet wird...
    {
        oldval -= ({ key });
        oldval += ({ key });
    }
    property[PI_VALUE] = oldval;
    return deep_copy(oldval);
}

mixed del_debug_group(mapping property, mixed key, object caller, object item)
{
    string* oldval = property[PI_VALUE];
    if (stringp(key))
    {
        oldval -= ({ key });
    }
    property[PI_VALUE] = oldval;
    return deep_copy(oldval);
}


mixed set_debug_info(mapping property, mixed value, object caller, object item)
{
    mapping oldval = property[PI_VALUE];
    if (mappingp(value))
    {
        oldval = filter(({mapping}) value,
            (: stringp($1) && (stringp($2) || intp($2)) :) );
    }
    property[PI_VALUE] = oldval;
    return oldval;
}

mixed get_debug_info(mapping property, mixed info, object caller, object item)
{
    mapping oldval = deep_copy(property[PI_VALUE]);
    if (info == 0 || !stringp(info))
    {
        return oldval;
    }
    if (member(oldval,info))
    {
        return oldval[info];
    }
    else
    {
        return 0;
    }
}

mixed add_debug_info(mapping property, mixed key, mixed value, 
                        object caller, object item)
{
    mapping oldval = property[PI_VALUE];
    if (stringp(key) && (stringp(value) || intp(value)))
    {
        oldval[key] = value;
    }
    property[PI_VALUE] = oldval;
    return deep_copy(oldval);
}

mixed del_debug_info(mapping property, mixed key, object caller, object item)
{
    mapping oldval = property[PI_VALUE];
    if (stringp(key))
    {
        m_delete(oldval,key);
    }
    property[PI_VALUE] = oldval;
    return deep_copy(oldval);
}

mixed set_crack_chances(mapping property, mixed value, object caller, object item)
{
    mapping oldval = property[PI_VALUE], newval;
    if (mappingp(value))
    {
        newval = ({mapping}) value;
        oldval = filter(newval, (: member(ALL_CRACK_CHANCES,$1)!=-1 
                                  && intp($2) && $2 >= 0 :) );
    }
    property[PI_VALUE] = oldval;
    return oldval;
}

mixed get_crack_chances(mapping property, mixed info, object caller, object item)
{
    mapping oldval = deep_copy(property[PI_VALUE]);
    if (info == 0 || !stringp(info))
    {
        if (member(oldval,CRACK_DEFAULT))
        {
            return oldval;
        }
        else
        {
            return oldval + ([ CRACK_DEFAULT : item->query_crack() ]);
        }
    }
    if (member(oldval,info))
    {
        return oldval[info];
    }
    else if (member(oldval,CRACK_DEFAULT))
    {
        return oldval[CRACK_DEFAULT];
    }
    return item->query_crack();
}

mixed add_crack_chances(mapping property, mixed key, mixed value, 
                        object caller, object item)
{
    mapping oldval = property[PI_VALUE];
    if (stringp(key) && member(ALL_CRACK_CHANCES,key)!=-1 && intp(value))
    {
        oldval = copy(oldval);
        oldval[key] = value;
        property[PI_VALUE] = oldval;
    }
    return deep_copy(oldval);
}

mixed del_crack_chances(mapping property, mixed key, object caller, object item)
{
    mapping oldval = property[PI_VALUE];
    if (stringp(key) && member(ALL_CRACK_CHANCES,key)!=-1)
    {
        oldval = copy(oldval);
        m_delete(oldval,key);
        property[PI_VALUE] = oldval;
    }
    return deep_copy(oldval);
}

mixed make_food(mapping property, int value, object caller, object item)
{
    if(!value)
        item->nahrung_remove_shadow();
    else if(playerp(item))
        return 0;
    else if(!item->query_nahrung())
        clone_object("/obj/nahrung_sh")->nahrung_setup_shadow(item);

    property[PI_VALUE] = value;
    return value;
}

mixed set_conservation(mapping property, mixed value, object caller, object item)
{
    mapping oldval = property[PI_VALUE], newval;
    if (mappingp(value))
    {
        newval = ({mapping}) value;
        oldval = filter(newval, (: member(ALL_P_CONSERVATION_KEYS,$1)!=-1 :) );
    }
    property[PI_VALUE] = oldval;
    return oldval;
}

mixed get_conservation(mapping property, mixed info, object caller, object item)
{
    mapping oldval = deep_copy(property[PI_VALUE]);
    if (info == 0 || !stringp(info))
    {
        return oldval;
    }
    if (member(oldval,info))
    {
        return oldval[info];
    }
    return 0;
}

mixed add_conservation(mapping property, mixed key, mixed value, 
                        object caller, object item)
{
    mapping oldval = property[PI_VALUE];
    if (stringp(key) && member(ALL_P_CONSERVATION_KEYS,key)!=-1)
    {
        oldval = deep_copy(oldval);
        oldval[key] = value;
        property[PI_VALUE] = oldval;
    }
    return deep_copy(oldval);
}

mixed del_conservation(mapping property, mixed key, object caller, object item)
{
    mapping oldval = property[PI_VALUE];
    if (stringp(key))
    {
        oldval = deep_copy(oldval);
        m_delete(oldval,key);
        property[PI_VALUE] = oldval;
    }
    return deep_copy(oldval);
}

void prepare_renewal() {}
void finish_renewal(object neu) {}
void abort_renewal() {}
