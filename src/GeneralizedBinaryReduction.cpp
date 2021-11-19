#include "GeneralizedBinaryReduction.hpp"
#include <algorithm>
#include <cassert>
#include <fstream>
#include <stack>
#include <tuple>
#include <unordered_map>
#include <vector>


// The enums and structs are named differently than in
// 'AstNodes.hpp' to avoid problems (Which did occur...)
enum NodeType {
    NODE_LEAF,
    NODE_BRANCH,
    NODE_CALL_EXPR,
    NODE_FUNCTION_CODE,
    NODE_FUNCTION_DEF,
};

struct Node {
    NodeType type;
    bool is_active;
};

struct LeafNode : public Node {
    char *pre_value;
    char *value;
};

struct BranchNode : public Node {
    std::vector<Node *> children;
};

struct CallExpr : public Node {
    std::vector<Node *> children;
    char *identifier;
};

// [A()!code]
struct FunctionCode : public Node {
    std::vector<Node *> dependencies; // Nodes that this FunctionCode depends on
    std::vector<Node *> children;
};

// [A()]
struct FunctionDef : public Node {
    std::vector<Node *> dependers; // Nodes that depend on this FunctionDef
    char *identifier;
    char *value;
    FunctionCode *code;
};


static Node *InitAst(TSNode ts_node, const char *source_code, uint32_t &prev_end_byte);


static void Print(Node *node) {
    if (node->type == NODE_FUNCTION_CODE) {
        FunctionCode *f = static_cast<FunctionCode *>(node);
        FunctionDef *d = static_cast<FunctionDef *>(f->dependencies[0]);
        printf("[%s()!code]", d->identifier);
    }
    else if (node->type == NODE_FUNCTION_DEF) {
        FunctionDef *f = static_cast<FunctionDef *>(node);
        printf("[%s()]", f->identifier);
    }
}

template <class T>
static void Print(std::vector<T> vector) {
    printf("{");
    if (vector.size() > 0) {
        Print(vector[0]);
        for (size_t i = 1; i < vector.size(); ++i) {
            printf(", ");
            Print(vector[i]);
        }
    }

    printf("}");
}

static void PrintXml(Node *node) {
    static int indent = 0;
    for (int i = 0; i < indent; ++i) printf(" ");
    if (node->type == NODE_LEAF) {
        LeafNode *l = static_cast<LeafNode *>(node);
        printf("<Leaf pre_value=\"%s\" value=\"%s\"/>\n", l->pre_value, l->value);
    }
    else if (node->type == NODE_BRANCH) {
        assert(node->type == NODE_BRANCH);
        BranchNode *b = static_cast<BranchNode *>(node);
        printf("<Branch>\n");
        indent += 4;
        for (size_t i = 0; i < b->children.size(); ++i) {
            PrintXml(b->children[i]);
        }

        indent -= 4;
        for (int i = 0; i < indent; ++i) printf(" ");
        printf("<Branch/>\n");
    }
    else if (node->type == NODE_CALL_EXPR) {
        CallExpr *c = static_cast<CallExpr *>(node);
        printf("<CallExpr identifier=\"%s\">\n", c->identifier);
        indent += 4;
        for (size_t i = 0; i < c->children.size(); ++i) {
            PrintXml(c->children[i]);
        }

        indent -= 4;
        for (int i = 0; i < indent; ++i) printf(" ");
        printf("<CallExpr/>\n");
    }
    else if (node->type == NODE_FUNCTION_CODE) {
        FunctionCode *f = static_cast<FunctionCode *>(node);
        printf("<FunctionCode>\n");
        indent += 4;
        for (size_t i = 0; i < f->children.size(); ++i) {
            PrintXml(f->children[i]);
        }

        indent -= 4;
        for (int i = 0; i < indent; ++i) printf(" ");
        printf("<FunctionCode/>\n");
    }
    else if (node->type == NODE_FUNCTION_DEF) {
        FunctionDef *f = static_cast<FunctionDef *>(node);
        printf("<FunctionDef identifier=\"%s\" value=\"%s\">\n", f->identifier, f->value);
        indent += 4;
        PrintXml(static_cast<Node *>(f->code));
        indent -= 4;
        for (int i = 0; i < indent; ++i) printf(" ");
        printf("<FunctionDef/>\n");
    }
}

