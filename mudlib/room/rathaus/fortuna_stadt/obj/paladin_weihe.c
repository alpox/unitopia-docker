inherit "/i/item";
inherit "/i/move";

object caster;
int dmg;
int kraft;

void setup(object who, int damage, int power)
{
    caster = who;
    dmg = damage;
    kraft = power;
    remove_call_out("tick");
    call_out("tick", 1);
}

void create()
{
    set_name("paladinweihe");
    set_short("Eine brennende Sonnenweihe");
    set_id(({
        "paladinweihe", "weihe", "sonnenweihe", "kreis",
        "sonnenkreis", "runenkreis"
    }));
    set_gender("weiblich");
    set_weight(0);
    set_material(({
        "licht", "magie"
    }));

    set_long(
"Ein Kreis aus weissgoldenen Sonnenrunen brennt auf dem Boden. Die Zeichen\n"
"wandern langsam ineinander, als wuerden sie ein Urteil sprechen. Fuer\n"
"Sterbliche ist die Weihe warm. Fuer Monster ist sie Feuer.\n"
"Die Kraft der Weihe ist nicht unendlich; jeder Puls und jeder Feind zehrt daran.\n");
}

void tick()
{
    object env, *obs, ob, art;
    int i, hits, hp;

    env = environment(this_object());
    if(!env || kraft <= 0 || dmg <= 0)
    {
        destruct(this_object());
        return;
    }

    tell_room(env, "Die Sonnenweihe pulsiert. Weissgoldene Runen schlagen Funken aus dem Boden.\n");

    obs = all_inventory(env);

    for(i=0; i<sizeof(obs); i++)
    {
        ob = obs[i];

        if(!living(ob) || playerp(ob))
            continue;

        tell_object(ob, "Die Sonnenweihe brennt wie reines Licht. Du stoehnst unter dem heiligen Druck auf.\n");
        tell_room(env, ob->query_name() + " stoehnt auf, als die Sonnenweihe in seine Schatten greift.\n", ({ ob }));

        ob->add_hp(-dmg);
        hits++;

        hp = 1;
        catch(hp = ob->query_hp());
        if(hp < 1 && caster)
        {
            art = present("paladinartefakt", caster);
            if(art)
                art->paladin_kill(ob, caster, "weihe");
        }
    }

    kraft -= 1 + hits;
    dmg -= hits * 2;
    if(dmg < 1) dmg = 1;

    if(kraft <= 0)
    {
        tell_room(env, "Die Sonnenweihe hat ihre Kraft verausgabt und loest sich in helle Funken auf.\n");
        destruct(this_object());
        return;
    }

    if(hits)
        tell_room(env, "Die Sonnenweihe flackert; jeder getroffene Schatten zehrt an ihrer Kraft.\n");

    call_out("tick", 5);
}

int query_dropable() { return 0; }
int query_giveable() { return 0; }