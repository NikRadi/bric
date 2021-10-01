#ifndef BRIC_ASTNODE_H
#define BRIC_ASTNODE_H
#include <stdbool.h>
#include <tree_sitter/api.h>
#include "FileHandling.h"
#include "List.h"


struct AstNode {
    TSNode ts_node;
    const char *type;
    const char *value;
    uint32_t start_byte;
    uint32_t end_byte;
    List children;
    bool is_ignored;
} typedef AstNode;


AstNode *AstInit(TSNode ts_node, char *source_code);

void AstPrint(AstNode *node);

void AstWriteLeafNodesToFile(AstNode *node, File file, char *source_code);

void AstFindAllNodesOfType(AstNode *node, List *list, const char *type);

#endif // BRIC_ASTNODE_H
