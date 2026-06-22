// This file is part of UNItopia Mudlib.
// -----------------------------------------------------------------------
//  Datei:  /room/rathaus/treppennische.c
//  Autor:  Myonara 08.Nov.2014 
// -----------------------------------------------------------------------
// Beschreibung: Ein Raum mit Hinweisen zum Goetterdasein.
// -----------------------------------------------------------------------

inherit "/i/room";

#include <apps.h>
#include <level.h>
#include <message.h>
#include <misc.h>
#include <move.h>
#include <notify_fail.h>
#include <room_types.h>

#define PDBG(x) send_message_to( ({find_player("myonara") }), \
                    MT_DEBUG, MA_UNKNOWN, wrap(x))
#define LOG_FILE "/log/glocke_zur_gott_werdung"
#define AUSGANG_LEO "/room/rathaus/reinkarnation"
#define TONPRUEFER "/room/rathaus/obj/soundchecker"

// Struktur [ix] -> ({ string key, string beschreibung })
private mixed * hinweisliste;
// Struktur ["schluessel"] => ix
private mapping hinweislinks;
// Struktur [wizp-object] => mapping ([idle_seit])
private mapping wizob = ([ ]);
// Struktur [player_realname] => int letzter klingler
private mapping plob = ([ ]);

private int add_hinweis(mixed keys, string text)
{
    int ix, ik;
    if (stringp(keys))
    {
        keys = ({ keys });
    }
    ix = sizeof(hinweisliste);
    hinweisliste += ({ ({ keys[0], text }) });
    for (ik = 0; ik < sizeof(keys); ik++)
    {
        hinweislinks[convert_umlaute(lower_case(keys[ik]))] = ix;
    }
    return ix;
}

string lies_hinweise(string parse_rest, string str, mapping was, object leser)
{
    int ix = was["nummer"];
    string * hlis;
    parse_rest = convert_umlaute(space(parse_rest));
    if (strstr(parse_rest,"fuer ")==0)
    {
        parse_rest = lower_case(parse_rest[5..]);
        if (member(hinweislinks,parse_rest))
        {
            ix = hinweislinks[parse_rest];
        }
    } 
    else if (strstr(parse_rest,"zu ")==0)
    {
        parse_rest = lower_case(parse_rest[3..]);
        if (member(hinweislinks,parse_rest))
        {
            ix = hinweislinks[parse_rest];
        }
    }
    if (ix <= 0 || ix >= sizeof(hinweisliste))
    {
        hlis = map(hinweisliste[1..], (: $1[0] :) );
        return wrap("Man kann die "+(sizeof(hinweisliste)-1)+" Hinweise entweder "
                "der Nummer nach lesen, "
                "oder aber die syntax 'lese hinweis für <stichwort>' nutzen.\n"
                "Folgende Stichwörter sind bekannt:\n"
                +liste(hlis," oder ") );
    }
    return wrap("Der "+ix+". Hinweis zu "+hinweisliste[ix][0]+" lautet:\n"
            +hinweisliste[ix][1]);
}

void init()
{
    add_action("cmd_laeuten","läute",-4);
}

int cmd_laeuten(string str)
{
    object *wizzes;
    int io,ti;
    str = lower_case(space(str));
    if (str != "glocke" || !playerp(TP))
    {
        FAILWP("Willst Du vielleicht die Glocke läuten?",FAIL_NOT_OBJ);
    }
    write_file(LOG_FILE, str = sprintf("%s: %s läutet die Glocke.\n",
            shorttimestr(time()), TP_RCN));
    PDBG(str);
    if (member(plob, TP_RN) && plob[TP_RN] > time())
    {
        FAILWP("Du willst schon wieder läuten, warte erst noch.", 
                FAIL_INTERNAL);
    }
    plob[TP_RN] = time() + 15*60;
    if (wizp(TP))
    {
        FAILWP("Als Gott solltest Du zbrülle nutzen.", FAIL_INTERNAL);
    }
    wizzes = filter(users(), (: vogtp($1) :));
    // GOETTER_REGISTER->query_lehrerlaubnis(TP_RN)
    for (io = 0; io < sizeof(wizzes); io++)
    {
        ti = time() - wizzes[io]->query_idle();
        if (member(wizob, wizzes[io]))
        {
            if (wizob[wizzes[io]]["idle_seit"] == ti)
            {
                wizzes[io] = 0;
            }
            else
            {
                wizob[wizzes[io]]["idle_seit"] = ti;
            }
        }
        else
        {
            wizob[wizzes[io]] = ([ "idle_seit" : ti ]);
        }
    }
    wizzes -= ({ 0 });
    send_message_to(wizzes, MT_DEBUG, MA_NOISE, str);
    send_message_to(TP, MT_NOISE, MA_NOISE, wrap(sizeof(wizzes)>0?
        "Die Glocke gibt einen hellen Klang von sich."
        : "Die Glocke ist stumm, keiner hat Dich gehört."));
    return 1;
}

