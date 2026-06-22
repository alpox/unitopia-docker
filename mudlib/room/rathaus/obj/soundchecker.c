// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/room/rathaus/obj/soundchecker.c
// Description: Tool fuer Admins / Rathaus zur Verwaltung der Sounds/Wuensche

inherit "/i/item";
inherit "/i/move";
inherit "/i/tools/security";
inherit "/i/tools/dynamic_browser";

#include <database.h>
#include <dynamic_browser.h>
#include <editor.h>
#include <files.h>
#include <level.h>
#include <message.h>
#include <misc.h>
#include <properties.h>
#include <security.h>

#include <soundcheck.h>

#define HINT_TMP_FILE ("/w/"+geteuid(TP)\
                      +((file_size("/w/"+geteuid(TP)+"/priv")==FSIZE_DIR)?"/priv":"")\
                      +"/SOUNDCHECK_HINT")
#define LINE      "-------------------------------------------------------------------------------"
#define MORE_LINE "...--------------------------------------------------------------------- (MORE)"
#define ADD_HEADER_LINES(x) \
    if (!TP->query_no_ascii_art() && !member(old, B_HEADER_LINES)) \
            old[B_HEADER_LINES] = ({ (x), LINE, MORE_LINE, MORE_LINE });

private string last_sound_file = 0;

private int get_level(object tp)
{
    if (adminp(tp))     return SC_LVL_ADMIN;
    if (wizp(tp))       return SC_LVL_WIZP;
    if (hlpp(tp))       return SC_LVL_HLP;
    if (guestp(tp))     return SC_LVL_GUEST;
    if (newbiep(tp))    return SC_LVL_NEWBIE;
    if (playerp(tp))    return SC_LVL_PLAYER;
    return 0;
}

// ------------------------------------------------------------- main menu

private string* get_sc_main_menu()
{
    string * lines = ({});
    int lvl = get_level(TP);
    if (lvl>=SC_LVL_WIZP)
        lines += ({" -T- Alle Tondateien."});
    else if (lvl == SC_LVL_HLP)
        lines += ({" -T- Alle freigegebenen Tondateien."});
    if (lvl>=SC_LVL_WIZP)
        lines += ({" -W- Alle Wünsche."});
    else if (lvl>=SC_LVL_PLAYER)
        lines += ({" -W- Alle offenen Wünsche."});
    if (lvl>=SC_LVL_WIZP)
        lines += ({" -V- Alle Vorschläge."});
    else if (lvl>=SC_LVL_PLAYER)
        lines += ({" -V- Alle offenen Vorschläge."});
    lines += ({" -H- Hinweise zu den Tondateien."});
    return lines;
}

static mixed sc_main_menu_init(mapping old)
{
    old[B_START_LINE] = 0;
    ADD_HEADER_LINES(
"----------- Hauptmenü, Übersichtshilfe mit ? ---------------------------------"
    );
    return old;
}

static int sc_main_menu_total(mapping menue)
{
    return sizeof(get_sc_main_menu());
}

static string * sc_main_menu_display(mapping menue)
{
    menue[B_DATA] = get_sc_main_menu();
    return staticmore_display(menue);
}

static string sc_main_menu_prompt(mapping menue)
{
    return "Tonprüfer Hauptmenü [?,q]";
}

static mixed sc_main_menu_action(string str, mapping * menues)
{
    str = lower_case(space(str));
    switch (str)
    {
        case "?":
        case "h": // Hinweise
            return menues+({([
                B_TYPE: "sc_hints",
            ])});
        case "t": // Tondateien
            return menues+({([
                B_TYPE: "sc_sounds",
            ])});
        case "w": // Wuensche
            return menues+({([
                B_TYPE: "sc_wishlist",
            ])});
        case "v": // Vorschlaege
            return menues+({([
                B_TYPE: "sc_all_proposals",
            ])});
    }
    
    return B_NOTHING;
}

// ------------------------------------------------------------- sc_hints

int ed_hint_level;
private void mini_ed_update_hint_end(string* str)
{
    if(str)
    {
       string txt = implode(str,"\n");
       SOUNDCHECK->update_hint(ed_hint_level,txt);
       browse_write_line("Hinweistext gesetzt.");
    }
    else
    {
        browse_write_line("Eingabe des Hinweistextes abgebrochen.");
    }
}

static mixed sc_hints_init(mapping old)
{
    old[B_START_LINE] = 0;
    ADD_HEADER_LINES(
"----------- Hinweismenü -------------------------------------------------------"
    );
    return old;
}

static int sc_hints_total(mapping menue)
{
    return sizeof(SOUNDCHECK->get_hint(get_level(TP)));
}

static string * sc_hints_display(mapping menue)
{
    menue[B_DATA] = SOUNDCHECK->get_hint(get_level(TP));
    return staticmore_display(menue);
}

static string sc_hints_prompt(mapping menue)
{
    int lvl = get_level(TP);
    if (lvl<5) // newbies und gaeste
        return sprintf("Hinweise %d/%d [q]",
                menue[B_CURRENT_LINE],menue[B_END_LINE]);
    else if (lvl > 25) // admins
        return sprintf("Hinweise %d/%d [n,l,z,q]",
                menue[B_CURRENT_LINE],menue[B_END_LINE]);
    else
        return sprintf("Hinweise %d/%d [z,q]",
                menue[B_CURRENT_LINE],menue[B_END_LINE]);
}

static mixed sc_hints_action(string str, mapping * menues)
{
    int lvl = get_level(TP);
    string *txt;
    string * split = explode(space(str)," ");
    switch(lower_case(split[0]))
    {
        case "n":
            if (lvl <= SC_LVL_WIZP)
            {
                browse_write_line("Keine Berechtigung zum Neuanlegen.");
                return B_DONE;
            }
            if (sizeof(split)<2)
            {
                browse_write_line("n <lvl>");
                return B_DONE;
            }
            ed_hint_level = to_int(split[1]);
            txt = SOUNDCHECK->get_one_hint(ed_hint_level);
            TP->mini_ed(#'mini_ed_update_hint_end, 0,//')(
                TP->uses_gmcp_edit() ? 0 : HINT_TMP_FILE, 
                ([
                    MINI_ED_START_TEXT:
                        "Gib nun den Hinweis ein.\n"
                        "Mit '**' oder '.' beenden, mit '~q' abbrechen.\n",
                    MINI_ED_TITLE:
                        "SoundChecker-Hinweis",
                ]),
                ([
                    MINI_ED_FORCE_WRAP: 1
                ]), txt);
            return B_QUIT;
        case "l":
            if (lvl <= SC_LVL_WIZP)
            {
                browse_write_line("Keine Berechtigung zum Löschen.");
                return B_DONE;
            }
            if (sizeof(split)<2)
            {
                browse_write_line("l <lvl>");
                return B_DONE;
            }
            ed_hint_level = to_int(split[1]);
            SOUNDCHECK->update_hint(ed_hint_level,0);
            browse_write_line("Hinweistext gelöscht.");
            return B_REBUILT;
        default: break;
    }
    return B_NOTHING;
}

