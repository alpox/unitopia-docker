// This file is part of UNItopia Mudlib.
// -----------------------------------------------------------------------
//  Datei:  /i/tools/dynamic_browser.c
//  Autor:  Myonara 30.Okt.2012 nach einer Idee von Gnomi.
// -----------------------------------------------------------------------
// Beschreibung: Ein dynamischer Browser, der keine Groessenbegrenzung 
//               kennt.
// -----------------------------------------------------------------------
// Aenderungen:
//  Myonara  13.Nov.2012 Integration von u.a. B_DYNAMICMORE,B_STATICMORE,
//                       B_HELP, B_DATA, B_FILE, B_QUIT, B_CONTINUE,
//                       Standardaction "",d,u,q,<. B_START_LINE
//  Myonara  20.Dez.2012 Umstellung B_NOTHING, B_CONTINUE.
//                       Umstellung B_W auf browse_write_line inkl Doku.
//                       Einfuehrung von <type>_select fuer Menues.
//                       <type>_init, <type>_reload, BF_DIRTY, B_REDRAW
//                       staticmore_init und dynamicmore_init
//  Sorcerer 04.Jan.2013 ins P und Registry
//  Myonara  04.Jan.2013 flag_help, flag_return, BF_RETURN
//  Myonara  27.Apr.2013 B_HEADER_LINES und _init beim Menuwechcsel aufrufen
//  Myonara  01.Mai.2013 neue Funktion _total eingefuehrt und genutzt.
//                       ende bei STATICMORE auf sizeof(...B_DATA) gesetzt.

// Wenn wir das Ende sehen, merken wir es uns in:
#define DYNAMICMORE_TOTAL_LINES "dynamicmore_total"

