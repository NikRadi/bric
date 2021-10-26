#include "BinaryReduction.hpp"
#include <cassert>


void BinaryReduction(AlgorithmParams params, std::vector<Ast **> units, std::vector<Unit> &removed_units) {
    std::vector<Unit> ast_units = ToUnits(units);
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
    size_t high_idx = sets_of_units.size() - 1;
//    while (true) {
    for (int counter = 0; counter < 10; ++counter) {
        size_t middle_idx = (low_idx + high_idx) / 2;
        printf("%ld %ld %ld\n", low_idx, middle_idx, high_idx);
        for (size_t i = middle_idx + 1; i < sets_of_units.size(); ++i) {
            Disable(sets_of_units[i]);
        }

        for (size_t i = 0; i <= middle_idx; ++i) {
            Enable(sets_of_units[i]);
        }

        Enable(units_to_keep_enabled);
        TreeWriteToFile(params.tree, params.c_file_name);
        if (system(params.run_predicate.c_str()) == 0) {
            if (middle_idx == 0) {
                for (size_t i = 0; i <= middle_idx; ++i) {
                    Disable(sets_of_units[i]);
                }

                TreeWriteToFile(params.tree, params.c_file_name);
                if (system(params.run_predicate.c_str()) == 0) {
                    break;
                }

                printf("BINARY REDUCTION ERROR (%ld, %ld, %ld, %ld)\n",
                    low_idx, middle_idx, high_idx, units.size()
                );
            }

            high_idx = middle_idx;
        }
        else if (low_idx == middle_idx) {
            units_to_keep_enabled.push_back(ast_units[high_idx]);
            Disable(ast_units);
            Enable(units_to_keep_enabled);
            TreeWriteToFile(params.tree, params.c_file_name);
            if (system(params.run_predicate.c_str()) == 0) {
                break;
            }

            if (middle_idx == 0) {
                break;
            }

            low_idx = 0;
            high_idx = middle_idx;
        }
        else {
            low_idx = middle_idx;
        }
    }

    for (size_t i = 0; i < ast_units.size(); ++i) {
        if (IsDisabled(ast_units[i])) {
            removed_units.push_back(ast_units[i]);
        }
    }

    assert(system(params.run_predicate.c_str()) == 0);
}
