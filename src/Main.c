#include <tree_sitter/api.h>
#include <string.h> // strndup
#include "AstNode.h"
#include "Ddmin.h"
#include "FileHandling.h"
#include "List.h"
#include "StringManipulation.h"
#include "TimeMeasure.h"


TSLanguage *tree_sitter_c();
static const char *USAGE_STR =
    "bric usage:\n"
    "bric predicate_file file_to_reduce\n";


int main(int argc, char **argv) {
    StartMeasuringTime();
    if (argc != 3) {
        printf("%s\n", USAGE_STR);
        return 0;
    }

    char *predicate_file_name = argv[1];
    char *run_predicate_command = AddPrefix("./", predicate_file_name);

    char *c_file_name = argv[2];
    char *c_file_text = ReadFile(c_file_name);
    char *c_file_name_without_extension = strndup(c_file_name, strlen(c_file_name) - 2); // Substract 2 for ".c"
    char *reduced_c_file_name = AddPostfix(c_file_name_without_extension, "_reduced.c");
    WriteAndReplaceFile(reduced_c_file_name, c_file_text);

    TSParser *parser = ts_parser_new();
    ts_parser_set_language(parser, tree_sitter_c());
    TSTree *tree = ts_parser_parse_string(parser, NULL, c_file_text, strlen(c_file_text));
    TSNode ts_root_node = ts_tree_root_node(tree);
    PrintSecondsElapsed("Finished parsing '%s'", c_file_name);

    //
    // TEST START
    //

    AstNode *root_node = AstInit(ts_root_node, c_file_text);
    List units = ListInit();
    AstFindAllNodesOfType(root_node, &units, "expression_statement");
    PrintSecondsElapsed("Finished analyzing");
    Ddmin(&units, run_predicate_command, root_node, c_file_name);
    PrintSecondsElapsed("Finished ddmin");
    SwitchFileContents(c_file_name, reduced_c_file_name);
    ListDelete(&units);

    //
    // TEST END
    //

    PrintSecondsElapsed("Done. Output in '%s'", reduced_c_file_name);
    free(run_predicate_command);
    free(c_file_text);
    free(c_file_name_without_extension);
    free(reduced_c_file_name);
    ts_tree_delete(tree);
    ts_parser_delete(parser);
    return 0;
}
