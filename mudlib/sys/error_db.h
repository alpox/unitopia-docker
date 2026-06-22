// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:    /sys/error_db.h
// Description: Defines fuer /apps/error_db.c
// Author:  Freaky/Monty (12.07.1995)
// Modified by: Freaky (30.07.1999) Subtypes eingefuehrt
//      Freaky (28.01.2000) Defines fuer die Fehlerstrukturen eingebaut

#ifndef ERROR_DB_H
#define ERROR_DB_H 1

// Ein Define ueber die Version der Datenbank fuer externe Tools.
#define ERROR_DB_VERSION_1 1

#define ERROR_DB                "/apps/error_db"
#define ERROR_DB_SAVE_FILE      "/var/error_db"
#define ERROR_DB_DIR            "/var/error"
#define ERROR_ARCHIVE           "/apps/error_archive"
#define ERROR_ARCHIVE_DB        "/var/error_archive.db"
#define ERROR_DB_DATA_DIR(nr)   sprintf(ERROR_DB_DIR"/%:03d",(nr)%1000)
#define ERROR_DB_DATA_FILE(nr)  sprintf(ERROR_DB_DIR"/%:03d/%010d",(nr)%1000,(nr))

// Defines fuer die Headerdaten:
#define EDB_H_TYPE      0   // Fehlertyp (int)
#define EDB_H_FILE      1   // Verantwortliche Datei (string)
#define EDB_H_PLAYER    2   // Verursachender Spieler (string)
#define EDB_H_OPENED    3   // Datum des Fehlers (int)
#define EDB_H_LAST_CHANGE 4 // Letzte Aenderung (int)
#define EDB_H_CLOSED    5   // Schliessung des Fehlers (int)
#define EDB_H_DEBUGGER  6   // Debuggerliste (string *)
#define EDB_H_LINE      7   // Zeilennummer bei RTE's
#define EDB_H_SIZE      8

// Die Fehlerklassen fuer die Datenbank, verwendet fuer den jeweils ersten
// Eintrag eines Fehlerarrays (EDB_H_TYPE)
#define EDB_FEHLER  1   // Fehler, mit dem 'fehler' erzeugt
#define EDB_IDEE    2   // Eine Idee
#define EDB_RUNTIME 3   // Runtime-Error
#define EDB_COMPILE 4   // Compile-Error
#define EDB_LOB     5       // Ein Lob 
#define EDB_TYPO    6       // Typo
#define EDB_DETAIL  7       // Raumdetail
#define EDB_ARCHIVE 8       // Sonderbehandlungs archivierte Fehler bei Maild.
#define EDB_ERRLIST 9       // Sonderbehandlung Fehlerlist-Zusammenfassungen.

// Maske fuer EDB_OPT_ERROR_TYPES << (EDB_H_TYPE - 1)
#define EDB_TMASK_FEHLER  0x01    // Fehler, mit dem 'fehler' erzeugt
#define EDB_TMASK_IDEE    0x02    // Eine Idee
#define EDB_TMASK_RUNTIME 0x04    // Runtime-Error
#define EDB_TMASK_COMPILE 0x08    // Compile-Error
#define EDB_TMASK_LOB     0x10    // Ein Lob 
#define EDB_TMASK_TYPO    0x20    // Typo
#define EDB_TMASK_DETAIL  0x40    // Raumdetail
#define EDB_TMASK_ARCHIVE 0x80
#define EDB_TMASK_ALL     0xFF    // einfach alle.

// Defines fuer die eigentlichen Fehlerdaten, Indizes in ein Mapping:
#define EDB_E_TYPE      "Type"      // Fehlertyp
#define EDB_E_WARNING   "Warning"   // !=0, wenn eine Warnung
#define EDB_E_PLAYER    "Player"    // Verursachender Spieler
#define EDB_E_DATE      "Date"      // Nochmal das Datum
#define EDB_E_ERROR     "Error"     // Die Fehlermeldung
#define EDB_E_HISTORY   "History"   // Fehlerverlauf
#define EDB_E_SUBERRORS "Suberrors" // Weitere Auftreten des Fehlers
#define EDB_E_OBJ       "Object"    // Urspruengliches Objekt
#define EDB_E_WHO       "Who"       // TI/TP
#define EDB_E_FIRSTROOM "First Room"    // Erster Raum des Objektes
#define EDB_E_USER_INFO "Debug Info"    // Debuginformationen
#define EDB_E_COMMAND   "Command"   // Eingegebener Befehl
#define EDB_E_TRACE     "Trace"     // Auszug aus dem Debug.log
#define EDB_E_SUBTYPE   "Subtype"   // Art des Fehlers
#define EDB_E_ROOM      "Room"      // Aktueller Raum des Spielers
#define EDB_E_OBJ_DESC  "Description"   // Spielerverstaendliche Objektbeschreibung
#define EDB_E_ITEM      "Item"      // Das Detail als Mapping
// nur zum Initialiseren (log_error.inc), ansonsten im Header:
#define EDB_E_DEBUGGER  "Debugger"  // Debugger der Fehler
#define EDB_E_FILE      "File"      // Dateiname + Include
#define EDB_E_LINE      "Line"      // Zeilennummer

