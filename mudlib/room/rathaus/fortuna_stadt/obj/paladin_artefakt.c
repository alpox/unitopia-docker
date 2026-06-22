
inherit "/i/item";
inherit "/i/install";

#include <move.h>
#include <message.h>

#define SCHWERT "/room/rathaus/fortuna_stadt/obj/lichtbreitschwert"
#define SCHILD  "/room/rathaus/fortuna_stadt/obj/gottesschild"
#define WEIHE   "/room/rathaus/fortuna_stadt/obj/paladin_weihe"

int resonanz, rang, tips_off;
int gp, last_gp_time, schild_bis;
int makel, eid_brueche, pruefungen;
int kills, weihen, heilungen, reinigungen;
string talent, relikt1, relikt2, relikt3;
mapping raumweihen, gebannte, belohnt;

/* ------------------------------------------------------------
 * Grundwerte / Initialisierung
 * ------------------------------------------------------------ */

void sicher_init()
{
    if(rang < 1) rang = 1;
    if(rang > 100) rang = 100;
    if(last_gp_time <= 0) last_gp_time = time();
    if(!talent) talent = "";
    if(!relikt1) relikt1 = "";
    if(!relikt2) relikt2 = "";
    if(!relikt3) relikt3 = "";
    if(!raumweihen) raumweihen = ([]);
    if(!gebannte) gebannte = ([]);
    if(!belohnt) belohnt = ([]);
}

string norm(string str)
{
    if(!str) return "";
    return convert_umlaute(lower_case(str));
}

string capname(object ob)
{
    string n;

    if(!ob) return "Jemand";
    n = 0;
    catch(n = ob->query_real_name());
    if(!n || n == "") catch(n = ob->query_name());
    if(!n || n == "") n = "paladin";
    return capitalize(n);
}

string *rang_namen()
{
    return ({
        "Sonnenfunke",
        "Lichttraeger",
        "Goldsucher",
        "Eidtraeger",
        "Schildknappe der Sonne",
        "Runentraeger",
        "Flammenknappe",
        "Morgenwaechter",
        "Lichtbote",
        "Eidritter",
        "Sonnenritter",
        "Goldklinge",
        "Weihehueter",
        "Schattenrichter",
        "Daemonenbanner",
        "Chorwache",
        "Morgenstern",
        "Heilender Arm",
        "Sonnenlanze",
        "Seraphischer Anwalt",
        "Templer des ersten Lichts",
        "Richter der stillen Flamme",
        "Hueter des Goldkreises",
        "Exorzist der Morgenroete",
        "Schild des Gebets",
        "Klinge der Weihe",
        "Lichtvogt",
        "Bannertraeger der Sonne",
        "Ordensrichter",
        "Goldener Paladin",
        "Wahrer Eidritter",
        "Sonnenherold",
        "Waechter der Schwachen",
        "Ritter der Sieben Strahlen",
        "Reiniger des Makels",
        "Schild der Hallen",
        "Brecher der Finsternis",
        "Chorritter",
        "Lichtmeister",
        "Seraphenklinge",
        "Grossritter der Sonne",
        "Hochtempler",
        "Hochrichter",
        "Hochexorzist",
        "Waechter des Sonnenaltars",
        "Traeger des brennenden Siegels",
        "Goldfluegel",
        "Eidbewahrer",
        "Lichtvogt der Hallen",
        "Paladin des Zweiten Chors",
        "Seraphischer Paladin",
        "Richter des Morgenlichts",
        "Templer der geweihten Erde",
        "Exorzist des reinen Namens",
        "Schildengel",
        "Schwertengel",
        "Heiliger Bannsprecher",
        "Goldener Standartentraeger",
        "Waechter der Unschuldigen",
        "Sonnenkomtur",
        "Komtur des Lichts",
        "Komtur der Weihe",
        "Komtur des Schildes",
        "Komtur der Heilung",
        "Komtur der Reinigung",
        "Erzpaladin",
        "Erztempler",
        "Erzrichter",
        "Erzexorzist",
        "Erzseraph",
        "Paladin der brennenden Krone",
        "Sonnenkrone",
        "Lichtfuerst",
        "Heiliger Protektor",
        "Herr der Weihe",
        "Daemonenrichter",
        "Goldener Erloeser",
        "Stimme des Sonnenordens",
        "Schwert des Chors",
        "Schild des Himmels",
        "Seraph der Erde",
        "Seraph des Feuers",
        "Seraph der Heilung",
        "Seraph des Gerichts",
        "Sonnenheiliger",
        "Heiliger der Weihe",
        "Heiliger des Schildes",
        "Heiliger der Reinigung",
        "Heiliger des Willens",
        "Heiliger der Goldfluegel",
        "Avatar der Morgenroete",
        "Avatar des Sonnenordens",
        "Avatar der heiligen Weihe",
        "Avatar des Goldschilds",
        "Avatar der Lichtklinge",
        "Avatar des reinen Willens",
        "Avatar der heilenden Sonne",
        "Avatar des letzten Chors",
        "Avatar der ungebrochenen Weihe",
        "Avatar des ungebrochenen Lichts",
    });
}

