inherit "/i/item";
inherit "/i/move";

#include <move.h>
#include <message.h>

#define ART "/room/rathaus/fortuna_stadt/obj/paladin_artefakt"

object pending_confirm;
object ready_to_choose;

int aktiviere(object who);
int frage_muenze(string str);
int sage_muenze(string str);
int fluestere_muenze(string str);

string norm(string str)
{
    if(!str)
        return "";
    return convert_umlaute(lower_case(str));
}

int meint_muenze(string str)
{
    str = norm(str);

    return strstr(str, "muenze") >= 0 ||
           strstr(str, "sonnenmuenze") >= 0 ||
           strstr(str, "paladinmuenze") >= 0 ||
           strstr(str, "lichtmuenze") >= 0 ||
           strstr(str, "artefaktmuenze") >= 0 ||
           strstr(str, "sonne") >= 0;
}

int phrase_waehle_sonne(string str)
{
    str = norm(str);

    return strstr(str, "ich waehle die sonne") >= 0 ||
           strstr(str, "ich waehle sonne") >= 0 ||
           strstr(str, "waehle die sonne") >= 0 ||
           strstr(str, "waehle sonne") >= 0;
}

string rede_inhalt(string str)
{
    string s, msg;

    if(!str)
        return "";

    s = norm(str);

    if(sscanf(s, "zu muenze %s", msg) == 1) return msg;
    if(sscanf(s, "zu sonnenmuenze %s", msg) == 1) return msg;
    if(sscanf(s, "zu paladinmuenze %s", msg) == 1) return msg;
    if(sscanf(s, "an muenze %s", msg) == 1) return msg;
    if(sscanf(s, "an sonnenmuenze %s", msg) == 1) return msg;
    if(sscanf(s, "an paladinmuenze %s", msg) == 1) return msg;

    if(phrase_waehle_sonne(str))
        return "Ich waehle die Sonne";

    return str;
}

void kommunikation(object ziel, string prefix, string text)
{
    if(!ziel)
        return;

    if(!text)
        text = "";

    catch(ziel->send_message_to(ziel, MT_NOISE, MA_COMM,
        wrap_say(prefix, text)));
}

void spieler_spricht(string txt, int fluestern, int ziel_muenze)
{
    object p, env, *obs;
    int i;
    string name, mein_prefix, anderer_prefix, anderer_text;

    if(!txt)
        txt = "";

    p = this_player();

    if(!p)
    {
        write((fluestern ? "Du fluesterst: " : "Du sagst: ") + txt + "\n");
        return;
    }

    name = 0;
    catch(name = p->query_real_name());
    if(!name || name == "")
        catch(name = p->query_name());
    if(!name || name == "")
        name = "Jemand";
    name = capitalize(name);

    if(fluestern)
    {
        if(ziel_muenze)
        {
            mein_prefix = "Du fluesterst der Sonnenmuenze zu:";
            anderer_prefix = name + " fluestert der Sonnenmuenze etwas zu:";
        }
        else
        {
            mein_prefix = "Du fluesterst:";
            anderer_prefix = name + " fluestert:";
        }

        anderer_text = "...";
    }
    else
    {
        if(ziel_muenze)
        {
            mein_prefix = "Du sagst zur Sonnenmuenze:";
            anderer_prefix = name + " sagt zur Sonnenmuenze:";
        }
        else
        {
            mein_prefix = "Du sagst:";
            anderer_prefix = name + " sagt:";
        }

        anderer_text = txt;
    }

    kommunikation(p, mein_prefix, txt);

    env = environment(p);

    if(env)
    {
        obs = all_inventory(env);

        for(i = 0; i < sizeof(obs); i++)
            if(living(obs[i]) && obs[i] != p)
                kommunikation(obs[i], anderer_prefix, anderer_text);
    }
}

void muenze_sagt(string txt)
{
    object p, env, *obs;
    int i;

    p = this_player();

    if(!p && environment(this_object()) && living(environment(this_object())))
        p = environment(this_object());

    if(p)
    {
        kommunikation(p, "Die Sonnenmuenze sagt:", txt);

        env = environment(p);

        if(env)
        {
            obs = all_inventory(env);

            for(i = 0; i < sizeof(obs); i++)
                if(living(obs[i]) && obs[i] != p)
                    kommunikation(obs[i], "Die Sonnenmuenze sagt:", txt);
        }

        return;
    }

    write("Die Sonnenmuenze sagt: " + txt + "\n");
}

