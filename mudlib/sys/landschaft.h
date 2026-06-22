// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /sys/landschaft.h
// Description: Konstanten fuer den Raumtyp Landschaft
// Author:      Sissi (1999)

#ifndef LANDSCHAFT_H
#define LANDSCHAFT_H 1

#define LANDSCHAFT "landschaft"

// Fuer /i/tools/lpc_parser Sonderbehandlung
#ifdef __LPC_PARSER__
#define L_SET(x) this_object()->add_type(LANDSCHAFT,x)
#else
#define L_SET(x) add_type(LANDSCHAFT,x)
#endif


#define L_DRINNEN       0x000001

#define L_WASSER        0x000002
#define L_FLIESSEND     0x000004
#define L_FLACH         0x000008
#define L_UNTERWASSER   0x000010

#define L_STEG          0x000020
#define L_STRAND        0x000040

#define L_WIESE         0x000080
#define L_WALD          0x000100
#define L_WUESTE        0x000200
#define L_STEPPE        0x000400
#define L_BERG          0x000800
#define L_MOOR          0x001000
#define L_SUMPF         0x002000
#define L_EIS           0x004000
#define L_DSCHUNGEL     0x008000

#define L_WEG           0x010000
#define L_SIEDLUNG      0x020000
#define L_HAUS          0x040000
#define L_KANALISATION	0x080000

#define L_LUFT          0x100000
#define L_MEERESGRUND   0x200000

#define L_ACKER         0x400000
#define L_HUEGEL        0x800000
#define L_LAVA         0x1000000

#endif // LANDSCHAFT_H
