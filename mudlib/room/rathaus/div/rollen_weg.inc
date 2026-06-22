// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /room/rathaus/div/rollen_weg.inc
// Description: Ausschalten von Funktionen fuer sicherheitskritische NPCs
// Author:	Gnomi

// Hiermit werden die Rollen und die chat Funktionen und alle Funktionen,
// welche irgendwelche frei waehlbaren Funktionen aufrufen, deaktiviert.

mixed do_fun_call(mixed ob, string funktion, varargs mixed *parameter) {}
mixed do_efun_call(string funktion, varargs mixed *parameter) {}
void rolle_v2() {}
void rolle() {}
varargs int beginne_rolle() {}
varargs int starte_rolle(string rollenname, string label, varargs mixed *params) {}

void add_a_chats(mixed msgs) {}
void add_chats(mixed msgs) {}
void load_a_chat(int chance, mixed chat) {}
void load_chat(int chance, mixed chat) {}

// set_parse_conversation kann beliebige Funktionen aufrufen.
private void add_parse_conversation(object ob, mixed *set) {::add_parse_conversation(ob,set);}
private void set_parse_conversation(object ob, mixed *set) {::set_parse_conversation(ob,set);}