// Defines fuer den Eintrag 'EDB_E_SUBTYPE'
#define EDB_RUN_RUNTIME         0   // 'richtiger' Runtime-Error
#define EDB_RUN_HB              1   // Heartbeat-Error
#define EDB_RUN_USER_DEFINED    2   // User defined Error (do_error)

// Defines fuer die Arrayelemente im Fehlerverlauf
#define EDB_EH_DATE 0   // Datum des Eintrages
#define EDB_EH_WIZ  1   // Verursacher des Eintrages
#define EDB_EH_TYPE 2   // Typ des Eintrages
#define EDB_EH_INFO 3   // Zusatzinfos
#define EDB_EH_SIZE 4

// Defines fuer Typen von Fehlerverlaufseintraegen
#define EDB_EHT_COMMENT         "Comment"
#define EDB_EHT_REASSIGN_FILE   "Reassign File"
#define EDB_EHT_CLOSE           "Close"
#define EDB_EHT_REOPEN          "Reopen"
#define EDB_EHT_CHANGE_TYPE     "Change Type"
#define EDB_EHT_FEEDBACK        "Feedback"
#define EDB_EHT_QUESTION        "Question"
#define EDB_EHT_ANSWER          "Answer"
#define EDB_EHT_ADD_DEBUGGER    "Add debugger"
#define EDB_EHT_REMOVE_DEBUGGER "Remove debugger"
// Hilfsdefine fuer format_history und add_history
#define EDB_HISTORY_TYPES ([ EDB_EHT_COMMENT : "Kommentar:    ", \
                       EDB_EHT_REASSIGN_FILE : "Neuzuweisung: ", \
                               EDB_EHT_CLOSE : "L\u00f6schen:      ", \
                              EDB_EHT_REOPEN : "Undo-L\u00f6schen: ", \
                         EDB_EHT_CHANGE_TYPE : "Fehlertyp:    ", \
                            EDB_EHT_FEEDBACK : "Feedback:     ", \
                            EDB_EHT_QUESTION : "R\u00fcckfrage:    ", \
                              EDB_EHT_ANSWER : "Antwort:      ", \
                        EDB_EHT_ADD_DEBUGGER : "Hinzugef\u00fcgt:  ", \
                     EDB_EHT_REMOVE_DEBUGGER : "Deb.entfernt: " ])
                
// Die Rueckgabewerte von delete_error(), hide_error() und add_debugger():
#define ERROR_OK     0  // hat geklappt
#define NO_ERROR     1  // Fehlernummer nicht (mehr) vorhanden
#define NOT_RESPONSIBLE  2  // Nicht zustaendig!
#define LAST_DEBUGGER    3  // Sonst ist keiner mehr zustaendig!
#define ALREADY_DEBUGGER 4  // Der debuggt schon (bei add_debugger())
#define NOT_VALID    5  // Den kann man als Debugger nicht eintragen!

