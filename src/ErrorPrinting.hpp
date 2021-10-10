#ifndef BRIC_ERROR_PRINTING_HPP
#define BRIC_ERROR_PRINTING_HPP


void ShowFatalErrorAndExit(const char *format, ...);

void ShowInternalErrorAndExit(const char *format, ...);

void ShowWarning(const char *format, ...);

#endif // BRIC_ERROR_PRINTING_HPP
