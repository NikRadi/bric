#include "AstNodes.hpp"
#include <fstream>
#include <cstring>
#include <tree_sitter/api.h>


static Ast *AstInit(TSNode ts_node, std::string source_code, uint32_t &prev_end_byte) {
    Ast *ast = new Ast;
    ast->ts_type = ts_node_type(ts_node);
    uint32_t num_children = ts_node_child_count(ts_node);
    if (num_children == 0) {
        ast->type = AST_TYPE_LEAF;
        uint32_t start_byte = ts_node_start_byte(ts_node);
        uint32_t num_bytes_pre = start_byte - prev_end_byte;
        ast->pre_value = source_code.substr(prev_end_byte, num_bytes_pre);

        uint32_t end_byte = ts_node_end_byte(ts_node);
        uint32_t num_bytes = end_byte - start_byte;
        ast->value = source_code.substr(start_byte, num_bytes);
        prev_end_byte = end_byte;
    }
    else {
        ast->type = AST_TYPE_BRANCH;
        for (uint32_t i = 0; i < num_children; ++i) {
            TSNode ts_child = ts_node_child(ts_node, i);
            Ast *child = AstInit(ts_child, source_code, prev_end_byte);
            ast->children.push_back(child);
        }
    }

    return ast;
}

Ast *AstInit(TSNode ts_root_node, std::string source_code) {
    uint32_t prev_end_byte = 0;
    return AstInit(ts_root_node, source_code, prev_end_byte);
}

static void WriteToFile(Ast *node, std::ofstream &ofstream) {
    if (!node->is_active) {
        return;
    }

    if (node->type == AST_TYPE_LEAF) {
        ofstream << node->pre_value;
        ofstream << node->value;
    }
    else {
        for (auto child : node->children) {
            WriteToFile(child, ofstream);
        }
    }
}

void WriteToFile(Ast *root, std::string file_name) {
    std::ofstream ofstream(file_name);
    WriteToFile(root, ofstream);
    ofstream.close();
}

void FindNodes(Ast *node, std::string ts_type, std::vector<Ast *> &nodes) {
    if (node->ts_type == ts_type) {
        nodes.push_back(node);
    }

    for (auto child : node->children) {
        FindNodes(child, ts_type, nodes);
    }
}

Ast *FindChild(Ast *node, std::string ts_type) {
    for (auto child : node->children) {
        if (child->ts_type == ts_type) {
            return child;
        }
    }

    return NULL;
}

void FindActiveNodes(Ast *node, int level, std::vector<Ast *> &nodes) {
    if (!node->is_active) {
        return;
    }

    if (level == 0) {
        nodes.push_back(node);
    }
    else {
        for (auto child : node->children) {
            FindActiveNodes(child, level - 1, nodes);
        }
    }
}

bool IsTestSuccessful(Ast *root_node, std::string f, std::string p) {
    WriteToFile(root_node, f);
    return system(p.c_str()) == 0;
}

void Print(Ast *node) {
    static int indent = 0;
    for (int i = 0; i < indent; ++i) printf(" ");
    if (node->type == AST_TYPE_LEAF) {
        printf("<Leaf \"%s\"/>\n", node->ts_type.c_str());
    }
    else {
        printf("<Branch \"%s\">\n", node->ts_type.c_str());
        indent += 4;
        for (auto child : node->children) {
            Print(child);
        }

        indent -= 4;
        for (int i = 0; i < indent; ++i) printf(" ");
        printf("<Branch/>\n");
    }
}

int Count(Ast *node) {
    int count = 0;
    for (auto child : node->children) {
        count += Count(child);
    }

    return count + 1;
}

int CountActive(Ast *node) {
    if (!node->is_active) {
        return 0;
    }

    int count = 0;
    for (auto child : node->children) {
        count += Count(child);
    }

    return count + 1;
}
