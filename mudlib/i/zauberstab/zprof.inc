// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/i/zauberstab/zprof.inc
// Description:	Browser für Profiling-Daten
// Author:	Gnomi (18.03.2024)

#include <input_to.h>
#include <strings.h>

#if __EFUN_DEFINED__(profile_call)

// Menüeinträge
#define PROF_RESULT    "prof_result"
#define PROF_FILES     "prof_files"
#define PROF_FILE      "prof_file"
#define PROF_NUM_LINES "prof_num_lines"

#define PROF_METRIC_TICKS    0
#define PROF_METRIC_DURATION 1
static int prof_metric;
static int showing_indirect_costs;

private int get_file_line_numbers(string file)
{
    int num;
    int last;

    foreach (bytes block: read_bytes_async(file))
    {
        num += sizeof(block & b"\n");
        if (sizeof(block))
            last = block[<1];
    }

    if (last && last != '\n')
        num++;

    return num;
}

private void show_profile_result(profile_result prof)
{
    string* files;

    if (prof.is_empty())
    {
        this_player()->send_message_to(this_player(), MT_NOTIFY, MA_UNKNOWN,
            "Es wurden keine Profilingdaten aufgezeichnet.");
        return;
    }

    files = prof.get_files();
    dynamic_browse(([
        B_TYPE:         "prof_files",
        B_START_LINE:   0,
        B_PROMPT:       "Dateien [q,<nr>,<,>,?] ",
        B_HELP:         ([
                            B_TYPE:     B_DYNAMICMORE,
                            B_PROMPT:   "Hilfe [q] ",
                            B_FILE:     "/doc/hilfe/goetter/zauberstab/prof_files_browser"
                        ]),
        B_HEADER_LINES: ({
                            "-------- Dateien -------------------------------------------------------------",
                            "------------------------------------------------------------------------------",
                            "...---------------------------------------------------------------------------",
                            "...---------------------------------------------------------------------------",
                        }),
        B_FLAGS:        BF_DIRTY,
        PROF_RESULT:    prof,
        PROF_FILES:     files,
    ]));
}

static mapping* prof_files_reload(mapping* menues)
{
    profile_result prof = menues[<1][PROF_RESULT];
    closure format;

    switch (prof_metric)
    {
        default:
        case PROF_METRIC_TICKS:
            format = function string(string filename) : int pos = 1
                     {
                        return sprintf("%7d | %3d: %-=63s\n", prof.get_file_cost(filename), pos++, filename);
                     };
            break;

        case PROF_METRIC_DURATION:
            format = function string(string filename) : int pos = 1
                     {
                        return sprintf("%5dµs | %3d: %-=63s\n", prof.get_file_time(filename)/1000, pos++, filename);
                     };
            break;
    }

    menues[<1][B_DATA] = explode(implode(map(menues[<1][PROF_FILES], format), ""), "\n")[..<2];

    return menues;
}

static string* prof_files_display(mapping menue)
{
    return staticmore_display(menue);
}

static int prof_files_total(mapping menue)
{
    return sizeof(menue[B_DATA]);
}

static mapping prof_files_search_forward(mapping menue)
{
    return staticmore_search_forward(menue);
}

static mapping*|int prof_files_action(string str, mapping* menues)
{
    switch(str)
    {
        case "t":
            prof_metric = PROF_METRIC_DURATION;
            menues[<1][B_FLAGS] |= BF_DIRTY;
            return menues;

        case "e":
            prof_metric = PROF_METRIC_TICKS;
            menues[<1][B_FLAGS] |= BF_DIRTY;
            return menues;
    }

    return B_NOTHING;
}

static mapping prof_files_select(int nr, mapping menue)
{
    profile_result prof = menue[PROF_RESULT];
    string file;

    if (nr < 1 || nr > sizeof(menue[PROF_FILES]))
        return 0;
    file = menue[PROF_FILES][nr-1];

    return ([
        B_TYPE:         "prof_file",
        B_START_LINE:   1,
        B_CURRENT_LINE: max(1, prof.get_first_line(file)-2),
        B_PROMPT:       explode(file, "/")[<1] + " (%d/%d) [q,z,<nr>,<,>,r,?] ",
        B_HELP:         ([
                            B_TYPE:     B_DYNAMICMORE,
                            B_PROMPT:   "Hilfe [q] ",
                            B_FILE:     "/doc/hilfe/goetter/zauberstab/prof_file_display"
                        ]),
        B_HEADER_LINES: ({
                            sizeof(file) < 60
                            ? ("-------- " + file + " --------" + "-"*(60-sizeof(file)))
                            : ("-------- " + file[<60..] + " --------"),
                            "------------------------------------------------------------------------------",
                            "...---------------------------------------------------------------------------",
                            "...---------------------------------------------------------------------------",
                        }),
        PROF_RESULT:    prof,
        PROF_FILE:      file,
        PROF_NUM_LINES: get_file_line_numbers(file),
    ]);
}

