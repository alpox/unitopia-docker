// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /i/tools/deklin_message.c
// Author:      Gnomi
// Description: Grammatikfunktionen für mehrere Empfänger

#if __EFUN_DEFINED__(create_deklin_message)
#include <apps.h>
#include <deklin.h>

deklin_message query_deklin_owner(
   <object|mapping|int> who   = OBJ_TO, int art  = ART_DER, int fall = FALL_NOM, string | <string|int|<string|int>*>* | int adj  = 0,
   <object|mapping|int> owner = OBJ_TO, int oart = ART_DER,                      string | <string|int|<string|int>*>* | int oadj = 0);

deklin_message query_deklin(object|mapping|int who = OBJ_TO, int art = ART_DER, int fall = FALL_NOM, string | <string|int|<string|int>*>* | int adj = 0, object|mapping|int owner = 0);

private object|mapping parse_deklin_object(object|mapping|int who)
{
   if(intp(who))
      if(who < 0)
         who = previous_object(-who);
      else switch(who)
      {
         case OBJ_TO : /* 0 */
            who = this_object();  break;
         case OBJ_PO : /* 1 */
            who = previous_object(); break;
         case OBJ_TP : /* 2 */
            who = this_player();      break;
         case OBJ_OW : /* 3 */
            who = auto_owner_search(this_object());
            break;
         case OBJ_TI : /* 4 */
            who = this_interactive(); break;
         default:
            who = this_object();
      }
   return who;
}

object auto_owner_search(object|mapping ob)
{
    return QUERY("real_owner", ob)  || sefun::auto_owner_search(ob);
}

deklin_message query_deklin(object|mapping|int who, int art, int fall, string | <string|int|<string|int>*>* | int adj, object|mapping|int owner)
{
    mapping result = ([]);

    who = parse_deklin_object(who);
    if (owner)
        owner = parse_deklin_object(owner);

    if ((art & ART_MASK) == ART_SEIN)
    {
        object r_owner = QUERY("real_owner", who);
        if (!owner)
            owner = sefun::auto_owner_search(who);

        if (r_owner && r_owner != owner)
            return query_deklin_owner(who, ART_DER | (art&ART_MASK), fall, adj, r_owner);

        if (objectp(owner))
            m_add(result, owner, sefun::query_deklin(who, ART_DEIN | (art & ~ART_MASK), fall, adj, owner));
    }
    m_add(result, 0, sefun::query_deklin(who, art, fall, adj, owner));
    if (objectp(who))
        m_add(result, who, sefun::query_deklin(who, ART_DU | (art & ART_CAPITALIZE), fall));

    return create_deklin_message(result);
}

deklin_message query_deklin_owner(
   <object|mapping|int> who  , int art , int fall, string | <string|int|<string|int>*>* | int adj,
   <object|mapping|int> owner, int oart,           string | <string|int|<string|int>*>* | int oadj)
{
    mapping result = ([]);

    who = parse_deklin_object(who);
    if (owner)
        owner = parse_deklin_object(owner);
    else
        owner = auto_owner_search(who);

    if (!owner)
        return query_deklin(who, ART_AAA | (art & ART_CAPITALIZE), fall, adj);

    m_add(result, 0, sefun::query_deklin_owner(who, art, fall, adj, owner, oart, oadj));
    if (objectp(who))
        m_add(result, who, sefun::query_deklin(who, ART_DU | (art & ART_CAPITALIZE), fall));
    if (objectp(owner))
        m_add(result, owner, sefun::query_deklin(who, ART_DEIN | (art & ~ART_MASK), fall, adj, owner));

    return create_deklin_message(result);
}

