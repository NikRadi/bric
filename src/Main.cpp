#include "AstNodes.hpp"
#include "BinaryReduction.hpp"
#include "DeltaDebugging.hpp"
#include "HierarchicalDeltaDebugging.hpp"
#include <fstream>
#include <streambuf>
#include <string>
#include <tree_sitter/api.h>


extern "C" TSLanguage *tree_sitter_c();


static std::string ReadFile(std::string file_name) {
    typedef std::istreambuf_iterator<char> istreambuf;
    std::ifstream ifstream(file_name);
    std::string file_content((istreambuf(ifstream)), istreambuf());
    ifstream.close();
    return file_content;
}

static void WriteFile(std::string file_name, std::string content) {
    std::ofstream ofstream(file_name);
    ofstream << content;
    ofstream.close();
}

int main() {
    system("setup_msvc.bat");
    const char *run_predicate_command = "Predicate.bat >NUL";
//    int return_code = system(run_predicate_command);
//    if (return_code != 0) {
//        printf("predicate returns %d\n", return_code);
//        return 0;
//    }

    const char *file_name = "Test.c";
    const char *reduced_file_name = "Reduced_Test.c";
    std::string source_code = ReadFile(file_name);

    TSParser *parser = ts_parser_new();
    ts_parser_set_language(parser, tree_sitter_c());
    TSTree *ts_tree = ts_parser_parse_string(parser, NULL, source_code.c_str(), static_cast<uint32_t>(source_code.size()));
    TSNode ts_root_node = ts_tree_root_node(ts_tree);

    Ast *root_node = AstInit(ts_root_node, source_code.c_str());
    std::vector<Ast *> function_defs;
    AstFindNodes(root_node, AST_TYPE_FUNCTION_DEF, function_defs);
//    DeltaDebugging(root_node, file_name, run_predicate_command, function_defs);

//    HierarchicalDeltaDebugging(root_node, file_name, run_predicate_command);

    BinaryReduction(root_node, file_name, run_predicate_command, function_defs);

    std::string reduced_source_code = ReadFile(file_name);
    WriteFile(file_name, source_code);
    WriteFile(reduced_file_name, reduced_source_code);
    printf("Finished\n");
    return 0;
}