string rang_name()
{
    string *r;

    sicher_init();
    r = rang_namen();
    return r[rang - 1];
}

int rang_kosten()
{
    sicher_init();
    return 80 + rang * 22;
}

int gp_max()
{
    int m;

    sicher_init();
    m = 300 + (rang - 1) * 8;
    if(relikt1 == "sonnenkern") m += 75;
    return m;
}

int in_weihe()
{
    object env;

    if(!this_player()) return 0;
    env = environment(this_player());
    if(!env) return 0;
    return !!present("paladinweihe", env);
}

int raum_altar_bonus()
{
    object env;
    string key;

    if(!this_player()) return 0;
    env = environment(this_player());
    if(!env || !raumweihen) return 0;
    key = object_name(env);
    if(key && raumweihen[key] >= 3) return 5;
    return 0;
}

int gp_regen()
{
    int r;

    sicher_init();
    r = 20 + (rang - 1) / 5;
    if(talent == "templer") r += 2;
    if(in_weihe()) r += 3;
    r += raum_altar_bonus();
    return r;
}

int apzp_regen()
{
    int r;

    sicher_init();
    r = 1 + rang / 25;
    if(r > 5) r = 5;
    if(talent == "seraph") r += 1;
    if(in_weihe()) r += 1;
    return r;
}

void create()
{
    set_name("paladinmuenze");
    set_short("Eine sonnengepraegte Paladinmuenze");
    set_gender("weiblich");
    set_id(({
        "paladinmuenze", "paladinartefakt", "muenze",
        "sonnenmuenze", "artefakt", "paladin#artefakt#muenze"
    }));
    set_weight(0);
    set_material(({
        "gold", "licht", "magie"
    }));
    set_no_move_reason("Die Paladinmuenze ist an Deinen Eid gebunden!");

    set_long(
"Die Muenze ist nun kein blosses Fundstueck mehr. Sie liegt warm in Deiner\n"
"Hand, als schlage tief in ihr ein kleines Sonnenherz. Auf ihrer einen\n"
"Seite brennt ein Paladin-Breitschwert aus Licht, auf der anderen ein\n"
"Gottesschild mit einem Rand wie ein Heiligenschein.\n"
"Sie ist still, aber nicht stumm. Sage zu ihr: wer bist du, gaben oder hilfe.\n");

    sicher_init();
    if(gp <= 0) gp = gp_max();
    set_heart_beat(1);
}

/* Harte Fallbacks gegen DeklinErr, falls die Mudlib beim Autoload sehr frueh fragt. */
string query_name() { return "paladinmuenze"; }
/* ------------------------------------------------------------
 * Kommunikation
 * ------------------------------------------------------------ */

void komm(object who, string prefix, string text)
{
    if(!who) return;
    if(!text) text = "";
    catch(send_message_to(({
        who
    }), MT_NOISE|MT_INDENT, MA_COMM, wrap_say(prefix, text)));
}

void komm_room(object except, string prefix, string text)
{
    object env, *obs;
    int i;

    if(!except) return;
    env = environment(except);
    if(!env) return;

    obs = all_inventory(env);
    for(i = 0; i < sizeof(obs); i++)
        if(living(obs[i]) && obs[i] != except)
            komm(obs[i], prefix, text);
}

void muenze_sagt(string txt)
{
    object p;

    p = this_player();
    if(!p && environment(this_object()) && living(environment(this_object())))
        p = environment(this_object());

    if(p)
    {
        komm(p, "Die Paladinmuenze sagt:", txt);
        komm_room(p, "Die Paladinmuenze sagt:", txt);
        return;
    }

    write("Die Paladinmuenze sagt: " + txt + "\n");
}

void paladin_ruft(string formel)
{
    object p;
    string ruf, name;

    p = this_player();
    ruf = formel ? formel : "";

    if(strlen(ruf) > 0 &&
       (ruf[strlen(ruf)-1] == '.' || ruf[strlen(ruf)-1] == '?' ||
        ruf[strlen(ruf)-1] == '!'))
        ruf = ruf[0..strlen(ruf)-2];

    ruf += "!";

    if(p)
    {
        komm(p, "Du rufst:", ruf);
        name = capname(p);
        komm_room(p, name + " ruft:", ruf);
        return;
    }

    write("Du rufst: " + ruf + "\n");
}