/*
FUNKTION: dynamic_browse
DEKLARATION: int dynamic_browse(mapping menue|mapping* menues)
BESCHREIBUNG:
Startet das Menue 'menue' bzw. geht direkt in ein Untermenue,
wenn ({ Hauptmenue, Untermenue, ... }) angegeben wurde.
 
Das Menue wird dabei durch ein Mapping angegeben, welches folgende
Eintraege haben kann:
 
    B_TYPE      Der Menuetyp (ein String). Alle weiteren Funktionsaufrufe
                enthalten diesen String vorangestellt (es wird also
                <type>_init, <type>_display usw. aufgerufen).
                "browse" ist als Typ verboten.
    B_OB        Gibt das Objekt an, in welchem die weiteren Funktionen
                zu finden sind (per Default: this_object()).
    B_OB_STR    Hier wird der Typ String des B_OB verarbeitet.
                Besonderheit: wenn ein # am Ende des String steht,
                wird ein Clone davon im Spieler gesucht.
    B_CURRENT_LINE  Aktuelle Zeilennummer.
    B_START_LINE Anfang der Zeilenzaehlung (meist 0 oder 1)
    B_END_LINE  Ende der Zeilen. wird z.B. durch <type>_total nur einmal 
                ermittelt, damit <type>_prompt die <type>_total nicht ein 2.Mal
                aufruft.
    B_NUM_LINES Maximalanzahl Zeilen zur aktuellen Anzeige 
                (wird intern erzeugt und initialisiert)
    B_FLAGS     folgende Flags werden unterstuetzt: 
                BF_DIRTY: dynamic_browse muss ueber <type>_reload 
                die Menumappings reinitialisieren, um fortzufahren.
                BF_RETURN: Dieses Menue ist ein Untermenue (wie B_HELP)
                und kann mit "z" verlassen werden. fuer B_STATICMORE
                und B_DYNAMICMORE (prompt-Anzeige)
                BF_NO_DISPLAY: die DisplayAnzeige (bei B_DONE) 
                einmal unterdruecken.
                BF_MENU_PATH: wird intern verwendet fuer query_menue_path.
    B_PROMPT    Ein String mit %d, der B_CURRENT_LINE ausgibt.
                Falls kein B_PROMPT und kein <type>_prompt implementiert ist,
                wird <type>-<currentline> ausgegeben.
    B_DATA      ist ein optionales Stringarray, dass wie bei more
                durchlaufen wird.  Der B_TYPE==B_STATICMORE
                wird dafuer genutzt.
    B_FILE      ist ein String fuer eine Datei, die angezeigt wird.
                Der B_TYPE==B_DYNAMICMORE wird dafuer genutzt.
    B_HELP      ist ein menuemapping wie gerade beschrieben,
                dass bei Aufruf von '?' in den Actions im Browser
                aufgerufen wird. zb mit B_DATA oder mit B_FILE.
    B_HEADER_LINES  Ein optionales Array der Groesse 4 mit folgenden Elementen:
                ({ erste Zeile, letzte Zeile,
                mittlere linie oben, mittlere linie unten})
    B_SEARCH_STRING: interner Suchstring der durch das Default-Kommando /
                gesetzt und an <type_search_forward(menue) uebergeben wird.
 
Im Laufe der Verarbeitung werden folgende Funktionen aufgerufen:

    mixed <type>_init(mapping old) 
            wenn diese Funktion existiert, wird sie zum Initialisieren
            des Menumappings genutzt. So braucht man beim Aufruf von
            nur den B_TYPE und evtl B_OB anzugeben, alles andere ergaenzt 
            dann die <type>_init Funktion. Bei Fehler notify_fail aufrufen
            und B_QUIT zurueckgeben. B_NOTHING laesst das mapping 
            unveraendert.
    string * <type>_display(mapping menue)       
            Gibt den aktuellen Ausschnitt der Daten als String-Array zurueck.
            ein leeres Array zeigt ein Ende der Daten, Rueckgabewert 0 
            zeigt einen Fehler an und bricht das Browsen ab.
    int <type>_total(mapping menue)
            Gibt, wenn existent, die insgesamte Anzahl von Zeilen aus,
            die aktuell vorhanden sind. Dient zur Endeerkennung.
    string <type>_prompt(mapping menue)
            Gibt den Promptstring des aktuellen Menus zurueck.
            Falls nicht vorhanden wird B_PROMPT versucht (s.o.)
    mixed <type>_action(string str, mapping * menues)
            auf die Eingabe reagieren, 
            Rueckgabe B_NOTHING: 
                      Standardbefehle [,u,d,q,?] im browser ausfuehren.
            Rueckgabe B_QUIT: Browsen beenden
            Rueckgabe B_CONTINUE: naechste Seite ausgeben.
            Rueckgabe B_REDRAW: wieder vom Anfang an ausgeben.
            Rueckgabe B_REBUILT: BF_DIRTY wird gesetzt, um gesamte Anzeige 
                                 zu erneuern.
            Rueckgabe B_DONE:  BF_NO_DISPLAY wird gesetzt, um zb fuer 
                               Fehlermeldungen ein ueberschreiben mit menue
                               zu vermeiden.
            Rueckgabe  mapping * neue oder alte Menuestruktur zum Ausfuehren
    mixed <type>_select(int nr, mapping menue)
            Wenn bei B_NOTHING die Standardbefehle ausgewertet werden,
            werden auch die Angabe von Nummern ausgewertet und
            wenn vorhanden an <type>_select weitergereicht.
            Als Rueckgabewerte wird ein Menu-mapping oder B_NOTHING erwartet.
    mixed <type>_reload(mixed menues)
            wird aufgerufen, wenn das BF_DIRTY-Flag gesetzt ist,
            um die Menustruktur zu reinitialisieren.
    mixed <type>_search_forward(mapping menue)
            mittels uebergebenen B_SEARCH_STRING wird nach diesem gesucht.
            das angepasste menue (mit neuem B_CURRENT_LINE) wird im
            Erfolgsfall zurueckgegeben, 0 sonst. Optionale Funktion,
            wenn nicht vorhanden, erscheint eine Standardfehlermeldung bei /.
VERWEISE: browse_write_line,push_menue,pop_menue,query_menue_path,
          restore_menue_path
GRUPPEN: dynamic_browse
*/

#include <browser.h>
#include <dynamic_browser.h>
#include <message.h>
#include <notify_fail.h>
#include <input_to.h>
#include <touch.h>

mapping *menu_path=({});

/*
FUNKTION: browse_write_line
DEKLARATION: public int browse_write_line(string str)
BESCHREIBUNG:
Gibt eine Zeile des Menues aus, oder eine Meldung innerhalb <type>_action.
Kann auch ueberlagert werden, wenn die Art der Ausgabe angepasst werden 
muss.
VERWEISE: dynamic_browser
GRUPPEN: dynamic_browse
*/
public int browse_write_line(string str)
{
    if (!stringp(str)) return 0;
    if (!sizeof(str) || str[<1..]!="\n") {
        str += "\n";
    }
    this_player()->send_message_to(this_player(),MT_NOTIFY,MA_READ, str);
    return 1;
}

/*
FUNKTION: push_menue
DEKLARATION: protected mapping* push_menue(mapping* menues,mappping newmenue)
BESCHREIBUNG:
Fuegt newmenue den menues hinzu und gibt das resultierende menues zurueck.
Fuegt ebenso newmenue dem Menupfad (query_menue_path) hinzu.
VERWEISE: dynamic_browser, query_menue_path,update_menue,pop_menue
GRUPPEN: dynamic_browse
*/
protected mapping* push_menue(mapping* menues,mapping newmenue)
{
    mapping tmp;
    if (!mappingp(newmenue))
    {
        return sizeof(menues)?menues:({});
    }
    if (!member(newmenue,B_OB))
    {
        newmenue[B_OB] = this_object();
    }
    if (!pointerp(menues) || !sizeof(menues))
    {
        menu_path = ({});
        menues = ({});
    }
    tmp = deep_copy(newmenue);
    tmp += ([ B_FLAGS:tmp[B_FLAGS]|BF_MENU_PATH ]);
    tmp[B_OB_STR] = object_name(tmp[B_OB]);
    m_delete(tmp,B_OB);
    menu_path += ({ tmp });
    menues +=    ({ deep_copy(newmenue) });
    return menues;
}

