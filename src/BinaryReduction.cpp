#include "BinaryReduction.hpp"
#include <cassert>


void BinaryReduction(AlgorithmParams params, std::vector<Ast **> units, std::vector<Unit> &removed_units) {
    std::vector<Unit> ast_units;
    for (size_t i = 0; i < units.size(); ++i) {
        Unit unit;
        unit.ast_value = *units[i];
        unit.ast_in_tree = units[i];

        ast_units.push_back(unit);
    }

    std::vector<std::vector<Unit>> sets_of_units;
    for (size_t i = 0; i < ast_units.size(); ++i) {
        std::vector<Unit> set_of_units;
        for (size_t j = 0; j <= i; ++j) {
            set_of_units.push_back(ast_units[j]);
        }

        sets_of_units.push_back(set_of_units);
    }

    std::vector<Unit> units_to_keep_enabled;
    size_t low_idx = 0;
    size_t high_idx = sets_of_units.size();
    while (true) {
        size_t middle_idx = (low_idx + high_idx) / 2;
        for (size_t i = middle_idx + 1; i < sets_of_units.size(); ++i) {
            Disable(sets_of_units[i]);
        }

        for (size_t i = 0; i <= middle_idx; ++i) {
            Enable(sets_of_units[i]);
        }

        Enable(units_to_keep_enabled);
        TreeWriteToFile(params.tree, params.c_file_name);
        if (system(params.run_predicate.c_str()) == 0) {
            low_idx = 0;
            high_idx = middle_idx;
            units_to_keep_enabled.push_back(ast_units[middle_idx]);

            Disable(ast_units);
            Enable(units_to_keep_enabled);
            TreeWriteToFile(params.tree, params.c_file_name);
            if (system(params.run_predicate.c_str()) == 0) {
                break;
            }
        }
        else {
            low_idx = middle_idx;
        }

        if (middle_idx == 0) {
            break;
        }
    }

    for (size_t i = 0; i < ast_units.size(); ++i) {
        if (IsDisabled(ast_units[i])) {
            removed_units.push_back(ast_units[i]);
        }
    }
}
