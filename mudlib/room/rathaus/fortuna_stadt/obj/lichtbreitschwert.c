inherit "/i/weapon/nahkampf_waffe";

object owner;
int belohnt;

void set_owner(object ob) { owner = ob; }

void set_duration(int sec)
{
    remove_call_out("erlischt");
    call_out("erlischt", sec);
}

void create()
{
    "*"::create();

    init_weapon("langschwert", 180, 140, 100000);

    set_name("lichtbreitschwert");
    set_short("Ein gefuehrtes, riesiges, leuchtendes Paladin-Breitschwert");
    set_id(({
        "lichtbreitschwert", "lichtschwert", "schwert",
        "breitschwert", "paladinschwert", "paladinbreitschwert",
        "waffe", "nahkampfwaffe"
    }));
    set_gender("saechlich");
    set_adjektiv(({
        "riesig", "leuchtend", "paladinisch"
    }));
    set_material("licht");

    set_long(
"Das Paladin-Breitschwert besteht nicht aus Stahl, sondern aus gebuendeltem,\n"
"weissgoldenem Licht. Die Klinge ist breit wie ein Urteil und hell wie\n"
"ein Sonnenaufgang ueber Glas. In der Schneide laufen Lichtfasern auseinander\n"
"und finden sich wieder, als wuerde die Sonne selbst atmen.\n");

    set_wield_msg("Das Paladin-Breitschwert flammt in Deinen Haenden auf.\n");
    set_wield_msg_other("$Der(OBJ_TP) fuehrt ein riesiges Paladin-Breitschwert aus Licht.\n");
    set_unwield_msg("Du senkst das Paladin-Breitschwert. Die Klinge bleibt als heller Eid bei Dir.\n");
    set_unwield_msg_other("$Der(OBJ_TP) senkt das Paladin-Breitschwert aus Licht.\n");

    set_broken_message("Das Lichtbreitschwert zerfaellt nicht. Es wird nur stiller.\n");
}

void erlischt()
{
    object env;

    env = environment(this_object());
    if(env)
        tell_object(env, "Das Paladin-Breitschwert zerfaellt in tausend weissgoldene Lichtfasern.\n");

    destruct(this_object());
}

varargs int do_attack(object enemy, int flag)
{
    object own, env, art;
    int ret, boese;

    own = environment(this_object());
    ret = ::do_attack(enemy, flag);

    if(!ret || flag || !own || !enemy)
        return ret;

    env = environment(own);
    boese = 0;
    catch(boese = enemy->query_align() < -100);

    switch(random(5))
    {
        case 0:
            tell_object(own, "Das Paladin-Breitschwert zieht eine Sonnenlinie durch die Luft.\n");
            if(enemy) tell_object(enemy, "Weissgoldenes Licht schneidet durch Deine Schatten.\n");
            if(env)
                tell_room(env, "Das Paladin-Breitschwert zerlegt Schatten in flirrende Lichtfasern.\n",
                          ({ own, enemy }));
            break;

        case 1:
            if(boese)
            {
                tell_object(own, "Die Klinge erkennt Bosheit und flammt vernichtend auf.\n");
                if(enemy)
                    tell_object(enemy, "Das heilige Licht frisst sich bis in Deine dunkelsten Fasern.\n");
            }
            else
                tell_object(own, "Die Klinge summt wie ein Chor hinter geschlossenem Gold.\n");
            break;

        case 2:
            if(env)
                tell_room(env, "Sonnenfunken stieben von der Klinge des Paladinschwerts.\n",
                          ({ own }));
            tell_object(own, "Sonnenfunken stieben von Deiner Klinge.\n");
            break;
    }

    if(enemy)
    {
        int hp;
        hp = 1;
        catch(hp = enemy->query_hp());
        if(hp < 1 && !belohnt)
        {
            belohnt = 1;
            art = present("paladinartefakt", own);
            if(art)
                art->paladin_kill(enemy, own, "schwert");
        }
    }

    return ret;
}

int query_sellable() { return 0; }
int query_repairable() { return 0; }
int query_dropable() { return 0; }
int query_giveable() { return 0; }