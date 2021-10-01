#include "TimeMeasure.h"
#include <stdarg.h>
#include <stdio.h>
#include <sys/time.h>


static struct timeval time_start;


void StartMeasuringTime() {
    gettimeofday(&time_start, NULL);
}

void PrintSecondsElapsed(char *format, ...) {
    struct timeval time_stop;
    gettimeofday(&time_stop, NULL);
    struct timeval time_taken;
    timersub(&time_stop, &time_start, &time_taken);
    printf("%ld.%06lds: ", (long int) time_taken.tv_sec, (long int) time_taken.tv_usec);

    va_list arg_list;
    va_start(arg_list, format);
    vfprintf(stdout, format, arg_list);
    va_end(arg_list);

    printf("\n");
}
