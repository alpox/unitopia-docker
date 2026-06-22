// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /sys/getopt.h
// Description: Defines fuer /i/tools/getopt::getopt()
// Author:      Garthan (13.02.94)

#ifndef GETOPT_H
#define GETOPT_H 1

#define GO_ATOM_ERRS            0x01
#define GO_ATOM_FAILS           0x02
#define GO_ATOM_SLOPPY          0x04
#define GO_ATOM_OPTIONS_FIRST   0x08

#define GO_ERRS             GO_ATOM_ERRS
#define GO_FAILS            (GO_ATOM_ERRS|GO_ATOM_FAILS)
#define GO_SLOPPY           GO_ATOM_SLOPPY
#define GO_OPTIONS_FIRST    GO_ATOM_OPTIONS_FIRST

#endif // GETOPT_H
