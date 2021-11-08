#include "BinaryReduction.hpp"
#include <algorithm>
#include <cassert>
#include <unordered_map>


static void FindDependencies(std::vector<Ast *> function_defs) {
    std::unordered_map<std::string, FunctionDef *> identifier_to_function;
    for (size_t i = 0; i < function_defs.size(); ++i) {
        Branch *function = static_cast<Branch *>(function_defs[i]);
        Ast *function_declarator = AstFindNodeInChildren(function, "function_declarator");
        Ast *identifier_leaf = AstFindNodeInChildren(static_cast<Branch *>(function_declarator), "identifier");
        const char *function_identifier = static_cast<Leaf *>(identifier_leaf)->value;
        identifier_to_function.insert({ function_identifier, static_cast<FunctionDef *>(function) });
    }

    for (size_t i = 0; i < function_defs.size(); ++i) {
        FunctionDef *function = static_cast<FunctionDef *>(function_defs[i]);
        Ast *compound_statement = AstFindNodeInChildren(static_cast<FunctionDef *>(function), "compound_statement");
        std::vector<Ast *> call_expressions;
        AstFindNodes(compound_statement, "call_expression", call_expressions);
        for (size_t j = 0; j < call_expressions.size(); ++j) {
            Branch *call_expression = static_cast<Branch *>(call_expressions[j]);
            Leaf *identifier = static_cast<Leaf *>(call_expression->children[0]);
            assert(strcmp(identifier->type, "identifier") == 0);
            auto function_def = identifier_to_function.find(identifier->value);
            if (function_def != identifier_to_function.end()) {
                function->dependencies.push_back(function_def->second);
            }
        }
    }
}

static void AddDependenciesToClosure(FunctionDef *function, std::vector<Ast *> &closure) {
    for (size_t i = 0; i < function->dependencies.size(); ++i) {
        for (size_t j = 0; j < closure.size(); ++j) {
            if (function->dependencies[i] == closure[j]) {
                return;
            }
        }

        closure.push_back(function->dependencies[i]);
        FunctionDef *dependency = static_cast<FunctionDef *>(function->dependencies[i]);
        if (dependency->dependencies.size() > 0) {
            AddDependenciesToClosure(dependency, closure);
        }
    }
}

static std::vector<std::vector<Ast *>> FindClosures(std::vector<Ast *> function_defs) {
    std::vector<std::vector<Ast *>> closures;
    for (size_t i = 0; i < function_defs.size(); ++i) {
        std::vector<Ast *> closure;
        FunctionDef *function = static_cast<FunctionDef *>(function_defs[i]);
        closure.push_back(function_defs[i]);
        if (function->dependencies.size() > 0) {
            AddDependenciesToClosure(function, closure);
        }

        bool is_closure_dublicate = false;
        std::sort(closure.begin(), closure.end());
        for (size_t j = 0; j < closures.size(); ++j) {
            if (closure == closures[j]) {
                is_closure_dublicate = true;
                break;
            }
        }

        if (!is_closure_dublicate) {
            closures.push_back(closure);
        }
    }

    return closures;
}

static bool IsPredicateSuccessful(Ast *root_node, const char *file_name, const char *run_predicate_command) {
    AstWriteToFile(root_node, file_name);
    return system(run_predicate_command) == 0;
}

void BinaryReduction(Ast *root_node, const char *file_name, const char *run_predicate_command, std::vector<Ast *> function_defs) {
    FindDependencies(function_defs);
    std::vector<std::vector<Ast *>> closures = FindClosures(function_defs);
    closures.push_back({ }); // closures[0] contains elements that we know must be active
    std::sort(closures.begin(), closures.end(), [](const std::vector<Ast *> &a, const std::vector<Ast *> &b) { return a.size() < b.size(); });
    for (size_t i = 0; i < closures.size(); ++i) {
        for (size_t j = 0; j < closures[i].size(); ++j) {
            closures[i][j]->flags &= ~AST_IS_ACTIVE;
        }
    }

    while (!IsPredicateSuccessful(root_node, file_name, run_predicate_command)) {
        size_t min_idx = 1;
        size_t max_idx = closures.size() - 1;
        size_t mid_idx = 0;
        while (min_idx < max_idx) {
            mid_idx = (min_idx + max_idx) >> 1;
            size_t i;
            for (i = min_idx; i <= mid_idx; ++i) {
                for (size_t j = 0; j < closures[i].size(); ++j) {
                    closures[i][j]->flags |= AST_IS_ACTIVE;
                }
            }

            for (i = mid_idx + 1; i < closures.size(); ++i) {
                for (size_t j = 0; j < closures[i].size(); ++j) {
                    closures[i][j]->flags &= ~AST_IS_ACTIVE;
                }
            }

            if (IsPredicateSuccessful(root_node, file_name, run_predicate_command)) {
                max_idx = mid_idx;
            }
            else {
                min_idx = mid_idx + 1;
            }
        }

        for (size_t i = 0; i < closures.size(); ++i) {
            if (i == min_idx) {
                for (size_t j = 0; j < closures[i].size(); ++j) {
                    closures[i][j]->flags |= AST_IS_ACTIVE;
                }
            }
            else {
                for (size_t j = 0; j < closures[i].size(); ++j) {
                    closures[i][j]->flags &= ~AST_IS_ACTIVE;
                }
            }
        }
    }
}