/* ------------------------------------------------------------
 * Persistenz / Autoload
 * ------------------------------------------------------------ */

mixed query_auto_load()
{
    sicher_init();
    return ({
        resonanz, rang, tips_off, gp, last_gp_time,
        makel, eid_brueche, pruefungen, kills, weihen, heilungen, reinigungen,
        talent, relikt1, relikt2, relikt3, raumweihen
    });
}

void init_arg(mixed arg)
{
    if(pointerp(arg) && sizeof(arg) >= 18)
    {
        resonanz = arg[0];
        rang = arg[1];
        tips_off = arg[2];
        gp = arg[3];
        last_gp_time = arg[4];
        makel = arg[5];
        eid_brueche = arg[6];
        pruefungen = arg[7];
        kills = arg[8];
        weihen = arg[9];
        heilungen = arg[10];
        reinigungen = arg[11];
        talent = arg[12];
        relikt1 = arg[13];
        relikt2 = arg[14];
        relikt3 = arg[15];
        raumweihen = arg[16];
    }
    else if(pointerp(arg) && sizeof(arg) >= 5)
    {
        resonanz = arg[0];
        rang = arg[1];
        tips_off = arg[2];
        gp = arg[3];
        last_gp_time = arg[4];
    }

    sicher_init();
    if(gp <= 0) gp = gp_max();
    set_heart_beat(1);
}

/* ------------------------------------------------------------
 * Regeneration / Status
 * ------------------------------------------------------------ */

varargs void regen_all(int show)
{
    int steps, oldgp, gain_gp, gain_apzp;
    object owner;

    sicher_init();
    oldgp = gp;
    steps = (time() - last_gp_time) / 30;

    if(steps > 0)
    {
        gain_gp = steps * gp_regen();
        gain_apzp = steps * apzp_regen();

        gp += gain_gp;
        last_gp_time += steps * 30;

        owner = environment(this_object());
        if(objectp(owner) && playerp(owner) && gain_apzp > 0)
        {
            catch(owner->add_hp(gain_apzp));
            catch(owner->add_sp(gain_apzp));
        }
    }

    if(gp > gp_max()) gp = gp_max();

    owner = environment(this_object());
    if(show && objectp(owner) && playerp(owner) && steps > 0)
        tell_object(owner,
"Paladinregeneration: +" + (steps * apzp_regen()) + " AP, +" +
(steps * apzp_regen()) + " ZP, +" + (gp - oldgp) + " GP.  GP: " +
gp + "/" + gp_max() + "\n");
}

void heart_beat()
{
    regen_all(1);
}

void zeige_gp()
{
    regen_all(0);
    write("GP: " + gp + "/" + gp_max() + "\n");
}

void zeige_koerper()
{
    object p;

    p = this_player();
    if(!p) return;

    write("AP: " + p->query_hp() + "/" + p->query_max_hp() +
          "  ZP: " + p->query_sp() + "/" + p->query_max_sp() +
          "  GP: " + gp + "/" + gp_max() + "\n");
}

int pgp(string str)
{
    zeige_gp();
    return 1;
}

int paladin_sp(string str)
{
    regen_all(0);
    write("Paladin-Rang: " + rang + "/100 - " + rang_name() +
          " | GP: " + gp + "/" + gp_max() +
          " | Regeneration: +" + apzp_regen() + " AP/ZP und +" +
          gp_regen() + " GP je 30 Sekunden\n");
    return 0;
}

string balken()
{
    int max, filled, i;
    string s;

    max = rang_kosten();
    filled = resonanz * 20 / max;
    if(filled > 20) filled = 20;
    if(filled < 0) filled = 0;

    s = "[";
    for(i=0; i<20; i++) s += i < filled ? "#" : "-";
    return s + "]";
}

int pstatus(string str)
{
    int rest;

    regen_all(0);
    write("\nPaladinmuenze der Sonne\n");
    write("Rang: " + rang + "/100 - " + rang_name() + "\n");
    write("Resonanz: " + resonanz + " / " + rang_kosten() + " " + balken() + "\n");
    write("GP: " + gp + "/" + gp_max() + "  +" + gp_regen() + " GP je 30 Sekunden\n");
    write("AP/ZP-Bonusregeneration: +" + apzp_regen() + " je 30 Sekunden\n");

    if(schild_bis > time())
    {
        rest = schild_bis - time();
        write("Gottesschild: aktiv, noch " + rest + " Sekunden.\n");
    }
    else write("Gottesschild: nicht aktiv.\n");

    write("Talent: " + (talent != "" ? talent : "noch nicht gewaehlt") + "\n");
    write("Relikte: " + (relikt1!=""?relikt1:"-") + " | " +
          (relikt2!=""?relikt2:"-") + " | " + (relikt3!=""?relikt3:"-") + "\n");
    write("Makel: " + makel + "  Eidbrueche: " + eid_brueche + "\n\n");
    return 1;
}