/*
FUNKTION: pop_menue
DEKLARATION: protected mapping* pop_menue(mapping* menues,int cnt = 1)
BESCHREIBUNG:
Entfernt cnt Menueelemente vom menues und menu_path.
VERWEISE: dynamic_browser, query_menue_path, push_menue,update_menue
GRUPPEN: dynamic_browse
*/
protected varargs mapping* pop_menue(mapping* menues,int cnt)
{
    if (!pointerp(menues) || !sizeof(menues))
    {
        return menues;
    }
    if (cnt <= 0) 
        cnt = 1;
    int size = sizeof(menues)-cnt;
    if (size > 0)
    {
        size--;
        menues = menues[0..size];
        menu_path = menu_path[0..size];
    }
    return menues;
}

/*
FUNKTION: update_menue
DEKLARATION: protected int update_menue(<mapping|mapping*> menues,mapping to_add)
BESCHREIBUNG:
Aktualisiert menues und den menu_path mit to_add sofern vorhanden.
Sinnvoll dann, wen sich Aufrufparameter aendern, die der menu_path sonst nicht
mitbekommen wuerde.
VERWEISE: dynamic_browser, query_menue_path, push_menue
GRUPPEN: dynamic_browse
*/
protected varargs int update_menue(<mapping|mapping*> menues,
    mapping to_add,mapping to_delete)
{
    if (mappingp(menues))
    {
        menues -= to_delete||([]);
        menues += to_add||([]);
    }
    else if (pointerp(menues) && sizeof(menues))
    {
        menues[<1] -= to_delete||([]);
        menues[<1] += to_add||([]);
    }
    if (sizeof(menu_path))
    {
        menu_path[<1] -= to_delete||([]);
        menu_path[<1] += to_add||([]);
        return 1;
    }
    return 0;
}

/*
FUNKTION: query_menue_path
DEKLARATION: protected mapping* query_menue_path()
BESCHREIBUNG:
Liefert den aktuellen Pfad zurueck.
VERWEISE: dynamic_browser, push_menue
GRUPPEN: dynamic_browse
*/
protected mapping* query_menue_path()
{
    return deep_copy(menu_path);
}

/*
FUNKTION: restore_menue_path
DEKLARATION: public mapping* restore_menue_path(mapping *menupath)
BESCHREIBUNG:
Setzt den internen menu_path und gibt das menues Array zurueck.
VERWEISE: dynamic_browser, push_menue
GRUPPEN: dynamic_browse
*/
protected mapping* restore_menue_path(mapping *menupath)
{
    if (!sizeof(menupath))
    {
        menu_path = ({});
        return ({});
    }
    else
    {
        menu_path = deep_copy(menupath);
        return deep_copy(menupath);
    }
}

/*
FUNKTION: notify_dyn_browse_data
DEKLARATION: protected void notify_dyn_browse_data(mapping menue,mapping *mpath)
BESCHREIBUNG:
Zeigt die aktuellen Menu-Daten genau vor dem Auswerten und Anzeigen.
Der Menue Pfad mpath wird zur Speicherung ebenso uebergeben.
VERWEISE: dynamic_browser
GRUPPEN: dynamic_browse
*/
protected void notify_dyn_browse_data(mapping menue,mapping *mpath)
{
    // Dummy zum ueberlagern.
}

nomask int dynamic_browse(mixed menues);
private nomask int dyn_browse_internal(mapping* menues);

private int provide_type_and_object(mapping menue,string mtype,
    <object|lwobject> mobj)
{
    string objstr;
    mtype = menue[B_TYPE];
    if (member(menue, B_OB) && objectp(menue[B_OB]))
    {
        mobj = menue[B_OB];
    }
    else if (member(menue, B_OB_STR)) 
    {
        objstr = menue[B_OB_STR];
        if (strstr(objstr,"#")>=0)
        {
            mobj = present_clone(objstr,this_player());
        }
        else
        {
            mobj = touch(objstr,NO_WRITE);
        }
    }
    else
    {
        mobj = this_object();
    }
    if (!stringp(mtype)) {
        browse_write_line("Menuedaten korrupt.\n");
        return B_QUIT;
    }
    if (!objectp(mobj)&&!lwobjectp(mobj)) {
        browse_write_line("Menü-Objekt verloren.\n");
        return B_QUIT;
    }
    return B_DONE;
}

