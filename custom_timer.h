#ifndef CUSTOM_TIMER_H
#define CUSTOM_TIMER_H


#include <inttypes.h>
#include <stdio.h>
#include <sys/time.h>
#include <math.h>

static long long _timestamp_sec;
static long long _timestamp_usec;

void start_timer(void);
long long time_elapsed(void);
void stop_timer(void);
void block(int ms);

#endif // CUSTOM_TIMER_H
