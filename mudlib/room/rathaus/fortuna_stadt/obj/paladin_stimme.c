inherit "/i/monster/monster";

void set_stimmenname(string name, string geschlecht)
{
    if(!name || name == "")
        name = "sonnenmuenze";

    set_name(lower_case(name));
    set_npc_name("paladinstimme" + random(1000000));
    set_id(({
        lower_case(name), "stimme", "paladinstimme"
    }));
    set_short("");
    set_long("");
    set_gender(geschlecht);
}

void create()
{
    monster::create();

    initialize("mensch", 1);

    set_name("sonnenmuenze");
    set_npc_name("paladinstimme");
    set_id(({
        "sonnenmuenze", "stimme", "paladinstimme"
    }));
    set_short("");
    set_long("");
    set_gender("weiblich");

    set_msg_in("");
    set_msg_out("");
    set_mmsg_in("");
    set_mmsg_out("");

    set_aggressive(0);
}

void sprich(string text)
{
    if(!text)
        text = "";

    do_command("sage " + text);
    call_out("verschwinde", 0);
}

void verschwinde()
{
    destruct(this_object());
}