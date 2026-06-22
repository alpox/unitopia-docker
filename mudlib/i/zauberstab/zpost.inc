// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /i/zauberstab/zmail.inc
// Description: Menue zum Mail-Notifier
// Author:      Myonara (29.10.2017)

#define WOCHENTAGE_L ({"Sonntag","Montag","Dienstag","Mittwoch","Donnerstag",\
    "Freitag","Samstag"})
#define WOCHENTAGE_K ({"So","Mo","Di","Mi","Do","Fr","Sa"})
#define FREQUENCY_M ([\
    NOTIFIER_FQ_NEVER: ""+NOTIFIER_FQ_NEVER+": Nie", \
    NOTIFIER_FQ_AT_ONCE: ""+NOTIFIER_FQ_AT_ONCE+": Zeitnah", \
    NOTIFIER_FQ_DAILY: ""+NOTIFIER_FQ_DAILY+": Taeglich", \
    NOTIFIER_FQ_WEEKLY: ""+NOTIFIER_FQ_WEEKLY+": Woechentlich", \
    NOTIFIER_FQ_MONTHLY: ""+NOTIFIER_FQ_MONTHLY+": Monatlich", \
    ])
#define FREQUENCY_MK ({ \
    NOTIFIER_FQ_NEVER, NOTIFIER_FQ_AT_ONCE, \
    NOTIFIER_FQ_DAILY, NOTIFIER_FQ_WEEKLY, NOTIFIER_FQ_MONTHLY })

#define NOTIFIER_MORE_HF \
"-------------------------------------------------------------------------------"
#define NOTIFIER_MORE_MI \
"...----------------------------------------------------------------------------"
#define NOTIFIER_MORE_HEADER(x) center(" "+space(x)+" ",79,"-")

int z_post(string str)
{
    dynamic_browse( ([
        B_TYPE: "config_wizard",
        NOTIFIER_WIZNAME: this_player()->query_real_name(),
        ]) );
    return 1;
}

// ----------------------------------------------------------------
private int* get_zeit(string tstr)
{
    string* split = explode(space(tstr),":");
    if (sizeof(split)!=2)
    {
        return 0;
    }
    int *ti = ({to_int(split[0]),to_int(split[1]) });
    if (ti[0]<0 || ti[0]>23 || ti[1] < 0 || ti[1]>59 )
    {
        return 0;
    }
    return ti;
}
// ----------------------------------------------------------------
private string* get_config_wizard(mapping menue)
{
    string rname = menue[NOTIFIER_WIZNAME];
    mapping cfg = NOTIFIER_MASTER->get_one_config(rname);
    string *lines = ({});
    int cnt;
    if (!mappingp(cfg)) 
        return ({"Kein Eintrag für "+rname+"."});
    lines += ({sprintf("(t) tägliche Sendezeit: %02d:%02d",
            cfg[NOTIFIER_CFG_D_HOUR],
            cfg[NOTIFIER_CFG_D_MIN]) });
    lines += ({sprintf("(w) wöchentliche Sendezeit: %s %02d:%02d",
        WOCHENTAGE_K[cfg[NOTIFIER_CFG_W_DAY]],
        cfg[NOTIFIER_CFG_W_HOUR],
        cfg[NOTIFIER_CFG_W_MIN]) });
    lines += ({"(m) Nicht häufiger als: "
        +FREQUENCY_M[cfg[NOTIFIER_CFG_FREQUENCY]] });
    if (cfg[NOTIFIER_CFG_FLAGS] & NOTIFIER_WF_ONLINE_N)
    {
        lines += ({"(b) Online-Benachrichigung: Ja"});
    }
    else
    {
        lines += ({"(b) Online-Benachrichigung: Nein"});
    }
    if (cfg[NOTIFIER_CFG_FLAGS] & NOTIFIER_WF_ONLINE_M)
    {
        lines += ({"(p) Online-Post: Ja"});
    }
    else
    {
        lines += ({"(p) Online-Post: Nein"});
    }
    cnt = NOTIFIER_MASTER->get_all_wiz_abo(([NOTIFIER_WIZNAME:rname]),1);
    lines+= ({"(a) "+cnt+" Abonnements."});
    cnt = NOTIFIER_MASTER->get_all_wiz_fdb_lists(
        ([NOTIFIER_WIZNAME:menue[NOTIFIER_WIZNAME] ]),1);
    lines+= ({"(l) "+cnt+" FDB-Listen."});
    cnt = NOTIFIER_MASTER->get_all_events(
        ([NOTIFIER_WIZNAME:menue[NOTIFIER_WIZNAME] ]),1);
    lines+= ({"(e) "+cnt+" Ereignisse."});
    return lines;
}