static void WriteToFile(Node *node, std::ofstream &ofstream) {
    if (!node->is_active) {
        return;
    }

    switch (node->type) {
        case NODE_LEAF: {
            LeafNode *l = static_cast<LeafNode *>(node);
            ofstream << l->pre_value;
            ofstream << l->value;
        } break;
        case NODE_BRANCH: {
            BranchNode *b = static_cast<BranchNode *>(node);
            for (auto child : b->children) {
                WriteToFile(child, ofstream);
            }
        } break;
        case NODE_CALL_EXPR: {
            CallExpr *c = static_cast<CallExpr *>(node);
            for (auto child : c->children) {
                WriteToFile(child, ofstream);
            }
        } break;
        case NODE_FUNCTION_CODE: {
            FunctionCode *f = static_cast<FunctionCode *>(node);
            for (auto child : f->children) {
                WriteToFile(child, ofstream);
            }
        } break;
        case NODE_FUNCTION_DEF: {
            FunctionDef *f = static_cast<FunctionDef *>(node);
            ofstream << f->value;
            ofstream << " {";
            WriteToFile(f->code, ofstream);
            if (f->code->children.size() <= 1) {
                ofstream << " }\n";
            }
            else {
                ofstream << "\n}\n";
            }
        } break;
    }
}

static void WriteToFile(Node *root_node, const char *file_name) {
    std::ofstream ofstream(file_name);
    WriteToFile(root_node, ofstream);
    ofstream.close();
}

static bool IsPredicateSuccessful(Node *root_node, const char *file_name, const char *run_predicate_command) {
    WriteToFile(root_node, file_name);
    return system(run_predicate_command) == 0;
}

static char *DuplicateString(const char *str, uint32_t start_byte, uint32_t end_byte) {
    assert(start_byte <= end_byte);
    uint32_t num_bytes = end_byte - start_byte;
    char *result = new char[num_bytes + 1];
    result[num_bytes] = '\0';
    strncpy(result, str + start_byte, num_bytes);
    return result;
}

static void FindChildren(Node *node, NodeType node_type, std::vector<Node *> &nodes) {
    if (node->type == node_type) {
        nodes.push_back(node);
    }

    if (node->type == NODE_BRANCH) {
        BranchNode *b = static_cast<BranchNode *>(node);
        for (size_t i = 0; i < b->children.size(); ++i) {
            FindChildren(b->children[i], node_type, nodes);
        }
    }
    else if (node->type == NODE_CALL_EXPR) {
        CallExpr *c = static_cast<CallExpr *>(node);
        for (size_t i = 0; i < c->children.size(); ++i) {
            FindChildren(c->children[i], node_type, nodes);
        }
    }
    else if (node->type == NODE_FUNCTION_CODE) {
        FunctionCode *f = static_cast<FunctionCode *>(node);
        for (size_t i = 0; i < f->children.size(); ++i) {
            FindChildren(f->children[i], node_type, nodes);
        }
    }
    else if (node->type == NODE_FUNCTION_DEF) {
        FunctionDef *f = static_cast<FunctionDef *>(node);
        FindChildren(static_cast<Node *>(f->code), node_type, nodes);
    }
}

static char *GetValue(TSNode ts_node, const char *source_code) {
    uint32_t start_byte = ts_node_start_byte(ts_node);
    uint32_t end_byte = ts_node_end_byte(ts_node);
    return DuplicateString(source_code, start_byte, end_byte);
}

