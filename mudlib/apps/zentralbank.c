// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /apps/zentralbank.c
// Description: Zentralbank, hier werden die Kurse verwaltet.
//              Zusatz (Myonara,02.09.16): Schliessfachverwaltung.
// Author:      Garthan (28.09.94)
//
// UID: Apps

#pragma no_shadow

inherit "/i/tools/security";
inherit "/i/money/exchange";
inherit "/i/tools/debuglog_db";
inherit "/i/tools/build_table";

#include <acl.h>
#include <config.h>
#include <database.h>
#include <files.h>
#include <level.h>
#include <message.h>
#include <misc.h>
#include <properties.h>
#include <security.h>
#include <time.h>

#include <money.h>

#ifdef TestMUD
#define DB_FILE "/save/zentralbank2.db"
#else
#define DB_FILE "/var/spool/banken/zentralbank2.db"
#endif

#define MAX_MIET_DAUER 3600*24*7*6 // 6 Wochen

private void init_db();

// Die Zentralbankeintraege:
//
// Der eingetragene Wert ist der Wert von 1000 Talern in der gegebenen Waehrung

static mapping compat_money_sg=([
    "sesterze": "sesterz",
    ]);

static mapping compat_money_pl=([
    "sesterzen": "sesterze",
    ]);

static mapping money=([
    "taler":   ({ "taler",    "maennlich", 1000 }),
    "zorkmid": ({ "zorkmids", "maennlich",  500 }),
    "gulden":  ({ "gulden",   "maennlich",  800 }),
    "mark":    ({ "mark",     "weiblich",   333 }),
    "krone":   ({ "kronen",   "weiblich",   500 }),
    "dinar":   ({ "dinare",   "maennlich",  570 }),
    "dukate":  ({ "dukaten",  "weiblich",  7690 }),
    "franken": ({ "franken",  "maennlich", 1300 }),
    "sesterz": ({ "sesterze", "maennlich",  400, 0, ({ "sesterze" }), ({ "sesterzen" }) }),
//    "sesterze": ({ "sesterzen", "weiblich",  400 }),
    "groschen":({ "groschen", "maennlich", 9200 }),
    "kauri":   ({ "kauri",    "saechlich", 2500, MONEY_NOT_DEKLIN }),
    "drachme": ({ "drachmen", "weiblich",   666 }),
//      "splitter": ({ "splitter","maennlich", 80 }),
    "riki"    : ({ "riki",    "maennlich", 2000, MONEY_NOT_DEKLIN }),
    ]);

static mapping money_descriptions = ([
    "taler":   "Der Taler ist eine einigermaßen runde, winzige Goldmünze. "
        "Auf der Vorderseite ist eine 1 zu sehen, die von einem "
        "Lorbeerkranz umgeben ist. Auf der Rückseite ist das "
        "Tadmorer Wappen zu bewundern.",
    "mark":    "Die Deutsche Mark. Sie ist aus silbrigem Metall, auf der "
        "Vorderseite ist eine 1, darunter steht \"DEUTSCHE MARK\", links "
        "und rechts der 1 ist jeweils Eichenlaub zu sehen. Die Rückseite "
        "zeigt einen Adler, der von den Worten \"BUNDESREPUBLIK "
        "DEUTSCHLAND\" umgeben ist.",
    "franken": "Der Franken ist eine flache, dreieckige Münze. Der Rand der "
        "Münze ist abgestuft und erinnert auffallend an die Silhouette der "
        "Schichtenwelt.\n"
        "Auf ihrer Rückseite wurde eine große Eins eingraviert.\n"
        "Die Münze besteht aus purem Gold.",
    "dinar":   "Der Dinar ist eine schwere Goldmünze mit einem "
        "Durchmesser von mehr als drei Zentimetern. Auf der Vorderseite "
        "ist unten deutlich eine kleine, arabische 1 zu erkennen. Darüber "
        "befindet sich das Wappen von Dörrland, in dem deutlich eine "
        "Königskobra zu erkennen ist, und ferner ein arabischer "
        "Text, der sich im Kreis am Münzenrand entlangzieht. Die Rückseite "
        "des Dinars zeigt eine aufgeprägte Oase, in deren Hintergrund die "
        "Sonne wie ein mächtiger Feuerball die ganze Fläche der Münze "
        "ausfüllt.",
    "dukate":   "Die Dukate ist eine recht schwere Platinmünze von "
        "ungefähr zweieinhalb Zentimetern Durchmesser. Auf ihrer "
        "Vorderseite siehst du, umrahmt von drei Minenstützbalken, einen "
        "Skorpion mit hoch erhobenem Stachel, unter dem eine zwergische "
        "Minenaxt und ein gekrümmter Dolch gekreuzt sind. Unter den Waffen "
        "ist eine kleine 1 eingeprägt. Auf der Rückseite ist ein Portrait "
        "des ersten Bürgermeisters von Dörrstadt, Artum, abgebildet, um "
        "das herum in einem Bogen der Wahlspruch der Landesbank von "
        "Dörrstadt und der Dörrstädter Händlervereinigung steht: "
        "PECUNIA NON OLET.",
/*
    "splitter":   "Ein Splitter von einem sehr farbenfrohen Kristall. "
        "Je nach Winkel bricht sich das Licht und erzeugt somit wundervolle "
        "Regenbogenmuster in deiner Umgebung. In dem Kristallsplitter "
        "scheinen Verunreinigungen mit Adamantium eingeschlossen zu sein.",
*/
    "gulden": "Der Gulden ist ein nahezu rundes Goldplättchen, in welches "
        "das Antlitz der Aphrodite geprägt ist. Auf der Rückseite erkennst "
        "du das Abbild eines Segelschiffs mit vier Masten.",
    "riki":"Der Riki ist eine runde Goldmünze von etwa dreieinhalb "
        "Zentimetern Durchmesser. Auf der Vorderseite ist eine Eins "
        "eingeprägt, die von einem Schriftzug in Runen umrundet wird. "
        "Auf der Rückseite ist ein Drache aus Rotgold eingelegt.",
    "krone":"Die Krone ist eine kleine, silberne Münze mit einem Loch "
	"in der Mitte, welches auf der Vorderseite von einem Wellenornament "
	"umgeben ist. Drumherum kann man '1 Krone' lesen, wobei die 1 von "
	"zwei Herzchen flankiert wird. Auf der Rückseite erkennt man ein "
	"dreimaliges M, das jeweils von einer Krone abgelöst wird. Über "
	"jedem M ist wieder ein kleines Herz zu sehen.",
    "sesterz":"Der Sesterz ist eine runde Münze mit einem Durchmesser "
        "von etwas mehr als drei Zentimetern und einem Gewicht von etwa "
	"einer Unze. Auf der Vorderseite des Sesterzes siehst du eine "
	"von einem Lorbeerkranz umrahmte Eins. Die Rückseite des "
	"Sesterzes schmückt das Profil Gaius Julius Cäsars. "
	"Das ganze Profil? Nein, ein kleiner Teil der Nasenspitze fehlt, "
	"der passte wohl nicht mehr drauf!",
]);

/*
FUNKTION: query_money_info
DEKLARATION: varargs mixed *query_money_info([string valuta])
BESCHREIBUNG:
query_money_info(valuta) liefert ein Feld (Array) mit Informationen zur im
Singular angegeben Waehrung valuta, sofern diese Waehrung in der
Zentralbank bekannt ist, ansonsten 0.

    Index          Inhalt
    ------------------------------------------------------------------------
    MONEY_VALUTA   Waehrung im Singular (kleingeschrieben)
    MONEY_VALUTAS  Waehrung im Plural (kleingeschrieben)
    MONEY_GENDER   Geschlecht
    MONEY_KURS     Wechselkurs: Anzahl in der Waehrung, die soviel wert
                   sind wie 1000 Taler.
                   Beispiel: 400 bei Sesterzen bedeutet, dass 400 Sesterzen
                   soviel wert sind wie 1000 Taler 
    MONEY_FLAGS    Flags (koennen verodert werden). Bisher verwendet:
                   MONEY_NOT_DEKLIN: Waehrung wird nicht dekliniert.
                   Der Eintrag MONEY_FLAGS muss nicht existieren, also
                   vorher testen, ob
                   sizeof(query_money_info(valuta))>MONEY_FLAGS.
    
    Die verwendeten Defines sind in /sys/money.h definiert.

Beispiel:

    query_money_info("sesterz") liefert
    ({"sesterz", "sesterzen", "weiblich", 400})

query_money_info() liefert ein Feld, das fuer jede in der Zentralbank
bekannte Waehrung query_money_info(waehrung) enthaelt, also:

    ({
        ({"groschen", "groschen", "maennlich", 9200                  }),
        ({"zorkmid",  "zorkmids", "maennlich", 500                   }),
        ({"riki",     "riki",     "maennlich", 2000, MONEY_NOT_DEKLIN}),
        ...
    })

VERWEISE: query_money_description, query_money_info
GRUPPEN: handel
*/
varargs mixed *query_money_info(string str)
{
   int i;
   string *idxs;
   mixed *tmp;
   
   if(!str)
      for(tmp = ({}), i = sizeof(idxs = m_indices(money)-({0})); i--;)
         tmp += ({ query_money_info(idxs[i]) });
   else
   {
      str = compat_money_sg[str] || str;
      if(tmp = money[str])
         return ({ str }) + tmp;
   }
   return tmp;
}

public int is_valid_serializer(object ob)
{
    return member( ({
        "/room/bank/obj/schliessfaecher.c",
#ifdef UNItopia
        "/z/Gilden/Alchemistengilde/obj/koffer.c",
        "/d/Kokosinsel/Stratos/Auktionshaus/apps/master.c",
        "/p/Schiffe/Moebel/obj/templates/Aufbewahren/spezialtruhe.c",
        "/d/Maerchenland/Luvaria/Feenkindergarten/apps/goettchen_master.c",
#endif
        }),program_name(ob)) != -1;
}