// ------------------------------------------------------------- sc_sounds
#define HELP_SC_SOUNDS ({ \
"Die Tondateien sind in Bereiche unterteilt: 'Basis' und 'Erweitert' sind",\
"fuer zentrale Objekte aller Art vorgesehen. Der Bereich Vorschlag enthaelt",\
"Unterverzeichnisse der Spender mit ihren Tondateien. Mit Angabe der Nummer",\
"kann man die Detailinformationen der Tondatei ansehen oder selektieren.",\
})

static mixed sc_sounds_init(mapping old)
{
    old[B_START_LINE] = 0;
    old[B_HELP] = ([ B_TYPE : B_STATICMORE,
                     B_PROMPT : "Hilfe zur Liste der Tondateien Zeile",
                     B_DATA : HELP_SC_SOUNDS ]);
    ADD_HEADER_LINES(
"----------- Liste der Tondateien ----------------------------------------------"
    );
    return old;
}

static int sc_sounds_total(mapping menue)
{
    return SOUNDCHECK->get_all_soundfiles(menue,1);
}

static string * sc_sounds_display(mapping menue)
{
    mapping * sounds = SOUNDCHECK->get_all_soundfiles(menue+([
        DB_DBG_LIMIT:menue[B_NUM_LINES],
        DB_DBG_OFFSET:menue[B_CURRENT_LINE],
        ]),0);
    if (!pointerp(sounds) || !sizeof(sounds))
    {
        return ({ "Keine Anzeige." });
    }
    string * lines = ({});
    mapping cache = ([]);
    int ix;
    for (ix=0;ix<sizeof(sounds);ix++)
    {
        int jx = menue[B_CURRENT_LINE] + ix + 1;
        cache[jx] = sounds[ix][SOUND_OPT_FILE];
        lines += ({ sprintf("%03d %s",jx,cache[jx]) });
    }
    menue[DB_DBG_CACHE] = cache;
    return lines;
}

static string sc_sounds_prompt(mapping menue)
{
    if (member(menue,SOUND_OPT_SELECTION))
        return sprintf("Selektion einer Tondatei %d/%d [<nr>,z,q]",
            menue[B_CURRENT_LINE],menue[B_END_LINE]);
    else
        return sprintf("Tondateien %d/%d [<nr>,z,q]",
            menue[B_CURRENT_LINE],menue[B_END_LINE]);
}

static mixed sc_sounds_action(string str, mapping * menues)
{
    mapping menue = menues[<1];
    mapping cache = menue[DB_DBG_CACHE];
    int jx = to_int(space(str));
    if (jx)
    {
        if (member(cache,jx))
        {
            switch (menue[SOUND_OPT_SELECTION])
            {
                case SOUND_OPT_WL_ID:
                    mapping wish = SOUNDCHECK->get_one_wish(
                        menue[SOUND_OPT_WL_ID]);
                    wish[SOUND_OPT_FILE] = cache[jx];
                    if (!SOUNDCHECK->set_one_wish(wish))
                    {
                        browse_write_line("Fehler beim Setzen der Tondatei.");
                        return B_DONE;
                    }
                    menues[<2][B_FLAGS] |= BF_DIRTY;
                    return menues[..<2]; // direkter Ruecksprung.
                case SOUND_OPT_PR_ID:
                    mapping proposal = SOUNDCHECK->get_one_proposal(
                        menue[SOUND_OPT_PR_ID]);
                    proposal[SOUND_OPT_FILE] = cache[jx];
                    if (!SOUNDCHECK->set_one_proposal(proposal))
                    {
                        browse_write_line("Fehler beim Setzen der Tondatei.");
                        return B_DONE;
                    }
                    menues[<2][B_FLAGS] |= BF_DIRTY;
                    return menues[..<2]; // direkter Ruecksprung.
                default:
                    return menues+({([
                        B_TYPE: "sc_one_sound",
                        SOUND_OPT_FILE: cache[jx],
                    ])});
            }
        }
        else
        {
            browse_write_line("Zahl außerhalb der aktuellen Anzeige.");
            return B_DONE;
        }
    }
    return B_NOTHING;
}

// ------------------------------------------------------------- sc_one_sound
#define HELP_SC_ONE_SOUND ({ \
"Zu jeder Tondatei wird gepflegt, wer die Quelle ist und wie das Copyright",\
"aussieht. Mit +/- kann man vor und zurueck navigieren und mit 'a' kann man",\
"die Tondatei abspielen, sofern sie existiert.",\
})

#define HELP_SC_ONE_SOUND_ADMIN HELP_SC_ONE_SOUND + ({ \
"Fuer Admins gibt es Eingriffe in den Status (siehe dortige Hilfe im",\
"Untermenu), Anpassung von Quelle'l' und Copyright'c'.",\
})

private string* get_sc_status(int st)
{
    string* stats = ({});
    if (st & SC_STATUS_DELETED)
    {
        stats += ({"Gelöscht"});
    }
    if (st & SC_STATUS_ACTIVE)
    {
        stats += ({"Aktiv"});
    }
    if (st & SC_SOUNDFILE_TARGET)
    {
        stats += ({"Zieldatei vorhanden"});
    }
    if (st & SC_SOUNDFILE_PLAY)
    {
        stats += ({"Abgespielt"});
    }
    if (st & SC_SOUNDFILE_PROP)
    {
        stats += ({"Eigenschaft vorhanden"});
    }
    if (st & SC_WISHLIST_OPEN)
    {
        stats += ({"Offen"});
    }
    if (st & SC_WISHLIST_FULFILLED)
    {
        stats += ({"Erfüllt"});
    }
    return stats;
}

private string* get_sc_one_sound(string file)
{
    mapping sound = SOUNDCHECK->get_one_soundfile(file);
    string* lines = ({});
    if (!mappingp(sound))
        return ({"Unbekannte Tondatei."});
    lines += ({ "Tondatei: "+sound[SOUND_OPT_FILE]});
    if (get_level(TP)==SC_LVL_ADMIN) // edit
    {
        lines += explode(wrap_say("S - Status: ",
            implode(get_sc_status(sound[SOUND_OPT_STATUS]),", ")),"\n")[..<2];
        lines += ({"L - Quelle: "});
        lines += explode(wrap(sound[SOUND_OPT_SOURCE]||""),"\n")[..<2];
        lines += ({"C - Copyright: "});
        lines += explode(wrap(sound[SOUND_OPT_COPYRIGHT]||""),"\n")[..<2];
    }
    else // view only
    {
        lines += ({"Quelle: "});
        lines += explode(wrap(sound[SOUND_OPT_SOURCE]||""),"\n")[..<2];
        lines += ({"Copyright: "});
        lines += explode(wrap(sound[SOUND_OPT_COPYRIGHT]||""),"\n")[..<2];
    }
    lines += ({ "A - Abspielen"});
    return lines;
}