/* ------------------------------------------------------------
 * Progression
 * ------------------------------------------------------------ */

int ist_geprueft()
{
    if(rang < 10) return 1;
    return pruefungen >= rang / 10;
}

void add_resonanz(int x)
{
    sicher_init();

    if(makel > 0) x = x * (100 - makel * 3) / 100;
    if(x < 1) x = 1;

    resonanz += x;

    while(rang < 100 && resonanz >= rang_kosten())
    {
        if(rang >= 10 && !ist_geprueft())
        {
            muenze_sagt("Dein Rang verlangt eine Pruefung. Rufe ppruefung.");
            break;
        }

        resonanz -= rang_kosten();
        rang++;
        gp = gp_max();
        muenze_sagt("Deine Resonanz steigt auf Rang " + rang + ": " + rang_name() + ".");
    }
}

void paladin_kill(object monster, object killer, string quelle)
{
    string key;
    int gain, boese, gp_gain;

    if(!killer || killer != environment(this_object()) || !monster) return;

    sicher_init();
    key = object_name(monster);
    if(!key || belohnt[key]) return;
    belohnt[key] = time();

    boese = 0;
    catch(boese = monster->query_align() < -100);

    gain = 20 + rang / 2;
    if(boese) gain += 15;
    if(talent == "richter") gain += 10;

    gp_gain = 8 + rang / 10;
    gp += gp_gain;
    if(gp > gp_max()) gp = gp_max();

    kills++;

    tell_object(killer,
"Die Paladinmuenze saugt einen Splitter vergehender Finsternis auf.\n"
"Resonanz +" + gain + " | GP +" + gp_gain +
" | Getoetete Wesen fuer den Eid: " + kills + "\n");

    add_resonanz(gain);
}

int spend_gp(int kosten, string gabe)
{
    regen_all(0);

    if(in_weihe()) kosten = kosten * 9 / 10;
    if(talent == "templer" && (gabe == "den Gottesschild" || gabe == "die Weihe"))
        kosten = kosten * 9 / 10;

    if(gp < kosten)
    {
        muenze_sagt("Fuer " + gabe + " fehlen Dir Gabenpunkte.");
        write("GP: " + gp + "/" + gp_max() + "\n");
        return 0;
    }

    gp -= kosten;
    zeige_gp();
    return 1;
}

/* ------------------------------------------------------------
 * Hilfen
 * ------------------------------------------------------------ */

int philfe(string str)
{
    write(
"Sonnengaben der Paladinmuenze\n"
"-----------------------------\n"
"  pschwert    25 GP  Paladin-Breitschwert aus Sonnenlicht, 5 Minuten.\n"
"  pschild     60 GP  Gottesschild mit goldenen Fluegeln, 3 Minuten.\n"
"  pweihe      75 GP  Weiht den Raum; trifft Monster, nie Spieler.\n"
"  preinigen   40 GP  Reinigung von Daemonischem, Fluechen und Schatten.\n"
"  pwille      55 GP  Bann. Mit Ziel: pwille kadmos. Ohne Ziel: Raum.\n"
"  pheile      50 GP  Heilung. Ohne Ziel Dich, sonst einen Spieler im Raum.\n"
"  psegnung    35 GP  Segnung, Resonanz und Makelpflege.\n"
"\n"
"Progression: pstatus, pgp, peid, pquest, ptalent, prelikt, ppruefung.\n"
"Gott/Test: pgott gp, pgott rang <zahl>, pgott pruefung, pgott rein.\n");
    return 1;
}

int hilfe_paladin(string str)
{
    str = norm(str);
    if(str == "paladin" || str == "sonnengaben")
        return philfe(str);
    return 0;
}

int peid(string str)
{
    write(
"Der Eid der Sonnenmuenze\n"
"------------------------\n"
"  Trage Licht, nicht Hochmut.\n"
"  Fuehre bei aktivem Lichtbreitschwert keine andere Waffe.\n"
"  Fluche nicht mit gebundenem Sonnenzeichen.\n"
"  Weihe verletzt keine Spieler.\n"
"\n"
"Makel: " + makel + "  Eidbrueche: " + eid_brueche + "\n");
    return 1;
}

