// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/secure/master/git.inc
// Description:	Unterstuetzung fuer die Versionsverwaltung
// Author:	Gnomi

// Die Funktionen hier sind nur fuer den Aufruf aus
// /i/zauberstab/zgit.inc gedacht.

mapping git_files = ([:1]);       // Mapping von Datei zu Author (real_name)
mapping git_authors = ([:2]);     // Mapping von Author zu einem Mapping von Dateien und letzte Uhrzeit

private void add_git_file(string file, string* path, string wiz)
{
#if __EFUN_DEFINED__(git_status)
    string orig;

    // Ist es ein Gott oder anderes Objekt?
    if (member(wiz, ':') >= 0 || lower_case(wiz) != wiz)
        return;

    // Diese Verzeichnisse ignorieren.
    if (sizeof(path & ({"save", "var", "log", "Admin", "INVALID_WRITE" })))
        return;

    // Diese Endungen ignorieren.
    foreach (string end: ({".log", ".o", ".db" }))
        if (file[>-sizeof(end)..] == end)
            return;

    orig = git_files[file];
    if (orig != wiz)
    {
        if (widthof(git_authors) < 2)
            git_authors = m_reallocate(git_authors, 2);

        m_delete(git_authors[orig] || ([]), file);
        if (!member(git_authors, wiz))
            m_add(git_authors, wiz, ([ file ]), time());
        else
        {
            m_add(git_authors[wiz], file);
            git_authors[wiz,1] = time();
        }

        m_add(git_files, file, wiz);

        save_master();
    }
#endif
}

mapping query_git_files_of(string wiz)
{
    return copy(git_authors[wiz]) || ([]);
}

mapping query_git_author(string file)
{
    return git_files[file];
}

mapping query_git_authors_before(int last)
{
    return m_reallocate(filter(git_authors, function int(string wiz, <mapping|int>* values)
    {
        return values[1] < last && sizeof(values[0]);
    }), 0);
}

void commit_git_files(string* files)
{
#if __EFUN_DEFINED__(git_status)
    foreach (mixed* filestatus: git_status(files))
    {
        string fname = filestatus[0] + "/" + filestatus[1];
        files -= ({fname});

        do_log_error(sprintf("Datei '%s' hat immer noch \u00c4nderungen.\n", fname), __FILE__, object_name(previous_object()), __LINE__, 0, 0);
    }

    foreach (string file: files)
    {
        string wiz = git_files[file];

        if (wiz)
            m_delete(git_authors[wiz], file);
        m_delete(git_files, file);
    }

    save_master();
#endif
}
