#ifndef BRIC_TREE_HPP
#define BRIC_TREE_HPP
#include <string>
#include <tree_sitter/api.h>
#include <vector>


enum AstType {
    AST_BRANCH, AST_LEAF,
};

struct Ast {
    TSNode ts_node;
    AstType type;
};

struct Leaf : public Ast {
    bool is_active = true;
    bool is_mergeable = false;
    uint start_byte;
    uint end_byte;
    Leaf *next_leaf = NULL;
    Leaf *prev_leaf = NULL;
    std::string value;

    Leaf(TSNode n) { ts_node = n; type = AST_LEAF; }
};

struct Branch : public Ast {
    Leaf *leftmost_leaf = NULL;
    Leaf *rightmost_leaf = NULL;
    std::vector<Ast *> children;

    Branch(TSNode n) { ts_node = n; type = AST_BRANCH; }
};

struct Tree {
    Branch *root_node = NULL;
    std::string source_code;
    std::vector<Ast *> leaves;
};


Tree TreeInit(TSNode ts_node, std::string source_code);

void TreeDelete(Tree &self);

void TreeFindNodes(Tree &self, std::vector<Ast *> &nodes, std::string type);

void TreeWriteToFile(Tree &self, std::string file_name, std::string mode);

void TreePrint(Tree &self);

#endif // BRIC_TREE_HPP