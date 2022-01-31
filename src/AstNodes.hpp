#ifndef BRIC_AST_NODES_HPP
#define BRIC_AST_NODES_HPP
#include <string>
#include <tree_sitter/api.h>
#include <vector>


enum AstType {
    AST_TYPE_LEAF,
    AST_TYPE_BRANCH,
};

struct Ast {
    AstType type;
    bool is_active = true;
    std::string ts_type;
    std::string pre_value;
    std::string value;
    std::vector<Ast *> children;
    std::vector<size_t> dependencies;
};


Ast *AstInit(TSNode ts_root_node, std::string source_code);

void WriteToFile(Ast *root_node, std::string file_name);

void FindNodes(Ast *node, std::string ts_type, std::vector<Ast *> &nodes);

Ast *FindChild(Ast *node, std::string ts_type);

void FindActiveNodes(Ast *node, int level, std::vector<Ast *> &nodes);

bool IsTestSuccessful(Ast *root_node, std::string f, std::string p);

void Print(Ast *node);

int Count(Ast *node);

int CountActive(Ast *node);

#endif // BRIC_AST_NODES_HPP
