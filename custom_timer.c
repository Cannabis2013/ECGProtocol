#include "custom_timer.h"

void start_timer(TIMER_IN *t_in)
{
    t_in->clocks = clock();
}

long  time_elapsed(TIMER_IN *t_in)
{
    clock_t clocks = clock();

    return 1000*(clocks - t_in->clocks)/CLOCKS_PER_SEC;
}

void block(int ms)
{
    clock_t start = clock();

    while ((1000*(clock() - start)/CLOCKS_PER_SEC) <= ms) {
    }
}


long toMs(clock_t clocks)
{
    return 1000*clocks/CLOCKS_PER_SEC;
}
