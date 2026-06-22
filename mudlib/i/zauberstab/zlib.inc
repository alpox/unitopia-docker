// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/i/zauberstab/zlib.inc
// Description: Libary-Funktionen des Zauberstabes
// Author:	Freaky (23.12.93)

#pragma strong_types

string check_invis(int i, string str) {
    if (i==V_VIS)
	return str;
    if (i==V_INVIS)
	return "("+str+")";
    if (i==V_HIDDEN)
	return "["+str+"]";
    if (i==V_SHIMMER)
        return "<"+str+">";
    if (i==V_NOLIST)
	return "{"+str+"}";

    if (i & V_ATOM_NOLIST)
	str="{"+str+"}";
    if (i & V_ATOM_HIDDEN)
	str="["+str+"]";
    if (i & V_ATOM_INVIS)
	str="("+str+")";
    if (i & V_ATOM_NOSHIMMER)
	str="<"+str+">";
    /*
    if (i & V_ATOM_INVIS && !(i & V_ATOM_NOSHIMMER))
	str="("+str+")";
    */
    return "|"+str+"|";
}

int secure(string verb, string str)
{
    if(this_player()==owner &&
       geteuid(this_object())==geteuid(owner) && 
       check_security())
	return 0;
#if 0
    // volk in tue - Kuerzeln erlauben
    if (verb == "volk" && !TI && environment()
        && previous_object()==environment() && this_player()==environment())
        return 0;
    if (TI==OWN && previous_object() && geteuid(previous_object())==geteuid())
	return 0;
#endif
    tell_object(OWN,"ACHTUNG!!!!!!!\n"+Name(TI)+" wollte dich zu:\n"+
	verb+" "+str+"\nzwingen.\n");
    tell_object(OWN,"TI:"+(TI?OFN(TI):"0")+" "+Name(TI)+"\n");
    tell_object(OWN,"TP:"+(TP?OFN(TP):"0")+" "+Name(TP)+"\n");
    tell_object(OWN,"PO:"+OFN(previous_object())+" "+
	Name(previous_object())+"\n");
    return 1;
}

int secure_call()
{
    if (this_interactive() && previous_object() &&
	    this_interactive() == this_player() &&
	    geteuid(previous_object()) == geteuid(this_interactive()) &&
	    geteuid(previous_object()) == geteuid())
	return 1;
    return 0;
}

mixed query_variable(string str)
{
#if 0
    if (secure_call())
#else
    if (check_security())
#endif
	return vars[str];
}

private varargs mixed *get_com1(string str, int flag) {
    string par, obst;

    if (!str)
	return ({"",SO(MARK_ICON)});
    if (sscanf(str,"-%s %s",par,obst)==2)
	return ({par,flag?VSO(obst):SO(obst)});
    if (sscanf(str,"-%s",par))
	return ({par,SO(MARK_ICON)});
    return ({"",flag?VSO(str):SO(str)});
}

private string convert_string(string str) {
    str=regreplace(str,"\\\\\\\\","\1",1);  /* wegen \ */
    str=regreplace(str,"\\\\\"","\"",1);  /* wegen Hochkommas */
    str=regreplace(str,"\\\\a","\a",1);   /* wegen ^G */
    str=regreplace(str,"\\\\b","\b",1);   /* wegen BS */
    str=regreplace(str,"\\\\e","\e",1);   /* wegen ESC */
    str=regreplace(str,"\\\\f","\f",1);   /* wegen FF */
    str=regreplace(str,"\\\\n","\n",1);   /* wegen Zeilenumbruch */
    str=regreplace(str,"\\\\r","\r",1);   /* wegen CR */
    str=regreplace(str,"\\\\t","\t",1);   /* wegen Tabs */
    return regreplace(str,"\1","\\",1); /* wegen \ */
}

