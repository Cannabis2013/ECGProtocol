#ifndef CUSTOM_TIMER_H
#define CUSTOM_TIMER_H


#include <inttypes.h>
#include <stdio.h>
#include <sys/time.h>
#include <math.h>
#include <stdlib.h>



typedef struct
{
    long long _timestamp_sec;
    long long _timestamp_usec;

}TIMER_IN;

void start_timer(TIMER_IN *t_in);
unsigned long long time_elapsed(TIMER_IN *t_in);
void stop_timer(void);
void block(int ms);

#endif // CUSTOM_TIMER_H