static mixed config_wizard_init(mapping old)
{
    old[B_START_LINE] = 0;
    old[NOTIFIER_WIZNAME] ||= this_player()->query_real_name();
    if (!TP->query_no_ascii_art())
        old[B_HEADER_LINES] = ({ 
            NOTIFIER_MORE_HEADER("Hauptkonfiguration"), 
            NOTIFIER_MORE_HF,
            NOTIFIER_MORE_MI, 
            NOTIFIER_MORE_MI });
    old[B_HELP] = ([
        B_TYPE : "dynamicmore",
        B_FILE : NOTIFIER_HELP("hauptkonfig"),
        ]);
    return old;
}

static int config_wizard_total(mapping menue)
{
    return sizeof(get_config_wizard(menue));
}

static string * config_wizard_display(mapping menue)
{
    menue[B_DATA] = get_config_wizard(menue);
    return staticmore_display(menue);
}

static string config_wizard_prompt(mapping menue)
{
    return "Hauptkonfiguration für "
        +capitalize(menue[NOTIFIER_WIZNAME])+" [t,w,m,b,p,a,l,e,?,q]";
}

static mixed config_wizard_action(string str, mapping * menues)
{
    mapping menue = menues[<1];
    string *split=explode(space(str)," ");
    int *ti,tmp;
    string param = implode(split[1..]," ");
    string rname = menue[NOTIFIER_WIZNAME];
    mapping cfg = NOTIFIER_MASTER->get_one_config(rname)||([]);
    if (!adminp(TP) && rname != this_player()->query_real_name())
    {
        browse_write_line("Kein Zugriff.");
        return B_QUIT;
    }
    switch (split[0]) 
    {
        case "t": // HH:MI
            ti = get_zeit(param);
            if (!ti) 
            {
                browse_write_line("Falsches Zeitformat (t 23:59)");
                return B_DONE;
            }
            cfg[NOTIFIER_CFG_D_HOUR] = ti[0];
            cfg[NOTIFIER_CFG_D_MIN] = ti[1];
            if (!NOTIFIER_MASTER->set_one_config(cfg))
            {
                browse_write_line("Fehler beim Setzen der Zeit.");
                return B_DONE;
            }
            browse_write_line("Tägliche Zeit gesetzt.");
            return B_REBUILT;
        case "w": // WT HH:MI
            if (sizeof(split)!=3) 
            {
                browse_write_line("Falsches Zeitformat (w Fr 23:59)");
                return B_DONE;
            }
            param = capitalize(lower_case(split[1]))[0..1];
            tmp = member(WOCHENTAGE_K,param);
            if (tmp == -1)
            {
                browse_write_line("Falsches Zeitformat (w Sa 23:59)");
                return B_DONE;
            }
            ti = get_zeit(split[2]);
            if (!ti) 
            {
                browse_write_line("Falsches Zeitformat (w Mi 23:59)");
                return B_DONE;
            }
            cfg[NOTIFIER_CFG_W_DAY] = tmp;
            cfg[NOTIFIER_CFG_W_HOUR] = ti[0];
            cfg[NOTIFIER_CFG_W_MIN] = ti[1];
            if (!NOTIFIER_MASTER->set_one_config(cfg))
            {
                browse_write_line("Fehler beim Setzen der Zeit.");
                return B_DONE;
            }
            browse_write_line("Wöchentliche Zeit gesetzt.");
            return B_REBUILT;
        case "m": // minimale Frequenz 0..4 siehe FREQUENCY_L
            if (sizeof(split)<2) 
            {
                browse_write_line("m <nr>  Gültige Werte und Bedeutung:");
                for (tmp=0;tmp<sizeof(FREQUENCY_MK);tmp++)
                {
                    browse_write_line(FREQUENCY_M[FREQUENCY_MK[tmp]]);
                }
                return B_DONE;
            }
            tmp = to_int(param);
            if (!member(FREQUENCY_M,tmp))
            {
                browse_write_line("Für \"Nicht häufiger als\" sind nur "
                    "zulässig:");
                for (tmp=0;tmp<sizeof(FREQUENCY_MK);tmp++)
                {
                    browse_write_line(FREQUENCY_M[FREQUENCY_MK[tmp]]);
                }
                return B_DONE;
            }
            cfg[NOTIFIER_CFG_FREQUENCY] = tmp;
            if (!NOTIFIER_MASTER->set_one_config(cfg))
            {
                browse_write_line(
                     "Fehler beim Setzen von \"nicht häufiger als\".");
                return B_DONE;
            }
            browse_write_line("\"Nicht häufiger als\" gesetzt.");
            return B_REBUILT;
        case "b":
            cfg[NOTIFIER_CFG_FLAGS] ^= NOTIFIER_WF_ONLINE_N;
            if (!NOTIFIER_MASTER->set_one_config(cfg))
            {
                browse_write_line(
                     "Fehler beim Ändern der Online-Benachrichtigung.");
                return B_DONE;
            }
            browse_write_line("Online-Benachrichtigung an/aus gewechselt.");
            return B_REBUILT;
        case "p":
            cfg[NOTIFIER_CFG_FLAGS] ^= NOTIFIER_WF_ONLINE_M;
            if (!NOTIFIER_MASTER->set_one_config(cfg))
            {
                browse_write_line(
                     "Fehler beim Ändern der Online-Post.");
                return B_DONE;
            }
            browse_write_line("Online-Post an/aus gewechselt.");
            return B_REBUILT;
        case "a": // Untermenue Abonnements
            return menues + ({([
                B_TYPE : "wiz_abonnements",
                NOTIFIER_WIZNAME: menue[NOTIFIER_WIZNAME],
            ])});
        case "l": // Untermenue FDB-Listen
            return menues + ({([
                B_TYPE : "wiz_all_listids",
                NOTIFIER_WIZNAME: menue[NOTIFIER_WIZNAME],
            ])});
        case "e": // Untermenue Ereignisse
            return menues + ({([
                B_TYPE : "all_events",
                NOTIFIER_WIZNAME: menue[NOTIFIER_WIZNAME],
            ])});
        case "nw": // namenswechsel (Adminkommando).
            if (!adminp(TP))
            {
                browse_write_line("Kein Zugriff.");
                return B_DONE;
            }
            if (param == "")
            {
                split = NOTIFIER_MASTER->get_all_wizards();
                browse_write_line(
                    "nw <name>");
                browse_write_line(wrap_say("Konfigurationen verfügbar für:",
                    implode(split||({}),", "))[..<2]);
                return B_DONE;
            }
            if (wizplayerp(param))
            {
                menue[NOTIFIER_WIZNAME] = param;
                menues[<1] = menue;
                browse_write_line(CAP(param) +" gesetzt.");
                return menues;
            }
            browse_write_line("Name passt zu keinem Gott.");
            return B_DONE;
     }
     return B_NOTHING;
}

