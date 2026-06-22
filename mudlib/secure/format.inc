// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/secure/format.inc
// Description:	Hilfsdatei zu /secure/event.c
// Author:	Garthan

string real_Der(object who)
{
   if(!playerp(who)) return Der(who);
   return who->query_real_cap_name()||capitalize(who->query_real_name());
}

mixed real_Who(object who)
{
   if(!playerp(who)) return who;
   return ([
    "name": 	who->query_real_name(),
    "cap_name":	who->query_real_cap_name(),
    "gender":	who->query_real_gender(),
   ]);
}

string ev_login(object who, string mess, mixed param)
{
   return INFO+real_Der(who)+" betritt "+(IS_INVIS(who)?"unsichtbar ":"")+MUD_NAME+".\n";
}

string ev_logout(object who, string mess, mixed param)
{
   return INFO+real_Der(who)+" hat "+MUD_NAME+(param?" für immer":IS_INVIS(who)?" unsichtbar":"")+
	  " verlassen.\n";
}

string ev_netztod(object who, string mess, mixed mode)
{
   if(mode)
      return INFO+real_Der(who)+(random(15000)?" bricht aus ":" erbricht aus ")+
	 seinem((["gender":"weiblich","name":"statue"]),0,real_Who(who))+".\n";
   else
      return INFO+real_Der(who)+" erstarrt zu einer Statue.\n";
}

string ev_tod(object who, string mess, mixed param)
{
    string church = who->query_kirche();
    if(church)
    {
	church = church->get_death_message(who);
	if(church)
	    return church;
    }
   return 
   "Die Totenglocke 'Verdombde' trägt eine traurige Botschaft übers Land:\n"+
   Der(who,"")+" ist tot!\n";
}

string ev_raetsel(object who, string mess, mixed param)
{
   if(mess)
      return wrap(mess);
   return INFO+Der(who)+" hat das Rätsel '"+param+"' gelöst.\n";
}

string ev_spiele(object who, string mess, mixed param)
{
   if(mess)
      return wrap(mess);
   return INFO+Der(who)+" hat erfolgreich '"+param+"' gespielt.\n";
}

string ev_gott(object who, string mess, mixed param)
{
   if(param)
      return Der(who)+" gehört nun der Engelsschar an.\n";
   return Der(who)+" wurde in den Kreis der Götter aufgenommen.\n";
}

string ev_gilden(object who, string gilde, mixed mode)
{
   string gender, name;
   gender = GILDEN_OB->query_one_gilden_info(gilde,GILDEN_GESCHLECHT)
      || "weiblich";
   name = GILDEN_OB->query_one_gilden_info(gilde,GILDEN_NAME) || gilde;
   if(mode)
      return INFO+real_Der(who)+" ist "+dem((["name":name,"gender":gender]))+" beigetreten.\n";
   else
      return INFO+real_Der(who)+" ist aus "+dem((["name":name,"gender":gender]))+" ausgetreten.\n";
}

private string level_str(mixed level)
{
    switch(level)
    {
        case 0: return "Monster";
        case LVL_PLAYER: return "Spieler";
        case LVL_HLP: return "Engel";
        case LVL_LEARNER: return "Lehrling";
        case LVL_GESELLE: return "Geselle";
        case LVL_VOGT: return "Vogt";
        default: return to_string(level);
    }
}

string ev_level(object who, string mess, mixed param)
{
   string name;
   if(!(name = who->query_real_name()) && !(name = der(who)))
      return 0;
   return INFO+capitalize(name)+
	  " Leveländerung: "+level_str(param[0])+" -> "+level_str(param[1])+".\n";
}

mixed ev_feedback(object who, string mess, mixed param)
{
   return stringp(param)?({mess,param}):mess;
}

string ev_ooc(object who, string mess, mixed param)
{
    return wrap_say("[OOC] "+real_Der(who)+":", mess);
}

string ev_notruf(object who, string mess, mixed param)
{
    if(!wizp(who))
	return wrap_say(real_Der(who)+" ruft das Pantheon um Hilfe:", mess);
    if(stringp(param) && strlen(param))
	return wrap_say(real_Der(who)+" antwortet zu "+param+":", mess);
    return wrap_say(real_Der(who)+" sagt zum Pantheon:", mess);
}

string ev_hilferuf(object who, string mess, mixed param)
{
    if (param)
        return wrap_say(real_Der(who)+" antwortet:", mess);
    return wrap_say(real_Der(who)+" ruft um Hilfe:", mess);
}

string ev_ueberfall(object taeter, string mess, object opfer)
{
    if(!playerp(taeter) || !playerp(opfer))
        return 0;
    return wrap_say("Überfall:", real_Der(taeter)+" greift "+real_Der(opfer)+" an.");
}