int pquest(string str)
{
    object env;
    string key;
    int rw;

    env = this_player() ? environment(this_player()) : 0;
    key = env ? object_name(env) : "";
    rw = (raumweihen && key && raumweihen[key]) ? raumweihen[key] : 0;

    write(
"Paladinprogression\n"
"------------------\n"
"  Rang: " + rang + "/100 - " + rang_name() + "\n"
"  Getoetete Monster: " + kills + "\n"
"  Gewirkte Weihen: " + weihen + "\n"
"  Heilungen: " + heilungen + "\n"
"  Reinigungen: " + reinigungen + "\n"
"  Pruefungen bestanden: " + pruefungen + "/10\n"
"  Weihekraft dieses Raumes: " + rw + "/3\n");
    return 1;
}

/* ------------------------------------------------------------
 * Talente / Relikte / Pruefung / Gott
 * ------------------------------------------------------------ */

int ptalent(string str)
{
    str = norm(str);

    if(!str || str == "")
    {
        write(
"Talentpfade ab Rang 10\n"
"----------------------\n"
"  ptalent seraph    Heilung, Schutz, Regeneration\n"
"  ptalent richter   Schaden/Resonanz gegen Boeses\n"
"  ptalent templer   Schild, Weihe, GP-Oekonomie\n"
"  ptalent exorzist  Reinigung und Bann\n");
        return 1;
    }

    if(rang < 10) { write("Talentpfade oeffnen sich erst ab Rang 10.\n"); return 1; }
    if(talent != "") { write("Dein Talentpfad ist bereits: " + talent + ".\n"); return 1; }

    if(str != "seraph" && str != "richter" && str != "templer" && str != "exorzist")
    { write("Waehle: seraph, richter, templer oder exorzist.\n"); return 1; }

    talent = str;
    muenze_sagt("Dein Pfad ist nun " + talent + ".");
    return 1;
}

int prelikt(string str)
{
    str = norm(str);

    if(!str || str == "")
    {
        write(
"Reliktfassungen\n"
"---------------\n"
"  Fassung I   Rang 5:  " + (relikt1!=""?relikt1:"leer") + "\n"
"  Fassung II  Rang 15: " + (relikt2!=""?relikt2:"leer") + "\n"
"  Fassung III Rang 30: " + (relikt3!=""?relikt3:"leer") + "\n"
"Test: prelikt sonnenkern, prelikt runenrand, prelikt engelsplitter\n");
        return 1;
    }

    if(str == "sonnenkern")
    {
        if(rang < 5) { write("Der Sonnenkern verlangt Rang 5.\n"); return 1; }
        relikt1 = "sonnenkern"; write("Der Sonnenkern rastet in Fassung I ein.\n"); return 1;
    }
    if(str == "runenrand")
    {
        if(rang < 15) { write("Der Runenrand verlangt Rang 15.\n"); return 1; }
        relikt2 = "runenrand"; write("Der Runenrand schliesst sich um die Muenze.\n"); return 1;
    }
    if(str == "engelsplitter")
    {
        if(rang < 30) { write("Der Engelsplitter verlangt Rang 30.\n"); return 1; }
        relikt3 = "engelsplitter"; write("Der Engelsplitter faengt Licht wie eine gefrorene Feder.\n"); return 1;
    }

    write("Unbekanntes Relikt.\n");
    return 1;
}

int ppruefung(string str)
{
    int ziel;

    ziel = rang / 10;
    if(rang < 10 || ziel <= pruefungen)
    { write("Derzeit wartet keine Paladinpruefung auf Dich.\n"); return 1; }

    write(
"Paladinpruefung " + ziel + "\n"
"----------------\n"
"  Getoetete Monster: " + kills + " / " + (ziel * 2) + "\n"
"  Weihen: " + weihen + " / " + ziel + "\n"
"  Heilungen: " + heilungen + " / " + ziel + "\n"
"  Reinigungen: " + reinigungen + " / " + ziel + "\n");

    if(kills >= ziel * 2 && weihen >= ziel && heilungen >= ziel && reinigungen >= ziel)
    {
        pruefungen = ziel;
        muenze_sagt("Die Pruefung ist bestanden. Dein Eid darf weiter wachsen.");
    }
    else muenze_sagt("Die Pruefung ist noch nicht vollendet.");

    return 1;
}