// ----------------------------------------------------------------

static mixed wiz_abonnements_init(mapping old)
{
    old[B_START_LINE] = 0;
    old[NOTIFIER_WIZNAME] ||= this_player()->query_real_name();
    if (!TP->query_no_ascii_art())
        old[B_HEADER_LINES] = ({ 
            NOTIFIER_MORE_HEADER("Abonnements"), 
            NOTIFIER_MORE_HF,
            NOTIFIER_MORE_MI, 
            NOTIFIER_MORE_MI });
    old[B_HELP] = ([
        B_TYPE : "dynamicmore",
        B_FILE : NOTIFIER_HELP("wizard_abo"),
        ]);
    return old;
}

static int wiz_abonnements_total(mapping menue)
{
    return NOTIFIER_MASTER->get_all_wiz_abo(
        ([NOTIFIER_WIZNAME:menue[NOTIFIER_WIZNAME]]),1);
}

static string * wiz_abonnements_display(mapping menue)
{
    mapping * m_abos = NOTIFIER_MASTER->get_all_wiz_abo(([
        NOTIFIER_WIZNAME:menue[NOTIFIER_WIZNAME],
        DB_DBG_LIMIT: menue[B_NUM_LINES],
        DB_DBG_OFFSET: menue[B_CURRENT_LINE]]),0);
    int ix;
    string *lines = ({});
    mapping cache = ([]);
    if (!sizeof(m_abos))
        return ({"Keine Anzeige."});
    for (ix = 0;ix<sizeof(m_abos);ix++)
    {
        int jx = ix+1+menue[B_CURRENT_LINE];
        cache[jx] = m_abos[ix][NOTIFIER_ABO_TYPE];
        lines += ({ sprintf("%2d %s (%d)",
            jx,cache[jx],m_abos[ix][NOTIFIER_CFG_FREQUENCY]) });
    }
    menue[DB_DBG_CACHE] = cache;
    return lines;
}

