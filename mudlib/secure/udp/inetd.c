// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/secure/udp/inetd.c
// Description:	INETD fuer die Intermud-Kommunikation
// Author:	Zebedee
// Modified by:	Freaky (06.10.94)
//		Garthan (22.02.96) news added to COMMANDS
//		Freaky (02.03.96) Fehler in receive_udp verbessert
//		Freaky (19.11.97) Security-Fix von Mubo und Police in send_udp

/*
 * UDP port handling code. Version 0.61
 * Written by Nostradamus for Zebedee.
 * Developed from an original concept by Alvin@Sushi.
 */

#pragma strict_types

#include <rtlimits.h>
#include <level.h>
#include <udp.h>

/* --- Configurable definitions. --- */

/* CD muds will probably need these include file. */
/* #include <std.h> */
/* #include "/config/sys/local.h" */

/* Public commands which will be accessible to any unlisted muds.
 * PING, QUERY and REPLY are included by default. */
#define COMMANDS ({ "channel", "finger", "tell", "locate", "who", "mail", "news" })

/* Define this to the object that receives incoming packets and passes
 * them to the inetd. Undefine for no receive_udp() security.
 * NOTE: The string must be of the format that object_name() returns. */
#define UDP_MASTER	__MASTER_OBJECT__

/* The maximum number of characters we can send in one packet.
 * You may need to reduce this, but 512 should be safe. */
#define MAX_PACKET_LEN	1024

/* --- End of Config. Do not alter anything below. --- */

#define UNKNOWN		0
#define UP		time()
#define DOWN(x)		(-abs(x))

#define DELIMITER	"|"
#define RETRY		"_RETRY"

private mapping hosts;              // "lower_case_mud_name" -> ({ entries - see HL_* defines })
private mapping addrs;              // "ip_address port" -> "lower_case_mud_name"
private mapping pending_data;       // "target_mud_name:packet_id" -> ([ packet sent ])
private mapping incoming_packets;   // "source_mud_name:packet_id" -> ({ received partial packets })
private int packet_id;

void set_host_list();
string encode_packet(mapping data);
varargs string send_udp(string mudname, mapping data, int expect_reply);

void log_message(string msg)
{
    sys_log(INETD_LOG_FILE, shorttimestr(time()) + ": " + msg + "\n");
}

void debug_message(string msg, varargs mixed* args)
{
#if 0
    sys_log(INETD_LOG_FILE, sprintf("%s: " + msg + "\n", shorttimestr(time()), args...));
#endif
}

void create()
{
    packet_id = 0;
    pending_data = ([ ]);
    incoming_packets = ([ ]);
    hosts = ([ ]);
    addrs = ([ ]);
    set_host_list();
    if (!this_player())
	call_out("startup", 1);
}