deklin_message    wer(object|mapping|int who = OBJ_TO, int art = ART_DER, string|<string|int|<string|int>*>*|int adj = 0, object|mapping|int owner = 0) { return query_deklin(who, art, FALL_NOM, adj, owner); }
deklin_message    Wer(object|mapping|int who = OBJ_TO, int art = ART_DER, string|<string|int|<string|int>*>*|int adj = 0, object|mapping|int owner = 0) { return query_deklin(who, art | ART_CAPITALIZE, FALL_NOM, adj, owner); }
deklin_message wessen(object|mapping|int who = OBJ_TO, int art = ART_DER, string|<string|int|<string|int>*>*|int adj = 0, object|mapping|int owner = 0) { return query_deklin(who, art, FALL_GEN, adj, owner); }
deklin_message Wessen(object|mapping|int who = OBJ_TO, int art = ART_DER, string|<string|int|<string|int>*>*|int adj = 0, object|mapping|int owner = 0) { return query_deklin(who, art | ART_CAPITALIZE, FALL_GEN, adj, owner); }
deklin_message    wem(object|mapping|int who = OBJ_TO, int art = ART_DER, string|<string|int|<string|int>*>*|int adj = 0, object|mapping|int owner = 0) { return query_deklin(who, art, FALL_DAT, adj, owner); }
deklin_message    Wem(object|mapping|int who = OBJ_TO, int art = ART_DER, string|<string|int|<string|int>*>*|int adj = 0, object|mapping|int owner = 0) { return query_deklin(who, art | ART_CAPITALIZE, FALL_DAT, adj, owner); }
deklin_message    wen(object|mapping|int who = OBJ_TO, int art = ART_DER, string|<string|int|<string|int>*>*|int adj = 0, object|mapping|int owner = 0) { return query_deklin(who, art, FALL_AKK, adj, owner); }
deklin_message    Wen(object|mapping|int who = OBJ_TO, int art = ART_DER, string|<string|int|<string|int>*>*|int adj = 0, object|mapping|int owner = 0) { return query_deklin(who, art | ART_CAPITALIZE, FALL_AKK, adj, owner); }
deklin_message    der(object|mapping|int who = OBJ_TO,                    string|<string|int|<string|int>*>*|int adj = 0) { return wer(who, ART_DER, adj); }
deklin_message    Der(object|mapping|int who = OBJ_TO,                    string|<string|int|<string|int>*>*|int adj = 0) { return Wer(who, ART_DER, adj); }
deklin_message    des(object|mapping|int who = OBJ_TO,                    string|<string|int|<string|int>*>*|int adj = 0) { return wessen(who, ART_DER, adj); }
deklin_message    Des(object|mapping|int who = OBJ_TO,                    string|<string|int|<string|int>*>*|int adj = 0) { return Wessen(who, ART_DER, adj); }
deklin_message    dem(object|mapping|int who = OBJ_TO,                    string|<string|int|<string|int>*>*|int adj = 0) { return wem(who, ART_DER, adj); }
deklin_message    Dem(object|mapping|int who = OBJ_TO,                    string|<string|int|<string|int>*>*|int adj = 0) { return Wem(who, ART_DER, adj); }
deklin_message    den(object|mapping|int who = OBJ_TO,                    string|<string|int|<string|int>*>*|int adj = 0) { return wen(who, ART_DER, adj); }
deklin_message    Den(object|mapping|int who = OBJ_TO,                    string|<string|int|<string|int>*>*|int adj = 0) { return Wen(who, ART_DER, adj); }
deklin_message    ein(object|mapping|int who = OBJ_TO,                    string|<string|int|<string|int>*>*|int adj = 0) { return wer(who, ART_EIN, adj); }
deklin_message    Ein(object|mapping|int who = OBJ_TO,                    string|<string|int|<string|int>*>*|int adj = 0) { return Wer(who, ART_EIN, adj); }
deklin_message  eines(object|mapping|int who = OBJ_TO,                    string|<string|int|<string|int>*>*|int adj = 0) { return wessen(who, ART_EIN, adj); }
deklin_message  Eines(object|mapping|int who = OBJ_TO,                    string|<string|int|<string|int>*>*|int adj = 0) { return Wessen(who, ART_EIN, adj); }
deklin_message  einem(object|mapping|int who = OBJ_TO,                    string|<string|int|<string|int>*>*|int adj = 0) { return wem(who, ART_EIN, adj); }
deklin_message  Einem(object|mapping|int who = OBJ_TO,                    string|<string|int|<string|int>*>*|int adj = 0) { return Wem(who, ART_EIN, adj); }
deklin_message  einen(object|mapping|int who = OBJ_TO,                    string|<string|int|<string|int>*>*|int adj = 0) { return wen(who, ART_EIN, adj); }
deklin_message  Einen(object|mapping|int who = OBJ_TO,                    string|<string|int|<string|int>*>*|int adj = 0) { return Wen(who, ART_EIN, adj); }
deklin_message   dein(object|mapping|int who = OBJ_TO,                    string|<string|int|<string|int>*>*|int adj = 0, object|mapping|int owner = 0) { return wer(who, ART_DEIN, adj, owner); }
deklin_message   Dein(object|mapping|int who = OBJ_TO,                    string|<string|int|<string|int>*>*|int adj = 0, object|mapping|int owner = 0) { return Wer(who, ART_DEIN, adj, owner); }
deklin_message deines(object|mapping|int who = OBJ_TO,                    string|<string|int|<string|int>*>*|int adj = 0, object|mapping|int owner = 0) { return wessen(who, ART_DEIN, adj, owner); }
deklin_message Deines(object|mapping|int who = OBJ_TO,                    string|<string|int|<string|int>*>*|int adj = 0, object|mapping|int owner = 0) { return Wessen(who, ART_DEIN, adj, owner); }
deklin_message deinem(object|mapping|int who = OBJ_TO,                    string|<string|int|<string|int>*>*|int adj = 0, object|mapping|int owner = 0) { return wem(who, ART_DEIN, adj, owner); }
deklin_message Deinem(object|mapping|int who = OBJ_TO,                    string|<string|int|<string|int>*>*|int adj = 0, object|mapping|int owner = 0) { return Wem(who, ART_DEIN, adj, owner); }
deklin_message deinen(object|mapping|int who = OBJ_TO,                    string|<string|int|<string|int>*>*|int adj = 0, object|mapping|int owner = 0) { return wen(who, ART_DEIN, adj, owner); }
deklin_message Deinen(object|mapping|int who = OBJ_TO,                    string|<string|int|<string|int>*>*|int adj = 0, object|mapping|int owner = 0) { return Wen(who, ART_DEIN, adj, owner); }
deklin_message   sein(object|mapping|int who = OBJ_TO,                    string|<string|int|<string|int>*>*|int adj = 0, object|mapping|int owner = 0) { return wer(who, ART_SEIN, adj, owner); }
deklin_message   Sein(object|mapping|int who = OBJ_TO,                    string|<string|int|<string|int>*>*|int adj = 0, object|mapping|int owner = 0) { return Wer(who, ART_SEIN, adj, owner); }
deklin_message seines(object|mapping|int who = OBJ_TO,                    string|<string|int|<string|int>*>*|int adj = 0, object|mapping|int owner = 0) { return wessen(who, ART_SEIN, adj, owner); }
deklin_message Seines(object|mapping|int who = OBJ_TO,                    string|<string|int|<string|int>*>*|int adj = 0, object|mapping|int owner = 0) { return Wessen(who, ART_SEIN, adj, owner); }
deklin_message seinem(object|mapping|int who = OBJ_TO,                    string|<string|int|<string|int>*>*|int adj = 0, object|mapping|int owner = 0) { return wem(who, ART_SEIN, adj, owner); }
deklin_message Seinem(object|mapping|int who = OBJ_TO,                    string|<string|int|<string|int>*>*|int adj = 0, object|mapping|int owner = 0) { return Wem(who, ART_SEIN, adj, owner); }
deklin_message seinen(object|mapping|int who = OBJ_TO,                    string|<string|int|<string|int>*>*|int adj = 0, object|mapping|int owner = 0) { return wen(who, ART_SEIN, adj, owner); }
deklin_message Seinen(object|mapping|int who = OBJ_TO,                    string|<string|int|<string|int>*>*|int adj = 0, object|mapping|int owner = 0) { return Wen(who, ART_SEIN, adj, owner); }
deklin_message dieser(object|mapping|int who = OBJ_TO,                    string|<string|int|<string|int>*>*|int adj = 0) { return wer(who, ART_DIESER, adj); }
deklin_message Dieser(object|mapping|int who = OBJ_TO,                    string|<string|int|<string|int>*>*|int adj = 0) { return Wer(who, ART_DIESER, adj); }
deklin_message dieses(object|mapping|int who = OBJ_TO,                    string|<string|int|<string|int>*>*|int adj = 0) { return wessen(who, ART_DIESER, adj); }
deklin_message Dieses(object|mapping|int who = OBJ_TO,                    string|<string|int|<string|int>*>*|int adj = 0) { return Wessen(who, ART_DIESER, adj); }
deklin_message diesem(object|mapping|int who = OBJ_TO,                    string|<string|int|<string|int>*>*|int adj = 0) { return wem(who, ART_DIESER, adj); }
deklin_message Diesem(object|mapping|int who = OBJ_TO,                    string|<string|int|<string|int>*>*|int adj = 0) { return Wem(who, ART_DIESER, adj); }
deklin_message diesen(object|mapping|int who = OBJ_TO,                    string|<string|int|<string|int>*>*|int adj = 0) { return wen(who, ART_DIESER, adj); }
deklin_message Diesen(object|mapping|int who = OBJ_TO,                    string|<string|int|<string|int>*>*|int adj = 0) { return Wen(who, ART_DIESER, adj); }
deklin_message     er(object|mapping|int who = OBJ_TO) { return wer(who, ART_ER); }
deklin_message     Er(object|mapping|int who = OBJ_TO) { return Wer(who, ART_ER); }
deklin_message    ihm(object|mapping|int who = OBJ_TO) { return wem(who, ART_ER); }
deklin_message    Ihm(object|mapping|int who = OBJ_TO) { return Wem(who, ART_ER); }
deklin_message    ihn(object|mapping|int who = OBJ_TO) { return wen(who, ART_ER); }
deklin_message    Ihn(object|mapping|int who = OBJ_TO) { return Wen(who ,ART_ER); }
deklin_message    ihr(object|mapping|int who = OBJ_TO, string|string*|int|int* who_adj = 0, object|mapping|int owner = 0, string|string*|int|int* owner_adj = 0, int who_art = ART_DER, int owner_art = ART_AAA) { return query_deklin_owner(who, who_art, FALL_NOM, who_adj, owner, owner_art, owner_adj); }
deklin_message    Ihr(object|mapping|int who = OBJ_TO, string|string*|int|int* who_adj = 0, object|mapping|int owner = 0, string|string*|int|int* owner_adj = 0, int who_art = ART_DER, int owner_art = ART_AAA) { return query_deklin_owner(who, who_art | ART_CAPITALIZE, FALL_NOM, who_adj, owner, owner_art, owner_adj); }
deklin_message  ihres(object|mapping|int who = OBJ_TO, string|string*|int|int* who_adj = 0, object|mapping|int owner = 0, string|string*|int|int* owner_adj = 0, int who_art = ART_DER, int owner_art = ART_AAA) { return query_deklin_owner(who, who_art, FALL_GEN, who_adj, owner, owner_art, owner_adj); }
deklin_message  Ihres(object|mapping|int who = OBJ_TO, string|string*|int|int* who_adj = 0, object|mapping|int owner = 0, string|string*|int|int* owner_adj = 0, int who_art = ART_DER, int owner_art = ART_AAA) { return query_deklin_owner(who, who_art | ART_CAPITALIZE, FALL_GEN, who_adj, owner, owner_art, owner_adj); }
deklin_message  ihrem(object|mapping|int who = OBJ_TO, string|string*|int|int* who_adj = 0, object|mapping|int owner = 0, string|string*|int|int* owner_adj = 0, int who_art = ART_DER, int owner_art = ART_AAA) { return query_deklin_owner(who, who_art, FALL_DAT, who_adj, owner, owner_art, owner_adj); }
deklin_message  Ihrem(object|mapping|int who = OBJ_TO, string|string*|int|int* who_adj = 0, object|mapping|int owner = 0, string|string*|int|int* owner_adj = 0, int who_art = ART_DER, int owner_art = ART_AAA) { return query_deklin_owner(who, who_art | ART_CAPITALIZE, FALL_DAT, who_adj, owner, owner_art, owner_adj); }
deklin_message  ihren(object|mapping|int who = OBJ_TO, string|string*|int|int* who_adj = 0, object|mapping|int owner = 0, string|string*|int|int* owner_adj = 0, int who_art = ART_DER, int owner_art = ART_AAA) { return query_deklin_owner(who, who_art, FALL_AKK, who_adj, owner, owner_art, owner_adj); }
deklin_message  Ihren(object|mapping|int who = OBJ_TO, string|string*|int|int* who_adj = 0, object|mapping|int owner = 0, string|string*|int|int* owner_adj = 0, int who_art = ART_DER, int owner_art = ART_AAA) { return query_deklin_owner(who, who_art | ART_CAPITALIZE, FALL_AKK, who_adj, owner, owner_art, owner_adj); }

