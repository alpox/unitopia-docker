// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/apps/ip_filter
// Description:	Regelt Zugang von Spielern aus bestimmten Domains
// Author:	Freaky (12.01.96)

#include <config.h>
#include <error.h>

#define IP_FILTER_FILE "/static/adm/IP_FILTER"

struct entry
{
    int allow;           /* Falls != 0, ist ist dies eine Erlaubnis.    */
    int guests_only;     /* Falls != 0, so sind nur Gaeste verboten.    */
    string *allow_names; /* Spielernamen, die erlaubt sind.             */
    string msg;          /* Auszugebende (oder loggende) Meldung.       */
};

/* IP-Bereich -> Entry
 *
 * Der IP-Bereich wird dargestellt als String: "IP-Addr/Mask"
 * Mask ist dabei die Anzahl der gültigen Bits (als Dezimalzahl)
 * und IP-Addr ist die IP-Addresse (IPv4 oder 6) mit Hexadezimalzahlen
 * ohne (Doppel-)Punkt oder Abkürzungen.
 */
private static mapping filters = ([:1]);

/* Alle in den Filters genutzte Bitmasks (sortiert). */
private static int *masks = ({});

string gen_address(int *ip, int width)
{
    return implode(map(ip, function string(int i) { return sprintf("%0*x", width, i); }), "");
}

string parse_address(string addr)
{
    int *ip;

    addr = trim(addr);

    // Versuchen wir mal zuerst IPv4:
    ip = ({0}) * 4;
    if (sscanf(addr, "%d.%d.%d.%d%~.1s", ip[0], ip[1], ip[2], ip[3]) == 4 ||
        sscanf(addr, "::ffff:%d.%d.%d.%d%~.1s", ip[0], ip[1], ip[2], ip[3]) == 4)
        return gen_address(ip, 2);

    // Nun also IPv6.
    if (!sizeof(addr - "0123456789abcdef:"))
    {
        string *blocks = explode(addr, ":");
        int pos = 0;

        if (sizeof(blocks & ({""})) > 1)
        {
            // Sonderbehandlung von "::" am Anfang oder Ende
            if (blocks[<1] == "" && sizeof(blocks[..<2] & ({""})) == 1)
                blocks = blocks[..<2];
            else if (blocks[0] == "" && sizeof(blocks[1..] & ({""})) == 1)
                blocks = blocks[1..];
            else if (!sizeof(blocks - ({""})))
                blocks = ({""});
            else
                return 0;
        }

        ip = ({0}) * 8;
        foreach (string block: blocks)
        {
            if (block == "")
                pos += sizeof(ip) - sizeof(blocks) + 1;
            else if (sizeof(block) > 4)
                return 0;
            else
                ip[pos++] = to_int("0x"+block);
        }

        if (pos != 8)
            return 0;

        return gen_address(ip, 4);
    }

    return 0;
}

string gen_mask(string addr, int bits)
{
    int pos = sizeof(addr)-1;

    if (bits < 0 || bits > 4*sizeof(addr))
        bits = 4*sizeof(addr);

    for (int left = 4*sizeof(addr) - bits; left;)
    {
        if (left < 4)
        {
            int mask = ({0, 1, 3, 7})[left];
            addr[pos..pos] = sprintf("%x", to_int("0x" + addr[pos..pos]) & ~mask);
            break;
        }
        else
        {
            addr[pos--] = '0';
            left -= 4;
        }
    }

    return sprintf("%s/%d", addr, bits);
}

void create()
{
    string file;
    int line_number;
    mapping mask_set = ([:0]);

    file=read_file(IP_FILTER_FILE);
    if (!file)
        return;

    foreach (string line: explode(file,"\n"))
    {
        string ips, names, msg;
        struct entry e;

        line_number++;

        if (!sizeof(line) || line[0] == '#')
            continue;

        if (sscanf(line,"%s;%s;%s",ips,names,msg) != 3)
        {
            printf("Fehler in '%s' in Zeile %d!\n", IP_FILTER_FILE, line_number);
            continue;
        }

        e = (<entry> msg: msg, allow_names: ({}));
        if (names == "*")
            e.allow = 1;
        else if (names == "!gast")
            e.guests_only = 1;
        else
            e.allow_names = explode(names,",");

        foreach (string ip: explode(ips,","))
        {
            int mask;
            string addr;

            if (sscanf(ip, "%s/%d", ip, mask) < 2)
                mask = -1;

            addr = parse_address(ip);
            if (!addr)
            {
                printf("Ungültige Addresse '%s' in '%s' in Zeile %d!\n", ip, IP_FILTER_FILE, line_number);
                continue;
            }

            addr = gen_mask(addr, &mask);
            if (addr in filters)
                printf("Doppelter Eintrag für '%s' in '%s' in Zeile %d!\n", addr, IP_FILTER_FILE, line_number);

            m_add(filters, addr, e);
            m_add(mask_set, mask);
        }
    }

    masks = sort_array(m_indices(mask_set), #'<);
}

string ip_filter(string ip, string name)
{
    string addr = parse_address(ip);
    if (!addr)
    {
        do_warning(sprintf("Ungültige Adresse '%s' erhalten.\n", ip));
        return 0;
    }

    foreach (int mask: masks)
    {
        string key = gen_mask(addr, mask);
        struct entry e = filters[key];
        if (!e)
            continue;

        if (e.allow)
            return 0;
        if (e.guests_only && name != "gast" && !(name in GUEST_NAMES))
            return 0;
        if (name in e.allow_names)
            return 0;
        return e.msg;
    }
}

#if 0
// schlecht, da im Mapping ein Array ist, das nicht kopiert wird ;(
// debug only
mapping query_filters()
{
   return deep_copy(filters);
}
mapping query_subnets()
{
   return deep_copy(subnets);
}
mapping query_ipmasken()
{
   return deep_copy(ipmasken);
}
#endif