void create()
{
    set_name("sonnenmuenze");
    set_short("Eine sonnengepraegte Muenze");
    set_gender("weiblich");

    set_id(({
        "muenze", "sonnenmuenze", "paladinmuenze", "lichtmuenze",
        "artefaktmuenze", "paladin#artefakt#muenze"
    }));

    set_weight(0);
    set_material(({
        "gold", "licht", "magie"
    }));

    set_long(
"Eine schwere Muenze aus mattem Gold liegt hier. Auf der Vorderseite\n"
"ist ein Paladin-Breitschwert aus Licht gepraegt, auf der Rueckseite\n"
"ein Schild, dessen Rand wie ein Heiligenschein brennt. Sie wirkt nicht\n"
"wie ein Schatz. Sie wirkt wie eine Berufung.\n"
"Sie traegt keine Gebrauchsanweisung. Sie antwortet auf Fragen.\n"
"Frage sie nach Herkunft, Gaben, Bindung, Hilfe oder Artefakt.\n");
}

void init()
{
    "*"::init();

    if(this_player() && environment(this_object()) == this_player())
        call_out("erster_hinweis", 1, this_player());

    add_action("reibe_muenze", "reibe");
    add_action("reibe_muenze", "reib");

    add_action("rede_muenze", "rede");
    add_action("rede_muenze", "sprich");
    add_action("rede_muenze", "spreche");

    add_action("frage_muenze", "frag");
    add_action("frage_muenze", "frage");
    add_action("frag_einfach", "herkunft");
    add_action("frag_einfach", "gaben");
    add_action("frag_einfach", "bindung");
    add_action("frag_einfach", "artefakt");

    add_action("sage_muenze", "sag");
    add_action("sage_muenze", "sage");
    add_action("sage_muenze", "sag:");
    add_action("sage_muenze", "sage:");

    add_action("fluestere_muenze", "fluestere");
    add_action("fluestere_muenze", "fluester");
    add_action("fluestere_muenze", "flüstere");
    add_action("fluestere_muenze", "flüster");

    add_action("ja_muenze", "j");
    add_action("ja_muenze", "ja");
    add_action("ja_muenze", "Ja");

    add_action("nein_muenze", "n");
    add_action("nein_muenze", "nein");
    add_action("nein_muenze", "Nein");

    add_action("hilfe_sonnengaben", "hilfe");
    add_action("hilfe_sonnengaben", "sonnengaben");
}

void erster_hinweis(object who)
{
    if(!who || environment(this_object()) != who)
        return;

    if(!pending_confirm && !ready_to_choose)
        muenze_sagt("Frage mich nach meiner Herkunft, meinen Gaben oder der Bindung.");
}

void sonnengaben()
{
    write(
"Sonnengaben der Muenze\n"
"----------------------\n"
"  pschwert    Paladin-Breitschwert aus Licht rufen, 25 GP\n"
"  pschild     Gottesschild fuer drei Minuten, 60 GP\n"
"  pweihe      Weihe des Raumes mit Lichtschaden, 75 GP\n"
"  preinigen   Reinigung von Daemonischem, Fluechen und Schatten, 40 GP\n"
"  pwille      heiliger Wille, der Monster kurz verstummen laesst, 55 GP\n"
"  pheile      Heilung von Ausdauer und Zauberkraft, 50 GP\n"
"\n"
"Die Bindung beginnt mit: reibe muenze\n");
}

int hilfe_sonnengaben(string str)
{
    str = norm(str);

    if(str == "sonnengaben" || str == "paladin")
    {
        sonnengaben();
        return 1;
    }

    return 0;
}

int rede_muenze(string str)
{
    if(str && !meint_muenze(str))
        return 0;

    muenze_sagt("Sprich, Reisender. Frage mich nach Herkunft, Gaben, Bindung, Hilfe oder Artefakt.");
    return 1;
}

int frag_einfach(string arg)
{
    return frage_muenze(query_verb() + " muenze");
}