nomask static int dyn_browse_input(string str, mapping* menues)
{
    int i;
    string mtype,search;
    <object|lwobject> mobj;
    mixed result;
    mapping newmenue;
    
    if (provide_type_and_object(menues[<1],&mtype,&mobj)==B_QUIT)
        return B_QUIT;
    if (call_resolved(&result, mobj, mtype+"_action", str, menues)) {
        if (intp(result)) {
            switch(result) {
            case B_QUIT: 
                return B_QUIT;
            case B_NOTHING: break;
            case B_CONTINUE: str = "";break;
            case B_REDRAW: str = "<"; break;
            case B_REBUILT: 
                str = "<";
                menues[<1][B_FLAGS] |= BF_DIRTY;
                break;
            case B_DONE:
                menues[<1][B_FLAGS] |= BF_NO_DISPLAY;
                dynamic_browse(menues);
                return B_CONTINUE;
            }
        } else if (pointerp(result)
                && sizeof(filter(result,(: mappingp($1) :))) ==
                   sizeof(result) ) {
            dynamic_browse(result);
            return B_CONTINUE;
        }
    }
    mapping menue = menues[<1];
    int start = menue[B_CURRENT_LINE];
    int end = -1;
    switch (sizeof(str))
    {
    case 0: str = "d"; break;
    case 1: search = 0; break;
    default: search = str[1..]; break;
    }
    switch (str[0..0]) 
    {
    case "":
    case "d":
        if (menue[B_TYPE] == B_STATICMORE) 
        {
            end = sizeof(menue[B_DATA])-1; 
        }
        else 
        {
            call_resolved(&end, mobj, mtype+"_total", menue);
            if (end > 0 && menue[B_START_LINE]==0) end--; 
        }
        if (end >= 0) {
            if (start < end) {
                start = min(  end - menue[B_NUM_LINES]/2 + 1, 
                            start + menue[B_NUM_LINES]);
            } else {
                start = max(  end - menue[B_NUM_LINES]/2 + 1, 
                              menue[B_START_LINE] );
                browse_write_line("Ende der Daten erreicht.");
            }             
            if (start < menue[B_START_LINE]) 
            {
                start = menue[B_START_LINE];
            }
        } else {
            start += menue[B_NUM_LINES] + 1;
        }
        menue[B_CURRENT_LINE] = start;
        menues[<1] = menue;
        break;
    case "u":
        if (member(menue, B_START_LINE)) {
            if (start <= menue[B_START_LINE]) {
                start = menue[B_START_LINE];
                browse_write_line("Anfang der Datei erreicht.\n");
            } else {
                start = max(menue[B_START_LINE], 
                        start - menue[B_NUM_LINES]);
            }
        } else if (start <= 1) {
            start = 1;
            browse_write_line("Anfang der Datei erreicht.\n");
        } else {
            start = max(1, start - menue[B_NUM_LINES]);
        }
        menue[B_CURRENT_LINE] = start;
        menues[<1] = menue;
        break;
    case "<":
        if (member(menue, B_START_LINE)) {
           start = menue[B_START_LINE];
        } else {
           start = 1;
        }
        menue[B_CURRENT_LINE] = start;
        menues[<1] = menue;
        break;
    case ">":
        if (menue[B_TYPE] == B_STATICMORE) 
        {
            end = sizeof(menue[B_DATA])-1; 
        }
        else 
        {
            call_resolved(&end, mobj, mtype+"_total", menue);
            if (end > 0 && menue[B_START_LINE]==0) end--; 
        }
        if (end >= 0) 
        {
            start = max(end - menue[B_NUM_LINES] + 1, menue[B_START_LINE] );
            menue[B_CURRENT_LINE] = start;
            menues[<1] = menue;
            break;
        }
        break; // ?
    case "q":
        return B_QUIT;
    case "?":
        if (member(menue, B_HELP) && mappingp(menue[B_HELP]) ) {
            mapping submenue = menue[B_HELP];
            if (member(submenue, B_FLAGS)) {
                submenue[B_FLAGS] |= BF_RETURN;
            } else {
                submenue[B_FLAGS] = BF_RETURN;
            }
            menues += ({ submenue });
        }
        break;
    case "r":
        break;
    case "/":
        if (search)
        {
            menue[B_SEARCH_STRING] = search;
        }
        else
        {
            if (!menue[B_SEARCH_STRING])
            {
                browse_write_line("Kein Suchstring angegeben!");
                menue[B_FLAGS] |= BF_NO_DISPLAY;
                menues[<1] = menue;
                break;
            }
        }
        if(call_resolved(&newmenue, mobj, mtype+"_search_forward", menue))
        {
            if (mappingp(newmenue))
            {
                menues[<1] = newmenue;
                break;
            } 
            else
            {
                browse_write_line("Nicht gefunden!");
                menue[B_FLAGS] |= BF_NO_DISPLAY;
                menues[<1] = menue;
                break;
            }
        }
        else
        {
            browse_write_line("Keine Suchfunktion implementiert.");
            menue[B_FLAGS] |= BF_NO_DISPLAY;
            menues[<1] = menue;
            break;
        }
    case "z":
        if (sizeof(menues)>1) 
        {
            menues = pop_menue(menues,1);
        } 
        else 
        {
            browse_write_line("Es geht nicht weiter zurück.\n");
            menue[B_FLAGS] |= BF_NO_DISPLAY;
            menues[<1] = menue;
        }
        break;
    default:
        if (sscanf(str,"%d",i)==1 && i > 0) 
        {
            if (call_resolved(&result, mobj, mtype+"_select", i, menue)) 
            {
                if (mappingp(result)) 
                {
                    menues += ({ result });
                    break;
                }
            }
        }        
    }
    dyn_browse_internal(menues);
    return B_CONTINUE;
}