// Bereitsstellen der Funktionen aus /i/money/exchange
// Hierauf wird dann von allen "kurslosen" /i/money/exchanges zugegriffen
void create()
{
   mixed *money;
   int i, *k;      // Einzutragende Kurse
   string *v, *vs; // Einzutragende Valuta, Valutas

   // Hier wird die Kurstafel aus exchange initialisiert.
   // so dass die Funktionen aus exchange funktionieren.
   k = ({});
   v = ({});
   vs = ({});

   money = sort_array(query_money_info(), lambda(({'a,'b}),
      ({ #'<, ({ #'[, 'a, MONEY_VALUTAS }), ({ #'[, 'b, MONEY_VALUTAS })  })));
   
   for(i = sizeof(money); i--;)
   {
      k += ({ money[i][MONEY_KURS] });
      vs += ({ money[i][MONEY_VALUTAS] });
      v += ({ money[i][MONEY_VALUTA] });
   }

   set_valuta_tafel(v);
   set_valutas_tafel(vs);
   set_kurs_tafel(k);
   init_security_for_actions();
   init_security_trust_mudlib();
   foreach(string adm: ADMINS)
        add_security_condition(adm);
   add_security_condition(PATH_ROOM_BANK);
   add_security_condition(#'is_valid_serializer);
   init_db();
}

/*
FUNKTION: query_money_description
DEKLARATION: string query_money_description(string valuta)
BESCHREIBUNG:
Liefert die umbrochene Beschreibung der im Singular angegebenen Waehrung
valuta, sofern eine Beschreibung dafuer in der Zentralbank bekannt ist,
ansonsten "".
VERWEISE: query_money_description, query_money_info
GRUPPEN: handel
*/
string query_money_description(string str) {
    return money_descriptions[str] ? wrap(money_descriptions[str]) : "";
}

string update_valuta(string str)
{
    return compat_money_sg[str] || str;
}

string update_valutas(string str)
{
    return compat_money_pl[str] || str;
}

//------------------------------------------------- Datenbankfunktionen
public void set_armatester_flag(int flag)
{
    check_security(CHECK_ERROR);
    set_db_info("ARMATESTER",flag?"1":"0");
}

public int query_armatester_flag()
{
    return get_db_info("ARMATESTER") == "1";
}

public void migrate_flags()
{
    mixed* columns = db_query_err("PRAGMA table_info(bank_files)");
    columns = map(columns,(: $1[1] :));
    // printf("bank_files:\n%Q\n",columns);
    if (member(columns,"flag_saveroom")==-1)
    {
        db_query_err("ALTER TABLE bank_files ADD COLUMN flag_saveroom NOT NULL DEFAULT 0");
        db_query_err("UPDATE bank_files SET flag_saveroom=1, flags=flags-?1 WHERE (flags & ?1) = ?1",ZB_BANK_SAFEROOM);
    }
}

private void init_db()
{
    init_debuglog(DB_FILE);
    db_debug(0,0,DB_DBG_CREATE,0);
    db_open();
    db_query_err("CREATE TABLE IF NOT EXISTS bank_customers ("
            "realname TEXT, " // eindeutige Id
            "first_login INTEGER, " // first_login mitspeichern und ueberwachen
            "offline_rent INTEGER, " // offline Mietabzuege speichern.
            "this_week INTEGER, "   // temporaeres Flag zur Kennzeichnung,
                                  // ob diese RL-Woche schon verrechnet wurde.
            "PRIMARY KEY(realname) " 
            ")");
    db_query_err("CREATE TABLE IF NOT EXISTS banks ("
            "bank_id TEXT, " // eindeutige BankenId
            "flags INTEGER, "// Flags:
            "info TEXT, " // mapping zusatzinfos
            "title TEXT, " // string Titel der Bank (zur Anzeige)
            "valuta TEXT, "
            "valutas TEXT, "
            "v_gender TEXT, "
            "short_title TEXT, "
            "PRIMARY KEY(bank_id) " 
            ")");
    db_query_err("CREATE TABLE IF NOT EXISTS bank_files ("
            "filename TEXT, " // eindeutiger filename Raum oder NPC
            "bank_id TEXT, " // BankenId
            "flags INTEGER, "// Flags:classic,env,npc-typen
            "flag_saveroom INTEGER, " // flag saveroom 1 oder 0
            "PRIMARY KEY(filename) "
            ")");
    db_query_err("CREATE TABLE IF NOT EXISTS bank_safes ("
            "itemid TEXT, " // unique string, wird vom safe erzeugt.
            "bank_id TEXT, " // string die bank, wie oben.
            "owner TEXT, "   // string der Eigentuermer
            "value INTEGER, " // der aktuelle Wert
            "error_count INTEGER, " // wie oft wurden RTEs gezaehlt.
            "shorttext TEXT, "      // string der short zur Anzeige im Buch        
            "factory TEXT, "  // string filename der Faytory, falls vorhanden
            "factory_id TEXT, "  // string die interne factory-ID
            "paid_until INTEGER, " // bis zu welchem Tag war/ist bezahlt.
            "item_rent INTEGER, "  // Miete abh.vom Item pro RL-Woche (Taler)
            "bank_rent FLOAT, "    // Miete abh.vom Fach,Bank,RL-Woche.(Taler)
            "rent_diff FLOAT, "    // Mieterestmengen < +-1.0
            "flags INTEGER, "      // Sperren und aehnliches.
            "deleted_on INTEGER, " // Zum Loeschen vormerken
            "PRIMARY KEY(itemid) "
            ")");
    db_query_err("CREATE INDEX IF NOT EXISTS bank_safes_owner ON bank_safes (owner)");
    db_query_err("CREATE INDEX IF NOT EXISTS bank_safes_bank_id ON bank_safes (bank_id)");
    db_query_err("CREATE INDEX IF NOT EXISTS bank_safes_owner_bank ON bank_safes (owner, bank_id)");
    db_query_err("CREATE INDEX IF NOT EXISTS idx_bank_files_bank_id ON bank_files(bank_id)");
    db_query_err("CREATE INDEX IF NOT EXISTS idx_bank_files_flags ON bank_files(flags)");
    db_query_err("DROP INDEX IF EXISTS idx_bank_files_flag_saveroom");
    db_query_err("CREATE INDEX IF NOT EXISTS idx_bank_files_flag_saveroom ON bank_files(filename,flag_saveroom,bank_id)");
    db_query_err("CREATE INDEX IF NOT EXISTS idx_bank_files_saveroom_bank "
     "ON bank_files(flag_saveroom, bank_id) WHERE flag_saveroom = 1");

    db_query_err("CREATE INDEX IF NOT EXISTS idx_banks_bank_id_short ON banks(bank_id, short_title)");

    db_query_err("CREATE TABLE IF NOT EXISTS bank_safe_files ("
            "itemid TEXT, file TEXT, PRIMARY KEY(itemid,file) "
            ")");
    db_query_err("CREATE TABLE IF NOT EXISTS bank_safe_items ("
            "itemrowid INTEGER PRIMARY KEY, "   // References bank_safes.rowid
            "item TEXT"                         // save_value() string.
            ")");
    db_query_err("CREATE TABLE IF NOT EXISTS bank_safe_file_counter ("
            "file TEXT, cnt INTEGER, PRIMARY KEY(file) "
            ")");
    
    db_query_err("CREATE TABLE IF NOT EXISTS bank_safe_origin ("
            "itemid TEXT,nr INTEGER,file TEXT,creator TEXT,"
            "created_on INTEGER, unique_id TEXT, PRIMARY KEY(itemid,nr) "
            ")");
    
    db_query_err("CREATE VIEW IF NOT EXISTS bank_safe_lfc AS SELECT "
            "itemid, nr,file,creator,created_on, "
            "file||' '||creator AS loadfile_creator, "
            "unique_id FROM bank_safe_origin");
    db_query_err("CREATE TABLE IF NOT EXISTS bank_safe_file_refs ("
            "itemid TEXT, file TEXT, cnt INTEGER, PRIMARY KEY(itemid,file) "
            ")");
    db_query_err("CREATE TABLE IF NOT EXISTS bank_safe_filter ("
            "file_name TEXT, flags INTEGER, replace_with TEXT, "
            "hint TEXT, "
            "PRIMARY KEY(file_name) "
            ")");
    db_query_err("CREATE TABLE IF NOT EXISTS bank_factory_filter ("
            "factory TEXT,factory_id TEXT, file_name TEXT, flags INTEGER, "
            "hint TEXT, "
            "PRIMARY KEY(factory,factory_id,file_name) "
            ")");
    db_query_err("CREATE TABLE IF NOT EXISTS bank_safe_request_files ("
            "factory TEXT,factory_id TEXT, file_name TEXT, "
            "requestor TEXT, reason TEXT "
            ")");
    string ti = shorttimestr(time()-query_up_time());
    if (get_db_info("MudStart") != ti)
    {
        set_armatester_flag(0);
        if (get_db_info("MudName") != MUD_NAME)
        {
            // Bewegungsdaten loeschen
            // Der Rest sind Stammdaten, die verwendet werden duerfen.
            db_begin();
            // db_query_err("DELETE FROM bank_safe_files");
            // db_query_err("DELETE FROM bank_safe_file_refs");
            // db_query_err("DELETE FROM bank_safe_origin");
            // db_query_err("DELETE FROM bank_safe_file_counter");
            // db_query_err("DELETE FROM bank_safes");
            // db_query_err("DELETE FROM bank_customers");
            set_db_info("MudName",MUD_NAME);
            db_commit();
        }
        // TODO Rollback fuer Crash-Detection?
        set_db_info("MudStart",ti);
    }
    migrate_flags();
}

string get_mud_info()
{
    return get_db_info("MudName")+" "+get_db_info("MudStart");
}

int sql(string sql)
{
    mixed * m;
    if (!check_security(CHECK_ERROR)) {
        //META_D_WARN("sql() security2 failed.",0,"statistik");
        return 0;
    }
    if (!TI || !wizp(TI)) {
        //META_D_WARN("sql() security1 failed.",TI,"statistik");
        return 0;
    }

    m = db_query(sql||"");
    
    if(!m) TI->send_message_to(TI,MT_NOTIFY,MA_USE,
       wrap("Es gab keine Rückgabe."));
    else if(!pointerp(m)) TI->send_message_to(TI,MT_NOTIFY,MA_USE,
       wrap(sprintf("Der Rückgabewert war: '%O'",m)));
    else if(sizeof(m)==0) TI->send_message_to(TI,MT_NOTIFY,MA_USE,
       wrap("Die Tabelle ist (noch) leer."));
    else 
       TI->more (map(
  build_table ( transpose_array(map(m,(: map($1,(: to_string($1) :)) :))),
                      ({"|","-"})
                ), (: space($1) :))
       );
    if(query_db_error()) 
       TI->send_message_to(TI,MT_NOTIFY,MA_USE,
          wrap("Es gab folgenden Fehler: "+ query_db_error()));
    return 1;
}

int tablelist(string str)
{
    check_security(CHECK_ERROR);
    string tb = space(str);
    if (tb == "") {
        return sql("SELECT type, name FROM sqlite_master "
            "WHERE type <> 'index' ORDER BY type,name");
    } else {
        return sql("SELECT sql FROM sqlite_master WHERE NAME = '"+tb+"'");
    }
}

public string query_database_description()
{
    return "Die Zentralbank mit Verwaltungsdaten für Schließfächer.";
}

// Funktion zur Pruefung, ob der Spieler den gleichen GEburtstag haben,
// wenn nicht, werden alle Bank-Daten geloescht.
private int check_and_update_player(<object|string> pl)
{
    mixed level_dates,result;
    int bday,i;
    string err;
    pl = find_player(pl) || pl;
    if (stringp(pl))
    {
        level_dates = PLAYER_READER->query(pl,"level_dates");
    }
    else if (playerp(pl))
    {
        level_dates = pl->query_level_dates();
        pl = pl->query_real_name();
    }
    else
    {
        return 0;
    }
    for (i = sizeof(level_dates); i--; )
        if (level_dates[i][LVL_D_LEVEL] == LVL_PLAYER)
            bday = level_dates[i][LVL_D_DATE];
    result = db_query("SELECT first_login FROM bank_customers "
        "WHERE realname = ?",pl);
    if (!sizeof(result))
    {
        db_query("INSERT INTO bank_customers (realname,first_login) "
            "VALUES (?,?)",pl, bday);
        return 1;
    }
    if (get_one_int(result)!= bday)
    {
        while (42)
        {
            db_begin();
            if (err = query_db_error()) break;
            db_query("DELETE FROM bank_safe_files WHERE itemid IN "
                "(SELECT itemid FROM bank_safes WHERE owner = ?)",pl);
            if (err = query_db_error()) break;
            db_query("DELETE FROM bank_safe_origin WHERE itemid IN "
                "(SELECT itemid FROM bank_safes WHERE owner = ?)",pl);
            if (err = query_db_error()) break;
            db_query("DELETE FROM bank_safe_file_refs WHERE itemid IN "
                "(SELECT itemid FROM bank_safes WHERE owner = ?)",pl);
            if (err = query_db_error()) break;
            db_query("DELETE FROM bank_safe_items WHERE itemrowid IN "
                "(SELECT rowid FROM bank_safes WHERE owner = ?)",pl);
            if (err = query_db_error()) break;
            db_query("DELETE FROM bank_safes WHERE owner = ?",pl);
            if (err = query_db_error()) break;
            db_commit();
            return -1;
        }
        db_rollback();
        raise_error(wrap("check_and_update_player: DB-Error:\n"+err));
    }
    return 1;
}

// Aufruf durch admin in armatester, um Banken zu registrieren, siehe dort
public int register_bank_room(string bank_id, string file,int flag)
{
    string err;
    mixed result;
    int ret = 0;
    if (extern_call() && !check_security()) 
{
        debuglog(sprintf("Security Issue %Q",caller_stack(1)),
            DB_DBGLVL_WARNING,TP_RN,"zentralbank:register_bank_room");
        return 0;
    }
    if (space(bank_id)=="" || REGMATCH(bank_id,"[^A-Za-z_]"))
    {
        return 0;
    }
    while (42)
    {
        err = 0;
        if (!db_open() || query_db_error()) return 0;
        db_begin(); 
        if (query_db_error()) break;
        result = db_query("SELECT info FROM banks WHERE bank_id = ?",bank_id);
        if (query_db_error()) break;
        if (sizeof(result)==0)
        {
            db_query("INSERT INTO banks (bank_id,flags,info) VALUES (?,?,?) ",
                bank_id,flag,"");
            if (query_db_error()) break;
        }
        result = db_query("SELECT bank_id FROM bank_files WHERE filename = ?",
            file);
        if (query_db_error()) break;
        if (sizeof(result)==0)
        {
            db_query("INSERT INTO bank_files (filename,bank_id,flags) "
                "VALUES (?,?,?) ", file, bank_id, flag);
            if (query_db_error()) break;
            ret = 1;
        }
        else
        {
            ret = -1;
        }
        db_commit();
        if (query_db_error()) break;
        db_close();
        if (query_db_error()) break;
        return ret;
    }
    err ||= query_db_error();
    db_rollback();
    debuglog(err,DB_DBGLVL_ERROR,TP_RN,"zentralbank:register_bank_room");
    return 0;
}

int strcmp(string s1,string s2)
{
    return s1 > s2;
}

// fuer die Bankaufsicht: Vitems fuer den Direktsprung zur Bank
public mapping* get_all_bank_safe_room_v_items()
{
    mixed result,line;
    string q;
    mapping* mresult = ({});
    check_security(CHECK_ERROR|CHECK_LAST_OBJECT);

    q = "SELECT bf.filename, bf.bank_id, b.title, b.short_title ";
    q+= "FROM banks b ";
    q+= "JOIN bank_files bf ON b.bank_id = bf.bank_id ";
    q+= "WHERE bf.flag_saveroom = 1";
    result = db_query_err(q); // , ZB_BANK_SAFEROOM
    if (sizeof(result))
        result = sort_array(result, (: strcmp($1[3] || $1[1], $2[3] || $2[1]) :));
    else 
        return 0;
    foreach (line : result)
    {
        if (stringp(line[3]))
            mresult += ({([
                "name" : lower_case(line[3]),
                "cap_name": line[3],
                "gender" : "weiblich",
                "id": ({"bank", lower_case(line[1]),lower_case(line[3]) }),
                "enter_room": line[0],
                "long":line[1]||"Kein Banktitel angegeben!",
            ])});
        else
            mresult += ({([
                "name" : lower_case(line[1]),
                "cap_name": line[1],
                "gender" : "weiblich",
                "id": ({"bank", lower_case(line[1])}),
                "enter_room": line[0],
                "long":line[1]||"Kein Banktitel angegeben!",
            ])});
            
    }
    return mresult;
}

// fuer init_faecher: Bankinfos setzen, falls noetig
public int update_bank_information(string bank_id,string val,string vals,
    string vgen)
{
    mixed result;
    string q;
    check_security(CHECK_ERROR|CHECK_LAST_OBJECT);
    q = "SELECT COUNT(*) FROM banks WHERE bank_id = ?1 AND valuta = ?2 "
        "AND valutas =?3 AND v_gender = ?4 ";
    result = db_query_err(q,bank_id,val,vals,vgen);
    if (sizeof(result) && get_one_int(result)==1)
        return 1;
    q = "SELECT COUNT(*) FROM banks WHERE bank_id = ?1 ";
    result = db_query_err(q,bank_id);
    if (!sizeof(result) || get_one_int(result)==0)
        return 0;
    db_query_err("UPDATE banks SET valuta = ?2,valutas = ?3, v_gender = ?4 "
        "WHERE bank_id = ?1 ",bank_id,val,vals,vgen);
    return 2;
}

// Aufruf durch admin in armatester, um BankNPCs zu registrieren, siehe dort
public int register_bank_npc(string bank_id, string file,int flag)
{
    string err;
    mixed result;
    int ret = 0;
    if (extern_call() && !check_security()) 
    {
        debuglog(sprintf("Security Issue %Q",caller_stack(1)),
            DB_DBGLVL_WARNING,TP_RN,"zentralbank:register_bank_npc");
        return 0;
    }
    if (space(bank_id)=="" || REGMATCH(bank_id,"[^A-Za-z_]"))
    {
        return 0;
    }
    while (42)
    {
        err = 0;
        if (!db_open() || query_db_error()) return 0;
        db_begin(); 
        if (query_db_error()) break;
        result = db_query("SELECT info FROM banks WHERE bank_id = ?",bank_id);
        if (query_db_error()) break;
        if (sizeof(result)==0)
        {
            return 0; // Fehler... muss erst der raum registriert sein.
        }
        result = db_query("SELECT bank_id FROM bank_files WHERE filename = ?",
            file);
        if (query_db_error()) break;
        if (sizeof(result)==0)
        {
            db_query("INSERT INTO bank_files (filename,bank_id,flags) "
                "VALUES (?,?,?) ", file, bank_id, flag);
            if (query_db_error()) break;
            ret = 1;
        }
        else
        {
            ret = -1;
        }
        db_commit();
        if (query_db_error()) break;
        db_close();
        if (query_db_error()) break;
        return ret;
    }
    err ||= query_db_error();
    db_rollback();
    debuglog(err,DB_DBGLVL_ERROR,TP_RN,"zentralbank:register_bank_npc");
    return 0;
}

// Aufruf durch Bankenaufsicht (Gesamtliste von Banken, evtl gefiltert.)
public <string*|int> get_bank_list(mapping options, int countflag)
{
    string q;
    mixed result;
    mixed * args = ({});
    int whereflag = 0;
    if (extern_call() && !check_security(CHECK_LAST_OBJECT)) 
    {
        debuglog(sprintf("Security Issue %Q",caller_stack(1)),
            DB_DBGLVL_WARNING,TP_RN,"zentralbank:get_bank_list");
        return 0;
    }
    if (countflag) 
    {
        q = "SELECT COUNT(*) FROM banks ";
    }
    else
    {
        q = "SELECT bank_id FROM banks ";
    }
    if (options[ZB_BANK_FILE_PATTERN])
    {
        q+= (whereflag++?"AND ":"WHERE ");
        q+= "bank_id IN (SELECT DISTINCT bank_id FROM bank_files ";
        q+= "WHERE filename LIKE ?)";
        args += ({ options[ZB_BANK_FILE_PATTERN] });
    }
    if (!countflag && member(options,DB_DBG_LIMIT))
    {
        q+= "ORDER BY bank_id ";
        q+= "LIMIT ? OFFSET ? ";
        args += ({options[DB_DBG_LIMIT],options[DB_DBG_OFFSET] });
    }
    result = db_query(q,args...);
    if (query_db_error())
    {
        return 0;
    }
    if (countflag)
    {
        return get_one_int(result);
    }
    result ||= ({});
    return map(result, (: $1[0] :) );
}

// Aufruf durch Bankenaufsicht zum Durchblaettern der Banken mit +-
public string get_next_bank(string bank_id, int plusminus)
{
    string q;
    mixed result;
    if (extern_call() && !check_security()) 
    {
        debuglog(sprintf("Security Issue %Q",caller_stack(1)),
            DB_DBGLVL_WARNING,TP_RN,"zentralbank:get_next_bank");
        return 0;
    }
    if (plusminus >= 0)
    {
        q = "SELECT bank_id FROM banks WHERE bank_id > ? "
            "ORDER BY bank_id LIMIT 1 ";
    }
    else
    {
        q = "SELECT bank_id FROM banks WHERE bank_id < ? "
            "ORDER BY bank_id DESC LIMIT 1 ";
    }
    result = db_query(q,bank_id);
    if (query_db_error())
    {
        return 0;
    }
    if (sizeof(result)==0)
    {
        return 0;
    }
    return result[0][0];
}

// Anzeige in bankenaufsicht (Mapping)  
// und als Ueberpruefung ob ne Bank vorhanden
public mapping get_bank(string bank_id)
{
    mapping m;
    mixed result;
    result = db_query("SELECT bank_id, flags, info FROM banks "
        "WHERE bank_id = ?",bank_id);
    if (query_db_error())
    {
        return 0;
    }
    if (sizeof(result)==0)
    {
        return ([]);
    }
    m = result[0][2] == "" ? ([]) : restore_value(result[0][2]);
    m[ZB_BANK_ID] = bank_id;
    m[ZB_BANK_FLAGS] = result[0][1];
    return m;
}

// Anzeige in Bankenaufsicht, nur wo man Zugriff hat...
public <int|string*> get_bank_ids_by_access(mapping options, int countflag)
{
    string q,*erg=({});
    mixed result,*args=({});
    int whereflag = 0;
    if (extern_call() && !check_security(CHECK_LAST_OBJECT)) 
    {
        debuglog(sprintf("Security Issue %Q",caller_stack(1)),
            DB_DBGLVL_WARNING,TP_RN,"zentralbank:get_bank_ids_by_access");
        return 0;
    }
    q = "SELECT bank_id, filename, flags FROM bank_files ";
    if (member(options,ZB_BANK_ID))
    {
        q+= (whereflag++?"AND ":"WHERE ");
        q+= "bank_id = ? "; // '"+options[ZB_BANK_ID]+"' ";
        args += ({options[ZB_BANK_ID]});
    }
    q+= "ORDER BY bank_id ";
    // TODO bei Bedarf: q+= "LIMIT 10000 OFFSET 0 ";
    result = db_query(q,args...);
    if (member(options,ZB_BANK_WIZARD))
    {
        if (!wizp(options[ZB_BANK_WIZARD]))
        {
            return 0;
        }
        if (!adminp(options[ZB_BANK_WIZARD]))
        {
            result = filter(result,(: MAY_WRITE($1[1], $2) :),
                options[ZB_BANK_WIZARD]);
        }
    }
    else
    {
        return 0;
    }
    foreach (mixed element : result) // unify: eindeutige bank_ids extrahieren
    {
        if (member(erg, element[0]) < 0) 
        {
            erg += ({ element[0] });
        }
    }
    if (countflag)
    {
        return sizeof(erg);
    }
    if (sizeof(erg)==0)
    {
        return ({});
    }
    if (member(options,DB_DBG_OFFSET))
    {
        int von = options[DB_DBG_OFFSET];
        int bis = von + options[DB_DBG_LIMIT];
        return erg[von..bis];
    }
    return erg;
}

// Aufruf durch Bankenaufsicht: Bankraeume und NPCs...
public <int|string*> get_bank_files(mapping options, int countflag)
{
    string q;
    mixed result,*args=({});
    int whereflag = 0;
    if (extern_call() && !check_security(CHECK_LAST_OBJECT)) 
    {
        debuglog(sprintf("Security Issue %Q",caller_stack(1)),
            DB_DBGLVL_WARNING,TP_RN,"zentralbank:get_bank_files");
        return 0;
    }
    if (countflag) 
    {
        q = "SELECT COUNT(*) FROM bank_files ";
    }
    else
    {
        q = "SELECT filename, flags FROM bank_files ";
    }
    if (member(options,ZB_BANK_ID))
    {
        q+= (whereflag++?"AND ":"WHERE ");
        q+= "bank_id = ? "; // '"+options[ZB_BANK_ID]+"' ";
        args += ({ options[ZB_BANK_ID] });
    }
    if (!countflag && member(options,DB_DBG_LIMIT))
    {
        q+= "ORDER BY filename ";
        q+= "LIMIT ? OFFSET ? ";
        args += ({options[DB_DBG_LIMIT],options[DB_DBG_OFFSET] });
    }
    result = db_query(q,args...);
    if (query_db_error())
    {
        return 0;
    }
    if (countflag)
    {
        return get_one_int(result);
    }
    return map(result, function string (mixed* line) 
        {
            string str = line[0];
            if (line[1] & ZB_BANK_CLASSIC)
            {
                str+= " (Bank)";
            }
            else if (line[1] & ZB_BANK_ENVIRONMENT)
            {
                str+= " (Raum)";
            }
            else 
            {
                switch (line[1]&ZB_NPC_BANKIER_WECHSLER)
                {
                    case ZB_NPC_BANKIER:
                        str += " (NPC:B.)"; break;
                    case ZB_NPC_GELD_WECHSLER:
                        str += " (NPC:.W)"; break;
                    case ZB_NPC_BANKIER_WECHSLER:
                        str += " (NPC:BW)"; break;
                    default:
                        str+= " (??)"; break;
                }
            }
            return str;
        });
}

// Armatester: beantragen von Objekten/Shadows
public int request_bank_safe_files(string *files, int flags, string user,
        string reason)
{
    string q,file;
    mixed result;
    int count = 0;
    if (extern_call() && !check_security(CHECK_LAST_OBJECT)) 
    {
        debuglog(sprintf("Security Issue %Q",caller_stack(1)),
            DB_DBGLVL_WARNING,TP_RN,"zentralbank:request_bank_safe_files");
        return 0;
    }
    if (!sizeof(files)) return 0;
    q = "SELECT file_name FROM bank_safe_filter WHERE (flags & ?)>0 ";
    q+= "AND file_name IN (";
    foreach(file : files)
    {
        if (count++>0) q+= ", ";
        q+= "'"+file+"'";
    }
    q+= ") ";
    result = db_query(q,flags);
    if (query_db_error()) return 0;
    result ||= ({});
    if (sizeof(result)<sizeof(files))
    {
        result = files - map(result, (: $1[0] :) );
        q = "INSERT INTO bank_safe_request_files "
            "(file_name, requestor, reason) VALUES ";
        count = 0;
        // user = db_escape_string(user||"");
        // reason = db_escape_string(reason||"");
        foreach(file : result)
        {
            if (count++>0) q+= ", ";
            q+= "('"+file+"', ?1, ?2) ";
        }
        db_query(q,user,reason);
        if (query_db_error()) return 0;
        return sizeof(result);
    }
    return -1;
}

// Bankenaufsicht: Liste der Antraege nach Dateiname anzeigen.
public <int|string*> get_file_requests(mapping options, int countflag)
{
    string q;
    mixed result,*args=({});
    int whereflag = 0;
    if (extern_call() && !check_security(CHECK_LAST_OBJECT)) 
    {
        debuglog(sprintf("Security Issue %Q",caller_stack(1)),
            DB_DBGLVL_WARNING,TP_RN,"zentralbank:get_file_requests");
        return 0;
    }
    if (countflag) 
    {
        q = "SELECT COUNT(DISTINCT file_name) FROM bank_safe_request_files ";
    }
    else
    {
        q = "SELECT file_name,COUNT(reason),factory,factory_id "
            "FROM bank_safe_request_files ";
    }
    if (options[ZB_BANK_FILE_PATTERN])
    {
        q+= (whereflag++?"AND ":"WHERE ");
        q+= "(file_name LIKE ? OR factory LIKE ? )";
        args += ({ options[ZB_BANK_FILE_PATTERN], options[ZB_BANK_FILE_PATTERN] });
    }
    if (!countflag && member(options,DB_DBG_LIMIT))
    {
        q+= "GROUP BY file_name,factory,factory_id ORDER BY file_name ";
        q+= "LIMIT ? OFFSET ? ";
        args += ({options[DB_DBG_LIMIT],options[DB_DBG_OFFSET] });
    }
    result = db_query(q,args...);
    if (query_db_error())
    {
        return 0;
    }
    if (countflag)
    {
        return get_one_int(result);
    }
    if (!sizeof(result)) return ({});
    return map(result, function string (mixed* line) 
        {
            if (space(line[2])!="")
                return sprintf("%s %s \"%s\" (%d)",
                    line[0],line[2],line[3],line[1]);
            else
                return sprintf("%s (%d)",line[0],line[1]);
        });
}

// Bankenaufsicht: Antraege je Datei anzeigen.
public <int|string*> get_requests_per_file(mapping options, int countflag)
{
    string q;
    mixed result,*args=({});
    if (extern_call() && !check_security(CHECK_LAST_OBJECT)) 
    {
        debuglog(sprintf("Security Issue %Q",caller_stack(1)),
            DB_DBGLVL_WARNING,TP_RN,"zentralbank:get_file_requests");
        return 0;
    }
    q = "SELECT requestor, reason, factory, factory_id, file_name "
        "FROM bank_safe_request_files ";
    if (member(options,ZB_BANK_FILE))
    {
        q+= "WHERE file_name = ? ";
        args+=({options[ZB_BANK_FILE]});
    }
    else
    {
        return 0; // Fehler ohne Datei...
    }
    if (!countflag && member(options,DB_DBG_LIMIT))
    {
        q+= "ORDER BY requestor,reason ";
    }
    result = db_query(q,args...);
    if (query_db_error())
    {
        return 0;
    }
    if (!sizeof(result)) return countflag ? 0 : ({});
    result = map(result, function string (mixed* line) 
        {
            if (space(line[2])!="")
                return wrap_say(line[0]+":",line[2]+": \""+line[3]+"\" "
                    +line[4]+" "+line[1]);
            else 
                return wrap_say(line[0]+": ",line[1])[..<2];
        });
    result = explode(implode(result,"\n"),"\n");
    if (countflag)
    {
        return sizeof(result);
    }
    if (member(options,DB_DBG_LIMIT))
    {
        int von = options[DB_DBG_OFFSET];
        int bis = von + options[DB_DBG_LIMIT];
        return result[von..bis];
    }
    return result;
}

// Bankenaufsicht: Freigaben EinzelDateien und Shadows anzeigen
public <int|string*> get_bank_safe_filters(mapping options, int countflag)
{
    string q;
    mixed result,*args=({});
    int whereflag = 0;
    if (extern_call() && !check_security(CHECK_LAST_OBJECT)) 
    {
        debuglog(sprintf("Security Issue %Q",caller_stack(1)),
            DB_DBGLVL_WARNING,TP_RN,"zentralbank:get_file_requests");
        return 0;
    }
    if (countflag)
    {
        q = "SELECT COUNT(file_name) FROM bank_safe_filter ";
    }
    else
    {
        q = "SELECT file_name FROM bank_safe_filter ";
    }
    if (options[ZB_BANK_FILE_PATTERN])
    {
        q+= (whereflag++?"AND ":"WHERE ");
        q+= "file_name LIKE ? ";
        args += ({ options[ZB_BANK_FILE_PATTERN] });
    }
    else if (member(options,ZB_BANK_FILE))
    {
        q+= (whereflag++?"AND ":"WHERE ");
        q+= "file_name = ? ";
        args += ({ options[ZB_BANK_FILE] });
    }
    switch (options[ZB_BANK_HINT])
    {
        case 1:
            q+= (whereflag++?"AND ":"WHERE ");
            q+= "hint IS NOT NULL ";
            break;
        case 2:
            q+= (whereflag++?"AND ":"WHERE ");
            q+= "hint IS NULL ";
            break;
    }
    if (!countflag && member(options,DB_DBG_LIMIT))
    {
        q+= "ORDER BY file_name ";
        q+= "LIMIT ? OFFSET ? ";
        args += ({options[DB_DBG_LIMIT],options[DB_DBG_OFFSET] });
    }
    result = db_query(q,args...);
    if (query_db_error() || !pointerp(result))
    {
        return 0;
    }
    if (countflag)
    {
        return get_one_int(result);
    }
    return map(result,(: $1[0] :) );
}

// bankenaufsicht: Antrag einzelne Datei in Freigabe umwandeln.
public int release_bank_safe_file(string file)
{
    string err;
    mixed result;
    if (extern_call() && !check_security(CHECK_LAST_OBJECT)) 
    {
        debuglog(sprintf("Security Issue %Q",caller_stack(1)),
            DB_DBGLVL_WARNING,TP_RN,"zentralbank:release_bank_safe_file");
        return 0;
    }
    while(42)
    {
        db_begin();
        if (query_db_error()) break;
        db_query("DELETE FROM bank_safe_request_files WHERE file_name = ?",
            file);
        if (query_db_error()) break;
        result = db_query("SELECT file_name FROM bank_safe_filter "
            "WHERE file_name = ?", file);
        if (query_db_error()) break;
        if (!sizeof(result))
        {
            db_query("INSERT INTO bank_safe_filter "
                "(file_name,flags,replace_with) "
                "VALUES (?,?,?) ", file, ZB_BANK_ACTIVE, "");
            if (query_db_error()) break;
        }
        db_commit();
        db_close();
        return 1;
    }
    err = query_db_error();
    db_rollback();
    debuglog("DB-Error: "+err,DB_DBGLVL_ERROR,TP_RN,
        "zentralbank:release_bank_safe_file");
    return 0;    
}

// bankenaufsicht: Antrag Factory-File in Freigabe umwandeln.
public int release_bank_safe_factory_file(string file,
    string factory, string factory_id)
{
    string err;
    mixed result;
    if (extern_call() && !check_security(CHECK_LAST_OBJECT)) 
    {
        debuglog(sprintf("Security Issue %Q",caller_stack(1)),
            DB_DBGLVL_WARNING,TP_RN,
            "zentralbank:release_bank_safe_factory_file");
        return 0;
    }
    while(42)
    {
        db_begin();
        if (query_db_error()) break;
        db_query("DELETE FROM bank_safe_request_files WHERE factory = ? "
            "AND factory_id = ?2 ",factory, factory_id);
        if (query_db_error()) break;
        result = db_query("SELECT file_name FROM bank_factory_filter "
            "WHERE factory = ? AND factory_id = ?2 ",factory, factory_id);
        if (query_db_error()) break;
        if (!sizeof(result))
        {
            db_query("INSERT INTO bank_factory_filter "
                "(factory,factory_id,file_name,flags) "
                "VALUES (?,?,?,?) ",
                factory, factory_id, file, ZB_BANK_ACTIVE);
            if (query_db_error()) break;
        }
        db_commit();
        db_close();
        return 1;
    }
    err = query_db_error();
    db_rollback();
    debuglog("DB-Error: "+err,DB_DBGLVL_ERROR,TP_RN,
        "zentralbank:release_bank_safe_factory_file");
    return 0;    
}

// Pruefen, ob Dateien in bank_safe_filter registriert sind.
private int check_bank_safe_files(string *files,int flags)
{
    string q,file;
    mixed result;
    int count = 0;
    if (!sizeof(files)) return 0;
    q = "SELECT file_name FROM bank_safe_filter WHERE (flags & ?)>0 ";
    q+= "AND file_name IN (";
    foreach(file : files)
    {
        if (count++>0) q+= ", ";
        q+= "'"+file+"'";
    }
    q+= ") ";
    result = db_query(q,flags);
    if (query_db_error()) return 0;
    result ||= ({});
    if (sizeof(result)<sizeof(files))
    {
        result = files - map(result, (: $1[0] :) );
        debuglog(sprintf("Dateien fehlen in bank_safe_filter %d %Q",
            flags, result),
            DB_DBGLVL_INFO,TP_RN,"zentralbank:bank_safe_filter");
        return 0;
    }
    return 1;
}

// Abholen der Flags fuer einen Bank_safe...(Filter)
private int get_flags_for_bank_safe_file(string file)
{
    string q;
    mixed result;
    q = "SELECT flags FROM bank_safe_filter WHERE file_name = ? ";
    result = db_query(q,file);
    if (query_db_error()) return 0;
    if (sizeof(result)<1) return 0;
    return get_one_int(result);
}

// Pruefen ob entweder Factory oder Einzelobjekt freigegeben ist.
private int check_bank_factory_or_safe_files(
    string factory,string factory_id,string *files,int flags)
{
    mixed result;
    if (!sizeof(files)) return 0;
    if (stringp(factory) && space(factory)!="" && stringp(factory_id))
    {
        result = db_query_err("SELECT flags FROM bank_factory_filter "
            "WHERE factory = ?1 AND factory_id = ?2 AND file_name = ?3 "
            "AND (flags & ?4) = ?4 ",factory, factory_id,files[0],flags);
        if (!sizeof(result))
        {
            return 0;
        }
        if (sizeof(files)>1)
        {
            return check_bank_safe_files(files[1..],flags);
        }
        return 1;
    }
    return check_bank_safe_files(files,flags);
}

// Armatester: factory_file registrieren inkl Shadows...
public int request_bank_factory_file(string factory, string factory_id,
        string* files, int flags, string user,string reason)
{
    if (extern_call() && !check_security(CHECK_LAST_OBJECT)) 
    {
        debuglog(sprintf("Security Issue %Q",caller_stack(1)),
            DB_DBGLVL_WARNING,TP_RN,"zentralbank:request_bank_factory_file");
        return 0;
    }
    if (!stringp(factory) || !stringp(factory_id))
    {
        return request_bank_safe_files(files,flags,user,reason);
    }
    if (!sizeof(files)) 
    {
        return 0;
    }
    if (check_bank_factory_or_safe_files(factory,factory_id,files[0..0],flags))
    {
        return request_bank_safe_files(files[1..],flags,user,reason);
    }
    db_query_err("INSERT INTO bank_safe_request_files "
            "(factory, factory_id, file_name, requestor, reason) VALUES "
            "(?1,?2,?3,?4,?5) ",factory,factory_id,files[0],user,reason);
    if (sizeof(files)>1)
    {
        return request_bank_safe_files(files[1..],flags,user,reason);
    }
    return 1;
}

private string get_program_name(object ob)
{
    string fn = ob->query_file_name();
    if (fn) return fn;
    string *name = explode(object_name(ob),"#");
    return (sizeof(name)==2)?name[0]:0;
}

// interne Funktion, siehe check_conservation
public varargs <int|string> precheck_conservation(object ob,int f_container,
    mapping check_attributes)
{
    mixed tmp;
    string factory,identifier,tmp_veraltet="",unique_clone;
    mapping m_cons,m_constraints;
    mapping* sh_data = ({});
    object *inv,iob,sh;
    int count;
    check_attributes ||= ([]);
    if (!objectp(ob))
    {
        return "Kein Gegenstand zum Prüfen!";
    }
    if (!ob->query_name() || !ob->query_gender())
    {
        return "Ein namens- oder geschlechtsloser Gegenstand ist nicht einlagerbar.";
    }
    inv = all_inventory(ob);
    ob->prepare_conservation();
    inv->prepare_conservation();
    unique_clone = get_program_name(ob) || program_name(ob)[..<3];
    iob = find_object(unique_clone);
    if (!clonep(ob) || strstr(object_name(ob),"/obj/")==-1)
    {
        if (strstr(unique_clone,"/obj/")!=-1 &&
           check_bank_factory_or_safe_files(factory,identifier,
            ({unique_clone}),ZB_BANK_ACTIVE))
        {
            iob = find_object(unique_clone);
        }
        else
        {
            ob->abort_conservation();
            inv->abort_conservation();
            return Der(ob)+plural(" ist"," sind",ob)
                +" zu einzigartig zum Einlagern.";
        }
    }
    if (iob && object_time(ob) < program_time(iob))
    {
        tmp_veraltet = " Wahrscheinlich veraltet.";
    }
    for (count=0,sh=ob;sh=shadow(sh,0);count++)
    {
        mixed m = sh->query_conservation_arg_sh(sh);
        if (m)
        {
            sh_data += ({ ([ get_program_name(sh) : m ]) });
        }
    }    
    if (sizeof(sh_data)!=count)
    {
        ob->abort_conservation();
        inv->abort_conservation();
        return "Eine unbekannte Wandlung verhindert aktuell die Einlagerung "
            +des(ob)+".";
    }
    m_cons = ob->query(P_CONSERVATION)||([]);
    m_constraints = ob->query_conservation_constraints();
    tmp = ob->query_no_retain();
    if (stringp(tmp))
    {
        ob->abort_conservation();
        inv->abort_conservation();
        return Der(ob)+plural(" laessst"," lassen",ob)
            +" sich nicht einlagern"
            +(tmp!=""?(": "+tmp):"."+tmp_veraltet);
    }
    factory = m_cons[P_CONSERVATION_FACTORY];
    tmp = stringp(factory) 
        ? touch(factory)->precheck_conservation(ob,check_attributes) : 0;
    if (stringp(tmp))
    {
        ob->abort_conservation();
        inv->abort_conservation();
        return (({string}) tmp)+tmp_veraltet;
    }
    tmp = m_cons[P_CONSERVATION_PRECHECK];
    if (stringp(tmp))
    {
        ob->abort_conservation();
        inv->abort_conservation();
        return (({string}) tmp)+tmp_veraltet;
    }
    tmp = ob->precheck_conservation(check_attributes);
    if (stringp(tmp))
    {
        ob->abort_conservation();
        inv->abort_conservation();
        return (({string}) tmp)+tmp_veraltet;
    }
    int flags = get_flags_for_bank_safe_file(
        unique_clone||get_program_name(ob));
    if (living(ob) && (flags&ZB_F_NPC)==0)
    {
        ob->abort_conservation();
        inv->abort_conservation();
        return "Ein Lebewesen wie "+der(ob)+plural(" lässt"," lassen",ob)
            +" sich nicht einlagern.";
    }
    if (ob->query_broken())
    {
        ob->abort_conservation();
        inv->abort_conservation();
        return Der(ob)+plural(" kann"," können",ob)
            +" kaputt nicht eingelagert werden.";
    }
    if (ob->query_wield())
    {
        ob->abort_conservation();
        inv->abort_conservation();
        return Der(ob)+plural(" kann"," können",ob)
            +" geführt nicht eingelagert werden.";
    }
    if (ob->query_worn())
    {
        ob->abort_conservation();
        inv->abort_conservation();
        return Der(ob)+plural(" kann"," können",ob)
            +" angezogen nicht eingelagert werden.";
    }
    if (sizeof(m_constraints)>0)
    {
        debuglog(sprintf("Constraints %Q %Q %Q",m_constraints,ob,
            ob->query_controller()),
            DB_DBGLVL_WARNING,TP_RN,"zentralbank:precheck_conservation");
        ob->abort_conservation();
        inv->abort_conservation();
        if (sizeof(m_constraints)==1 && member(m_constraints,"CONTROLLER"))
        {
            return Der(ob)+plural(" kann"," können",ob)
                +" zur Zeit nicht sicher vor Armageddon "
                "eingelagert werden."+tmp_veraltet;
        }
        return Der(ob)+plural(" kann"," können",ob)
                +" prinzipiell nicht sicher vor Armageddon "
                "eingelagert werden."+tmp_veraltet;
    }
    identifier = stringp(factory) ? m_cons[P_CONSERVATION_IDENTIFIER] : 0;
    if (ob->query_auto_load() && (flags&ZB_F_AUTOLOADER)==0)
    {
        ob->abort_conservation();
        inv->abort_conservation();
        return Der(ob)+" übersteht Armageddon von alleine, "
        "eine Einlagerung ist nicht nötig.";
    }
#ifndef TestMUD
    if (ob->query_no_store() && (flags&ZB_F_WAREHOUSE_STORE)==0)
    {
        ob->abort_conservation();
        inv->abort_conservation();
        return "Die Einlagerung "+des(ob)+" wurde vom Bankenkonsortium "
            "untersagt, bis eine Sondergenehmigung vorliegt.";
    }
    if (!check_bank_factory_or_safe_files(factory,identifier,
        ({unique_clone||get_program_name(ob)}),ZB_BANK_ACTIVE))
    {
        ob->abort_conservation();
        inv->abort_conservation();
        return Dem(ob)+" fehlt die Freigabe durch das Bankenkonsortium.";
    }
#endif
    if (ob->query_container() && (flags&ZB_F_NPC)==0)
    {
        if (f_container)
        {
            if (sizeof(inv))
            {
                ob->abort_conservation();
                inv->abort_conservation();
                return "Innerhalb eines anderen Behältnisses darf dieses "
                       "Behältnis nur leer sein.";
            }
        }
        else
        {
            if (sizeof(inv) && (flags&ZB_F_VALID_CONTAINER)==0)
            {
                ob->abort_conservation();
                inv->abort_conservation();
                return "Dieses Behältnis darf nur leer eingelagert werden.";
            }
            foreach (iob : inv)
            {
                tmp = precheck_conservation(iob,
                    (flags&ZB_F_SPECIAL_CONTAINER)==0?1:2,
                    check_attributes);
                if (stringp(tmp))
                {
                    ob->abort_conservation();
                    inv->abort_conservation();
                    return "Prüfung für "+den(iob)+ " in "+dem(ob)
                        +" fehlgeschlagen, Grund: "+tmp;
                }
            }
        }
    }
    else if (f_container)
    {
        if ((flags&ZB_F_VALID_IN_CONTAINER)==0 && f_container==1)
        {
            ob->abort_conservation();
            inv->abort_conservation();
            return Der(ob)+plural(" darf"," dürfen",ob)
                +" nicht innerhalb eines Behältnisses "
                   "eingelagert werden.";
        }
    }
    return 1;
}

private <int|string> check_contained_ob(object ob,object con)
{
    // Hier wurde precheck_conservation schon durchgefuehrt,
    // es geht nur noch um technische Pruefung
    mapping* sh_data = ({});
    string factory,tmp_veraltet="",unique_clone;
    object sh,iob,*inv;
    mapping m_cons,m_constraints;
    int count;
    
    inv = all_inventory(ob);
    unique_clone = get_program_name(ob) || program_name(ob)[..<3];
    iob = find_object(unique_clone);
    if (iob && object_time(ob) < program_time(iob))
    {
        tmp_veraltet = " Wahrscheinlich veraltet.";
    }
    
    for (count=0,sh=ob;sh=shadow(sh,0);count++)
    {
        mixed m = sh->query_conservation_arg_sh(sh);
        if (m)
        {
            sh_data += ({ ([ get_program_name(sh) : m ]) });
        }
    }    
    if (sizeof(sh_data)!=count)
    {
        ob->abort_conservation();
        inv->abort_conservation();
        return "Eine unbekannte Wandlung verhindert aktuell die Einlagerung "
            +des(ob)+" innerhalb von "+dem(con)+".";
    }
    m_cons = ob->query(P_CONSERVATION)||([]);
    factory = m_cons[P_CONSERVATION_FACTORY];
    mixed ob_data,mud_data=0;
    if (stringp(factory))
    {
        ob_data = touch(factory)->get_conservation_data(ob);
    }
    else if (function_exists("query_conservation_arg",ob))
    {
        ob_data = ob->query_conservation_arg();
    }
    else
    {
        ob_data = 1;
    }
    m_constraints = ob->query_conservation_constraints();
    mud_data = ob->query_conservation_data();
    if (sizeof(m_constraints)>0 || !ob_data || !mud_data)
    {
        ob->abort_conservation();
        inv->abort_conservation();
        return Der(ob)+" innerhalb von "+dem(con)
            +plural(" lässt"," lassen",ob)
                +" sich prinzipiell nicht einlagern."+tmp_veraltet;
    }
    sh_data = filter(sh_data, function int (mapping m)
        {
            string key;
            mixed data;
            foreach (key,data : m)
            {
                if (data==0) return 0;
            }
            return 1;
        } );
#ifndef TestMUD
    if (! check_bank_factory_or_safe_files (
            factory, m_cons[P_CONSERVATION_IDENTIFIER],
            ({unique_clone}) + map(sh_data,(: m_indices($1)[0] :) ),
            ZB_BANK_ACTIVE))
    {
        ob->abort_conservation();
        inv->abort_conservation();
        return Dem(ob)+" innerhalb von "+dem(con)
            +" fehlt die Freigabe durch das Bankenkonsortium!";
    }
#endif
    int flags = get_flags_for_bank_safe_file(
        unique_clone||get_program_name(ob));
    if (ob->query_container() && sizeof(all_inventory(ob)) && (flags&ZB_F_NPC)==0)
    {
        ob->abort_conservation();
        inv->abort_conservation();
        return Der(ob)+" innerhalb von "+dem(con)
            +plural(" kann"," können",ob)
                +" nur leer eingelagert werden.";
    }
    return 1;
}

/*
FUNKTION: check_conservation
DEKLARATION: public depreceated varargs <int|string> check_conservation(object ob,mapping check_attributes)
BESCHREIBUNG:
Bitte ab sofort die Funktion all_check_conservation verwenden!
Liefert eine Begruendung als String, wenn es nicht eingelagert werden kann
und eine 1, wenn es generell eingelagert werden kann.
check_attributes enthaelt folgende Schlüssel für den precheck_conservation:
- ZB_CHECK_TYPE
    - ZB_CT_UNKNOWN:        ist nur während der Umstellung unbekannt.
    - ZB_CT_TEST_ONLY:      für Seherkristall und ähnliches.
    - ZB_CT_SCHLIESSFACH:   Das Bankschliessfach
    - ZB_CT_SPEZIALTRUHE:   Die Schiffsmoebel-Spezialtruhe
    - ZB_CT_AUKTION:        Eine Auktion (erzeugen/einlagern)
    - ZB_CT_ALCH_KOFFER:    Ein Alchemistenkoffer
- ZB_CHECK_CON
    Das Objekt/Mapping der Schliessfach/Truhe/Auktionator, was das Einlagern macht.
- ZB_CHECKER_OB
    Das object/mapping, was die aktuelle Prüfung durchführt.
VERWEISE: precheck_conservation,all_check_conservation
GRUPPEN: armageddon
*/
public varargs <int|string> check_conservation(object ob,mapping check_attributes)
{
    check_attributes ||= ([]);
    <int|string> tmp = precheck_conservation(ob,0,check_attributes);
    int count;
    object sh,cob,iob,*inv = all_inventory(ob);
    string factory,tmp_veraltet,unique_clone;
    mapping m_cons,m_constraints;
    
    if (stringp(tmp)) return tmp;
    m_cons = ob->query(P_CONSERVATION)||([]);
    factory = m_cons[P_CONSERVATION_FACTORY];
    mapping* sh_data = ({});
    
    unique_clone = get_program_name(ob) || program_name(ob)[..<3];
    iob = find_object(unique_clone);
    if (iob && object_time(ob) < program_time(iob))
    {
        tmp_veraltet = " Wahrscheinlich veraltet.";
    }
    
    for (count=0,sh=ob;sh=shadow(sh,0);count++)
    {
        mixed m = sh->query_conservation_arg_sh(sh);
        if (m)
        {
            sh_data += ({ ([ get_program_name(sh) : m ]) });
        }
    }    
    if (sizeof(sh_data)!=count)
    {
        ob->abort_conservation();
        inv->abort_conservation();
        return "Eine unbekannte Wandlung verhindert aktuell die Einlagerung "
            +des(ob)+".";
    }
    mixed ob_data,mud_data=0;
    if (stringp(factory))
    {
        ob_data = touch(factory)->get_conservation_data(ob);
    }
    else if (function_exists("query_conservation_arg",ob))
    {
        ob_data = ob->query_conservation_arg();
    }
    else
    {
        ob_data = 1;
    }
    m_constraints = ob->query_conservation_constraints();
    mud_data = ob->query_conservation_data();
    if (sizeof(m_constraints)>0 || !ob_data || !mud_data)
    {
        ob->abort_conservation();
        inv->abort_conservation();
        return Der(ob)+plural(" lässt"," lassen",ob)
                +" sich prinzipiell nicht einlagern."
            +tmp_veraltet;
    }
    sh_data = filter(sh_data, function int (mapping m)
        {
            string key;
            mixed data;
            foreach (key,data : m)
            {
                if (data==0) return 0;
            }
            return 1;
        } );
#ifndef TestMUD
    if (! check_bank_factory_or_safe_files (
            factory, m_cons[P_CONSERVATION_IDENTIFIER],
            ({unique_clone}) + map(sh_data,(: m_indices($1)[0] :) ),
            ZB_BANK_ACTIVE))
    {
        ob->abort_conservation();
        inv->abort_conservation();
        return Dem(ob)+" fehlt die Freigabe durch das Bankenkonsortium!";
    }
#endif
    int flags = get_flags_for_bank_safe_file(
        unique_clone||get_program_name(ob));
    if (ob->query_container() && (flags&ZB_F_NPC)==0)
    {
        foreach (cob : all_inventory(ob))
        {
            tmp = check_contained_ob(cob,ob);
            if (tmp!=1)
                return tmp;
        }
    }
    ob->abort_conservation();
    inv->abort_conservation();
    return 1;
}

/*
FUNKTION: all_check_conservation 
DEKLARATION: public depreceated varargs <int|string> all_check_conservation(object ob,mapping check_attributes)
BESCHREIBUNG:
Bitte ab sofort die Funktion all_check_conservation verwenden!
Liefert eine Begruendung als String, wenn es nicht eingelagert werden kann
und eine 1, wenn es generell eingelagert werden kann.
In dieser Funktion wird Erfolg und Misserfolg mitdokumentiert.
check_attributes enthaelt folgende Schlüssel für den precheck_conservation:
- ZB_CHECK_TYPE
    - ZB_CT_UNKNOWN:        ist nur während der Umstellung unbekannt.
    - ZB_CT_TEST_ONLY:      für Seherkristall und ähnliches.
    - ZB_CT_SCHLIESSFACH:   Das Bankschliessfach
    - ZB_CT_SPEZIALTRUHE:   Die Schiffsmoebel-Spezialtruhe
    - ZB_CT_AUKTION:        Eine Auktion (erzeugen/einlagern)
    - ZB_CT_ALCH_KOFFER:    Ein Alchemistenkoffer
- ZB_CHECK_CON
    Das Objekt/Mapping der Schliessfach/Truhe/Auktionator, was das Einlagern macht.
- ZB_CHECKER_OB
    Das object/mapping, was die aktuelle Prüfung durchführt.
VERWEISE: precheck_conservation,check_conservation
GRUPPEN: armageddon
*/
public varargs <int|string> all_check_conservation(object ob,mapping check_attributes)
{
    check_attributes ||= ([]);
    <int|string> result = check_conservation(ob,check_attributes);
    string type;
    string oknok = stringp(result) ? "nok" : "ok";
    object who = check_attributes[ZB_CHECKER_OB] || TI || TP;
    switch (check_attributes[ZB_CHECK_TYPE])
    {
        case ZB_CT_AUKTION:
            type = "Auktion";
            break;
        case ZB_CT_SCHLIESSFACH:
            type = "Schliessfach";
            break;
        case ZB_CT_SPEZIALTRUHE:
            type = "Spezialtruhe";
            break;
        case ZB_CT_TEST_ONLY:
            type = "Test";
            break;
        case ZB_CT_ALCH_KOFFER:
            type = "AlchKoffer";
            break;
        case ZB_CT_UNKNOWN:
            type = "Unbekannt-1(PO:"+object_name(PO)+")";
            break;
        default:
            type = "Unbekannt-2(PO:"+object_name(PO)+")";
            break;
    }
    string logline = sprintf("%Q:%s:%s: %Q=%Q\n",who,type,oknok,ob,result);
    sys_log("armatester",shorttimestr(time())+" "+logline);
    return result;
}

// ------------------------------bank_safes----------------------------------
private string * get_files(mapping vi)
{
    string * result = ({vi[ARMA_LOAD_FILE] });
    mapping sh;
    foreach (sh : vi[ARMA_SHADOW_DATA]||({}) )
    {
        result += m_indices(sh);
    }
    return result;
}

private mixed* get_object_files(mapping vi,int o_index,string itemid)
{
    ++o_index;
    mapping origin = vi[ARMA_ORIGIN_INFO];
    if (!member(origin,P_ORIGIN_UNIQUE_ID))
    {
        origin[P_ORIGIN_UNIQUE_ID] = itemid+":"+o_index;
        vi[ARMA_ORIGIN_INFO] = origin;
    }
    return ({ ({ 
        o_index,
        vi[ARMA_LOAD_FILE],
        origin[P_ORIGIN_CREATOR],
        origin[P_ORIGIN_CREATED_ON],
        origin[P_ORIGIN_UNIQUE_ID],
        }) });
}

// Einlagerungsvorgang des Inhalts eines Schliessfaches.
public int enter_bank_safe(string bank_id,string owner,string itemid,
        string* files,mapping vi,int value, 
        string factory, string identifier, mapping file_counter)
{
    string file,err;
    mapping content;
    int cnt,o_index = 0;
    mixed result;
    mixed obfiles,obfile;
    if (extern_call() && !check_security(CHECK_LAST_OBJECT)) 
    {
        debuglog(sprintf("Security Issue %Q",caller_stack(1)),
            DB_DBGLVL_WARNING,owner,"zentralbank:enter_bank_safe");
        return 0;
    }    
    while (42)
    {
        err = 0;
        if (!db_open() || query_db_error()) return 0;
        db_begin(); 
        obfiles = get_object_files(vi,&o_index,itemid);
        foreach (content : vi[ARMA_CONTENT_DATA]||({}))
        {
            obfiles += get_object_files(content,&o_index,itemid);
            files += get_files(content);
        }
        if (query_db_error()) break;
        result = db_query("INSERT INTO bank_safes "
            "(itemid,bank_id,owner,value,error_count,shorttext,"
            "factory,factory_id,paid_until,flags,"
            "item_rent,bank_rent,rent_diff) "
            "VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?)",itemid,bank_id,owner,
            value,0,vi["short"],
            factory||"",identifier||"",time(),0,1,1.0,0.0);
        if (query_db_error()) break;
        db_query("INSERT INTO bank_safe_items (itemrowid, item) VALUES (?,?)",
            result[0], file=save_value(vi,2));
        if (query_db_error()) break;

        foreach (obfile : obfiles)
        {
            db_query("INSERT OR REPLACE INTO bank_safe_origin (itemid,nr,"
                "file,creator,created_on,unique_id) VALUES (?,?,?,?,?,?)",
                itemid,obfile[0],obfile[1],obfile[2],obfile[3],obfile[4]);
            if (query_db_error()) break;
        }
        if (query_db_error()) break;
        foreach (file : files)
        {
            // ALT:
            db_query("INSERT OR REPLACE INTO bank_safe_files "
                "(itemid,file) VALUES (?,?)", itemid, file);
            if (query_db_error()) break;
            // NEU: 
            result = db_query("SELECT cnt FROM bank_safe_file_refs "
                "WHERE itemid = ?1 AND file = ?2",itemid,file);
            if (query_db_error()) break;
            if (sizeof(result))
            {
                cnt = get_one_int(result) + 1;
                db_query("UPDATE bank_safe_file_refs SET cnt = ?3 "
                    "WHERE itemid = ?1 AND file = ?2",itemid,file,cnt);
            }
            else
            {
                db_query("INSERT OR REPLACE INTO bank_safe_file_refs "
                    "(itemid,file,cnt) VALUES (?,?,?)", itemid, file,1);
            }
            if (query_db_error()) break;
        }
        if (query_db_error()) break;
        foreach (file,cnt : file_counter)
        {
            result = db_query("SELECT cnt FROM bank_safe_file_counter "
                "WHERE file = ?",file);
            if (sizeof(result))
            {
                cnt += get_one_int(result);
            }
            db_query("INSERT OR REPLACE INTO bank_safe_file_counter "
                "(file,cnt) VALUES (?1,?2) ",file,cnt);
            if (query_db_error()) break;
        }
        if (query_db_error()) break;
        db_commit();
        if (query_db_error()) break;
        db_close();
        return 1;
    }
    err ||= query_db_error();
    db_rollback();
    debuglog(err,DB_DBGLVL_ERROR,owner,"zentralbank:enter_bank_safe");
    return 0;
}

public string get_value_in_current_valuta(float amount,string valuta,
    string valutas, string v_gender)
{
    amount = convert_float(amount,"taler",valuta);
    if (amount <= 1.0)
    {
        return ein((["name":valuta,"gender":v_gender]));
    }
    else
    {
        return to_int(ceil(amount))+" "+capitalize(valutas);
    }
}

// Routine um die initialen Kosten eines soeben eingelagerten Schliessfach-
// inhalts zu berechnen.
public string update_bank_safe_initial_rent(string itemid, string bank_id,
          string owner, float bank_rent, float item_rent, int trophy_flag,
          string valuta,string valutas,string v_gender)
{
    mixed result;
    string err;
    if (extern_call() && !check_security(CHECK_LAST_OBJECT)) 
    {
        debuglog(sprintf("Security Issue %Q",caller_stack(1)),
            DB_DBGLVL_WARNING,owner,
            "zentralbank:update_bank_safe_initial_rent");
        return 0;
    }    
    result = db_query("SELECT value,flags FROM bank_safes WHERE itemid = ? "
        "AND (deleted_on IS NULL OR deleted_on = 0)",
        itemid);
    if (!sizeof(result))
    {
        debuglog("Kein Item vorhanden:"+itemid,DB_DBGLVL_ERROR,owner||TP_RN,
            "zentralbank:update_bank_safe_initial_rent");
        return 0; // interner Fehler, itemid nicht vorhanden.
    }
    if (item_rent <= 0.0)
    {
        item_rent = ceil(to_float(result[0][0])/100.0);
        if (item_rent < 0.0)
        {
            item_rent = 0.1;
        }
    }
    result = db_query("SELECT COUNT(itemid) FROM bank_safes "
        "WHERE bank_id = ?1 AND owner = ?2 "
        "AND (deleted_on IS NULL OR deleted_on = 0)",
        bank_id,owner);
    if (!sizeof(result) || get_one_int(result)<=0)
    {
        debuglog("bank_id nicht vorhanden:"+bank_id,DB_DBGLVL_ERROR,
            owner||TP_RN,
            "zentralbank:update_bank_safe_initial_rent");
        return 0; // interner Fehler, bank_id nicht vorhanden.
    }
    if (bank_rent <= 0.0)
    {
        debuglog(sprintf("bank_rent:%f %Q",bank_rent,result),
            DB_DBGLVL_ERROR,owner||TP_RN,
            "zentralbank:update_bank_safe_initial_rent");
        return 0; // Fehler beim Konvertieren??
    }
    while (42)
    {
        db_begin();
        if (err=query_db_error()) break;
        if (!trophy_flag)
            db_query("UPDATE bank_safes SET bank_rent = ?2 "
                "WHERE bank_id = ?1 AND owner = ?3 AND ((flags & ?4)=?5) "
                "AND (deleted_on IS NULL OR deleted_on = 0)",
                bank_id, bank_rent, owner,ZB_F_SAFE_TROPHY,
                trophy_flag?ZB_F_SAFE_TROPHY:0);
        if (err=query_db_error()) break;
        db_query("UPDATE bank_safes SET item_rent = ?2, flags = (flags | ?3) "
                "WHERE itemid = ?1 AND (deleted_on IS NULL OR deleted_on = 0)",
                itemid,item_rent, trophy_flag?ZB_F_SAFE_TROPHY:0);
        if (err=query_db_error()) break;
        db_commit();
        return sprintf(
"Gegenstandsmiete ca. %s, Schließfachmiete ca. %s Pro RL-Woche.",
            get_value_in_current_valuta(item_rent,valuta,valutas,v_gender),
            get_value_in_current_valuta(bank_rent,valuta,valutas,v_gender) );
    }
    db_rollback();
    debuglog(err,DB_DBGLVL_ERROR,owner||TP_RN,
        "zentralbank:update_bank_safe_initial_rent");
    return 0; // Fehler Datenbank...
}

// Liefert eine Liste von vitems, die in den Schliessfaechern von owner
// in dieser bank_id eingelagert wurden. Aufruf durch obj/schliessfaecher.
public mapping* retrieve_bank_safe(string bank_id,string owner)
{
    string err;
    mapping m,*result=({});;
    mixed r,l;
    if (extern_call() && !check_security(CHECK_LAST_OBJECT)) 
    {
        debuglog(sprintf("Security Issue %Q",caller_stack(1)),
            DB_DBGLVL_WARNING,owner,"zentralbank:retrieve_bank_safe");
        return 0;
    }    
    while (42)
    {
        err = 0;
        if (!db_open() || query_db_error()) return 0;
        r = db_query("SELECT item FROM bank_safe_items "
                          "INNER JOIN bank_safes ON bank_safes.rowid = bank_safe_items.itemrowid "
                          "WHERE bank_id = ? AND owner = ? "
                          "  AND (deleted_on IS NULL OR deleted_on = 0)"
                          "ORDER BY itemid ",bank_id,owner);
        if (query_db_error()) break;
        foreach (l: r||({}) )
        {
            m = restore_value(l[0]);
            if (mappingp(m))
                result += ({ m });
        }
        return result;
    }
    err ||= query_db_error();
    debuglog(err,DB_DBGLVL_ERROR,owner,"zentralbank:retrieve_bank_safe");
    return 0;
}

// Vor dem Zurueckholen wird geprueft, ob Objekt oder Shadow
// durch einen Alternativpfad ersetzt wurde, 0 sonst.
// Aufruf durch obj/schliessfaecher.
public string replace_bank_safe_file(string file, int flags)
{
    string q;
    mixed result;
    if (!stringp(file)) return 0;
    q = "SELECT replace_with FROM bank_safe_filter WHERE (flags & ?)>0 ";
    q+= "AND file_name = ? AND replace_with IS NOT NULL ";
    result = db_query_err(q,flags,file);
    if (sizeof(result))
    {
        return get_one_string(result);
    }
    return 0;
}

/*
FUNKTION: get_owner_stored_file_count
DEKLARATION: public int get_owner_stored_file_count(string owner, string file)
BESCHREIBUNG:
Liefert die die Summe ueber alle Dateien mit Namen 'file', entweder zum owner
bzw ueber alle owner (==0).
VERWEISE: check_conservation
GRUPPEN: handel
*/
public int get_owner_stored_file_count(string owner, string file)
{
    string q;
    q = "SELECT SUM(cnt) FROM bank_safe_file_refs ";
    q+= "WHERE file = ?1 AND itemid IN ";
    q+= "(SELECT itemid FROM bank_safes ";
    if (stringp(owner))
        q+= "WHERE owner = ?2 AND deleted_on IS NULL)";
    else
        q+= "WHERE deleted_on IS NULL)";
    mixed result = db_query_err(q,file,owner);
    return sizeof(result) ? get_one_int(result) : 0;
}

// Bankenaufsicht: Ein bank_safe_filter anzeigen
public mapping get_one_bank_safe_filter(string file)
{
    mixed result;
    string q;
    q = "SELECT file_name, flags,replace_with,hint FROM bank_safe_filter ";
    q+= "WHERE file_name = ?";
    result = db_query_err(q,file);
    if (!sizeof(result)) return 0;
    return mkmapping( 
        ({ZB_BANK_FILE,ZB_BANK_FLAGS,ZB_BANK_REPLACE_WITH,ZB_BANK_HINT}),
        result[0]);
}

// Bankenaufsicht: bank_safe_filter veraendern.
public int set_one_bank_safe_filter(string file, mapping change)
{
    if (extern_call() && !check_security(CHECK_LAST_OBJECT)) 
    {
        debuglog(sprintf("Security Issue %Q",caller_stack(1)),
            DB_DBGLVL_WARNING,TP_RN,"zentralbank:set_one_bank_safe_filter");
        return 0;
    }   
    if (!mappingp(change)) 
        return 0;
    mapping old = get_one_bank_safe_filter(file);
    if (!mappingp(old))
    {
        if (!change[ZB_BANK_FILE])
        {
            return 0; // Primaerschluessel fehlt.
        }
        db_query("INSERT INTO bank_safe_filter "
            "(file_name, flags,replace_with) VALUES (?,?,?)",
            change[ZB_BANK_FILE],change[ZB_BANK_FLAGS],
            change[ZB_BANK_REPLACE_WITH]||"");
        if (query_db_error())
        {
            return 0;// duplicate error?
        }
        return 1;
    }
    int count = 0;
    string q = "";
    mixed *args=({});
    if (member(change,ZB_BANK_FILE) && change[ZB_BANK_FILE]!=file)
    {
        q+= (count++ ?",":"");
        q+= "file_name = ? ";
        args += ({change[ZB_BANK_FILE]});
    }
    if (member(change,ZB_BANK_FLAGS) 
        && change[ZB_BANK_FLAGS]!=old[ZB_BANK_FLAGS])
    {
        q+= (count++ ?",":"");
        q+= "flags = ? ";
        args += ({change[ZB_BANK_FLAGS]});
    }
    if (member(change,ZB_BANK_REPLACE_WITH) 
        && change[ZB_BANK_REPLACE_WITH]!=old[ZB_BANK_REPLACE_WITH])
    {
        q+= (count++ ?",":"");
        q+= "replace_with = ? ";
        args += ({change[ZB_BANK_REPLACE_WITH]});
    }
    if (member(change,ZB_BANK_HINT) 
        && change[ZB_BANK_HINT]!=old[ZB_BANK_HINT])
    {
        q+= (count++ ?",":"");
        if (change[ZB_BANK_HINT]=="") 
        {
            q+= "hint = NULL ";
        } 
        else 
        {
            q+= "hint = ? ";
            args += ({ change[ZB_BANK_HINT] });
        }
    }
    if (count)
    {
        args += ({ file });
        db_query("UPDATE bank_safe_filter SET "+q
            +" WHERE file_name = ?",args...);
        if (query_db_error())
        {
            return 0;// duplicate key?
        }
        return 1; // ok
    }
    return 1; // no change...
}

// Nach dem Auslagern in ein offenes Schliessfach werden die zugehoerigen
// Daten geloescht. Aufurf durch obj/schliessfacher
public int leave_bank_safes(string itemid, string owner,mapping file_counter)
{
    string err,file;
    int cnt;
    mixed result;
    if (extern_call() && !check_security(CHECK_LAST_OBJECT)) 
    {
        debuglog(sprintf("Security Issue %Q",caller_stack(1)),
            DB_DBGLVL_WARNING,owner,"zentralbank:leave_bank_safes");
        return 0;
    }    
    db_query_err("UPDATE bank_safes SET deleted_on = ?2 WHERE itemid = ?1 "
                 "AND (deleted_on IS NULL OR deleted_on = 0)",itemid,time());
    return 1;
    while (42) // recycle for final delete...
    {
        err = 0;
        if (!db_open() || query_db_error()) return 0;
        db_begin(); 
        if (query_db_error()) break;
        db_query("DELETE FROM bank_safe_file_refs WHERE itemid = ? ",itemid);
        if (query_db_error()) break;
        db_query("DELETE FROM bank_safe_files WHERE itemid = ? ",itemid);
        if (query_db_error()) break;
        db_query("DELETE FROM bank_safe_items WHERE itemrowid = (SELECT rowid FROM bank_safes WHERE itemid = ?)",itemid);
        if (query_db_error()) break;
        db_query("DELETE FROM bank_safes WHERE itemid = ?",itemid);
        if (query_db_error()) break;
        foreach (file,cnt : file_counter)
        {
            result = db_query("SELECT cnt FROM bank_safe_file_counter "
                "WHERE file = ?",file);
            if (sizeof(result))
            {
                cnt = get_one_int(result) - cnt;
            }
            else
            {
                cnt = -1;
            }
            if (cnt>0)
                db_query("INSERT OR REPLACE INTO bank_safe_file_counter "
                    "(file,cnt) VALUES (?1,?2) ",file,cnt);
            else
                db_query("DELETE FROM bank_safe_file_counter "
                    "WHERE file = ?1 ",file);
            if (query_db_error()) break;
        }       
        if (query_db_error()) break;
        db_commit();
        if (query_db_error()) break;
        db_close();
        return 1;
    }
    err ||= query_db_error();
    db_rollback();
    debuglog(err,DB_DBGLVL_ERROR,owner,"zentralbank:leave_bank_safes");
    return 0;
}

public int leave_from_all_banks(string file)
{
    if (extern_call() && !check_security(CHECK_LAST_OBJECT)) 
    {
        debuglog(sprintf("Security Issue %Q",caller_stack(1)),
            DB_DBGLVL_WARNING,TP_RN||file,"zentralbank:leave_from_all_banks");
        return 0;
    }
    if (!stringp(file) || strstr(file,"/obj/")!=-1)
    {
        debuglog(sprintf("not a touch object %Q %Q",file,caller_stack(1)),
            DB_DBGLVL_WARNING,TP_RN||file,"zentralbank:leave_from_all_banks");
        return 0;
    }
    db_query_err("UPDATE bank_safes SET deleted_on = ?2 "
                 "WHERE itemid IN (SELECT DISTINCT itemid "
                 "FROM bank_safe_origin WHERE file = ?1) "
                 "AND (deleted_on IS NULL OR deleted_on = 0)",
                 file,time()); 
    return 1;// TODO look into leave_bank_safes beyond return 1;
}

// soll das globale Konto pruefen und verrechnen
private int check_global_konto(<object|string> pl, float sum)
{
    int delta = to_int(floor(sum));
    int konto;
    pl = stringp(pl) ? (find_player(pl)||pl) : pl;
    if (playerp(pl))
    {
        konto = pl->query_konto();
    }
    else if (stringp(pl))
    {
        return 0; // konto = "/secure/player_reader"->query(pl,"konto");
    }
    else 
    {
        return 0;
    }
    if (-delta > konto)
        return -1; // Konto hat keine Deckung mehr
    if (playerp(pl))
    {
        pl->add_konto(delta);
    }
    else
    {
        // "/secure/player_modifier"->add_konto(pl,delta);
        // TODO mudlib priviledge offline_add_konto
        db_query_err("UPDATE bank_customers "
            "SET offline_rent = offline_rent + ?2 "
            "WHERE realname = ?1",pl,-delta);
    }
    return 1; // Konto wurde verrechnet.
}

// Aktualisiert die Miete fuer einen Spieler,, evtl nur pro bank 
// oder Schliessfach
public varargs int update_rent_for_safes(<object|string> pl, 
        string bank_id, string itemid)
{
    string owner,err;
    mixed result;
    int choice;
    if (stringp(pl)) // TODO security??
    {
        if (!player_exists(pl))
        {
            return 0;
        }
        owner = pl;
        pl = find_player(owner);
    }
    else if (playerp(pl))
    {
        owner = pl->query_real_name();
    }
    else
    {
        return 0;
    }
    check_and_update_player(owner);
  while(42)
  {
    db_begin();
    if (stringp(itemid))
    {
        db_query("UPDATE bank_safes SET rent_diff = rent_diff - "
          "((bank_rent+item_rent)*MIN(?1 - paid_until,?3)*1.0/604800.0), "
          "paid_until = ?1 WHERE itemid = ?2 "
          "AND (deleted_on IS NULL OR deleted_on = 0)",
          time(),itemid,MAX_MIET_DAUER);
        if (err = query_db_error()) break;
        result = db_query("SELECT rent_diff FROM bank_safes WHERE itemid = ?",
            itemid);
        if ((err = query_db_error()) || sizeof(result)==0) break;
        choice = check_global_konto(pl||owner, result[0][0]);
        switch(choice)
        {
        case 1:
            db_query("UPDATE bank_safes SET rent_diff = 0.0, "
                "flags = (flags & ?2) WHERE itemid = ?1 AND "
                "(deleted_on IS NULL OR deleted_on = 0)",
                itemid,~ZB_F_SAFE_LOCKED);
            if (err = query_db_error()) break;
            db_commit();
            return -(to_int(floor(result[0][0]))||1);
        case -1:
            db_query("UPDATE bank_safes SET flags = (flags | ?1) "
                "WHERE itemid = ?2 AND (deleted_on IS NULL OR deleted_on = 0)",
                ZB_F_SAFE_LOCKED,itemid);
            if (err = query_db_error()) break;
            db_commit();
            return to_int(floor(result[0][0]))||1;
        default:
            err = "Update Konto (Schliessfach) fehlgeschlagen.";
            break;
        }
        break;
    }
    else if (stringp(bank_id))
    {
        db_query("UPDATE bank_safes SET rent_diff = rent_diff - "
          "((bank_rent+item_rent)*MIN(?1 - paid_until,?4)*1.0/604800.0), "
          "paid_until = ?1 WHERE bank_id = ?2 AND owner = ?3 "
          "AND (deleted_on IS NULL OR deleted_on = 0)",
          time(),bank_id,owner,MAX_MIET_DAUER);
        if (err = query_db_error()) break;
        result = db_query("SELECT SUM(rent_diff) FROM bank_safes "
            "WHERE bank_id = ?1 AND owner = ?2 "
            "AND (deleted_on IS NULL OR deleted_on = 0)", bank_id, owner);
        if (err = query_db_error()) break;
        choice = check_global_konto(pl||owner, result[0][0]);
        switch(choice)
        {
        case 1:
            db_query("UPDATE bank_safes SET rent_diff = 0.0, "
                "flags = (flags & ?3) WHERE bank_id = ?1 AND owner = ?2 "
                "AND (deleted_on IS NULL OR deleted_on = 0)",
                bank_id,owner,~ZB_F_SAFE_LOCKED);
            if (err = query_db_error()) break;
            db_commit();
            return -(to_int(floor(result[0][0]))||1);
        case -1:
            db_query("UPDATE bank_safes SET flags = (flags | ?1) "
                "WHERE bank_id = ?2 AND owner = ?3 "
                "AND (deleted_on IS NULL OR deleted_on = 0) ",
                ZB_F_SAFE_LOCKED,bank_id,owner);
            if (err = query_db_error()) break;
            db_commit();
            return to_int(floor(result[0][0]))||1;
        default:
            err = "Update Konto (Bank) fehlgeschlagen.";
            break;
        }
        break;
    }
    else
    {
        db_query("UPDATE bank_safes SET rent_diff = rent_diff - "
          "((bank_rent+item_rent)*MIN(?1 - paid_until,?3)*1.0/604800.0), "
          "paid_until = ?1 WHERE owner = ?2 "
          "AND (deleted_on IS NULL OR deleted_on = 0) ",
          time(),owner,MAX_MIET_DAUER);
        if (err = query_db_error()) break;
        result = db_query("SELECT SUM(rent_diff) FROM bank_safes "
            "WHERE owner = ? AND (deleted_on IS NULL OR deleted_on = 0)", 
            owner);
        if (err = query_db_error()) break;
        choice = check_global_konto(pl||owner, result[0][0]);
        switch(choice)
        {
        case 1:
            db_query("UPDATE bank_safes SET rent_diff = 0.0, "
                "flags = (flags & ?2) WHERE owner = ?1 "
                "AND (deleted_on IS NULL OR deleted_on = 0) ",
                owner,~ZB_F_SAFE_LOCKED);
            if (err = query_db_error()) break;
            db_commit();
            return -(to_int(floor(result[0][0]))||1);
        case -1:
            db_query("UPDATE bank_safes SET flags = (flags | ?1) "
                "WHERE owner = ?2 AND (deleted_on IS NULL OR deleted_on = 0) ",
                ZB_F_SAFE_LOCKED,owner);
            if (err = query_db_error()) break;
            db_commit();
            return to_int(floor(result[0][0]))||1;
        default:
            err = "Update Konto (alle Banken) fehlgeschlagen.";
            break;
        }
        break;
    }
  }
  db_rollback();
  debuglog("DB-Error: "+err,DB_DBGLVL_ERROR,owner,
        "zentralbank:update_rent_for_safes");
    return 0;
}

// Pruefung, ob Schliessfaecher einer Bank wegen fehlender Miete gesperrt ist.
// TODO Ueberarbeitung check_bank_safe_locks
public string check_bank_safe_locks(string bank_id, <object|string> pl)
{
    mixed result;
    string owner,qt;
    owner = playerp(pl) ? pl->query_real_name() : pl;
    if (!bank_id)
    {
        qt = " FROM bank_safes WHERE owner = ?2 "
             "AND (deleted_on IS NULL OR deleted_on = 0) ";
        result = db_query("SELECT "
            "(SELECT COUNT(itemid)"+qt+"), "
            "(SELECT COUNT(itemid)"+qt+"AND (flags & ?1)>0) ",
            ZB_F_SAFE_LOCKED,owner);
        if (!sizeof(result))
            return 0;
        result = select_row(result);
        if (result[1]>0)
        {
            if (result[1] >= result[0])
            {
                return sprintf("Alle %d Schließfächer sind gesperrt.",
                    result[1]);
            }
            else
            {
                return sprintf("%d von %d Schließfächer sind gesperrt.",
                    result[1],result[0]);
            }
        }
        return 0;
    }
    else // bank_id
    {
        qt = " FROM bank_safes WHERE owner = ?2 AND bank_id = ?3 "
             "AND (deleted_on IS NULL OR deleted_on = 0) ";
        result = db_query("SELECT "
            "(SELECT COUNT(itemid)"+qt+"), "
            "(SELECT COUNT(itemid)"+qt+"AND (flags & ?1)>0) ",
            ZB_F_SAFE_LOCKED,owner,bank_id);
        if (!sizeof(result))
            return 0;
        result = select_row(result);
        if (result[1]>0)
        {
            if (result[1] >= result[0])
            {
                return sprintf(
                    "Alle %d Schließfächer dieser Bank sind gesperrt.",
                    result[1]);
            }
            else
            {
                return sprintf(
                    "%d von %d Schließfächer dieser Bank sind gesperrt.",
                    result[1],result[0]);
            }
        }
        return 0;
    }
}

// Pruefen, ob ein Schliessfach gesperrt ist.
public int check_bank_safe_lock(string itemid)
{
    if (!stringp(itemid))
        return 0;
    mixed result = db_query("SELECT itemid FROM bank_safes "
        "WHERE itemid = ?1 AND (flags & ?2) > 0 "
        "AND (deleted_on IS NULL OR deleted_on = 0) ", 
        itemid, ZB_F_SAFE_LOCKED);
    return sizeof(result) ? 1: 0;
}

// Interne Hilfsfunktion zum filtern und vereinfachen der Abfrage.
private mixed bsf_filter_and_unify(mixed result, string output_option)
{
    mixed erg = ({});
    int i;
    mapping index = ([
        ZB_BANK_FILE: 0,
        ZB_BANK_ITEMID: 1,
        ZB_BANK_ID: 2,
        ZB_BANK_OWNER: 3,
        ]);
    if (!member(index,output_option))
    {
        return result;
    }
    i = index[output_option];
    foreach (mixed element : result)
    {
        if (member(erg, element[i]) < 0) 
        {
            erg += ({ element[i] });
        }
    }
    return erg;
}

// Bankenaufsicht: Anzeige der eingelagerten Schliessfaecher
// mit unterschiedlichen Optionen.
public mixed retrieve_bank_safe_files(mapping options, int countflag)
{
    string q;
    mixed *erg=({});
    mixed result,*args=({});
    if (extern_call() && !check_security(CHECK_LAST_OBJECT)) 
    {
        debuglog(sprintf("Security Issue %Q",caller_stack(1)),
            DB_DBGLVL_WARNING,TP_RN,"zentralbank:retrieve_bank_safe_files");
        return 0;
    }
    if (member(options,ZB_BANK_ITEMID))
        q = "SELECT bsf.file, bsf.itemid, bs.bank_id, bs.owner, "
                   "bsi.item, bs.error_count, bs.deleted_on "
            "FROM bank_safes bs "
            "INNER JOIN bank_safe_files bsf ON bsf.itemid = bs.itemid "
            "INNER JOIN bank_safe_items bsi ON bsi.itemrowid = bs.rowid ";
    else
        q = "SELECT bsf.file, bsf.itemid, bs.bank_id, bs.owner,"
            "0, bs.error_count, bs.deleted_on "
            "FROM bank_safes bs,bank_safe_files bsf "
            "WHERE bsf.itemid = bs.itemid ";
    if (member(options,ZB_DELETED_ON))
    {
        q+= "AND bs.deleted_on > 0 ";
    }
    else
    {
        q+= "AND (bs.deleted_on IS NULL OR bs.deleted_on = 0) ";
    }
    if (options[ZB_BANK_FILE_PATTERN])
    {
        q+= "AND bs.bank_id IN (SELECT DISTINCT bank_id FROM bank_files ";
        q+= "WHERE filename LIKE ? ";
        args += ({options[ZB_BANK_FILE_PATTERN]});
    }
    if (member(options,ZB_BANK_ID))
    {
        q+= "AND bs.bank_id = ? ";
        args += ({options[ZB_BANK_ID]});
    }
    if (member(options,ZB_BANK_OWNER))
    {
        q+= "AND bs.owner = ? ";
        args += ({options[ZB_BANK_OWNER]});
    }
    if (member(options,ZB_BANK_ITEMID))
    {
        q+= "AND bsf.itemid = ? ";
        args += ({options[ZB_BANK_ITEMID]});
    }
    if (options[ZB_BANK_WITH_ERROR])
    {
        q+= "AND bs.error_count > 0 ";
    }
    if (!countflag)
        switch (options[ZB_BANK_SORTBY])
        {
            default:
            case ZB_BANK_FILE: q+= "ORDER BY bsf.file ";break;
            case ZB_BANK_ID: q+= "ORDER BY bs.bank_id ";break;
            case ZB_BANK_ITEMID: q+= "ORDER BY bsf.itemid ";break;
            case ZB_BANK_OWNER: q+= "ORDER BY bs.owner ";break;
            case ZB_BANK_WITH_ERROR: q+= "ORDER BY bs.error_count DESC ";break;
        }
    result = db_query(q,args...);
    if (member(options,ZB_BANK_WIZARD))
    {
        if (!wizp(options[ZB_BANK_WIZARD]))
        {
            return 0;
        }
        if (!adminp(options[ZB_BANK_WIZARD]))
        {
            result = filter(result,(: MAY_WRITE($1[1], $2) :),
                options[ZB_BANK_WIZARD]);
        }
        erg = bsf_filter_and_unify(result,options[ZB_BANK_OUTPUT]);
    }
    else
    {
        erg = bsf_filter_and_unify(result,options[ZB_BANK_OUTPUT]);
    }
    if (countflag)
    {
        return sizeof(erg);
    }
    if (sizeof(erg)==0)
    {
        return ({});
    }
    if (member(options,DB_DBG_OFFSET))
    {
        int von = options[DB_DBG_OFFSET];
        int bis = von + options[DB_DBG_LIMIT];
        return erg[von..bis];
    }
    return erg;
}

public varargs mapping retrieve_bank_unique_item(string unique_id,string itemid)
{
    string q;
    mixed r1;
    mapping result = ([]),item;
    mixed *args = ({unique_id});
    int nr;

    q = "SELECT itemid,nr,file,creator,created_on,unique_id ";
    q+= "FROM bank_safe_origin WHERE unique_id = ? ";
    if (stringp(itemid))
    {
        q+= "AND itemid = ? ";
        args += ({ itemid });
    }
    else
    {
        q+= "AND itemid IN (SELECT itemid FROM bank_safes "
            "WHERE deleted_on IS NULL)";
    }
    r1 = db_query_err(q, args...);
    if (sizeof(r1)!=1) 
        return ([]);
    else
        r1 = r1[0];
    itemid = result[ZB_BANK_ITEMID] = r1[0];
    nr = r1[1];
    result[ZB_BANK_FILE] = r1[2];
    result[ZB_BANK_CREATOR] = r1[3];
    result[ZB_BANK_CREATED_ON] = r1[4];
    result[ZB_BANK_UNIQUE_ID] = r1[5];
    r1 = db_query_err("SELECT item FROM bank_safe_items "
                      "    INNER JOIN bank_safes ON bank_safe_items.itemrowid = bank_safes.rowid "
                      "    WHERE itemid = ? ",itemid);
    if (sizeof(r1)!=1) 
        return ([]);
    item = restore_value(r1[0][0]);
    if (nr <= 1)
    {
        result[ZB_BANK_ITEM] = item;
        return result;
    }
    nr -= 2;
    if (nr >= sizeof(item[ARMA_CONTENT_DATA]) )
        return ([]);
    result[ZB_BANK_ITEM] = item[ARMA_CONTENT_DATA][nr];
    return result;
}

// Bankaufsicht: alle Einlagernden Owner aufzeigen.
public <int|string*> retrieve_bank_safes_by_owner(mapping options, int countflag)
{
    string q;
    mixed result,*args=({});
    if (extern_call() && !check_security(CHECK_LAST_OBJECT)) 
    {
        debuglog(sprintf("Security Issue %Q",caller_stack(1)),
            DB_DBGLVL_WARNING,TP_RN,"zentralbank:retrieve_bank_safes_by_owner");
        return 0;
    }
    if (countflag)
        q = "SELECT COUNT(DISTINCT bs.owner) FROM bank_safes bs ";
    else
        q = "SELECT DISTINCT bs.owner FROM bank_safes bs ";
    q+= "WHERE (bs.deleted_on IS NULL OR bs.deleted_on = 0) ";
    if (!countflag)
    {
        q+= "ORDER BY bs.owner ";
        q+= "LIMIT ? OFFSET ? ";
        args += ({options[DB_DBG_LIMIT],options[DB_DBG_OFFSET] });
    }
    result = db_query(q,args...);
    if (!result || !sizeof(result))
        return 0;
    if (countflag)
    {
        return get_one_int(result);
    }
    return map(result, (: $1[0] :) );
}

// Bankenaufsicht: einlagernde Dateien
public <int|string*> retrieve_bank_safe_by_files(mapping options, int countflag)
{
    string q;
    mixed result,*args=({});
    int whereflag = 0;
    if (extern_call() && !check_security(CHECK_LAST_OBJECT)) 
    {
        debuglog(sprintf("Security Issue %Q",caller_stack(1)),
            DB_DBGLVL_WARNING,TP_RN,"zentralbank:retrieve_bank_safe_by_files");
        return 0;
    }
    if (countflag)
        q = "SELECT COUNT(DISTINCT bsf.file) "
            "FROM bank_safe_files bsf ";
    else
        q = "SELECT DISTINCT bsf.file "
            "FROM bank_safe_files bsf ";
    if (member(options,ZB_BANK_ITEMID))
    {
        q+= (whereflag++?"AND ":"WHERE ");
        q+= "AND bsf.itemid = ? ";
        args += ({ options[ZB_BANK_ITEMID] });
    }
    if (1 || options[ZB_BANK_FILE_PATTERN] || 
        sizeof(m_indices(options)
            &({ZB_BANK_ID,ZB_BANK_OWNER,ZB_BANK_WITH_ERROR}))>0)
    {
        int where2nd = 1;
        q+= (whereflag++?"AND ":"WHERE ");
        q+= "bsf.itemid IN (SELECT bs.itemid FROM bank_safes bs ";
        q+= "WHERE (bs.deleted_on IS NULL OR bs.deleted_on = 0) ";
        if (options[ZB_BANK_FILE_PATTERN])
        {
            q+= (where2nd++?"AND ":"WHERE ");
            q+= "bs.bank_id IN (SELECT DISTINCT bank_id FROM bank_files ";
            q+= "WHERE filename LIKE ? ";
            args += ({ options[ZB_BANK_FILE_PATTERN] });
        }
        if (member(options,ZB_BANK_ID))
        {
            q+= (where2nd++?"AND ":"WHERE ");
            q+= "bs.bank_id = ? ";
            args += ({ options[ZB_BANK_ID] });
        }
        if (member(options,ZB_BANK_OWNER))
        {
            q+= (where2nd++?"AND ":"WHERE ");
            q+= "bs.owner = ? ";
            args += ({ options[ZB_BANK_OWNER] });
        }
        if (options[ZB_BANK_WITH_ERROR])
        {
            q+= (where2nd++?"AND ":"WHERE ");
            q+= "bs.error_count > 0 ";
        }
        q+= ") ";
    }
    if (!countflag)
    {
        q+= "ORDER BY bsf.file ";
        q+= "LIMIT ? OFFSET ? ";
        args += ({options[DB_DBG_LIMIT],options[DB_DBG_OFFSET] });
    }
    result = db_query_err(q,args...);
    if (!result || !sizeof(result))
        return 0;
    if (countflag)
    {
        return get_one_int(result);
    }
    return map(result, (: $1[0] :) );
}

// Bankenaufsicht: die eingelargten Gegenstaende nach itemids...
public <int|string*> retrieve_bank_safe_item_ids(mapping options, int countflag)
{
    string q;
    mixed result,*args=({});
    int whereflag = 0;
    if (extern_call() && !check_security(CHECK_LAST_OBJECT)) 
    {
        debuglog(sprintf("Security Issue %Q",caller_stack(1)),
            DB_DBGLVL_WARNING,TP_RN,"zentralbank:retrieve_bank_safe_item_ids");
        return 0;
    }
    if (countflag)
        q = "SELECT COUNT(bs.itemid) FROM bank_safes bs ";
    else
        q = "SELECT bs.itemid,bs.shorttext FROM bank_safes bs ";
    whereflag = 1;
    q+= "WHERE (bs.deleted_on IS NULL OR bs.deleted_on = 0) ";
    if (options[ZB_BANK_FILE_PATTERN])
    {
        q+= (whereflag++?"AND ":"WHERE ");
        q+= "bs.bank_id IN (SELECT DISTINCT bank_id FROM bank_files ";
        q+= "WHERE filename LIKE ? ) ";
        args += ({ options[ZB_BANK_FILE_PATTERN] });
    }
    if (options[ZB_BANK_FILE])
    {
        q+= (whereflag++?"AND ":"WHERE ");
        q+= "bs.itemid IN (SELECT itemid FROM bank_safe_files ";
        q+= "WHERE file = ? ) ";
        args += ({ options[ZB_BANK_FILE] });
    }
    if (member(options,ZB_BANK_ID))
    {
        q+= (whereflag++?"AND ":"WHERE ");
        q+= "bs.bank_id = ? ";
        args += ({ options[ZB_BANK_ID] });
    }
    if (member(options,ZB_BANK_OWNER))
    {
        q+= (whereflag++?"AND ":"WHERE ");
        q+= "bs.owner = ? ";
        args += ({ options[ZB_BANK_OWNER] });
    }
    if (options[ZB_BANK_WITH_ERROR])
    {
        q+= "AND bs.error_count > 0 ";
    }
    if (!countflag)
    {
        switch (options[ZB_BANK_SORTBY])
        {
            default:
            case ZB_BANK_ID: q+= "ORDER BY bs.bank_id,bs.itemid ";break;
            case ZB_BANK_ITEMID: q+= "ORDER BY bs.itemid ";break;
            case ZB_BANK_OWNER: q+= "ORDER BY bs.owner,bs.itemid ";break;
            case ZB_BANK_WITH_ERROR: q+= "ORDER BY bs.error_count DESC ";break;
        }
        q+= "LIMIT ? OFFSET ? ";
        args += ({options[DB_DBG_LIMIT],options[DB_DBG_OFFSET] });
    }
    result = db_query_err(q,args...);
    if (!result || !sizeof(result))
        return 0;
    if (countflag)
    {
        return get_one_int(result);
    }
    return map(result, (: $1[0] + " "+$1[1] :) );
}

// Bankenaufsicht: Banken mit eingelagerten Dateien
public <int|string*> retrieve_banks_with_safe_files(mapping options, int countflag)
{
    string q;
    mixed result,*args=({});
    int whereflag = 0;
    if (extern_call() && !check_security(CHECK_LAST_OBJECT)) 
    {
        debuglog(sprintf("Security Issue %Q",caller_stack(1)),
            DB_DBGLVL_WARNING,TP_RN,"zentralbank:retrieve_banks_with_safe_files");
        return 0;
    }
    if (countflag)
    {
         q = "SELECT COUNT(DISTINCT bs.bank_id) FROM bank_safes bs ";
    }
    else
    {
        q = "SELECT bs.bank_id,COUNT(bs.itemid) FROM bank_safes bs ";
    }
    whereflag = 1;
    q+= "WHERE (bs.deleted_on IS NULL OR bs.deleted_on = 0) ";
    if (options[ZB_BANK_FILE_PATTERN])
    {
        q+= (whereflag++?"AND ":"WHERE ");
        q+= "bs.bank_id IN (SELECT DISTINCT bank_id FROM bank_files ";
        q+= "WHERE filename LIKE ? ) ";
        args += ({ options[ZB_BANK_FILE_PATTERN] });
    }
    if (!countflag && member(options,DB_DBG_LIMIT))
    {
        q+= "GROUP BY bs.bank_id ORDER BY bs.bank_id ";
        q+= "LIMIT ? OFFSET ? ";
        args += ({options[DB_DBG_LIMIT],options[DB_DBG_OFFSET] });
    }
    result = db_query_err(q,args...);
    if (!result || !sizeof(result))
        return 0;
    if (countflag)
    {
        return get_one_int(result);
    }
    if (options[ZB_BANK_OUTPUT])
        return map(result, (: sprintf("%s (%d)",$1[0],$1[1]) :) );
    else
        return map(result, (: $1[0] :) );
}
public mapping get_extra_info_from_bank_safe(string itemid)
{
    mixed r1 = db_query_err("SELECT item FROM bank_safe_items "
                                "INNER JOIN bank_safes ON bank_safes.rowid = bank_safe_items.itemrowid "
                                "WHERE itemid = ?", itemid);
    if (!sizeof(r1)) return 0;
    mapping vi = restore_value(r1[0][0]);
    return filter(vi, (: strstr($1,"Root:Arma")==0 :));
}

public string* get_files_from_bank_safe(string itemid)
{
    mapping vi = get_extra_info_from_bank_safe(itemid),content;
    string * files;
    files = ({vi[ARMA_LOAD_FILE]});
    files += map(vi[ARMA_SHADOW_DATA], (: m_indices($1)[0] :) );
    foreach (content:vi[ARMA_CONTENT_DATA])
    {
        files += ({content[ARMA_LOAD_FILE]});
        files += map(content[ARMA_SHADOW_DATA], (: m_indices($1)[0] :) );
    }
    return files;
}

// mietuebersicht: fach detailanzeige.
public <int|string*> get_fach_detailanzeige(mapping options,int countflag)
{
    string q,q2="",bank;
    mixed r1;
    mapping iteminfo;
    int i;
    if (countflag)
        q = "SELECT COUNT(DISTINCT bs.itemid) ";
    else
        q = "SELECT b.short_title,bs.shorttext,bs.itemid,0 ";
    q+= "FROM banks b,bank_safes bs ";
    q+= "WHERE b.bank_id = bs.bank_id ";
    q+= "AND (bs.deleted_on IS NULL OR bs.deleted_on = 0) ";
    q+= "AND bs.owner = ?1 ";
    if (!countflag && member(options,DB_DBG_LIMIT))
    {
        q2  = "ORDER BY b.short_title,bs.itemid LIMIT 10000 ";
    }
    r1 = db_query_err(q+q2,options[ZB_BANK_OWNER]||"");
    if (countflag)
    {
        return get_one_int(r1);
    }
    bank = "";
    r1 = map(r1||({}),function mixed* (mixed* m) {
        if (bank != m[0]) 
        {
            bank = m[0];
            i = 0;
        }
        m[3] = ++i;
        return m;
    } );
    if (options[ZB_BANK_SORTBY]!= ZB_BANK_HINT)
    {
        r1 = map(r1||({}),function string (mixed* m) {
            return sprintf("%s,%02d: %s",m[0],m[3],m[1]);
        } );
        i = options[DB_DBG_OFFSET]+options[DB_DBG_LIMIT];
        return r1 [options[DB_DBG_OFFSET]..i];
    }
    r1 = transpose_array(r1);
    iteminfo = mkmapping(r1[2],r1[3]); // itemid : lfdnr.
    q2 = "ORDER BY bs.shorttext,b.short_title,bs.itemid ";
    q2+= "LIMIT "+options[DB_DBG_LIMIT]+" OFFSET "
        +options[DB_DBG_OFFSET];
    r1 = db_query_err(q+q2,options[ZB_BANK_OWNER]||"");
    return map(r1||({}),function string (mixed* m) {
        return sprintf("%s,%02d: %s",
            m[0],iteminfo[m[2]],m[1]);
    });
}


// Bankenaufsicht: Statistik ueber eingelagerrte Dateien und Shadows
// alt und falsch...
public <int|string*> get_file_statistik_alt0(mapping options,int countflag)
{
    string q;
    mixed result,*args=({});
    int whereflag = 0;
    if (countflag)
    {
        q= "SELECT COUNT(file) FROM bank_safe_file_counter ";
    }
    else
    {
        q= "SELECT file,cnt FROM bank_safe_file_counter ";
    }
    if (options[ZB_BANK_FILE_PATTERN])
    {
        q+= (whereflag++?"AND ":"WHERE ");
        q+= "file LIKE ? ";
        args+= ({options[ZB_BANK_FILE_PATTERN]});
    }
    if (!countflag && member(options,DB_DBG_LIMIT))
    {
        q+= "ORDER BY cnt DESC ";
        q+= "LIMIT ? OFFSET ? ";
        args += ({options[DB_DBG_LIMIT],options[DB_DBG_OFFSET] });
    }
    result = db_query_err(q,args...);
    if (!result) return 0;
    if (countflag) return get_one_int(result);
    return map(result,function string(mixed arr) {
        return sprintf("%3d %s",arr[1],arr[0]);
    });
}

public <int|string*> get_owner_statistik(mapping options,int countflag)
{
    string q;
    mixed result,*args=({});
    if (countflag)
    {
        q= "SELECT COUNT(DISTINCT owner) FROM bank_safes ";
    }
    else
    {
        q= "SELECT owner,COUNT(DISTINCT bank_id),COUNT(itemid),"
           "SUM(item_rent)+SUM(bank_rent) FROM bank_safes ";
    }
    q+= "WHERE (deleted_on IS NULL OR deleted_on = 0) ";
    if (!countflag && member(options,DB_DBG_LIMIT))
    {
        q+= "GROUP BY owner ";
        q+= "ORDER BY owner ";
        q+= "LIMIT ? OFFSET ? ";
        args += ({options[DB_DBG_LIMIT],options[DB_DBG_OFFSET] });
    }
    result = db_query_err(q,args...);
    if (!result) return 0;
    if (countflag) return get_one_int(result);
    return map(result,function string(mixed arr) {
        return sprintf("%-12s %02d %03d %.1f",arr[0],arr[1],arr[2],arr[3]);
    });
}

// Bankenaufsicht: Statistik ueber eingelagerrte Dateien und Shadows(neu)
public <int|string*> get_file_statistik(mapping options,int countflag)
{
    string q;
    mixed r,*args=({});
    if (countflag)
        q = "SELECT COUNT(DISTINCT file) FROM bank_safe_file_refs ";
    else
        q = "SELECT file,SUM(cnt) AS scnt FROM bank_safe_file_refs ";
    q+= "WHERE itemid IN (SELECT itemid FROM bank_safes ";
    q+= "WHERE (deleted_on IS NULL OR deleted_on = 0)) ";
    if (options[ZB_BANK_FILE_PATTERN])
    {
        q+= "AND file LIKE ? ";
        args += ({ options[ZB_BANK_FILE_PATTERN] });
    }
    if (!countflag && member(options,DB_DBG_LIMIT))
    {
        q+= "GROUP BY file ";
        q+= "ORDER BY scnt DESC ";
        q+= "LIMIT ? OFFSET ? ";
        args += ({options[DB_DBG_LIMIT],options[DB_DBG_OFFSET] });
    }
    r = db_query_err(q,args...);
    if (!r) return 0;
    if (countflag) return get_one_int(r);
    return map(r,function string(mixed arr) {
        return sprintf("%3d %s",arr[1],arr[0]);
    });
}

public string* get_schliessfach_uebersicht()
{
    string q,* lines;
    mixed r;
    
    q = "SELECT COUNT(itemid),COUNT(DISTINCT bank_id),COUNT(DISTINCT owner), ";
    q+= "SUM(item_rent),SUM(bank_rent) FROM bank_safes ";
    q+= "WHERE (deleted_on IS NULL OR deleted_on = 0) ";
    r = db_query_err(q);
    if (!sizeof(r)) return ({"Keine Anzeige"});
    r = r[0];
    lines = ({ sprintf("%d Fächer in %d Banken mit insgesamt %d Eigentümern",
            r[0],r[1],r[2]) });
    lines+= ({ sprintf("Gesamt-Miete Gegenstände: %.1f bzw. Fächer: %.1f",
        r[3],r[4]) });
    q = "SELECT COUNT(DISTINCT itemid), COUNT(file), SUM(cnt) ";
    q+= "FROM bank_safe_file_refs ";
    q+= "WHERE itemid IN (SELECT itemid FROM bank_safes ";
    q+= "WHERE (deleted_on IS NULL OR deleted_on = 0)) ";
    r = db_query_err(q);
    if (!sizeof(r)) return lines;
    r = r[0];
    lines+= ({sprintf("Bei %d Fächer insgesamt %d Dateien mit Zähler %d",
        r[0],r[1],r[2]) });
    lines+= ({sprintf("Quote %.2f Dateien pro Fach.",
        to_float(r[2])/to_float(r[0]) ) });
    return lines;
}

public <int|string*> get_load_file_statistik(mapping options,int countflag)
{
    string q;
    mixed r,*args=({});
    if (countflag)
        q = "SELECT COUNT(DISTINCT loadfile_creator) FROM bank_safe_lfc ";
    else
        q = "SELECT COUNT(unique_id) AS cui,loadfile_creator "
            "FROM bank_safe_lfc ";
    q+= "WHERE itemid IN (SELECT itemid FROM bank_safes ";
    q+= "WHERE (deleted_on IS NULL OR deleted_on = 0)) ";
    if (options[ZB_BANK_FILE_PATTERN])
    {
        q+= "AND file LIKE ? ";
        args += ({ options[ZB_BANK_FILE_PATTERN] });
    }
    if (!countflag && member(options,DB_DBG_LIMIT))
    {
        q+= "GROUP BY loadfile_creator ";
        q+= "ORDER BY cui DESC ";
        q+= "LIMIT ? OFFSET ? ";
        args += ({options[DB_DBG_LIMIT],options[DB_DBG_OFFSET] });
    }
    r = db_query_err(q,args...);
    if (!r) return 0;
    if (countflag) return get_one_int(r);
    return map(r,function string(mixed arr) {
        return sprintf("%3d %s",arr[0],arr[1]);
    });
}

public <int|string*> get_oldest_statistik(mapping options,int countflag)
{
    string q;
    mixed r,*args=({});
    if (countflag)
        q = "SELECT COUNT(*) FROM bank_safe_origin ";
    else
        q = "SELECT itemid,created_on,file FROM bank_safe_origin ";
    q+= "WHERE itemid IN (SELECT itemid FROM bank_safes ";
    q+= "WHERE (deleted_on IS NULL OR deleted_on = 0)) ";
    if (options[ZB_BANK_FILE_PATTERN])
    {
        q+= "AND file LIKE ? ";
        args += ({ options[ZB_BANK_FILE_PATTERN] });
    }
    if (!countflag && member(options,DB_DBG_LIMIT))
    {
        q+= "ORDER BY created_on ASC,file ASC ";
        q+= "LIMIT ? OFFSET ? ";
        args += ({options[DB_DBG_LIMIT],options[DB_DBG_OFFSET] });
    }
    r = db_query_err(q,args...);
    if (!r) return 0;
    if (countflag) return get_one_int(r);
    return map(r,function string(mixed arr) {
        return sprintf("%s %s %s",arr[0],explode(
            shorttimestr(arr[1])," ")[0],arr[2]);
    });
}

public <int|string*> get_timed_lfc(mapping options,int countflag)
{
    string q;
    mixed r,*args=({});
    if (countflag)
        q = "SELECT COUNT(DISTINCT unique_id) FROM bank_safe_lfc ";
    else
        q = "SELECT unique_id,created_on,loadfile_creator "
            "FROM bank_safe_lfc ";
    if (member(options,ZB_BANK_ITEMID))
    {
        q+= "WHERE itemid = ? ";
        args += ({ options[ZB_BANK_ITEMID] });
    }
    else
    {
        q+= "WHERE itemid IN (SELECT itemid FROM bank_safes ";
        q+= "WHERE (deleted_on IS NULL OR deleted_on = 0)) ";
    }
    if (options[ZB_BANK_FILE_PATTERN])
    {
        q+= "AND file LIKE ? ";
        args += ({ options[ZB_BANK_FILE_PATTERN] });
    }
    if (!countflag && member(options,DB_DBG_LIMIT))
    {
        q+= "GROUP BY loadfile_creator ";
        q+= "ORDER BY nr ASC ";
        q+= "LIMIT ? OFFSET ? ";
        args += ({options[DB_DBG_LIMIT],options[DB_DBG_OFFSET] });
    }
    r = db_query_err(q,args...);
    if (!r) return 0;
    if (countflag) return get_one_int(r);
    return map(r,function string(mixed arr) {
        return sprintf("%s %s %s",arr[0],shorttimestr(arr[1]),arr[2]);
    });
}

static void _loop_file_counters()
{
    string q,itemid,itemrowid;
    mixed r,obfiles,obfile;
    mapping item,*contents,content;
    int o_index = 0;
    while (remove_call_out("_loop_file_counters")!=-1);
    q = "SELECT bank_safes.itemid, bank_safes.rowid, item FROM bank_safes "
        "INNER JOIN bank_safe_items ON bank_safe_items.itemrowid = bank_safes.rowid "
        "LEFT OUTER JOIN bank_safe_origin ON bank_safe_origin.itemid = bank_safes.itemid "
        "WHERE (deleted_on IS NULL OR deleted_on = 0) "
          "AND bank_safe_origin.itemid IS NULL "
        "LIMIT 1";
    r = db_query_err(q);
    if (!sizeof(r)) return;
    r = r[0];
    itemid = r[0];
    itemrowid = r[1];
    item = restore_value(r[2]);
    // files = get_files(item);
    contents = item[ARMA_CONTENT_DATA]||({});
    obfiles = get_object_files(item,&o_index,itemid);
    foreach (content : contents)
    {
        obfiles += get_object_files(content,&o_index,itemid);
        // files += get_files(content);
    }
    foreach (obfile : obfiles)
    {
        db_query_err("INSERT OR REPLACE INTO bank_safe_origin (itemid,nr,"
            "file,creator,created_on,unique_id) VALUES (?,?,?,?,?,?)",
            itemid,obfile[0],obfile[1],obfile[2],obfile[3],obfile[4]);
    }
    db_query_err("UPDATE bank_safe_items SET item = ? WHERE itemrowid = ?",
        save_value(item,2), itemrowid);
    call_out("_loop_file_counters",2);
}

public void start_loop_file_counters()
{
    if (extern_call() && !check_security()) 
    {
        debuglog(sprintf("Security Issue %Q",caller_stack(1)),
            DB_DBGLVL_WARNING,TP_RN,"zentralbank:start_loop_file_counters");
        return;
    }
    //call_out("_loop_file_counters",2);
}

public varargs int report_error_bank_safe(string itemid, string owner, 
        int error_reset)
{
    string err;
    if (extern_call() && !check_security()) 
    {
        debuglog(sprintf("Security Issue %Q",caller_stack(1)),
            DB_DBGLVL_WARNING,owner,"zentralbank:report_error_bank_safe");
        return 0;
    }    
    while (42)
    {
        err = 0;
        if (!db_open() || query_db_error()) return 0;
        db_begin(); 
        if (query_db_error()) break;
        if (error_reset)
        {
            db_query("UPDATE bank_safes SET error_count = 0 "
                "WHERE itemid = ?",itemid);
        }
        else
        {
            db_query("UPDATE bank_safes SET error_count = error_count + 1 "
                "WHERE itemid = ?",itemid);
        }
        if (query_db_error()) break;
        db_commit();
        db_close();
        return 1;
    }
    err ||= query_db_error();
    db_rollback();
    debuglog(err,DB_DBGLVL_ERROR,owner,"zentralbank:report_error_bank_safe");
    return 0;
}

// Pruefung durchs Schliessfach beim Einlagern, um Bankmiete zu berechnen.
public int get_current_safe_count(string owner,string bank_id)
{
    mixed result = db_query_err("SELECT COUNT(itemid) FROM bank_safes "
        "WHERE bank_id =?1 AND owner = ?2 "
        "AND (deleted_on IS NULL OR deleted_on = 0)",bank_id,owner);
    return sizeof(result) ? get_one_int(result):__INT_MAX__;
}

/*
FUNKTION: get_conservation_item_rent
DEKLARATION: public varargs float get_conservation_item_rent(object ob,int standard_flag)
BESCHREIBUNG:
Liefert die Gegenstandsmiete für ein Objekt. Ist standard_flag gesetzt, so
wird 1% des Gegenstandswertes angenommen, falls sonst nix gesetzt ist.
Reihenfolge ist Factory,Objekt,Property,Standartwert(falls gewuenscht).
VERWEISE: get_schliessfach_miete
GRUPPEN: armageddon
*/
public varargs float get_conservation_item_rent(object ob,int standard_flag)
{
    float result;
    if (!objectp(ob))
        return 0.0;
    string factory = ob->query(P_CONSERVATION,P_CONSERVATION_FACTORY);
    if (stringp(factory))
    {
        result = factory->query_conservation_item_rent(ob);
        if (result>0.0) 
            return result;
    }
    result = ob->query_conservation_item_rent();
    if (result>0.0) 
        return result;
    result = ob->query(P_CONSERVATION,P_CONSERVATION_ITEM_TARIFF);
    if (result>0.0) 
        return result;
    if (standard_flag)
        result = (ob->query_value() / 100.0);
    return result;
}

public int get_diff_banken(string owner)
{
    string q;
    q = "SELECT COUNT(DISTINCT bank_id) ";
    q+= "FROM bank_safes WHERE owner = ?1 ";
    q+= "AND (deleted_on IS NULL OR deleted_on = 0) ";
    q+= "AND bank_id NOT LIKE 'w_%'";
    mixed r1 = db_query_err(q,owner);
    q = "SELECT COUNT(DISTINCT bank_id) FROM bank_files ";
    q+= "WHERE flag_saveroom = 1 AND bank_id NOT LIKE 'w_%' ";
    mixed r2 = db_query_err(q); // ZB_BANK_SAFEROOM
    return get_one_int(r1) - get_one_int(r2);
}

public mapping get_open_files(int count_results,int shadow_flag)
{
    int ix,jx;
    string q;
    mixed r1;
    mapping result = ([]);
    q = "SELECT file_name,hint FROM bank_safe_filter ";
    q+= "WHERE hint IS NOT NULL AND file_name NOT IN ";
    q+= "(SELECT DISTINCT file FROM bank_safe_files) ";
    if (shadow_flag)
        q+= "AND (flags & ?1)=?1 ";
    else
        q+= "AND (flags & ?1)=0 ";
    q+= "LIMIT 10000"; // sicher ist sicher (Arraygroesse)
    r1 = db_query_err(q,ZB_F_SHADOWS);
    for (ix=0;ix < count_results && sizeof(r1)>0;ix++)
    {
        jx = random(sizeof(r1));
        result[r1[jx][0]] = r1[jx][1];
        r1 = arr_delete(r1,jx);
    }
    return result;
}

// larmalarma: gib die Files zurueck, die inzwischen eingelagert sind o.ae.
public mapping verify_open_files(mapping list)
{
    if (sizeof(list)==0) return ([]);
    mapping result = ([]);
    mixed r1;
    int ix;
    string q = "SELECT file_name,hint FROM bank_safe_filter ";
    q+= "WHERE file_name IN ("+implode(({"?"})*sizeof(list),",")+") ";
    q+= "AND (hint is NULL OR file_name IN ";
    q+= "(SELECT DISTINCT file FROM bank_safe_files) ) ";
    r1 = db_query_err(q,m_indices(list)...);
    for (ix=0;ix<sizeof(r1);ix++)
    {
        result[r1[ix][0]] = r1[ix][1];
    }
    return result;
}

//    q+= "WHERE file_name IN "+convert_array_to_sql_list(m_indices(list));
// Das siegreiche Gnomi denkt: zlpc mixed* args = ({1,2,3}); string query =
//        "SELECT X WHERE Y in (" + implode(({"?"}) * sizeof(args), ",") +
//        ")"; sl_exex(query, args...);

public int get_hidden_file_counters(int shadow_flag)
{
    string q;
    mixed r1;
    q = "SELECT COUNT(file_name) FROM bank_safe_filter ";
    q+= "WHERE hint IS NULL AND file_name NOT IN ";
    q+= "(SELECT DISTINCT file FROM bank_safe_files) ";
    if (shadow_flag)
        q+= "AND (flags & ?1)=?1 ";
    else
        q+= "AND (flags & ?1)=0 ";
    r1 = db_query_err(q,ZB_F_SHADOWS);
    
    return get_one_int(r1);
}


varargs public string get_konten_uebersicht(string owner,object reader)
{
    string result, q,*nasc=({});
    mixed qr,qline,qgesamt,rgesamt=({});
    result = "Schliessfachkontenuebersicht für "+capitalize(owner)+":\n";
    q = "SELECT COUNT(DISTINCT bank_id),COUNT(DISTINCT itemid), ";
    q+= "SUM(bank_rent)+SUM(item_rent), ";
    q+= "MIN(paid_until),MAX(paid_until), ";
    q+= "SUM(rent_diff + ((bank_rent+item_rent)*MIN(?2 - paid_until,?4)*1.0/604800.0)), ";
    q+= "COUNT(itemid), SUM(CASE WHEN ((flags& ?3)=?3) THEN 1 ELSE 0 END) ";
    q+= "FROM bank_safes WHERE owner = ?1 ";
    q+= "AND (deleted_on IS NULL OR deleted_on = 0) GROUP BY owner ";
    qr = db_query_err(q,owner,time(),ZB_F_SAFE_LOCKED,MAX_MIET_DAUER);
    if (!sizeof(qr)) return result+"Keine Schließfachkonten gefunden.\n";
    if (qr[0][0] == 0) return result+"Keine Schließfächer eröffnet.\n";
    qgesamt = qr[0];
    
    q = "SELECT b.bank_id,b.short_title,b.valuta,b.valutas,b.v_gender, ";
    q+= "SUM(bs.bank_rent)+SUM(bs.item_rent), ";
    q+= "MIN(bs.paid_until),MAX(bs.paid_until), ";
    q+= "SUM(rent_diff + ((bank_rent+item_rent)*MIN(?2 - paid_until,?4)*1.0/604800.0)), ";
    q+= "COUNT(itemid), SUM(CASE WHEN ((bs.flags& ?3)=?3) THEN 1 ELSE 0 END) ";
    q+= "FROM banks b,bank_safes bs WHERE b.bank_id = bs. bank_id ";
    q+= "AND bs.owner = ?1 AND (bs.deleted_on IS NULL OR bs.deleted_on = 0) ";
    q+= "GROUP BY b.bank_id,bs.owner ORDER BY b.bank_id ";
    qr = db_query_err(q,owner,time(),ZB_F_SAFE_LOCKED,MAX_MIET_DAUER);
    if (reader->query_no_ascii_art()) 
    {
        foreach(qline:qr) 
        {
            nasc += ({"Bank             : "+(qline[1]||"0")});
            nasc += ({"  Wochenmiete    : "+get_value_in_current_valuta(
                                    qline[5],qline[2],qline[3],qline[4]) });
            nasc += ({"  Offen          : "+get_value_in_current_valuta(
                                    qline[8],qline[2],qline[3],qline[4]) });
            nasc += ({"  Fächer         : "+to_string(qline[9]) });
            nasc += ({"  gesperrt       : "+to_string(qline[10]) });
            nasc += ({"  zuletzt gezahlt: "+
                            explode(shorttimestr(qline[6])," ")[0] });
        }
        nasc += ({"Gesamt über alle Banken:"});
        nasc += ({"  Wochenmiete    : "+get_value_in_current_valuta(qgesamt[2],
                                        "taler","taler","maennlich") });
        nasc += ({"  Offen          : "+get_value_in_current_valuta(qgesamt[5],
                                        "taler","taler","maennlich") });
        nasc += ({"  Fächer         : "+to_string(qgesamt[6]),  });
        nasc += ({"  zuletzt gezahlt: "+
                        explode(shorttimestr(qgesamt[3])," ")[0] });
        return implode(nasc,"\n");
    }
    rgesamt += ({({"Bank","Wochenmiete","Offen","Fächer",
            "gesperrt","gezahlt\nam"}) });
    foreach (qline : qr)
    {
        rgesamt += ({({
            qline[1]||"0",
            get_value_in_current_valuta(qline[5],qline[2],qline[3],qline[4]),
            get_value_in_current_valuta(qline[8],qline[2],qline[3],qline[4]),
            to_string(qline[9]), 
            to_string(qline[10]),
            explode(shorttimestr(qline[6])," ")[0],
        })});
    }
    rgesamt += ({({
        "Gesamt ca.",
        get_value_in_current_valuta(qgesamt[2],"taler","taler","maennlich"),
        get_value_in_current_valuta(qgesamt[5],"taler","taler","maennlich"),
        to_string(qgesamt[6]), 
        "(-)",
        explode(shorttimestr(qgesamt[3])," ")[0],
    })});

    return implode(build_table(transpose_array(rgesamt), 0, 
        ({"l","r","r","r","r","c"}) ),"\n");
}

// TODO DELETE FROM bank_safes WHERE deleted_on IS NOT NULL

mapping query_saved_properties_for_conservation(object ob)
{
    check_security(CHECK_ERROR|CHECK_LAST_OBJECT);
    return objectp(ob) ? ob->query_saved_properties_for_conservation() : 0;
}

void set_saved_properties_for_conservation(object ob, mapping m)
{
    check_security(CHECK_ERROR|CHECK_LAST_OBJECT);
    if (objectp(ob)) 
    {
        ob->set_saved_properties_for_conservation(m);
    }
}


void prepare_renewal() 
{
    db_debug(0,0,DB_DBG_FLUSH_BUFFER,0);
    db_close();
}
void abort_renewal() {}
void finish_renewal(object neu) {}