private deklin_message pick_verb(string singular_2pers, string singular_3pers, string plural_2pers, string plural_3pers, object|mapping|int who = OBJ_TO, int art = ART_DER)
{
    mapping result = ([]);

    who = parse_deklin_object(who);

    m_add(result, 0, plural(singular_3pers, plural_3pers, who, art));
    if (objectp(who))
        m_add(result, who, plural(singular_2pers, plural_2pers, who));

    return create_deklin_message(result);
}

deklin_message ist(object|mapping|int who = OBJ_TO, int spaces = 0)
{
    switch(spaces)
    {
        case 0:
            return pick_verb("bist", "ist", "seid", "sind", who);
        case 1:
            return pick_verb(" bist", " ist", " seid", " sind", who);
        case 2:
            return pick_verb("bist ", "ist ", "seid ", "sind ", who);
        default:
            return pick_verb(" bist ", " ist ", " seid ", " sind ", who);
    }
}

deklin_message hat(object|mapping|int who = OBJ_TO, int spaces = 0)
{
    switch(spaces)
    {
        case 0:
            return pick_verb("hast", "hat", "habt", "haben", who);
        case 1:
            return pick_verb(" hast", " hat", " habt", " haben", who);
        case 2:
            return pick_verb("hast ", "hat ", "habt ", "haben ", who);
        default:
            return pick_verb(" hast ", " hat ", " habt ", " haben ", who);
    }
}

/*
FUNKTION: verb
DEKLARATION: deklin_message verb(string singular_3pers, object|mapping|int who = OBJ_TO, int art = ART_DER)
BESCHREIBUNG:
Liefert das für das Objekt who konjugierte Form des angegebenen Verbes.
Das Verb muß in der 3. Person Singular angegeben werden.

Beispiel:
    Der(ob) + " " + verb("geht", ob) + " nach Hause."

VERWEISE: verb
GRUPPEN: grammatik
*/
deklin_message verb(string singular_3pers, object|mapping|int who = OBJ_TO, int art = ART_DER)
{
    string *formen = DEKLIN->konjugiere(singular_3pers);
    return pick_verb(formen[1], formen[2], formen[4], formen[5], who, art);
}

deklin_message text_for(object who, string text_who, string text_all = "")
{
    return create_deklin_message(([who: text_who, 0: text_all]));
}

#endif
