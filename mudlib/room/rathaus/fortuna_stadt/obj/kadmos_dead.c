#define ROOM "/room/rathaus/fortuna_stadt/zentrum"
#define NPC  "/room/rathaus/fortuna_stadt/obj/kadmos_dummy"

void respawn()
{
    object room, ob;

    room = touch(ROOM);
    if(!room)
        return;

    if(present("kadmos", room) || present("trainingsdummy", room))
        return;

    ob = clone_object(NPC);
    if(ob)
        ob->move(room);
}

void monster_died(object monster, object moerder)
{
    object art;

    if(moerder)
    {
        art = present("paladinartefakt", moerder);
        if(art)
            art->paladin_kill(monster, moerder, "kampf");
    }

    call_out("respawn", 3);
}

void design_corpse(object corpse)
{
    if(corpse)
        corpse->set_short("Die zerbrochene Huelle eines Trainingsdummys");
}