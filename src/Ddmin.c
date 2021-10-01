#include "Ddmin.h"


static char *file_name = NULL;
static char *source_code = NULL;
static char *run_predicate_command = NULL;
static List *original_units;
static AstNode *root_node = NULL;


static void PrintNodesInPartition(List *partition) {
    printf("[");
    AstNode *node = (AstNode *) ListGet(partition, 0);
    printf("%s (%d)", node->value, node->is_ignored);
    for (int i = 1; i < partition->count; ++i) {
        node = (AstNode *) ListGet(partition, i);
        printf(", %s (%d)", node->value, node->is_ignored);
    }

    printf("]\n");
}

static int Min(int a, int b) {
    return (a < b) ? a : b;
}

static int Max(int a, int b) {
    return (a > b) ? a : b;
}

static void SetAstNodesIsIgnoredValues(List *partition, bool is_ignored) {
    for (int i = 0; i < partition->count; ++i) {
        AstNode *node = (AstNode *) ListGet(partition, i);
        node->is_ignored = is_ignored;
    }
}

static void SetPartitionsIsIgnoredValues(List *partitions, bool is_ignored) {
    for (int i = 0; i < partitions->count; ++i) {
        List *partition = (List *) ListGet(partitions, i);
        SetAstNodesIsIgnoredValues(partition, is_ignored);
    }
}

static List *TestPartitions(List *partitions, bool default_is_ignored_value) {
    for (int i = 0; i < partitions->count; ++i) {
        List *partition = (List *) ListGet(partitions, i);
        SetAstNodesIsIgnoredValues(partition, !default_is_ignored_value);
        printf("Testing partition (set to %d): ", !default_is_ignored_value);
        PrintNodesInPartition(partition);
        PrintNodesInPartition(original_units);
        printf("\n");

        File file = OpenFile(file_name, "w+");
        AstWriteLeafNodesToFile(root_node, file, source_code);
        CloseFile(file);
        int return_code = system(run_predicate_command);
        if (return_code == 0) {
            return partition;
        }

        SetAstNodesIsIgnoredValues(partition, default_is_ignored_value);
    }

    return NULL;
}

static void DdminRecursion(List *units, int num_partitions) {
    printf("\n====\n\n");
    printf("units->count: %d\n", units->count);
    printf("num_partitions: %d\n", num_partitions);

    printf("Testing subsets\n");
    List partitions = ListPartition(units, num_partitions);
    SetPartitionsIsIgnoredValues(&partitions, true);
    printf("Got units: ");
    PrintNodesInPartition(units);
    List *partition = TestPartitions(&partitions, true);
    if (partition != NULL) {
        printf("Reducing to subset\n");
        if (partition->count > 1) {
            DdminRecursion(partition, 2);
        }

        return;
    }

    if (num_partitions > 2) {
        printf("Testing complements\n");
        SetPartitionsIsIgnoredValues(&partitions, false);
        printf("Got units: ");
        PrintNodesInPartition(units);
        partition = TestPartitions(&partitions, false);
        if (partition != NULL) {
            printf("Reducing to complement\n");
            partitions = ListRemoveItem(&partitions, partition);
            partitions = ListFlatten(&partitions);
            DdminRecursion(&partitions, Max(num_partitions - 1, 2));
            return;
        }
    }

    if (num_partitions < units->count) {
        printf("Increasing granularity\n");
        DdminRecursion(units, Min(units->count, 2 * num_partitions));
        return;
    }

    SetAstNodesIsIgnoredValues(units, false);
    PrintNodesInPartition(original_units);
    File file = OpenFile(file_name, "w+");
    AstWriteLeafNodesToFile(root_node, file, source_code);
    CloseFile(file);
    printf("done\n");
}

void Ddmin(List *units, char *_run_predicate_command, AstNode *_root_node, char *_file_name, char *_source_code) {
    file_name = _file_name;
    source_code = _source_code;
    run_predicate_command = _run_predicate_command;
    root_node = _root_node;
    original_units = units;
    DdminRecursion(units, 2);
}