int frage_muenze(string str)
{
    str = norm(str);

    if(!meint_muenze(str))
        return 0;

    if(strstr(str, "wer") >= 0 || strstr(str, "bist") >= 0)
        muenze_sagt("Ich bin die Sonnenmuenze, ein Paladin-Artefakt vor der Bindung.");
    else if(strstr(str, "herkunft") >= 0 || strstr(str, "woher") >= 0 || strstr(str, "kommst") >= 0)
        muenze_sagt("Ich wurde im letzten Chor des Sonnenordens gepraegt, als seine Hallen schon brannten.");
    else if(strstr(str, "orden") >= 0)
        muenze_sagt("Der Sonnenorden schuetzte Schwache, bannte Daemonen und weihte Orte, an denen Schatten Namen bekamen.");
    else if(strstr(str, "artefakt") >= 0)
        muenze_sagt("Ein Artefakt ist ein Eid in Gestalt. Ich werde nicht getragen wie Gold, sondern gebunden wie ein Versprechen.");
    else if(strstr(str, "mach") >= 0 || strstr(str, "was") >= 0)
        muenze_sagt("Ich pruefe Deinen Willen. Wenn Du mich bindest, erwachen meine Gaben.");
    else if(strstr(str, "helfen") >= 0 || strstr(str, "hilfe") >= 0)
        muenze_sagt("Ich helfe Dir mit Schwert, Schild, Weihe, Reinigung, Wille und Heilung. Frage nach meinen Gaben.");
    else if(strstr(str, "frag") >= 0)
        muenze_sagt("Du kannst mich fragen: wer bist du, woher kommst du, was ist ein Artefakt, was sind deine Gaben, wie binde ich dich, willst du helfen.");
    else if(strstr(str, "gabe") >= 0 || strstr(str, "faehig") >= 0)
        muenze_sagt("Meine Gaben heissen Schwert, Schild, Weihe, Reinigung, Wille und Heilung. Rufe: hilfe sonnengaben.");
    else if(strstr(str, "bind") >= 0 || strstr(str, "reib") >= 0 || strstr(str, "waehl") >= 0)
        muenze_sagt("Nimm mich, reibe mich, antworte mit Ja, und sprich laut: Ich waehle die Sonne.");
    else
        muenze_sagt("Frage klarer, Reisender. Frage nach meiner Herkunft, meinen Gaben oder der Bindung.");

    return 1;
}

int sage_muenze(string str)
{
    string s, inhalt;

    s = norm(str);
    inhalt = rede_inhalt(str);

    if(phrase_waehle_sonne(str))
    {
        spieler_spricht("Ich waehle die Sonne", 0, 0);

        if(ready_to_choose != this_player())
        {
            muenze_sagt("Noch nicht. Reibe mich zuerst, dann entscheide Dich.");
            return 1;
        }

        if(aktiviere(this_player()))
            ready_to_choose = 0;

        return 1;
    }

    if(!meint_muenze(str))
        return 0;

    spieler_spricht(inhalt, 0, 1);

    if(strstr(s, "hallo") >= 0 || strstr(s, "salve") >= 0 || strstr(s, "gruss") >= 0)
        muenze_sagt("Hallo, Reisender. In Dir ist mehr Licht, als Du ahnst.");
    else if(strstr(s, "wer") >= 0 || strstr(s, "bist") >= 0)
        muenze_sagt("Ich bin die Sonnenmuenze. Ein Paladin-Artefakt, das noch schlaeft.");
    else if(strstr(s, "herkunft") >= 0 || strstr(s, "woher") >= 0 || strstr(s, "kommst") >= 0)
        muenze_sagt("Ich komme aus einem Sonnenorden, der fiel, weil er blieb, als alle anderen flohen.");
    else if(strstr(s, "orden") >= 0)
        muenze_sagt("Der Sonnenorden war Schild vor dem Abgrund. Seine Ritter starben, aber ihr Licht blieb in mir.");
    else if(strstr(s, "artefakt") >= 0)
        muenze_sagt("Ein Artefakt ist ein Wille, der Form angenommen hat. Meine Form ist Gold, mein Wesen ist Eid.");
    else if(strstr(s, "mach") >= 0 || strstr(s, "was") >= 0)
        muenze_sagt("Ich warte auf eine Hand, die mich nicht als Schatz nimmt, sondern als Eid.");
    else if(strstr(s, "will helfen") >= 0 || strstr(s, "helfen") >= 0)
        muenze_sagt("Dann frage nach meinen Gaben. Wer helfen will, muss wissen, welches Licht er ruft.");
    else if(strstr(s, "frag") >= 0)
        muenze_sagt("Frage mich nach Herkunft, Artefakt, Gaben, Bindung, Orden oder Hilfe.");
    else if(strstr(s, "gabe") >= 0 || strstr(s, "faehig") >= 0)
        muenze_sagt("Rufe 'hilfe sonnengaben'. Dort stehen die Namen der Gaben.");
    else if(strstr(s, "bind") >= 0 || strstr(s, "reib") >= 0 || strstr(s, "waehl") >= 0)
        muenze_sagt("Reibe mich. Antworte mit Ja. Dann sprich laut: Ich waehle die Sonne.");
    else if(strstr(s, "sonnengaben") >= 0 || strstr(s, "hilfe") >= 0)
    {
        muenze_sagt("Hoere gut zu. Ich oeffne Dir die Namen meiner Gaben.");
        sonnengaben();
    }
    else
        muenze_sagt("Ich hoere Dich. Doch frage mich nach Herkunft, Gaben, Bindung oder Hilfe.");

    return 1;
}