static TSNode FindChild(TSNode ts_node, const char *type) {
    uint32_t num_children = ts_node_child_count(ts_node);
    for (uint32_t i = 0; i < num_children; ++i) {
        TSNode child = ts_node_child(ts_node, i);
        const char *node_type = ts_node_type(child);
        if (strcmp(node_type, type) == 0) {
            return child;
        }
    }

    assert(false);
    return { };
}

static CallExpr *InitCallExpr(TSNode ts_node, const char *source_code, uint32_t &prev_end_byte) {
    CallExpr *call_expr = new CallExpr;
    call_expr->type = NODE_CALL_EXPR;
    call_expr->is_active = true;

    TSNode identifier = ts_node_child(ts_node, 0);
    call_expr->identifier = GetValue(identifier, source_code);

    uint32_t num_children = ts_node_child_count(ts_node);
    for (uint32_t i = 0; i < num_children; ++i) {
        TSNode ts_child = ts_node_child(ts_node, i);
        Node *child = InitAst(ts_child, source_code, prev_end_byte);
        call_expr->children.push_back(child);
    }

    return call_expr;
}

static FunctionCode *InitFunctionCode(TSNode ts_node, const char *source_code, uint32_t &prev_end_byte) {
    FunctionCode *function_code = new FunctionCode;
    function_code->type = NODE_FUNCTION_CODE;
    function_code->is_active = true;

    // Skip first and last children ('{' and '}')
    TSNode first_child = ts_node_child(ts_node, 0);
    prev_end_byte = ts_node_end_byte(first_child);
    uint32_t num_children = ts_node_child_count(ts_node);
    for (uint32_t i = 1; i < num_children - 1; ++i) {
        TSNode ts_child = ts_node_child(ts_node, i);
        Node *child = InitAst(ts_child, source_code, prev_end_byte);
        function_code->children.push_back(child);
    }

    return function_code;
}

static FunctionDef *InitFunctionDef(TSNode ts_node, const char *source_code, uint32_t &prev_end_byte) {
    FunctionDef *function_def = new FunctionDef;
    function_def->type = NODE_FUNCTION_DEF;
    function_def->is_active = true;

    TSNode function_declarator = FindChild(ts_node, "function_declarator");
    TSNode identifier = FindChild(function_declarator, "identifier");
    function_def->identifier = GetValue(identifier, source_code);

    // Since the last child is of type 'compound_statement',
    // the end byte must be that of the second last child
    uint32_t num_children = ts_node_child_count(ts_node);
    TSNode second_last_child = ts_node_child(ts_node, num_children - 2);
    uint32_t start_byte = ts_node_start_byte(ts_node);
    uint32_t end_byte = ts_node_end_byte(second_last_child);
    function_def->value = DuplicateString(source_code, start_byte, end_byte);

    TSNode function_code = ts_node_child(ts_node, num_children - 1);
    function_def->code = InitFunctionCode(function_code, source_code, prev_end_byte);
    return function_def;
}

static Node *InitAst(TSNode ts_node, const char *source_code, uint32_t &prev_end_byte) {
    const char *node_type = ts_node_type(ts_node);
    if (strcmp(node_type, "call_expression") == 0) {
        return static_cast<Node *>(InitCallExpr(ts_node, source_code, prev_end_byte));
    }

    if (strcmp(node_type, "function_definition") == 0) {
        return static_cast<Node *>(InitFunctionDef(ts_node, source_code, prev_end_byte));
    }

    uint32_t num_children = ts_node_child_count(ts_node);
    if (num_children == 0) {
        LeafNode *leaf_node = new LeafNode;
        leaf_node->type = NODE_LEAF;
        leaf_node->is_active = true;

        uint32_t start_byte = ts_node_start_byte(ts_node);
        uint32_t end_byte = ts_node_end_byte(ts_node);
        leaf_node->pre_value = DuplicateString(source_code, prev_end_byte, start_byte);
        leaf_node->value = DuplicateString(source_code, start_byte, end_byte);

        prev_end_byte = end_byte;
        return static_cast<Node *>(leaf_node);
    }

    BranchNode *branch_node = new BranchNode;
    branch_node->type = NODE_BRANCH;
    branch_node->is_active = true;
    for (uint32_t i = 0; i < num_children; ++i) {
        TSNode ts_child = ts_node_child(ts_node, i);
        Node *child = InitAst(ts_child, source_code, prev_end_byte);
        branch_node->children.push_back(child);
    }

    return static_cast<Node *>(branch_node);
}

