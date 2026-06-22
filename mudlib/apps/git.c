// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/apps/git.c
// Description:	Hilfesroutinen zur Versionsverwaltung
// Author:	Gnomi

#define AUTO_COMMIT_TIME        259200  // 3 Tage

#include <time.h>

private void nightly_call()
{
#if __EFUN_DEFINED__(git_commit)
    call_out(#'nightly_call, 24*60*60);

    string msg = strftime("Automatischer Commit am %d.%m.%Y", time());

    // Auto-Commit offener Aenderungen
    foreach(string wiz: __MASTER_OBJECT__->query_git_authors_before(time() - AUTO_COMMIT_TIME))
    {
        mapping repos = ([:1]); // Repository -> Array of Files
        mapping wizfiles = __MASTER_OBJECT__->query_git_files_of(wiz);

        if (!sizeof(wizfiles))
            continue;

        foreach(string* info: git_status(m_indices(wizfiles)))
        {
            if (info[2] == "!!") // Ignoriert?
                continue;

            if (member(repos, info[0]))
                repos[info[0]] += ({ info[1] });
            else
                m_add(repos, info[0], info[1..1]);
        }

        foreach (string repo, string* files: repos)
        {
            git_commit(sizeof(repo) ? repo : "/", files, msg, wiz);
            __MASTER_OBJECT__->commit_git_files(map(files, function string(string file) { return repo + "/" + file; }));

            foreach (string file: files)
                m_delete(wizfiles, repo + "/" + file);
        }

        // Dateien, die offenbar nicht committet werden koennen.
        if (sizeof(wizfiles))
            __MASTER_OBJECT__->commit_git_files(m_indices(wizfiles));
    }
#endif
}

private void check_call_out()
{
#ifndef TestMUD
    if (find_call_out(#'nightly_call) < 0)
    {
        mixed t = timearray(time()+24*60*60);
        t[TM_HOUR] = t[TM_MIN] = t[TM_SEC] = 0;

        call_out(#'nightly_call, array_to_time(t) - time());
    }
#endif
}

void create()
{
    check_call_out();
}

void reset()
{
    check_call_out();
}
