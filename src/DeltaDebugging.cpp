#include "DeltaDebugging.hpp"
#include <fstream>
#include <string>
#include <vector>


static size_t Max(size_t a, size_t b) {
    return (a > b) ? a : b;
}

static std::vector<std::vector<Ast *>> Partition(std::vector<Ast *> nodes, size_t num_partitions) {
    std::vector<std::vector<Ast *>> partitions;
    std::vector<size_t> num_items_in_partition(num_partitions, 0);
    size_t num_items_in_each_partition = nodes.size() / num_partitions;
    size_t num_leftover_items = nodes.size() % num_partitions;
    for (size_t i = 0; i < num_partitions; ++i) {
        num_items_in_partition[i] = num_items_in_each_partition;
    }

    for (size_t i = 0; i < num_leftover_items; ++i) {
        num_items_in_partition[i] += 1;
    }

    size_t num_items_added = 0;
    for (size_t i = 0; i < num_partitions; ++i) {
        std::vector<Ast *> partition;
        for (size_t j = num_items_added; j < num_items_added + num_items_in_partition[i]; ++j) {
                partition.push_back(nodes[j]);
        }

        partitions.push_back(partition);
        num_items_added += num_items_in_partition[i];
    }

    return partitions;
}

static bool IsPredicateSuccessful(Ast *root_node, const char *file_name, const char *run_predicate_command) {
    AstWriteToFile(root_node, file_name);
    return system(run_predicate_command) == 0;
}

static void DeltaDebugging(Ast *root_node, const char *file_name, const char *run_predicate_command, std::vector<Ast *> nodes, size_t num_partitions) {
    if (nodes.size() == 1) {
        nodes[0]->flags &= ~AST_IS_ACTIVE;
        if (!IsPredicateSuccessful(root_node, file_name, run_predicate_command)) {
            nodes[0]->flags |= AST_IS_ACTIVE;
        }

        return;
    }

    std::vector<std::vector<Ast *>> partitions = Partition(nodes, num_partitions);
    for (size_t i = 0; i < partitions.size(); ++i) {
        for (size_t j = 0; j < partitions[i].size(); ++j) {
            partitions[i][j]->flags &= ~AST_IS_ACTIVE;
        }
    }

    for (size_t i = 0; i < partitions.size(); ++i) {
        for (size_t j = 0; j < partitions[i].size(); ++j) {
            partitions[i][j]->flags |= AST_IS_ACTIVE;
        }

        if (IsPredicateSuccessful(root_node, file_name, run_predicate_command)) {
            DeltaDebugging(root_node, file_name, run_predicate_command, partitions[i], 2);
            return;
        }

        for (size_t j = 0; j < partitions[i].size(); ++j) {
            partitions[i][j]->flags &= ~AST_IS_ACTIVE;
        }
    }

    for (size_t i = 0; i < partitions.size(); ++i) {
        for (size_t j = 0; j < partitions[i].size(); ++j) {
            partitions[i][j]->flags |= AST_IS_ACTIVE;
        }
    }

    if (num_partitions > 2) {
        for (size_t i = 0; i < partitions.size(); ++i) {
            for (size_t j = 0; j < partitions[i].size(); ++j) {
                partitions[i][j]->flags &= ~AST_IS_ACTIVE;
            }

            if (IsPredicateSuccessful(root_node, file_name, run_predicate_command)) {
                std::vector<Ast *> new_nodes;
                for (size_t j = 0; j < partitions.size(); ++j) {
                    if (j == i) {
                        continue;
                    }

                    for (size_t k = 0; k < partitions[j].size(); ++k) {
                        new_nodes.push_back(partitions[j][k]);
                    }
                }

                DeltaDebugging(root_node, file_name, run_predicate_command, new_nodes, Max(num_partitions - 1, 2ul));
                return;
            }

            for (size_t j = 0; j < partitions[i].size(); ++j) {
                partitions[i][j]->flags |= AST_IS_ACTIVE;
            }
        }
    }

    if (num_partitions < nodes.size()) {
        DeltaDebugging(root_node, file_name, run_predicate_command, nodes, Max(nodes.size(), 2 * num_partitions));
    }
}

void DeltaDebugging(Ast *root_node, const char *file_name, const char *run_predicate_command, std::vector<Ast *> nodes) {
    DeltaDebugging(root_node, file_name, run_predicate_command, nodes, 2);
}
