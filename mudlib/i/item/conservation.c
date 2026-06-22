// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/i/item/conservation.c
// Description:	Datenhandling fuer das Einlagern von Standardobjekten.
// Author:	Myonara

// set_-Funktionen mit ihren Argumenten
private static mapping setters = ([]);
// Beispiel: 
// (["life": ({ ({"set_life",({100})}), ({"add_life",({-5}) }),...}),...])
private static mapping sequences = ([]);
// Generell sind Objekte nicht einlagerbar. (GENERAL_NO)
// da die Pruefung von v_items aufwendig ist, wird das initial gesperrt.
private static mapping constraints = 
   (["GENERAL_NO" : "INITIAL", "add_v_item" : "INITIAL" ]);
// Controllerlogik: initiale Controller sollen ignoriert werden:
private static mapping initial_ctrls;

public mapping query_initial_ctrls()
{
    return initial_ctrls; // zu debugzwecken.
}

// interne Vergleichsfunktion, ob es mehr als die initialen Controller gibt.
// Sonderfaelle: enthaltene 0-Werte sind beiderseits zu ignorieren
private int count_more_then_initial_controllers()
{
    string key;
    mixed value1;mixed value2;
    mapping ctrls = deep_copy(this_object()->query_controller());
    if (!ctrls) return 0;
    ctrls = filter(ctrls,(: $2!= 0 :));
    if (!initial_ctrls) return sizeof(ctrls);
    mapping old_keys = ([]);
    foreach (key,value1 : deep_copy(initial_ctrls))
    {
        //DEBUG(sprintf("\nkey:%Q value1: %Q",key,value1));
        if (member(ctrls,key))
        {
            value2 = ctrls[key];
            //DEBUG(sprintf("value2:%Q",value2));
            if (pointerp(value1))
            {
                value1 -= ({0});
                if (pointerp(value2))
                {
                    value2 -= ({0});
                    if (sizeof(value2-value1)>0)
                    {
                        return -4;//cnt++;
                    }
                    else
                    {
                        old_keys += ([key:1]);
                    }
                    continue;
                }
                else if (value2 == 0)
                {
                    continue;
                }
                else
                {
                    if (member(value1,value2)==-1)
                    {
                        return -3;//cnt++;
                    }
                    else
                    {
                        old_keys += ([key:1]);
                    }
                    continue;
                }
            }
            else if (value1 == 0)
            {
                continue;
            }
            else
            {
                if (pointerp(value2))
                {
                    value2 -= ({0});
                    if (sizeof(value2-({value1}))==0)
                    {
                        old_keys += ([key:1]);
                    }
                    else
                    {
                        return -2;//cnt++;
                    }
                    continue;
                }
                else if (value2 == 0)
                {
                    continue;
                }
                else
                {
                    if (value1 == value2)
                    {
                        old_keys += ([key:1]);
                    }
                    else
                    {
                        return -1;//cnt++;
                    }
                    continue;
                }
            }
        }
    }
    return sizeof(ctrls-old_keys);
}

/*
FUNKTION: add_setter_conservation
DEKLARATION: varargs void add_setter_conservation(string setter,mixed * args,string sequence)
BESCHREIBUNG:
Fuegt Daten zur Speicherung von Standardobjekten hinzu, in zwei Modi:
- sequence == 0: die set-Funktion setter wird in einem internen Mapping 
  gespeichert. Die Reihenfolge in Bezug auf andere setters ist irrelevant.
- stringp(sequence): Innerhalb der sequence wird die Reihenfolge
  beibehalten. (Beispiel set_life folgend von mehreren add_life)
GRUPPEN: armageddon
VERWEISE: delete_seq_conservation,query_conservation_data
*/
varargs void add_setter_conservation(string setter,mixed * args, 
                  string sequence)
{
    if (living(this_object())) return; // nur lebloses!
    // Sequenzen wie add_v_item oder set_live/add_live werden in sich
    // der Reihe nach abgespeichert,
    // ein neues set_live loescht erst mit delete_seq_conservation
    // damit von vorne begonnen werden kann.
    if (stringp(sequence))
    {
        if (member(sequences,sequence))
        {
            sequences[sequence] += ({ ({setter,args}) });
        }
        else
        {
            sequences[sequence] = ({ ({setter,args}) });
        }
    }
    else
    {
        // bei den set-Funktionen ist die Reihenfolge egal, hoffe ich.
        setters[setter] = args;
    }
}

// Funktion fuers schliessfaecher-Objekt beim auslagern
mapping* backup_conservation_setters()
{
    return deep_copy(({ setters,sequences }));
}

