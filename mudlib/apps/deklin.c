// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /apps/deklin.c
// Description: Hilfsfunktionen und Daten für Grammatikfunktionen
// Author:      Gnomi

#include <error.h>

// Liste von starken oder unregelmäßigen Verben
mapping verben = ([
    "bäckt": ({ "backe", "bäckst", "bäckt", "backen", "backt", "backen" }),
    "befiehlt": ({ "befehle", "befiehlst", "befiehlt", "befehlen", "befehlt", "befehlen" }),
    "birgt": ({ "berge", "birgst", "birgt", "bergen", "bergt", "bergen" }),
    "birst": ({ "berste", "birst", "birst", "bersten", "berstet", "bersten" }),
    "bläst": ({ "blase", "bläst", "bläst", "blasen", "blast", "blasen" }),
    "brät": ({ "brate", "brätst", "brät", "braten", "bratet", "braten" }),
    "bricht": ({ "breche", "brichst", "bricht", "brechen", "brecht", "brechen" }),
    "darf": ({ "darf", "darfst", "darf", "dürfen", "dürft", "dürfen" }),
    "drischt": ({ "dresche", "drischst", "drischt", "dreschen", "drescht", "dreschen" }),
    "erlischt": ({ "erlösche", "erlischst", "erlischt", "erlöschen", "erlöscht", "erlöschen" }),
    "fährt": ({ "fahre", "fährst", "fährt", "fahren", "fahrt", "fahren" }),
    "fällt": ({ "falle", "fällst", "fällt", "fallen", "fallt", "fallen" }),
    "fängt": ({ "fange", "fängst", "fängt", "fangen", "fangt", "fangen" }),
    "frisst": ({ "fresse", "frisst", "frisst", "fressen", "fresst", "fressen" }),
    "gibt": ({ "gebe", "gibst", "gibt", "geben", "gebt", "geben" }),
    "gräbt": ({ "grabe", "gräbst", "gräbt", "graben", "grabt", "graben" }),
    "hält": ({ "halte", "hältst", "hält", "halten", "haltet", "halten" }),
    "hat": ({ "habe", "hast", "hat", "haben", "habt", "haben" }),
    "hilft": ({ "helfe", "hilfst", "hilft", "helfen", "helft", "helfen" }),
    "isst": ({ "esse", "isst", "isst", "essen", "esst", "essen" }),
    "ist": ({ "bin", "bist", "ist", "sind", "seid", "sind" }),
    "kann": ({ "kann", "kannst", "kann", "können", "könnt", "können" }),
    "lädt": ({ "lade", "lädst", "lädt", "laden", "ladet", "laden" }),
    "lässt": ({ "lasse", "lässt", "lässt", "lassen", "lasst", "lassen" }),
    "läuft": ({ "laufe", "läufst", "läuft", "laufen", "lauft", "laufen" }),
    "liest": ({ "lese", "liest", "liest", "lesen", "lest", "lesen" }),
    "mag": ({ "mag", "magst", "mag", "mögen", "mögt", "mögen" }),
    "missfällt": ({ "missfalle", "missfällst", "missfällt", "missfallen", "missfallt", "missfallen" }),
    "misst": ({ "messe", "misst", "misst", "messen", "messt", "messen" }),
    "nimmt": ({ "nehme", "nimmst", "nimmt", "nehmen", "nehmt", "nehmen" }),
    "rät": ({ "rate", "rätst", "rät", "raten", "ratet", "raten" }),
    "schläft": ({ "schlafe", "schläfst", "schläft", "schlafen", "schlaft", "schlafen" }),
    "schlägt": ({ "schlage", "schlägst", "schlägt", "schlagen", "schlagt", "schlagen" }),
    "schmilzt": ({ "schmelze", "schmilzt", "schmilzt", "schmelzen", "schmelzt", "schmelzen" }),
    "sieht": ({ "sehe", "siehst", "sieht", "sehen", "seht", "sehen" }),
    "spricht": ({ "spreche", "sprichst", "spricht", "sprechen", "sprecht", "sprechen" }),
    "sticht": ({ "steche", "stichst", "sticht", "stechen", "stecht", "stechen" }),
    "stiehlt": ({ "stehle", "stiehlst", "stiehlt", "stehlen", "stehlt", "stehlen" }),
    "stirbt": ({ "sterbe", "stirbst", "stirbt", "sterben", "sterbt", "sterben" }),
    "stößt": ({ "stoße", "stößt", "stößt", "stoßen", "stoßt", "stoßen" }),
    "trägt": ({ "trage", "trägst", "trägt", "tragen", "tragt", "tragen" }),
    "trifft": ({ "treffe", "triffst", "trifft", "treffen", "trefft", "treffen" }),
    "tritt": ({ "trete", "trittst", "tritt", "treten", "tretet", "treten" }),
    "verdirbt": ({ "verderbe", "verdirbst", "verdirbt", "verderben", "verdebt", "verderben" }),
    "vergisst": ({ "vergesse", "vergisst", "vergisst", "vergessen", "vergesst", "vergessen" }),
    "wächst": ({ "wachse", "wächst", "wächst", "wachsen", "wachst", "wachsen" }),
    "wäscht": ({ "wasche", "wäschst", "wäscht", "waschen", "wascht", "waschen" }),
    "weiß": ({ "weiß", "weißt", "weiß", "wissen", "wisst", "wissen" }),
    "wirbt": ({ "werbe", "wirbst", "wirbt", "werben", "werbt", "werben" }),
    "wirft": ({ "werfe", "wirfst", "wirft", "werfen", "werft", "werfen" }),
    "will": ({ "will", "willst", "will", "wollen", "wollt", "wollen" }),
]);

