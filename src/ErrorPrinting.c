#include "ErrorPrinting.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h> // exit


void ShowFatalErrorAndExit(char *format, ...) {
    fprintf(stderr, "bric fatal error: ");
    va_list arg_list;
    va_start(arg_list, format);
    vfprintf(stderr, format, arg_list);
    fprintf(stderr, "\n");
    va_end(arg_list);
    exit(1);
}

void ShowWarning(char *format, ...) {
    fprintf(stdout, "bric warning: ");
    va_list arg_list;
    va_start(arg_list, format);
    vfprintf(stdout, format, arg_list);
    va_end(arg_list);
}