static string wiz_abonnements_prompt(mapping menue)
{
    return sprintf("Abonnements für %s (%d/%d) [n,<nr>,?,z,q]",
            capitalize(menue[NOTIFIER_WIZNAME]),
            menue[B_CURRENT_LINE],menue[B_END_LINE]);
}

static mixed wiz_abonnements_action(string str, mapping * menues)
{
    str = space(str);
    mapping menue = menues[<1];
    mapping cache = menue[DB_DBG_CACHE]||([]);
    int ix;
    switch (str)
    {
        case "n": // neu
            if (NOTIFIER_MASTER->get_free_abos(menue[NOTIFIER_WIZNAME],1)>0)
            {
                return menues + ({([
                    B_TYPE:"select_abo",
                    NOTIFIER_WIZNAME: menue[NOTIFIER_WIZNAME],
                ])});
            }
            browse_write_line("Alle verfügbaren Abonnements ausgeschöpft.");
            return B_DONE;
        default:
            if (member(cache,ix = to_int(str)))
            {
                return menues + ({([
                     B_TYPE : "wiz_one_abo",
                     NOTIFIER_WIZNAME: menue[NOTIFIER_WIZNAME],
                     NOTIFIER_ABO: cache[ix],
                ])});
            }
    }
    return B_NOTHING;
}

// ----------------------------------------------------------------
static mixed select_abo_init(mapping old)
{
    old[B_START_LINE] = 0;
    if (!TP->query_no_ascii_art())
        old[B_HEADER_LINES] = ({ 
            NOTIFIER_MORE_HEADER("Abonnement aussuchen"), 
            NOTIFIER_MORE_HF,
            NOTIFIER_MORE_MI, 
            NOTIFIER_MORE_MI });
    old[B_HELP] = ([
        B_TYPE : "dynamicmore",
        B_FILE : NOTIFIER_HELP("select_abo"),
        ]);
    return old;
}

static int select_abo_total(mapping menue)
{
    return NOTIFIER_MASTER->get_free_abos(menue[NOTIFIER_WIZNAME],1);
}

static string * select_abo_display(mapping menue)
{
    string *lines = NOTIFIER_MASTER->get_free_abos(menue[NOTIFIER_WIZNAME],0);
    // TODO paging bei get_Free_abos einfuehren sobald notwendig.
    int ix;
    mapping cache = ([]);
    for (ix = 0;ix<sizeof(lines);ix++)
    {
        cache[ix+1+menue[B_CURRENT_LINE]] = lines[ix];
        lines[ix] = sprintf("%2d %s",ix+1+menue[B_CURRENT_LINE],lines[ix]);
    }
    menue[DB_DBG_CACHE] = cache;
    return lines;
    
}

static string select_abo_prompt(mapping menue)
{
    return sprintf("Wähle ein Abonnements für %s (%d/%d) [<nr>,?,z,q]",
            capitalize(menue[NOTIFIER_WIZNAME]),
            menue[B_CURRENT_LINE],menue[B_END_LINE]);
}

static mixed select_abo_action(string str, mapping * menues)
{
    str = space(str);
    mapping menue = menues[<1];
    mapping cache = menue[DB_DBG_CACHE]||([]);
    int ix;
    if (member(cache,ix = to_int(str)))
    {
        if (!NOTIFIER_MASTER->set_one_wiz_abo( ([
             NOTIFIER_WIZNAME: menue[NOTIFIER_WIZNAME],
             NOTIFIER_ABO: cache[ix],
             NOTIFIER_CFG_FREQUENCY : NOTIFIER_FQ_NEVER,
            ]) ))
        {
            browse_write_line("Fehler beim Anlegen des Abonnements");
            return B_DONE;
        }
        browse_write_line("Abonnement erfolgreich angelegt.");
        return menues[..<2] + ({([
             B_TYPE : "wiz_one_abo",
             NOTIFIER_WIZNAME: menue[NOTIFIER_WIZNAME],
             NOTIFIER_ABO: cache[ix],
        ])});
    }
    return B_NOTHING;
}

