// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/room/rathaus/div/leo_hlp.inc
// Description:	Teil von Leo, der das Engelwerden regelt.
// Author:	Garthan
// Modified by: Nethar (16.09.01) Wahlraetsel eingefuegt

//
// teste_engel wird aufgerufen, um zu pruefen ob eine Spieler Engel werden 
// darf.
//
private int teste_engel(object ob)
{
   int a, sum_stat, sum_real_stat, count;
   string *quests, *choice_quests;

#ifdef PLAYER_DAY
   if(ob->player_day())
      REJECT("sage Nicht in diesem Monat!");
#endif
   if(LVL_HLP >= BANISHD->query_banished(ob->query_real_name()))
      REJECT("sage DU nicht!");
   if(guestp(ob))
      REJECT("sage Ein Gast kann das nicht!");
#ifdef UNItopia
   if(ob->query_gilde()=="Orkgilde") {
     do_command("wundere");
     do_command("schüttle Kopf");
     do_command("sage Du glaubst doch nicht im Ernst, dass Orks in den Himmel "
        "kommen und ihnen Flügel wachsen? Nene.");
     exec_command("tätschle",this_player());
     return 0;
   }
#endif
   if(hlpp(ob))
      REJECT("sage Du bist doch bereits ein Engel!");
   if(wizp(ob))
      REJECT("sage Du bist doch bereits ein Gott!");
   if (ob->query_moerder())
      REJECT("sage Mörder wie Du müssen sich "
	     "erst von ihrer Schuld befreien!");
   if(ob->query_sum_skill() < TOTAL_EXPERIENCE)
      REJECT("sage Du hast noch nicht genügend Erfahrung "
	     "für diesen Schritt!");
   if(sizeof(quests = QUEST_ROOM->query_quests(Q_NECESSARY)) <= 0)
      REJECT("sage Sorry, ohne installierte Rätsel kann niemand "
		 "in die Engelsschar aufgenommen werden!");
   
   if(sizeof(choice_quests = QUEST_ROOM->query_quests(Q_WAHL)) <= 0)
      REJECT("sage Sorry, ohne installierte Wahlrätsel kann niemand "
         "in die Engelsschar aufgenommen werden!");
   
   // Pflichties
   for(count = 0, a = sizeof(quests); a--;)
      if(!QUEST_ROOM->quest_min_solved(quests[a], ob))
         count++;
   if(count)
      REJECT("sage Du musst erst alle Abenteuer der rechten Tafel "
             "lösen!");  
   
   // Wahlraetsel
   for(count = 0, a = sizeof(choice_quests); a--;)
      if(QUEST_ROOM->quest_min_solved(choice_quests[a], ob))
         count++;
   if(count < NEEDED_CHOICE_QUESTS)
      REJECT("sage Du musst mindestens "+NEEDED_CHOICE_QUESTS+
             " Abenteuer der linken Tafel bestehen! "+
             (NEEDED_CHOICE_QUESTS-count)+" Abenteuer "+
             ((NEEDED_CHOICE_QUESTS-count)==1 ? "fehlt " : "fehlen ")+
             "Dir also noch.");

   for(a=0; a<STAT_NUMBER; a++)
      sum_stat += ob->query_stat(a);
   for(a=0; a<STAT_NUMBER; a++)
      sum_real_stat += ob->query_real_stat(a);
   if(sum_real_stat/STAT_NUMBER < 75)
   {
      do_command("sage Deine Fähigkeiten sind für diesen Schritt "
	         "noch nicht hoch genug!");
      if(sum_stat/STAT_NUMBER >= 75)
         do_command("sage Auch wenn das so scheinen mag!");
      else
      if(sum_stat != sum_real_stat)
         do_command("sage Da helfen auch Magie und andere Tricks nichts!");
      return 0;
   }
   return 1;
}

private void tell_him()
{
   do_command("sage Man spricht in Tadmor von Engeln, die des öfteren "
	      "um die Kirchtürme der Kathedrale kreisen und elegant über "
	      "den Kartukultininurtaplatz gleiten. Vielleicht solltest Du "
	      "dort Dein Glück suchen?");
}

//
// erschaffe_engel wird aufgerufen, wenn alle Formularitaeten
// abgeschlossen sind, und der Engel gesetzt wird.
//
private void erschaffe_engel(string str)
{
    if(!str || lower_case(str) != "ja")
    {
        this_player()->send_message(MT_NOISE,MA_COMM,
	    wrap_say(Der(this_player())+" sagt:",str));
        do_command("sage Wie Du meinst, das sollte auch gut überlegt sein.");
        return;
    }
    this_player()->send_message(MT_NOISE,MA_COMM,
	    Der(this_player())+" sagt: Ja.\n");

    // Ein ein neuer Engel ... let's do it.

    // Level setzen
    this_player()->set_level(LVL_HLP);

    // GOETTER Logfile updaten
    sys_log("ENGEL",timestr(time())+": "+
	    capitalize(this_player()->query_real_name())+".\n");

    // Meldung an alle ausgeben.
    touch(EVENT_MASTER)->event("Gott", this_player(), 0, 1);

    // Titel aendern.
    switch(this_player()->query_gender()) {
	case "weiblich":
	    this_player()->set_title(", die Neue in der Engelsschar");
	    break;
	case "maennlich":
	    this_player()->set_title(", der Neue in der Engelsschar");
	    break;
	default:
	    this_player()->set_title(", das Neue in der Engelsschar");
	}

    // Player abspeichern.
    this_player()->do_save();

    this_player()->set_mmsg_out(
       "$Der() erhebt sich in die Luft und fliegt davon");
    this_player()->set_mmsg_in(
       "$Ein() fliegt heran und landet elegant neben Dir");

    // Begruessung...

    this_object()->send_message(MT_LOOK,MA_MAGIC,wrap(
       "Leo hebt die Arme und spricht einen sehr komplizierten Zauber auf "+
       den(this_player())+"."),
       wrap(
       "Leo hebt die Arme und spricht einen sehr komplizierten Zauber auf "+
       "Dich."), this_player());
    do_command("sage Engel "+this_player()->query_cap_name()+", "
       "Deine Aufgabe ist es nun, Lyrion den Schutzengel auf seiner Wolke "
       "im Himmel aufzusuchen!");
    call_out(#'tell_him, 2);
}

//
// engel wird aufgerufen, wenn der Spieler darum bittet, Engel zu werden.
//
void engel(string str)
{
   if(strstr(str," kein ")>=0 || strstr(str," nicht ")>=0)
   {
     if(wizp(this_player()))
       exec_command("sage Du brauchst auch nicht Engel werden, denn "
         "wie du vielleicht schon mitbekommen hast, bist du ein Gott.");
     else if(hlpp(this_player()))
       exec_command("sage Hmm. Das hätten wir uns wohl eher überlegen sollen.");
     else
       exec_command("sage Schön. Dann bleib einfach wie du bist.");
     return;
   }
   if(!teste_engel(this_player()))
      return;

   do_command("sage Nun gut. Bist Du bereit für diesen bedeutungsvollen "
	       "Schritt? So antworte mit ja oder nein, Sterblich"+
	      (this_player()->query_gender()=="weiblich"?"e":"er")+"!");
   input_to(#'erschaffe_engel, INPUT_PROMPT, "[ja/nein] ? ");
}
