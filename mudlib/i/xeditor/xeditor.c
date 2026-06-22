// This file ist part of Avalon Mudlib.
// Copyright 1996-99 by Avatar@Avalon. All rights reserved.
// ----------------------------------------------------------------
// File:        /i/xeditor/xeditor.c
// Description: Zeichenorientierter Editor
// Author:      Avatar

#pragma save_types

#undef AVALON_MUDLIB
#define UNITOPIA_MUDLIB

inherit "/i/tools/getopt";

#include <acl.h>
#include <input_to.h>
#include <getopt.h>
#include <config.h>
#include <apps.h>
#include <rtlimits.h>
#include "xeditor.h"

#ifndef MUD_NAME
#define MUD_NAME "Test-MUD"
#endif
 
#define SECURE if( \
   !owner || \
   this_player() != owner || \
   owner_is_wizard() && geteuid() != owner->query_real_name() ) \
   { \
      write("Dies ist nicht Dein Editor.\n"); \
      return -1; \
   }

private mapping keys;
private mapping options;

private static mapping possible_keys = ([]);
private static int keyrc_lasttime;
private static string key_buffer="";
private static int cur_x, cur_y;
private static int delta_x, delta_y;
private static string filename;
private static object owner;
private static object editor_shadow;
private static int msg_printed;
private static int owner_wants_quit;
private static mapping qst_map;
private static mapping fb_map;
private static int block_begin;
private static int block_end;
private static string *buffer;
private static string S_clr_scr;
private static string S_clr_eos;
private static string S_reset;
private static string S_home;
private static string S_revers;
private static string S_norm;
private static string S_underline;

private static string S_pos_;
#define S_pos(x,y) S_pos_+(y)+";"+(x)+"H"

//
// Predefines
//

//
// public.inc
//
string owner_add_path(string fname);
string owner_query_current_path();
void owner_set_current_path(string path);
string owner_search_filename(string name);
string owner_convert_filename(string fname);
void owner_set_editor_options(mapping options);
mapping owner_query_editor_options();
string owner_query_cap_name();
int owner_is_wizard();
int owner_query_client_height();
int owner_query_client_width();
void editor_save_current_file();
varargs int edit_command(string str, closure call_at_end);
int edit_buffer(string *edit_buffer, closure call_at_end);
void init_xeditor(object _owner);
static int query_remove_after_editing();
void notify_net_dead(object who);
mapping query_editor_keys();
mapping query_editor_options();
object query_editor_owner();


//
// utils.inc
//
private string Head();
varargs private string Line(int not_revers);
private string Clean_line();
private string Cursor();
private string Screen();
private void out(string txt);
private void msg(string txt);
private string editor_read_file(string filename);
private int editor_write_file(string filename, string data);
varargs private string *load_buffer(string str, int template);
private void erase_msg();
private int line_length();
private void add_str(string str);
private void clear_key_buffer();
private varargs void qst(string txt, string def, closure func, mixed extra);
varargs private mixed __qst();
private int qst_ok();
private void qst_esc();
private varargs void ask(string txt, closure func, mixed extra);
void ask_loop(string str, closure func, mixed extra);
private void file_browser_init_buffer();
private varargs void file_browser(closure func, mixed extra, string cd_path,
   string inherit_top);
private void file_browser_quit(int esc);
private void file_browser_selection();
private void add_key(int key, int *path, mixed keys);
private int editor_load_keyrc();
varargs void editor_keyboard_test(string str, int flag);
varargs private void editor_save_options();
private void editor_get_options();
private void convert_options(mapping opt, string ind, string X, string x,
   mixed def, int flag);
private void start_editor();
private void continue_editing();

//
// commands.inc
//
varargs private void cursor_up(int count);
varargs private void cursor_down(int count, int no_output);
private int cursor_on_word();
varargs private void cursor_left(int word);
varargs private void cursor_right(int word);
varargs private int cursor_goto_line_2(string _line, int x, int dx);
private void cursor_goto_line();
private void cursor_delete(int backspace);
private void cursor_delete_line();
private void cursor_return();
private void cursor_tab();
varargs private void cursor_home(int screen);
varargs private void cursor_end(int screen);
private void switch_insert_mode();
private void quit_editor(string txt, int esc);
private int editor_save_3(int res, mixed extra);
private varargs int editor_save_2(string fname, mixed save_block);
private void editor_save();
private int editor_load_3(int res, mixed extra);
varargs private int editor_load_2(string fname);
private void editor_load();
private int editor_merge_2(string str, int template);
private void editor_merge();
private void editor_template();
private void editor_mark_begin();
private void editor_mark_end();
private void editor_block_delete();
private void editor_block_handling(int mode);
private void editor_block_copy();
private void editor_block_move();
private void editor_block_left();
private void editor_block_right();
private void editor_block_unmark();
private void editor_block_save();
private int editor_end_2(int res);
private int editor_end_3(int res);
private void editor_end(int with_save);
varargs void editor_help(string flag);
private void editor_refresh();
private int editor_fn_2b(int res);
private int editor_fn_2(int res);
private void editor_find_next(int backward);
private int editor_replace(string expr);
private int editor_find_3(string cmd);
private int editor_find_2(string expr);
private void editor_find(int quick);
private void editor_find_quick(string expr);

//
// loop.inc
//
void loop(string str);


#include "utils.inc"
#include "commands.inc"
#include "loop.inc"
#include "public.inc"