mixed *str2mixed(string str) {
    string *all, sub, test;
    int i, j, jj;
    mixed *end;

    if (!str)
	FAIL("Kein Argument übergeben!\n");

    end=({});
    if (str=="")
	return end;
    all=explode(str,",");
    if (!sizeof(all))
	FAIL("Fehler im Argument '"+str+"'\n");

    for (i=0; i<sizeof(all); i++) { // Achtung! Zaehler i wird noch veraendert!
	sub=all[i];
	for(j=0;strlen(sub)>j && sub[j]==' ';j++);
	sub=sub[j..<1];
	if (sub=="") {					/* leer */
	    string fal="";
	    if (i>0)
		fal=all[i-1];
	    fal+=","+all[i];
	    if (sizeof(all)>i+1)
		fal+=","+all[i+1];
	    FAIL("Ein Komma zuviel: '"+fal+"'\n");
	    }
	else if (sscanf(sub,"%d.%d%s",j,jj,test)>=2)		/* Float */
	    if (jj>=0 && !strlen(test && trim(test)))
		end+=({ to_float(sub) });
	    else
		FAIL("Fehler im Argument: '"+sub+"'\n");
	else if (sscanf(sub,"%d%s",j,test))			/* Integer */
	    if (!strlen(test && trim(test)))
		end+=({j});
	    else
		FAIL("Fehler im Argument: '"+sub+"'\n");
	else if (sub[0]=='\"') {			/* String */
		string tmp, ttmp;
		tmp=sub[1..];
		ttmp = trim(tmp);
		
		while (sizeof(all)>(i+1) &&
			(!sizeof(ttmp) || ttmp[<1]!='\"' || (sizeof(ttmp)>1 && ttmp[<2]=='\\')) ) {
		    i++;
		    tmp+=","+all[i];
		    ttmp = trim(tmp);
		    }
		if ((i+1)>=sizeof(all) &&
			(!sizeof(ttmp) || ttmp[<1]!='\"'))
		    FAIL("Fehler im Argument: '"+tmp+ "' (es fehlt ein \")\n");
		for(j=1;tmp[<j]!='\"';j++);
		end+=({convert_string(tmp[0..<(j+1)])});
		}
	else if (sub[0..0]==MARK_ICON && ((j=(sub+"0")[1]-'0')>=0) && j<10)
							/* Markierung */
		end+=({OWN->query_mark(j)});
	else if (sub[0..3]=="OBJ(") {			/* Objekt */
		object ob;
		sub = trim(sub);
                if (sub[<1]!=')')
		    FAIL("Fehler im Argument: '"+sub+"' Es fehlt ')'\n");
		ob=SO(sub[4..<2]);
		if (!ob)
		    return 0;
		end+=({ob});
		}
	else if (sub[0..3]=="VAR(") {			/* Variable */
		sub = trim(sub);
		if (sub[<1]!=')')
		    FAIL("Fehler im Argument: '"+sub+"' Es fehlt ')'\n");
		end+=({vars[sub[4..<2]]});
		}
	else if (sub[0..1]=="([") {			/* Mapping */
		string *va, ts;
		mixed index, value;
		mapping map;

		ts=sub[2..];
		map=([]);
		j=0;
		do {
		    if (trim(ts)[<2..]=="])") {
			j=1;
			ts=trim(ts)[0..<3];
			if(ts=="")
			    break;
			}
		    else if (ts=="")
			FAIL("Da ist ein Komma zuviel bei '"+
						all[i-1]+","+all[i]+"'\n");
		    /* Suche Index und Value */
		    va=explode(ts,":");
		    if (sizeof(va)!=2)
			FAIL("Das Argument '"+ts+
					"' ist mir zu kompliziert :(\n");
		    if (!(index=str2mixed(va[0])))
			return 0;
		    if (!(value=str2mixed(va[1])))
			return 0;
		    map[index[0]]=value[0];
		    if (!j) {
			i++;
		    	ts=all[i];
			if (trim(ts)=="])")
			    j=1;
			}
		    } while(!j);
		end+=({map});
		}
	else if (sub[0..1]=="({") {			/* Array */
		string tst;
                int k,l,m;
		mixed *tend;

		tst="";
		do {
		    tst+=","+all[i];
		    l=0;
		    m=strlen(all[i])-1;
		    while (l<strlen(all[i])) {
			if (all[i][l..l+1]=="({") { k++; l+=2; }
			else if(all[i][l]==' ')   l++;
			else break;
		    }
		    while (m>0) {
			if (all[i][m-1..m]=="})") { k--; m-=2; }
			else if(all[i][m]==' ') m--;
			else break;
		    }
		    i++;
		    if (i>=sizeof(all) && k>0)
		       FAIL("Fehler im Argument: '"+all[i-1]+
				   "' (es fehlt ein '})' )\n");
		    if (k<0)
		       FAIL("Fehler im Argument: '"+all[i-1]+
				   "' (es ist ein '})' zuviel)\n");
		    }
		while (k>0);

		tst = trim(trim(tst)[3..<3]);
		if (sizeof(tst) && tst[<1]==',')
		    tend=str2mixed(tst[0..<2]);
		else
		    tend=str2mixed(tst);
		if (!tend)
		    return 0;  /* Fehler */
		end+=({tend});
		i--; // damit die FOR-Schleife an der richtigen Stelle weitermacht
	}
	else						/* alles andere */
	    FAIL("Fehler im Argument: '"+sub+"'\n");
	}
    return end;
}