//----------------------------------------------------------------------------

private string *get_wiz_one_abo(mapping menue)
{
    mapping m_wizabo = NOTIFIER_MASTER->get_one_wiz_abo(
        menue[NOTIFIER_WIZNAME],menue[NOTIFIER_ABO]);
    if (!mappingp(m_wizabo))
        return ({"Keine Anzeige."});
    string *lines = ({ capitalize(menue[NOTIFIER_WIZNAME])
        +"-Abonnement: "+menue[NOTIFIER_ABO] });
    lines += ({"(f) Aktuelle Sendehäufigkeit: "
        +FREQUENCY_M[m_wizabo[NOTIFIER_CFG_FREQUENCY]] });
    // TODO mailformats if any...
    return lines;
}

static mixed wiz_one_abo_init(mapping old)
{
    old[B_START_LINE] = 0;
    if (!TP->query_no_ascii_art())
        old[B_HEADER_LINES] = ({ 
            NOTIFIER_MORE_HEADER("Abonnement anpassen"), 
            NOTIFIER_MORE_HF,
            NOTIFIER_MORE_MI, 
            NOTIFIER_MORE_MI });
    old[B_HELP] = ([
        B_TYPE : "dynamicmore",
        B_FILE : NOTIFIER_HELP("wiz_one_abo"),
        ]);
    return old;
}

static int wiz_one_abo_total(mapping menue)
{
    return sizeof(get_wiz_one_abo(menue));
}

static string * wiz_one_abo_display(mapping menue)
{
    return get_wiz_one_abo(menue);
}

static string wiz_one_abo_prompt(mapping menue)
{
    return "Abonnement "+menue[NOTIFIER_ABO]+" für "
        +capitalize(menue[NOTIFIER_WIZNAME])+" [f,?,z,q]";
}

static mixed wiz_one_abo_action(string str, mapping * menues)
{
    mapping menue = menues[<1];
    mapping m_wizabo = NOTIFIER_MASTER->get_one_wiz_abo(
        menue[NOTIFIER_WIZNAME],menue[NOTIFIER_ABO]);
    int tmp;
    string* split = explode(space(str)," ");
    string param = implode(split[1..]," ");
    switch(split[0])
    {
        case "f":
            if (sizeof(split)<2) 
            {
                browse_write_line("f <nr>  Gültige Werte und Bedeutung:");
                for (tmp=0;tmp<sizeof(FREQUENCY_MK);tmp++)
                {
                    browse_write_line(FREQUENCY_M[FREQUENCY_MK[tmp]]);
                }
                return B_DONE;
            }
            tmp = to_int(param);
            if (!member(FREQUENCY_M,tmp))
            {
                browse_write_line("Für Sendehäufigkeit sind nur "
                    "zulässig:");
                for (tmp=0;tmp<sizeof(FREQUENCY_MK);tmp++)
                {
                    browse_write_line(FREQUENCY_M[FREQUENCY_MK[tmp]]);
                }
                return B_DONE;
            }
            m_wizabo[NOTIFIER_CFG_FREQUENCY] = tmp;
            if (!NOTIFIER_MASTER->set_one_wiz_abo(m_wizabo))
            {
                browse_write_line(
                     "Fehler beim Setzen der Sendehäufigkeit.");
                return B_DONE;
            }
            browse_write_line("Sendehäufigkeit gesetzt.");
            return B_REBUILT;
    }
    return B_NOTHING;
}

//----------------------------------------------------------------------------
static mixed wiz_all_listids_init(mapping old)
{
    old[B_START_LINE] = 0;
    old[NOTIFIER_WIZNAME] ||= this_player()->query_real_name();
    if (!TP->query_no_ascii_art())
        old[B_HEADER_LINES] = ({ 
            NOTIFIER_MORE_HEADER("FDB-Listen von "+CAP(old[NOTIFIER_WIZNAME])), 
            NOTIFIER_MORE_HF,
            NOTIFIER_MORE_MI, 
            NOTIFIER_MORE_MI });
    old[B_HELP] = ([
        B_TYPE : "dynamicmore",
        B_FILE : NOTIFIER_HELP("wiz_all_listids"),
        ]);
    return old;
}

