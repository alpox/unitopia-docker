// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /i/tools/kleidung.c
// Description: Informationen zu Kleidungstypen
// Author:      Gnomi

private functions inherit "/i/tools/config";

#include <error.h>

struct clothes_type
{
    int quantity;
    int quantity_per_hand;
    int order;

    string* count_types;
    string* hide_types;
};

private mapping read_kleidungstypen()
{
    mapping result = ([:1]);

    if (program_name() != __FILE__)
        return 0;

    foreach(string name, mapping info: read_config("/static/adm/KLEIDUNGEN"))
    {
        struct clothes_type entry = (<clothes_type> 1, 0, __INT_MAX__, ({}), ({}));
        string val;

        if (sizeof(val = info["quantity"]))
        {
            if (val[<1]=='h')
            {
                entry.quantity_per_hand = 1;
                entry.quantity = to_int(to_int(val[..<2]));
            }
            else
                entry.quantity = to_int(to_int(val));
        }
        if (sizeof(val = info["order"]))
            entry.order = to_int(val);

        if (sizeof(val = info["count"]))
            entry.count_types = map(explode(val,","),#'trim);
        if (sizeof(val = info["hides"]))
            entry.hide_types = map(explode(val,","),#'trim);

        if (sizeof(info - ({"quantity","count","hides","order"})))
            do_warning("Unbekannte Einträge für " + name+ ".\n");

        m_add(result, name, entry);
    }

    // Als nächstes prüfen wir, ob nur bekannte Kleidungstypen verwendet wurden.
    foreach (string name, struct clothes_type entry: result)
    {
        foreach (string unknown: entry.count_types - result)
            do_warning(sprintf("Kleidungstyp '%s' verweist auf unbekannten Kleidungstyp '%s'.\n", name, unknown));
        foreach (string unknown: entry.hide_types - result)
            do_warning(sprintf("Kleidungstyp '%s' verweist auf unbekannten Kleidungstyp '%s'.\n", name, unknown));
    }

    // Die transitive Hülle für 'hide_types' berechnen.
    foreach (string name, struct clothes_type entry: result)
    {
        string *all = ({});
        string *rest = entry.hide_types;

        while (sizeof(rest))
        {
            string hide = rest[0];
            struct clothes_type next = result[hide];

            all += ({hide});
            rest = (sizeof(rest)>1 ? rest[1..] : ({})) + (next.hide_types - all - rest);
        }

        entry.hide_types = all;
    }

    return result;
}

private nosave mapping kleidungstypen = read_kleidungstypen();

/*
FUNKTION: query_clothes_info
DEKLARATION: struct clothes_type query_clothes_info(string type)
BESCHREIBUNG:
Liefert Informationen zum angegebenen Kleidungstyp zurück.
Ein Rückgabewert von 0 indiziert einen nicht genehmigten Kleidungstyp.
Ansonsten wird eine Struktur mit folgenden Einträgen geliefert:

    int quantity                Erlaubte Anzahl von Kleidungsstücken,
                                die man gleichzeitig tragen darf.

    int quantity_per_hand       Falls 1, so wird quantity pro Hand gezählt.

    int order                   Definiert die Anzeigereihenfolge.

    string* count_types         Eine Liste von Kleidungstypen, die bei obiger
                                quantity mitgezählt werden.

    string* hide_types          Eine Liste von Kleidungstypen, die beim
                                Betrachten von dieser Kleidung verdeckt werden.
VERWEISE: query_typ, query_armour_info
GRUPPEN: kleidung
*/
struct clothes_type query_clothes_info(string type)
{
    if (program_name() == __FILE__)
        return kleidungstypen && kleidungstypen[type];
    else
        return __FILE__.query_clothes_info(type);
}

/*
FUNKTION: query_armour_info
DEKLARATION: struct clothes_type query_armour_info(string armour_class)
BESCHREIBUNG:
Liefert Informationen zur angegebenen Rüstungsklasse zurück.
Die Informationen sind äquivalent zu query_clothes_info().
VERWEISE: query_armour_class, query_clothes_info
GRUPPEN: kleidung
*/
struct clothes_type query_armour_info(string armour_class)
{
    return query_clothes_info("armour:" + armour_class);
}


/*
FUNKTION: determine_obvious_clothes
DEKLARATION: object* determine_obvious_clothes(object *obs)
BESCHREIBUNG:
Liefert aus der angegebenen Objektliste diejenigen, die sichtbar sind, also
nicht von anderen Kleidungsstücken aus der Liste verdeckt werden.
VERWEISE: query_typ
GRUPPEN: kleidung
*/
object* determine_obvious_clothes(object *obs)
{
    mapping typen = ([:2]);
    object *result = ({});

    // Objekte nach Typen sortieren.
    foreach (object ob: obs)
    {
        string typ = ob->query_typ();
        if (!typ)
        {
            typ = ob->query_armour_class();
            if (typ)
                typ = "armour:" + typ;
            else
            {
                result += ({ob});
                continue;
            }
        }

        typen[typ] = (typen[typ] || ({})) + ({ ob });
    }

    // Verdeckte Objekte aussortieren.
    foreach (string typ, object* cl, struct clothes_type info: &typen)
    {
        info = query_clothes_info(typ);
        if (!info)
        {
            result += cl;
            m_delete(typen, typ);
            continue;
        }

        typen -= info.hide_types;

        if (member(info.hide_types, typ) >= 0) /* Verdeckt sich selbst? Dann überlebt ein Objekt. */
            m_add(typen, typ, cl[0..0], info);
    }

    // Verbleibende Objekte sortieren.
    foreach (string typ: sort_array(m_indices(typen), function int(string a, string b) { return typen[a,1].order > typen[b,1].order; }))
        result += typen[typ,0];

    return result;
}
