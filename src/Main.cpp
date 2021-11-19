#include "AstNodes.hpp"
#include "BinaryReduction.hpp"
#include "DeltaDebugging.hpp"
#include "GeneralizedBinaryReduction.hpp"
#include "HierarchicalDeltaDebugging.hpp"
#include <cassert>
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
//    FILE *file = fopen(file_name.c_str(), "r");
//    assert(file != NULL);
//    fseek(file, 0, SEEK_END);
//    size_t file_size =  static_cast<size_t>(ftell(file));
//    fseek(file, 0, SEEK_SET);
//    char *file_content = new char[file_size];
//    fread(file_content, sizeof(char), file_size, file);
//    fclose(file);
//    return file_content;
}

static void WriteFile(std::string file_name, std::string content) {
    std::ofstream ofstream(file_name);
    ofstream << content;
    ofstream.close();
}

//static void PrintXml(TSNode node, const char *source_code) {
//    static int indent = 0;
//    for (int i = 0; i < indent; ++i) printf(" ");
//    const char *node_type = ts_node_type(node);
//    uint32_t num_children = ts_node_child_count(node);
//    if (num_children == 0) {
//        uint32_t start_byte = ts_node_start_byte(node);
//        uint32_t end_byte = ts_node_end_byte(node);
//        uint32_t num_bytes = end_byte - start_byte;
//        char *node_value = new char[num_bytes + 1];
//        node_value[num_bytes] = '\0';
//        strncpy(node_value, source_code + start_byte, num_bytes);
//        printf("<%s value=\"%s\"/>\n", node_type, node_value);
//    }
//    else {
//        printf("<%s>\n", node_type);
//        indent += 4;
//        for (uint32_t i = 0; i < num_children; ++i) {
//            TSNode child = ts_node_child(node, i);
//            PrintXml(child, source_code);
//        }
//
//        indent -= 4;
//        for (int i = 0; i < indent; ++i) printf(" ");
//        printf("<%s/>\n", node_type);
//    }
//}

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
//    PrintXml(ts_root_node, source_code.c_str());

//    Ast *root_node = AstInit(ts_root_node, source_code.c_str());
//    AlgorithmParams params;
//    params.root_node = root_node;
//    params.file_name = file_name;
//    params.run_predicate_command = run_predicate_command;

//    std::vector<Ast *> function_defs;
//    AstFindNodes(root_node, AST_TYPE_FUNCTION_DEF, function_defs);
//    DeltaDebugging(params, function_defs);

//    HierarchicalDeltaDebugging(params);

//    BinaryReduction(params, function_defs);

    GeneralizedBinaryReduction(ts_root_node, file_name, run_predicate_command, source_code.c_str());

    std::string reduced_source_code = ReadFile(file_name);
    WriteFile(file_name, source_code);
    WriteFile(reduced_file_name, reduced_source_code);
    printf("Finished\n");
    return 0;
}
