#include "AlgorithmGeneral.hpp"


Unit ToUnit(Ast **ast) {
    Unit unit;
    unit.ast_value = *ast;
    unit.ast_in_tree = ast;
    return unit;
}

std::vector<Unit> ToUnits(std::vector<Ast **> asts) {
    std::vector<Unit> units;
    for (size_t i = 0; i < asts.size(); ++i) {
//        Unit unit;
//        unit.ast_value = *asts[i];
//        unit.ast_in_tree = asts[i];
        Unit unit = ToUnit(asts[i]);
        units.push_back(unit);
    }

    return units;
}

std::vector<std::vector<Unit>> Partition(std::vector<Unit> units, size_t num_partitions) {
    std::vector<std::vector<Unit>> partitions;
    std::vector<size_t> num_items_in_partition(num_partitions, 0);
    size_t num_items_in_each_partition = units.size() / num_partitions;
    size_t num_leftover_items = units.size() % num_partitions;
    for (size_t i = 0; i < num_partitions; ++i) {
        num_items_in_partition[i] = num_items_in_each_partition;
    }

    for (size_t i = 0; i < num_leftover_items; ++i) {
        num_items_in_partition[i] += 1;
    }

    int num_items_added = 0;
    for (size_t i = 0; i < num_partitions; ++i) {
        std::vector<Unit> partition;
        for (size_t j = num_items_added; j < num_items_added + num_items_in_partition[i]; ++j) {
            partition.push_back(units[j]);
        }

        num_items_added += num_items_in_partition[i];
        partitions.push_back(partition);
    }

    return partitions;
}

void Enable(Unit unit) {
    *unit.ast_in_tree = unit.ast_value;
}

void Enable(std::vector<Unit> partition) {
    for (size_t i = 0; i < partition.size(); ++i) {
        Enable(partition[i]);
//        Unit unit = partition[i];
//        *unit.ast_in_tree = unit.ast_value;
    }
}

void Disable(Unit unit) {
    *unit.ast_in_tree = NULL;
}

void Disable(std::vector<Unit> partition) {
    for (size_t i = 0; i < partition.size(); ++i) {
        Disable(partition[i]);
//        Unit unit = partition[i];
//        *unit.ast_in_tree = NULL;
    }
}

bool IsDisabled(Unit unit) {
    return *unit.ast_in_tree == NULL;
}
