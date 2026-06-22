inherit "/i/room";

void spawn_paladin_muenze()
{
    object ob;

    if(present("sonnenmuenze", this_object()) ||
       present("paladinmuenze", this_object()) ||
       present("paladin#artefakt#muenze", this_object()))
        return;

    ob = clone_object("/room/rathaus/fortuna_stadt/obj/paladin_muenze");
    if(ob) ob->move(this_object());
}

void spawn_kadmos()
{
    object ob;

    if(present("kadmos", this_object()) ||
       present("trainingsdummy", this_object()))
        return;

    ob = clone_object("/room/rathaus/fortuna_stadt/obj/kadmos_dummy");
    if(ob) ob->move(this_object());
}

void reset()
{
    ::reset();
    spawn_paladin_muenze();
    spawn_kadmos();
}

void create()
{
    ::create();

    set_short("Der Sternplatz von Fortuna");
    set_long(
"Dies ist das Herz der verborgenen Stadt. In das dunkle Pflaster ist\n"
"ein riesiger achtzackiger Stern eingelassen, dessen Linien wie frische\n"
"Glut unter Glas leuchten. Ringsum ragen Haeuser mit schmalen Balkonen\n"
"auf, an denen Pflanzen wachsen, die kleine elektrische Funken verlieren.\n"
"Wer hier steht, hoert die Stadt atmen.\n");

    set_room_domain("Pantheon");

    add_exit("/room/rathaus/fortuna_stadt/n", "norden");
    add_exit("/room/rathaus/fortuna_stadt/w", "westen");
    add_exit("/room/rathaus/fortuna_stadt/e", "osten");
    add_exit("/room/rathaus/fortuna_stadt/s", "sueden");

    call_out("spawn_paladin_muenze", 1);
    call_out("spawn_kadmos", 2);
}