static mixed sc_one_sound_init(mapping old)
{
    int lvl = get_level(TP);
    old[B_START_LINE] = 0;
    old[B_HELP] = ([ B_TYPE : B_STATICMORE,
                     B_PROMPT : "Hilfe zur Liste der Tondateien Zeile",
                     B_DATA : (lvl==SC_LVL_ADMIN)?
                                HELP_SC_ONE_SOUND_ADMIN:
                                HELP_SC_ONE_SOUND ]);
    ADD_HEADER_LINES(
"----------- Anzeige einer Tondatei --------------------------------------------"
    );
    return old;
}

static int sc_one_sound_total(mapping menue)
{
    return sizeof(get_sc_one_sound(menue[SOUND_OPT_FILE]));
}

static string * sc_one_sound_display(mapping menue)
{
    menue[B_DATA] = get_sc_one_sound(menue[SOUND_OPT_FILE]);
    return staticmore_display(menue);
}

static string sc_one_sound_prompt(mapping menue)
{
    if (get_level(TP)==SC_LVL_ADMIN)
        return sprintf("Eine Tondatei (%d/%d) [+,-,s,l,c,a,z,q]",
            menue[B_CURRENT_LINE],menue[B_END_LINE]);
    else
        return sprintf("Eine Tondatei (%d/%d) [+,-,a,z,q]",
            menue[B_CURRENT_LINE],menue[B_END_LINE]);
}

static mixed sc_one_sound_action(string str, mapping * menues)
{
    mapping menue = menues[<1];
    mapping sound = SOUNDCHECK->get_one_soundfile(menue[SOUND_OPT_FILE]);
    string *split = explode(space(str)," ");
    string param = implode(split[1..]," ");
    int lvl = get_level(TP);
    switch (lower_case(split[0]))
    {
        case "+":
            sound = SOUNDCHECK->get_next_soundfile(menue[SOUND_OPT_FILE],"+");
            if (!mappingp(sound))
            {
                browse_write_line("Ende der Liste erreicht.");
                return B_DONE;
            }
            return menues[..<2] + ({([
                B_TYPE:"sc_one_sound",
                SOUND_OPT_FILE:sound[SOUND_OPT_FILE],
                ])});
        case "-":
            sound = SOUNDCHECK->get_next_soundfile(menue[SOUND_OPT_FILE],"-");
            if (!mappingp(sound))
            {
                browse_write_line("Anfang der Liste erreicht.");
                return B_DONE;
            }
            return menues[..<2] + ({([
                B_TYPE:"sc_one_sound",
                SOUND_OPT_FILE:sound[SOUND_OPT_FILE],
                ])});
        case "s": // Status
            if (lvl < SC_LVL_ADMIN)
            {
                browse_write_line("Kein Zugriff.");
                return B_DONE;
            }
            return menues + ({([
                B_TYPE: "sc_all_status",
                SOUND_OPT_FILE: menue[SOUND_OPT_FILE],
                SOUND_OPT_SUM: SC_STATUS_SUM_BASIS|SC_SOUNDFILE_SUM,
                SOUND_OPT_STATUS: sound[SOUND_OPT_STATUS],
            ])});
        case "l": // Quelle
            if (lvl < SC_LVL_ADMIN)
            {
                browse_write_line("Kein Zugriff.");
                return B_DONE;
            }
            if (sizeof(split)<2)
            {
                // TODO editor?
                browse_write_line("l <quelle>");
                return B_DONE;
            }
            sound[SOUND_OPT_SOURCE] = param;
            if (!SOUNDCHECK->set_one_soundfile(sound))
            {
                browse_write_line("Fehler beim Speichern der Quelle.");
                return B_DONE;
            }
            browse_write_line("Quelle gesetzt.");
            return B_REBUILT;
        case "c": // Copyright
            if (lvl < SC_LVL_ADMIN)
            {
                browse_write_line("Kein Zugriff.");
                return B_DONE;
            }
            if (sizeof(split)<2)
            {
                // TODO editor?
                browse_write_line("c <cop<right>");
                return B_DONE;
            }
            sound[SOUND_OPT_COPYRIGHT] = param;
            if (!SOUNDCHECK->set_one_soundfile(sound))
            {
                browse_write_line("Fehler beim Speichern des Copyright.");
                return B_DONE;
            }
            browse_write_line("Copyright gesetzt.");
            return B_REBUILT;
        case "a": // Abspielen
            last_sound_file = menue[SOUND_OPT_FILE];
            TP->receive_message(MT_NOTIFY,MA_UNKNOWN,TP,
                "Abspielen der Tondatei.\n", 
                (["message:sound":menue[SOUND_OPT_FILE] ]));
            return B_DONE;
    }
    return B_NOTHING;
}

// ----------------------------------------------------------- sc_all_status
#define HELP_SC_ALL_STATUS ({ \
"Eingriffe in den Status haben weitreichende Konsequenzen:",\
"- 'Geloescht' wird  bei naechster Neuanlage ueberschrieben...",\
"- 'Aktiv' ist zur freien Verwendung und Anzeige bzw. Abspielen freigegeben.",\
"- 'Zieldatei vorhanden' wird beim Einlesen gesetzt.",\
"- 'Abgespielt' Versuch ueber z.B.receive_message_low abzuspielen.",\
"- 'Eigenschaft gesetzt' wurde z.B. der Property P_SOUND uebergeben.",\
"- 'Wunsch offen/erfuellt' gibt Filtermoeglichkeiten fuer Wuensche.",\
"- 'Vorschlag offen/erfuellt' gibt Filtermoeglichkeiten fuer Vorschlaege.",\
})
private string* get_sc_status_lines(int st,int sum)
{
    string* stats = ({});
    if ( (sum & SC_STATUS_SUM_BASIS)>0)
    {
        if (st & SC_STATUS_DELETED)
        {
            stats += ({"01 [X] Gelöscht"});
        }
        else
        {
            stats += ({"01 [.] Gelöscht"});
        }
        if (st & SC_STATUS_ACTIVE)
        {
            stats += ({"02 [X] Aktiv"});
        }
        else
        {
            stats += ({"02 [.] Aktiv"});
        }
    }
    if ( (sum & SC_SOUNDFILE_SUM)>0)
    {
        if (st & SC_SOUNDFILE_TARGET)
        {
            stats += ({"03 [X] Zieldatei vorhanden"});
        }
        else
        {
            stats += ({"03 [.] Zieldatei vorhanden"});
        }
        if (st & SC_SOUNDFILE_PLAY)
        {
            stats += ({"04 [X] Abgespielt"});
        }
        else
        {
            stats += ({"04 [.] Abgespielt"});
        }
        if (st & SC_SOUNDFILE_PROP)
        {
            stats += ({"05 [X] Eigenschaft vorhanden"});
        }
        else
        {
            stats += ({"05 [.] Eigenschaft vorhanden"});
        }
    }
    if ( (sum & SC_WISHLIST_SUM)>0)
    {
        if (st & SC_WISHLIST_OPEN)
        {
            stats += ({"06 [X] Wunsch Offen"});
        }
        else
        {
            stats += ({"06 [.] Wunsch Offen"});
        }
        if (st & SC_WISHLIST_FULFILLED)
        {
            stats += ({"07 [X] Wunsch Erfüllt"});
        }
        else
        {
            stats += ({"07 [.] Wunsch Erfüllt"});
        }
    }
    if ( (sum & SC_PROPOSAL_SUM)>0)
    {
        if (st & SC_PROPOSAL_OPEN)
        {
            stats += ({"08 [X] Vorschlag Offen"});
        }
        else
        {
            stats += ({"08 [.] Vorschlag Offen"});
        }
        if (st & SC_PROPOSAL_FULFILLED)
        {
            stats += ({"09 [X] Vorschlag Erfüllt"});
        }
        else
        {
            stats += ({"09 [.] Vorschlag Erfüllt"});
        }
    }
    return stats;
}