private nomask string dyn_browse_get_header_line(mapping menue, int nr)
{
    if (nr<1 || nr>4 || !pointerp(menue[B_HEADER_LINES]) 
            || sizeof(menue[B_HEADER_LINES]) != 4)
    {
        return 0;
    }
    return menue[B_HEADER_LINES][nr-1];
}

// Testweise fuer Test in plugins
protected void dyn_input_to(mixed fun,int flags, varargs mixed* pars)
{
    input_to(fun, flags, pars...);
    // input_to("dyn_browse_input", INPUT_PROMPT, prompt, menues);
}

private nomask int dyn_browse_internal(mapping* menues)
{
    int end;
    string mtype,prompt,*display;
    <object|lwobject> mobj;
    mixed result;
    
  do
  {
    if (provide_type_and_object(menues[<1],&mtype,&mobj)==B_QUIT)
        return B_QUIT;
    notify_dyn_browse_data(menues[<1],menu_path);
    if (member(menues[<1],B_FLAGS) 
            && (menues[<1][B_FLAGS] & BF_MENU_PATH)!=0) 
    {
        if (call_resolved(&result, mobj, mtype+"_init", menues[<1])) {
            if (mappingp(result)) 
            {
                menues[<1] = result;
                menues[<1][B_FLAGS] &= ~BF_MENU_PATH;
            } 
            else if (result == B_QUIT) 
            {
                return 0; // notify_fail im Init.
            }
        }
    }
    if (member(menues[<1],B_FLAGS) && (menues[<1][B_FLAGS] & BF_DIRTY)!=0) 
    {
        if (call_resolved(&result, mobj, mtype+"_reload", menues)) 
        {
            if(pointerp(result) && sizeof(result)
                && sizeof(filter(result,(:mappingp($1):)))==sizeof(result))
            {
                 if (sizeof(result)!=sizeof(menues))
                 {
                     menues = result;
                     continue; // Menuwechsel bei unterschiedlicher Laenge
                 }
                 menues = result;
                 if (mtype != menues[<1][B_TYPE])
                 {
                    continue; // Menuwechsel zu einem anderen Menu
                 }
                 else
                 {
                    menues[<1][B_FLAGS] &= ~BF_DIRTY;
                 }
            } 
            else if (intp(result)) 
            {
                if (result != B_DONE)
                    return 1;
                else
                {
                    menues[<1][B_FLAGS] |= BF_NO_DISPLAY;
                    break; // Schleife verlassen
                }
            }
            else
            {
                result = filter(result,(:mappingp($1):));
                if (sizeof(result))
                {
                    menues = result; // Bereinigter Menuebaum.
                    menues[<1][B_FLAGS] &= ~BF_DIRTY;
                    continue;
                }
                else
                {
                    return 1;
                }
            }
        }
    }
    break; // Schleife verlassen.
  } while (1);
    if (menues[<1][B_TYPE] == B_STATICMORE) {
        menues[<1][B_START_LINE] = 0;
    }
    if (!member(menues[<1],B_CURRENT_LINE)) {
        if (member(menues[<1],B_START_LINE)) {
            menues[<1][B_CURRENT_LINE] = menues[<1][B_START_LINE];
        } else {
            menues[<1][B_CURRENT_LINE] = 1;
            menues[<1][B_START_LINE] = 1;
        }
    }
    menues[<1][B_NUM_LINES] = this_player()->query_more_chunk()
      - ((pointerp(menues[<1][B_HEADER_LINES]) 
            && sizeof(menues[<1][B_HEADER_LINES]) == 4) ? 3 : 1);

    if (menues[<1][B_FLAGS] & BF_NO_DISPLAY)
    {
        menues[<1][B_FLAGS] &= ~BF_NO_DISPLAY; // bit abloeschen.
    }
    else
    {
        //  JA, das soll einen Fehler werfen, wenn die fun nicht existiert.
        display = call_proved(mobj, mtype+"_display", menues[<1]);
        browse_write_line(dyn_browse_get_header_line(menues[<1],
                (menues[<1][B_CURRENT_LINE] > menues[<1][B_START_LINE])?3:1));
        if (menues[<1][B_TYPE] == B_STATICMORE) {
            end = sizeof(menues[<1][B_DATA]) - 1;
        } else if (call_resolved(&result, mobj, mtype+"_total", menues[<1])){
            end = result;
            if (end > 0 && menues[<1][B_START_LINE]==0) end--;
        } else {
            end = -1;
        }
        menues[<1][B_END_LINE] = end;
        if (pointerp(display) && sizeof(display)) {
            map(display, (: browse_write_line($1) :));
            if (end >= 0 && 
                   (menues[<1][B_CURRENT_LINE]+menues[<1][B_NUM_LINES])>=end) {
                browse_write_line(dyn_browse_get_header_line(menues[<1],2));
            } else {
                browse_write_line(dyn_browse_get_header_line(menues[<1],4));
            }
        } else {
            browse_write_line(dyn_browse_get_header_line(menues[<1],2));
        }
    }
    prompt = call_other(mobj, mtype+"_prompt", menues[<1]);
    if (!stringp(prompt) && stringp(menues[<1][B_PROMPT])) {
        prompt = sprintf(menues[<1][B_PROMPT],menues[<1][B_CURRENT_LINE]);
    } else if (!stringp(prompt)) {
        prompt = sprintf("%s-%d[u,d,q]",capitalize(mtype),
                         menues[<1][B_CURRENT_LINE]);
    }
    if (sizeof(prompt) && prompt[<1]!=' ')
    {
        prompt += " ";
    }
    dyn_input_to("dyn_browse_input", INPUT_PROMPT, prompt, menues);
    return 1;
}

