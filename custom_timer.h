#ifndef CUSTOM_TIMER_H
#define CUSTOM_TIMER_H


#include <inttypes.h>
#include <stdio.h>
#include <sys/time.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>



typedef struct
{
    clock_t clocks;

}TIMER_IN;

void start_timer(TIMER_IN *t_in);
long time_elapsed(TIMER_IN *t_in);
void stop_timer(void);

long toMs(clock_t clocks);
void block(int ms);

#endif // CUSTOM_TIMER_H
