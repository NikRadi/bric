#include "AstNode.h"
#include <stdarg.h>
#include <assert.h>
#include <stdlib.h> // malloc
#include <string.h> // strndup


#define NEW(x) (x *) malloc(sizeof(x))

static const int INDENT_AMOUNT = 4;
static int indent = 0;


static void PrintIndented(const char *format, ...) {
    va_list arg_list;
    va_start(arg_list, format);
    for (int i = 0; i < indent; ++i) {
        printf(" ");
    }

    vfprintf(stdout, format, arg_list);
    printf("\n");
    va_end(arg_list);
}

static void PrintStartSection(AstNode *node) {
    char *is_ignored_str = (node->is_ignored) ? "true" : "false";
    PrintIndented("<%s is_ignored=\"%s\">", node->type, is_ignored_str);
    indent += INDENT_AMOUNT;
}

static void PrintEndSection(const char *node_type) {
    indent -= INDENT_AMOUNT;
    PrintIndented("<%s/>", node_type);
}

static bool TSNodeIsType(TSNode node, const char *type) {
    const char *node_type = ts_node_type(node);
    return strcmp(node_type, type) == 0;
}

static bool AstNodeIsType(AstNode *node, const char *type) {
    return TSNodeIsType(node->ts_node, type);
}

static AstNode *GetChild(AstNode *node, int idx) {
    return (AstNode *) ListGet(&node->children, idx);
}

// @Improvement: The 'source_code' parameter is not really needed.
//               The information is saved in each nodes parent.
static void WriteAstLeafNodesToFileRecursively(AstNode *node, File file, char *source_code) {
    assert(!node->is_ignored);
    if (node->children.count == 0) {
        fputs(node->value, file.stream);
        return;
    }

    assert(node->children.count > 0);
    AstNode *last_child = GetChild(node, 0);
    WriteAstLeafNodesToFileRecursively(last_child, file, source_code);
    for (int i = 1; i < node->children.count; ++i) {
        AstNode *current_child = GetChild(node, i);
        if (!current_child->is_ignored) {
            uint32_t bytes_to_copy = current_child->start_byte - last_child->end_byte;
            if (bytes_to_copy > 0) {
                char *str_start = source_code + last_child->end_byte;
                char *str_between = strndup(str_start, bytes_to_copy);
                fputs(str_between, file.stream);
            }

            WriteAstLeafNodesToFileRecursively(current_child, file, source_code);
        }

        last_child = current_child;
    }
}

static AstNode *NewAstNode(TSNode ts_node, char *source_code) {
    AstNode *ast_node = NEW(AstNode);
    ast_node->ts_node = ts_node;
    const char *node_type = ts_node_type(ts_node);
    ast_node->type = strdup(node_type);
    ast_node->children = ListInit();
    ast_node->is_ignored = false;

    ast_node->start_byte = ts_node_start_byte(ts_node);
    ast_node->end_byte = ts_node_end_byte(ts_node);
    uint32_t node_size_byte = ast_node->end_byte - ast_node->start_byte;
    ast_node->value = strndup(source_code + ast_node->start_byte, node_size_byte);
    return ast_node;
}

AstNode *AstInit(TSNode ts_node, char *source_code) {
    AstNode *node = NewAstNode(ts_node, source_code);
    // For our use case, it makes better sense that 'string_literal'
    // types are leaves. Therefore, we ignore the children of
    // 'string_literal' types.
    if (!TSNodeIsType(ts_node, "string_literal")) {
        for (int i = 0; i < ts_node_child_count(ts_node); ++i) {
            TSNode ts_child_node = ts_node_child(ts_node, i);
            AstNode *child_node = AstInit(ts_child_node, source_code);
            ListAdd(&node->children, child_node);
        }
    }

    return node;
}

void AstPrint(AstNode *node) {
    PrintStartSection(node);
    PrintIndented("[%s]", node->value);
    for (int i = 0; i < node->children.count; ++i) {
        AstNode *child_node = GetChild(node, i);
        AstPrint(child_node);
    }

    PrintEndSection(node->type);
}

void AstWriteLeafNodesToFile(AstNode *node, File file) {
    WriteAstLeafNodesToFileRecursively(node, file, node->value);
    fputs("\n", file.stream);
}

void AstFindAllNodesOfType(AstNode *node, List *list, const char *type) {
    if (AstNodeIsType(node, type)) {
        ListAdd(list, node);
    }

    for (int i = 0; i < node->children.count; ++i) {
        AstNode *child = GetChild(node, i);
        AstFindAllNodesOfType(child, list, type);
    }
}
