#include "GeneralizedBinaryReduction.hpp"
#include <algorithm>
#include <cassert>
#include <cstring>
#include <fstream>
#include <regex>
#include <stack>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>


// When adding new structs to reduce, search for '@remember' and update

// The enums and structs are named differently than in
// 'AstNodes.hpp' to avoid problems (Which did occur...)
enum NodeType {
    NODE_LEAF,
    NODE_BRANCH,
    NODE_CALL_EXPR,
    NODE_FUNCTION_CODE,
    NODE_FUNCTION_DEF,
    NODE_IF_STATEMENT,
    NODE_FOR_LOOP,
    NODE_VARIABLE_ASSIGNMENT,
};

struct Node {
    NodeType type;
    bool is_active = true;
};

struct LeafNode : public Node {
    LeafNode() { type = NODE_LEAF; }
    char *pre_value;
    char *value;
};

struct BranchNode : public Node {
    BranchNode() { type = NODE_BRANCH; }
    std::vector<Node *> children;
};



// @remember: add struct

// DISABLED:
// Running time goes from 4s to 90s and a problem occurs:
// e.g. int x = 2 + func_call();
// the func_call() CallExpr will not be dependant on the int x = 2... but
// on the function code, meaning it can remove the func_call(); while the rest remains.
//
// Code in function definition with identifier 'A' calling function definition with identifier 'B'
// [A()!code$B()]
// Dependencies
// - [A()!code$B()] => [A()!code]
// - [A()!code$B()] => [B()]
struct CallExpr : public Node {
    CallExpr() { type = NODE_CALL_EXPR; }
    std::vector<Node *> dependencies;
    std::vector<Node *> children;
    char *identifier;
};

// Code belonging to a function definition with identifier 'A'
// [A()!code]
// Dependencies:
// - [A()!code] => [A()]
struct FunctionCode : public Node {
    FunctionCode() { type = NODE_FUNCTION_CODE; }
    std::vector<Node *> dependencies; // Nodes that this FunctionCode depends on
    std::vector<Node *> dependers;
    std::vector<Node *> children;
};

// Function definition with identifier 'A'
// [A()]
// Has no dependencies
struct FunctionDef : public Node {
    FunctionDef() { type = NODE_FUNCTION_DEF; }
    std::vector<Node *> dependers; // Nodes that depend on this FunctionDef
    char *identifier;
    char *value;
    FunctionCode *code;
};

struct IfStatement : public Node {
    IfStatement() { type = NODE_IF_STATEMENT; }
    std::vector<Node *> children;
    std::vector<Node *> dependencies;
};

struct ForLoop : public Node {
    ForLoop() { type = NODE_FOR_LOOP; }
    std::vector<Node *> children;
    std::vector<Node *> dependencies;
};

struct VariableAssignment : public Node {
    VariableAssignment() { type = NODE_VARIABLE_ASSIGNMENT; }
    std::vector<Node *> children;
    std::vector<Node *> dependencies;
    char *identifier;
};


static Node *InitAst(TSNode ts_node, const char *source_code, uint32_t &prev_end_byte);


// static void PrintXml(Node *node) {
//     if (!node->is_active) return;
//     static int indent = 0;
//     for (int i = 0; i < indent; ++i) printf(" ");
//     if (node->type == NODE_LEAF) {
//         LeafNode *l = static_cast<LeafNode *>(node);
//         printf("<Leaf pre_value=\"%s\" value=\"%s\"/>\n", l->pre_value, l->value);
//     }
//     else if (node->type == NODE_BRANCH) {
//         assert(node->type == NODE_BRANCH);
//         BranchNode *b = static_cast<BranchNode *>(node);
//         printf("<Branch>\n");
//         indent += 4;
//         for (size_t i = 0; i < b->children.size(); ++i) {
//             PrintXml(b->children[i]);
//         }

//         indent -= 4;
//         for (int i = 0; i < indent; ++i) printf(" ");
//         printf("<Branch/>\n");
//     }
//     else if (node->type == NODE_CALL_EXPR) {
//         CallExpr *c = static_cast<CallExpr *>(node);
//         printf("<CallExpr identifier=\"%s\">\n", c->identifier);
//         indent += 4;
//         for (size_t i = 0; i < c->children.size(); ++i) {
//             PrintXml(c->children[i]);
//         }

