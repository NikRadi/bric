#include "BinaryReduction.hpp"
#include "Ddmin.hpp"
#include "ErrorPrinting.hpp"
#include "FileHandling.hpp"
#include "Tree.hpp"
#include <string>
#include <tree_sitter/api.h>


enum AlgorithmType {
    ALGO_DDMIN, ALGO_BINARY_REDUCTION, ALGO_HDD
};

struct Args {
    bool print_help = false;
    AlgorithmType algorithm_type = ALGO_DDMIN;
    std::string c_file_name;
    std::string predicate_file_name;
};

extern "C" TSLanguage *tree_sitter_c();
const std::string HELP =
    "bric usage:\n"
    "bric c_file predicate [arguments]\n"
    "\n"
    "arguments:\n"
    "--ddmin    Use the ddmin algorithm to reduce. (Default)\n"
    "--br       Use the binary reduction algorithm to reduce.\n"
    "--hdd      Use the hierarchical delta debugging algorithm to reduce.\n"
    "\n";


static std::string ToStr(AlgorithmType type) {
    switch (type) {
        case ALGO_DDMIN:            return "ddmin";
        case ALGO_BINARY_REDUCTION: return "binary reduction";
        case ALGO_HDD:              return "hdd";
        default: ShowInternalErrorAndExit("ToStr(AlgorithmType) - %d", type);
    }

    return NULL; // Just to disable "no return statement" warning
}

static Args ParseArgs(int argc, char **argv) {
    Args args;
    if (argc < 3) {
        args.print_help = true;
        return args;
    }

    args.c_file_name = argv[1];
    args.predicate_file_name = argv[2];
    for (int i = 3; i < argc; ++i) {
        std::string arg = argv[i];
        printf("arg %s %d\n", arg.c_str(), arg == "--br");
        if      (arg == "--ddmin") args.algorithm_type = ALGO_DDMIN;
        else if (arg == "--br")    args.algorithm_type = ALGO_BINARY_REDUCTION;
        else if (arg == "--hdd")   args.algorithm_type = ALGO_HDD;
        else printf("unknown argument '%s'\n", arg.c_str());
    }

    return args;
}

int main(int argc, char **argv) {
    Args args = ParseArgs(argc, argv);
    if (args.print_help) {
        printf("%s\n", HELP.c_str());
        return 0;
    }

    printf("File to reduce:     %s\n", args.c_file_name.c_str());
    printf("Predicate:          %s\n", args.predicate_file_name.c_str());
    printf("%d\n", args.algorithm_type);
    printf("Algorithm:          %s\n", ToStr(args.algorithm_type).c_str());
    printf("\n");

    std::string run_predicate_command = "./" + args.predicate_file_name + "> /dev/null 2>&1";
    int return_code = system(run_predicate_command.c_str());
    if (return_code != 0) {
        printf("Predicate returns code %d. Make sure it returns 0.\n", return_code);
        return 0;
    }

    std::string c_file_content = ReadFile(args.c_file_name);
    std::string c_file_name_no_extension = args.c_file_name.substr(0, args.c_file_name.size() - 2);
    std::string reduced_c_file_name = c_file_name_no_extension + "_reduced.c";
    WriteAndReplaceFile(reduced_c_file_name, c_file_content);

    TSParser *parser = ts_parser_new();
    ts_parser_set_language(parser, tree_sitter_c());
    TSTree *ts_tree = ts_parser_parse_string(parser, NULL, c_file_content.c_str(), c_file_content.size());
    TSNode ts_root_node = ts_tree_root_node(ts_tree);

    Tree tree = TreeInit(ts_root_node, c_file_content);
    std::vector<Ast *> units;
    TreeFindNodes(tree, units, "function_definition");
    printf("Total %ld nodes\n", units.size());
    switch (args.algorithm_type) {
        case ALGO_DDMIN:            { Ddmin(tree, units, run_predicate_command, args.c_file_name); } break;
        case ALGO_BINARY_REDUCTION: { BinaryReduction(tree, units, run_predicate_command, args.c_file_name); } break;
        case ALGO_HDD:              { ShowWarning("not implemented"); } break;
        default:                    { printf("Unknown algorithm option\n"); } break;
    }

    SwitchFileContents(args.c_file_name, reduced_c_file_name);
    printf("Done. Result in '%s'\n", reduced_c_file_name.c_str());
    TreeDelete(tree);
    return 0;
}
