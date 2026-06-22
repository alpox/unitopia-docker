// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/secure/simul_efun/string.inc
// Description: Funktionen zum vereinfachten Stringhandling
// Author:	Freaky, Francis

#pragma strict_types
#pragma save_types

/* --- Globale Variablen: --- */
// Regexp und Closure fuer convert_umlaute
private static string umlaute_regexp;
private static closure umlaute_konversion;

// Fuer escape_string:
private static closure escape_pcre_wildcard;
private static closure escape_regexp_wildcard;
private static closure escape_regexp_nocase;
private static closure escape_replace;
private static closure escape_getdir;

/* --- Initialisierung: --- */
protected void string_create()
{
    mapping umlaute;

#if __VERSION__ > "3.5.2"
    umlaute = m_allocate(8,1);

    umlaute["\u00e4"] = "ae";
    umlaute["\u00f6"] = "oe";
    umlaute["\u00fc"] = "ue";
    umlaute["\u00c4"] = "Ae";
    umlaute["\u00d6"] = "Oe";
    umlaute["\u00dc"] = "Ue";
    umlaute["\u00df"] = "ss";
    umlaute["\u1e9e"] = "SS";
    umlaute["\u20ac"] = "EUR";

    umlaute_regexp = "[^\t\n -~]";
#else
    umlaute = m_allocate(23,1);
    // UTF-8
    umlaute[to_string(({195,164}))] = "ae";
    umlaute[to_string(({195,182}))] = "oe";
    umlaute[to_string(({195,188}))] = "ue";
    umlaute[to_string(({195,132}))] = "Ae";
    umlaute[to_string(({195,150}))] = "Oe";
    umlaute[to_string(({195,156}))] = "Ue";
    umlaute[to_string(({195,159}))] = "ss";
    umlaute[to_string(({226,130,172}))] = "EUR";

    // ISO-8859-1, ISO-8859-15, Windows-1252
    umlaute[to_string(({228}))] = "ae";
    umlaute[to_string(({246}))] = "oe";
    umlaute[to_string(({252}))] = "ue";
    umlaute[to_string(({196}))] = "Ae";
    umlaute[to_string(({214}))] = "Oe";
    umlaute[to_string(({220}))] = "Ue";
    umlaute[to_string(({223}))] = "ss";
    umlaute[to_string(({128}))] = "EUR";
    umlaute[to_string(({164}))] = "EUR";

    // CP437/CP850
    umlaute[to_string(({132}))] = "ae";
    umlaute[to_string(({148}))] = "oe";
    umlaute[to_string(({129}))] = "ue";
    umlaute[to_string(({142}))] = "Ae";
    umlaute[to_string(({153}))] = "Oe";
    umlaute[to_string(({154}))] = "Ue";
    umlaute[to_string(({225}))] = "ss";

    umlaute_regexp = implode(sort_array(
        filter(m_indices(umlaute),(:sizeof($1)>1:)),
        (: sizeof($1)<sizeof($2) :)),"|") + "|[^\t\n -~]";
#endif

    umlaute_konversion = lambda( ({'a}), ({#'||, ({#'[, umlaute, 'a}), ""}) );
}

/* --- Simul-Efuns: --- */

/*
FUNKTION: liste
DEKLARATION: string liste(string *str [, string pad])
BESCHREIBUNG:
Macht aus einem Stringarray eine normale Aufzaehlung:
    liste(({"bla","schnack","sabbel"})) ->
	"bla, schnack und sabbel"
    liste() -> ""
    liste(({"bla","schnack","sabbel"}), " oder ") ->
	"bla, schnack oder sabbel"
VERWEISE: unliste
GRUPPEN: string, simul_efun
*/
#if defined(TestMUD) && __VERSION__ > "3.6.3"
string liste(string *str, string pad = " und ")
#else
varargs string liste(string *str, string pad)
#endif
{
    if (!sizeof(str))
	return "";
    if (sizeof(str) == 1)
	return str[0];
    return implode(str[0..<2], ", ")+(pad ? pad : " und " )+str[<1];
}


/*
FUNKTION: unliste
DEKLARATION: string * unliste(string str [, string pad, ...])
BESCHREIBUNG:
Versucht aus einer Aufzaehlung wieder ein String-Array zu machen.
Es ist auch moeglich, mehrere pad anzugeben. Gibt man pad als 0
an, so wird gar nicht erst nach einem solchen gesucht.

    unliste("eins, zwei und drei")
        -> ({"eins", "zwei", "drei"})

    unliste("gruen, rot oder blau", " und ", " oder ")
        -> ({"gruen", "rot", "blau"})
VERWEISE: liste
GRUPPEN: string, simul_efun
*/
string * unliste(string str, varargs mixed * paddings)
{
    string pad;

    str = trim(str);

    if(!sizeof(paddings))
        paddings = ({" und "});

    if(paddings[0])
    {
        foreach(pad : paddings)
        {
            str = implode(explode(str, pad), ",");
        }
    }

    return regexplode(str, "[ \t]*,[ \t]*", RE_OMIT_DELIM)-({""});
}


#if __EFUN_DEFINED__(wrap)
#if __VERSION__ < "3.5.0-U041"
private nosave closure wrap_cl = lambda(({'str, 'len, 'left}), ({#'efun::wrap, 'str, 'len, 'left}));
varargs string wrap(string str, int len, int left)
{
    return funcall(wrap_cl, str, len, left);
}
#endif
#else
/*
FUNKTION: wrap
DEKLARATION: string wrap(string str [, int len [, int left]])
BESCHREIBUNG:
wrap bricht einen String um. Leere Strings werden nicht umgebrochen.

<len> ist die Gesamtbreite des Textes. Wenn <len> nicht angegeben wird, wird
auf 75 Zeichen ungebrochen. Der Text kann noch um <left> Zeichen eingerueckt
werden; <len> bleibt dabei die Gesamtbreite inklusive dieser <left>
eingerueckten Zeichen. Will man <left> angeben, ohne <len> anzugeben,
so gibt man fuer <len> einfach 0 an.
GRUPPEN: string, simul_efun
VERWEISE: sprintf, copies, left, right, center, wrap_say
*/
#if defined(TestMUD) && __VERSION__ > "3.6.3"
string wrap(string str, int len = 75, int left = 0)
#else
varargs string wrap(string str, int len, int left)
#endif
{
    if(!sizeof(str))
	return str;

    if (len < 1) len = 75;
    if ((left >= len) || (left < 0)) left = 0;
    str = sprintf("%*s%-=*s\n",left,"",len-left,str);
    if (str[<2..] == "\n\n")
        return str [0..<2];
    else
        return str;
}
#endif


#if __EFUN_DEFINED__(wrap_say)
#if __VERSION__ < "3.5.0-U041"
private nosave closure wrap_say_cl = lambda(({'str1, 'str2, 'len, 'left}), ({#'efun::wrap_say, 'str1, 'str2, 'len, 'left}));
varargs string wrap_say(string str1, string str2, int len, int left)
{
    return funcall(wrap_say_cl, str1, str2, len, left);
}
#endif
#else
/*
FUNKTION: wrap_say
DEKLARATION: string wrap_say(string str1, string str2 [, int len [, int left]])
BESCHREIBUNG:
wrap_say() erzeugt einen String, der etwa so aussieht:
say(wrap_say("Monty sagt:","blablabla ....."));->

Monty sagt: blablabla blablabla blablabla blablabla blablabla blablabla
	blablabla blablabla blablabla blablabla blablabla blablabla
	blablabla blablabla ...

Der erste optionale Parameter bestimmt die Breite, auf die str2 umgebrochen
werden soll.

Der zweite optionale Parameter bestimmt, wie weit links eingerueckt werden
soll. Standardwert ist 8 Zeichen. Aus Einheitlichkeitsgruenden sollte
hier auch nur in Ausnahmefaellen was anderes angegeben werden.

VERWEISE: sprintf, copies, left, right, center, wrap
GRUPPEN: string, simul_efun
*/
#if defined(TestMUD) && __VERSION__ > "3.6.3"
string wrap_say(string str1, string str2, int len = 75, int left = 8)
#else
varargs string wrap_say(string str1, string str2, int len, int left)
#endif
{
    if (len<1)
	len = 75;
    if ((left < 1) || (left > 70))
        left = 8;
    if (sizeof (str1) >= left)
        return sprintf("%s%=-*s\n", str1[0..left-1], len-left, str1[left..]+" "+str2);
    return sprintf("%s %s%=-*s\n", str1, str2[0..left-sizeof(str1)-2], len-left, str2[left-sizeof(str1)-1..<1]);
}
#endif


/*
FUNKTION: copies
DEKLARATION: string copies(string pattern, int times)
BESCHREIBUNG:
Gibt 'times' aneinandergesetzte Kopien von 'pattern' zurueck.

Beispiele:
        copies("*",10)          ->      "**********"
        copies("hu",2)          ->      "huhu"
        copies("tra",0)         ->      ""

GRUPPEN: string, simul_efun
VERWEISE: sprintf, wrap, left, right, center
*/
string copies(string str,int len)
{
    if (len <= 0 || !sizeof(str))
	return "";
    return str * len;
}

/*
FUNKTION: center
DEKLARATION: string center(string text, int len [,string pad] )
BESCHREIBUNG:
Gibt einen String der Breite len zurueck, in dem text zentriert wurde.
Wenn noetig wird mit dem Zeichen pad aufgefuellt. Voreinstellung fuer
pad ist das Leerzeichen.

Beispiele:
        center("abc",7)                 ->      "  abc  "
        center("abc",8,"-")             ->      "--abc---"
        center("Der blaue Himmel",7)    ->      "laue Hi"

GRUPPEN: simul_efun, string
VERWEISE: sprintf, copies, left, right, wrap
*/
#if defined(TestMUD) && __VERSION__ > "3.6.3"
string center(mixed text, int len, string pad = 0)
#else
varargs string center(mixed text, int len, string pad)
#endif
{
    int b;

    if (len<=0)
	return "";
    text = ""+text;
    if ((b=sizeof(text))>=len) {
	b=(b-len)/2;
	return text[b..b+len-1];
	}
    b = (len-b) % 2;
    if (pad)
	return sprintf("%|*'"+pad+"'s",len+b,text)[b..len];
    return sprintf("%|*s",len+b,text)[b..len];
}

#if __EFUN_DEFINED__(left)
#if __VERSION__ < "3.5.0-U041"
private nosave closure left_cl = lambda(({'str, 'len, 'pad}), ({#'efun::left, 'str, 'len, 'pad}));
varargs string left(mixed text, int len, string pad)
{
    if (len<=0)
	return "";
    return funcall(left_cl, text, len, pad || " ");
}
#endif
#else
/*
FUNKTION: left
DEKLARATION: string left(string text, int len [,string pattern] )
BESCHREIBUNG:
Gibt einen String der Breite len zurueck, in dem text linksbuendig
enthalten ist. Wenn noetig wird mit dem Zeichen pad aufgefuellt.
Voreinstellung fuer pad ist das Leerzeichen.

Beispiele:
        left("abc d",8)         -->     "abc d   "
        left("abc d",8,".")     -->     "abc d..."
        left("abc  def",7)      -->     "abc  de"

GRUPPEN: simul_efun, string
VERWEISE: sprintf, copies, center, right, wrap
*/
#if defined(TestMUD) && __VERSION__ > "3.6.3"
string left(mixed text, int len, string pad = 0)
#else
varargs string left(mixed text, int len, string pad)
#endif
{
    if (len<=0)
	return "";
    if (pad)
	return sprintf("%'"+pad+"':-*s",len,""+text);
    return sprintf("%:-*s",len,""+text);
}
#endif

/*
FUNKTION: right
DEKLARATION: string right(string text, int len [,string pattern] )
BESCHREIBUNG:
Gibt einen String der Breite len zurueck, in dem text rechtsbuendig
enthalten ist. Wenn noetig wird mit dem Zeichen pad aufgefuellt.
Voreinstellung fuer pad ist das Leerzeichen.

Beispiele:
        right("abc d",8)        -->     "   abc d"
        right("abc d",8,".")    -->     "...abc d"
        right("abc  def",7)     -->     "bc  def"

GRUPPEN: simul_efun, string
VERWEISE: sprintf, copies, center, right, wrap
*/
#if defined(TestMUD) && __VERSION__ > "3.6.3"
string right(mixed text, int len, string pad = 0)
#else
varargs string right(mixed text, int len, string pad)
#endif
{
    if (len<=0)
	return "";
    text = ""+text; /* um eventuelle integerzahlen in string zu verwandeln */
    if (len<=sizeof(text))
	return text[<len..];
    if (pad)
	return sprintf("%'"+pad+"':*s",len,text);
    return sprintf("%:*s",len,text);
}

/*
FUNKTION: substr
DEKLARATION: string substr(string text, int n[, int len [,string pattern]] )
BESCHREIBUNG:
Gibt den Teil von text zurueck, der mit dem n'ten Zeichen beginnt
und len lang ist. Wenn noetig wird mit dem Zeichen pad aufgefuellt.
Voreinstellung fuer pad ist das Leerzeichen.
Wird len nicht angegeben, so wird der Rest von text zurueckgegeben.

Beispiele:
        substr("abc",2)         -->     "bc"
        substr("abc",2,4)       -->     "bc  "
        substr("abc",2,6,".")   -->     "bc...."

GRUPPEN: simul_efun, string
VERWEISE: sprintf, copies, center, right, wrap
*/
#if defined(TestMUD) && __VERSION__ > "3.6.3"
string substr(string text, int n, int len = 0, string pad = " ")
#else
varargs string substr(string text, int n, int len, string pad)
#endif
{
#if 0
   if (!stringp(text) || !intp(n) || !intp(len) || (pad && !stringp(pad)))
	if (find_player("freaky"))
	    tell_object(find_player("freaky"),sprintf(
	    "SUBSTR: text:%O n:%O len:%O pad:%O PO:%O\n",text,n,len,pad,
	    previous_object()));
#endif

    if (len > 0)
	return sprintf("%'"+(pad||" ")+"'-*.*s",
                  len, len, to_string(text)[(--n<0?0:n)..]);
    if (!len)
        return to_string(text)[(--n<0?0:n)..];
}

/*
FUNKTION: strip
DEKLARATION: string strip(string text)
BESCHREIBUNG:
Entfernt vorangestellte und nachfolgende Spaces und Tabs aus text.

Beispiele:
        strip("   abc   ")              -->     "abc"

GRUPPEN: simul_efun, string
VERWEISE: trim, sprintf, copies, center, left, right, wrap
*/
string strip(string text)
{
    return trim(text || "");
}

/*
FUNKTION: space
DEKLARATION: string space(string str [,int n [,string pad]] )
BESCHREIBUNG:
Loescht alle ueberschuessigen Spaces, und Tabs zwischen den Woertern
und fuellt die Zwischenraeume durch den String <pad> auf

Beispiele:
        space("   hallo  wie gehts   ") -> "hallo wie gehts"
        space("hallo  wie gehts",2)     -> "hallo  wie  gehts"
        space("hallo  wie gehts",1,"")  -> "hallowiegehts"
        space("hallo  wie gehts",3,"+") -> "hallo+++wie+++gehts"

GRUPPEN: simul_efun, string
VERWEISE: trim
*/
#if defined(TestMUD) && __VERSION__ > "3.6.3"
string space(string str, int n = 1, string pad = " ")
#else
varargs string space(string str, int n, string pad)
#endif
{
    if (!str)
	return "";
    if (n < 1)
	n = 1;
    if (!pad)
	return regreplace(trim(str),"[ \t]+"," " * n,1);
    return regreplace(trim(str),"[ \t]+",pad * n,1);
}


/*
FUNKTION: add_dot_to_msg
DEKLARATION: string add_dot_to_msg(string msg)
BESCHREIBUNG:
Haengt an <msg> einen Punkt an, wenn <msg> kein Satzzeichen !?.,;:
am Ende enthaelt. Hat <msg> ein '$' am Ende, so wird dieses abgeschnitten.
GRUPPEN: simul_efun, string
VERWEISE:
*/
mixed add_dot_to_msg(mixed msg)
{
    int c;

    if (stringp(msg) && sizeof(msg) && (c = msg[<1]) && member(".!?,:;", c) < 0)
	return (c=='$')?msg[0..<2]:(msg + ".");
    return msg;
}

/*
FUNKTION: convert_umlaute
DEKLARATION: string convert_umlaute(string str)
BESCHREIBUNG:
Konvertiert alle (einbuchstabigen) Umlaute in die zweibuchstabige
Darstellung ("o -> oe) und loescht alle Zeichen aus dem String, die
nicht darstellbar sind (ASCII < 32 und ASCII > 126 (ausser \t und \n))
GRUPPEN: simul_efun, string
VERWEISE:
*/
string convert_umlaute(string str)
{
    if (!str)
        return 0;

    // Highly optimized for speed.
    return regreplace(str, umlaute_regexp, umlaute_konversion, 1);
}

#if ! __EFUN_DEFINED__(query_verb_ascii)
#if defined(TestMUD) && __VERSION__ > "3.6.3"
string query_verb_ascii(int flag = 0)
#else
varargs string query_verb_ascii(int flag)
#endif
{
    return convert_umlaute(query_verb(flag));
}
#endif

/*
FUNKTION: escape_string
DEKLARATION: varargs string escape_string(string str, int mode)
BESCHREIBUNG:
Die Funktion fuegt Escape-Zeichen (\) nach verschiedenen Regeln in
den Eingabestring 'str' ein und liefert das Ergebnis zurueck. Die
verwendeten Regeln haengen von 'mode' ab.

Mit dieser Funktion lassen sich beliebige Strings in Regular Expressions
einbauen, auch wenn diese Sonderzeichen enthalten. Damit eignet sich diese
Funktion zum Beispiel auch fuer beliebige Spielereingaben.

Die Bestandteile des Bitflags 'mode' fuer Regular Expressions:

   ESCAPE_REGEXP:       (Default)
       Der zurueckgelieferte String ist ein regexp-Muster, das auf
       den Eingabestring 'str' passt.

   ESCAPE_PCRE:
       Anstelle von ESCAPE_REGEXP fuer PCRE-kompatible Patterns.

   ESCAPE_ANY:          (Default)
       Das zurueckgelieferte Muster passt auf beliebige Teilstrings.

   ESCAPE_EXACT:
       Das zurueckgelieferte Muster muss auf den kompletten String passen.

   ESCAPE_WORD:
       Das zurueckgelieferte Muster muss auf ein Wort passen.

   ESCAPE_CASE:         (Default)
       Das zurueckgelieferte Muster beachtet Gross- und Kleinschreibung.

   ESCAPE_NOCASE:
       Das zurueckgelieferte Muster ignoriert Gross- und Kleinschreibung.

   ESCAPE_LISTE:
       Das zurueckgelieferte Muster beachtet durch Kommata getrennte Listen.
       Wenn man sich selbst um die Trennung kuemmern moechte, kann man statt
       einem String auch ein String-Array uebergeben.

   ESCAPE_WILDCARD:
       Das zurueckgelieferte Muster beachtet einfache Wildcards:

           ?   steht fuer ein beliebiges einzelnes Zeichen
           *   steht fuer beliebig viele (oder keine) Zeichen

       Die Wildcards duerfen im Eingabestring selbst escaped werden.

          \?   steht fuer ?
          \*   steht fuer *

Weitere Werte fuer 'mode', die nicht kombinierbar sind:

   ESCAPE_REPLACE:
       Es wird kein Muster, sondern ein Replacepattern zurueckgeliefert.
       Damit setzt regreplace() genau den Eingabestring ein, auch wenn
       dieser Sonderzeichen enthaelt.

   ESCAPE_GETDIR:
       Der zurueckgelieferte String matcht Dateien bei get_dir()
       exakt der Eingabe, Wildcards und Escapes werden ignoriert.

BEISPIELE:
    escape_string("(foo|bar)")
        -> Das Muster passt auf "(foo|bar)", "bla (foo|bar) bla"

    escape_string("(foo|bar)", ESCAPE_PCRE)
        -> Dasselbe, jedoch fuer PCRE.

    escape_string("(foo|bar)", ESCAPE_EXACT)
        -> Das Muster passt ausschliesslich auf "(foo|bar)".

    escape_string("bla*", ESCAPE_WILDCARD)
        -> Das Muster passt auf "bla", "blafasel", "/pfad/zum/blablubb"

    escape_string("bla*", ESCAPE_WILDCARD|ESCAPE_EXACT)
        -> Jetzt passt das Muster nicht mehr auf "/pfad/zum/blablubb"

    escape_string("\\1&\\2&\\3", ESCAPE_REPLACE)
        -> Als replacepattern verwendet, wird \1&\2&\3 eingesetzt.

    escape_string("tisch, stuhl, sofa", ESCAPE_LISTE)
        -> Das Muster passt auf tisch oder stuhl oder sofa,
           aber auch auf holztisch, stuhlbein und ledersofa.

    escape_string("tisch, stuhl, sofa", ESCAPE_LISTE|ESCAPE_CASE|ESCAPE_WORD)
        -> Das Muster passt auf Tisch oder Stuhl oder Sofa,
           jedoch nicht mehr auf Holztisch, Stuhlbein und dergleichen.
           Zudem ist die Gross- und Kleinschreibung in diesem Beispiel egal.

    escape_string("", ESCAPE_LISTE)
    escape_string(" ", ESCAPE_LISTE)
    escape_string(" , ,,,", ESCAPE_LISTE)
        -> Es handelt sich um leere Listen, der Rueckgabewert ist 0.

VERWEISE: regexp, regreplace, get_dir
GRUPPEN: string, simul_efun
*/
#if defined(TestMUD) && __VERSION__ > "3.6.3"
string escape_string(mixed str, int mode = 0)
#else
varargs string escape_string(mixed str, int mode)
#endif
{
#define ESCAPE_LAMBDA(x) lambda( ({'s}), ({#'||, ({#'[, x, 's}), "\\\\&"}) )

    mixed ret;

    // --- Parameter: ---

    if(intp(mode))
    {
        if(mode & ESCAPE_GETDIR)
        {
            if(mode != ESCAPE_GETDIR)
            {
                raise_error("Bad arg 2 to escape_string: ESCAPE_GETDIR not passed exclusively\n");
            }

            else if(!stringp(str))
            {
                raise_error("Bad arg 1 to escape_string: ESCAPE_GETDIR requires string\n");
            }
        }

        else if(mode & ESCAPE_REPLACE)
        {
            if(mode != ESCAPE_REPLACE)
            {
                raise_error("Bad arg 2 to escape_string: ESCAPE_REPLACE not passed exclusively\n");
            }

            else if(!stringp(str))
            {
                raise_error("Bad arg 1 to escape_string: ESCAPE_REPLACE requires string\n");
            }
        }
    }

    else
    {
        raise_error("Bad arg 2 to escape_string: not an int\n");
    }

    if(stringp(str))
    {
        if(mode & ESCAPE_LISTE)
        {
            ret = str = unliste(str, 0);
        }

        else
        {
            ret = ({ str });
        }
    }

    else if(pointerp(str))
    {
        mode |= ESCAPE_LISTE;
        ret = str;
    }

    else
    {
        raise_error("Bad argument 1 to escape_string(): not a string or pointer\n");
    }

    // --- Exklusiv-Flags: ---
    if(mode == ESCAPE_GETDIR)
    {
        escape_getdir ||= ESCAPE_LAMBDA(
            ([ "\\\\": "\\\\\\\\\\\\\\\\", // Escapter Backslash
               "\\":   "\\\\\\\\",         // Normaler Backslash
               "\\?":  "\\\\\\\\\\\\?",    // escaptes ?
               "\\*":  "\\\\\\\\\\\\*",    // escaptes *
            ]) );

        return regreplace(str, "\\\\\\\\|\\\\[?*]|[?*\\\\]",
                          escape_getdir, RE_GLOBAL);
    }

    if(mode == ESCAPE_REPLACE)
    {
        // Keine Unterschiede zwischen traditioneller Regexp und PCRE?
        escape_replace ||= ESCAPE_LAMBDA(
            ([ "\\\\":  "\\\\\\\\\\\\\\\\", // escapter Backslash
               "\\":    "\\\\\\\\",         // normaler Backslash
               "\\&":   "\\\\\\\\\\\\\\&",  // escaptes &
            ]) );

        return regreplace(str, "\\\\\\\\|\\\\[0-9&]|[&\\\\]",
                               escape_replace, RE_GLOBAL);
    }

    // --- PCRE: ---
    if(mode & ESCAPE_PCRE)
    {
        if(mode & ESCAPE_WILDCARD)
        {
            escape_pcre_wildcard ||= ESCAPE_LAMBDA(
                ([ "\\\\": "\\\\",                // escapter Backslash
                   "\\E":  "\\\\E\\\\\\\\E\\\\Q", // PCRE quote
                   "\\*":  "*",                   // escaptes *,
                   "*":    "\\\\E.*\\\\Q",        // Wildcard *
                   "\\?":  "?",                   // escaptes ?
                   "?":    "\\\\E.\\\\Q"          // Wildcard ?
                ]) );

            ret = map(ret, #'regreplace, "\\\\\\\\|\\\\E|\\\\[*?]|[\\\\?*]",
                                         escape_pcre_wildcard, RE_GLOBAL);
        }

        else
        {
            ret = map(ret, #'regreplace, "\\\\E",
                                         "\\\\E\\\\\\\\E\\\\Q", RE_GLOBAL);
        }

        return sprintf("%s(%s\\Q%s\\E)%s",
                      mode & ESCAPE_EXACT ? "^"
                                          : mode & ESCAPE_WORD ? "\\b"
                                                               : "",
                      mode & ESCAPE_NOCASE ? "(?i)" : "",
                      implode(ret, "\\E|\\Q"),
                      mode & ESCAPE_EXACT ? "$"
                                          : mode & ESCAPE_WORD ? "\\b"
                                                               : "");
    }

    // --- REGEXP: ---
    else // mode & ESCAPE_REGEXP
    {
        if(mode & ESCAPE_WILDCARD)
        {
            escape_regexp_wildcard ||= ESCAPE_LAMBDA(
                ([ "\\\\":  "\\\\\\\\", // escapter Backslash
                   "\\*":   "\\\\*",    // escaptes *
                   "*":     ".*",       // Wildcard *
                   "\\?":   "?",        // escaptes ?
                   "?":     "."         // Wildcard ?
                ]) );

            ret = map(ret, #'regreplace, "\\\\\\\\|\\\\[<>*?]|[\\\\?*.^$|()+[\\]]",
                                         escape_regexp_wildcard, RE_GLOBAL);
        }

        else
        {
            // regexp-Zeichen escapen.
            ret = map(ret, #'regreplace, "\\\\[<>]|[\\\\*.^$|()+[\\]]",
                                         "\\\\&", RE_GLOBAL);
        }

        if(mode & ESCAPE_NOCASE)
        {
            escape_regexp_nocase ||= function string (string s)
            {
                // Diese Zeile sieht so kryptisch aus weil sie von Gnomi ist
                return ("["+implode(map(transpose_array(({explode(upper_case(s),""),explode(lower_case(s),"")})),#'implode,""),"][")+"]");
            };

            ret = map(ret, #'regreplace, "[A-Za-z]+",
                                         escape_regexp_nocase, RE_GLOBAL);
        }

        if (!(mode & (ESCAPE_EXACT|ESCAPE_WORD)) && sizeof(ret) == 1)
            return ret[0];

        return sprintf("%s(%s)%s",
                       mode & ESCAPE_EXACT ? "^"
                                           : mode & ESCAPE_WORD ? "\\<"
                                                                : "",
                       implode(ret, "|"),
                       mode & ESCAPE_EXACT ? "$"
                                           : mode & ESCAPE_WORD ? "\\>"
                                                                : "");
    }

#undef ESCAPE_LAMBDA
}

#if __EFUN_DEFINED__(json_serialize) && __VERSION__ < "3.5.0-U041"
private nosave closure json_serialize_cl = lambda(({'val}), ({#'efun::json_serialize, 'val}));
varargs string json_serialize(mixed val)
{
    return funcall(json_serialize_cl, val);
}
#endif

#if __EFUN_DEFINED__(json_parse) && __VERSION__ < "3.5.0-U041"
private nosave closure json_parse_cl = lambda(({'text}), ({#'efun::json_parse, 'text}));
varargs mixed json_parse(string text)
{
    return funcall(json_parse_cl, text);
}
#endif
