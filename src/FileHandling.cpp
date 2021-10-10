#include "FileHandling.hpp"
#include "ErrorPrinting.hpp"
#include <fstream>
#include <iostream>
#include <stdlib.h> // malloc, free
#include <sstream>


File OpenFile(std::string file_name, std::string mode) {
    FILE *stream = fopen(file_name.c_str(), mode.c_str());
    if (stream == NULL) {
        ShowFatalErrorAndExit("could not open file '%s'", file_name.c_str());
    }

    File file;
    file.stream = stream;
    file.name = file_name;
    return file;
}

void CloseFile(File file) {
    int return_code = fclose(file.stream);
    if (return_code != 0) {
        ShowWarning("could not close file '%s'", file.name.c_str());
    }
}

std::string ReadFile(std::string file_name) {
    std::ifstream stream;
    stream.open(file_name);
    std::stringstream buffer;
    buffer << stream.rdbuf();
    return buffer.str();
}

void WriteAndReplaceFile(std::string file_name, std::string text) {
    File file = OpenFile(file_name, "w");
    fputs(text.c_str(), file.stream);
    CloseFile(file);
}

void SwitchFileContents(std::string file1_name, std::string file2_name) {
    std::string file1_content = ReadFile(file1_name);
    std::string file2_content = ReadFile(file2_name);
    WriteAndReplaceFile(file1_name, file2_content);
    WriteAndReplaceFile(file2_name, file1_content);
}
