#ifndef BRIC_TREE_HPP
#define BRIC_TREE_HPP
#include <string>
#include <tree_sitter/api.h>
#include <vector>


struct Branch;

enum AstType {
    AST_BRANCH, AST_LEAF
};

struct Ast {
    AstType type;
    TSNode ts_node;
    Branch *parent = NULL;
    int child_idx = -1;
};

struct Leaf : public Ast {
    std::string value;
    bool has_prev_leaf;
    uint prev_leaf_end_byte;

    Leaf(TSNode n) { ts_node = n; type = AST_LEAF; }
};

struct Branch : public Ast {
    std::vector<Ast *> children;

    Branch(TSNode n) { ts_node = n; type = AST_BRANCH; }
};

struct Tree {
    Ast *root_node;
    std::string source_code;
};


Tree *TreeInit(TSNode ts_node, std::string source_code);

void TreeDelete(Tree *self);

void TreeFindUnits(Tree *self, std::vector<Ast **> &units, std::string ts_type);

void TreeWriteToFile(Tree *self, std::string file_name);

void TreePrintXml(Tree *self);

#endif // BRIC_TREE_HPP
