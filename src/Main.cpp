#include "AstNodes.hpp"
#include "BinaryReduction.hpp"
#include "DeltaDebugging.hpp"
#include "GeneralizedBinaryReduction.hpp"
#include "HierarchicalDeltaDebugging.hpp"
#include <cassert>
#include <fstream>
#include <streambuf>
#include <string>
#include "Timer.hpp"
#include <tree_sitter/api.h>


extern "C" TSLanguage *tree_sitter_c();
static const char *HELP_STR = ""
    "bric <file_name> <predicate_name> [options]\n"
    "\n"
    "options:\n"
    "-gbr       generalized binary reduction (default)\n"
    "-ddmin     delta debugging\n"
    "-hdd       hierarchical delta debugging\n"
    "-br        binary reduction\n";


enum AlgorithmType {
    ALGO_DELTA_DEBUGGING,
    ALGO_HIERARCHICAL_DELTA_DEBUGGING,
    ALGO_BINARY_REDUCTION,
    ALGO_GENERALIZED_BINARY_REDUCTION,
};


static const char *ToString(AlgorithmType type) {
    switch (type) {
        case ALGO_DELTA_DEBUGGING:              return "delta debugging";
        case ALGO_HIERARCHICAL_DELTA_DEBUGGING: return "hierarchical delta debugging";
        case ALGO_BINARY_REDUCTION:             return "binary reduction";
        case ALGO_GENERALIZED_BINARY_REDUCTION: return "generalized binary reduction";
    }
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

int main(int argc, char **argv) {
    Timer timer;
    timer.Start();
    if (argc < 3) {
        printf("%s", HELP_STR);
        return 0;
    }

    std::string file_name = argv[1];
    std::string predicate_name = argv[2];
    AlgorithmType algorithm_type = ALGO_GENERALIZED_BINARY_REDUCTION;
    if (argc == 4) {
        const char *algo = argv[3];
        if (strcmp(algo, "-ddmin") == 0) {
            algorithm_type = ALGO_DELTA_DEBUGGING;
        }
        else if (strcmp(algo, "-hdd") == 0) {
            algorithm_type = ALGO_HIERARCHICAL_DELTA_DEBUGGING;
        }
        else if (strcmp(algo, "-br") == 0) {
            algorithm_type = ALGO_BINARY_REDUCTION;
        }
        else if (strcmp(algo, "-gbr") == 0) {
            algorithm_type = ALGO_GENERALIZED_BINARY_REDUCTION;
        }
        else {
            printf("unknown option: %s\n", algo);
        }
    }

    printf("file name: %s\n", file_name.c_str());
    printf("predicate: %s\n", predicate_name.c_str());
    printf("algorithm: %s\n", ToString(algorithm_type));

    std::string run_predicate_command = predicate_name + " >NUL";
    int return_code = system(run_predicate_command.c_str());
    if (return_code != 0) {
        printf("predicate returns %d\n", return_code);
        return 0;
    }

    std::string reduced_file_name = "Reduced_" + file_name;
    std::string source_code = ReadFile(file_name);

    TSParser *parser = ts_parser_new();
    ts_parser_set_language(parser, tree_sitter_c());
    TSTree *ts_tree = ts_parser_parse_string(parser, NULL, source_code.c_str(), static_cast<uint32_t>(source_code.size()));
    TSNode ts_root_node = ts_tree_root_node(ts_tree);
//    PrintXml(ts_root_node, source_code.c_str());
    if (algorithm_type == ALGO_GENERALIZED_BINARY_REDUCTION) {
        GeneralizedBinaryReduction(ts_root_node, file_name.c_str(), run_predicate_command.c_str(), source_code.c_str());
    }
    else {
        Ast *root_node = AstInit(ts_root_node, source_code.c_str());
        AlgorithmParams params;
        params.root_node = root_node;
        params.file_name = file_name.c_str();
        params.run_predicate_command = run_predicate_command.c_str();

        std::vector<Ast *> function_defs;
        AstFindNodes(root_node, AST_TYPE_FUNCTION_DEF, function_defs);
        if (algorithm_type == ALGO_DELTA_DEBUGGING) {
            DeltaDebugging(params, function_defs);
        }
        else if (algorithm_type == ALGO_HIERARCHICAL_DELTA_DEBUGGING) {
            HierarchicalDeltaDebugging(params);
        }
        else if (algorithm_type == ALGO_BINARY_REDUCTION) {
            BinaryReduction(params, function_defs);
        }
    }

    std::string reduced_source_code = ReadFile(file_name);
    WriteFile(file_name, source_code);
    WriteFile(reduced_file_name, reduced_source_code);

    timer.Stop();
    printf("\n");
    printf("Finished in %llums\n", timer.ElapsedMilliseconds());
    printf("Reduced file name: %s\n", reduced_file_name.c_str());
    return 0;
}
