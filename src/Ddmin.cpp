#include "Ddmin.hpp"
#include "Tree.hpp"
#include <algorithm> // min, max
#include <cassert>


static void Ddmin(AlgorithmParams params, std::vector<Unit> units, std::vector<Unit> &removed_units, size_t num_partitions) {
    if (units.size() == 1) {
        Disable(units);
        TreeWriteToFile(params.tree, params.c_file_name);
        if (system(params.run_predicate.c_str()) == 0) {
            removed_units.push_back(units[0]);
        }
        else {
            Enable(units);
            TreeWriteToFile(params.tree, params.c_file_name);
        }

        return;
    }

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

            j += 1; // Skip partitions[i] since it is not removed
            for (; j < partitions.size(); ++j) {
                removed_units.insert(removed_units.end(), partitions[j].begin(), partitions[j].end());
            }

            Ddmin(params, partitions[i], removed_units, 2);
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

                j += 1; // Skip partitions[i] since it is removed
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
    assert(units.size() > 0);
    std::vector<Unit> ast_units = ToUnits(units);
    Ddmin(params, ast_units, removed_units, 2);
}

void Ddmin(AlgorithmParams params, std::vector<Unit> units, std::vector<Unit> &removed_units) {
    assert(units.size() > 0);
    Ddmin(params, units, removed_units, 2);
}
