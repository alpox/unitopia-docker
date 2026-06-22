// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /i/living/spells.c
// Description: Die Magischen Grundzauber sind hier definiert.

#pragma save_types
#pragma strong_types

#include <stats.h>
#include <invis.h>
#include <config.h>
#include <message.h>
#include <parse_com.h>
#include <simul_efuns.h>
#include <eyes.h>
#include <mapil.h>

protected functions inherit "/i/tools/mapil";

// Prototypes:
void add_skill_points(string *path, int amount);
void set_own_light(int amount);

#define SICHT_COST  40
#define SINN_COST   10
#define LICHT_COST  30
#define LICHT_DAUER_MIN 90
#define LICHT_DAUER_MAX 120
#define AURA_COST   100
#define AURA_DAUER  15
#define BEWERTE_COST 40

#define SPALTEN 4       /* Fuer die Ausgabe der Ausgaenge. */

int magic_allowed(string begruendung)
{
    mixed tmp;

    tmp = environment(this_object())->query_type("keine_magie");
    if (tmp)
    {   
        if (stringp(tmp))
            return notify_fail(wrap(tmp));
        else
            return notify_fail(wrap(begruendung));
    }
    return 1;
}


int sicht_command(string str)
{
    object env;
    string dest;
    string result;

    if (!str)
        return notify_fail("In welche Richtung?\n");
    env = environment();
    if (!env)
        return notify_fail("Hier geht das nicht!\n");
    if (!magic_allowed("Das darfst du hier nicht.\n"))
        return 0;
    dest = env->query_one_exit(expand_direction(str,DIR_ALS_DEFAULT));
    if (!dest || dest == "" || !(env = touch(dest)))
        return notify_fail("In dieser Richtung gibt es nichts zu erkennen.\n");
    if (this_object()->query_sp() < SICHT_COST)
        return notify_fail("Du hast gerade nicht die nötige Konzentration, "
                    "um dir Aussicht zu verschaffen.\n");
    this_object()->add_sp(-SICHT_COST/2);
    if (!IS_INVIS(this_object()))
        this_object()->send_message( MT_LOOK, MA_MAGIC,
           wrap(Der() + " versetzt sich in den Zustand höchster geistiger "
                "Konzentration."));
    this_object()->send_message_to( this_object(),
        MT_NOTIFY, MA_MAGIC,
        "Du sammelst deine geistigen Kräfte...\n");
    
#ifdef NEW_STATS
    if (random(70) > this_object()->query_stat(STAT_INT) - 15)
#else
    if (random(100) > 20+this_object()->query_stat(STAT_INT))
#endif
        this_object()->send_message_to( this_object(),
           MT_NOTIFY|MT_SENSE, MA_MAGIC,
           "dein Versuch schlägt jedoch fehl.\n");
    else if (env->query_room_domain()=="Pantheon" ||
        env->forbidden("spy_here", this_object()))
        this_object()->send_message_to( this_object(),
           MT_NOTIFY|MT_SENSE, MA_MAGIC,
           "du kannst jedoch gar nichts erkennen.\n");
    else
    {
        this_object()->add_sp(-SICHT_COST/2);
        this_object()->send_message_to( this_object(),
           MT_NOTIFY|MT_SENSE, MA_MAGIC,
           "und vor deinem geistigen Auge erscheint folgendes Bild:\n\n");
        result=this_object()->describe_room(env, EYE_FORCE_LONG);
        this_object()->send_message_to( this_object(),
           MT_NOTIFY|MT_LOOK, MA_MAGIC,
           result);
        add_skill_points(({"skill","zauber","sicht"}),LEARNING_3);
        this_object()->notify("sicht",this_player(),str,env);
    }
    
    mapil_notify(M_B_MAGIE, M_T_INFO, 0,
        "sicht",this_object(),({environment(this_object()),env}),
        result?100:0,
        (["message":result,
          "target":env,]));

    return 1;
}

/*
FUNKTION: notify_sicht
DEKLARATION: void notify_sicht(object wer, string richtung, object zielraum)
BESCHREIBUNG:
Nachdem wer erfolgreich den Zauberspruch 'sicht' in Richtung richtung mit
dem Blick in zielraum beendet hat, wird wer->notify("sicht", wer, richtung,
zielraum) aufgerufen.
VERWEISE: add_controller, notify, describe_room
GRUPPEN: zaubern
*/

