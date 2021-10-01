#include <tree_sitter/api.h>
#include <string.h> // strndup
#include "AstNode.h"
#include "Ddmin.h"
#include "FileHandling.h"
#include "List.h"


TSLanguage *tree_sitter_c();
static const char *USAGE_STR =
    "bric usage:\n"
    "bric predicate_file file_to_reduce\n";


static char *AddPrefix(const char *prefix, const char *str) {
    int str_len = strlen(str);
    int prefix_len = strlen(prefix);
    int result_len = str_len + prefix_len;
    char *result = (char *) malloc(result_len * sizeof(char));
    strncpy(result, prefix, prefix_len);
    strncpy(result + prefix_len, str, str_len);
    return result;
}

static char *AddPostfix(const char *str, const char *postfix) {
    return AddPrefix(str, postfix);
}

int main(int argc, char **argv) {
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

    //
    // TEST START
    //

    AstNode *root_node = AstInit(ts_root_node, c_file_text);
    List units = ListInit();
    AstFindAllNodesOfType(root_node, &units, "expression_statement");
    Ddmin(&units, run_predicate_command, root_node, c_file_name, c_file_text);
    SwitchFileContents(c_file_name, reduced_c_file_name);
    ListDelete(&units);

    //
    // TEST END
    //

    free(run_predicate_command);
    free(c_file_text);
    return 0;
}