int pgott(string str)
{
    int level, wert;

    level = 0;
    catch(level = this_player()->query_level());
    if(level < 20) { write("Dieser Befehl ist nur fuer Goetter/Testumgebungen gedacht.\n"); return 1; }

    str = norm(str);

    if(!str || str == "" || str == "gp")
    {
        gp = gp_max();
        write("Gotteskraft fuellt Deine Gabenpunkte. GP: " + gp + "/" + gp_max() + "\n");
        return 1;
    }
    if(sscanf(str, "gp %d", wert) == 1)
    {
        gp = wert; if(gp > gp_max()) gp = gp_max(); if(gp < 0) gp = 0;
        write("Gabenpunkte gesetzt. GP: " + gp + "/" + gp_max() + "\n");
        return 1;
    }
    if(sscanf(str, "rang %d", wert) == 1)
    {
        rang = wert; if(rang < 1) rang = 1; if(rang > 100) rang = 100;
        gp = gp_max();
        write("Paladinrang gesetzt: " + rang + " - " + rang_name() + "\n");
        return 1;
    }
    if(str == "pruefung") { pruefungen = 10; write("Alle Paladinpruefungen freigeschaltet.\n"); return 1; }
    if(str == "rein") { makel = 0; eid_brueche = 0; write("Makel und Eidbrueche gereinigt.\n"); return 1; }

    write("pgott gp [zahl], pgott rang <zahl>, pgott pruefung, pgott rein\n");
    return 1;
}

/* ------------------------------------------------------------
 * Gaben
 * ------------------------------------------------------------ */

int block_andere_waffen(string str)
{
    object sw;

    sw = present("lichtbreitschwert", this_player());
    if(!sw || !str) return 0;

    str = norm(str);
    if(strstr(str, "lichtschwert") >= 0 ||
       strstr(str, "lichtbreitschwert") >= 0 ||
       strstr(str, "paladinschwert") >= 0)
        return 0;

    write("Das Lichtbreitschwert ist eine zweihändige Bindungswaffe. Die Muenze duldet keine zweite Waffe.\n");
    return 1;
}

int eid_fluch(string str)
{
    int schaden;

    makel++;
    eid_brueche++;
    schaden = 5 + makel * 3;
    gp -= 15; if(gp < 0) gp = 0;

    write("Das Sonnenzeichen brennt schmerzhaft auf. Der Eid duldet keinen Fluch.\n");
    this_player()->add_hp(-schaden);
    zeige_koerper();
    return 1;
}

int pschwert(string str)
{
    object ob;

    ob = present("lichtbreitschwert", this_player());
    if(ob)
    {
        ob->set_duration(300);
        write("Das Paladin-Breitschwert flammt neu auf. Seine Dauer wird erneuert.\n");
        return 1;
    }

    if(!spend_gp(25, "das Paladin-Breitschwert")) return 1;

    ob = clone_object(SCHWERT);
    if(ob && ob->move(this_player()) == MOVE_OK)
    {
        catch(this_player()->set_align(1000));
        catch(ob->set_owner(this_player()));
        catch(ob->set_duration(300));
        write("Lichtfasern straeuben sich auseinander und verweben sich zu einem Paladin-Breitschwert.\n");
        say(capname(this_player()) + " ruft ein riesiges Paladin-Breitschwert aus Licht.\n");
        add_resonanz(7);
    }
    else write("Das Licht sammelt sich, aber kein Schwert entsteht.\n");

    return 1;
}

int pschild(string str)
{
    object ob;
    int dauer;

    if(schild_bis > time()) { write("Der Gottesschild brennt bereits um Dich.\n"); return 1; }
    if(!spend_gp(60, "den Gottesschild")) return 1;

    ob = clone_object(SCHILD);
    if(ob && ob->move(this_player()) == MOVE_OK)
    {
        dauer = 180;
        if(talent == "seraph") dauer += 60;
        schild_bis = time() + dauer;
        catch(ob->set_owner(this_player()));
        catch(ob->set_duration(dauer));
        write("Du hebst die Hand.\nUeber Dir reisst der Himmel in goldenem Licht auf.\n");
        write("Aus Deinem Ruecken wachsen fuer einen Atemzug gewaltige goldene Fluegel.\n");
        say("Ein Lichtstrahl faellt auf " + capname(this_player()) +
            ". Goldene Fluegel flammen auf.\n");
        add_resonanz(10);
    }
    else write("Der Schild entsteht nicht. Etwas stoert das Licht.\n");

    return 1;
}