static int wiz_all_listids_total(mapping menue)
{
    return NOTIFIER_MASTER->get_all_wiz_fdb_lists(
        ([NOTIFIER_WIZNAME:menue[NOTIFIER_WIZNAME] ]),1);
}

static string * wiz_all_listids_display(mapping menue)
{
    mapping * fdblists = NOTIFIER_MASTER->get_all_wiz_fdb_lists(
        ([NOTIFIER_WIZNAME:menue[NOTIFIER_WIZNAME],
            DB_DBG_LIMIT:menue[B_NUM_LINES],
            DB_DBG_OFFSET:menue[B_CURRENT_LINE] ]),0);
    int ix;
    string *lines = ({});
    mapping cache = ([]);
    if (!sizeof(fdblists))
        return ({"Keine Anzeige."});
    for (ix = 0;ix<sizeof(fdblists);ix++)
    {
        int jx = ix+1+menue[B_CURRENT_LINE];
        cache[jx] = fdblists[ix][NOTIFIER_FDB_LISTID];
        lines += ({ sprintf("%2d %s (%d,%d)",
            jx,cache[jx],fdblists[ix][NOTIFIER_CFG_FREQUENCY],
            fdblists[ix][NOTIFIER_MAIL_FORMAT]) });
    }
    menue[DB_DBG_CACHE] = cache;
    return lines;
}

static string wiz_all_listids_prompt(mapping menue)
{
    return sprintf("Alle FDB-Listen für %s (%d/%d) [n <listid>,<nr>,z,q]",
            capitalize(menue[NOTIFIER_WIZNAME]),
            menue[B_CURRENT_LINE],menue[B_END_LINE]);
}

static mixed wiz_all_listids_action(string str, mapping * menues)
{
    string *split = explode(space(str)," ");
    string param = implode(split[1..]," ");
    mapping menue = menues[<1];
    mapping cache = menue[DB_DBG_CACHE]||([]);
    mapping m;
    mixed r;
    int ix;
    switch (split[0])
    {
        case "n": // neu
            r = NOTIFIER_MASTER->check_fdb_listid(param);
            if (pointerp(r))
            {
                m = ([
                    NOTIFIER_FDB_LISTID:param,
                    NOTIFIER_WIZNAME:menue[NOTIFIER_WIZNAME],
                    NOTIFIER_CFG_FREQUENCY:NOTIFIER_FQ_NEVER,
                    NOTIFIER_MAIL_FORMAT:1,
                    ]);
                if (!NOTIFIER_MASTER->set_one_wiz_fdb_list(m))
                {
                    browse_write_line("Fehler beim Anlegen der "
                        "Fehlerlistenkonfiguration.");
                    return B_DONE;
                }
                browse_write_line("Fehlerlistenkonfiguration angelegt.");
                return menues+ ({([
                    B_TYPE:"wiz_one_fdb_list",
                    NOTIFIER_WIZNAME: menue[NOTIFIER_WIZNAME],
                    NOTIFIER_FDB_LISTID: param,
                    ])});
            }
            browse_write_line("Ungültige Listid:\n"+r);
            return B_DONE;
        default:
            if (member(cache,ix = to_int(str)))
            {
                return menues + ({([
                     B_TYPE : "wiz_one_fdb_list",
                     NOTIFIER_WIZNAME: menue[NOTIFIER_WIZNAME],
                     NOTIFIER_FDB_LISTID: cache[ix],
                ])});
            }
    }
    return B_NOTHING;
}

//----------------------------------------------------------------------------
private string* get_one_fdb_list(mapping menue)
{
    mapping m_fdblist = NOTIFIER_MASTER->get_one_wiz_fdb_list(
        menue[NOTIFIER_WIZNAME],menue[NOTIFIER_FDB_LISTID] );
    string *lines = ({});
    if (!mappingp(m_fdblist))
        return ({"Keine Anzeige."});
    lines += ({ "Listid: "+m_fdblist[NOTIFIER_FDB_LISTID] });
    lines += ({ "für "+ m_fdblist[NOTIFIER_WIZNAME] });
    lines += ({ "(f) Sendehäufigkeit: "
        +FREQUENCY_M[m_fdblist[NOTIFIER_CFG_FREQUENCY]] });
    lines += ({ "(m) Mail-Format: "+
        (m_fdblist[NOTIFIER_MAIL_FORMAT]==1?"lang":"kurz") });
    return lines;
}

