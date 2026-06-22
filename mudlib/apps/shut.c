// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /apps/shut.c
// Description: Regelmäßige Armageddons
// Author:      Gnomi

#include <time.h>

#define ARMA  "/room/rathaus/div/shut"

// Zum nächsten ersten um 20 Uhr.
private int next_first(int date)
{
    int* next = timearray(date);
    next[TM_SEC] = next[TM_MIN] = next[TM_WDAY] = next[TM_YDAY] = 0;
    next[TM_HOUR] = 20;
    next[TM_MDAY] = 1;
    next[TM_MON]++; // Wer werden schon nicht im Dezember aufgerufen.

    return array_to_time(next);
}

private void start()
{

    ARMA.shut((next_first(time()) - time()) / 60);
}

int query_next_shutdown()
{
    int next = find_call_out(#'start);
    if (next < 0)
        return 0;
    return next_first(time() + next);
}

void create()
{
    // Wir berechnen die Startzeit (ein Monat vor dem Reboot):
    int* next = timearray(time());
    next[TM_SEC] = next[TM_MIN] = next[TM_HOUR] = next[TM_WDAY] = next[TM_YDAY] = 0;
    next[TM_MDAY] = 1;
    if (next[TM_MON] < 2)
        next[TM_MON] = 2;
    else if (next[TM_MON] < 8)
        next[TM_MON] = 8;
    else
    {
        next[TM_MON] = 2;
        next[TM_YEAR]++;
    }

    // Wenn das letzte Arma weniger als 4 Monate vor dieser Startzeit liegt,
    // dem Start um 6 Monate verschieben.
    int *boot = timearray(__BOOT_TIME__);
    if(next[TM_MON] + next[TM_YEAR]*12 - boot[TM_MON] - boot[TM_YEAR] * 12 < 5)
    {
        if (next[TM_MON] < 7)
            next[TM_MON]+=6;
        else
        {
            next[TM_MON] -= 6;
            next[TM_YEAR]++;
        }
    }

    // Wir legen den Countdown auf den 15., damit nur zwei Wochen lang gebrüllt wird.
    call_out(#'start, array_to_time(next) - time() + 14*DAY);
}
