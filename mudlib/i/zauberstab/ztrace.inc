// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/i/zauberstab/ztrace.inc
// Description:	Browser für Trace-Daten
// Author:	Gnomi (30.10.2024)

#include <input_to.h>
#include <message.h>
#include <term.h>

#if __EFUN_DEFINED__(trace_call)
struct trace_info
{
    trace_result tr;
    trace_cursor cur;
    mapping files;
    string last_command;
};

private void handle_input(string str, struct trace_info info);

private void print_code(struct trace_info info)
{
    trace_cursor cur = info.cur;
    string text = sprintf("Objekt: %Q\n", cur.get_object());
    string fname = cur.get_file_name();
    string code;
    int cur_pos = cur.get_line_number();
    int start_pos = max(1, cur_pos - 5);
    int end_pos = cur_pos + 6;

    if (info.files)
    {
        code = info.files[fname];
        if (code)
            code = implode(explode(code, "\n")[start_pos-1..end_pos-1],"\n")+"\n";
    }
    if (!code)
        code = read_file(fname, start_pos, end_pos - start_pos);
    if (!code)
    {
        string domain_fname = map2domain(fname);
        if(domain_fname)
            code = read_file(domain_fname, start_pos, end_pos - start_pos);
    }
    if (!code)
        text += sprintf("--- %s: Nicht lesbar ---\n", fname);
    else
    {
        string *lines = explode(code,"\n");
        if (!sizeof(lines[<1]))
            lines = lines[..<2];

        lines = map(lines, function string(string line) : int pos = start_pos
        {
            return sprintf("%s %4d %c ", pos == cur_pos ? VT_BOLD : "",
                                         pos,
                                         pos == cur_pos ? '>' : ' ')
                 + line
                 + (pos++ == cur_pos ? VT_NORM : "");
        });

        text += sefun::left("--- " + cur.get_file_name() + ": ", 79, "-") + "\n"
             + implode(lines, "\n") + "\n"
             + "-------------------------------------------------------------------------------\n";
    }
    this_player().send_message_to(this_player(), MT_NOTIFY, MA_UNKNOWN, text);
}

private void print_variables(string** variables)
{
    string result = "";
    int width = 8;
    foreach (string* entry: variables)
        width = max(width, text_width(entry[0]));
    width = min(20, width);

    foreach (string* entry: variables)
        result += sprintf("%-=*s %-=*s\n", width, entry[0], 78-width, entry[1]);

    if (!sizeof(result))
        this_player().send_message_to(this_player(), MT_NOTIFY, MA_UNKNOWN,
            "Keine lokalen Variablen aufgezeichnet.");
    else
        this_player().send_message_to(this_player(), MT_NOTIFY, MA_UNKNOWN,
            "-------------------------------------------------------------------------------\n"+
            result+
            "-------------------------------------------------------------------------------\n");
}

private void print_variable(string name, string *values)
{
    if (sizeof(values))
    {
        this_player().send_message_to(this_player(), MT_NOTIFY, MA_UNKNOWN,
            sefun::left("--- Variable '" + name + "': ", 79, "-") + "\n" +
            implode(values, "\n\n") + "\n" +
            "-------------------------------------------------------------------------------\n");
    }
    else
    {
        this_player().send_message_to(this_player(), MT_NOTIFY, MA_UNKNOWN,
            sprintf("Variable '%s' ist unbekannt.\n", name));
    }
}

private void print_prompt(struct trace_info info)
{
    input_to(#'handle_input, INPUT_PROMPT, "[q,n,s,f,v,?] ", info);
}

private void show_step(struct trace_info info)
{
    print_code(info);
    print_prompt(info);
}

private void handle_input(string str, struct trace_info info)
{
    str = trim(str||"");
    if (!sizeof(str))
        str = info.last_command;

    switch (str)
    {
        case "q":
            return;

        case "r":
            break;

        case "n":
        {
            trace_cursor cur = info.cur;
            object prev_ob = cur.get_object();
            string prev_file = cur.get_file_name();
            int prev_line = cur.get_line_number();
            do
            {
                if (!cur.step_over())
                {
                    this_player().send_message_to(this_player(), MT_NOTIFY, MA_UNKNOWN,
                        "Ende des Traces erreicht.");
                    info.last_command = str;
                    print_prompt(info);
                    return;
                }
            } while(cur != info.tr.end() && prev_ob == cur.get_object()
                                         && prev_file == cur.get_file_name()
                                         && prev_line == cur.get_line_number());
            break;
        }

        case "s":
            if (!info.cur.step_into())
            {
                this_player().send_message_to(this_player(), MT_NOTIFY, MA_UNKNOWN,
                    "Ende des Traces erreicht.");
                info.last_command = str;
                print_prompt(info);
                return;
            }
            break;

        case "f":
            if (!info.cur.step_out())
            {
                this_player().send_message_to(this_player(), MT_NOTIFY, MA_UNKNOWN,
                    "Du bist bereits auf der obersten Ebene.");
                info.last_command = str;
                print_prompt(info);
                return;
            }
            break;

        case "v":
            print_variables(info.cur.get_variables());
            info.last_command = str;
            print_prompt(info);
            return;

        case "?":
            info.last_command = "";
            this_player().more("/doc/hilfe/goetter/zauberstab/trace_display", 0, 0, M_AUTO_END,
                ([ "id": "Trace:Help",
                   "info": info,
                ]));
            return;

        default:
            if (!strstr(str, "v "))
            {
                string name = trim(str[2..]);
                if (!sizeof(name))
                    print_variables(info.cur.get_variables());
                else
                    print_variable(name, info.cur.get_variable(name));
                info.last_command = str;
                print_prompt(info);
                return;
            }

            print_prompt(info);
            return;
    }
    info.last_command = str;

    print_code(info);
    print_prompt(info);
}

private void show_trace_result(struct trace_info info)
{
    info.cur = info.tr.begin();
    if (!info.cur)
    {
        this_player().send_message_to(this_player(), MT_NOTIFY, MA_UNKNOWN,
            "Keine Trace-Daten vorhanden.");
        return;
    }
    show_step(info);
}

private void handle_trace_help_end(mapping more_id)
{
    print_prompt(more_id["info"]);
}

#endif
