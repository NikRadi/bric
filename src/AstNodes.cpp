#include "AstNodes.hpp"
#include <fstream>
#include <string>
#include <tree_sitter/api.h>


static Ast *AstInit(TSNode ts_node, const char *source_code, uint32_t &prev_end_byte) {
    uint32_t num_children = ts_node_child_count(ts_node);
    if (num_children == 0) {
        uint32_t start_byte = ts_node_start_byte(ts_node);
        uint32_t num_bytes_pre_value = start_byte - prev_end_byte;
        char *leaf_pre_value = new char[num_bytes_pre_value + 1];
        leaf_pre_value[num_bytes_pre_value] = '\0';
        strncpy(leaf_pre_value, source_code + prev_end_byte, num_bytes_pre_value);

        uint32_t end_byte = ts_node_end_byte(ts_node);
        uint32_t num_bytes_value = end_byte - start_byte;
        char *leaf_value = new char[num_bytes_value + 1];
        leaf_value[num_bytes_value] = '\0';
        strncpy(leaf_value, source_code + start_byte, num_bytes_value);
        prev_end_byte = end_byte;

        Leaf *leaf = new Leaf;
        leaf->type = ts_node_type(ts_node);
        leaf->pre_value = leaf_pre_value;
        leaf->value = leaf_value;
        return static_cast<Ast *>(leaf);
    }

    const char *node_type = ts_node_type(ts_node);
    Branch *branch = new Branch;
    branch->type = node_type;
    if (strcmp(node_type, "function_definition") == 0) {
        branch->flags |= AST_TYPE_FUNCTION_DEF;
    }

    for (uint32_t i = 0; i < num_children; ++i) {
        TSNode ts_child = ts_node_child(ts_node, i);
        Ast *child = AstInit(ts_child, source_code, prev_end_byte);
        branch->children.push_back(child);
    }

    return static_cast<Ast *>(branch);
}

static void AstWriteToFile(Ast *node, std::ofstream &ofstream) {
    if (!(node->flags & AST_IS_ACTIVE)) {
        return;
    }

    if (node->flags & AST_TYPE_LEAF) {
        Leaf *leaf = static_cast<Leaf *>(node);
        ofstream << leaf->pre_value;
        ofstream << leaf->value;
    }
    else {
        Branch *branch = static_cast<Branch *>(node);
        for (size_t i = 0; i < branch->children.size(); ++i) {
            AstWriteToFile(branch->children[i], ofstream);
        }
    }
}

Ast *AstInit(TSNode ts_node, const char *source_code) {
    uint32_t prev_end_byte = 0;
    return AstInit(ts_node, source_code, prev_end_byte);
}


void AstWriteToFile(Ast *root_node, const char *file_name) {
    std::ofstream ofstream(file_name);
    AstWriteToFile(root_node, ofstream);
    ofstream.close();
}

Ast *AstFindChild(Branch *node, const char *type) {
    for (size_t i = 0; i < node->children.size(); ++i) {
        if (strcmp(node->children[i]->type, type) == 0) {
            return node->children[i];
        }
    }

    return NULL;
}

void AstFindNodes(Ast *node, const char *type, std::vector<Ast *> &nodes) {
    if (strcmp(node->type, type) == 0) {
        nodes.push_back(node);
    }

    if (node->flags & AST_TYPE_BRANCH) {
        Branch *branch = static_cast<Branch *>(node);
        for (size_t i = 0; i < branch->children.size(); ++i) {
            AstFindNodes(branch->children[i], type, nodes);
        }
    }
}

void AstFindNodes(Ast *node, AstFlags flags, int level, std::vector<Ast *> &nodes) {
    if (level == 0 && (node->flags & flags) == flags) {
        nodes.push_back(node);
    }

    if (node->flags & AST_TYPE_BRANCH) {
        Branch *branch = static_cast<Branch *>(node);
        for (size_t i = 0; i < branch->children.size(); ++i) {
            AstFindNodes(branch->children[i], flags, level - 1, nodes);
        }
    }
}

void AstFindNodes(Ast *node, AstFlags flags, std::vector<Ast *> &nodes) {
    if ((node->flags & flags) == flags) {
        nodes.push_back(node);
    }

    if (node->flags & AST_TYPE_BRANCH) {
        Branch *branch = static_cast<Branch *>(node);
        for (size_t i = 0; i < branch->children.size(); ++i) {
            AstFindNodes(branch->children[i], flags, nodes);
        }
    }
}
