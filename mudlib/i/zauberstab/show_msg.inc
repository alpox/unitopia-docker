// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/i/zauberstab/show_msg.inc
// Description: zet Befehl, Teil der Meldungsansicht
//		wird vom Engeltool mitbenutzt.
// Author:	Garthan (29.04.94)

#pragma strong_types

  W("Design:\n");
  W("-------\n");
  W(MWRAP("PTitel:",OWN->query_personal_title()||"- keiner -"));
  W(MWRAP("Titel:",OWN->query_title()||"- keiner -"));
//W(MWRAP("Ansehen:",OWN->query_align_title()||"- keines -"));
  W(MWRAP("  ==>",OWN->query_short(OWN)));
  W("\nBewegungs-Meldungen:\n");
  W(  "--------------------\n");
  if(tmp = OWN->query_msg_in())
  {
     W(MWRAP("Ankommen:",tmp));
     W(MWRAP("  ==>",OWN->query_exp_msg_in("von Norden")));
  }
  else
  {
     W(MWRAP("Ankommen:","0"));
     W(MWRAP("  ==>",Ein(OWN)+" nähert sich."));
  }
  if(tmp = OWN->query_msg_out())
  {
     W(MWRAP("Verlassen:",tmp));
     W(MWRAP("  ==>",OWN->query_exp_msg_out("nach Süden")));
  }
  else
  {
     W(MWRAP("Verlassen:","0"));
     W(MWRAP("  ==>",Der(OWN)+" entfernt sich."));
  }
  if(tmp = OWN->query_mmsg_in())
  {
     W(MWRAP("ZAnkommen:",tmp));
     W(MWRAP("  ==>",OWN->query_exp_mmsg_in()));
  }
  else
  {
     W(MWRAP("ZAnkommen:","0"));
     W(MWRAP("  ==>",Ein(OWN)+" nähert sich."));
  }
  if(tmp = OWN->query_mmsg_out())
  {
     W(MWRAP("ZVerlassen: ",tmp));
     W(MWRAP("  ==>   ",OWN->query_exp_mmsg_out()));
  }
  else
  {
     W(MWRAP("ZVerlassen:","0"));
     W(MWRAP("  ==>",Der(OWN)+" entfernt sich."));
  }
  if(tmp = OWN->query_msg_invis())
  {
     W(MWRAP("Unsichtbar:",tmp));
     W(MWRAP("  ==>   ",OWN->query_exp_msg_invis()));
  }
  else
  {
     W(MWRAP("Unsichtbar:","nicht gesetzt"));
     W(MWRAP("  ==>   ","ZVerlassens - Meldung"));
  }
  if(tmp = OWN->query_msg_vis())
  {
     W(MWRAP("Sichtbar:",tmp));
     W(MWRAP("  ==>   ",OWN->query_exp_msg_vis()));
  }
  else
  {
     W(MWRAP("Sichtbar:","nicht gesetzt"));
     W(MWRAP("  ==>   ","ZAnkommens - Meldung"));
  }