int sinn_command(string str)
{
    mixed *parsed, ob;
    int align, ob_align;
    string result;

    parsed = parse_com(str);
    if (parse_com_error(parsed,"Von wem?\n",1))
        return 0;
    if (!magic_allowed("Deine Sinne werden hier gestört.\n"))
        return 0;
    ob = parsed[PARSE_OBS][0];
    if (!(mappingp(ob) && ob["living"]) && !(objectp(ob) && living(ob)))
        return notify_fail(wrap(Der(ob) + plural(" lebt"," leben",ob) + " nicht."));

    if (this_object()->query_sp() < SINN_COST)
        return notify_fail("Für diesen Zauber fehlt dir die nötige "
                "Konzentration.\n");
    this_object()->add_sp(-SINN_COST/2);

    if (!IS_INVIS(this_object()))
        this_object()->send_message(MT_LOOK, MA_MAGIC, wrap(
            Der()+" versetzt sich in den Zustand höchster geistiger "
                "Konzentration."));
    this_object()->send_message_to(this_object(), MT_NOTIFY, MA_MAGIC,
        "Du sammelst deine geistigen Kräfte...\n");
    
#ifdef NEW_STATS
    if (random(70) > this_object()->query_stat(STAT_INT) - 15)
#else
    if (random(100) > 20+this_object()->query_stat(STAT_INT))
#endif
    {
        this_object()->send_message_to(this_object(), MT_NOTIFY, MA_MAGIC,
            "dein Versuch schlägt jedoch fehl.\n");
    }
    else
    {
        this_object()->add_sp(-SINN_COST/2);
        ob_align = QUERY("align",ob);
        align = this_object()->query_align();
        if (ob_align == align)
            result = "Ihr "+plural("beide ","",ob)+"schenkt euch nichts.";
        else if (ob_align <= A_MIES)
        {
            if (align < ob_align)
                result = Der(ob) +
                    plural(" ist zwar selbst kein sehr umgängliches Wesen,",
                       " sind zwar selbst keine sehr umgängliche Wesen",ob) +
                    " doch du bist es noch viel weniger.";
            else if (align > ob_align)
            {
                if (align > A_NETT)
                    result = Der(ob)+ist(ob,3)+"so ziemlich das Gegenteil von dir.";
                else
                    result = Der(ob)+
                        plural(" ist selbst kein sehr umgängliches Wesen,",
                            " sind selbst keine sehr umgängliche Wesen,",ob)+
                          " und du bist nicht viel besser.";
            }
        }
        else if (ob_align > A_NETT)
        {
            if (align > ob_align)
                result = Der(ob)+plural(" hat"," haben",ob)+
                    " zwar selbst eine positive Einstellung, "+
                    plural("kann","können",ob)+
                    " dir aber nicht das Wasser reichen.";
            else if (align < ob_align)
            {
                if (align < A_MIES)
                    result = Der(ob)+
                        plural(" ist im Vergleich zu dir ein Engel.",
                            " sind im Vergleich zu dir Engel.", ob);
                else
                    result = "Du kannst "+dem(ob)+" noch nicht das Wasser reichen.";
            }
        }
        else
        {
            if (align > ob_align)
            {
                if (align > A_NETT)
                    result = Der(ob)+ist(ob,3)+"nicht ganz so umgänglich wie du.";
                else
                    result = "Du bist zwar selbst weder gut noch böse, aber "
                          "immer noch besser als "+der(ob)+".";
            }
            else
            {
                if (align < A_NETT)
                    result = Der(ob)+ist(ob,3)+"zwar selbst weder gut noch böse, "
                          "aber allemal besser als du.";
                else
                    result = Der(ob)+ist(ob,3)+"zwar selbst weder gut noch böse, "
                          "aber du bist nicht viel besser.";
            }
        }
        this_object()->send_message_to(this_object(), MT_NOTIFY, MA_MAGIC,
            wrap(result));
    }
    
    mapil_notify(M_B_HANDWERK, M_T_INFO, 0,
        "sinn",this_object(),ob,result?100:0,
        (["message":result]));
    
    return 1;
}