int pweihe(string str)
{
    object env, ob;
    int dmg, kraft;
    string key;

    env = environment(this_player());
    if(!env) return 1;

    if(present("paladinweihe", env)) { write("Der Raum ist bereits geweiht.\n"); return 1; }
    if(!spend_gp(75, "die Weihe")) return 1;

    dmg = 15 + rang / 3;
    if(talent == "richter") dmg += 5;

    kraft = 7;
    if(talent == "templer") kraft += 2;
    if(relikt2 == "runenrand") kraft += 2;

    ob = clone_object(WEIHE);
    if(ob && ob->move(env) == MOVE_OK)
        catch(ob->setup(this_player(), dmg, kraft));

    key = object_name(env);
    if(!raumweihen[key]) raumweihen[key] = 0;
    raumweihen[key]++;
    weihen++;

    write("Ein Kreis aus Sonnenrunen brennt in das Pflaster, gross wie ein Altar.\n");
    say(capname(this_player()) + " weiht den Raum mit einem Kreis aus Sonnenrunen.\n");
    add_resonanz(12);
    return 1;
}

int preinigen(string str)
{
    object env, *obs, ob;
    int i, removed;
    string n;

    if(!spend_gp(40, "die Reinigung")) return 1;
    paladin_ruft("Vult lava quod est sordium.");

    env = environment(this_player());
    if(!env) return 1;

    obs = all_inventory(env);
    for(i=0; i<sizeof(obs); i++)
    {
        ob = obs[i];
        if(ob == this_player() || playerp(ob)) continue;
        n = object_name(ob);
        catch(n += " " + ob->query_name());
        n = norm(n);
        if(strstr(n, "daemon") >= 0 || strstr(n, "daem") >= 0 ||
           strstr(n, "demon") >= 0 || strstr(n, "fluch") >= 0 ||
           strstr(n, "zauber") >= 0 || strstr(n, "schatten") >= 0)
        {
            tell_room(env, "Ein unreines Ding vergeht im Licht der Reinigung.\n");
            catch(ob->remove());
            removed++;
        }
    }

    if(makel > 0) { makel--; write("Ein Teil Deines Makels verbrennt im Licht.\n"); }
    reinigungen++;
    add_resonanz(8 + removed * 5);
    return 1;
}

void wille_release(object ob)
{
    int old;

    if(!ob || !gebannte) return;
    if(member(gebannte, ob) >= 0)
    {
        old = gebannte[ob];
        if(old) catch(ob->set_aggressive(1));
        m_delete(gebannte, ob);
    }
}

int pwille(string str)
{
    object env, *obs, ob;
    int i, count, old, dur;

    env = environment(this_player());
    if(!env) return 1;

    if(str && str != "")
    {
        ob = present(norm(str), env);
        if(!ob || !living(ob) || ob == this_player() || playerp(ob))
        { write("Dieses Ziel kannst Du nicht mit heiliger Willenskraft bannen.\n"); return 1; }
    }

    if(!spend_gp(55, "den Willen")) return 1;
    paladin_ruft("Vult riga quod est aridum.");

    dur = 8 + rang / 10;
    if(talent == "exorzist") dur += 5;
    if(dur > 35) dur = 35;

    if(ob)
    {
        old = 0; catch(old = ob->query_aggressive());
        gebannte[ob] = old;
        catch(ob->stop_all_fights());
        catch(ob->set_aggressive(0));
        call_out("wille_release", dur, ob);
        write(capname(ob) + " wird von Deinem Willen gebannt.\n");
        add_resonanz(10);
        return 1;
    }

    obs = all_inventory(env);
    for(i=0; i<sizeof(obs); i++)
    {
        ob = obs[i];
        if(!living(ob) || ob == this_player() || playerp(ob)) continue;
        old = 0; catch(old = ob->query_aggressive());
        gebannte[ob] = old;
        catch(ob->stop_all_fights());
        catch(ob->set_aggressive(0));
        call_out("wille_release", dur, ob);
        count++;
    }

    write(count ? count + " Wesen werden von Deinem Willen gebannt.\n" :
                  "Dein Wille erfuellt den Raum, doch kein Feind stemmt sich dagegen.\n");
    add_resonanz(8 + count * 3);
    return 1;
}

int pheile(string str)
{
    object env, ziel;
    int menge;

    env = environment(this_player());
    if(!str || str == "" || norm(str) == "mich") ziel = this_player();
    else if(env) ziel = present(norm(str), env);

    if(!ziel || !living(ziel)) { write("Dieses Ziel kannst Du nicht heilen.\n"); return 1; }
    if(ziel != this_player() && !playerp(ziel))
    { write("Die Muenze weigert sich, ein feindliches Wesen zu heilen.\n"); return 1; }

    if(!spend_gp(50, "die Heilung")) return 1;
    paladin_ruft("Vult sana quod est saucium.");

    menge = 90 + rang / 2;
    if(talent == "seraph") menge = menge * 5 / 4;
    if(relikt3 == "engelsplitter") menge += 25;

    ziel->add_hp(menge);
    ziel->add_sp(8 + rang / 10);
    heilungen++;
    zeige_koerper();
    add_resonanz(7);
    return 1;
}

