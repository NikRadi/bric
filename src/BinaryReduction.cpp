#include "BinaryReduction.hpp"
#include <algorithm>
#include <stack>
#include <unordered_map>


// #define REDUCE_CLOSURES
#define REDUCE_UNITS


static std::string FindFunctionDefinitionIdentifier(Ast *f) {
    Ast *function_declarator = FindChild(f, "function_declarator");
    if (function_declarator == NULL) {
        Ast *p = FindChild(f, "pointer_declarator");
        function_declarator = FindChild(p, "function_declarator");
    }

    Ast *identifier = FindChild(function_declarator, "identifier");
    if (identifier == NULL) {
        Ast *p = FindChild(function_declarator, "parenthesized_declarator");
        identifier = FindChild(p, "identifier");
    }

    return identifier->value;
}

static void FindDependencies(std::vector<Ast *> function_definitions) {
    std::unordered_map<std::string, size_t> identifier_to_i;
    for (size_t i = 0; i < function_definitions.size(); ++i) {
        Ast *f = function_definitions[i];
        std::string identifier = FindFunctionDefinitionIdentifier(f);
        identifier_to_i.insert({ identifier, i });
    }

    for (size_t i = 0; i < function_definitions.size(); ++i) {
        Ast *f = function_definitions[i];
        std::string f_identifier = FindFunctionDefinitionIdentifier(f);
        Ast *compound_statement = FindChild(f, "compound_statement");

        std::vector<Ast *> call_expressions;
        FindNodes(compound_statement, "call_expression", call_expressions);
        for (auto c : call_expressions) {
            std::string identifier = c->children[0]->value;
            // Is the function calling itself recursively?
            if (identifier != f_identifier) {
                auto idx = identifier_to_i.find(identifier);
                // Do we have the function definition? Else don't reduce it
                if (idx != identifier_to_i.end()) {
                    auto exists = std::find(f->dependencies.begin(), f->dependencies.end(), idx->second);
                    // Does the dependency already exist?
                    if (exists == f->dependencies.end()) {
                        f->dependencies.push_back(idx->second);
                    }
                }

            }

        }
    }
}

static std::vector<std::vector<Ast *>> FindClosures(const std::vector<Ast *> &units) {
    std::vector<std::vector<Ast *>> result;
    for (auto unit : units) {
        std::vector<Ast *> c;
        c.push_back(unit);
        for (auto idx : unit->dependencies) {
            c.push_back(units[idx]);
        }

        std::sort(c.begin(), c.end());
        auto it = std::find(result.begin(), result.end(), c);
        if (it == result.end()) {
            result.push_back(c);
        }
    }

    return result;
}

#ifdef REDUCE_CLOSURES
static size_t FindSmallestPrefix(std::vector<std::vector<Ast *>> &scc, size_t max_idx, Ast *r, std::string f, std::string p) {
    size_t min_idx = 0;
    while (min_idx < max_idx) {
        size_t mid_idx = (min_idx + max_idx) >> 1;
        for (size_t i = mid_idx + 1; i <= max_idx; ++i) {
            for (auto unit : scc[i]) {
                unit->is_active = false;
            }
        }

        for (size_t i = 0; i <= mid_idx; ++i) {
            for (auto unit : scc[i]) {
                unit->is_active = true;
            }
        }

        if (IsTestSuccessful(r, f, p)) {
            max_idx = mid_idx;
        }
        else {
            min_idx = mid_idx + 1;
        }
    }

    return min_idx;
}
#endif // REDUCE_CLOSURES
#ifdef REDUCE_UNITS
static size_t FindSmallestPrefix(std::vector<Ast *> &units, size_t max_idx, Ast *r, std::string f, std::string p) {
    size_t min_idx = 0;
    while (min_idx < max_idx) {
        size_t mid_idx = (min_idx + max_idx) >> 1;
        for (size_t i = mid_idx + 1; i <= max_idx; ++i) {
            units[i]->is_active = false;
        }

        for (size_t i = 0; i <= mid_idx; ++i) {
            units[i]->is_active = true;
        }

        if (IsTestSuccessful(r, f, p)) {
            max_idx = mid_idx;
        }
        else {
            min_idx = mid_idx + 1;
        }
    }

    return min_idx;
}
#endif // REDUCE_UNITS

static bool Compare1(std::vector<Ast *> v1, std::vector<Ast *> v2) {
    return v1.size() < v2.size();
}

static bool Compare2(std::vector<Ast *> v1, std::vector<Ast *> v2) {
    int v1_size = 0;
    for (auto unit : v1) {
        v1_size += CountActive(unit);
    }

    int v2_size = 0;
    for (auto unit : v2) {
        v2_size += CountActive(unit);
    }

    return v1_size < v2_size;
}

void BinaryReduction(std::vector<Ast *> units, Ast *r, std::string f, std::string p) {
    FindDependencies(units);

#ifdef REDUCE_CLOSURES
    std::vector<std::vector<Ast *>> scc = FindClosures(units);
    std::sort(scc.begin(), scc.end(), Compare1);
    for (auto unit : units) {
        unit->is_active = false;
    }

    size_t max_idx = scc.size() - 1;
#endif // REDUCE_CLOSURES
#ifdef REDUCE_UNITS
    size_t max_idx = units.size() - 1;
#endif // REDUCE_UNITS


    std::vector<Ast *> result;
    while (true) {
        if (IsTestSuccessful(r, f, p)) {
            break;
        }

#ifdef REDUCE_CLOSURES
        max_idx = FindSmallestPrefix(scc, max_idx, r, f, p);
        for (auto unit : scc[max_idx]) {
            unit->is_active = true;
            result.push_back(unit);
        }
#endif
#ifdef REDUCE_UNITS
        max_idx = FindSmallestPrefix(units, max_idx, r, f, p);
        units[max_idx]->is_active = true;
        result.push_back(units[max_idx]);
#endif

        for (auto unit : units) {
            unit->is_active = false;
        }

        for (auto unit : result) {
            unit->is_active = true;
        }

        max_idx -= 1;
    }
}
