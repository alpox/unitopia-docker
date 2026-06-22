// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /sys/properties.h
// Author:      Gnomi
// Description: Defines fuer Properties

#ifndef __PROPERTIES_H__
#define __PROPERTIES_H__

#define P_NAMESPACE_ROOT    "Root:"

#define P_LOOK_MSG          P_NAMESPACE_ROOT "look_msg"
#define P_READ_MSG          P_NAMESPACE_ROOT "read_msg"
#define P_SMELL_MSG         P_NAMESPACE_ROOT "smell_msg"
#define P_HEAR_MSG          P_NAMESPACE_ROOT "hear_msg"
#define P_ATTACK_MSG        P_NAMESPACE_ROOT "attack_msg"
#define P_FEEL_MSG          P_NAMESPACE_ROOT "feel_msg"
#define P_TAKE_MSG          P_NAMESPACE_ROOT "take_msg"

#define P_NAHRUNG           P_NAMESPACE_ROOT "nahrung"

#define P_CRACK_CHANCES     P_NAMESPACE_ROOT "crack_chances"
#define CRACK_DEFAULT       P_CRACK_CHANCES ":default"
#define CRACK_BOMB          P_CRACK_CHANCES ":bomb"
#define CRACK_LOCK          P_CRACK_CHANCES ":lock"
#define CRACK_MAGIC         P_CRACK_CHANCES ":magic"
#define ALL_CRACK_CHANCES ({CRACK_DEFAULT,CRACK_BOMB,CRACK_LOCK,CRACK_MAGIC})

#define P_DEBUG_INFO        P_NAMESPACE_ROOT "debug_info"
#define P_DEBUG_GROUP       P_NAMESPACE_ROOT "debug_group"

#define P_CONSERVATION      P_NAMESPACE_ROOT "conservation"
#define P_CONSERVATION_PRECHECK    "conservation:precheck"
#define P_CONSERVATION_FACTORY     "conservation:factory"
#define P_CONSERVATION_IDENTIFIER  "conservation:identifier"
#define P_CONSERVATION_ITEM_TARIFF "conservation:item:tariff"
#define P_CONSERVATION_TROPHEY     "conservation:trophey"
#define ALL_P_CONSERVATION_KEYS ({P_CONSERVATION_PRECHECK, \
    P_CONSERVATION_FACTORY, P_CONSERVATION_IDENTIFIER, \
    P_CONSERVATION_ITEM_TARIFF, P_CONSERVATION_TROPHEY })

#define P_ORIGIN            P_NAMESPACE_ROOT "origin"
#define P_ORIGIN_ROOM       "origin:first:room"
#define P_ORIGIN_PLAYER     "origin:first_player"
#define P_ORIGIN_CREATED_ON "origin:object:time"
#define P_ORIGIN_CREATOR    "origin:creator"
#define P_ORIGIN_UNIQUE_ID  "origin:unique:id"

#define P_WATER_ORIGIN      P_NAMESPACE_ROOT "water:origin"

#define P_SOUND_ACTIONS     P_NAMESPACE_ROOT "sound:actions"

#define P_KEEP_OR_SELL      P_NAMESPACE_ROOT "keep_or_sell"
#define P_DONT_SELL_CONTENT P_NAMESPACE_ROOT "dont_sell_content"

#define CONTROLLER_PROPERTY_NAME(propname) regreplace((propname), "[^A-Za-z0-9_]", "__", 1)

#define MODIFY_SET(propname)       ("modify_set_"      +CONTROLLER_PROPERTY_NAME(propname))
#define FORBIDDEN_SET(propname)    ("forbidden_set_"   +CONTROLLER_PROPERTY_NAME(propname))
#define NOTIFY_SET(propname)       ("notify_set_"      +CONTROLLER_PROPERTY_NAME(propname))
#define MODIFY_QUERY(propname)     ("modify_query_"    +CONTROLLER_PROPERTY_NAME(propname))
#define MODIFY_ADD(propname)       ("modify_add_"      +CONTROLLER_PROPERTY_NAME(propname))
#define FORBIDDEN_ADD(propname)    ("forbidden_add_"   +CONTROLLER_PROPERTY_NAME(propname))
#define NOTIFY_ADD(propname)       ("notify_add_"      +CONTROLLER_PROPERTY_NAME(propname))
#define FORBIDDEN_DELETE(propname) ("forbidden_delete_"+CONTROLLER_PROPERTY_NAME(propname))
#define NOTIFY_DELETE(propname)    ("notify_delete_"   +CONTROLLER_PROPERTY_NAME(propname))

#define MODIFY_SET_V_ITEM(propname)       ("modify_set_v_item_"      +CONTROLLER_PROPERTY_NAME(propname))
#define FORBIDDEN_SET_V_ITEM(propname)    ("forbidden_set_v_item_"   +CONTROLLER_PROPERTY_NAME(propname))
#define NOTIFY_SET_V_ITEM(propname)       ("notify_set_v_item_"      +CONTROLLER_PROPERTY_NAME(propname))
#define MODIFY_QUERY_V_ITEM(propname)     ("modify_query_v_item_"    +CONTROLLER_PROPERTY_NAME(propname))
#define MODIFY_ADD_V_ITEM(propname)       ("modify_add_v_item_"      +CONTROLLER_PROPERTY_NAME(propname))
#define FORBIDDEN_ADD_V_ITEM(propname)    ("forbidden_add_v_item_"   +CONTROLLER_PROPERTY_NAME(propname))
#define NOTIFY_ADD_V_ITEM(propname)       ("notify_add_v_item_"      +CONTROLLER_PROPERTY_NAME(propname))
#define FORBIDDEN_DELETE_V_ITEM(propname) ("forbidden_delete_v_item_"+CONTROLLER_PROPERTY_NAME(propname))
#define NOTIFY_DELETE_V_ITEM(propname)    ("notify_delete_v_item_"   +CONTROLLER_PROPERTY_NAME(propname))

#endif
