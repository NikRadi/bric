#include "BinaryReduction.hpp"
#include "Ddmin.hpp"
#include "Hdd.hpp"
#include "TimePrinting.hpp"
#include "Tree.hpp"
#include <fstream>
#include <streambuf>
#include <string>
#include <tree_sitter/api.h>


extern "C" TSLanguage *tree_sitter_c();


std::string ReadFile(std::string file_name) {
    typedef std::istreambuf_iterator<char> istreambuf;
    std::ifstream ifstream(file_name);
    std::string file_content((istreambuf(ifstream)), istreambuf());
    ifstream.close();
    return file_content;
}

void WriteFile(std::string file_name, std::string content) {
    std::ofstream ofstream(file_name);
    ofstream << content;
    ofstream.close();
}

int main(int argc, char **argv) {
    StartMeasuringTime();
    PrintWithTime("parsing args");
    std::string c_file_name = "Test.c";
    std::string predicate_file_name = "Predicate.sh";

    std::string reduced_c_file_name = "reduced_" + c_file_name;
    std::string run_predicate = "./" + predicate_file_name + " > /dev/null 2>&1";
    int return_code = system(run_predicate.c_str());
    if (return_code != 0) {
        printf("Predicate returns code %d\n", return_code);
        return 0;
    }

    std::string source_code = ReadFile(c_file_name);
    WriteFile(reduced_c_file_name, source_code);

    PrintWithTime("parsing '%s'", c_file_name.c_str());
    TSParser *parser = ts_parser_new();
    ts_parser_set_language(parser, tree_sitter_c());
    TSTree *ts_tree = ts_parser_parse_string(parser, NULL, source_code.c_str(), source_code.size());
    TSNode ts_root_node = ts_tree_root_node(ts_tree);

    Tree *tree = TreeInit(ts_root_node, source_code);
    AlgorithmParams params;
    params.c_file_name = c_file_name;
    params.run_predicate = run_predicate;
    params.tree = tree;

    std::vector<Ast **> units;
    TreeFindUnits(tree, units, "if_statement");
    std::vector<Unit> removed_units;

//    Ddmin(params, units, removed_units);
    BinaryReduction(params, units, removed_units);
//    Hdd(params);
//    HddWithBinaryReduction(params);
    Enable(removed_units);

    std::string reduced_source_code = ReadFile(c_file_name);
    WriteFile(reduced_c_file_name, reduced_source_code);
    WriteFile(c_file_name, source_code);
    PrintWithTime("reduced source code in '%s'\n", reduced_c_file_name.c_str());

    TreeDelete(tree);
    return 0;
}
