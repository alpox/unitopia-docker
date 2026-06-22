// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/room/rathaus/obj/tips_formblatt.c
// Description: Formblatt fuer Spieler, um neue Tips zu erzeugen.
// Author:	Sissi (14.01.2000)

inherit "/i/item";
inherit "/i/install";
inherit "../i/new_tip";

#include <message.h>
 
void create()
{
    set_name ("Formblatt für neue Tips");
    set_gender ("saechlich");
    set_id (({"formblatt","blatt","form"}));
    set_weight (0);
    set_no_move_reason (wrap_say("Das Formblatt sagt:",
        "Es ist nicht gestattet, mich wegzulegen, wegzustecken "
        "oder wegzugeben. Du musst mich schon ausfüllen.")
        +"Das Formblatt lächelt Dich fröhlich an.");
    set_long ("Ein ödes, langweiliges Formblatt, wie man es "
        "eben in Rathäusern bekommt. Wie es sich für ein richtiges "
        "Formblatt gehört, kann man es lesen. Das Aufregendste "
        "an dem Formblatt dürfte allerdings sein, dass Du es ausfüllen "
        "kannst, schließlich ist das Ausgefülltwerden der ganze "
        "Lebensinhalt eines solchen Formblattes. Wahrscheinlich würde "
        "es Dir auch Spaß machen, das Formblatt zu zerreißen; das "
        "Formblatt würde davon allerdings eher nicht begeistert "
        "sein.");
    set_read ("Oben auf dem Formblatt steht eine große Überschrift: "
        "\"Einziges behördlich zugelassenes Formblatt für einen "
        "Tipvorschlag\". So in der Art hast Du es "
        "Dir auch vorgestellt. Unter dieser Überschrift steht \"Bitte "
        "hier den Vorschlag für den neuen Tip in leserlicher "
        "Handschrift eintragen:\", als ob Du jemals in Deinem Leben "
        "eine unleserliche Handschrift gehabt hättest... Frechheit. "
        "Danach sind dann einige Zeilen frei, die den Text für einen "
        "Tip aufnehmen und festhalten sollen. Darunter steht dann: "
        "\"Dieser Tip ist geeignet für:\", daneben sind vier Kästchen zum "
        "Ankreuzen mit den Texten \"Anfänger\", \"normale Spieler\", "
        "\"Engel\" und \"Götter\" angebracht. Natürlich sind alle "
        "gleich groß. Na dann viel Spaß beim Ausfüllen.");
    set_smell ("Papiergeruch, kein Zweifel.");
    set_noise ("Du hörst, wie sich das Formblatt darauf freut, von "
        "Dir ausgefüllt zu werden.");
    set_feel ("Fühlt sich aalglatt an. Wie es sich für Formblätter "
        "gehört.");
}

void init ()
{
    add_action ("fuelle","fülle",-4);
    add_action ("zerreisse","zerreiße",-7);
}


static int fuelle (string s)
{
    notify_fail ("fülle was? Vielleicht das Formblatt aus?\n");
    if (me (s) != "aus") return 0;
    this_player()->send_message (
        MT_LOOK, MA_USE,
        wrap (Der(this_player())+" beginnt, "+den()+" auszufüllen."),
        wrap ("Du beginnst, "+den()+" auszufüllen. "
            +(random(10)==3 ? Der()+" freut sich sichtlich darüber. Ach "
                "Quatsch, das bildest Du Dir sicher nur ein.":"")),
        this_player());
    new_tip ();
    return 1;
}

static int zerreisse (string s)
{
    if (!me (s)) {
        notify_fail (("Was willst Du zerreißen? Vielleicht "+den()+"?"));
        return 0;
    }
    send_message (MT_LOOK, MA_USE,
        wrap (Der(this_player())+" zerreißt mit sichtlichem Genuss "
           +den()+" in lauter kleine Fetzen, die sofort vom Wind "
           "davongetragen werden. Welcher Wind? Naja, der Wind halt, "
           "der kleine Fetzen von zerrissenen Formblättern sofort "
           "davonträgt."),
        wrap ("Du zerreißt mit sichtlichem Genuss "
           +den()+" in lauter kleine Fetzen, die sofort vom Wind "
           "davongetragen werden. Welcher Wind? Naja, der Wind halt, "
           "der kleine Fetzen von zerrissenen Formblättern sofort "
           "davonträgt."),
        this_player());
    remove ();
    return 1;
}

void new_tip_done ()
{
    object leo;
    leo = present ("leo",environment(this_player()));
    this_player()->send_message (
        MT_LOOK, MA_USE,
        wrap (Der(this_player())+" hat "+den()+" fertig "
            "ausgefüllt."),
        "",
        this_player());
    if (leo) {
        this_player()->send_message (
            MT_LOOK, MA_MOVE,
            wrap (Der(this_player())+" gibt "+den()+" an "
                +den (leo)+"."),
            wrap ("Du gibst "+den()+" an "+den(leo)+"."),
            this_player());
        leo->do_command ("sage Dankeschön für den neuen Tipvorschlag.");
        leo->send_message (
            MT_LOOK, MA_MOVE,
            wrap ("Leo macht eine merkwürdige Geste mit den Fingern, und "
               +der()+" ist verschwunden. Hoffentlich kommt "
               +er()+" auch da an, wo "+er()+" ankommen soll."));
    } else
        send_message (MT_LOOK, MA_MOVE,
            wrap (Der()+" leuchtet kurz auf und ist dann verschwunden."),
            wrap (Dein()+" leuchtet kurz auf und ist dann verschwunden."),
            this_player());
    remove ();
}
        
