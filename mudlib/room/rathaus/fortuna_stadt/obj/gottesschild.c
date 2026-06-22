inherit "/i/item";
inherit "/i/move";

object owner;

void set_owner(object ob) { owner = ob; }

void set_duration(int sec)
{
    remove_call_out("erlischt");
    call_out("erlischt", sec);
}

void create()
{
    set_name("gottesschild");
    set_short("Ein kreisender Gottesschild aus Sonnenlicht");
    set_id(({
        "gottesschild", "schild", "paladinschild", "sonnenschild",
        "goldene fluegel", "fluegel"
    }));
    set_gender("maennlich");
    set_weight(0);
    set_material(({
        "licht", "magie"
    }));

    set_long(
"Der Gottesschild ist kein Schild aus Holz oder Metall. Er ist ein Kreis\n"
"aus hellem Sonnenfeuer, der dicht um seinen Traeger steht. Hinter ihm\n"
"flimmern goldene Fluegel aus Licht, so gross und still wie ein Urteil.\n"
"An seinem Rand wandern winzige Schriftzeichen wie Funken ueber Gold.\n");
}

void erlischt()
{
    if(owner)
        tell_object(owner, "Der Gottesschild zerfaellt in helle Funken.\n");
    destruct(this_object());
}

int query_armour() { return 1; }
int query_dropable() { return 0; }
int query_giveable() { return 0; }