// Optionen fuer die Filterung, Suche und Sortierung von Fehlern
#define EDB_OPT_ZUSTAENDIGER   "options:zustaendiger"
#define EDB_OPT_GROUP_ONE      "options:group:one"
#define EDB_OPT_GROUP_PREFIX   "options:group:prefix" 
#define EDB_OPT_GROUP_PREFIXES "options:group:prefix:multi" 
#define EDB_OPT_GROUP_PATTERN  "options:group:pattern"
#define EDB_OPT_GROUP_ALL      "options:group:all"
#define EDB_OPT_GROUP_INCLUDE  "options:group:include"
#define EDB_OPT_GROUP_EXCLUDE  "options:group:exclude"
#define EDB_OPT_GROUP_EXCLUDE_PATTERN "options:group:exclude:pattern"
#define EDB_OPT_ADD_W_GROUP    "options:add:w:group"
#define EDB_OPT_ADD_S_GROUP    "options:add:s:group"
#define EDB_OPT_ADD_Z_GROUP    "options:add:z:group"
#define EDB_OPT_EXLUSIVE_GROUPS "options:groups:exclusives"
#define EDB_OPT_ARCHIVE_PATTERN "options:archive:pattern"
#define EDB_OPT_ARCHIVE_INVIS  "options:archive:invis"
#define EDB_OPT_WIEDERVORLAGE  "options:wiedervorlage"
#define EDB_OPT_SHORT_HISTORY  "options:short:history"
#define EDB_OPT_ERRNUMS        "options:errnums" 
#define EDB_OPT_FILE_PATTERN   "options:file:pattern"
#define EDB_OPT_VERURSACHER_PATTERN "options:verursacher:pattern"
#define EDB_OPT_ERROR_TYPE     "options:error:type"
#define EDB_OPT_ERROR_TYPES    "options:error:types"
#define EDB_OPT_NO_EMPTY_ERR   "options:no:empty:errors"
#define EDB_OPT_ONLY_NEW_ERR   "options:only:new:errors"
#define EDB_OPT_NEW_ERRNUMS    "options:new:errnums"
#define EDB_OPT_NO_PRIVATES    "options:no:privates"
#define EDB_OPT_NEWER_SINCE    "options:newer:since"
#define EDB_OPT_NEWER_BEFORE   "options:newer:before"
#define EDB_OPT_CREATED_SINCE  "options:created:since"
#define EDB_OPT_CREATED_BEFORE "options:created:before"
#define EDB_OPT_ERRNUM_START   "options:errnum:start"
#define EDB_OPT_ERRNUM_STOP    "options:errnum:stop"    
#define EDB_OPT_SORT_OPEN_ASC  "option:sort:opened:ascending"
#define EDB_OPT_SORT_OPEN_DESC "option:sort:opened:descending"
#define EDB_OPT_SORT_CHANGE_ASC "option:sort:changed:ascending"
#define EDB_OPT_SORT_CHANGE_DESC "option:sort:changed:descending"
#define EDB_OPT_HIERACHY_1     "option:hierachy:1"
#define EDB_OPT_HIERACHY_2     "option:hierachy:2"
#define EDB_OPT_HIERACHY_3     "option:hierachy:3"
#define EDB_OPT_HIERACHY_9     "option:hierachy:9"
#define EDB_OPT_HASHTAGS       "option:hashtags"
#define EDB_OPT_LISTID         "option:list:id"
#define EDB_OPT_FILTER_TYPE    "option:filter:type"
#define EDB_OPT_FILTER_GROUP   "option:filter:group"
#define EDB_OPT_FILTER_COUNT   "option:filter:count"
#define EDB_OPT_FILTER_MAP     "option:filter:map"
#define EDB_OPT_ACTIONID       "option:action:id"
#define EDB_OPT_ACTION_TYPE    "option:action:type"
#define EDB_OPT_ACTION_PARAM   "option:action:param"
#define EDB_OPT_ACTION_COUNT   "option:action:count"
#define EDB_OPT_ERRNUM_COUNT   "option:errnum:count"
#define EDB_OPT_DEBUGGERS      "option:action:debuggers"
#define EDB_OPT_WORKING_SET    "option:working:set"

// Defines fuer die Fehlerliste (Rueckgabe von query_error_list)
#define EDB_ERRLIST_ERRNUM   0  // die referenzierte Fehlernummer.
#define EDB_ERRLIST_TIME     1  // Zeitstempel: EDB_H_OPENED/EDB_H_LAST_CHANGE
#define EDB_ERRLIST_TYPE     2  // Typeschluessel: F,I,R,C,W,T,D
#define EDB_ERRLIST_PLAYER   3  // eingertagener Verursacher
#define EDB_ERRLIST_FILE     4  // Referenzierte Datei.

// Defines fuer die Abspeicherung der neuen Fehler im Player
#define EDB_LAST_READ       0  // (int) Zeitstempel
#define EDB_NEW_WIZ_ERRORS  1  // (int*) Fehlerliste
#define EDB_WIEDERVORLAGE       2  // (mapping (int time:int*fehlerliste)