private int sc_status_to_bit(int nr)
{
    switch(nr)
    {
        case 1: return SC_STATUS_DELETED;
        case 2: return SC_STATUS_ACTIVE;
        case 3: return SC_SOUNDFILE_TARGET;
        case 4: return SC_SOUNDFILE_PLAY;
        case 5: return SC_SOUNDFILE_PROP;
        case 6: return SC_WISHLIST_OPEN;
        case 7: return SC_WISHLIST_FULFILLED;
        case 8: return SC_PROPOSAL_OPEN;
        case 9: return SC_PROPOSAL_FULFILLED;
        default: return 0;
    }
}

static mixed sc_all_status_init(mapping old)
{
    old[B_START_LINE] = 0;
    old[B_HELP] = ([ B_TYPE : B_STATICMORE,
                     B_PROMPT : "Hilfe zur Auswahl der Statusbits Zeile",
                     B_DATA : HELP_SC_ALL_STATUS ]);
    ADD_HEADER_LINES(
"----------- Auswahl der Statusbits --------------------------------------------"
    );
    return old;
}

static int sc_all_status_total(mapping menue)
{
    return sizeof(get_sc_status_lines(
        menue[SOUND_OPT_STATUS],menue[SOUND_OPT_SUM]));
}

static string * sc_all_status_display(mapping menue)
{
    menue[B_DATA] = get_sc_status_lines(
        menue[SOUND_OPT_STATUS],menue[SOUND_OPT_SUM]);
    return staticmore_display(menue);
}

static string sc_all_status_prompt(mapping menue)
{
    int sum = menue[SOUND_OPT_SUM];
    string sumtxt = "";
    if (sum == (SC_STATUS_SUM_BASIS|SC_SOUNDFILE_SUM))
        sumtxt = "von Tondatei "+menue[SOUND_OPT_FILE]+" ";
    else if (sum == (SC_STATUS_SUM_BASIS|SC_WISHLIST_SUM))
        sumtxt = "von Wunschnr. "+menue[SOUND_OPT_WL_ID]+" ";
    return sprintf("Status %s(%d/%d) [<nr>,z,q]",
        sumtxt,menue[B_CURRENT_LINE],menue[B_END_LINE]);
}

static mixed sc_all_status_action(string str, mapping * menues)
{
    mapping menue = menues[<1];
    int nr = to_int(str) ;
    int newstat = sc_status_to_bit(nr);
    int oldstat = menue[SOUND_OPT_STATUS];
    int sum = menue[SOUND_OPT_SUM];
    if (nr)
    {
        if (!newstat)
        {
            browse_write_line("Ungültige Nummer.");
            return B_DONE;
        }
        if ((sum & newstat)==0)
        {
            browse_write_line("Ungültige Nummer!");
            return B_DONE;
        }
        if (get_level(TP)<SC_LVL_ADMIN)
        {
            browse_write_line("Kein Änderungsrecht!");
            return B_DONE;
        }
        oldstat ^= newstat;
        if (sum == (SC_STATUS_SUM_BASIS|SC_SOUNDFILE_SUM))
        {
            mapping sound = SOUNDCHECK->get_one_soundfile(
                menue[SOUND_OPT_FILE]);
            sound[SOUND_OPT_STATUS] = oldstat;
            menue[SOUND_OPT_STATUS] = oldstat;
            if (SOUNDCHECK->set_one_soundfile(sound))
            {
                browse_write_line("Status der Tondatei gesetzt.");
                return B_REBUILT;
            }
        }
        else if (sum == (SC_STATUS_SUM_BASIS|SC_WISHLIST_SUM))
        {
            mapping wish = SOUNDCHECK->get_one_wish(menue[SOUND_OPT_WL_ID]);
            wish[SOUND_OPT_STATUS] = oldstat;
            menue[SOUND_OPT_STATUS] = oldstat;
            if (SOUNDCHECK->set_one_wish(wish))
            {
                browse_write_line("Status des Wunsches gesetzt.");
                return B_REBUILT;
            }
        }
        browse_write_line("Fehler beim Status-setzen.");
        return B_DONE;
    }
    return B_NOTHING;
}

// ----------------------------------------------------------- sc_wishlist
#define HELP_SC_WISHLIST ({ \
"Die Wunschliste ist primaer fuer das Pantheon gedacht, um gesuchte Tondateien",\
"bekannt zu machen. Aber auch Spieler und Engel koennen Wuensche hier anlegen.",\
"Die Prioritaet, nach der sortiert wird, wird von den Admins vergeben.",\
"Unter Angabe der Nummer kann man Detailinformationen zum Wunsch anschauen",\
"oder selektieren.",\
})

static mixed sc_wishlist_init(mapping old)
{
    old[B_START_LINE] = 0;
    old[B_HELP] = ([ B_TYPE : B_STATICMORE,
                     B_PROMPT : "Hilfe zur Wunschliste Zeile",
                     B_DATA : HELP_SC_WISHLIST ]);
    ADD_HEADER_LINES(
"----------- Wunschliste -------------------------------------------------------"
    );
    return old;
}
 
static int sc_wishlist_total(mapping menue)
{
    return SOUNDCHECK->get_wishlist(menue,1);
}
 
static string * sc_wishlist_display(mapping menue)
{
    mapping * wishes = SOUNDCHECK->get_wishlist(menue
        +([DB_DBG_LIMIT:menue[B_NUM_LINES],
           DB_DBG_OFFSET:menue[B_CURRENT_LINE]]),0);
    if (!pointerp(wishes) || !sizeof(wishes))
        return ({"Keine Anzeige."});
    int ix;
    string *lines=({});
    for (ix=0;ix<sizeof(wishes);ix++)
    {
        lines += ({left(sprintf("%03d %s",
            wishes[ix][SOUND_OPT_WL_ID],wishes[ix][SOUND_OPT_WISH]),79) });
    }
    return lines;
}
 