nomask int dynamic_browse(mixed menues)
{
    string mtype,objstr;
    <object|lwobject> mobj = 0;
    mixed result;

    if (mappingp(menues)) {
        menues = ({ menues });
    } else if (pointerp(menues)) {
        if (sizeof(filter(menues,(: mappingp($1) :)))
               <sizeof(menues)) {
            menues = "Keine Menüs vorhanden.\n";
        }
    } else {
        menues = "Kein Menü vorhanden.\n";
    }
    if (stringp(menues)) {
        notify_fail(menues,FAIL_WRONG_ARG);
        return 0;
    }
    mtype = menues[<1][B_TYPE];
    if (member(menues[<1], B_OB) && objectp(menues[<1][B_OB]))
    {
        mobj = menues[<1][B_OB];
    }
    else if (member(menues[<1], B_OB_STR)) 
    {
        objstr = menues[<1][B_OB_STR];
        if (strstr(objstr,"#")>=0)
        {
            mobj = present_clone(objstr,this_player());
        }
        else
        {
            mobj = touch(objstr,NO_WRITE);
        }
    }
    else
    {
        mobj = this_object();
    }
    if (!stringp(mtype)) {
        notify_fail("Kein Menü identifiziert.\n",FAIL_WRONG_ARG);
        return 0;
    }
    if (!objectp(mobj)&&!lwobjectp(mobj)) {
        notify_fail("Menü-Objekt verloren.\n",FAIL_WRONG_ARG);
        return 0;
    }
    if (call_resolved(&result, mobj, mtype+"_init", menues[<1])) {
        if (mappingp(result)) {
            menues[<1] = result;
        } else if (result == B_QUIT) {
            return 0; // notify_fail im Init.
        }
    }
    return dyn_browse_internal(menues);
}

//------------------------------------------------------------------------
private int get_end_line(string file)
{
    int mid, low, up = 1;
    // Logarithmische Suche nach dem Dateiende
    // Erstmal die 2er Potenz finden, die hinter dem Dateiende liegt.
    while(read_file(file, up, 1))
        up <<= 1;
    // Die vorherige Potenz(low) war noch in Ordnung, up nicht.
    low = up >> 1;
    // Nun das Intervall halbieren
    mid = (up+low) >> 1;
 
    // Solange das Intervall nicht 0 lang ist
    while(mid != low)
    {
        if(read_file(file, mid, 1))
        {
            // Erfolg: die Mitte ist Anfang der gesuchten Intervallhaelfte
            low = mid;
        }
        else
        {
            // Misserfolg: die Mitte ist das Ende der Intervallhaelfte.
            up = mid;
        }
        // und die naechste Mitte finden:
        mid = (up+low) >> 1;
    }
    
    return mid;
}