static mixed wiz_one_fdb_list_init(mapping old)
{
    old[B_START_LINE] = 0;
    if (!TP->query_no_ascii_art())
        old[B_HEADER_LINES] = ({ 
            NOTIFIER_MORE_HEADER("Konfiguration einer FDB-Liste"), 
            NOTIFIER_MORE_HF,
            NOTIFIER_MORE_MI, 
            NOTIFIER_MORE_MI });
    old[B_HELP] = ([
        B_TYPE : "dynamicmore",
        B_FILE : NOTIFIER_HELP("wiz_one_fdb_list"),
        ]);
    return old;
}

static int wiz_one_fdb_list_total(mapping menue)
{
    return sizeof(get_one_fdb_list(menue));
}

static string * wiz_one_fdb_list_display(mapping menue)
{
    return get_one_fdb_list(menue);
}

static string wiz_one_fdb_list_prompt(mapping menue)
{
    return "Fehlerlistenkonfiguration [f,m,z,q]";
}

static mixed wiz_one_fdb_list_action(string str, mapping * menues)
{
    mapping menue = menues[<1];
    mapping m_fdblist = NOTIFIER_MASTER->get_one_wiz_fdb_list(
        menue[NOTIFIER_WIZNAME],menue[NOTIFIER_FDB_LISTID] );
    string* split = explode(space(str)," ");
    string param = implode(split[1..]," ");
    int tmp;
    switch (split[0])
    {
        case "f":
            if (sizeof(split)<2) 
            {
                browse_write_line("f <nr>  Gültige Werte und Bedeutung:");
                for (tmp=0;tmp<sizeof(FREQUENCY_MK);tmp++)
                {
                    browse_write_line(FREQUENCY_M[FREQUENCY_MK[tmp]]);
                }
                return B_DONE;
            }
            tmp = to_int(param);
            if (!member(FREQUENCY_M,tmp))
            {
                browse_write_line("Für Sendehäufigkeit sind nur "
                    "zulässig:");
                for (tmp=0;tmp<sizeof(FREQUENCY_MK);tmp++)
                {
                    browse_write_line(FREQUENCY_M[FREQUENCY_MK[tmp]]);
                }
                return B_DONE;
            }
            m_fdblist[NOTIFIER_CFG_FREQUENCY] = tmp;
            if (!NOTIFIER_MASTER->set_one_wiz_fdb_list(m_fdblist))
            {
                browse_write_line(
                     "Fehler beim Setzen der Sendehäufigkeit.");
                return B_DONE;
            }
            browse_write_line("Sendehäufigkeit gesetzt.");
            return B_REBUILT;
        case "m":
            m_fdblist[NOTIFIER_MAIL_FORMAT] = 
                (m_fdblist[NOTIFIER_MAIL_FORMAT]==1) ? 2 : 1;
            if (!NOTIFIER_MASTER->set_one_wiz_fdb_list(m_fdblist))
            {
                browse_write_line(
                     "Fehler beim Setzen des Mailformats.");
                return B_DONE;
            }
            browse_write_line("Mailformat gesetzt.");
            return B_REBUILT;
    }
    return B_NOTHING;
}
//----------------------------------------------------------------------------
static mixed all_events_init(mapping old)
{
    old[B_START_LINE] = 0;
    if (!TP->query_no_ascii_art())
        old[B_HEADER_LINES] = ({ 
            NOTIFIER_MORE_HEADER("Anzeige aller Event/Mailstuecke"), 
            NOTIFIER_MORE_HF,
            NOTIFIER_MORE_MI, 
            NOTIFIER_MORE_MI });
    old[B_HELP] = ([
        B_TYPE : "dynamicmore",
        B_FILE : NOTIFIER_HELP("all_events"),
        ]);
    return old;
}

static int all_events_total(mapping menue)
{  
    return NOTIFIER_MASTER->get_all_events(menue,1);
}

