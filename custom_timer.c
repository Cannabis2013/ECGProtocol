#include "custom_timer.h"

void start_timer()
{
    struct timeval _tVal;
    gettimeofday(&_tVal,0);

    _timestamp_sec = _tVal.tv_sec;
    _timestamp_usec = _tVal.tv_usec;
}

long long time_elapsed()
{
    long long _elapsed;
    long long _current_time_sec,
            _current_time_usec;

    struct timeval _tVal;
    gettimeofday(&_tVal,0);

    _current_time_sec = _tVal.tv_sec;
    _current_time_usec = _tVal.tv_usec;

    _elapsed = 1000*(_current_time_sec - _timestamp_sec) + (long long) round((_current_time_usec - _timestamp_usec)/1000);
    return _elapsed;
}

void block(int ms)
{
    struct timeval _tVal;
    gettimeofday(&_tVal,0);

    uint sec = _tVal.tv_sec;
    uint usec = _tVal.tv_usec;

    int times_up = 0;

    while (!times_up) {
        gettimeofday(&_tVal,0);
        long long elapsed = 1000*(_tVal.tv_sec - sec) + (long long) round((_tVal.tv_usec - usec)/1000);
        times_up = elapsed > ms ? 1 : 0;
    }
}