static string sc_wishlist_prompt(mapping menue)
{
    if (member(menue,SOUND_OPT_SELECTION))
        return sprintf("Selektion eines Wunsches %d/%d [<nr>,z,q]",
            menue[B_CURRENT_LINE],menue[B_END_LINE]);
    else
        return sprintf("Wunschliste (%d/%d) [<nr>,n,z,q]",
            menue[B_CURRENT_LINE],menue[B_END_LINE]);
}
 
static mixed sc_wishlist_action(string str, mapping * menues)
{
    mapping menue = menues[<1];
    mapping wish;
    int wl_id = to_int(space(str));
    string *split=explode(space(str)," "),param = implode(split[1..]," ");
    if (wl_id)
    {
        wish = SOUNDCHECK->get_one_wish(wl_id);
        if (mappingp(wish))
        {
            switch (menue[SOUND_OPT_SELECTION])
            {
                case SOUND_OPT_PR_ID:
                    mapping proposal = SOUNDCHECK->get_one_proposal(
                        menue[SOUND_OPT_PR_ID]);
                    proposal[SOUND_OPT_WL_ID] = wl_id;
                    if (!SOUNDCHECK->set_one_proposal(proposal))
                    {
                        browse_write_line("Fehler beim Setzen der Wunsch-Id");
                        return B_DONE;
                    }
                    menues[<2][B_FLAGS] |= BF_DIRTY;
                    return menues[..<2]; // direkter Ruecksprung.
                default:
                    return menues + ({([
                        B_TYPE:"sc_one_wish",
                        SOUND_OPT_WL_ID:wl_id,
                        ])});
            }
        }
        browse_write_line("Ungültige Wunschnummer.");
        return B_DONE;
    }
    switch (lower_case(space(split[0])))
    {
        case "n": // neu
            if (get_level(TP)<SC_LVL_WIZP)
            {
                browse_write_line("Kein Zugriff.");
                return B_DONE;
            }
            if (param == "")
            {
                browse_write_line("n <wunsch>");
                return B_DONE;
            }
            wl_id = SOUNDCHECK->set_one_wish( ([
                SOUND_OPT_STATUS: SC_STATUS_ACTIVE|SC_WISHLIST_OPEN,
                SOUND_OPT_PRIORITY: 1,
                SOUND_OPT_FROM: TP_RCN,
                SOUND_OPT_WISH: param, 
                ]) );
            if (wl_id)
            {
                return menues+({([
                    B_TYPE:"sc_one_wish",
                    SOUND_OPT_WL_ID:wl_id, 
                ])});
            }
            browse_write_line("Fehler beim Anlegen eines Wunsches.");
            return B_DONE;
    }
    return B_NOTHING;
}

// ----------------------------------------------------------- sc_one_wish
#define HELP_SC_ONE_WISH ({ \
"Es werden Status, Prioritaet, von wem und Inhalt des Wunsches angezeigt.",\
"Falls eine Tondatei (z.B. ein Vorschlag) verknuepft wurde, wird diese ",\
"angezeigt und kann abgespielt werden.",\
})
#define HELP_SC_ONE_WISH_ADMIN HELP_SC_ONE_WISH+({ \
"Diese koennen von Admins auch angepasst werden. Wenn eine Tondatei angegeben",\
"wird, so gilt der Wunsch als erfuellt (wenn es kein Vorschlag/ ist).",\
})
private string* get_one_wish(int wl_id)
{
    string *lines=({});
    mapping wish = SOUNDCHECK->get_one_wish(wl_id);
    int lvl = get_level(TP);
    if (!mappingp(wish))
        return ({"Keine Anzeige."});
    lines += ({"Wunsch-Id: "+wish[SOUND_OPT_WL_ID] });
    if (lvl == SC_LVL_ADMIN)
    {
        lines += explode(wrap_say("S - Status: ",
            implode(get_sc_status(wish[SOUND_OPT_STATUS]),", ")),"\n")[..<2];
        lines += ({"P - Priorität: "+wish[SOUND_OPT_PRIORITY] });
        lines += ({"V - von: "+wish[SOUND_OPT_FROM]});
        lines += explode(wrap_say("W - Wunsch: ",
            wish[SOUND_OPT_WISH]),"\n")[..<2];
        if (wish[SOUND_OPT_FILE]==0)
            lines += ({"T - Tondatei: <null>"});
        else
        {
            lines += ({"T - Tondatei: "+wish[SOUND_OPT_FILE]});
            lines += ({"A - Abspielen der Tondatei." });
        }
    }
    else
    {
        lines += explode(wrap_say("Status: ",
            implode(get_sc_status(wish[SOUND_OPT_STATUS]),", ")),"\n")[..<2];
        lines += ({"Priorität: "+wish[SOUND_OPT_PRIORITY] });
        lines += ({"von: "+wish[SOUND_OPT_FROM]});
        lines += explode(wrap_say("W - Wunsch: ",
            wish[SOUND_OPT_WISH]),"\n")[..<2];
        if (wish[SOUND_OPT_FILE]==0)
            lines += ({"Tondatei: keine"});
        else
        {
            lines += ({"Tondatei: "+wish[SOUND_OPT_FILE]});
            lines += ({"A - Abspielen der Tondatei." });
        }
    }
    return lines;
}

static mixed sc_one_wish_init(mapping old)
{
    int lvl = get_level(TP);
    old[B_START_LINE] = 0;
    old[B_HELP] = ([ B_TYPE : B_STATICMORE,
                     B_PROMPT : "Hilfe zur Wunschanzeige Zeile",
                     B_DATA : (lvl==SC_LVL_ADMIN)?
                                HELP_SC_ONE_WISH_ADMIN:
                                HELP_SC_ONE_WISH ]);
    ADD_HEADER_LINES(
"----------- Anzeige eines Wunsches --------------------------------------------"
    );
    return old;
}
 
static int sc_one_wish_total(mapping menue)
{
    return sizeof(get_one_wish(menue[SOUND_OPT_WL_ID]));
}
 
static string * sc_one_wish_display(mapping menue)
{
    menue[B_DATA] = get_one_wish(menue[SOUND_OPT_WL_ID]);
    return staticmore_display(menue);
}
 
static string sc_one_wish_prompt(mapping menue)
{
    if (get_level(TP)==SC_LVL_ADMIN)
        return sprintf("Wunschanzeige Nr.%d (%d/%d) [s,p,v,w,t,z,q]",
            menue[SOUND_OPT_WL_ID],menue[B_CURRENT_LINE],menue[B_END_LINE]);
    else
        return sprintf("Wunschanzeige Nr.%d (%d/%d) [w,z,q]",
            menue[SOUND_OPT_WL_ID],menue[B_CURRENT_LINE],menue[B_END_LINE]);
}
 