/*
FUNKTION: konjugiere
DEKLARATION: string *konjugiere(string verb)
BESCHREIBUNG:
Konjugiert das übergebe Verb und liefert eine Liste der 6 Formen als Array
(1.-3. Person Singular und 1.-3. Person Plural) zurück.

ACHTUNG: Das übergebene Verb muß in der dritten Person Singular stehen.
VERWEISE: verb
GRUPPEN: grammatik
*/
string *konjugiere(string verb)
{
    string stamm;
    string *result = verben[verb];
    if (result)
        return result;

    switch (verb[<1])
    {
        case 't':
            // Präsens
            stamm = verb[..<2];
            if (stamm[<1] == 'e')
                return ({ stamm, stamm + "st", verb, stamm + "n", verb, stamm + "n" });
            if (stamm[<1] in "sxzß")
                return ({ stamm + "e", stamm + "t", verb, stamm + "en", verb, stamm + "en" });
            if (stamm[<2..] in ({ "el", "er"}))
                return ({ stamm + "e", stamm + "st", verb, stamm + "n", verb, stamm + "n" });

            return ({ stamm + "e", stamm + "st", verb, stamm + "en", verb, stamm + "en" });

        case 'e':
            // Präteritum, schwache Verben
            // Konjunktiv I und II.
            return ({ verb, verb + "st", verb, verb + "n", verb + "t", verb + "n" });

        case 'd':
            // Präsens & Präteritum, starke Verben
            if (verb[<2] == 'n')
                return ({ verb, verb + "est", verb, verb + "en", verb + "t", verb + "en" });
            return ({ verb, verb + "st", verb, verb + "en", verb + "t", verb + "en" });

        default:
            // Präsens & Präteritum, starke Verben
            return ({ verb, verb + "st", verb, verb + "en", verb + "t", verb + "en" });
    }
}

void create()
{
    foreach (string form, string* formen: verben)
        foreach (string prefix: ({ "an", "auf", "be", "durch", "über", "um", "unter", "ver", "zer" }))
            m_add(verben, prefix+form, map(formen, function string(string f) { return prefix + f; }));
}
