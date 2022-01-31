#include "AstNodes.hpp"
#include "BinaryReduction.hpp"
#include "DeltaDebugging.hpp"
#include "GeneralizedBinaryReduction.hpp"
#include "HierarchicalDeltaDebugging.hpp"
#include "Timer.hpp"
#include <cstdio>
#include <fstream>
#include <tree_sitter/api.h>
#include <string>


extern "C" TSLanguage *tree_sitter_c();
static const char *HELP_STR = ""
    "bric <file_name> <predicate_name> [algorithm]\n"
    "\n"
    "algorithms:\n"
    "-gbr       generalized binary reduction (default)\n"
    "-ddmin     delta debugging\n"
    "-hdd       hierarchical delta debugging\n"
    "-br        binary reduction\n";


static bool FileExists(std::string file_name) {
    FILE *file = fopen(file_name.c_str(), "r");
    if (file == NULL) {
        return false;
    }

    fclose(file);
    return true;
}

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

// Used only for debugging
static void WriteToFile2(Ast *node, std::ofstream &ofstream) {
    static size_t indent = 0;
    ofstream << std::string(indent, ' ');
    if (node->type == AST_TYPE_LEAF) {
        ofstream << "<Leaf ts_type=\"";
        ofstream << node->ts_type;
        // ofstream << "\" pre_value=\"";
        // ofstream << node->pre_value;
        ofstream << "\" value=\"";
        ofstream << node->value;
        ofstream << "\"/>\n";
    }
    else {
        ofstream << "<Branch ts_type=\"";
        ofstream << node->ts_type;
        ofstream << "\">\n";
        indent += 2;
        for (auto child : node->children) {
            WriteToFile2(child, ofstream);
        }

        indent -= 2;
        ofstream << std::string(indent, ' ');
        ofstream << "<Branch/>\n";
    }
}

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("%s", HELP_STR);
        return 0;
    }

    std::string file_name = argv[1];
    std::string test_name = argv[2];

    if (!FileExists(file_name)) {
        printf("could not find C file '%s'\n", file_name.c_str());
        return 0;
    }

    if (!FileExists(test_name)) {
        printf("could not find test file '%s'\n", test_name.c_str());
        return 0;
    }

#if defined(_WIN64)
    std::string run_test = test_name + " >nul 2>nul";
#elif defined(__linux__)
    std::string run_test = "./" + test_name + " >temp.txt 2>&1";
#else
    printf("unknown operating system\n");
    return 0;
#endif

    int return_code = system(run_test.c_str());
    if (return_code != 0) {
        printf("predicate returns %d\n", return_code);
        return 0;
    }

    Timer timer;
    timer.Start();

    std::string f_reduced = "reduced_" + file_name;
    std::string source_code = ReadFile(file_name);
    WriteFile(f_reduced, source_code);

    TSParser *parser = ts_parser_new();
    ts_parser_set_language(parser, tree_sitter_c());
    uint32_t source_code_size = static_cast<uint32_t>(source_code.size());
    TSTree *ts_tree = ts_parser_parse_string(parser, NULL, source_code.c_str(), source_code_size);
    TSNode ts_root_node = ts_tree_root_node(ts_tree);
    // GeneralizedBinaryReduction(ts_root_node, file_name.c_str(), run_test.c_str(), source_code.c_str());
    Ast *ast = AstInit(ts_root_node, source_code);
    // printf("%d\n", Count(ast));
    // std::ofstream ofstream("hdd.html");
    // WriteToFile2(ast, ofstream);
    // ofstream.close();

    std::vector<Ast *> function_definitions;
    FindNodes(ast, "function_definition", function_definitions);
    // DeltaDebugging(function_definitions, ast, file_name, run_test);
    // HierarchicalDeltaDebugging(ast, file_name, run_test);
    BinaryReduction(function_definitions, ast, file_name, run_test);
    timer.Stop();

    // std::string reduced_source_code = ReadFile(file_name);
    // WriteFile(f_reduced, reduced_source_code);
    // WriteFile(file_name, source_code);

    printf("Finished (%llums)\n", timer.ElapsedMilliseconds());
    return 0;
}