/*
FUNKTION: dynamicmore
DEKLARATION: ([B_TYPE : "dynamicmore",B_FILE:"<pfad>"])
BESCHREIBUNG:
Mapping zum Aufruf an dynamic_browser, das Dateien beliebiger Laenge
anzeigen kann.
VERWEISE: dynamic_browser
GRUPPEN: dynamic_browse
*/
mixed dynamicmore_init(mapping old)
{
    string fn = old[B_FILE];
    if (!stringp(fn) || file_size(fn) <= 0) {
        notify_fail("Keine Datei gefunden.\n", FAIL_WRONG_ARG);
        return B_QUIT;
    }
    if (old[B_CURRENT_LINE] == -1)
    {
        old[B_CURRENT_LINE] = max(get_end_line(old[B_FILE])
                              - old[B_NUM_LINES],1);
        return old;
    }
    return B_NOTHING;
}

string * dynamicmore_display(mapping menue)
{
    if (!mappingp(menue) ) {
        return 0;
    }
    string fn = menue[B_FILE];
    if (!stringp(fn) || file_size(fn)<0) {
        return 0; 
    }
    if (menue[B_CURRENT_LINE] == -1)
    {
        menue[B_CURRENT_LINE] = max(get_end_line(menue[B_FILE])
                              - menue[B_NUM_LINES],1);
    }
    string content = read_file(fn,menue[B_CURRENT_LINE],menue[B_NUM_LINES]);
    if (!stringp(content)) {
        browse_write_line("---Ende der Datei---");
        return ({ });
    }
    int lines = sizeof(content & "\n");
    if (lines < menue[B_NUM_LINES])
        m_add(menue, DYNAMICMORE_TOTAL_LINES, menue[B_CURRENT_LINE] + lines);
    if (content[<1] == '\n')
        content=content[..<2];
    return explode(content,"\n");
}

int dynamicmore_total(mapping menue)
{
    return menue[DYNAMICMORE_TOTAL_LINES];
}

string dynamicmore_prompt(mapping menue)
{
    if (!mappingp(menue)) 
    {
        return 0;
    }
    string fn = menue[B_FILE];
    if (!stringp(fn) || file_size(fn)<0) 
    {
        return "[q]";
    }
    string name = explode(fn,"/")[<1];
    int offset = menue[B_CURRENT_LINE];
    int flag_help = member(menue, B_HELP);
    int flag_return = 0;
    if (member(menue,B_FLAGS) && ((menue[B_FLAGS]&BF_RETURN)!=0)) 
    {
        flag_return = 1;
    }
    if (member(menue,B_PROMPT))
    {
        name = menue[B_PROMPT];
    }
    return name + ":" + offset + " [u,d,<,>,"
            +(flag_return?"z,":"")
            +(flag_help?"?,":"")+"q]";
}
    
mixed dynamicmore_action(string str, mapping * menues)
{
    str = lower_case(space(str));
    if (!pointerp(menues) || sizeof(menues)==0 || !mappingp(menues[<1])) {
        return B_QUIT; // Abbruch, TODO Fehlercode??
    }
    mapping menue = menues[<1];
    int start = menue[B_CURRENT_LINE];
    int end;
    switch (str) {
    case "":
    case "d":
        start += menue[B_NUM_LINES];
        end = get_end_line(menue[B_FILE]); // TEUER bei grossen Dateien.
        if (start >= end) { start = end-1; } 
        menue[B_CURRENT_LINE] = start;
        menues[<1] = menue;
        return menues;
    case "u":
        if (start <= 1) {
            browse_write_line("Anfang der Datei erreicht.\n");
        }
        start = max(1, start - menue[B_NUM_LINES]);
        menue[B_CURRENT_LINE] = start;
        menues[<1] = menue;
        return menues;
    case ">":// TEUER bei grossen Dateien.
        menue[B_CURRENT_LINE] = max(get_end_line(menue[B_FILE])
                              - menue[B_NUM_LINES],1);
        menues[<1] = menue;
        return menues;
    case "<":
        menue[B_CURRENT_LINE] = 1;
        menues[<1] = menue;
        return menues;
    case "q":
        return B_QUIT; 
    case "z":
        if (sizeof(menues)>1) {
            menue[B_CURRENT_LINE] = 1;
            return pop_menue(menues,1);
        } else {
            browse_write_line("Es geht nicht weiter zurück.\n");
            return menues;
        }
    }
}

//------------------------------------------------------------------------
/*
FUNKTION: dynamicmore
DEKLARATION: ([B_TYPE : "staticmore",B_DATA:<string* displaydata>])
BESCHREIBUNG:
Mapping zum Aufruf an dynamic_browser, das Daten aus displaydata anzeigt.
VERWEISE: dynamic_browser,staticmore_display
GRUPPEN: dynamic_browse
*/
mixed staticmore_init(mapping old)
{
    if (!member(old,B_DATA) || !pointerp(old[B_DATA])) {
        notify_fail("Keine Daten gefunden.\n", FAIL_WRONG_ARG);
        return B_QUIT;
    }
    return B_NOTHING;
}