string feuerkugel_look (mapping vitem, object betrachter)
{
    if (betrachter == this_object())
        return wrap ("Über deinem Kopf schwebt eine kleine Kugel aus "
            "reinem Feuer. Sie ist ganz schön hell. Du hast sie herbei "
            "gezaubert.");
    return wrap ("Über dem Kopf von "+dem()+" schwebt eine kleine "
        "Kugel aus reinem Feuer. Sie ist ganz schön hell.");
}

int licht_command(string str)
{
    object env;
    int result;

    if (str)
        return 0;

    env = environment();
    if (!env)
        return notify_fail("Hier hilft das auch nichts.\n");
    if (!magic_allowed("Das darfst du hier nicht.\n"))
        return 0;
    if (this_object()->query_sp() < LICHT_COST)
        return notify_fail("Du kannst dich nicht richtig auf den Lichtzauber "
                "konzentrieren.\n");
    this_object()->add_sp(-LICHT_COST/2);
    if (!IS_INVIS(this_object()))
        this_object()->send_message(MT_NOTIFY, MA_MAGIC,
           wrap(Der() + " versetzt sich in den Zustand höchster geistiger "
                "Konzentration."));
    this_object()->send_message_to(this_object(), MT_NOTIFY, MA_MAGIC,
        "Du sammelst deine geistigen Kräfte...\n");
#ifdef NEW_STATS
    if (random(70) > this_object()->query_stat(STAT_INT) - 15)
#else
    if (random(100) > 20+this_object()->query_stat(STAT_INT))
#endif
        this_object()->send_message_to(this_object(), MT_NOTIFY, MA_MAGIC,
           "dein Versuch schlägt jedoch fehl.\n");
    else
    {
        this_object()->add_sp(-LICHT_COST/2);
        // TODO: wirklich eine Kugel erzeugen (Objekt)
        this_object()->add_v_item (([
            "name":"feuerkugel", "gender":"weiblich",
            "id":({"kugel","lichtkugel","feuerkugel","feuer#kugel"}),
            "long":#'feuerkugel_look,
            "far":"Die Feuerkugel schwebt viel zu weit oben, "
                  "als dass Du da rankommst. Außerdem ist sie "
                  "sicherlich sehr, sehr heiß."
        ]));
        // Vorerst durch Ueberlagerung von query_own_light() geloest:
        // set_own_light(1);
        this_object()->send_message_to(this_object(), MT_NOTIFY, MA_MAGIC,
            "und du erzeugst eine kleine Feuerkugel.\n");
        if (!IS_INVIS(this_object()))
            this_object()->send_message(MT_LOOK, MA_MAGIC,
                wrap(Der()+" erzeugt eine Feuerkugel."));
        add_skill_points(({"skill","zauber","licht"}),LEARNING_3);
        int licht_dauer = LICHT_DAUER_MIN 
            + random(LICHT_DAUER_MAX - LICHT_DAUER_MIN + 1);
        call_out("licht_aus",licht_dauer);
        result=1; 
    }

    mapil_notify(M_B_MAGIE, M_T_MANIPULATION, 0,
        "licht",this_object(),0,result?100:0,
        ([]));

    return 1;
}

void licht_aus()
{
    // Vorwerst durch Ueberlagerung von query_own_light() geloest:
    // set_own_light(0);
    this_object()->delete_v_item (({"feuer#kugel"}));
    this_object()->send_message_to(this_object(), MT_LOOK, MA_REMOVE,
       "Deine Feuerkugel erlischt.\n");
    if (!IS_INVIS(this_object()))
        this_object()->send_message(MT_LOOK, MA_REMOVE,
            wrap("Die Feuerkugel von "+dem()+" erlischt."));
    mapil_notify(M_B_MAGIE, M_T_MANIPULATION, 0,
        "licht_aus",this_object(),0,100,
        ([]));
}

