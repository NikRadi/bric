#include "BinaryReduction.hpp"
#include "Tree.hpp"
#include <cassert>


void BinaryReduction(AlgorithmParams params, std::vector<Ast **> units, std::vector<Unit> &removed_units) {
    BinaryReduction(params, ToUnits(units), removed_units);
}

void BinaryReduction(AlgorithmParams params, std::vector<Unit> units, std::vector<Unit> &removed_units) {
    std::vector<std::vector<Unit>> sets_of_units;
    for (size_t i = 0; i < units.size(); ++i) {
        std::vector<Unit> set_of_units;
        for (size_t j = 0; j <= i; ++j) {
            set_of_units.push_back(units[j]);
        }

        sets_of_units.push_back(set_of_units);
    }

    std::vector<Unit> units_to_keep_enabled;
    int low_idx = 0;
    int high_idx = sets_of_units.size() - 1;
    int last_idx_that_worked = high_idx;
    while (low_idx <= high_idx) {
        int middle_idx = (low_idx + high_idx) / 2;
//        printf("%d %d %d\n", low_idx, middle_idx, high_idx);
        for (size_t i = middle_idx + 1; i < sets_of_units.size(); ++i) {
            Disable(sets_of_units[i]);
        }

        for (size_t i = 0; i <= static_cast<size_t>(middle_idx); ++i) {
            Enable(sets_of_units[i]);
        }

        Enable(units_to_keep_enabled);
        TreeWriteToFile(params.tree, params.c_file_name);
        if (system(params.run_predicate.c_str()) == 0) {
//            printf("worked\n");
            if (low_idx == high_idx) {
                units_to_keep_enabled.push_back(units[middle_idx]);
                Disable(units);
                Enable(units_to_keep_enabled);
                TreeWriteToFile(params.tree, params.c_file_name);
                if (system(params.run_predicate.c_str()) == 0) {
                    break;
                }

                low_idx = 0;
            }

            high_idx = middle_idx - 1;
            last_idx_that_worked = middle_idx;
        }
        else {
//            printf("did not work\n");
            if (low_idx == high_idx) {
                units_to_keep_enabled.push_back(units[last_idx_that_worked]);
                high_idx = last_idx_that_worked - 1;
                low_idx = 0;
            }
            else {
                low_idx = middle_idx + 1;
            }
        }
    }

    for (size_t i = 0; i < units.size(); ++i) {
        if (IsDisabled(units[i])) {
            removed_units.push_back(units[i]);
        }
    }

    assert(system(params.run_predicate.c_str()) == 0);
}