static void AddDependencies(std::vector<Node *> &list, Node *node) {
    if (node->type == NODE_FUNCTION_CODE) {
        FunctionCode *f = static_cast<FunctionCode *>(node);
        for (size_t i = 0; i < f->dependencies.size(); ++i) {
            list.push_back(f->dependencies[i]);
            AddDependencies(list, f->dependencies[i]);
        }
    }
}

static void FindDependencies(Node *node, std::vector<Node *> &nodes_to_reduce) {
    std::vector<Node *> function_defs;
    FindChildren(node, NODE_FUNCTION_DEF, function_defs);
    for (size_t i = 0; i < function_defs.size(); ++i) {
        FunctionDef *f = static_cast<FunctionDef *>(function_defs[i]);

        nodes_to_reduce.push_back(function_defs[i]);
        nodes_to_reduce.push_back(static_cast<Node *>(f->code));

        // Syntactic dependency: [A()!code] => [A()]
        f->code->dependencies.push_back(static_cast<Node *>(f));
        f->dependers.push_back(static_cast<Node *>(f->code));

        std::vector<Node *> call_exprs;
        FindChildren(static_cast<Node *>(f->code), NODE_CALL_EXPR, call_exprs);
        for (size_t j = 0; j < call_exprs.size(); ++j) {
            CallExpr *c = static_cast<CallExpr *>(call_exprs[j]);
            for (size_t k = 0; k < function_defs.size(); ++k) {
                FunctionDef *f2 = static_cast<FunctionDef *>(function_defs[k]);
                if (strcmp(c->identifier, f2->identifier) == 0) {
                    // Referential semantic dependency: [A()!code] => [B()]
                    f->code->dependencies.push_back(function_defs[k]);
                    f2->dependers.push_back(static_cast<Node *>(f->code));
                    break;
                }
            }
        }
    }
}

static std::vector<Node *> Postorder(std::vector<Node *> nodes, std::unordered_map<Node *, bool> &is_visited) {
    std::vector<Node *> result;
    std::stack<std::tuple<Node *, bool>> stack;
    for (size_t i = 0; i < nodes.size(); ++i) {
        size_t idx = nodes.size() - 1 - i;
        std::tuple<Node *, bool> t = std::make_tuple(nodes[idx], false);
        stack.push(t);
    }

    while (!stack.empty()) {
        std::tuple<Node *, bool> t = stack.top();
        stack.pop();
        Node *node = std::get<0>(t);
        bool post = std::get<1>(t);
        if (post) {
            result.push_back(node);
        }
        else if (!is_visited[node]) {
            is_visited[node] = true;
            stack.push(std::make_tuple(node, true));
            if (node->type == NODE_FUNCTION_CODE) {
                FunctionCode *f = static_cast<FunctionCode *>(node);
                for (auto dependency: f->dependencies) {
                    stack.push(std::make_tuple(dependency, false));
                }
            }
        }
    }

    return result;
}

static std::vector<std::vector<Node *>> Progression(std::vector<Node *> variableorder, std::vector<Node *> nodes, std::vector<Node *> items, std::vector<Node *> learned_set) {
    std::vector<std::vector<Node *>> result;
    std::unordered_map<Node *, bool> is_visited;
    for (auto node : nodes) {
        auto it = std::find(items.begin(), items.end(), node);
        is_visited[node] = it == items.end();
    }

    for (auto node : learned_set) {
        is_visited[node] = true;
    }

    result.push_back(learned_set);
    for (auto node : variableorder) {
        if (!is_visited[node]) {
            std::vector<Node *> postorder = Postorder({ node }, is_visited);
            result.push_back(postorder);
        }
    }

    return result;
}