int aura_command(string str)
{
    object shadow;

    if (str)
        return 0;
    if (this_object()->query_has_aura())
        return notify_fail("Du hast bereits eine Aura!\n");
    if (!magic_allowed("Deine magische Kraft wird hier gestört.\n"))
        return 0;
    if (this_object()->query_sp() < AURA_COST)
        return notify_fail("Du kannst dich nicht richtig auf deine Aura "
                "konzentrieren.\n");
    this_object()->add_sp(-AURA_COST/2);
    if (!IS_INVIS(this_object()))
        this_object()->send_message(MT_LOOK, MA_MAGIC,
            wrap(Der()+" versetzt sich in den Zustand höchster geistiger "
                 "Konzentration."));
    this_object()->send_message_to(this_object(), MT_NOTIFY, MA_MAGIC,
        "Du sammelst deine geistigen Kräfte...\n");
#ifdef NEW_STATS
    if (random(70) > this_object()->query_stat(STAT_INT) - 15)
#else
    if (random(100) > 20+this_object()->query_stat(STAT_INT))
#endif
        this_object()->send_message_to(this_object(), MT_NOTIFY, MA_MAGIC,
            "dein Versuch schlägt jedoch fehl.\n");
    else
    {
        this_object()->add_sp(-AURA_COST/2);
        this_object()->send_message_to(this_object(), MT_NOTIFY, MA_MAGIC,
            "Du umgibst dich mit der Aura der Unantastbarkeit.\n");
        if (!IS_INVIS(this_object()))
            this_object()->send_message(MT_LOOK, MA_MAGIC,
                wrap("Um nichts in der Welt würdest du jetzt " + den() +
                    " angreifen."));
        shadow = clone_object("/obj/aura_shadow");
        shadow->init_aura(this_object(),AURA_DAUER);
        add_skill_points(({"skill","zauber","abwehr"}),LEARNING_4);
    }
    mapil_notify(M_B_MAGIE, M_T_PROTECT, M_F_POSITIVE,
        "aura",this_object(),0,shadow?100:0,
        ([]));
    return 1;
}

private string pv(mixed ob, string ez, string mz)
{
   return ob && QUERY("plural",ob) ? mz : ez;
}

