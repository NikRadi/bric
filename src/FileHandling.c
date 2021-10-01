#include "FileHandling.h"
#include "ErrorPrinting.h"
#include <stdlib.h> // malloc, free


File OpenFile(char *file_name, char *mode) {
    FILE *file = fopen(file_name, mode);
    if (file == NULL) {
        ShowFatalErrorAndExit("could not open file '%s'", file_name);
    }

    File f;
    f.stream = file;
    f.name = file_name;
    return f;
}

void CloseFile(File file) {
    int return_code = fclose(file.stream);
    if (return_code != 0) {
        ShowWarning("could not close file '%s'", file.name);
    }
}

char *ReadFile(char *file_name) {
    File file = OpenFile(file_name, "r");
    fseek(file.stream, 0, SEEK_END);
    int file_len = ftell(file.stream);
    char *file_text = (char *) malloc(file_len * sizeof(char));
    rewind(file.stream);
    fread((void *) file_text, sizeof(char), file_len, file.stream);
    CloseFile(file);
    return file_text;
}

void WriteAndReplaceFile(char *file_name, char *text) {
    File file = OpenFile(file_name, "w");
    fputs(text, file.stream);
    CloseFile(file);
}

void SwitchFileContents(char *file1, char *file2) {
    char *file1_content = ReadFile(file1);
    char *file2_content = ReadFile(file2);
    WriteAndReplaceFile(file1, file2_content);
    WriteAndReplaceFile(file2, file1_content);
    free(file1_content);
    free(file2_content);
}
