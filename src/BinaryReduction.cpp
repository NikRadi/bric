#include "BinaryReduction.hpp"
#include "FileHandling.hpp"
#include <assert.h>
#include <stdio.h>


void BinaryReduction(Tree &tree, std::string unit_ts_type, std::string run_predicate_command, std::string file_name) {
    std::vector<Ast *> units;
    TreeFindNodes(tree, units, unit_ts_type);
    printf("Found %ld units\n", units.size());

    std::vector<std::vector<Ast *>> sets_of_units;
    for (size_t i = 0; i < units.size(); ++i) {
        std::vector<Ast *> set_of_units;
        for (size_t j = 0; j <= i; ++j) {
            set_of_units.push_back(units[j]);
        }

        sets_of_units.push_back(set_of_units);
    }

    std::vector<Ast *> units_to_keep_activated;
    int num_tests = 0;
    size_t low_idx = 0;
    size_t high_idx = sets_of_units.size();
    while (true) {
        printf("Test %d\n", num_tests);
        num_tests += 1;
        size_t middle_idx = (low_idx + high_idx) / 2;
        for (size_t i = middle_idx + 1; i < sets_of_units.size(); ++i) {
            SetIsActive(sets_of_units[i], false);
        }

        for (size_t i = 0; i < middle_idx + 1; ++i) {
            SetIsActive(sets_of_units[i], true);
        }

        SetIsActive(units_to_keep_activated, true);
        TreeWriteToFile(tree, file_name, "w");
        int return_code = system(run_predicate_command.c_str());
        if (return_code == 0) {
            printf("Success on unit %ld\n", middle_idx);
            Ast *unit = sets_of_units[middle_idx].back();
            units_to_keep_activated.push_back(unit);
            low_idx = 0;
            high_idx = middle_idx;
        }
        else {
            low_idx = middle_idx;
        }

        if (middle_idx == 0) {
            break;
        }
    }

    SetIsActive(units, false);
    SetIsActive(units_to_keep_activated, true);
    TreeWriteToFile(tree, file_name, "w");
}