// Aufheben geloeschter Fehler:
#define EDB_BACKUP_COUNT    1000      // Nur die X zuletzt geloeschten Fehler aufheben.
#define EDB_BACKUP_GROUP        "Backup"  // Name der (Pseudo-)Gruppe, in der die Fehler landen.
#define EDB_NO_ARCHIVE_GROUP "KeinArchiv" // Name der Gruppe, die unsichtbar im Archiv verbleibt.

#if 1
// Die Subeintraege der Fehler
#define EDB_TYPE        0
#define EDB_FILE        1
#define EDB_ERROR       2
#define EDB_LAST_CHANGE     3
#define EDB_COMMENT     4
#define EDB_DATE        5
#define EDB_DEBUGGER        6

// Fehler, Idee, Lob, Typo
#define EDB_F_FILE      EDB_FILE
#define EDB_F_ERROR     EDB_ERROR
#define EDB_F_LAST_CHANGE   EDB_LAST_CHANGE
#define EDB_F_COMMENT       EDB_COMMENT
#define EDB_F_DATE      EDB_DATE
#define EDB_F_DEBUGGER      EDB_DEBUGGER
#define EDB_F_WHO       7
#define EDB_F_ROOM      8
#define EDB_F_OBJECT_CLEARTEXT  9
#define EDB_F_USER_INFO     10
#define EDB_F_SIZE      11

// Runtime-Fehler
#define EDB_R_FILE      EDB_FILE
#define EDB_R_ERROR     EDB_ERROR
#define EDB_R_LAST_CHANGE   EDB_LAST_CHANGE
#define EDB_R_COMMENT       EDB_COMMENT
#define EDB_R_DATE      EDB_DATE
#define EDB_R_DEBUGGER      EDB_DEBUGGER
#define EDB_R_OB        7
#define EDB_R_LINE      8
#define EDB_R_COMMAND       9
#define EDB_R_WHO       10
#define EDB_R_FIRST_ROOM    11
#define EDB_R_SUB_TYPE      12
#define EDB_R_DEBUG_OFFSET  13
#define EDB_R_DEBUG_SIZE    14
#define EDB_R_USER_INFO     15
#define EDB_R_SIZE      16

// Compile-Fehler
#define EDB_C_FILE      EDB_FILE
#define EDB_C_ERROR     EDB_ERROR
#define EDB_C_LAST_CHANGE   EDB_LAST_CHANGE
#define EDB_C_COMMENT       EDB_COMMENT
#define EDB_C_DATE      EDB_DATE
#define EDB_C_DEBUGGER      EDB_DEBUGGER
#define EDB_C_SIZE      7

#define EDB_TIME_TO_DELETE  5184000 // 60 Tage

// Fehler-Kommentare
#define EDB_COM_WHO     0
#define EDB_COM_DATE        1
#define EDB_COM_COMMENT     2
#define EDB_COM_SIZE        3

#endif

// List-Filters
#define EDB_LFT_WHITELIST       1
#define EDB_LFT_BLACKLIST       2
#define EDB_LFT_LIST_READERS    3
#define EDB_LFT_LIST_WRITERS    4
#define EDB_LFT_TARGET_LISTIDS  5
#define EDB_LFT_MAX             5
#define EDB_LIST_FILTER_TYPES ({ \
    "","Whitelist","Blacklist","Readers","Writers","Targets"})

// List-Actions
#define EDB_LA_INPUT_ACTION     0x000001
#define EDB_LA_IN_LIST_ACTION   0x000002
#define EDB_LA_SOURCES          0x000003

#define EDB_LA_ON_NEW           0x000010
#define EDB_LA_ON_DBG_CHANGE    0x000020
#define EDB_LA_ON_CHANGE        0x000040
#define EDB_LA_ON_DELETE        0x000080
#define EDB_LA_ON_READ          0x000100
#define EDB_LA_ON_READ_OWN      0x000200
#define EDB_LA_ON_READ_ARCHIVE  0x000400
#define EDB_LA_TRIGGERS         0x0007F0
#define EDB_LA_TRIGGERS_SUBSET1 0x0004C0

#define EDB_LA_REMOVE           0x001000
#define EDB_LA_FORWARD          0x002000
#define EDB_LA_MAIL             0x004000
#define EDB_LA_NEW              0x008000
#define EDB_LA_FORWARD_OTHERS   0x010000
#define EDB_LA_REACTIONS        0x01F000

// Flags add or delete debugger
#define EDB_DBGF_ADD    1
#define EDB_DBGF_DEL    2

#endif // ERROR_DB_H
