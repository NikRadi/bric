#ifndef BRIC_AST_NODES_HPP
#define BRIC_AST_NODES_HPP
#include <tree_sitter/api.h>
#include <vector>


enum AstFlags {
    AST_IS_ACTIVE         = 0b0001,
    AST_TYPE_LEAF         = 0b0010,
    AST_TYPE_BRANCH       = 0b0100,
    AST_TYPE_FUNCTION_DEF = 0b1100,
};

struct Ast {
    int flags = AST_IS_ACTIVE;
    const char *type = NULL;
};

struct Leaf : public Ast {
    char *pre_value;
    char *value;

    Leaf() { flags |= AST_TYPE_LEAF; }
};

struct Branch : public Ast {
    std::vector<Ast *> children;

    Branch() { flags |= AST_TYPE_BRANCH; }
};

struct FunctionDef : public Branch {
    std::vector<Ast *> dependencies;

    FunctionDef() { flags |= AST_TYPE_FUNCTION_DEF; }
};

Ast *AstInit(TSNode root_node, const char *source_code);

void AstWriteToFile(Ast *root_node, const char *file_name);

Ast *AstFindNodeInChildren(Branch *node, const char *type);

void AstFindNodes(Ast *node, const char *type, std::vector<Ast *> &nodes);

void AstFindNodes(Ast *node, AstFlags flags, int level, std::vector<Ast *> &nodes);

void AstFindNodes(Ast *node, AstFlags flags, std::vector<Ast *> &nodes);

#endif // BRIC_AST_NODES_HPP
