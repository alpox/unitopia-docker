// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /secure/master/auto_include.inc
// Description: Funktion fuer den Driver-Hook H_AUTO_INCLUDE
// Author:      Freaky (18.11.2001)

string auto_include(string base_file, string current_file, int sys_include)
{
    string ret;

    if (sys_include || current_file)
    {
        // Kein Auto-Include String fuer Include-Dateien.
        // Es gelten die Einstellungen der Datei, die den #include ausfuehrt.
        return 0;
    }

    // Fehlenden / hinzufuegen
    base_file = "/"+base_file;


/*
   Einstellungen fuer alle
*/
    ret = "#pragma combine_strings, verbose_errors, warn_deprecated, "
                  "strong_types, save_types, pedantic, "
                  "no_warn_missing_return\n"
          "default private variables;\n";

#if __VERSION__ >= "3.5.0-2790"
    ret += "#pragma rtt_checks\n";
#endif // __VERSION__ >= "3.5.0-2790"
#if __VERSION__ > "3.6.1-U048"
    ret += "#pragma warn_unused_variables\n";
#endif
#if __VERSION__ > "3.6.4-U056"
    /* Dead-Code-Warnung nur fuer die Core-Lib und ausgewaehlte Bereiche
       anzeigen.
    */
#ifdef TestMUD
    if (regmatch(base_file, "^/(apps|i|obj|room|secure|p|d/(Arktis|Doerrland|Vaniorh)|z/Gilden/(Alchemistengilde|Hexenvolk|Ritterorden)|z/(Raetsel|Spiele))/|^/map/([^m]|map)"))
#else
    if (regmatch(base_file, "^/(apps|i|obj|room|secure|z/Gilden/Hexenvolk)/|^/map/([^m]|map)"))
#endif
        ret += "#pragma warn_dead_code\n";
#endif

#if __VERSION__ > "3.6.4-U059"
    ret += "#pragma warn_applied_functions\n";
#endif
#if __VERSION__ > "3.6.7-U064"
    ret += "#pragma save_local_names\n";
#endif
#if __VERSION__ < "3.6.8-U065"
    // Der Driver hat dies fehlerhafterweise bei PCRE2 nicht definiert.
    ret += "#undef __PCRE__\n#define __PCRE__ 1\n";
#endif

    ret += "#undef FILTER_MSG_BY_ATTRIBUTES\n#define FILTER_MSG_BY_ATTRIBUTES\n";

#ifdef __LPC_LWOBJECTS__
    if (strstr(base_file, "/lwo/") >= 0)
        ret += "#pragma lightweight\n";
#endif
#ifdef __LPC_COROUTINES__
    ret += "#pragma warn_unused_values\n";
#endif

/*
   Einstellungen fuer TestMUD (Orbit), nicht UNItopia
*/
#ifdef TestMUD

#else

#endif // TestMUD


/*
   besondere Einstellungen fuer Teile UNItopias, nicht TestMUD (Orbit)
     TODO: zu allgemeinen Einstellungen machen bzw. loswerden!
     ACHTUNG: /map/m enthaelt Files aller Domains!
*/
#if defined(UNItopia) && !defined(TestMUD)

/*
// das hier wieder aktivieren, falls kurzfrisitig doch ein Raetsel 
// die RTTCs wieder ausschalten muss
#define BAD_CODESTYLE "^/(z/Raetsel/(<DIR1>|<DIR2>)" \
                           "Admin|apps|i|obj|room|sys)" \
                        "|map/m)"

    if (!sizeof(regexp(({base_file}), BAD_CODESTYLE)))
    {
        ret += "#pragma rtt_checks\n";
    }
*/

#endif // defined(UNItopia) && !defined(TestMUD)


    return ret;
}