static mixed sc_one_wish_action(string str, mapping * menues)
{
    mapping soundfile,menue = menues[<1];
    mapping wish = SOUNDCHECK->get_one_wish(menue[SOUND_OPT_WL_ID]);
    string *split=explode(space(str)," "),param=implode(split[1..]," ");
    int lvl = get_level(TP);
    switch (lower_case(split[0]))
    {
        case "s": // status (Untermenue)
            if (lvl < SC_LVL_ADMIN)
            {
                browse_write_line("Kein Zugriff.");
                return B_DONE;
            }
            return menues + ({([
                B_TYPE: "sc_all_status",
                SOUND_OPT_SUM: SC_STATUS_SUM_BASIS|SC_WISHLIST_SUM,
                SOUND_OPT_WL_ID: menue[SOUND_OPT_WL_ID],
            ])});
        case "p": // p <nr> Prioritaet.
            if (lvl < SC_LVL_ADMIN)
            {
                browse_write_line("Kein Zugriff.");
                return B_DONE;
            }
            if (param=="" || to_int(param)==0)
            {
                browse_write_line("p <nr>");
                return B_DONE;
            }
            wish[SOUND_OPT_PRIORITY] = to_int(param);
            if (!SOUNDCHECK->set_one_wish(wish))
            {
                browse_write_line("Fehler beim Setzen der Priorität.");
                return B_DONE;
            }
            browse_write_line("Priorität gesetzt.");
            return B_REBUILT;
        case "v": // v <von>
            if (lvl < SC_LVL_ADMIN)
            {
                browse_write_line("Kein Zugriff.");
                return B_DONE;
            }
            if (param=="")
            {
                browse_write_line("v <von>");
                return B_DONE;
            }
            wish[SOUND_OPT_FROM] = param;
            if (!SOUNDCHECK->set_one_wish(wish))
            {
                browse_write_line("Fehler beim Setzen von wem.");
                return B_DONE;
            }
            browse_write_line("Von wem gesetzt.");
            return B_REBUILT;
        case "w": // w <wunsch>
            if (lower_case(wish[SOUND_OPT_FROM])!=lower_case(TP_RCN)
                && lvl < SC_LVL_ADMIN)
            {
                browse_write_line("Kein Zugriff.");
            }
            if (param=="")
            {
                browse_write_line("w <wunsch>");
                return B_DONE;
            }
            wish[SOUND_OPT_WISH] = param;
            if (!SOUNDCHECK->set_one_wish(wish))
            {
                browse_write_line("Fehler beim Setzen des Wunsches.");
                return B_DONE;
            }
            browse_write_line("Wunsch gesetzt.");
            return B_REBUILT;
        case "tl": // tl Tondatei loeschen.
            if (lvl < SC_LVL_ADMIN)
            {
                browse_write_line("Kein Zugriff.");
                return B_DONE;
            }
            m_delete(wish,SOUND_OPT_FILE);
            if (!SOUNDCHECK->set_one_wish(wish))
            {
                browse_write_line("Fehler beim Löschen der Tondatei.");
                return B_DONE;
            }
            browse_write_line("Tondatei gelöscht.");
            return B_REBUILT;
        case "t": // t <tondatei> oder Auswahlmenue!
            if (lvl < SC_LVL_ADMIN)
            {
                browse_write_line("Kein Zugriff.");
                return B_DONE;
            }
            if (param=="")
            {
                return menues + ({([
                    B_TYPE: "sc_sounds",
                    SOUND_OPT_SELECTION: SOUND_OPT_WL_ID,
                    SOUND_OPT_WL_ID: menue[SOUND_OPT_WL_ID],
                ])});
            }
            soundfile = SOUNDCHECK->get_one_soundfile(param);
            if (!mappingp(soundfile))
            {
                browse_write_line("Unbekannte Tondatei, "
                    "Auswahl ohne Parameter möglich.");
                return B_DONE;
            }
            wish[SOUND_OPT_FILE] = soundfile;
            if (!SOUNDCHECK->set_one_wish(wish))
            {
                browse_write_line("Fehler beim Setzen der Tondatei.");
                return B_DONE;
            }
            browse_write_line("Tondatei gesetzt.");
            return B_REBUILT;
        case "a": // Abspielen
            if (wish[SOUND_OPT_FILE]==0)
            {
                browse_write_line("Keine Tondatei verknüpft.");
                return B_DONE;
            }
            last_sound_file = wish[SOUND_OPT_FILE];
            TP->receive_message_low("Abspielen der Tondatei.\n", 
                (["message:sound":wish[SOUND_OPT_FILE] ]));
            return B_DONE;
    }
    return B_NOTHING;
}

// --------------------------------------------------------- sc_all_proposals
#define HELP_SC_ALL_PROPOSALS ({ \
"Zu jeder Tondatei wird gepflegt, wer die Quelle ist und wie das Copyright",\
"aussieht. Mit +/- kann man vor und zurueck navigieren und mit 'a' kann man",\
"die Tondatei abspielen, sofern sie existiert.",\
})
static mixed sc_all_proposals_init(mapping old)
{
    old[B_START_LINE] = 0;
    old[B_HELP] = ([ B_TYPE : B_STATICMORE,
                     B_PROMPT : "Hilfe zur Liste der Vorschläge Zeile",
                     B_DATA : HELP_SC_ALL_PROPOSALS ]);
    ADD_HEADER_LINES(
"----------- Liste der Vorschläge ----------------------------------------------"
    );
    return old;
}

static int sc_all_proposals_total(mapping menue)
{
    return SOUNDCHECK->get_all_proposals(menue,1);
}

static string * sc_all_proposals_display(mapping menue)
{
    mapping *proposals = SOUNDCHECK->get_all_proposals(menue+([
        DB_DBG_LIMIT:menue[B_NUM_LINES],
        DB_DBG_OFFSET:menue[B_CURRENT_LINE],
        ]),0);
    if (!pointerp(proposals) || !sizeof(proposals))
        return ({ "Keine Anzeige." });
    string *lines = ({});
    int ix;
    for (ix=0;ix<sizeof(proposals);ix++)
    {
        lines += ({left(sprintf("%03d %s",
            proposals[ix][SOUND_OPT_PR_ID],
            proposals[ix][SOUND_OPT_DESCR]),79) });
    }
    return lines;
}

static string sc_all_proposals_prompt(mapping menue)
{
    return sprintf("Vorschlagsliste (%d/%d) [<nr>,n,z,q]",
        menue[B_CURRENT_LINE],menue[B_END_LINE]);
}