static string* prof_file_display(mapping menue)
{
    profile_result prof = menue[PROF_RESULT];
    string file = menue[PROF_FILE];
    string content;
    closure format;

    if (!menue[PROF_NUM_LINES])
        return ({ "Datei ist nicht lesbar." });

    content = read_file(file, menue[B_CURRENT_LINE], menue[B_NUM_LINES]);
    if (!content)
        return ({ "--- Ende der Datei --- " });

    if (content[<1] == '\n')
        content = content[..<2];

    switch (prof_metric)
    {
        default:
        case PROF_METRIC_TICKS:
            if (showing_indirect_costs)
                format = function string(string line) : int pos = menue[B_CURRENT_LINE]
                         {
                             int cost = prof.get_line_cost(file, pos) + prof.get_line_indirect_cost(file, pos++);
                             if (cost)
                                 return sprintf("%7d | %.68s", cost, line);
                             else
                                 return sprintf("        | %.68s", line);
                         };
            else
                format = function string(string line) : int pos = menue[B_CURRENT_LINE]
                         {
                             int cost = prof.get_line_cost(file, pos++);
                             if (cost)
                                 return sprintf("%7d | %.68s", cost, line);
                             else
                                 return sprintf("        | %.68s", line);
                         };
            break;

        case PROF_METRIC_DURATION:
            if (showing_indirect_costs)
                format = function string(string line) : int pos = menue[B_CURRENT_LINE]
                         {
                             int ns = prof.get_line_time(file, pos) + prof.get_line_indirect_time(file, pos++);
                             if (ns)
                                 return sprintf("%5dµs | %.68s", ns/1000, line);
                             else
                                 return sprintf("        | %.68s", line);
                         };
            else
                format = function string(string line) : int pos = menue[B_CURRENT_LINE]
                         {
                             int ns = prof.get_line_time(file, pos++);
                             if (ns)
                                 return sprintf("%5dµs | %.68s", ns/1000, line);
                             else
                                 return sprintf("        | %.68s", line);
                         };
            break;
    }

    return map(explode(content, "\n"), format);
}

static int prof_file_total(mapping menue)
{
    return menue[PROF_NUM_LINES];
}

static string prof_file_prompt(mapping menue)
{
    return sprintf(menue[B_PROMPT], menue[B_CURRENT_LINE], menue[PROF_NUM_LINES]);
}

static mapping prof_file_search_forward(mapping menue)
{
    string file = menue[PROF_FILE];
    string str = lower_case(menue[B_SEARCH_STRING]);
    int start = menue[B_CURRENT_LINE];
    int cur = 0;

    foreach (string line: read_lines(file))
    {
        if (++cur <= start)
            continue;
        if (str in lower_case(line))
        {
            menue[B_CURRENT_LINE] = cur;
            return menue;
        }
    }

    return 0;
}

static mapping*|int prof_file_action(string str, mapping* menues)
{
    int line;

    switch(str)
    {
        case "t":
            prof_metric = PROF_METRIC_DURATION;
            menues[<2][B_FLAGS] |= BF_DIRTY;
            return menues;

        case "e":
            prof_metric = PROF_METRIC_TICKS;
            menues[<2][B_FLAGS] |= BF_DIRTY;
            return menues;

        case "i":
            showing_indirect_costs = !showing_indirect_costs;
            menues[<2][B_FLAGS] |= BF_DIRTY;
            return menues;

        default:
            if (!sizeof(str))
                break;

            if (sscanf(str, "%d", line) && line > 0 && line <= menues[<1][PROF_NUM_LINES])
            {
                menues[<1][B_CURRENT_LINE] = line;
                return menues;
            }
            break;
    }

    return B_NOTHING;
}

#endif /* __EFUN_DEFINED__(profile_call) */