static string * all_events_display(mapping menue)
{
    string *lines = ({});
    mapping * mevt = NOTIFIER_MASTER->get_all_events(menue+([
        DB_DBG_LIMIT:menue[B_NUM_LINES],
        DB_DBG_OFFSET:menue[B_CURRENT_LINE] ]),0);
    int ix;
    mapping cache = ([]);
    if (!sizeof(mevt))
        return ({"Keine Anzeige."});
    for (ix = 0;ix<sizeof(mevt);ix++)
    {
        int jx = ix+1+menue[B_CURRENT_LINE];
        cache[jx] = mevt[ix][NOTIFIER_EVT_EVENTID];
        lines+= ({ left(sprintf("%03d %s %s",
            jx,cache[jx],mevt[ix][NOTIFIER_EVT_SUMMARY]),79) });
    }
    menue[DB_DBG_CACHE] = cache;
    return lines;
}

static string all_events_prompt(mapping menue)
{
    string wiz = menue[NOTIFIER_WIZNAME];
    if (stringp(wiz))
        return sprintf("Events für %s %d/%d [<nr>,z,q]",
            wiz,menue[B_CURRENT_LINE],menue[B_END_LINE] );
    else
        return sprintf("Events %d/%d [<nr>,z,q]",
            menue[B_CURRENT_LINE],menue[B_END_LINE] );
}

static mixed all_events_action(string str, mapping * menues)
{
    mapping menue = menues[<1];
    mapping cache = menue[DB_DBG_CACHE]||([]);
    int jx = to_int(space(str));
    if (member(cache,jx))
    {
        return menues + ({([
            B_TYPE : "one_event",
            NOTIFIER_EVT_EVENTID: cache[jx],
        ])});
    }
    return B_NOTHING;
}
//----------------------------------------------------------------------------
string * get_one_event(string eventid)
{
    string * lines = ({}),*wizzes;
    mapping mevt = NOTIFIER_MASTER->get_one_event(eventid),*mwiz;
    int tim;
    if (!mevt)
        return ({"Keine Anzeige"});
    lines += ({"Event-ID: "+eventid});
    mwiz = NOTIFIER_MASTER->get_all_wiz_events(
        ([NOTIFIER_EVT_EVENTID:eventid]),0);
    wizzes = map(mwiz||({}),(: $1[NOTIFIER_WIZNAME] :));
    lines += explode(wrap_say("Empfänger (e <name>): ",
        implode(wizzes,", "))[..<2],"\n");
    lines += ({"Betreff:", mevt[NOTIFIER_EVT_SUMMARY]});
    tim = mevt[NOTIFIER_EVT_CREATED_ON];
    lines += ({"Erstellt: "+(tim?shorttimestr(tim):"-") });
    tim = mevt[NOTIFIER_EVT_OBSOLETE_ON];
    lines += ({"Obsolete: "+(tim?shorttimestr(tim):"-") });
    lines += ({"Nachrichtentext:"});
    lines += explode(wrap(mevt[NOTIFIER_EVT_BODY]||"\n")[..<2],"\n");
    return lines;
}
static mixed one_event_init(mapping old)
{
    old[B_START_LINE] = 0;
    if (!TP->query_no_ascii_art())
        old[B_HEADER_LINES] = ({ 
            NOTIFIER_MORE_HEADER("Anzeige eines Events/Mailstuecks"), 
            NOTIFIER_MORE_HF,
            NOTIFIER_MORE_MI, 
            NOTIFIER_MORE_MI });
    old[B_HELP] = ([
        B_TYPE : "dynamicmore",
        B_FILE : NOTIFIER_HELP("one_event"),
        ]);
    return old;
}

static int one_event_total(mapping menue)
{
    return sizeof(get_one_event(menue[NOTIFIER_EVT_EVENTID]));
}

static string * one_event_display(mapping menue)
{
    menue[B_DATA] = get_one_event(menue[NOTIFIER_EVT_EVENTID]);
    return staticmore_display(menue);
}

static string one_event_prompt(mapping menue)
{
    return sprintf("Anzeige Event \"%s\" %d/%d[e <name>,z,q]",
        menue[NOTIFIER_EVT_EVENTID],
        menue[B_CURRENT_LINE],menue[B_END_LINE]);
}

static mixed one_event_action(string str, mapping * menues)
{
    // TODO e <name> neu bzw bestehend.
    return B_NOTHING;
}
//----------------------------------------------------------------------------