int psegnung(string str)
{
    int bonus;

    if(!spend_gp(35, "die Segnung")) return 1;
    paladin_ruft("Vult benedic quod est lumen.");

    bonus = 6 + rang / 10;
    write("Goldfunken steigen aus der Paladinmuenze und legen sich wie warmer Tau auf Deinen Eid.\n");

    if(in_weihe())
    {
        gp += 15; if(gp > gp_max()) gp = gp_max();
        write("Die Sonnenweihe antwortet auf Deine Segnung. GP +15.\n");
    }

    if(makel > 0) { makel--; write("Ein dunkler Riss im Sonnenzeichen schliesst sich. Makel -1.\n"); }

    add_resonanz(bonus);
    zeige_gp();
    return 1;
}

/* ------------------------------------------------------------
 * Münzendialog nach Bindung / Aussehen
 * ------------------------------------------------------------ */

int sage_muenze(string str)
{
    string s;

    s = norm(str);
    if(strstr(s, "muenze") < 0 && strstr(s, "sonne") < 0 && strstr(s, "paladin") < 0)
        return 0;

    if(strstr(s, "keine tipps") >= 0 || strstr(s, "tipps aus") >= 0)
    {
        tips_off = 1; muenze_sagt("Dann schweige ich, bis Du mich wieder rufst."); return 1;
    }
    if(strstr(s, "tipps an") >= 0 || strstr(s, "gib tipps") >= 0)
    {
        tips_off = 0; muenze_sagt("Dann spreche ich wieder, wenn Licht einen Weg kennt."); return 1;
    }

    if(strstr(s, "hallo") >= 0) muenze_sagt("Hallo, Paladin. Dein Weg ist noch jung, aber nicht leer.");
    else if(strstr(s, "gabe") >= 0) muenze_sagt("Rufe 'hilfe sonnengaben'. Namen sind Tueren, und Du hast den Schluessel.");
    else muenze_sagt("Ich hoere. Doch nicht jedes Licht muss sofort Antwort sein.");

    return 1;
}

string extra_look()
{
    object p, sw, sh;

    p = environment(this_object());
    if(!p || !living(p)) return 0;

    sw = present("lichtbreitschwert", p);
    sh = present("gottesschild", p);

    if(sw && sh)
        return capname(p) + " wirkt wie eine Gotteserscheinung: Goldfunken wabern um die Gestalt, ein Paladin-Breitschwert brennt in den Haenden, und goldene Fluegel flammen hinter einem kreisenden Gottesschild.\n";
    if(sh)
        return capname(p) + " ist von Goldfunken umgeben; hinter der Gestalt flammen goldene Fluegel aus Licht.\n";
    if(sw)
        return capname(p) + " traegt Goldfunken auf Haut und Kleidung; in den Haenden brennt ein Paladin-Breitschwert aus weissgoldenem Licht.\n";
    return 0;
}

/* ------------------------------------------------------------
 * Befehlsregistrierung
 * ------------------------------------------------------------ */

void init()
{
    if(this_player() && environment(this_object()) &&
       this_player() != environment(this_object()))
        return;

    add_action("paladin_sp", "sp");
    add_action("block_andere_waffen", "fuehre");
    add_action("block_andere_waffen", "fuehr");
    add_action("block_andere_waffen", "zuecke");

    add_action("eid_fluch", "fluche");
    add_action("eid_fluch", "fluch");
    add_action("eid_fluch", "verfluche");

    add_action("pschwert", "pschwert");
    add_action("pschwert", "paladin");
    add_action("pschild", "pschild");
    add_action("pweihe", "pweihe");
    add_action("preinigen", "preinigen");
    add_action("pwille", "pwille");
    add_action("pheile", "pheile");
    add_action("psegnung", "psegnung");

    add_action("pstatus", "pstatus");
    add_action("pgp", "pgp");
    add_action("philfe", "philfe");
    add_action("philfe", "sonnengaben");
    add_action("hilfe_paladin", "hilfe");

    add_action("peid", "peid");
    add_action("pquest", "pquest");
    add_action("ptalent", "ptalent");
    add_action("prelikt", "prelikt");
    add_action("ppruefung", "ppruefung");
    add_action("pgott", "pgott");

    add_action("sage_muenze", "sag");
    add_action("sage_muenze", "sage");
    add_action("sage_muenze", "sag:");
    add_action("sage_muenze", "sage:");

    regen_all(0);
}

void force_init()
{
    init();
}

int query_sellable() { return 0; }
int query_giveable() { return 0; }
int query_dropable() { return 0; }
