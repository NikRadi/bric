#include <cstring>
#include <fstream>
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

static void CountNodes(TSNode node, int &nodes, int &error_nodes) {
    const char *type = ts_node_type(node);
    if (strcmp(type, "ERROR") == 0) {
        error_nodes += 1;
    }
    else {
        nodes += 1;
        uint32_t num_children = ts_node_child_count(node);
        for (uint32_t i = 0; i < num_children; ++i) {
            TSNode child = ts_node_child(node, i);
            CountNodes(child, nodes, error_nodes);
        }
    }
}

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("need file name\n");
        return 0;
    }

    std::string file_name = argv[1];
    std::string source_code = ReadFile(file_name);

    TSParser *parser = ts_parser_new();
    ts_parser_set_language(parser, tree_sitter_c());
    TSTree *ts_tree = ts_parser_parse_string(parser, NULL, source_code.c_str(), static_cast<uint32_t>(source_code.size()));
    TSNode ts_root_node = ts_tree_root_node(ts_tree);

    int nodes = 0;
    int error_nodes = 0;
    CountNodes(ts_root_node, nodes, error_nodes);
    printf("%d,%d\n", nodes, error_nodes);
    return 0;
}