//         indent -= 4;
//         for (int i = 0; i < indent; ++i) printf(" ");
//         printf("<CallExpr/>\n");
//     }
//     else if (node->type == NODE_FUNCTION_CODE) {
//         FunctionCode *f = static_cast<FunctionCode *>(node);
//         printf("<FunctionCode>\n");
//         indent += 4;
//         for (size_t i = 0; i < f->children.size(); ++i) {
//             PrintXml(f->children[i]);
//         }

//         indent -= 4;
//         for (int i = 0; i < indent; ++i) printf(" ");
//         printf("<FunctionCode/>\n");
//     }
//     else if (node->type == NODE_FUNCTION_DEF) {
//         FunctionDef *f = static_cast<FunctionDef *>(node);
//         printf("<FunctionDef identifier=\"%s\" value=\"%s\">\n", f->identifier, f->value);
//         indent += 4;
//         PrintXml(static_cast<Node *>(f->code));
//         indent -= 4;
//         for (int i = 0; i < indent; ++i) printf(" ");
//         printf("<FunctionDef/>\n");
//     }
//     else if (node->type == NODE_IF_STATEMENT) {
//         IfStatement *f = static_cast<IfStatement *>(node);
//         printf("<IfStatement>\n");
//         indent += 4;
//         for (size_t i = 0; i < f->children.size(); ++i) {
//             PrintXml(f->children[i]);
//         }

//         indent -= 4;
//         for (int i = 0; i < indent; ++i) printf(" ");
//         printf("<IfStatement/>\n");
//     }
// }

static void PrintXml(TSNode node, const char *source_code) {
   static int indent = 0;
   for (int i = 0; i < indent; ++i) printf(" ");
   const char *node_type = ts_node_type(node);
   uint32_t num_children = ts_node_child_count(node);
   if (num_children == 0) {
       uint32_t start_byte = ts_node_start_byte(node);
       uint32_t end_byte = ts_node_end_byte(node);
       uint32_t num_bytes = end_byte - start_byte;
       char *node_value = new char[num_bytes + 1];
       node_value[num_bytes] = '\0';
       strncpy(node_value, source_code + start_byte, num_bytes);
       printf("<%s value=\"%s\"/>\n", node_type, node_value);
   }
   else {
       printf("<%s>\n", node_type);
       indent += 4;
       for (uint32_t i = 0; i < num_children; ++i) {
           TSNode child = ts_node_child(node, i);
           PrintXml(child, source_code);
       }

       indent -= 4;
       for (int i = 0; i < indent; ++i) printf(" ");
       printf("<%s/>\n", node_type);
   }
}

