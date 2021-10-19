#include "Hdd.hpp"
#include "BinaryReduction.hpp"
#include "Ddmin.hpp"


static void Hdd(AlgorithmParams params, std::vector<Unit> &removed_units) {
    std::vector<Ast **> units;
    TreeFindUnits(params.tree, units, "function_definition");
    Ddmin(params, units, removed_units);

    units.clear();
    TreeFindUnits(params.tree, units, "expression_statement");
    Ddmin(params, units, removed_units);
}

static void HddWithBinaryReduction(AlgorithmParams params, std::vector<Unit> &removed_units) {
    std::vector<Ast **> units;
    TreeFindUnits(params.tree, units, "function_definition");
    BinaryReduction(params, units, removed_units);

    units.clear();
    TreeFindUnits(params.tree, units, "expression_statement");
    BinaryReduction(params, units, removed_units);
}

void Hdd(AlgorithmParams params) {
    std::vector<Unit> removed_units;
    size_t num_removed_units = 0;
    do {
        size_t size_before_run = removed_units.size();
        Hdd(params, removed_units);
        size_t size_after_run = removed_units.size();
        num_removed_units = size_before_run - size_after_run;
    } while (num_removed_units > 0);
    Hdd(params, removed_units);
    Enable(removed_units);
}

void HddWithBinaryReduction(AlgorithmParams params) {
    std::vector<Unit> removed_units;
    size_t num_removed_units = 0;
    do {
        size_t size_before_run = removed_units.size();
        HddWithBinaryReduction(params, removed_units);
        size_t size_after_run = removed_units.size();
        num_removed_units = size_before_run - size_after_run;
    } while (num_removed_units > 0);
    Hdd(params, removed_units);
    Enable(removed_units);
}
