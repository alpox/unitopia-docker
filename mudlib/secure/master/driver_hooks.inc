// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/secure/master/driver_hooks.inc
// Description:	Setzt die Driverhooks
// Author:	Freaky
// Modified by:	Freaky (12.01.2000) Include-Closure raus

#pragma strict_types

// setzt alle Driver-Hooks
private void setup_driver_hooks()
{

#   include "/secure/master/move_closure.inc"

    set_driver_hook(H_RESET,    "reset");
    set_driver_hook(H_CREATE_OB,    "create");
    set_driver_hook(H_CREATE_CLONE, "create");
    set_driver_hook(H_CREATE_SUPER, "create");
#ifdef __LPC_LWOBJECTS__
    set_driver_hook(H_CREATE_LWOBJECT, "create");
#if __VERSION__ > "3.6.5"
    set_driver_hook(H_CREATE_LWOBJECT_COPY, "recreate");
    set_driver_hook(H_CREATE_LWOBJECT_RESTORE, "recreate");
#endif
#endif
    set_driver_hook(H_CLEAN_UP, "clean_up");
    set_driver_hook(H_LOAD_UIDS, #'h_load_uids);
    set_driver_hook(H_CLONE_UIDS, #'h_clone_uids);
#ifdef __LPC_LWOBJECTS__
    set_driver_hook(H_LWOBJECT_UIDS, #'h_lwobject_uids);
#endif
#if __VERSION__ > "3.5.2"
    set_driver_hook(H_FILE_ENCODING, "UTF-8");
#endif

#if 0
    set_driver_hook(H_LOAD_UIDS,
	unbound_lambda( ({'object_name}), ({
	  #'?,
	  ({#'==,
	    ({#'=, 'creator_name, ({#'get_load_uid, 'object_name})}),
	    ({#'geteuid, ({#'previous_object})}),
	  }),
	   ({#'geteuid, ({#'previous_object})}),
	  ({#'==, 'creator_name, BACKBONE_UID}),
	   ({#'geteuid, ({#'previous_object})}),
	   ({#'({, 'creator_name, 1}),
	}) )
    );
    set_driver_hook(H_CLONE_UIDS,
	unbound_lambda( ({ /* object */ 'blueprint, 'new_name}), ({
	  #'?,
	  ({#'==,
	    ({#'=, 'creator_name, ({#'get_clone_uid, 'new_name})}),
	    ({#'geteuid, ({#'previous_object})}),
	  }),
	   ({#'geteuid, ({#'previous_object})}),
	  ({#'==, 'creator_name, BACKBONE_UID}),
	   ({#'geteuid, ({#'previous_object})}),
	   ({#'({, 'creator_name, 1}),
	}) )
    );
#endif

#if 1
#undef  PROPERTY_ROOT_MASTER
#define PROPERTY_ROOT_MASTER "/apps/properties"
    set_driver_hook(H_DEFAULT_METHOD,
        function int(mixed result, object ob, string fun, varargs mixed* args)
        {
            int found = 0;
            if(fun == "query_room")
                return 0;

            if(!strstr(fun, "query_"))
            {
                if(!({int})PROPERTY_ROOT_MASTER->is_short_property_name(fun[6..]))
                    return 0;
                set_this_object(previous_object());
                found = call_direct_resolved(&result, ob, "query", fun[6..], args...);
            }
            else if(!strstr(fun, "set_"))
            {
                if(!({int})PROPERTY_ROOT_MASTER->is_short_property_name(fun[4..]))
                    return 0;
                set_this_object(previous_object());
                found = call_direct_resolved(&result, ob, "set", fun[4..], args...);
            }
            else if(!strstr(fun, "add_"))
            {
                if(!({int})PROPERTY_ROOT_MASTER->is_short_property_name(fun[4..]))
                    return 0;
                set_this_object(previous_object());
                found = call_direct_resolved(&result, ob, "add", fun[4..], args...);
            }
            else if(!strstr(fun, "delete_"))
            {
                if(!({int})PROPERTY_ROOT_MASTER->is_short_property_name(fun[7..]))
                    return 0;
                set_this_object(previous_object());
                found = call_direct_resolved(&result, ob, "delete", fun[7..], args...);
            }

#if __VERSION__ > "3.5.0.5-U034" && __VERSION__ < "3.5.0.5-U036"
            if (found && !result)
                found = 0;
#endif

            return found;
        });
#endif

    set_driver_hook(H_PRINT_PROMPT,
	function void(string prompt)
	{
	    mixed res;
	    
	    if(!call_resolved(&res, previous_object(), "print_prompt", prompt))
		efun::tell_object(previous_object(), prompt);
	});

    set_driver_hook(H_TELNET_NEG, "got_telnet");
    set_driver_hook(H_NOECHO, function void(int flag, object ob)
	{
	    if(ob) ob->set_noecho(flag);
	});

    set_driver_hook(H_NOTIFY_FAIL, "Wie bitte?\n");
    set_driver_hook(H_NO_IPC_SLOT, MUD_NAME " ist im Moment \u00fcberf\u00fcllt. "
	"Versuche es doch sp\u00e4ter nochmal. :(\n");
    set_driver_hook(H_MODIFY_COMMAND_FNAME, "modify_command"); // Fuer Player
    set_driver_hook(H_MODIFY_COMMAND, "modify_command"); // Fuer NPCs
    set_driver_hook(H_ERQ_STOP,#'erq_stop);
    set_driver_hook(H_SEND_NOTIFY_FAIL,"receive_notify_fail");
    set_driver_hook(H_AUTO_INCLUDE,#'auto_include);

#ifdef H_MSG_DISCARDED
    set_driver_hook(H_MSG_DISCARDED, "\n\n*** Zuviel Text gesendet. ***\n\n");
#endif

    // set_driver_hook(H_SIGNALS, #'crash);

    // set_driver_hook(H_INCLUDE_DIRS, ({"/sys/"}) );

#if 0
    set_driver_hook(H_INCLUDE_DIRS,
    // von Mubo@SiliconDreams, mit Genehmigung fuer UNItopia;
    // andere Muds bitte erst bei Mubo anfragen.
    // Sissi (30.08.97): geht jetzt auch in Mapraeumen
    // Tucita (6.08.98): /p/Sub/Sub[/Sub]/sys, optimiert
      unbound_lambda( ({'inc_name,'current_file}),
	({#'?,
	  ({#'||,
	    ({#'==, 'inc_name, 0}),
	    ({#'==, 'inc_name, ""})
	  }), 0,
	  ({#',,
	    ({#'=, 'erg,
	      ({#'=, 'inc_name, ({#'+, "sys/", 'inc_name})})
	    }),
	    ({#'>, ({#'file_size, 'erg}), 0 })
	  }), 'erg,
	  ({#',,
	    ({#'=, 'cfp, ({#'explode,
		({#'||, ({#'funcall, ({#'symbol_function,"map2domain"}),
			   'current_file }),
			   // /*({*/ ({#'+, "/", 'current_file }) }),
			'current_file }),
		"/"}) }),
	    ({#'=, 'cfpl, ({#'sizeof, 'cfp}) }),
	    ({#'?,
	      ({#'&&,
		({#'>, 'cfpl, 2}),
		({#'member, (["d","z","p","w"]), ({#'[, 'cfp, 0})})
	      }),
	      ({#'?,
		({#',,
		  ({#'=, 'ergp,
		    ({#'+, ({#'[, 'cfp, 0}),
		      ({#'+, "/",
			({#'+, ({#'[, 'cfp, 1}), "/"})
		      })
		    })
		  }),
		  ({#'=, 'erg, ({#'+,'ergp, 'inc_name}) }),
		  ({#'>, ({#'file_size, 'erg}), 0 })
		}), 'erg,
		({#'&&,
		  ({#'>, 'cfpl, 3}),
		  ({#'||,
		    ({#'==, ({#'[, 'cfp, 0}), "d"}),
		    ({#'==, ({#'[, 'cfp, 0}), "z"}),
		    ({#'&&,
		      ({#'==, ({#'[, 'cfp, 0}), "p"}),
		      ({#'>=, ({#'[, ({#'[, 'cfp, 1}), 0}), 'A'}),
		      ({#'<=, ({#'[, ({#'[, 'cfp, 1}), 0}), 'Z'})
		    })
		  })
		}),
		({#'?,
		  ({#',,
		    ({#'+=, 'ergp, ({#'+, ({#'[, 'cfp, 2}), "/"}) }),
		    ({#'=,'erg, ({#'+,'ergp, 'inc_name}) }),
		    ({#'>, ({#'file_size, 'erg}), 0 })
		  }), 'erg,
		  ({#'&&,
		    ({#'>, 'cfpl, 4}),
		    ({#'>=, ({#'[, ({#'[, 'cfp, 2}), 0}), 'A'}),
		    ({#'<=, ({#'[, ({#'[, 'cfp, 2}), 0}), 'Z'}),
		    ({#',,
		      ({#'+=, 'ergp, ({#'+, ({#'[, 'cfp, 3}), "/"}) }),
		      ({#'=,'erg, ({#'+,'ergp, 'inc_name}) }),
		      ({#'>, ({#'file_size, 'erg}), 0 })
		    })
		  }), 'erg,
		  0
		}),
		0
	      }),
	      0
	    })
	  }), 'erg,
	  0
	})
      )
    );
#endif
}

