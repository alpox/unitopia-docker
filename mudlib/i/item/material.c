// ----------------------------------------------------------------
// File:	/i/item/material.c
// Description: Materialhandling
// Author:	Aeneas (23.12.93)

#pragma save_types
#pragma strong_types

private string *material;

#define SAVE_MATERIAL_TO_CONS this_object()->add_setter_conservation( \
    "set_material",({material}) )

/*
FUNKTION: set_material
DEKLARATION: void set_material(<string|string*> materialien)
BESCHREIBUNG:
Setzt die wichtigsten Grundbestandteile, aus denen das Objekt besteht.
Alle erlaubten Materialien stehen in /doc/richtlinien/materialien.
Uebergeben kann ein einzelnes Material oder eine Liste von Materialien.
VERWEISE: material, add_material, delete_material, query_material
GRUPPEN: grundlegendes
*/
void set_material(<string|string*> str)
{
    if (pointerp(str))
	material = str;
    else if (stringp(str))
	material = ({ str });
    SAVE_MATERIAL_TO_CONS;
}

/*
FUNKTION: add_material
DEKLARATION: void add_material(<string|string*> materialien)
BESCHREIBUNG:
Fuegt weitere Materialien hinzu.
Alle erlaubten Materialien stehen in /doc/richtlinien/materialien.
Uebergeben kann ein einzelnes Material oder eine Liste von Materialien.
VERWEISE: material, set_material, delete_material, query_material
GRUPPEN: grundlegendes
*/
void add_material(<string|string*> str)
{
    if (!pointerp(str) && !stringp(str))
	return;
    if (!material)
	material = ({});
    if (stringp(str))
	material = (material - ({ str })) + ({ str });
    else
	material = (material - str) + str;
    SAVE_MATERIAL_TO_CONS;
}

/*
FUNKTION: query_material
DEKLARATION: string *query_material()
BESCHREIBUNG:
Gibt alle Material-Typen zurueck, aus denen das Objekt besteht.
Naeheres siehe /doc/funktionsweisen/material.
VERWEISE: material, set_material, add_material, delete_material
GRUPPEN: grundlegendes
*/
string *query_material()
{
    return material || ({});
}

/*
FUNKTION: material
DEKLARATION: int material(string m)
BESCHREIBUNG:
Gibt 1 zurueck, wenn m in der Material-Liste des Objektes auftaucht,
ansonsten 0. Naeheres siehe /doc/funktionsweisen/material.
VERWEISE: set_material, add_material, delete_material, query_material
GRUPPEN: grundlegendes
*/
int material(string str)
{
    return member(this_object()->query_material() || ({}), str) >= 0;
}

/*
FUNKTION: delete_material
DEKLARATION: void delete_material(<string|string*> materialien)
BESCHREIBUNG:
Loescht einzelne Material-Typen aus der Material-Liste des Objektes.
Es kann ein einzelnes Material oder eine Liste von Materialien
entfernt werden.
VERWEISE: material, set_material, add_material, query_material
GRUPPEN: grundlegendes
*/
void delete_material(<string|string*> strs)
{
    if (pointerp(material))
    {
        if (pointerp(strs))
            material -= strs;
        else if (stringp(strs))
            material -= ({ strs });
    }
    SAVE_MATERIAL_TO_CONS;
}