static void WriteToFile(Node *node, std::ofstream &ofstream) {
    if (!node->is_active) {
        return;
    }

    // @remember: add to switch
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
            ofstream << "{";
            WriteToFile(f->code, ofstream);
            ofstream << "\n}\n";
        } break;
        case NODE_IF_STATEMENT: {
            IfStatement *i = static_cast<IfStatement *>(node);
            for (auto child : i->children) {
                WriteToFile(child, ofstream);
            }
        } break;
        case NODE_FOR_LOOP: {
            ForLoop *i = static_cast<ForLoop *>(node);
            for (auto child : i->children) {
                WriteToFile(child, ofstream);
            }
        } break;
        case NODE_VARIABLE_ASSIGNMENT: {
            VariableAssignment *i = static_cast<VariableAssignment *>(node);
            for (auto child : i->children) {
                WriteToFile(child, ofstream);
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

static char *FindAndReplaceAll(char *str, const char *from, const char *to) {
    std::string s(str);
    std::string f(from);
    std::string t(to);
    auto reg = std::regex(f);
    std::string result = std::regex_replace(s, reg, t);
    if (result != std::string(str)) {
        while (true) {
            std::string new_result = std::regex_replace(result, reg, t);
            if (new_result == result) {
                break;
            }

            result = new_result;
        }
    }

    return DuplicateString(result.c_str(), 0, static_cast<uint32_t>(result.length()));
}

static char *FindAndErase(char *str, char c) {
    std::string s(str);
    s.erase(std::remove(s.begin(), s.end(), c), s.end());
    return DuplicateString(s.c_str(), 0, static_cast<uint32_t>(s.length()));
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

    return { };
}

static CallExpr *InitCallExpr(TSNode ts_node, const char *source_code, uint32_t &prev_end_byte) {
    CallExpr *call_expr = new CallExpr;
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

static IfStatement *InitIfStatement(TSNode ts_node, const char *source_code, uint32_t &prev_end_byte) {
    IfStatement *if_statement = new IfStatement;
    uint32_t num_children = ts_node_child_count(ts_node);
    for (uint32_t i = 0; i < num_children; ++i) {
        TSNode ts_child = ts_node_child(ts_node, i);
        Node *child = InitAst(ts_child, source_code, prev_end_byte);
        if_statement->children.push_back(child);
    }

    return if_statement;
}

static ForLoop *InitForLoop(TSNode ts_node, const char *source_code, uint32_t &prev_end_byte) {
    ForLoop *for_loop = new ForLoop;
    uint32_t num_children = ts_node_child_count(ts_node);
    for (uint32_t i = 0; i < num_children; ++i) {
        TSNode ts_child = ts_node_child(ts_node, i);
        Node *child = InitAst(ts_child, source_code, prev_end_byte);
        for_loop->children.push_back(child);
    }

    return for_loop;
}

static TSNode FindVariableAssignmentIdentifierNode(TSNode node, const char *source_code) {
    const char *type = ts_node_type(node);
    if (strcmp(type, "identifier") == 0) {
        return node;
    }
    else if (strcmp(type, "assignment_expression") == 0 ||
             strcmp(type, "subscript_expression") == 0 ||
             strcmp(type, "field_expression") == 0) {
        return FindVariableAssignmentIdentifierNode(ts_node_child(node, 0), source_code);
    }
    else {
        PrintXml(node, source_code);
        printf("%s\n", type);
        assert(false);
        return {};
    }
}

static char *FindVariableAssignmentIdentifier(TSNode node, const char *source_code) {
}

static VariableAssignment *InitVariableAssignment(TSNode ts_node, const char *source_code, uint32_t &prev_end_byte) {
    VariableAssignment *variable_assignment = new VariableAssignment;
    TSNode n = FindVariableAssignmentIdentifierNode(ts_node_child(ts_node, 0), source_code);
    variable_assignment->identifier = GetValue(n, source_code);

    uint32_t num_children = ts_node_child_count(ts_node);
    for (uint32_t i = 0; i < num_children; ++i) {
        TSNode ts_child = ts_node_child(ts_node, i);
        Node *child = InitAst(ts_child, source_code, prev_end_byte);
        variable_assignment->children.push_back(child);
    }

    return variable_assignment;
}

static FunctionCode *InitFunctionCode(TSNode ts_node, const char *source_code, uint32_t &prev_end_byte) {
    FunctionCode *function_code = new FunctionCode;

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

    TSNode function_declarator = FindChild(ts_node, "function_declarator");
    assert(!ts_node_is_null(function_declarator));
    TSNode identifier = FindChild(function_declarator, "identifier");
    if (ts_node_is_null(identifier)) {
        TSNode decl = FindChild(function_declarator, "parenthesized_declarator");
        assert(!ts_node_is_null(decl));
        identifier = FindChild(decl, "identifier");
    }

    assert(!ts_node_is_null(identifier));
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
    // @remember parse it
    const char *node_type = ts_node_type(ts_node);
    if (strcmp(node_type, "if_statement") == 0) {
        return static_cast<Node *>(InitIfStatement(ts_node, source_code, prev_end_byte));
    }

    if (strcmp(node_type, "for_statement") == 0) {
        return static_cast<Node *>(InitForLoop(ts_node, source_code, prev_end_byte));
    }

    if (strcmp(node_type, "function_definition") == 0) {
        return static_cast<Node *>(InitFunctionDef(ts_node, source_code, prev_end_byte));
    }

    if (strcmp(node_type, "expression_statement") == 0) {
        TSNode c1 = ts_node_child(ts_node, 0);
        const char *t1 = ts_node_type(c1);
        if (strcmp(t1, "call_expression") == 0) {
            return static_cast<Node *>(InitCallExpr(ts_node, source_code, prev_end_byte));
        }

        if (strcmp(t1, "assignment_expression") == 0) {
            return static_cast<Node *>(InitVariableAssignment(ts_node, source_code, prev_end_byte));
            // TSNode c2 = ts_node_child(c1, 0);
            // const char *t2 = ts_node_type(c2);
            // if (strcmp(t2, "identifier") == 0) {
            //     return static_cast<Node *>(InitVariableAssignment(ts_node, source_code, prev_end_byte));
            // }
        }
    }

    uint32_t num_children = ts_node_child_count(ts_node);
    if (num_children == 0) {
        LeafNode *leaf_node = new LeafNode;

        uint32_t start_byte = ts_node_start_byte(ts_node);
        uint32_t end_byte = ts_node_end_byte(ts_node);
        char *pre_value = DuplicateString(source_code, prev_end_byte, start_byte);
        uint32_t num_bytes_in_pre_value = start_byte - prev_end_byte;
        if (num_bytes_in_pre_value > 0) {
            leaf_node->pre_value = FindAndErase(pre_value, '}');
            leaf_node->pre_value = FindAndReplaceAll(leaf_node->pre_value, "\n\n", "\n");
        }
        else {
            leaf_node->pre_value = pre_value;
        }

        leaf_node->value = DuplicateString(source_code, start_byte, end_byte);
        prev_end_byte = end_byte;
        return static_cast<Node *>(leaf_node);
    }

    BranchNode *branch_node = new BranchNode;
    for (uint32_t i = 0; i < num_children; ++i) {
        TSNode ts_child = ts_node_child(ts_node, i);
        Node *child = InitAst(ts_child, source_code, prev_end_byte);
        branch_node->children.push_back(child);
    }

    return static_cast<Node *>(branch_node);
}

static std::vector<Node *> *GetDependencies(Node *node) {
    // @remember: add to switch statement
    switch (node->type) {
        case NODE_FUNCTION_CODE: {
            return &static_cast<FunctionCode *>(node)->dependencies;
        }
        case NODE_CALL_EXPR: {
            return &static_cast<CallExpr *>(node)->dependencies;
        }
        case NODE_IF_STATEMENT: {
            return &static_cast<IfStatement *>(node)->dependencies;
        }
        case NODE_FOR_LOOP: {
            return &static_cast<ForLoop *>(node)->dependencies;
        }
        case NODE_VARIABLE_ASSIGNMENT: {
            return &static_cast<VariableAssignment *>(node)->dependencies;
        }
        default: return NULL;
    }
}

static void AddDependencies(std::vector<Node *> &list, Node *node) {
    auto dependencies = GetDependencies(node);
    if (dependencies != NULL) {
        for (auto dependency : *dependencies) {
            list.push_back(dependency);
            AddDependencies(list, dependency);
        }
    }
}

static void FindDependencies(Node *node, std::vector<Node *> &nodes_to_reduce) {
    // @remember: add dependency and add to nodes_to_reduce

    std::vector<Node *> function_defs;
    FindChildren(node, NODE_FUNCTION_DEF, function_defs);

    for (size_t i = 0; i < function_defs.size(); ++i) {
        FunctionDef *f = static_cast<FunctionDef *>(function_defs[i]);

        nodes_to_reduce.push_back(function_defs[i]);
        nodes_to_reduce.push_back(static_cast<Node *>(f->code));

        // Syntactic dependency: [A()!code] => [A()]
        f->code->dependencies.push_back(static_cast<Node *>(f));
        f->dependers.push_back(static_cast<Node *>(f->code));
        // printf("[A()!code] => [A()]: A: %s\n", f->identifier);

        std::vector<Node *> if_statements;
        FindChildren(static_cast<Node *>(f), NODE_IF_STATEMENT, if_statements);
        for (auto if_statement : if_statements) {
            IfStatement *is = static_cast<IfStatement *>(if_statement);
            is->dependencies.push_back(static_cast<Node *>(f->code));
            nodes_to_reduce.push_back(if_statement);
            f->code->dependers.push_back(if_statement);
        }

        std::vector<Node *> for_loops;
        FindChildren(static_cast<Node *>(f), NODE_FOR_LOOP, for_loops);
        for (auto for_loop : for_loops) {
            ForLoop *fl = static_cast<ForLoop *>(for_loop);
            fl->dependencies.push_back(static_cast<Node *>(f->code));
            nodes_to_reduce.push_back(for_loop);
            f->code->dependers.push_back(for_loop);
        }

        std::vector<Node *> variable_assignments;
        FindChildren(static_cast<Node *>(f), NODE_VARIABLE_ASSIGNMENT, variable_assignments);
        for (auto variable_assignment : variable_assignments) {
            nodes_to_reduce.push_back(variable_assignment);
            VariableAssignment *n = static_cast<VariableAssignment *>(variable_assignment);
            n->dependencies.push_back(static_cast<Node *>(f->code));
        }

        std::vector<Node *> call_exprs;
        FindChildren(static_cast<Node *>(f->code), NODE_CALL_EXPR, call_exprs);
        for (size_t j = 0; j < call_exprs.size(); ++j) {
            nodes_to_reduce.push_back(call_exprs[j]);
            CallExpr *c = static_cast<CallExpr *>(call_exprs[j]);

            // Syntactic dependency: [A()!code$B()] => [A()!code]
            // Before adding the dependency, check if 'f->code' already has a depender
            // that is of type NODE_CALL_EXPR and has identifier c->identifier
            bool is_already_added = false;
            for (size_t k = 0; k < f->code->dependers.size(); ++k) {
                if (f->code->dependers[k]->type == NODE_CALL_EXPR) {
                    auto n = static_cast<CallExpr *>(f->code->dependers[k]);
                    if (strcmp(n->identifier, c->identifier) == 0) {
                        is_already_added = true;
                        break;
                    }
                }
            }

            if (!is_already_added) {
                f->code->dependers.push_back(static_cast<Node *>(c));
                c->dependencies.push_back(static_cast<Node *>(f->code));
                // printf("[A()!code$B()] => [A()!code]: A: %s        B: %s\n", f->identifier, c->identifier);
            }

            // Referential semantic dependency: [A()!code$B()] => [B()]
            for (size_t k = 0; k < function_defs.size(); ++k) {
                FunctionDef *f2 = static_cast<FunctionDef *>(function_defs[k]);

                if (strcmp(c->identifier, f2->identifier) == 0) {
                    // Before adding the dependency, check if 'f2->code' already has a depender
                    // that is of type NODE_CALL_EXPR and has identifier c->identifier
                    is_already_added = false;
                    for (size_t w = 0; w < f2->dependers.size(); ++w) {
                        if (f2->dependers[w]->type == NODE_CALL_EXPR) {
                            auto n = static_cast<CallExpr *>(f2->dependers[w]);
                            if (strcmp(n->identifier, c->identifier) == 0) {
                                is_already_added = true;
                                break;
                            }
                        }
                    }

                    if (!is_already_added) {
                        f2->dependers.push_back(static_cast<Node *>(c));
                        c->dependencies.push_back(static_cast<Node *>(f2));
                        // printf("[A()!code$B()] => [B()]: A: %s        B: %s\n", f->identifier, c->identifier);
                    }

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
            auto dependencies = GetDependencies(node);
            if (dependencies != NULL) {
                for (auto d : *dependencies) {
                    stack.push(std::make_tuple(d, false));
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
    size_t min_idx = 0;
    size_t max_idx = d.size() - 1;
    while (min_idx < max_idx) {
        size_t mid_idx = (min_idx + max_idx) >> 1;
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
            max_idx = mid_idx;
        }
        else {
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
    // for (size_t i = 0; i < d.size(); ++i) {
    //     auto x = d[i];
    //     printf("==== d[%zu]\n", i);
    //     for (size_t j = 0; j < x.size(); ++j) {
    //         auto y = x[j];
    //         printf("%zu - %d ", j, y->is_active);
    //         switch (y->type) {
    //             case NODE_FUNCTION_CODE: {
    //                 auto n = static_cast<FunctionCode *>(y);
    //                 auto f = static_cast<FunctionDef *>(n->dependencies[0]);
    //                 printf("code: %s\n", f->identifier);
    //             } break;
    //             case NODE_FUNCTION_DEF: {
    //                 auto f = static_cast<FunctionDef *>(y);
    //                 printf("func: %s\n", f->identifier);
    //             } break;
    //             case NODE_CALL_EXPR: {
    //                 auto e = static_cast<CallExpr *>(y);
    //                 printf("call: %s\n", e->identifier);
    //             } break;
    //             case NODE_IF_STATEMENT: {
    //                 printf("ifst:\n");
    //             } break;
    //         }
    //     }

    //     printf("\n");
    // }
    do {
        size_t idx = FindSmallestSuccessfulIdx(root_node, file_name, run_predicate_command, d);
        // It works with d[0] and the search is finished
        if (idx == 0) {
            break;
        }

        for (auto node : d[idx]) {
            learned_set.push_back(node);
            auto dependencies = GetDependencies(node);
            if (dependencies != NULL) {
                for (auto dependency : *dependencies) {
                    learned_set.push_back(dependency);
                }
            }
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
}
