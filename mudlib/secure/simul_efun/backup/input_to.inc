// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/secure/simul_efun/input_to.inc
// Description: Um Umlaute bei input_to zu konvertieren
// Author:	Gnomi (17.02.2000)
// Modified by:	Freaky (24.02.2000) verbessert und einen Bug raus, der
//			entstand, wenn man die simul-efun zerstoerte

#include "/sys/input_to.h"

varargs void input_to(mixed fun, int flag, varargs mixed *args)
{
    closure cl;

    set_this_object(previous_object());
    if (stringp(fun))
    {
	if (!(cl = symbol_function(fun,this_object())))
	    raise_error("Function '" + fun +
		    "' for input_to() does not exist\n");
    }
    else if (closurep(fun))
	cl = fun;
    else
	raise_error("Bad argument 1 to input_to()\n");

    if(flag & INPUT_PROMPT)
    {
	if(!sizeof(args))
	    raise_error("Missing prompt argument to input_to().\n");
	
	if((flag & (INPUT_CHARMODE|INPUT_NOECHO)) ||
	    !({int})this_player()->start_input_to(
		lambda(({'str}), ({#'apply, cl, 'str, quote(args[1..<1]) })),
		args[0], flag))
	{
	    // Workaround fuer fehlende Driver-Prompt-Anzeige,
	    // wenn noch ein Prompt aktiv ist:
	    if(efun::this_interactive()!=this_player() &&
		(!(flag&INPUT_APPEND) || 
#if __EFUN_DEFINED__(query_input_pending)
		 !efun::query_input_pending(this_player())
#else
		 !(interactive(this_player()) && efun::interactive_info(this_player(), II_INPUT_PENDING))
#endif
		))
	    {
		mixed res;
		mixed curprompt = funcall(args[0]);
		if(!call_resolved(&res, this_player(), "print_prompt", curprompt))
		    this_player()->receive_message_low(curprompt);
	    }
	    
#if __VERSION__ > "3.5.2"
	    efun::input_to(cl, flag, args...);
#else
	    efun::input_to(lambda(({'str}),
		({#'apply,cl,
		    flag & INPUT_CHARMODE ? 'str :
			({symbol_function("convert_umlaute"),'str}),
		    quote(args[1..<1]) })),
		flag, args[0]);
#endif
	}
    }
    else
    {
	if((flag & (INPUT_CHARMODE|INPUT_NOECHO)) ||
	    !({int})this_player()->start_input_to(
		lambda(({'str}), ({#'apply, cl, 'str, quote(args) })),
		"", flag))
#if __VERSION__ > "3.5.2"
	    efun::input_to(cl, flag, args...);
#else
	    efun::input_to(lambda(({'str}),
		({#'apply,cl,
		    flag & INPUT_CHARMODE ? 'str :
			({symbol_function("convert_umlaute"),'str}),
		    quote(args) })),
		flag);
#endif
    }
}

object query_input_pending(object ob)
{
    int pp = playerp(ob);

    set_this_object(previous_object());

    return (pp && ({object})ob->query_pending_input_to()) ||
#if __EFUN_DEFINED__(query_input_pending)
	efun::query_input_pending(ob);
#else
	(efun::interactive(ob) && efun::interactive_info(ob, II_INPUT_PENDING));
#endif
}
