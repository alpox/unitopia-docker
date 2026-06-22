// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/i/zauberstab/zgit.inc
// Description:	Befehle zur Versionsverwaltung
// Author:	Gnomi (22.11.2019)

#include <input_to.h>
#include <strings.h>

#if __EFUN_DEFINED__(git_status)

// Eintraege fuer das Menue.
#define ZLOG_COMMITS    "zlog_commits"
#define ZLOG_BASEDIR    "zlog_basedir"
#define ZLOG_COMMITID   "zlog_commit"
#define ZLOG_FILES      "zlog_files"
#define ZLOG_FILE       "zlog_file"
#define ZLOG_IDX        "zlog_idx"

#define ZCOMMIT_FILES   "zcommit_files"
#define ZCOMMIT_MESSAGE "zcommit_msg"
#define ZCOMMIT_ALL     "zcommit_all"

int z_log(string str)
{
    mapping opt;
    string* files;
    string msg;
    int called;

    SECURE;
    HELP_NO_PIPE("zlog");

    opt = str ? getopt(str, ([ "o": 0, "f": 0, "F": 0, "s": "%s", "S": "%s"]), GO_FAILS) : ([]);
    if (!opt)
        return 0;

    if (opt["f"] && opt["F"])
    {
        this_player()->send_message_to(this_player(), MT_FAIL, MA_UNKNOWN, "zlog: Entweder -f oder -F angeben.");
        return 1;
    }

    if (opt["s"] && opt["S"])
    {
        this_player()->send_message_to(this_player(), MT_FAIL, MA_UNKNOWN, "zlog: Entweder -s oder -S angeben.");
        return 1;
    }

    if (opt["o"])
    {
        object ob = SO(sizeof(opt["args"]) && implode(opt["args"], " "));
        if (!ob)
            return 0;

        files = ({ program_name(ob) });
    }
    else
        files = expand_wildcards(sizeof(opt["args"]) && implode(opt["args"], " "));

    closure show_commits = function int(string|mixed* commits) : int called = &called
    {
        string repo;

        called = 1;

        if (stringp(commits))
        {
            this_player()->send_message_to(this_player(), MT_FAIL, MA_UNKNOWN, commits);
            return 1;
        }

        if (sizeof(commits)<2)
        {
            this_player()->send_message_to(this_player(), MT_FAIL, MA_UNKNOWN, "Keine Commits vorhanden.");
            return 1;
        }

        repo = commits[0];
        if (repo[<1] != '/')
            repo += "/";

        msg = implode(map(commits[1..], function string(mixed* commit) : int pos = 1,
                                                                             showfile = opt["f"] ? 1 : opt["F"] ? 2 : 0
        {
            string entry = sprintf(" %5d: %s %-10s %.48s\n", pos++,
                            shorttimestr(commit[1], 1, TIMESTR_ONLY_DATE),
                            capitalize(commit[2]),
                            commit[3]);
            switch (showfile)
            {
                case 1:
                {
                    int bestcount = -1;
                    string bestname;
                    foreach (mixed* file: commit[5])
                    {
                        int count = file[1] + file[2];
                        if (count > bestcount)
                        {
                            bestcount = count;
                            bestname = file[0];
                        }
                    }
                    if (bestname)
                    {
                        string rest = "";
                        int restlen;
                        if (sizeof(commit[5]) > 1)
                            rest = sprintf(" (+%d)", sizeof(commit[5])-1);

                        bestname = repo + bestname;
                        restlen = 59 - sizeof(rest);
                        if (sizeof(bestname) > restlen)
                            entry += "                   ..." + bestname[<restlen-3..] + rest + "\n";
                        else
                            entry += "                   " + bestname + rest + "\n";
                    }
                    break;
                }

                case 2:
                    entry += implode(map(commit[5], function(mixed* file)
                    {
                        string fname = repo + file[0];
                        if (sizeof(fname) > 59)
                            return "                   ..." + fname[<56..] + "\n";
                        return "                   " + fname + "\n";
                    }), "");
                    break;
            }

            return entry;
        }), "");

        return dynamic_browse(([
            B_TYPE:         "zlog",
            B_START_LINE:   0,
            B_PROMPT:       "Commits [q,<nr>,<,>,?] ",
            B_HELP:         ([
                                B_TYPE:     B_DYNAMICMORE,
                                B_PROMPT:   "Hilfe [q] ",
                                B_FILE:     "/doc/hilfe/goetter/zauberstab/zlog_commits_browser"
                            ]),
            B_DATA:         explode(msg, "\n")[0..<2],
            B_HEADER_LINES: ({
                                "-------- Commits -------------------------------------------------------------",
                                "------------------------------------------------------------------------------",
                                "...---------------------------------------------------------------------------",
                                "...---------------------------------------------------------------------------",
                            }),
            ZLOG_COMMITS:   map(commits[1..], #'[, 0),
            ZLOG_BASEDIR:   commits[0],
        ]));
    };

    if (opt["s"] || opt["S"])
    {
        msg = catch(git_search_commits(show_commits, files, (opt["s"] || opt["S"])[0], opt["S"] ? 1 : 0, (opt["f"] || opt["F"]) ? 1 : 0));
        if (msg)
            this_player()->send_message_to(this_player(), MT_FAIL, MA_UNKNOWN, msg[1..]);
        else if(!called)
            this_player()->send_message_to(this_player(), MT_NOTIFY, MA_UNKNOWN, "Suche Commits...");
        return 1;
    }
    else
    {
        mixed* commits;

        msg = catch(commits = git_list_commits(files, (opt["f"] || opt["F"]) ? 1 : 0));
        if (msg)
        {
            this_player()->send_message_to(this_player(), MT_FAIL, MA_UNKNOWN, msg[1..]);
            return 1;
        }

        return funcall(show_commits, commits);
    }
}

static string* zlog_display(mapping menue)
{
    return staticmore_display(menue);
}

static int zlog_total(mapping menue)
{
    return sizeof(menue[B_DATA]);
}

static mapping zlog_search_forward(mapping menue)
{
    return staticmore_search_forward(menue);
}

static mapping zlog_select(int nr, mapping menue)
{
    mixed* infos;
    string msg;

    if (nr < 1 || nr > sizeof(menue[ZLOG_COMMITS]))
        return 0;

    msg = catch(infos = git_info_commit(menue[ZLOG_BASEDIR], menue[ZLOG_COMMITS][nr-1]));
    if (msg)
    {
        this_player()->send_message_to(this_player(), MT_FAIL, MA_UNKNOWN, msg[1..]);
        return 0;
    }

    infos[4] = explode(wrap(infos[4], 78), "\n");
    if (infos[4][<1] == "")
        infos[4] = infos[4][..<2];

    return ([
        B_TYPE:         "zlog_commit",
        B_START_LINE:   0,
        B_PROMPT:       "Commit [q,z,<nr>,<,>,r,?] ",
        B_HELP:         ([
                            B_TYPE:     B_DYNAMICMORE,
                            B_PROMPT:   "Hilfe [q] ",
                            B_FILE:     "/doc/hilfe/goetter/zauberstab/zlog_commit_browser"
                        ]),
        B_DATA:         ({
                            sprintf("Urheber: %-10s %58s", capitalize(infos[2]), timestr(infos[1])),
                            "",
                        }) + explode(wrap(infos[3],78), "\n")
                           + infos[4] +
                        ({
                            "         Änderung   Datei in " + menue[ZLOG_BASEDIR],
                            "         ---------  ----------------------------------------------------------",
                        }) +
                        map(infos[5..], function string(mixed* file) : int pos = 1
                        {
                            string fname = file[0];
                            if (sizeof(fname) > 55)
                                fname = "..." + fname[>-52..];

                            return sprintf(" %5d: %+5d,%-5d %s", pos++, file[1], -file[2], fname);
                        }),
        B_HEADER_LINES: ({
                            "-------- Commit --------------------------------------------------------------",
                            "------------------------------------------------------------------------------",
                            "...---------------------------------------------------------------------------",
                            "...---------------------------------------------------------------------------",
                        }),
        ZLOG_BASEDIR:   menue[ZLOG_BASEDIR],
        ZLOG_COMMITID:  menue[ZLOG_COMMITS][nr-1],
        ZLOG_FILES:     map(infos[5..], #'[, 0),
        ZLOG_IDX:       nr-1,
    ]);
}

static string* zlog_commit_display(mapping menue)
{
    return staticmore_display(menue);
}

static int zlog_commit_total(mapping menue)
{
    return sizeof(menue[B_DATA]);
}

static int|mapping* zlog_commit_action(string str, mapping* menues)
{
    mapping commitmenue = menues[<1];
    string msg;

    switch (str)
    {
        case "r":
        {
            string repo = commitmenue[ZLOG_BASEDIR];
            if (repo[<1] != '/')
                repo += "/";
            foreach (string file: commitmenue[ZLOG_FILES])
            {
                file = explode(file, " => ")[0];
                if (!MAY_WRITE(repo + file, this_player()))
                {
                    this_player()->send_message_to(this_player(), MT_FAIL, MA_UNKNOWN, "Du hast keine Schreibrechte auf '" + repo + file +"'.");
                    return B_DONE;
                }
            }

            msg = catch(git_reverse(commitmenue[ZLOG_BASEDIR], commitmenue[ZLOG_COMMITID]));
            if (msg)
            {
                this_player()->send_message_to(this_player(), MT_FAIL, MA_UNKNOWN, msg[1..]);
                return B_DONE;
            }

            this_player()->send_message_to(this_player(), MT_NOTIFY, MA_UNKNOWN, "Änderungen rückgängig gemacht.");
            return B_DONE;
        }

        case "+":
        case "-":
        {
            int nr = commitmenue[ZLOG_IDX];

            if (str == "+")
            {
                nr++;
                if (nr == sizeof(menues[<2][ZLOG_COMMITS]))
                    return B_DONE;
            }
            else if (str == "-")
            {
                nr--;
                if (nr < 0)
                    return B_DONE;
            }

            mapping m = zlog_select(nr+1, menues[<2]);
            if (!m)
                return B_DONE;

            return menues[0..<2] + ({ m });
        }
    }

    return B_NOTHING;
}

static mapping zlog_commit_search_forward(mapping menue)
{
    return staticmore_search_forward(menue);
}

static mapping zlog_commit_select(int nr, mapping menue)
{
    string msg, diff;

    if (nr < 1 || nr > sizeof(menue[ZLOG_FILES]))
        return 0;

    msg = catch(diff = git_show_diff(menue[ZLOG_BASEDIR], menue[ZLOG_COMMITID], explode(menue[ZLOG_FILES][nr-1], " => ")[<1]));
    if (msg)
    {
        this_player()->send_message_to(this_player(), MT_FAIL, MA_UNKNOWN, msg[1..]);
        return 0;
    }

    return ([
        B_TYPE:         "zlog_commit_diff",
        B_PROMPT:       "Diff [q,z,<,>,r,w datei,?] ",
        B_HELP:         ([
                            B_TYPE:     B_DYNAMICMORE,
                            B_PROMPT:   "Hilfe [q] ",
                            B_FILE:     "/doc/hilfe/goetter/zauberstab/zlog_diff_browser"
                        ]),
        B_DATA:         explode(diff[0..<2], "\n"),
        B_HEADER_LINES: ({
                            "-------- Diff ----------------------------------------------------------------",
                            "------------------------------------------------------------------------------",
                            "...---------------------------------------------------------------------------",
                            "...---------------------------------------------------------------------------",
                        }),
        ZLOG_BASEDIR:   menue[ZLOG_BASEDIR],
        ZLOG_COMMITID:  menue[ZLOG_COMMITID],
        ZLOG_FILE:      menue[ZLOG_FILES][nr-1],
        ZLOG_IDX:       nr-1,
    ]);
}

static string* zlog_commit_diff_display(mapping menue)
{
    return staticmore_display(menue);
}

static int zlog_commit_diff_total(mapping menue)
{
    return sizeof(menue[B_DATA]);
}

static int|mapping* zlog_commit_diff_action(string str, mapping* menues)
{
    mapping diffmenue = menues[<1];
    string msg;

    // Datei mit Erfolgsmeldung rausschreiben.
    closure cat_file = function void(string fpath)
    {
        msg = catch(git_cat(diffmenue[ZLOG_BASEDIR], diffmenue[ZLOG_COMMITID], diffmenue[ZLOG_FILE], fpath));
        if (msg)
            this_player()->send_message_to(this_player(), MT_FAIL, MA_UNKNOWN, msg[1..]);
        else
            this_player()->send_message_to(this_player(), MT_NOTIFY, MA_UNKNOWN, "Datei geschrieben.\n");
    };

    // Pruefen, ob Datei existiert, danach Datei rausschreiben.
    closure check_cat_file = function int(string fname)
    {
        string fpath = ({string})ADD_PATH(trim(fname));

        int fsize = file_size(fpath);
        if (fsize == FSIZE_DIR)
            this_player()->send_message_to(this_player(), MT_NOTIFY, MA_UNKNOWN, sprintf("%s ist ein Verzeichnis.", fpath));
        else if (fsize != FSIZE_NOFILE)
        {
            this_player()->send_message_to(this_player(), MT_NOTIFY, MA_UNKNOWN, sprintf("%s existiert bereits.", fpath));
            input_to(function void(string answer)
            {
                if (member((["j", "ja", "y", "yes"]), lower_case(answer || "")))
                    funcall(cat_file, fpath);
                else
                    this_player()->send_message_to(this_player(), MT_NOTIFY, MA_UNKNOWN, "Schreiben abgebrochen.\n");

                menues[<1][B_FLAGS] |= BF_NO_DISPLAY;
                dynamic_browse(menues);

            }, INPUT_PROMPT, "Überschreiben (j/n)? ");
            return 0;
        }
        else
            funcall(cat_file, fpath);
        return 1;
    };

    switch (str)
    {
        case "+":
        case "-":
        {
            int nr = diffmenue[ZLOG_IDX];

            if (str == "+")
            {
                nr++;
                if (nr == sizeof(menues[<2][ZLOG_FILES]))
                    return B_DONE;
            }
            else if (str == "-")
            {
                nr--;
                if (nr < 0)
                    return B_DONE;
            }

            mapping m = zlog_commit_select(nr+1, menues[<2]);
            if (!m)
                return B_DONE;

            return menues[0..<2] + ({ m });
        }

        case "r":
        {
            string file = diffmenue[ZLOG_BASEDIR];
            if (file[<1] != '/')
                file += "/";
            file += diffmenue[ZLOG_FILE];

            if (!MAY_WRITE(file, this_player()))
            {
                this_player()->send_message_to(this_player(), MT_FAIL, MA_UNKNOWN, "Du hast keine Schreibrechte auf '" + file +"'.");
                return B_DONE;
            }

            msg = catch(git_reverse(diffmenue[ZLOG_BASEDIR], diffmenue[ZLOG_COMMITID], diffmenue[ZLOG_FILE]));
            if (msg)
            {
                this_player()->send_message_to(this_player(), MT_FAIL, MA_UNKNOWN, msg[1..]);
                return B_DONE;
            }

            this_player()->send_message_to(this_player(), MT_NOTIFY, MA_UNKNOWN, "Änderungen an '" + file + "' rückgängig gemacht.");
            return B_DONE;
        }

        case "w":
            input_to(function void(string fname)
            {
                if (strlen(fname) && !funcall(check_cat_file, fname))
                    return;

                menues[<1][B_FLAGS] |= BF_NO_DISPLAY;
                dynamic_browse(menues);
            }, INPUT_PROMPT, "Dateiname: ");
            return B_QUIT;

        default:
            if (sizeof(str) > 2 && str[0..1] == "w ")
            {
                if (funcall(check_cat_file, str[2..]))
                    return B_DONE;
                else
                    return B_QUIT;
            }
            break;
    }

    return B_NOTHING;
}

static mapping zlog_commit_diff_search_forward(mapping menue)
{
    return staticmore_search_forward(menue);
}

int z_commit(string str)
{
    mapping opt;
    string *files, *optpaths;
    mixed** info;
    string msg;

    SECURE;
    HELP_NO_PIPE("zcommit");

    opt = str ? getopt(str, ([ "f": 0, "m": "\"%s\"" ]), GO_FAILS) : ([]);
    if (!opt)
        return 0;

    if (opt["f"])
    {
        optpaths = files = expand_wildcards(sizeof(opt["args"]) && implode(opt["args"], " "));
    }
    else
    {
        mapping wizfiles = ({mapping})__MASTER_OBJECT__->query_git_files_of(geteuid(this_player()));

        if (sizeof(opt["args"]))
        {
            optpaths = expand_wildcards(sizeof(opt["args"]) && implode(opt["args"], " "));

            foreach (string fpath: optpaths)
                if (sizeof(fpath) < 2 || member(({"/d/", "/d", "/p/", "/p", "/z/", "/z", "/de"}), fpath[0..2])<0)
                {
                    this_player()->send_message_to(this_player(), MT_FAIL, MA_UNKNOWN, "Du kannst nur Dateien in /d, /p oder /z committen.");
                    return 1;
                }

            files = m_indices(filter(wizfiles, function int(string wizfile)
            {
                foreach (string fpath: optpaths)
                    if (!strstr(wizfile, fpath))
                        return 1;
                return 0;
            }));
        }
        else
            files = m_indices(wizfiles);

        if (!sizeof(files))
        {
            this_player()->send_message_to(this_player(), MT_FAIL, MA_UNKNOWN, "Du hast keine offenen Änderungen" +
                (optpaths ? " in " + liste(optpaths, ", ") : "") + ".");
            return 1;
        }
    }

    msg = catch(info = git_status(files));
    if (msg)
    {
        this_player()->send_message_to(this_player(), MT_FAIL, MA_UNKNOWN, msg[1..]);
        return 1;
    }

    if (!sizeof(info))
    {
        this_player()->send_message_to(this_player(), MT_FAIL, MA_UNKNOWN, "Keine Änderungen vorhanden oder nicht Teil des Repositories.");
        return 1;
    }

    // Ignorierte Dateien ausblenden
    info = filter(info, function int(mixed* file) { return file[2] != "!!"; });
    if (!sizeof(info))
    {
        this_player()->send_message_to(this_player(), MT_FAIL, MA_UNKNOWN,
            sizeof(opt["args"]) ? "Die geänderten Dateien sind nicht Teil des Repositories."
                                : "Keine Änderungen vorhanden oder nicht Teil des Repositories.");
        return 1;
    }

    return dynamic_browse(([
        B_TYPE:         "zcommit",
        B_START_LINE:   0,
        B_PROMPT:       "Änderungen [q,c,<nr>,n,a,d <nr>,<,>,?] ",
        B_HELP:         ([
                            B_TYPE:     B_DYNAMICMORE,
                            B_PROMPT:   "Hilfe [q] ",
                            B_FILE:     "/doc/hilfe/goetter/zauberstab/zcommit_file_browser"
                        ]),
        B_HEADER_LINES: ({
                            "-------- Geänderte Dateien: --------------------------------------------------",
                            "------------------------------------------------------------------------------",
                            "...---------------------------------------------------------------------------",
                            "...---------------------------------------------------------------------------",
                        }),
        ZCOMMIT_FILES:  map(sort_array(info, function int(mixed* file1, mixed* file2)
                        {
                            if (file1[0] != file2[0])
                                return file1[0] > file2[0];
                            return file1[1] > file2[1];
                        }),function mixed*(mixed* file) : object tp = this_player()
                        {
                            int maywrite = (({mixed})__MASTER_OBJECT__->valid_write(file[0] + "/" + file[1], geteuid(tp), "git_commit_test", tp)) == 1;
                            return file + ({ maywrite, maywrite });
                        }),
        ZCOMMIT_MESSAGE: sizeof(opt["m"]) && implode(opt["m"], " "),
        ZCOMMIT_ALL:    opt["f"],
    ]));
}

static string* zcommit_display(mapping menue)
{
    int start = menue[B_CURRENT_LINE];
    int end = start + menue[B_NUM_LINES];
    mixed** files = menue[ZCOMMIT_FILES];
    if (start > sizeof(files))
        return 0;
    if (end >= sizeof(files))
        end = sizeof(files)-1;
    if (start < 0)
        start = 0;
    if (end < start) return 0;

    return map(files[start..end], menue[ZCOMMIT_ALL] ?
    function string(mixed* file) : int pos = start + 1
    {
        string fname = file[0] + "/" + file[1];
        string wiz = ({string})__MASTER_OBJECT__->query_git_author(fname);
        if (sizeof(fname) > 55)
            fname = "..." + fname[>-52..];

        if (wiz)
            wiz = "("+capitalize(wiz)+")";

        return sprintf(" %1s %5d: %-55s %12s", file[3] ? "*" : " ", pos++, fname, wiz || "");
    } :
    function string(mixed* file) : int pos = start + 1
    {
        string fname = file[0] + "/" + file[1];
        if (sizeof(fname) > 68)
            fname = "..." + fname[>-65..];

        return sprintf(" %1s %5d: %s", file[3] ? "*" : " ", pos++, fname);
    });
}

static int zcommit_total(mapping menue)
{
    return sizeof(menue[ZCOMMIT_FILES]);
}

static mapping zcommit_search_forward(mapping menue)
{
    int start = menue[B_CURRENT_LINE]+1;
    mixed** files = menue[ZCOMMIT_FILES];
    string search = menue[B_SEARCH_STRING];
    int end = sizeof(files);

    if (start > sizeof(files) || !stringp(search) || !sizeof(search))
        return 0;
    if (start < 0)
        start = 0;
    if (end <= start)
        return 0;

    for (int i = start; i < end ;i++)
    {
        mixed* file = files[i];
        if (strstr(file[0] + "/" + file[1], search) >= 0)
        {
            menue[B_CURRENT_LINE] = i;
            return menue;
        }
    }
    return 0;
}

private void do_commit(mapping menue)
{
    if (!menue[ZCOMMIT_MESSAGE])
    {
        this_player()->mini_ed(function void(string* text)
        {
            if(!text || !sizeof(text - ({""})))
                return;

            menue[ZCOMMIT_MESSAGE] = implode(text, "\n");
            do_commit(menue);
        }, 0, 0,
        ([
            MINI_ED_START_TEXT: "Gib bitte die Änderungsmeldung ein:\n"
                                "Auf der ersten Zeile eine Zusammenfassung, danach eine Leerzeile\n"
                                "und anschließend eine genauere Beschreibung der Änderung.\n\n"
                                "Mit '**' oder '.' kannst Du das Editieren beenden, mit '~q' abbrechen.\n",
            MINI_ED_TITLE:      "Änderungsmeldung",
        ]));
    }
    else
    {
        mapping repos = ([]);
        foreach (mixed* file: menue[ZCOMMIT_FILES])
        {
            if (!file[3])
                continue;

            if (!member(repos, file[0]))
                m_add(repos, file[0], file[1..1]);
            else
                repos[file[0]] += file[1..1];
        }

        foreach (string repo, string* files: repos)
        {
            string msg = catch(git_commit(sizeof(repo) ? repo : "/", files, menue[ZCOMMIT_MESSAGE]));
            if (msg)
            {
                this_player()->send_message_to(this_player(), MT_FAIL, MA_UNKNOWN, msg[1..]);
                return;
            }

            __MASTER_OBJECT__->commit_git_files(map(files, function string(string file) { return repo + "/" + file; }));
        }

        this_player()->send_message_to(this_player(), MT_NOTIFY, MA_UNKNOWN, "Commit durchgeführt.");
    }
}

private int has_files_to_commit(mapping menue)
{
    foreach (mixed* file: menue[ZCOMMIT_FILES])
        if (file[3])
            return 1;
    return 0;
}

static int|mapping* zcommit_action(string str, mapping* menues)
{
    switch (str)
    {
        case "Q":
        case "q":
            if (str == "Q" || !has_files_to_commit(menues[<1]))
                return B_QUIT;

            input_to(function void(string answer)
            {
                if (sizeof(answer) && member("jy", answer[0]) >= 0)
                    do_commit(menues[<1]);
            }, INPUT_PROMPT, "Die Änderungen committen (ja/nein)? ");
            return B_QUIT;

        case "c":
            if (!has_files_to_commit(menues[<1]))
            {
                this_player()->send_message_to(this_player(), MT_FAIL, MA_UNKNOWN, "Keine Dateien zum Committen ausgewählt.");
                return B_DONE;
            }

            do_commit(menues[<1]);
            return B_QUIT;

        case "n":
            foreach (mixed* file: menues[<1][ZCOMMIT_FILES])
                file[3] = 0;

            return B_REDRAW;

        case "a":
            foreach (mixed* file: menues[<1][ZCOMMIT_FILES])
                file[3] = file[4];

            return B_REDRAW;

        default:
        {
            int nr;

            if (sscanf(str, "%d", nr)==1 && nr > 0)
            {
                mixed** files = menues[<1][ZCOMMIT_FILES];
                if (nr <= sizeof(files))
                {
                    mixed* file = files[nr-1];
                    if (file[3])
                        file[3] = 0;
                    else if (!file[4])
                    {
                        this_player()->send_message_to(this_player(), MT_FAIL, MA_UNKNOWN, "Die Datei darfst Du nicht schreiben.");
                        return B_DONE;
                    }
                    else
                        file[3] = 1;
                    return B_REDRAW;
                }
            }
            else if (sscanf(str, "d%d", nr)==1 && nr > 0)
            {
                mixed** files = menues[<1][ZCOMMIT_FILES];
                if (nr <= sizeof(files))
                {
                    mixed* file = files[nr-1];
                    string msg, diff;

                    if (file[1][<1] == '/')
                    {
                        this_player()->send_message_to(this_player(), MT_FAIL, MA_UNKNOWN, "Das ist ein neues Verzeichnis.");
                        return B_DONE;
                    }

                    if (file[2][0] == '?')
                    {
                        return menues + ({ ([
                            B_TYPE:         B_DYNAMICMORE,
                            B_PROMPT:       "Datei [q,z,<,>,?] ",
                            B_FILE:         file[0] + "/" + file[1],
                            B_HEADER_LINES: ({
                                                "-------- Neue Datei ----------------------------------------------------------",
                                                "------------------------------------------------------------------------------",
                                                "...---------------------------------------------------------------------------",
                                                "...---------------------------------------------------------------------------",
                                            }),
                        ]) });
                    }

                    msg = catch(diff = git_status_diff(sizeof(file[0]) ? file[0] : "/", file[1]));
                    if (msg)
                    {
                        this_player()->send_message_to(this_player(), MT_FAIL, MA_UNKNOWN, msg[1..]);
                        return B_DONE;
                    }

                    return menues + ({ ([
                        B_TYPE:         B_STATICMORE,
                        B_PROMPT:       "Diff",
                        B_HELP:         ([
                                            B_TYPE:     B_DYNAMICMORE,
                                            B_PROMPT:   "Hilfe [q] ",
                                            B_FILE:     "/doc/hilfe/goetter/zauberstab/zlog_commit_diff_browser"
                                        ]),
                        B_DATA:         explode(diff[0..<2], "\n"),
                        B_HEADER_LINES: ({
                                            "-------- Diff ----------------------------------------------------------------",
                                            "------------------------------------------------------------------------------",
                                            "...---------------------------------------------------------------------------",
                                            "...---------------------------------------------------------------------------",
                                        }),
                    ]) });
                }
            }
            break;
        }
    }
    return B_NOTHING;
}

#endif /* __EFUN_DEFINED__(git_status) */