// Funktion fuers schliessfaecher-Objekt beim auslagern
void restore_conservation_setters(mapping * old_setters)
{
    if (!old_setters || sizeof(old_setters)<2 
        || !mappingp(old_setters[0])|| !mappingp(old_setters[1])) return;
    setters = old_setters[0] || setters;
    sequences = old_setters[1] || sequences;
}

/*
FUNKTION: delete_seq_conservation
DEKLARATION: void delete_seq_conservation(string sequence)
BESCHREIBUNG:
Wenn eine Sequenz mit set neu initialisiert wird, so muss diese Sequenz
vor dem add_setter_conservation mittels dieser Funktion geloescht werden.
GRUPPEN: armageddon
VERWEISE: add_setter_conservation
*/
void delete_seq_conservation(string sequence)
{
    if (living(this_object())) return; // nur lebloses!
    if (member(sequences,sequence))
        m_delete(sequences,sequence);
}

/*
FUNKTION: set_conservation_constraint
DEKLARATION: void set_conservation_constraint(string setter, mixed args)
BESCHREIBUNG:
Werden nicht statische oder abspeicherbare Veraenderungen an dem Objekt
vorgenommen, so wird mittels set_conservation_constraint das Objekt
als nicht einlagerbar geflagt (Args!=0) oder der Constraint wieder 
geloescht (args==0).
GRUPPEN: armageddon
VERWEISE: query_conservation_constraints,query_conservation_constraint
*/
void set_conservation_constraint(string setter, mixed args)
{
    if (living(this_object())) return; // nur lebloses!
    if (args == 0)
    {
        if (member(constraints,setter))
            m_delete(constraints,setter);
    }
    else
    {
        constraints[setter] = args;
    }
}

/*
FUNKTION: clear_initial_conservation_data
DEKLARATION: void clear_initial_conservation_data()
BESCHREIBUNG:
Diese Funktion setzt die iniitalen Daten und Constraints zurueck.
Es gibt drei Einsatzfaelle:
- Am Ende des Creates in einer .../obj/... Datei.
- Am Ende des Creates eines Inherits, welches mittels replace_programm
  genutzt wird, in folgender Form:
  if (program_name(this_object()) == __FILE__) 
      clear_initial_conservation_data();
- Am Ende der Initialisierung durch eine Factory.
In allen anderen Faellen ist diese Funktion nicht empfohlen, da sonst
gesetzte Daten verloren gehen koennen.
GRUPPEN: armageddon
VERWEISE: add_setter_conservation, set_conservation_constraint
*/
void clear_initial_conservation_data()
{
    initial_ctrls = deep_copy(this_object()->query_controller());
    setters = ([]);
    sequences = ([]);
    constraints = ([]);
}

/*
FUNKTION: query_conservation_data
DEKLARATION: mixed* query_conservation_data()
BESCHREIBUNG:
Es werden die mit add_setter_conservation gesetzten Daten zurueckgeliefert.
Zuerst die Elemente ohne Sequenz und dann die Sequenzen.
GRUPPEN: armageddon
VERWEISE: add_setter_conservation
*/
mixed* query_conservation_data()
{
    mixed * result = transpose_array(unmkmapping(setters)), * val;
    string seq;

    foreach (seq,val : sequences)
    {
        result += val;
    }
    return result;
    // wird vor dem einlagern aufgerufen...
}

/*
FUNKTION: query_conservation_constraint
DEKLARATION: mixed query_conservation_constraint(string key)
BESCHREIBUNG:
Liefert einen Constraint mit Schluessel key zurueck, falls vorhanden,
0 sonst.
GRUPPEN: armageddon
VERWEISE: set_conservation_constraint, query_conservation_constraints
*/
mixed query_conservation_constraint(string key)
{
    if (key == "CONTROLLER")
        return count_more_then_initial_controllers();
    return constraints[key];
}

/*
FUNKTION: query_conservation_constraints
DEKLARATION: mixed query_conservation_constraints()
BESCHREIBUNG:
Liefert die Gesamtheit alle Constraints zurueck.
GRUPPEN: armageddon
VERWEISE: set_conservation_constraint, query_conservation_constraint
*/
mapping query_conservation_constraints()
{
    int cnt = count_more_then_initial_controllers();
    if (cnt)
        return constraints +(["CONTROLLER": cnt]);
    else
        return constraints;
    // auch bei einlagern und query_debug_info.
}
protected mapping query_saved_properties();

mapping query_saved_properties_for_conservation()
{
    if (program_name(previous_object()) != "/apps/zentralbank.c")
        return 0;
    return query_saved_properties();
}
protected void set_saved_properties(mapping m);
void set_saved_properties_for_conservation(mapping m)
{
    if (program_name(previous_object()) != "/apps/zentralbank.c")
        return;
    set_saved_properties(m);
}