mixed staticmore_search_forward(mapping menue)
{
    if (!mappingp(menue) )
        return 0;
    int start = menue[B_CURRENT_LINE]+1;
    mixed * content = menue[B_DATA];
    int end = sizeof(content);
    int i;
    string search = menue[B_SEARCH_STRING];
    if (start > sizeof(content) || !stringp(search) || !sizeof(search))
        return 0;
    if (start < 0)
        start = 0;
    if (end <= start) 
        return 0;
    for (i=start; i < end ;i++)
    {
         if (strstr( content[i], search)!=-1)
         {
             menue[B_CURRENT_LINE] = i;
             return menue;
         }
    }
    return 0; // nicht gefunden
}

/*
FUNKTION: staticmore_display
DEKLARATION: string * staticmore_display(mapping menue)
BESCHREIBUNG:
Interne Funktion, die zur Anzeige von menue[B_DATA] unter Beruecksichtigung
von B_NUM_LINES und B_CURRENT_LINE genutzt werden kann.
VERWEISE: dynamic_browser,staticmore
GRUPPEN: dynamic_browse
*/
string * staticmore_display(mapping menue)
{
    if (!mappingp(menue) ) {
        return 0;
    }
    int start = menue[B_CURRENT_LINE];
    int end = start + menue[B_NUM_LINES];
    string * content = menue[B_DATA];
    if (start > sizeof(content)) {
        return 0;
    }
    if (end >= sizeof(content)) {
        end = sizeof(content)-1;
    }
    if (start < 0) {
        start = 0;
    }
    if (end < start) return 0;
    return content[start..end];
}

string staticmore_prompt(mapping menue)
{
    if (!mappingp(menue)) {
        return 0;
    }
    int start = menue[B_CURRENT_LINE];
    string * content = menue[B_DATA];
    int end = sizeof(content);
    int flag_help = member(menue, B_HELP);
    int flag_return = 0;
    string name = "";
    if (member(menue,B_FLAGS) && ((menue[B_FLAGS]&BF_RETURN)!=0)) {
        flag_return = 1;
    }
    if (member(menue,B_PROMPT))
    {
        name = menue[B_PROMPT];
        if (sizeof(name)>0 && name[<1]!=' ')
        {
            name += " ";
        }
    }    
    return name + start + "/" + end + " [u,d,<,>,"
            +(flag_return?"z,":"")
            +(flag_help?"?,":"")+"q]";
}
    
mixed staticmore_action(string str, mapping * menues)
{
    str = lower_case(space(str));
    if (!pointerp(menues) || sizeof(menues)==0 || !mappingp(menues[<1])) {
        return B_QUIT; // Abbruch, TODO Fehlercode??
    }
    mapping menue = menues[<1];
    int start = menue[B_CURRENT_LINE];
    int end = sizeof(menue[B_DATA]);
    switch (str) {
    case "":
    case "d":
        start += menue[B_NUM_LINES];
        if (start >= end) { start = end-1; }
        menue[B_CURRENT_LINE] = start;
        menues[<1] = menue;
        return menues;
    case "u":
        if (start <= 0) {
            browse_write_line("Anfang der Daten erreicht.\n");
        }
        start = max(0, start - menue[B_NUM_LINES]);
        menue[B_CURRENT_LINE] = start;
        menues[<1] = menue;
        return menues;
    case ">":
        menue[B_CURRENT_LINE] = max(sizeof(menue[B_DATA])
                              - menue[B_NUM_LINES],1);
        menues[<1] = menue;
        return menues;
    case "<":
        menue[B_CURRENT_LINE] = 0;
        menues[<1] = menue;
        return menues;
    case "q":
        return B_QUIT; 
    case "z":
        if (sizeof(menues)>1) {
            menue[B_CURRENT_LINE] = 0;
            return pop_menue(menues,1);
        } else {
            browse_write_line("Es geht nicht weiter zurück.\n");
            return menues;
        }
    }
}

//----------------------------------------------------------------------------
/*
BEISPIEL: menu_dynamic_browse
// <type> durch den effektiven Menuenamen ersetzen
// Doku siehe ? dynamic_browse
static mixed <type>_init(mapping old)
{
    // optional zum Initialisieren des Menues
}

mixed <type>_reload(mixed menues)
{
    // optional, wenn waehrend der Anzeige ein reload notwendig wird.
    // z.B. ein neuer Aufruf von <type>_init.
}

static int <type>_total(mapping menue)
{
    // optional dient zur Endeerkennung.
}

static string * <type>_display(mapping menue)
{
    // Pflicht, Zeigt den aktuellen Ausschnitt an.
}

static string <type>_prompt(mapping menue)
{
    // optional zur Anzeige des Menue-Prompts.
}

static mixed <type>_action(string str, mapping * menues)
{
    // Die Reaktionen des Menues.
    return B_NOTHING;
}
GRUPPEN: dynamic_browse
*/
