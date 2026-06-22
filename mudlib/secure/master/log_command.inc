// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/secure/master/log_command.inc
// Description:	Loggt Wiz-Commands
// Author:	

#pragma strict_types

#define COMMAND_FILE 	  "/var/adm/logfiles/"+(({string})ob->query_real_name()||name)+".log"
#define PUFFER_FILE(ob,i) "/var/adm/puffer/"+({string})ob->query_real_name()+".puffer"+i

void log_command(string str, string name)
{
    object ob;
    int * t, * ut;
    ob = previous_object();

    if ((!playerp(ob) || ob!=this_interactive()) && !name)
        return;

    ut = utime();
    t = localtime(ut);

    write_file(COMMAND_FILE, 
        sprintf("%3s %3s %'0'2d %4d %'0'2d:%'0'2d:%'0'2d.%'0'2d\n"
	        "    %-=74s\n",
            ({"Sun","Mon","Tue","Wed","Thu","Fri","Sat"})[t[TM_WDAY]],
            ({"Jan","Feb","Mar","Apr","May","Jun",
              "Jul","Aug","Sep","Oct","Nov","Dec"})[t[TM_MON]],
            t[TM_MDAY], t[TM_YEAR],
            t[TM_HOUR], t[TM_MIN], t[TM_SEC], ut[1]/10000,
            str));
}

void clean_puffer()
{
    int tim;
    mixed *dir;

    dir = get_dir("/var/adm/puffer/",GETDIR_NAMES|GETDIR_DATES);
    tim = time();
    for (int i = 0; i < sizeof(dir); i += 2)
    {
	if ((dir[i + 1] + 1468800) < tim)
	    rm("/var/adm/puffer/" + dir[i]); //Nach 17 Tagen loeschen
    }
}

int log_puffer(string *str)
{
    object ob;
    int i;
    clean_puffer();
    ob = previous_object();
    if (!playerp(ob) ||
        (ob!=this_interactive() && !adminp(this_interactive())))
	return 0;
    for(i=1;i<11;i++)
	if(file_size(PUFFER_FILE(ob,i))<0) break;
    if(i==11) return 0;
    write_file(PUFFER_FILE(ob,i), implode(str,""));
    return 1;
}

int may_write_to_mudadm()
{
    return load_name(previous_object())=="/obj/mailreader" &&
	sizeof(get_dir(PUFFER_FILE(environment(previous_object()),"*"))) && 1;
}
