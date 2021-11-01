#include "Hdd.hpp"
#include "BinaryReduction.hpp"
#include "Ddmin.hpp"
#include "Tree.hpp"
#include <cassert>


void Hdd(AlgorithmParams params, std::vector<Unit> &removed_units) {
    size_t num_removed_units;
    do {
        size_t num_removed_units_before = removed_units.size();
        int level = 0;
        while (true) {
            std::vector<Unit> units_in_level = TreeFindUnitsInLevel(params.tree, level);
            if (units_in_level.size() == 0) {
                break;
            }

            BinaryReduction(params, units_in_level, removed_units);
            level += 1;
        }

        size_t num_removed_units_after = removed_units.size();
        num_removed_units = num_removed_units_after - num_removed_units_before;
        printf("num_removed_units: %ld\n", num_removed_units);
        for (size_t i = num_removed_units_before; i < num_removed_units_after; ++i) AstPrintXml(removed_units[i].ast_value);
//        TreeWriteToFile(params.tree, std::to_string(num_removed_units) + "ddmin" + params.c_file_name);
    } while (num_removed_units > 0);
}