void create() 
{
    int ix;
    "*"::create();
    set_short("Eine göttliche Treppennische");
    set_long("Die Nische unter der Treppe ist vollgepackt mit Büchern "
            "und Hinweisen zur Gottwerdung. "
            "Hier fühlt es sich seltsam an.");
    set_feel("Ein Schaudern durchlaueft dich und du spürst, "
            "wie das Göttern dich den Geheimnissen des Raumzeitgefüges "
            "näher bringen wird. Das Wissen kann allerdings auch vieles "
            "entzaubern, wodurch dein naives Mudweltbild schwer erschüttert "
            "werden kann und du dich womöglich von der Spielerschaft entfernst.");
    add_type(RT_KUNSTLICHT, 1);
    set_own_light(1);
    add_exit(AUSGANG_LEO, "ausgang");
    
    hinweisliste = ({ });
    hinweislinks = ([ ]);
    add_hinweis( "", ""); // Leerstring als Element 0!
    add_hinweis( ({ "Voraussetzung","voraussetzungen","engel","gott","götter" }),
"Als Voraussetzung muss man Engel sein oder der Zweitie eines "
"Engels sein. Man darf mit keinem seiner Charaktere einen Mord begangen haben, "
"bzw. diese müssen verziehen sein."
);
    add_hinweis( ({ "Zweitiegott","zweitie","xtie" }),
"Man kann, wenn man Zweitie eines Engels ist, auch Gott werden. Dazu braucht "
"es neben dem 1. Lehrmeister auch einen Admin, der den Zweitie vom Engel "
"bestätigt. Der Name des Engels ist von den Domainlords "
"und Admins vertraulich zu behandeln."
);
    add_hinweis( ({ "Programmieranfänger","anfänger","neulinge", 
        "neuling"}), 
"Absolute Programmieranfänger werden erstmal von der Informationsflut "
"erschlagen (Enzyclopedia UNItopia, Wiki, diverse Buecher). Trotzdem ist es "
"möglich, Gott zu werden. Hier zeigt die "
"Erfahrung, dass es sich lohnt, eigene Notizen zu machen, bis man sich "
"von selbst zurecht findet. Wichtig sind auch Lehrmeister, die am Anfang "
"zur gleichen Zeit wie einer selbst da sind und alles bis ins Detail "
"erklären können bzw. Schritt für Schritt alles erklären."
);
    add_hinweis( ({ "Spielen","Spieler", "spielie" }),
"Um sich besser auf das Gesellenstück und das Gottsein konzentrieren zu "
"können soll man als Lehrling/Geselle nicht nebenher spielen. "
"Nach Abschluss des Gesellenstücks (und frühestens nach 8 Wochen Gott sein) "
"dürfen Götter mit ihren übrigen Charakteren wieder ganz normal spielen. "
"Gott und Spielie dürfen nie gleichzeitig online sein und als Spielie darf "
"man kein Götterwissen weitergeben."
);
    add_hinweis( ({ "Lehrmeister","lm", "ausbilder","meister" }),
"1,2,3: Man braucht einen Lehrmeister, um Lehrling zu werden, man braucht "
"einen zweiten Lehrmeister, um als Geselle dann endlich im eigenen Bereich "
"programmieren zu dürfen. Und ein dritter Lehrmeister (DL) nimmt u.a. das "
"Gesellenstück ab und ermöglicht, ein vollständiger Vogt im Pantheon zu "
"werden. Die Lehrmeister (kurz LM) sind die ersten Kontaktpersonen, um Fragen "
"zu klären und einem eine Einführung in die Programmiersprache, Konzepte, "
"Götterkodex und vieles mehr zu geben."
);
    add_hinweis( ({ "Lehrling" }),
"Der Lehrling ist die erste Stufe zur Gottwerdung. Als Lehrling hat man "
"keine Schreibrechte und nur eingeschränkte Befehle zur Verfügung, aber "
"man kann sich schon viele Sachen anschauen, z.B. Götterbücher und sogar "
"Programmcode. Man kann nix kaputt machen. "
"Es gibt eine Mindestzeit, bis man Geselle werden kann."
);
    add_hinweis( ({ "Geselle", "gesellin","geselliges" }),
"Der Geselle ist die zweite Stufe zur Gottwerdung. Man hat dann alleinige "
"Schreibrechte im eigenen Bereich und kann Stück für Stück die Grundlagen "
"ausprobieren und dann in Absprache mit den Lehrmeistern ein Gesellenstück "
"abliefern."
);
    add_hinweis( ({ "Gesellenstück", "gs" }),
"Das Gesellenstück ist der erste Beitrag zu UNItopia, den ein Geselle "
"leistet. Es soll eigenständig sein, den technischen und beschreibenden "
"Fähigkeiten des Gesellen entsprechen und bei Abnahme in UNItopia "
"eingebaut werden. Es soll auch zeigen, dass ein Vogt soweit ist, auf "
"eine Domain 'losgelassen' zu werden."
);
    add_hinweis( ({ "Vogt" }),
"Der Vogt ist ein selbstständiger Gott und damit die dritte Stufe der "
"Gottwerdung, der verschiedene Projekte, darunter sein Gesellenstück, "
"pflegt."
);
    add_hinweis( ({ "Domainlord","dl","lord", "lady","domain","gebiet" }),
"Der Domainlord ist ein Zuständiger für ein ganzes Gebiet "
"(Domain oder Fachgebiet) wie zum Beispiel die Ebenen."
);
    add_hinweis( ({ "Sehbehinderte","blind","blinder", "blinde",
            "sehbehindert","sehbehinderter" }),
"Aus Erfahrung ist für Sehbehinderte eine Braillezeile neben der "
"Sprachausgabe dringend empfohlen, da Einrückungen, Sonderzeichen und "
"Zeilenlängen ohne Braillezeile kaum zu bewältigen sind. Ebenso ist ein "
"Korrekturlesen mit Braillezeile einfacher. Es hat sich ebenso gezeigt, "
"dass ein Erlernen des internen Editors ed als Teil der Ausbildung sehr "
"hilfreich ist, da dieser zeilenorientiert arbeitet."
);
    add_hinweis( ({ "Fragen","frage","hinweis","glocke" }),
"Bei weiteren Fragen kann man sich ruhig an einen Gott seines Vertrauens "
"wenden. Oder die Glocke betrachten."
);

    add_v_item( ([
        "name": "hinweise",
        "id" : ({ "hinweise","hinweis" }),
        "gender" : "maennlich",
        "plural" : 1,
        "nummer" : 0,
        "long" : "Zahlreiche Hinweise sind überall verteilt, man kann sie "
                 "lesen.",
        "look_msg" : "",
        "read" : #'lies_hinweise,
        "read_msg" : "",
    ]) );
    add_v_item( ([
        "name": "bücher",
        "id" : ({ "bücher","buch" }),
        "gender" : "saechlich",
        "plural" : 1,
        "long" : "Die zahlreichen Bücher symbolisieren die Informationsflut, "
                 "der sich ein Gott am Anfang gegenüber sieht.",
        "look_msg" : "",
        "read" : "Diese Bücher sind für Götter bestimmt.",
        "read_msg": "",
    ]) );
    add_v_item( ([
        "name" : "glocke",
        "id" :  ({ "glocke" }),
        "gender" : "weiblich",
        "long" : "Die Glocke ist nicht nur eine Glocke, sie merkt sich, "
                 "wer und wann sie läutet und informiert das Pantheon "
                 "darüber. Bitte nur nutzen, wenn man Fragen zur "
                 "oder Interesse an der Gottwerdung hat.",
        "look_msg" : "",
    ]) );
    for (ix = 1; ix < sizeof(hinweisliste); ix ++)
    {
        add_v_item( ([
            "name": "hinweis",
            "id" : ({ "hinweis" }),
            "gender" : "maennlich",
            "nummer" : ix,
            "long" : "Ein Hinweis über "+hinweisliste[ix][0]+".",
            "look_msg" : "",
            "read" : #'lies_hinweise,
            "read_msg" : "",
        ]) );
    }
    
    object top = clone_object(TONPRUEFER);
    top->move(TO,([MOVE_FLAGS:MOVE_ERR_REMOVE]));
}
