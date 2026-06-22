// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/secure/udp/tell.c
// Description:	Handles a UDP-Tell-Request
// Author:	

#include <udp.h>
#include <message.h>

string last_sender;
string query_current_sender() { return last_sender; }

void udp_tell(mapping data)
{
    object ob;
    string away;

    if (!data[RECIPIENT])
    {
	INETD->send_udp(data[NAME],
			([
			REQUEST: REPLY,
			RECIPIENT: data[SENDER],
			ID: data[ID],
			DATA: "Root@"+LOCAL_NAME+": Was willst Du ?\n"
			]) );
    }
    else if(!stringp(data[RECIPIENT]) ||
	    !stringp(data[SENDER]) ||
	    !stringp(data[NAME]) ||
	    !stringp(data[DATA]))
		return;
    else if (ob = find_player(lower_case(data[RECIPIENT])))
    {
#if __VERSION__ <= "3.5.2"
	data[DATA] = convert_umlaute(to_string(data[DATA]));
#endif
	if (!interactive(ob))
	{
	    INETD->send_udp(data[NAME],
			    ([
			    REQUEST: REPLY,
			    RECIPIENT: data[SENDER],
			    ID: data[ID],
			    DATA: wrap(capitalize(data[RECIPIENT])+
					"@"+
					LOCAL_NAME+
					" ist versteinert und würde nicht "+
					"antworten." +
					((away=ob->query_away())
					    ?" Er ist weg: " + away:""))
			    ]) );
	}
	else
	{
	    string prefix = capitalize(data[SENDER]) + "@" + data[NAME];
	    string re_prefix;
	    string text = data[DATA];
	    string adverb = "";
	    
	    if (away=ob->query_away())
		INETD->send_udp(data[NAME],
			    ([
			    REQUEST: REPLY,
			    RECIPIENT: data[SENDER],
			    ID: data[ID],
			    DATA: capitalize(data[RECIPIENT])+
					"@"+
					LOCAL_NAME+
					" ist weg: "+away+"\n"
			    ]) );
			    
	    switch(data[METHOD])
	    {
		case "gemote":
		    prefix = get_genitiv(prefix);
		    // FALLTHROUGH
		case "emote":
		    {
			int ende = strrstr(text, "*");
			if(ende>0)
			    text = text[1..ende-1];
		    }
		    re_prefix = prefix;
		    break;
		
		case "adverb":
		    if(text[0]=='*')
		    {
			int aende = strstr(text,"*",1);
			if(aende>0)
			{
			    adverb = text[1..aende-1] + " ";
			    text = text[aende+1..<1];
			    if(text[0]==' ')
				text = text[1..<1];
			}
		    }
		    //FALLTHROUGH
		default:
		    prefix += " redet "+adverb+"zu dir:";
		    re_prefix = "Du redest "+adverb+"zu "+
			 capitalize(data[RECIPIENT])+"@"+LOCAL_NAME+":";
		    break;
	    }
	    
	    last_sender = data[SENDER] + "@" + data[NAME];
	
            ob->receive_message(MT_SENSE|MT_FAR,MA_COMM,this_object(),
		wrap_say(prefix, text));
	    ob->add_to_rede_puffer(wrap_say(prefix, text));
	
	    last_sender = 0;
	    
	    INETD->send_udp(data[NAME],
		([
		    REQUEST: REPLY,
		    RECIPIENT: data[SENDER],
		    ID: data[ID],
		    DATA: wrap_say(re_prefix, text)
		]) );
	}
    }
    else
	INETD->send_udp(data[NAME],
			([
			REQUEST: REPLY,
			RECIPIENT: data[SENDER],
			ID: data[ID],
			DATA: "Root@"+
				      LOCAL_NAME+
				      ": "+
				      capitalize(data[RECIPIENT])+
				      " ist gerade nicht da.\n"
			]) );
}