int fluestere_muenze(string str)
{
    string s, inhalt;

    s = norm(str);

    if(!meint_muenze(str))
        return 0;

    inhalt = rede_inhalt(str);
    spieler_spricht(inhalt, 1, 1);

    if(phrase_waehle_sonne(str))
    {
        muenze_sagt("Der Eid verlangt eine laute Stimme. Sprich: Ich waehle die Sonne.");
        return 1;
    }

    if(strstr(s, "hallo") >= 0 || strstr(s, "salve") >= 0 || strstr(s, "gruss") >= 0)
        muenze_sagt("Ich hoere auch leise Worte, Reisender.");
    else if(strstr(s, "herkunft") >= 0 || strstr(s, "woher") >= 0 || strstr(s, "kommst") >= 0)
        muenze_sagt("Meine Herkunft liegt in einem Orden, der am Ende nicht floh.");
    else if(strstr(s, "gabe") >= 0 || strstr(s, "faehig") >= 0)
        muenze_sagt("Meine Gaben sind Schwert, Schild, Weihe, Reinigung, Wille und Heilung.");
    else if(strstr(s, "bind") >= 0 || strstr(s, "reib") >= 0 || strstr(s, "waehl") >= 0)
        muenze_sagt("Reibe mich. Antworte mit Ja. Dann sprich laut: Ich waehle die Sonne.");
    else
        muenze_sagt("Ich hoere Dein Fluesterwort. Frage nach Herkunft, Gaben oder Bindung.");

    return 1;
}

int reibe_muenze(string str)
{
    if(!meint_muenze(str))
        return 0;

    if(environment(this_object()) != this_player())
    {
        muenze_sagt("Nimm mich zuerst in Deine Hand.");
        return 1;
    }

    if(present("paladinartefakt", this_player()))
    {
        muenze_sagt("Der Eid ist bereits geschlossen. Du musst mich nicht erneut reiben.");
        return 1;
    }

    pending_confirm = this_player();

    write(
"Du reibst mit dem Daumen ueber die sonnengepraegte Muenze.\n"
"Ein warmer Kreis aus Licht oeffnet sich auf Deiner Handflaeche.\n");

    muenze_sagt("Bist Du sicher, dass Du die Sonne waehlen willst? Antworte mit Ja oder Nein.");
    return 1;
}

int ja_muenze(string str)
{
    if(pending_confirm != this_player())
        return 0;

    pending_confirm = 0;
    ready_to_choose = this_player();

    muenze_sagt("Dann sprich Deinen Willen laut aus: Ich waehle die Sonne.");
    return 1;
}

int nein_muenze(string str)
{
    if(pending_confirm != this_player())
        return 0;

    pending_confirm = 0;
    ready_to_choose = 0;

    muenze_sagt("Dann bleibe ich Muenze, bis Dein Wille heller brennt.");
    return 1;
}

int aktiviere(object who)
{
    object art;

    if(!who || !playerp(who))
        return 0;

    if(present("paladinartefakt", who))
    {
        muenze_sagt("Der Eid ist bereits geschlossen.");
        destruct(this_object());
        return 1;
    }

    art = clone_object(ART);

    if(!art || art->move(who) != MOVE_OK)
    {
        tell_object(who, "Das Licht sammelt sich, findet aber keinen festen Platz bei Dir.\n");
        return 0;
    }

    catch(art->force_init());

    tell_object(who,
"Du sprichst: Ich waehle die Sonne.\n"
"Die Muenze wird schwerer, waermer, wirklicher.\n"
"Ein Sonnenzeichen brennt fuer einen Herzschlag ueber Deiner Brust.\n");

    muenze_sagt("In nomine Dei. Trage Licht, wo Schatten herrscht. Rufe 'philfe' oder 'hilfe sonnengaben'.");

    if(environment(who))
        tell_room(environment(who),
"Ein heller Ring aus Sonnenlicht flammt um die Haende eines Paladins auf.\n",
({ who }));

    destruct(this_object());
    return 1;
}

int query_sellable() { return 0; }
int query_giveable() { return 1; }
int query_dropable() { return 1; }