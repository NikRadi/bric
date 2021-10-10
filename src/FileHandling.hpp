#ifndef BRIC_FILE_HANDLING_HPP
#define BRIC_FILE_HANDLING_HPP
#include <stdio.h>
#include <string>


struct File {
    FILE *stream;
    std::string name;
};


File OpenFile(std::string file_name, std::string mode);

void CloseFile(File file);

std::string ReadFile(std::string file_name);

void WriteAndReplaceFile(std::string file_name, std::string text);

void SwitchFileContents(std::string file1_name, std::string file2_name);


#endif // BRIC_FILE_HANDLING_HPP
