#include "Ddmin.hpp"
#include <algorithm> // min, max


static void Ddmin(AlgorithmParams params, std::vector<Unit> units, std::vector<Unit> &removed_units, size_t num_partitions) {
    std::vector<std::vector<Unit>> partitions = Partition(units, num_partitions);
    for (size_t i = 0; i < partitions.size(); ++i) {
        Disable(partitions[i]);
    }

    for (size_t i = 0; i < partitions.size(); ++i) {
        Enable(partitions[i]);
        TreeWriteToFile(params.tree, params.c_file_name);
        if (system(params.run_predicate.c_str()) == 0) {
            // All partitions except partitions[i] is removed
            size_t j = 0;
            for (; j < i; ++j) {
                removed_units.insert(removed_units.end(), partitions[j].begin(), partitions[j].end());
            }

            j += 1;
            for (; j < partitions.size(); ++j) {
                removed_units.insert(removed_units.end(), partitions[j].begin(), partitions[j].end());
            }

            if (partitions[i].size() > 1) {
                Ddmin(params, removed_units, partitions[i], 2);
            }

            return;
        }

        Disable(partitions[i]);
    }

    for (size_t i = 0; i < partitions.size(); ++i) {
        Enable(partitions[i]);
    }

    if (num_partitions > 2) {
        for (size_t i = 0; i < partitions.size(); ++i) {
            Disable(partitions[i]);
            TreeWriteToFile(params.tree, params.c_file_name);
            if (system(params.run_predicate.c_str()) == 0) {
                // Make a new std::vector<Unit> containing all except partitions[i]
                std::vector<Unit> new_units;
                size_t j = 0;
                for (; j < i; ++j) {
                    new_units.insert(new_units.end(), partitions[j].begin(), partitions[j].end());
                }

                j += 1;
                for (; j < partitions.size(); ++j) {
                    new_units.insert(new_units.end(), partitions[j].begin(), partitions[j].end());
                }

                removed_units.insert(removed_units.end(), partitions[i].begin(), partitions[i].end());
                Ddmin(params, new_units, removed_units, std::max(num_partitions - 1, 2ul));
                return;
            }

            Enable(partitions[i]);
        }
    }

    if (num_partitions < units.size()) {
        Ddmin(params, units, removed_units, std::min(units.size(), 2 * num_partitions));
        return;
    }

    TreeWriteToFile(params.tree, params.c_file_name);
}

void Ddmin(AlgorithmParams params, std::vector<Ast **> units, std::vector<Unit> &removed_units) {
    std::vector<Unit> ast_units;
    for (size_t i = 0; i < units.size(); ++i) {
        Unit unit;
        unit.ast_value = *units[i];
        unit.ast_in_tree = units[i];

        ast_units.push_back(unit);
    }

    Ddmin(params, ast_units, removed_units, 2);
}