static mixed sc_all_proposals_action(string str, mapping * menues)
{
    mapping proposal;
    int pr_id = to_int(space(str));
    string *split=explode(space(str)," "),param = implode(split[1..]," ");
    if (pr_id)
    {
        proposal = SOUNDCHECK->get_one_proposal(pr_id);
        if (mappingp(proposal))
        {
            return menues + ({([
                B_TYPE:"sc_one_proposal",
                SOUND_OPT_PR_ID:pr_id,
                ])});
        }
        browse_write_line("Ungültige Vorschlagsnummer.");
        return B_DONE;
    }
    switch (lower_case(space(split[0])))
    {
        case "n": // neu
            if (get_level(TP)<SC_LVL_PLAYER)
            {
                browse_write_line("Kein Zugriff.");
                return B_DONE;
            }
            if (param == "")
            {
                browse_write_line("n <vorschlag>");
                return B_DONE;
            }
            pr_id = SOUNDCHECK->set_one_proposal( ([
                SOUND_OPT_STATUS: SC_STATUS_ACTIVE|SC_PROPOSAL_OPEN,
                SOUND_OPT_FROM: TP_RCN,
                SOUND_OPT_DESCR: param, 
                ]) );
            if (pr_id)
            {
                return menues+({([
                    B_TYPE:"sc_one_proposal",
                    SOUND_OPT_PR_ID:pr_id, 
                ])});
            }
            browse_write_line("Fehler beim Anlegen eines Vorschlags.");
            return B_DONE;
    }
    return B_NOTHING;
}

// --------------------------------------------------------- sc_one_proposal
#define HELP_SC_ONE_PROPOSAL ({ \
"Es werden Status, von wem und Beschreibung des Vorschlags angezeigt.",\
"Falls eine Tondatei oder ein Wunsch verknuepft wurde, wird diese ",\
"angezeigt und kann abgespielt werden.",\
})
#define HELP_SC_ONE_PROPOSAL_ADMIN HELP_SC_ONE_PROPOSAL+({ \
"Diese koennen von Admins auch angepasst werden.",\
})
private string* get_one_proposal(int pr_id)
{
    string *lines=({});
    mapping proposal = SOUNDCHECK->get_one_proposal(pr_id);
    int lvl = get_level(TP);
    if (!mappingp(proposal))
        return ({"Keine Anzeige."});
    lines += ({"Vorschlags-Id: "+pr_id});
    if (lvl == SC_LVL_ADMIN)
    {
        lines += explode(wrap_say("S - Status: ",
          implode(get_sc_status(proposal[SOUND_OPT_STATUS]),", ")),"\n")[..<2];
        lines += ({"V - von: "+proposal[SOUND_OPT_FROM]});
        lines += explode(wrap_say("B - Beschreibung: ",
            proposal[SOUND_OPT_DESCR]),"\n")[..<2];
        if (proposal[SOUND_OPT_FILE]==0)
            lines += ({"T - Tondatei: <null>"});
        else
        {
            lines += ({"T - Tondatei: "+proposal[SOUND_OPT_FILE]});
            lines += ({"A - Abspielen der Tondatei." });
        }
        if (proposal[SOUND_OPT_WL_ID]==0)
            lines += ({"W - Wunschreferenz: <null>"});
        else
            lines += ({"W - Wunschreferenz: "+proposal[SOUND_OPT_WL_ID]});
    }
    else
    {
        lines += explode(wrap_say("Status: ",
          implode(get_sc_status(proposal[SOUND_OPT_STATUS]),", ")),"\n")[..<2];
        lines += ({"von: "+proposal[SOUND_OPT_FROM]});
        lines += explode(wrap_say("B - Beschreibung: ",
            proposal[SOUND_OPT_DESCR]),"\n")[..<2];
        if (proposal[SOUND_OPT_FILE]==0)
            lines += ({"Tondatei: <null>"});
        else
        {
            lines += ({"Tondatei: "+proposal[SOUND_OPT_FILE]});
            lines += ({"A - Abspielen der Tondatei." });
        }
        if (proposal[SOUND_OPT_WL_ID]==0)
            lines += ({"Wunschreferenz: <null>"});
        else
            lines += ({"Wunschreferenz: "+proposal[SOUND_OPT_WL_ID]});
    }
    return lines;
}

static mixed sc_one_proposal_init(mapping old)
{
    int lvl = get_level(TP);
    old[B_START_LINE] = 0;
    old[B_HELP] = ([ B_TYPE : B_STATICMORE,
                     B_PROMPT : "Hilfe zur Liste der Tondateien Zeile",
                     B_DATA : (lvl==SC_LVL_ADMIN)?
                                HELP_SC_ONE_PROPOSAL_ADMIN:
                                HELP_SC_ONE_PROPOSAL ]);
    ADD_HEADER_LINES(
"----------- Anzeige eines Vorschlags ------------------------------------------"
    );
    return old;
}

static int sc_one_proposal_total(mapping menue)
{
    return sizeof(get_one_proposal(menue[SOUND_OPT_PR_ID]));
}

static string * sc_one_proposal_display(mapping menue)
{
    menue[B_DATA] = get_one_proposal(menue[SOUND_OPT_PR_ID]);
    return staticmore_display(menue);
}

static string sc_one_proposal_prompt(mapping menue)
{
    if (get_level(TP)==SC_LVL_ADMIN)
        return sprintf("Vorschlagsanzeige Nr.%d (%d/%d) [s,v,b,t,a,w,z,q]",
            menue[SOUND_OPT_PR_ID],menue[B_CURRENT_LINE],menue[B_END_LINE]);
    else
        return sprintf("Vorschlagsanzeige Nr.%d (%d/%d) [b,a,z,q]",
            menue[SOUND_OPT_PR_ID],menue[B_CURRENT_LINE],menue[B_END_LINE]);
}

