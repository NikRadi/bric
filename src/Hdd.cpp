#include "Hdd.hpp"
#include "BinaryReduction.hpp"
#include "Ddmin.hpp"


constexpr int NUM_TS_TYPES = 5;
constexpr const char *TS_TYPES[NUM_TS_TYPES] = {
    "function_definition",
    "expression_statement",
    "if_statement",
    "while_statement",
    "for_statement"
};

static void Hdd(AlgorithmParams params, std::vector<Unit> &removed_units) {
    for (int i = 0; i < NUM_TS_TYPES; ++i) {
        std::vector<Ast **> units;
        TreeFindUnits(params.tree, units, TS_TYPES[i]);
        if (units.size() > 0) {
            Ddmin(params, units, removed_units);
        }
    }
}

static void HddWithBinaryReduction(AlgorithmParams params, std::vector<Unit> &removed_units) {
    for (int i = 0; i < NUM_TS_TYPES; ++i) {
        std::vector<Ast **> units;
        TreeFindUnits(params.tree, units, TS_TYPES[i]);
        if (units.size() > 0) {
            BinaryReduction(params, units, removed_units);
        }
    }
}

void Hdd(AlgorithmParams params) {
    std::vector<Unit> removed_units;
    size_t num_removed_units;
    do {
        size_t size_before_run = removed_units.size();
        Hdd(params, removed_units);
        size_t size_after_run = removed_units.size();
        num_removed_units = size_before_run - size_after_run;
    } while (num_removed_units > 0);

    Enable(removed_units);
}

void HddWithBinaryReduction(AlgorithmParams params) {
    std::vector<Unit> removed_units;
    size_t num_removed_units;
    do {
        size_t size_before_run = removed_units.size();
        HddWithBinaryReduction(params, removed_units);
        size_t size_after_run = removed_units.size();
        num_removed_units = size_before_run - size_after_run;
    } while (num_removed_units > 0);

    Enable(removed_units);
}