static size_t FindSmallestSuccessfulIdx(Node *root_node, const char *file_name, const char *run_predicate_command, std::vector<std::vector<Node *>> &d) {
    size_t min_idx = 1;
    size_t max_idx = d.size() - 1;
    while (min_idx < max_idx) {
        size_t mid_idx = (min_idx + max_idx) >> 1;
        printf("%zu %zu %zu\n", min_idx, mid_idx, max_idx);
        for (size_t i = 0; i <= mid_idx; ++i) {
            for (auto node : d[i]) {
                node->is_active = true;
            }
        }

        for (size_t i = mid_idx + 1; i < d.size(); ++i) {
            for (auto node : d[i]) {
                node->is_active = false;
            }
        }

        if (IsPredicateSuccessful(root_node, file_name, run_predicate_command)) {
            printf("success\n");
            max_idx = mid_idx;
        }
        else {
            printf("failure\n");
            min_idx = mid_idx + 1;
        }
    }

    return min_idx;
}

void GeneralizedBinaryReduction(TSNode ts_root_node, const char *file_name, const char *run_predicate_command, const char *source_code) {
    uint32_t prev_end_byte = 0;
    Node *root_node = InitAst(ts_root_node, source_code, prev_end_byte);

    std::vector<Node *> nodes_to_reduce;
    FindDependencies(root_node, nodes_to_reduce);

    std::vector<Node *> function_defs;
    FindChildren(root_node, NODE_FUNCTION_DEF, function_defs);
    FunctionDef *main_function = NULL;
    for (size_t i = 0; i < function_defs.size(); ++i) {
        FunctionDef *f = static_cast<FunctionDef *>(function_defs[i]);
        if (strcmp(f->identifier, "main") == 0) {
            main_function = f;
            break;
        }
    }

    // Add [main()!code] since we know it is needed for the program to be valid
    assert(main_function != NULL);
    std::vector<Node *> learned_set;
    learned_set.push_back(static_cast<Node *>(main_function->code));
    AddDependencies(learned_set, main_function->code);

    std::unordered_map<Node *, bool> is_visited;
    for (auto node : nodes_to_reduce) {
        is_visited[node] = false;
    }

    for (auto node : learned_set) {
        is_visited[node] = true;
    }

    std::vector<Node *> variableorder = Postorder(nodes_to_reduce, is_visited);
    std::vector<std::vector<Node *>> d = Progression(variableorder, nodes_to_reduce, nodes_to_reduce, learned_set);

    printf("nodes to reduce: ");
    Print(nodes_to_reduce);
    printf("\n\n");

    printf("variableorder: ");
    Print(variableorder);
    printf("\n\n");

    int counter = 0;
    do {
        printf("\n============\n");
        printf("progression %d: ", counter);
        Print(d);
        printf("\n\n");
        counter += 1;
        size_t idx = FindSmallestSuccessfulIdx(root_node, file_name, run_predicate_command, d);
        // It works with d[0] and the search is finished
        if (idx == 0) {
            break;
        }

        for (auto node : d[idx]) {
            learned_set.push_back(node);
        }

        std::vector<Node *> items;
        for (size_t i = 0; i <= idx; ++i) {
            for (auto node : d[i]) {
                items.push_back(node);
            }
        }

        d = Progression(variableorder, nodes_to_reduce, items, learned_set);

        // Ensure nodes in d[0] are active
        for (auto node : d[0]) {
            node->is_active = true;
        }

        // Ensure all other nodes are not active
        for (size_t i = 1; i < d.size(); ++i) {
            for (auto node : d[i]) {
                node->is_active = false;
            }
        }
    } while (!IsPredicateSuccessful(root_node, file_name, run_predicate_command));

    Print(d);
    printf("\n\n");
}