private string add_plugin(string file, mixed args);

static int set_param(string was, mixed par)
{
    string msg;

    switch (lower_case(was))
    {
	case "prompt":	OWN->set_prompt_string(par||"> ");
			return 1;
	case "print":   params[was]=par||"array";
			OWN->set_zauberstab_info(params);
			return 1;
	case "zeilen":	OWN->set_more_chunk(to_int(par));
			return 1;
	case "max-out":	params[was]=to_int(par);
			set_max_output(to_int(par));
			OWN->set_zauberstab_info(params);
			return 1;
	case "ankommen": OWN->set_msg_in(par);
			 return 1;
	case "verlassen": OWN->set_msg_out(par);
			  return 1;
	case "zankommen": OWN->set_mmsg_in(par);
			  return 1;
	case "zverlassen": OWN->set_mmsg_out(par);
			   return 1;
	case "unsichtbar": OWN->set_msg_invis(par);
			   return 1;
	case "sichtbar": OWN->set_msg_vis(par);
			 return 1;
	case "clone_msg" : OWN->set_clone_msg(par);
			   c_clone_msg = mixed_to_closure(
			      add_dot_to_msg(par),({'ob}),1);
			   return 1;
	case "destruct_msg" : OWN->set_destruct_msg(par);
			      c_destruct_msg = mixed_to_closure(
			         add_dot_to_msg(par),({'ob}),1);
			   return 1;
	case "home_msg"  : OWN->set_home_msg(par);
			   return 1;
	case "titel":	OWN->set_title(par||"");
			return 1;
	case "ptitel":
	case "pt":
	case "personal_title":
			OWN->set_personal_title(par);
			return 1;
	case "ansehen": OWN->set_align_title(par);
			return 1;
	case "file":	OWN->set_eye_option("file",par=="an");
			return 1;
	case "wer-liste": OWN->set_no_wer(par && lower_case(par)=="nein");
			  return 1;
	case "last-init":
	case "volk":
	case "own-rtes":
	case "zfe-groups":
    case "zfe-g":
	case "zfe-s":
	case "zfe-privgroups":
    case "zfe-short-history":
    case "zsqlite":
                      params[was]=par;
	              OWN->set_zauberstab_info(params);
		      return 1;
	case "fingertext": if (!par) {
			  OWN->set_finger_info(0);
			  return 1;
			}
			if(strlen(par) > 500)
			  FAIL("Mehr als 500 Zeichen sind nicht erlaubt.\n");
			OWN->set_finger_info(implode(explode(par,"\\n"),"\n"));
			return 1;
	case "plugins":
	{
	    int num = sizeof(par);
	    foreach(string pl : par)
		catch((msg = add_plugin(pl,par[pl])) && write(msg); reserve (get_eval_cost()-25000)/num*(--num)+10000);
	    return 1;
	}
	default: FAIL("Parameter "+was+" gibt es nicht.\n");
    }
}

private varargs void own_ed(string file, string func)
{
    if(query_editing(OWN))
    {
	write("Du editierst bereits eine Datei.\n");
	return;
    }
    
    ed_func = func;
    OWN->notify_ed_enter(1);
    ed(file,"ed_ends");
}

static void ed_ends()
{
    closure c;

    OWN->notify_ed_exit(1);

    if (ed_func && c = symbol_function(ed_func,TO))
	funcall(c);
}

/*
 * Eigene left() Funktion, da im Zauberstab maximal 80 Zeichen ausgegeben 
 * werden und dann auch nur mit Spaces aufgefuellt wird
 * (ist schneller als die left()-Funktion in /secure/simul_efun)
 */
string left(string str, int len) {
    return (str+"                                                                                  ")[0..len-1];
}
