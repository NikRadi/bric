#include "TimePrinting.hpp"
#include <chrono>
#include <cstdarg>
#include <cstdio>


static std::chrono::steady_clock::time_point time_start;


void StartMeasuringTime() {
    time_start = std::chrono::steady_clock::now();
}

void PrintWithTime(const char *format, ...) {
    std::chrono::steady_clock::time_point time_now = std::chrono::steady_clock::now();
    va_list arg_list;
    va_start(arg_list, format);
    printf("%ldms: ", std::chrono::duration_cast<std::chrono::milliseconds>(time_now - time_start).count());
    vfprintf(stdout, format, arg_list);
    printf("\n");
    va_end(arg_list);
}
