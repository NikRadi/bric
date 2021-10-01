#ifndef BRIC_FILEHANDLING_H
#define BRIC_FILEHANDLING_H
#include <stdio.h>


struct File {
    FILE *stream;
    char *name;
} typedef File;


File OpenFile(char *file_name, char *mode);

void CloseFile(File file);

char *ReadFile(char *file_name);

void WriteAndReplaceFile(char *file_name, char *text);

void SwitchFileContents(char *file1, char *file2);

#endif // BRIC_FILEHANDLING_H
