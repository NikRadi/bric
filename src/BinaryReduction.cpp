#include "BinaryReduction.hpp"
#include <algorithm>
#include <cassert>
#include <cstring>
#include <unordered_map>


static void FindDependencies(std::vector<Ast *> function_defs) {
    std::unordered_map<std::string, Branch *> identifier_to_function;
    for (size_t i = 0; i < function_defs.size(); ++i) {
        Branch *function = static_cast<Branch *>(function_defs[i]);
        Ast *function_declarator = AstFindChild(function, "function_declarator");
        Ast *identifier_leaf = AstFindChild(static_cast<Branch *>(function_declarator), "identifier");
        if (identifier_leaf == NULL) {
            Ast *decl = AstFindChild(static_cast<Branch *>(function_declarator), "parenthesized_declarator");
            assert(decl != NULL);
            identifier_leaf = AstFindChild(static_cast<Branch *>(decl), "identifier");
        }

        assert(identifier_leaf != NULL);
        const char *function_identifier = static_cast<Leaf *>(identifier_leaf)->value;
        identifier_to_function.insert({ function_identifier, function });
    }

    for (size_t i = 0; i < function_defs.size(); ++i) {
        Branch *function = static_cast<Branch *>(function_defs[i]);
        Ast *compound_statement = AstFindChild(function, "compound_statement");
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

static void AddDependenciesToClosure(Branch *function, std::vector<Ast *> &closure) {
    for (size_t i = 0; i < function->dependencies.size(); ++i) {
        for (size_t j = 0; j < closure.size(); ++j) {
            if (function->dependencies[i] == closure[j]) {
                return;
            }
        }

        closure.push_back(function->dependencies[i]);
        Branch *dependency = static_cast<Branch *>(function->dependencies[i]);
        if (dependency->dependencies.size() > 0) {
            AddDependenciesToClosure(dependency, closure);
        }
    }
}

static std::vector<std::vector<Ast *>> FindClosures(std::vector<Ast *> function_defs) {
    std::vector<std::vector<Ast *>> closures;
    for (size_t i = 0; i < function_defs.size(); ++i) {
        std::vector<Ast *> closure;
        Branch *function = static_cast<Branch *>(function_defs[i]);
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

static size_t BinarySearch(AlgorithmParams params, std::vector<std::vector<Ast *>> closures, size_t max_idx) {
    size_t min_idx = 0;
    while (min_idx < max_idx) {
        size_t mid_idx = (min_idx + max_idx) >> 1;
        for (size_t i = min_idx; i <= mid_idx; ++i) {
            for (size_t j = 0; j < closures[i].size(); ++j) {
                closures[i][j]->flags |= AST_IS_ACTIVE;
            }
        }

        for (size_t i = mid_idx + 1; i <= max_idx; ++i) {
            for (size_t j = 0; j < closures[i].size(); ++j) {
                closures[i][j]->flags &= ~AST_IS_ACTIVE;
            }
        }

        if (IsPredicateSuccessful(params)) {
            max_idx = mid_idx;
        }
        else {
            min_idx = mid_idx + 1;
        }
    }

    return min_idx;
}

void BinaryReduction(AlgorithmParams params, std::vector<Ast *> function_defs) {
    FindDependencies(function_defs);
    std::vector<std::vector<Ast *>> closures = FindClosures(function_defs);
    std::vector<std::vector<Ast *>> final_set;
    std::sort(closures.begin(), closures.end(), [](const std::vector<Ast *> &a, const std::vector<Ast *> &b) { return a.size() < b.size(); });
    size_t max_idx = closures.size() - 1;
    while (max_idx > 0) {
        max_idx = BinarySearch(params, closures, max_idx);
        final_set.push_back(closures[max_idx]);
        for (size_t i = 0; i < closures.size(); ++i) {
            for (size_t j = 0; j < closures[i].size(); ++j) {
                closures[i][j]->flags &= ~AST_IS_ACTIVE;
            }
        }

        for (size_t i = 0; i < final_set.size(); ++i) {
            for (size_t j = 0; j < final_set[i].size(); ++j) {
                    final_set[i][j]->flags |= AST_IS_ACTIVE;
            }
        }

        if (IsPredicateSuccessful(params)) {
            break;
        }

        max_idx -= 1;
    }
}
