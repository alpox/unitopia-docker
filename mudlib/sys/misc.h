// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /sys/misc.h
// Description: Defines, die gerne und haeufig verwendet werden
// Author:      Diverse

#if !defined(MISC_H) && !defined(__UDL_PARSER__)
#define MISC_H

//Erstmal alles undef'n, besser als jedesmal nachzufragen,
//ob es nicht zufaellig definiert wurde...
#undef TP
#undef TP_N
#undef TP_RN
#undef TP_CN
#undef TP_RCN
#undef TO
#undef PO
#undef PP
#undef PL
#undef TI
#undef QSO
#undef ENV 
#undef EENV
#undef ENVR
#undef ENVP
#undef ENVL
#undef ENVRP
#undef ENVRL
#undef ENV_TO
#undef ENV_TP
#undef RN
#undef RCN
#undef NAME
#undef RNAME
#undef CNAME
#undef RCNAME
#undef SHORT
#undef LN
#undef ON
#undef FN
#undef FAIL
#undef FAILW
#undef CAP
#undef LOW
#undef UPP
#undef MIN
#undef MAX
#undef REGMATCH
#undef CHOOSE
#undef NOOP
#undef statuep
#undef livingp
#undef inheritp
#undef in_renewal


#define TP        this_player()
#define TP_N      (({string})this_player()->query_name())
#define TP_RN     (playerp(this_player()) && ({string})this_player()->query_real_name())
#define TP_CN     (({string})this_player()->query_cap_name())
#define TP_RCN    (playerp(this_player()) && ({string})this_player()->query_real_cap_name())
#define TO        this_object()
#define PO        previous_object()

// previous_player: Letzter Player im caller_stack
#define PP        (filter(caller_stack(1),#'playerp)+({this_player()}))[0]
// previous_living: Letztes Lebewesen im caller_stack
#define PL        (filter(caller_stack(1),#'living)+({this_player()}))[0]

#define TI        this_interactive()
#define QSO       query_shadow_owner()

#define ENV(x)    ((x) && environment(x))
#define EENV(x)   (environment(x) && environment(environment(x)))
// naechster umgebender Raum
#define ENVR(x)   (filter_objects(all_environment(x)||({}),"query_room")+({0}))[0]
// naechster umgebender Spieler
#define ENVP(x)   (filter(all_environment(x)||({}),#'playerp)+({0}))[0]
// naechstes umgebendes Lebewesen
#define ENVL(x)   (filter(all_environment(x)||({}),#'living)+({0}))[0]
// naechster umgebender Raum oder Spieler
#define ENVRP(x)  (filter(all_environment(x)||({}),(:({int})$1->query_room() || playerp($1):))+({0}))[0]
// naechster umgebender Raum oder Lebewesen
#define ENVRL(x)  (filter(all_environment(x)||({}),(:({int})$1->query_room() || living($1):))+({0}))[0]

#define ENV_TO    environment()
#define ENV_TP    ENV(TP)

#define RN(ob)    (({string})(ob)->query_real_name() || ({string})(ob)->query_name())
#define RCN(ob)    (({string})(ob)->query_real_cap_name() || ({string})(ob)->query_cap_name())
#define NAME(x)   (({string})(x)->query_name())
#define RNAME(x)  (playerp(x) && ({string})(x)->query_real_name())
#define CNAME(x)  (({string})(x)->query_cap_name())
#define RCNAME(x) (playerp(x) && ({string})(x)->query_real_cap_name())
#define SHORT(x)  (({string})(x)->query_short(TP))
#define FN(x)     object_name(x)
#define ON(x)     object_name(x)
#define LN(x)     load_name(x)

// '||0' deswegen, damit man dies auch bei Funktionen nutzen kann,
// die keinen int-Wert sondern z.B. object als Rueckgabetyp haben.
#define FAIL(x)   return (notify_fail(x)||0)
#define FAILW(x)  return (notify_fail(wrap(x))||0)
#define FAILP(x,p)   return (notify_fail(x,p)||0)
#define FAILPW(x,p)  return (notify_fail(wrap(x),p)||0)
#define FAILWP(x,p)  return (notify_fail(wrap(x),p)||0)
#define CAP(x)    capitalize(x)
#define LOW(x)    lower_case(x)
#define UPP(x)    upper_case(x)
#define NOOP      do{}while(0)

#define CHOOSE(x)	(funcall((:$1[random(sizeof($1))]:),(x)))

// regexp-Sting-matching
#define REGMATCH(str, pattern)  (sizeof( regexp( ({ str }), pattern)) > 0)
// Ist player eine Statue
#define statuep(player)  (playerp(player) && !interactive(player))
#define monsterp(ob)     (living(ob) && !playerp(ob))
// Ist ob ein Lebewesen oder lebendiges V-Item
#define livingp(ob)      ( (objectp(ob) && living(ob)) \
                         ||(mappingp(ob) && ob["living"]))
// ist x ein Inherit
#define inheritp(x) (objectp(x) && strstr(object_name(x), "/i/") != -1)

// Wird das Objekt gerade zern -t erneuert? Liefert das alte Objekt.
#define in_renewal(ob) find_object(regreplace(object_name(ob), \
                                              "/([^/]*)$","/$\\1$", 0))

#endif /* MISC_H */
