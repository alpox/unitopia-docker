// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/obj/money.c
// Description:

inherit "/i/money/money";

varargs void create(int wert, string valuta)
{
    replace_program("/i/money/money");
    ::create(wert, valuta);
}
