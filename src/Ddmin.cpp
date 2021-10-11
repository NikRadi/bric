#include "Ddmin.hpp"
#include "FileHandling.hpp"
#include <algorithm> // max, min
#include <assert.h>
#include <stdio.h>


static Tree tree;
static std::string run_predicate_command;
static std::string file_name;
static std::vector<Ast *> starting_units;


static void Print(Ast *node) {
    if (node->type == AST_LEAF) {
        Leaf *leaf = (Leaf *) node;
        printf("<l(%d) '%s'", leaf->is_active, leaf->value.c_str());
    }
    else {
        Branch *branch = (Branch *) node;
        Leaf *leaf = branch->leftmost_leaf;
        bool is_active = leaf->is_active;
        printf("<b(%d) '", leaf->is_active);
        while (leaf != branch->rightmost_leaf) {
            assert(leaf->is_active == is_active);
            printf("%s ", leaf->value.c_str());
            leaf = leaf->next_leaf;
        }

        printf("%s'>", leaf->value.c_str());
    }
}

static void Print(std::vector<Ast *> nodes) {
    printf("[");
    if (nodes.size() > 0) {
        Print(nodes[0]);
        for (size_t i = 1; i < nodes.size(); ++i) {
            printf(", ");
            Print(nodes[i]);
        }
    }

    printf("]\n");
}

static std::vector<std::vector<Ast *>> PartitionUnits(std::vector<Ast *> units, int num_partitions) {
    std::vector<std::vector<Ast *>> partitions;
    std::vector<int> num_items_in_partition(num_partitions, 0);
    int num_items_in_each_partition = (int) (units.size() / num_partitions);
    int num_leftover_items = units.size() % num_partitions;
    for (int i = 0; i < num_partitions; ++i) {
        num_items_in_partition[i] = num_items_in_each_partition;
    }

    for (int i = 0; i < num_leftover_items; ++i) {
        num_items_in_partition[i] += 1;
    }

    int num_items_added = 0;
    for (int i = 0; i < num_partitions; ++i) {
        std::vector<Ast *> partition;
        for (int j = num_items_added; j < num_items_added + num_items_in_partition[i]; ++j) {
            partition.push_back(units[j]);
        }

        num_items_added += num_items_in_partition[i];
        partitions.push_back(partition);
    }

    return partitions;
}

static std::vector<Ast *> Flatten(std::vector<std::vector<Ast *>> partitions) {
    std::vector<Ast *> result;
    for (size_t i = 0; i < partitions.size(); ++i) {
        std::vector<Ast *> partition = partitions[i];
        result.insert(result.end(), partition.begin(), partition.end());
    }

    return result;
}

static void SetIsActive(std::vector<Ast *> &partition, bool is_active) {
    for (size_t i = 0; i < partition.size(); ++i) {
        Ast *node = partition[i];
        if (node->type == AST_LEAF) {
            Leaf *leaf = (Leaf *) node;
            leaf->is_active = is_active;
        }
        else {
            Branch *branch = (Branch *) node;
            Leaf *leaf = branch->leftmost_leaf;
            while (leaf != branch->rightmost_leaf) {
                leaf->is_active = is_active;
                leaf = leaf->next_leaf;
            }

            leaf->is_active = is_active;
        }
    }
}

static void SetIsActive(std::vector<std::vector<Ast *>> &partitions, bool is_active) {
    for (size_t i = 0; i < partitions.size(); ++i) {
        SetIsActive(partitions[i], is_active);
    }
}

static int TestPartitions(std::vector<std::vector<Ast *>> partitions, bool default_is_active) {
    for (size_t i = 0; i < partitions.size(); ++i) {
        SetIsActive(partitions[i], !default_is_active);
        TreeWriteToFile(tree, file_name, "w");
        int return_code = system(run_predicate_command.c_str());
        if (return_code == 0) {
            printf("Success on partition %ld\n", i);
            return i;
        }

        SetIsActive(partitions[i], default_is_active);
    }

    return -1;
}

static void Ddmin(std::vector<Ast *> units, int num_partitions) {
    printf("Testing subsets\n");
    std::vector<std::vector<Ast *>> partitions = PartitionUnits(units, num_partitions);
    SetIsActive(partitions, false);
    int partition_idx = TestPartitions(partitions, false);
    if (partition_idx >= 0) {
        printf("Reducing to subset\n");
        std::vector<Ast *> partition = partitions[partition_idx];
        if (partition.size() > 1) {
            Ddmin(partition, 2);
        }

        return;
    }

    if (num_partitions > 2) {
        printf("Testing complements\n");
        SetIsActive(partitions, true);
        int partition_idx = TestPartitions(partitions, true);
        if (partition_idx >= 0) {
            printf("Reducing to complements\n");
            partitions.erase(partitions.begin() + partition_idx);
            std::vector<Ast *> new_partitions = Flatten(partitions);
            Ddmin(new_partitions, std::max(num_partitions - 1, 2));
            return;
        }
    }

    if (static_cast<size_t>(num_partitions) < units.size()) {
        printf("Increasing granularity\n");
        Ddmin(units, std::min((int) units.size(), 2 * num_partitions));
        return;
    }

    SetIsActive(partitions, true);
    TreeWriteToFile(tree, file_name, "w");
}

void Ddmin(Tree &_tree, std::vector<Ast *> units, std::string _run_predicate_command, std::string _file_name) {
    tree = _tree;
    run_predicate_command = _run_predicate_command;
    file_name = _file_name;
    starting_units = units;
    Ddmin(units, 2);
}
