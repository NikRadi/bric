#include "ErrorPrinting.hpp"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h> // exit


static void ShowError(FILE *stream, const char *error_type, const char *format, va_list arg_list) {
    fprintf(stream, "bric %s: ", error_type);
    fprintf(stream, format, arg_list);
    fprintf(stream, "\n");
}

void ShowFatalErrorAndExit(const char *format, ...) {
    va_list arg_list;
    va_start(arg_list, format);
    ShowError(stderr, "fatal error", format, arg_list);
    va_end(arg_list);
    exit(1);
}

void ShowInternalErrorAndExit(const char *format, ...) {
    va_list arg_list;
    va_start(arg_list, format);
    ShowError(stderr, "internal error", format, arg_list);
    va_end(arg_list);
    exit(1);
}

void ShowWarning(const char *format, ...) {
    va_list arg_list;
    va_start(arg_list, format);
    ShowError(stdout, "warning", format, arg_list);
    va_end(arg_list);
}