int bewerte_command(string str)
{
    mixed *parsed, ob;
    string action, name;
    int max, min, real, rel_real, rel_max, rel_min;
    string result;

    parsed = parse_com(str);
    if (parse_com_error(parsed,"Bewerte was?\n",1))
        return 0;
    ob = parsed[PARSE_OBS][0];
    if (!objectp(ob))
        return notify_fail(wrap(Dieser(ob)+" "+ist(ob) +
                " keine Waffe, die du benutzen kannst."));
    action = ob->query_weapon_class();
    if (!objectp(ob) || !ob->query_weapon() || !action || action == "" ||
            member(({"nahkampf","schuss","wurf","defensiv"}),action) < 0)
        return notify_fail(wrap(Dieser(ob) + " " + ist(ob) + " keine Waffe."));
    if (!present(ob,this_object()) || !ob->query_wield())
        return notify_fail(wrap("Du musst " + den(ob) + " führen, um " + ihn(ob) +
                " bewerten zu können."));
    if (this_object()->query_sp() < BEWERTE_COST)
        return notify_fail("Du kannst dich nicht richtig für eine gute "
                "Bewertung konzentrieren.\n");
    this_object()->add_sp(-BEWERTE_COST/2);
    if (!IS_INVIS(this_object()))
        this_object()->send_message( MT_LOOK, MA_MAGIC,
            wrap(Der()+" versetzt sich in den Zustand höchster geistiger Konzentration."));
    this_object()->send_message_to( this_object(), MT_NOTIFY, MA_MAGIC,
        "Du sammelst deine geistigen Kräfte...\n");
#ifdef NEW_STATS
    if (random(70) > this_object()->query_stat(STAT_INT) - 15)
#else
    if (random(100) > 20+this_object()->query_stat(STAT_INT))
#endif
        this_object()->send_message_to( this_object(), MT_NOTIFY, MA_MAGIC,
           "dein Versuch schlägt jedoch fehl.\n");
    else
    {
        this_object()->add_sp(-BEWERTE_COST/2);
        min = ob->query_min_damage();
        max = ob->query_max_damage();
        real = ob->compute_damage();
        if ((max-min) == 0)
            rel_real = 100;
        else
            rel_real = ((real-min)*100)/(max-min);
        if (action == "defensiv")
        {
            name = "Verteidigungswaffe";
            rel_max = (max*100)/5;
            rel_min = (min*100)/5;
        }
        else if (action == "wurf" || action == "einmal")
        {
            name = "Wurfwaffe";
            rel_max = (max*100)/50;
            rel_min = (min*100)/50;
        }
        else
        {
            name = "Waffe";
            rel_max = (max*100)/30;
            rel_min = (min*100)/30;
        }
        if (rel_max > 100)
            result=Dieser(ob)+" "+ist(ob)+
                             " eine verbotene "+name+",";
        else if (rel_max > 86)
            result=Dieser(ob)+" "+ist(ob)+
                             " eine absolute Spitzen-"+name+",";
        else if (rel_max > 72)
            result=Dieser(ob)+" "+ist(ob)+ " eine Top-"+name+",";
        else if (rel_max > 58)
            result=Dieser(ob)+" "+ist(ob)+
                             " eine sehr gute "+name+",";
        else if (rel_max > 44)
            result=Dieser(ob)+" "+ist(ob)+ " eine gute "+name+",";
        else if (rel_max > 30)
            result=Dieser(ob)+" "+ist(ob)+
                             " eine normale "+name+",";
        else if (rel_max > 0)
            result=Dieser(ob)+" "+ist(ob)+
                             " eine stinknormale "+name+",";
        else
            result=Er(ob)+" "+ist(ob)+" wohl kaputt.";

        if (rel_real <= 2)
            result+="\naber Du beherrschst "+ihn(ob)+" dafür gar nicht!";
        else if (rel_real <= 16)
            result+="\naber Du benimmst dich damit wie ein Anfänger!";
        else if (rel_real <= 30)
            result+="\naber Du beherrschst "+ihn(ob)+" dafür auch nur mäßig.";
        else if (rel_real <= 44)
            result+="\naber Du beherrschst "+ihn(ob)+" nur mittelmäßig.";
        else if (rel_real <= 58)
            result+="\nund Du beherrschst "+ihn(ob)+" einigermaßen.";
        else if (rel_real <= 72)
            result+="\nund Du beherrschst "+ihn(ob)+" gut.";
        else if (rel_real <= 86)
            result+="\nund Du beherrschst "+ihn(ob)+" sehr gut!";
        else if (rel_real <= 100)
            result+="\nund Du bist mit "+ihm(ob)+" ein richtiger Virtuose!";
        else
            result+="\nund Du kannst mit "+ihm(ob)+" fast Zaubern!";

        if (action == "defensiv")
        {
            if (rel_min >= 80)
                result+="\nAuf jeden Fall "+plural("schützt", "schützen", ob)+
                      " "+er(ob)+" dich sehr gut!";
            else if (rel_min >= 60)
                result+="\nAuf jeden Fall "+plural("schützt", "schützen", ob)+
                      " "+er(ob)+" dich gut!";
            else if (rel_min >= 40)
                result+="\nAuf jeden Fall "+plural("schützt", "schützen", ob)+
                      " "+er(ob)+" dich brauchbar!";
            else if (rel_min > 0 )
                result+="\nAuf jeden Fall "+
                      plural("schützt", "schützen", ob)+" "+er(ob)+" dich!";
        }
        else
        {
            if (rel_min >= 86)
		result += "\nDu kannst damit jemandem schwersten Schaden zufügen!";
            else if (rel_min >= 72)
                result += "\nDu kannst damit jemandem sehr schweren Schaden zufügen!";
            else if (rel_min >= 58)
                result += "\nDu kannst damit jemandem schweren Schaden zufügen!";
            else if (rel_min >= 44)
                result += "\nDu kannst damit jemandem großen Schaden zufügen!";
            else if (rel_min >= 30)
                result += "\nDu kannst damit jemandem größeren Schaden zufügen!";
            else if (rel_min >= 16)
                result += "\nDu kannst damit jemandem Schaden zufügen!";
            else if (rel_min > 0)
                result += "\nDu könntest damit jemandem Schaden zufügen!";
        }
        this_object()->send_message_to( this_object(), MT_NOTIFY, MA_MAGIC, 
            wrap(result));  
        add_skill_points(({"skill","zauber","bewerte"}),LEARNING_3);
    }
    mapil_notify(M_B_HANDWERK, M_T_INFO, 0,
        "bewerte",this_object(),ob,result?100:0,
        (["message":result]));

    return 1;
}