static mixed sc_one_proposal_action(string str, mapping * menues)
{
    mapping soundfile,wish,menue = menues[<1];
    mapping proposal = SOUNDCHECK->get_one_proposal(menue[SOUND_OPT_PR_ID]);
    string *split=explode(space(str)," "),param=implode(split[1..]," ");
    int lvl = get_level(TP);
    switch (lower_case(split[0]))
    {
        case "s": // status (Untermenue)
            if (lvl < SC_LVL_ADMIN)
            {
                browse_write_line("Kein Zugriff.");
                return B_DONE;
            }
            return menues + ({([
                B_TYPE: "sc_all_status",
                SOUND_OPT_SUM: SC_STATUS_SUM_BASIS|SC_PROPOSAL_SUM,
                SOUND_OPT_PR_ID: menue[SOUND_OPT_PR_ID],
            ])});
        case "v": // v <von>
            if (lvl < SC_LVL_ADMIN)
            {
                browse_write_line("Kein Zugriff.");
                return B_DONE;
            }
            if (param=="")
            {
                browse_write_line("v <von>");
                return B_DONE;
            }
            proposal[SOUND_OPT_FROM] = param;
            if (!SOUNDCHECK->set_one_proposal(proposal))
            {
                browse_write_line("Fehler beim Setzen von wem.");
                return B_DONE;
            }
            browse_write_line("Von wem gesetzt.");
            return B_REBUILT;
        case "b": // b <beschreibung>
            if (lower_case(proposal[SOUND_OPT_FROM])!=lower_case(TP_RCN)
                && lvl < SC_LVL_ADMIN)
            {
                browse_write_line("Kein Zugriff.");
            }
            if (param=="")
            {
                browse_write_line("b <beschreibung>");
                return B_DONE;
            }
            proposal[SOUND_OPT_DESCR] = param;
            if (!SOUNDCHECK->set_one_proposal(proposal))
            {
                browse_write_line("Fehler beim Setzen des Vorschlags.");
                return B_DONE;
            }
            browse_write_line("Vprschlag gesetzt.");
            return B_REBUILT;
        case "tl": // tl Tondatei loeschen.
            if (lvl < SC_LVL_ADMIN)
            {
                browse_write_line("Kein Zugriff.");
                return B_DONE;
            }
            m_delete(proposal,SOUND_OPT_FILE);
            if (!SOUNDCHECK->set_one_proposal(proposal))
            {
                browse_write_line("Fehler beim Löschen der Tondatei.");
                return B_DONE;
            }
            browse_write_line("Tondatei gelöscht.");
            return B_REBUILT;
        case "t": // t <tondatei> oder Auswahlmenue!
            if (lvl < SC_LVL_ADMIN)
            {
                browse_write_line("Kein Zugriff.");
                return B_DONE;
            }
            if (param=="")
            {
                return menues + ({([
                    B_TYPE: "sc_sounds",
                    SOUND_OPT_SELECTION: SOUND_OPT_PR_ID,
                    SOUND_OPT_PR_ID: menue[SOUND_OPT_PR_ID],
                ])});
            }
            soundfile = SOUNDCHECK->get_one_soundfile(param);
            if (!mappingp(soundfile))
            {
                browse_write_line("Unbekannte Tondatei, "
                    "Auswahl ohne Parameter möglich.");
                return B_DONE;
            }
            proposal[SOUND_OPT_FILE] = param;
            if (!SOUNDCHECK->set_one_proposal(proposal))
            {
                browse_write_line("Fehler beim Setzen der Tondatei.");
                return B_DONE;
            }
            browse_write_line("Tondatei gesetzt.");
            return B_REBUILT;
        case "a": // Abspielen
            if (proposal[SOUND_OPT_FILE]==0)
            {
                browse_write_line("Keine Tondatei verknüpft.");
                return B_DONE;
            }
            last_sound_file = proposal[SOUND_OPT_FILE];
            TP->receive_message_low("Abspielen der Tondatei.\n", 
                (["message:sound":proposal[SOUND_OPT_FILE] ]));
            return B_DONE;
        case "wl": // wl Wunschreferenz loeschen.
            if (lvl < SC_LVL_ADMIN)
            {
                browse_write_line("Kein Zugriff.");
                return B_DONE;
            }
            m_delete(proposal,SOUND_OPT_WL_ID);
            if (!SOUNDCHECK->set_one_proposal(proposal))
            {
                browse_write_line("Fehler beim Löschen der Wunschreferenz.");
                return B_DONE;
            }
            browse_write_line("Wunschreferenz gelöscht.");
            return B_REBUILT;
        case "w": // w <wunschreferenz> oder Auswahlmenue!
            if (lvl < SC_LVL_ADMIN)
            {
                browse_write_line("Kein Zugriff.");
                return B_DONE;
            }
            if (param=="")
            {
                return menues + ({([
                    B_TYPE: "sc_wishlist",
                    SOUND_OPT_SELECTION: SOUND_OPT_PR_ID,
                    SOUND_OPT_PR_ID: menue[SOUND_OPT_PR_ID],
                ])});
            }
            wish = SOUNDCHECK->get_one_wish(to_int(param));
            if (!mappingp(wish))
            {
                browse_write_line("Unbekannte Wunschreferenz, "
                    "Auswahl ohne Parameter möglich.");
                return B_DONE;
            }
            proposal[SOUND_OPT_WL_ID] = to_int(param);
            if (!SOUNDCHECK->set_one_proposal(proposal))
            {
                browse_write_line("Fehler beim Setzen der Wunschreferenz.");
                return B_DONE;
            }
            browse_write_line("Wunschreferenz gesetzt.");
            return B_REBUILT;
    }
    return B_NOTHING;
}
// ----------------------------------------------------------- standard lfuns

string query_noise()
{
    if (!stringp(last_sound_file))
        return "Keine letzte Tondatei bekannt.";
    TP->receive_message_low("Abspielen der Tondatei.\n", 
                (["message:sound":last_sound_file ]));
    return "";
}

mixed query_auto_load()
{
    if (!wizp(ENV_TO)) return 0;
    return ([ "last_sound_file":last_sound_file ]);
}

void init_arg(mixed m)
{
    if (mappingp(m))
    {
        last_sound_file = m["last_sound_file"];
    }
}

varargs string query_read(string parse_rest, string str,object leser)
{
    if (get_level(TP)<5)
        dynamic_browse(([B_TYPE:"sc_hints"]));
    else
        dynamic_browse(([B_TYPE:"sc_main_menu"]));
    return "";
}

void create()
{
    set_name("tonprüfer");
    set_gender("maennlich");
    set_id( ({"tonprüfer","prüfer"}) );
    set_long("Einer dieser phantastischen Tonprüfer, den man lesen kann.");
    set_noise("Momentan ist der Tonprüfer ganz still.");
    set(P_LOOK_MSG,"");
    set(P_READ_MSG,"");
    set(P_SMELL_MSG,"");
    set(P_HEAR_MSG,"");
    set(P_FEEL_MSG,"");
    set(P_TAKE_MSG,"");
    set_no_move(1);
    set_no_move_reason(Der()+" ist ein Fixpunkt in diesem Universum und kann "
        "nicht bewegt werden.");
}

void prepare_renewal()
{
}

void abort_renewal()
{
}

void finish_renewal(object neu)
{
}


/*
Mascantin Arija sagt: Naja, Mush und Tintin z.B. unterstuetzen
        Hintergrundmusik
Mascantin Arija sagt: Zusaetzlich zu normalen Sounds
Mascantin Arija sagt: Tinyfugu meiner Meinung nach auch
Mascantin Arija sagt: Mudlet macht das auch mit

Du sagst: hmm. ich wuerde aber sagen,d ass man eine musikbox im rathaus
        aufstellt, wo man bisherige sounds anhoeren kann.
Du sagst: und dass man an der musikbox eintragen kann, wer helfen moechte
        und was noch zu machen ist
        
Avalon:
Mascantin Arija sagt: Alles was hier in der Liste under Soundpack steht,
        ist Mush
Mascantin Arija sagt: Mudlet: 3
Mascantin Arija sagt: Mushclient: 2
Mascantin Arija sagt: Mushclient bis 4.01: 2
Mascantin Arija sagt: Soundpack (blind): 11
Mascantin Arija sagt: Soundpack (sehend): 2
Mascantin Arija sagt: Tinyfugue5: 1
Mascantin Arija sagt: Xterm: 1
*/
