// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/i/player/filter_messages.c
// Description:	Nachrichten filten aufgrund von Attributen, z.B. Kampf.
// Author:	Myonara

#include <add_hp.h>
#include <fight_options.h>
#include <message.h>

#ifdef UNItopia
#define PDBG_GROUP "Root:Filter:Messages"
#include <p/debugger.h>
#endif

private mapping filter_settings = ([FIM_MAX_COUNT:5 ]);
private nosave int supressed_msgs = 0;

mapping query_filter_settings()
{
    return deep_copy(filter_settings);
}

void set_filter_settings(mapping fs)
{
    // TODO validierungscheck
    filter_settings = fs||([]);
}

protected int filter_msg_by_attributes(int msg_type,int msg_action,
    mapping attributes,object who,mixed verursacher,string msg)
{
    int msgfilter,ahflags,strength;
    
    if (!mappingp(attributes))
        return 0;
    if (member(attributes,MSG_RECEIVER_WHOM) == 0)
        return 0;
    
    mapping ahinfos = attributes[MSG_AH_INFOS]||([]);
    mapping combat_log = ahinfos[AH_COMBAT_LOG];
    ahflags = ahinfos[AH_FLAGS];
    
    
    if (attributes[MSG_RECEIVER_WHOM]==AH_VICTIM)
    {
        msgfilter = filter_settings[FIM_WHO_ENEMY];
    }
    else if (attributes[MSG_RECEIVER_WHOM]==AH_ATTACKER)
    {
        msgfilter = filter_settings[FIM_WHO_SELF];
    }
    else // if (attributes[MSG_RECEIVER_WHOM]==MSG_OTHERS)
    {
        msgfilter = filter_settings[FIM_WHO_OTHERS];
    }
    
    if ( (ahflags & AH_CRITICAL) > 0
        && (msgfilter & FIM_FILTER_FIGHT_CRITICAL)==0)
    {
        return 0;
    }
    if (member(attributes,MSG_FIRST_MSG) 
        && (msgfilter & FIM_FILTER_FIGHT_FIRST_MSG)==0)
    {
        return 0;
    }
    if (member(attributes,FIM_BROKEN))
    {
        if (member(attributes,FIM_WEAPON)
            && (msgfilter & FIM_FILTER_WEAPON_BROKEN)==0)
        {
            return 0;
        }
        if (member(attributes,FIM_ARMOUR)
            && (msgfilter & FIM_FILTER_ARMOUR_BROKEN)==0)
        {
            return 0;
        }
    }
    if (member(attributes,MSG_LAST_MSG) 
        && (msgfilter & FIM_FILTER_FIGHT_LAST_MSG)==0)
    {
        return 0;
    }
    if (mappingp(combat_log))
    {
        switch(combat_log[ahinfos[AH_ATTACKER]])
        {
            case 0:      strength = FIM_FILTER_CATEGORY_0NULL;    break;
            case 1..4:   strength = FIM_FILTER_CATEGORY_1WEAK;    break;
            case 5..9:   strength = FIM_FILTER_CATEGORY_2MEDIUM;  break;
            case 10..19: strength = FIM_FILTER_CATEGORY_3STRONG;  break;
            case 20..29: strength = FIM_FILTER_CATEGORY_4HIGH;    break;
            default:     strength = FIM_FILTER_CATEGORY_5FRACTAL; break;
        }
        if ( (msgfilter & strength) == 0)
        {
            return 0;
        }
    }
    if (filter_settings[FIM_MAX_COUNT] > 0)
    {
        if (supressed_msgs++ <= filter_settings[FIM_MAX_COUNT])
        {
#ifdef UNItopia
            PDBG(sprintf("1:MT_%d MA_%d %s who_%Q veru_%Q msg_%Q",
                msg_type,msg_action,mixed2str(attributes),
                who, verursacher,msg));
#endif
            return 1;
        }
        supressed_msgs = 0;
        return 0;
    } 
    else if (filter_settings[FIM_MAX_COUNT] < 0)
    {
        return 0;
    }
#ifdef UNItopia
    PDBG(sprintf("2:MT_%d MA_%d %s who_%Q veru_%Q msg_%Q",
        msg_type,msg_action,mixed2str(attributes),
        who, verursacher,msg));
#endif
    return 1;
}