string* addr_ping_queue = ({});
private void flush_addr_ping()
{
    foreach(string addr: addr_ping_queue[0..PINGS_AT_ONCE-1])
    {
        string* dest = explode(addr, " ");
        string packet = encode_packet(([ REQUEST: PING, NAME: LOCAL_NAME, UDP_PORT: LOCAL_UDP_PORT, ID: 0 ]));
        if (packet && sizeof(packet) <= MAX_PACKET_LEN)
        {
            debug_message("send_ping_addr: Sending ping for %s %s", dest[0], dest[1]);
            efun::send_udp(dest[0], to_int(dest[1]), to_bytes(packet, "ASCII//TRANSLIT"));
        }
        else
            debug_message("send_ping_addr: Ping packet to big for %s %s", dest[0], dest[1]);
    }
    addr_ping_queue = addr_ping_queue[PINGS_AT_ONCE..];
    if (sizeof(addr_ping_queue))
        call_out(#'flush_addr_ping, 1);
}

private void send_ping_addr(string addr, int port)
{
    string key = addr + " " + port;
    if (!(key in addr_ping_queue))
    {
        debug_message("send_ping_addr: Scheduling ping for %s %d", addr, port);
        addr_ping_queue += ({key});
        if (sizeof(addr_ping_queue) == 1)
            call_out(#'flush_addr_ping, 1);
    }
    else
        debug_message("send_ping_addr: Duplicate ping request for %s %d", addr, port);
}

string* ping_queue = ({});
void send_ping(string name)
{
    if(!name)
    {
	if(sizeof(ping_queue))
	{
	    foreach(string pname: ping_queue[0..PINGS_AT_ONCE-1])
	    {
                debug_message("send_ping: Sending ping to %s", pname);
		send_udp(pname, ([ REQUEST: PING ]), 1);
	    }
	    ping_queue = ping_queue[PINGS_AT_ONCE..<1];
	    if(sizeof(ping_queue))
		call_out(#'send_ping, 2);
	}
    }
    else if(member(ping_queue, name)<0)
    {
        debug_message("send_ping: Scheduling ping to %s", name);
	ping_queue += ({name});
	if(sizeof(ping_queue)==1)
	    call_out(#'send_ping, 2);
    }
}

mixed *parse_host_line(mixed line, int log_errors)
{
    int stat;
    string *local_cmds;
    string name, encoding;

    if (extern_call() &&
	strstr(object_name(previous_object()),"/secure/udp/"))
	    return 0;

    if (line == "" || line[0] == '#')
	return 0;

    line = explode(line, "\t");
    if(sizeof(line)==1)
        line = explode(line[0], ":");

    if (sizeof(line) < 5 ||
	strlen(line[HL_HOST_IP]-"0123456789abcdef.:") ||
	strlen(line[HL_HOST_UDP_PORT]-"0123456789"))
    {
	if(log_errors)
	    log_message("*Parse error in hosts file: line '" + implode(line, "\t") + "'");
	return 0;
    }
    name = line[HL_HOST_NAME];

    local_cmds = explode(line[HL_LOCAL_COMMANDS], ",");
    if ("*" in local_cmds)
	local_cmds = local_cmds - ({ "*" }) + COMMANDS;

    if(sizeof(line)>HL_HOST_STATUS)
	stat = to_int(line[HL_HOST_STATUS]);
    else
	stat = UNKNOWN;

    if(sizeof(line)>HL_HOST_ENCODING)
        encoding = line[HL_HOST_ENCODING];
    else
        encoding = "ASCII";

    return ({
        capitalize(name),
        line[HL_HOST_IP],
        to_int(line[HL_HOST_UDP_PORT]),
        local_cmds,
        explode(line[HL_HOST_COMMANDS], ","),
        stat,
        encoding
    });
}

private void add_host(mixed *host)
{
    string name;
    mixed* cur;
    string addr, prev;

    if (!host)
        return;

    name = lower_case(host[HL_HOST_NAME]);
    cur = hosts[name];
    addr = host[HL_HOST_IP] + " " + host[HL_HOST_UDP_PORT];

    debug_message("add_host: %s -> %Q", name, host);

    if(cur)
    {
        host[HL_LOCAL_COMMANDS] = cur[HL_LOCAL_COMMANDS];
        host[HL_HOST_STATUS] = cur[HL_HOST_STATUS];
        m_delete(addrs, cur[HL_HOST_IP] + " " + cur[HL_HOST_UDP_PORT]);
    }

    prev = addrs[addr];
    if (prev && prev != name)
    {
        log_message("Name change:\n" + addr + ": " + hosts[prev][HL_HOST_NAME] + " -> " + host[HL_HOST_NAME] + "\n");
        m_delete(hosts, prev);
    }

    hosts[name] = host;
    addrs[addr] = name;
}

void update_encoding(string mudname, string encoding)
{
    mixed *host_data;

    if (extern_call() && strstr(object_name(previous_object()),"/secure/udp/"))
        return;

    if (catch(to_bytes("X", encoding)))
    {
        log_message("Illegal encoding for '" + mudname + "': '" + encoding + "'\n");
        return;
    }

    host_data = hosts[lower_case(mudname)];
    if (!host_data)
    {
        log_message("Got encoding for unknown mud '" + mudname + "'\n");
        return;
    }

    if (host_data[HL_HOST_ENCODING] != encoding)
    {
        host_data[HL_HOST_ENCODING] = encoding;
        log_message("Change encoding for '" + mudname + "' to '" + encoding + "'\n");
    }
}

void ping_host_if_unknown(mixed *host)
{
    string name;
    mixed *cur;

    if (!host)
        return;

    if (extern_call() && strstr(object_name(previous_object()),"/secure/udp/"))
        return;

    name = lower_case(host[HL_HOST_NAME]);
    cur = hosts[name];

    debug_message("ping_host_if_unknown: %s -> %Q", name, host);

    if(lower_case(name) == lower_case(MUD_NAME))
        return;

    if (!cur || cur[HL_HOST_IP] != host[HL_HOST_IP] || cur[HL_HOST_UDP_PORT] != host[HL_HOST_UDP_PORT])
        send_ping_addr(host[HL_HOST_IP], host[HL_HOST_UDP_PORT]);
    else if (name != lower_case(cur[HL_HOST_NAME]))
        send_ping(cur[HL_HOST_NAME]);
}

string read_host_file(string file)
{
    int limit, pos;
    bytes buf, add;
    
    limit = query_limits()[LIMIT_BYTE];
    if(!limit)
	return read_file(file);

    buf = to_bytes(({}));
    
    while(add = read_bytes(file, pos, limit))
    {
	buf += add;
	pos += limit;
    }
    
    return sizeof(buf) && to_text(buf, "UTF-8");
}

void set_host_list()
{
    mixed data;
    
    data = read_host_file(HOST_SAVE_FILE);
    if (!data)
	data = read_host_file(HOST_FILE);
    if (data)
	for(int i = sizeof(data = explode(data, "\n")); i--; )
	    add_host(parse_host_line(data[i], 1));
}

void save_host_file()
{
    hosts = filter(hosts,
	(: $2[HL_HOST_STATUS]>=0 ||
	    time()+$2[HL_HOST_STATUS]<DOWN_TIME_FOR_REMOVAL :));

    rm(HOST_SAVE_FILE);  

    foreach(string host: sort_array(m_indices(hosts),#'>))
    {
	mixed data = hosts[host];

	write_file(HOST_SAVE_FILE,
	    data[HL_HOST_NAME]+"\t"+
	    data[HL_HOST_IP]+"\t"+
	    data[HL_HOST_UDP_PORT]+"\t"+
	    implode(data[HL_LOCAL_COMMANDS],",")+"\t"+
	    implode(data[HL_HOST_COMMANDS],",")+"\t"+
	    data[HL_HOST_STATUS]+"\t"+
	    data[HL_HOST_ENCODING]+"\n");
    }
}

void reset()
{
    save_host_file();
}

int remove()
{
    save_host_file();
    destruct(this_object());
    return 1;
}

void prepare_renewal() {save_host_file();}
void finish_renewal(object neu) {}
void abort_renewal() {}

void startup(string * remaining = 0) {
    if (!remaining)
        remaining = m_indices(hosts);
    if (!sizeof(remaining))
        return;

    call_out(#'startup, 1, remaining[1..]);

    debug_message("startup: Sending lists request for %s.", remaining[0]);
    send_udp(remaining[0], ([ REQUEST: "query", DATA: "list" ]), 1);

    debug_message("startup: Sending hosts request for %s.", remaining[0]);
    send_udp(remaining[0], ([ REQUEST: "query", DATA: "hosts" ]), 1);
}

mixed decode(string arg) {
    if (sizeof(arg) && arg[0] == '$')
	return arg[1..];
    if (to_string(to_int(arg)) == arg)
	return to_int(arg);
    return arg;
}

mixed decode_packet(string packet) {
    string *data;
    mapping ret;
    string info, tmp;
    mixed class;
    int i;

    data = explode(packet, DELIMITER);
    /* If this packet has been split, handle buffering. */
    if (data[0][0..strlen(PACKET)] == PACKET + ":") {
	int id, n;

	if (sscanf(
	data[0][strlen(PACKET)+1..]
	, "%s:%d:%d/%d", class, id, i, n) != 4)
	    return 0;
	class = lower_case(class) + ":" + id;
	if (pointerp(incoming_packets[class])) {
	    incoming_packets[class][i-1] =
	    packet[strlen(data[0])+1..];
	    if (member(incoming_packets[class], 0) == -1) {
		ret = decode_packet(implode(incoming_packets[class], ""));
		incoming_packets = m_delete(incoming_packets, class);
		return ret;
	    }
	} else {
	    incoming_packets[class] = allocate(n);
	    incoming_packets[class][i-1] =
	    packet[strlen(data[0])+1..];
	    /* If no timeout is running then start one. */
	    if (!pending_data[class])
		call_out("incoming_time_out", REPLY_TIME_OUT, class);
	}
	return 1;
    }
    ret = ([ ]);
    for(i = 0; i < sizeof(data); i++) {
	if (sscanf(data[i], "%s:%s", tmp, info) != 2)
	    return 0;
	switch(({string})(class = decode(tmp))) {
	    case DATA:
		return ret + ([ DATA: decode(
		    implode(data[i..], DELIMITER)[strlen(DATA)+1..]
		    )
		]);
            case NAME:
                ret[NAME] = decode(info)[0..31];
                continue;
	    default:
		ret[class] = decode(info);
		continue;
	}
    }
    return ret;
}

private int valid_request(mapping data) {
    mixed *new_host_data;
    mixed host_data;
    string req;
    
    if (!data[NAME] || !data[UDP_PORT] || !(req = data[REQUEST])) {
	log_message("Illegal packet.");
	return 0;
    }
    if ((host_data = hosts[lower_case(data[NAME])]))
    {
	if (data[HOST] != host_data[HL_HOST_IP]) {
	    log_message("Host change:\n" +
	    host_data[HL_HOST_NAME] + ": " + host_data[HL_HOST_IP] + " -> " +
	    data[HOST] + "\n");
	    new_host_data = copy(host_data);
	    new_host_data[HL_HOST_IP] = data[HOST];
	}
	if (data[UDP_PORT] != host_data[HL_HOST_UDP_PORT]) {
	    if (data[UDP_PORT] > 65535 || data[UDP_PORT] <= 0)
	    {
	        log_message("Ignoring illegal port change:\n" +
	            host_data[HL_HOST_NAME] + " (" + host_data[HL_HOST_IP] + "): " +
	            host_data[HL_HOST_UDP_PORT] + " -> " + data[UDP_PORT] + "\n");
	    }
	    else
	    {
	        log_message("Port change:\n" +
	            host_data[HL_HOST_NAME] + " (" + host_data[HL_HOST_IP] + "): " +
	            host_data[HL_HOST_UDP_PORT] + " -> " + data[UDP_PORT] + "\n");
	        new_host_data ||= copy(host_data);
	        new_host_data[HL_HOST_UDP_PORT] = data[UDP_PORT];
	    }
	}
    } else {
	new_host_data = ({
	    data[NAME],
	    data[HOST],
	    data[UDP_PORT],
	    COMMANDS,
	    ({ "*" }),
	    UP,
	    "ASCII"
	});
	log_message("New mud.\n" + data[NAME] + ":" + data[HOST] + ":" + data[UDP_PORT] + "\n");
    }

    if (new_host_data)
    {
        add_host(new_host_data);
        debug_message("valid_request: Scheduling hosts request for %s.", data[NAME]);
        call_out(#'send_udp, random(60)+70, data[NAME], ([ REQUEST: "query", DATA: "hosts" ]), 1);
        call_out(#'send_udp, random(60)+10, data[NAME], ([ REQUEST: "query", DATA: "list" ]), 1);
    }

    if (req != PING && req != UDP_QUERY && req != REPLY &&
    member(host_data[HL_LOCAL_COMMANDS], data[REQUEST]) == -1) {
	/* This should probably send a system message too. */
	send_udp(host_data[HL_HOST_NAME], ([
	    REQUEST: REPLY,
	    RECIPIENT: data[SENDER],
	    ID: data[ID],
	    DATA: "Invalid request @" + LOCAL_NAME + ": " +
		capitalize(data[REQUEST]) + "\n"
	]) );
	log_message("Invalid request.");
	return 0;
    }
    return 1;
}

void receive_udp(string sender, string packet) {
    mixed data;
    string req;

#ifdef UDP_MASTER
    if (!previous_object() ||
    object_name(previous_object()) != UDP_MASTER) {
	log_message("Illegal call of receive_udp() by " +
	object_name(previous_object()) + "\n");
	return;
    }
#endif
    if (!mappingp(data = decode_packet(packet))) {
	if (!data)
	    log_message(
                sprintf("Received invalid packet.\nSender: %s\nPacket:\n%#Q\n",
                        sender, packet));
	return;
    }
    if(!strstr(sender,"::ffff:"))
        sender = sender[7..<1];

    data[HOST] = sender;
    if (!valid_request(data)) {
	log_message("Invalid Request:\nSender: " + sender + "\nPacket:\n" + packet + "\n");
	return;
    }
    hosts[lower_case(data[NAME])][HL_HOST_STATUS] = UP;
    req = data[REQUEST];
    if (req == REPLY) {
    	mapping pending;

	/* If we can't find the reply in the pending list then bin it. */
	if (!(pending = pending_data[lower_case(data[NAME]) + ":" + data[ID]]))
	    return;
	data[REQUEST] = pending[REQUEST];
	pending_data =
	m_delete(pending_data, lower_case(data[NAME]) + ":" + data[ID]);
    }
    call_other(UDP_CMD_DIR + req, "udp_" + req, copy(data));
}

string expand_mud(string mudname)
{
    string *names;

    mudname = lower_case(mudname);

    if (hosts[mudname])
	return mudname;

    names = filter(m_indices(hosts),
	(: !strstr($1, $2) :), mudname);

    if (sizeof(names) == 1)
	return names[0];

    names = filter(names,
	(: hosts[$1][HL_HOST_STATUS] > 0 :));

    if (sizeof(names) == 1)
	return names[0];

    return 0;
}

string encode(mixed arg) {
    if (objectp(arg))
	return object_name(arg);
    if (stringp(arg) && sizeof(arg) && (arg[0] == '$' ||
        to_string(to_int(arg)) == ({string})arg))
	return "$" + arg;
    return to_string(arg);
}

string encode_packet(mapping data) {
    int i;
    mixed indices;
    string header, body, ret;
    status data_flag;

    for(i = sizeof(indices = m_indices(data)); i--; ) {
	if (indices[i] == DATA) {
	    data_flag = 1;
	    continue;
	}
	header = encode(indices[i]);
	body = encode(data[indices[i]]);
	if (sscanf(header, "%~s" + DELIMITER + "%~s") ||
	sscanf(body, "%~s" + DELIMITER + "%~s"))
	    return 0;
	if (ret)
	    ret +=
	    DELIMITER + header + ":" + body;
	else
	    ret = header + ":" + body;
    }
    if (ret) {
	if (data_flag)
	    ret += DELIMITER + DATA + ":" + encode(data[DATA]);
	return ret;
    }
}

string *explode_packet(string packet, int len) {
    string* exploded = ({});
    int pos;

    for(pos = 0; pos < sizeof(packet); pos+=len)
	exploded += ({ packet[pos..pos+len-1] });

    return exploded;
}

varargs string send_udp(string mudname, mapping data, int expect_reply) {
    mixed host_data;
    string *packet_arr;
    string packet;
    int i;

    if (extern_call() && !adminp(this_interactive()) &&
	strstr(object_name(previous_object()),"/secure/udp/") &&
	strstr(object_name(previous_object()),"/room/serv/") &&
#ifdef UNItopia
        strstr(object_name(previous_object()),"/p/Item/Toy/obj/stoffdrache#") &&
#endif
	(playerp(previous_object()) ?
	 (lower_case(data[SENDER]) != ({string})previous_object()->query_real_name()) : 1
	)
       )
    {
      log_message("Illegaler Aufruf von send_udp()\n"
	"Objekt: "+ object_name(previous_object()) +
	(playerp(previous_object()) ?
	    "\""+({string})previous_object()->query_real_name()+"\"" : "") +
	(this_interactive() ?
	    "\nInteraktiv: "+ ({string})this_interactive()->query_real_name() : "") +
	"\n"
      );
      return "inetd: illegaler Aufruf!\n";
    }

    mudname = lower_case(mudname);
    if (!(host_data = hosts[mudname]))
    {
	string expanded = expand_mud(mudname);
	    
	if(expanded)
		host_data = hosts[mudname = expanded];
	else
	    return "Unknown mud: " + capitalize(mudname) + "\n";
    }
    if (data[REQUEST] != PING &&
    data[REQUEST] != UDP_QUERY &&
    data[REQUEST] != REPLY &&
    member(host_data[HL_HOST_COMMANDS],"*") == -1 &&
    member(host_data[HL_HOST_COMMANDS],data[REQUEST]) == -1)
	return capitalize(data[REQUEST]) + ": Command unavailable @" +
	host_data[HL_HOST_NAME] + "\n";
    data += ([ NAME: LOCAL_NAME, UDP_PORT: LOCAL_UDP_PORT ]);
    if (expect_reply) {
	/* Don't use zero. */
	data[ID] = ++packet_id;
	/* Don't need copy_mapping() as we are changing the mapping size. */
	pending_data[mudname + ":" + packet_id] =
	data + ([ NAME: host_data[HL_HOST_NAME] ]);
	call_out("reply_time_out", REPLY_TIME_OUT, mudname + ":" + packet_id);
    }
    if (!(packet = encode_packet(data))) {
	if (expect_reply)
	    pending_data = m_delete(pending_data, mudname + ":" + packet_id);
	log_message("Illegal packet sent by " + object_name(previous_object()) + "\n");
	return "inetd: Illegal packet.\n";
    }
    if (strlen(packet) <= MAX_PACKET_LEN)
	packet_arr = ({ packet });
    else {
	string header;
	int max;

	header = PACKET + ":" + lower_case(LOCAL_NAME) + ":" +
	(expect_reply ? packet_id : ++packet_id) + ":";
	packet_arr = explode_packet(packet,
	/* Allow 8 extra chars: 3 digits + "/" + 3 digits + DELIMITER */
	MAX_PACKET_LEN - (strlen(header) + 8));
	for(i = max = sizeof(packet_arr); i--; )
	    packet_arr[i] =
	    header + (i+1) + "/" + max + DELIMITER + packet_arr[i];
    }
    for(i = sizeof(packet_arr); i--; ) {
	if (!efun::send_udp(host_data[HL_HOST_IP],
		    host_data[HL_HOST_UDP_PORT], to_bytes(packet_arr[i], host_data[HL_HOST_ENCODING] + "//TRANSLIT")))
	    return "inetd: Error in sending packet.\n";
    }
    return 0;
}

void incoming_time_out(string id) {
    incoming_packets = m_delete(incoming_packets, id);
}

void reply_time_out(mixed id) {
    mapping data;

    if (data = pending_data[id]) {

	if (data[RETRY] < RETRIES) {
	    mapping send;

	    data[RETRY]++;
	    /* We must use a copy so the NAME field in pending_data[id]
	     * isn't corrupted by send_udp(). */
	    send = copy(data);
	    send = m_delete(send, RETRY);
	    call_out("reply_time_out", REPLY_TIME_OUT, id);
	    send_udp(data[NAME], send);
	    return;
	}
	data = m_delete(data, RETRY);
	call_other(UDP_CMD_DIR + REPLY, "udp_" + REPLY,
	data + ([ SYSTEM: TIME_OUT ]));
	/* It's just possible this was removed from the host list. */
	if (hosts[lower_case(data[NAME])])
	    hosts[lower_case(data[NAME])][HL_HOST_STATUS] = DOWN(hosts[lower_case(data[NAME])][HL_HOST_STATUS]);
	incoming_time_out(lower_case(data[NAME]) + ":" + id);
    }
    pending_data = m_delete(pending_data, id);
}

varargs mixed query(string what) {
    switch(what) {
	case "commands":
	    return COMMANDS;
	case "hosts":
	    return copy(hosts);
	case "pending":
	    return copy(pending_data);
	case "incoming":
	    return copy(incoming_packets);
    }
}

string known_mud(string mudname)
{
    string expanded = expand_mud(mudname);
    
    return expanded && hosts[expanded][HL_HOST_NAME];
}

string delete_host(string name)
{
    mixed *host_data;
    if(extern_call() && !adminp(this_interactive()))
        return 0;

    name = expand_mud(name);
    host_data = hosts[name];

    if(host_data)
    {
        send_ping(name);
        send_ping(0);
        m_delete(hosts, name);
        m_delete(addrs, host_data[HL_HOST_IP] + " " + host_data[HL_HOST_UDP_PORT]);
        return name;
    }

    return 0;
}

mixed* query_host(string mudname)
{
    return copy(hosts[lower_case(mudname)]);
}
