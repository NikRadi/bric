#include "List.h"
#include <assert.h>
#include <stdlib.h> // malloc, free
#include <string.h> // memcpy
#include <stdio.h> // debug


static void ReallocateMemory(List *self) {
    void **new_data = (void **) malloc(self->capacity * sizeof(void *));
    memcpy(new_data, self->data, self->count * sizeof(void *));
    free(self->data);
    self->data = new_data;
}

static void InitWithCapacity(List *self, int capacity) {
    self->capacity = capacity;
    self->count = 0;
    self->data = malloc(capacity * sizeof(void *));
}

List ListInit() {
    return ListInitWithCapacity(8);
}

List ListInitWithCapacity(int capacity) {
    List list;
    InitWithCapacity(&list, capacity);
    return list;
}

void ListDelete(List *self) {
    // capacity and count are set to -1 so that
    // it is known this list is deleted.
    self->capacity = -1;
    self->count = -1;
    free(self->data);
}

void ListDeleteListOfLists(List *self) {
    for (int i = 0; i < self->count; ++i) {
        List *child_list = (List *) ListGet(self, i);
        ListDelete(child_list);
    }

    ListDelete(self);
}

void ListAdd(List *self, void *item) {
    if (self->count == self->capacity) {
        self->capacity <<= 2;
        ReallocateMemory(self);
    }

    self->data[self->count] = item;
    self->count += 1;
}

void *ListGet(List *self, int idx) {
    assert(0 <= idx && idx < self->count);
    return self->data[idx];
}

List ListRemoveIdx(List *self, int idx) {
    assert(0 <= idx && idx < self->count);
    List new_self = ListInitWithCapacity(self->capacity);
    new_self.count = self->count - 1;
    memcpy(new_self.data, self->data, idx * sizeof(void *));
    memcpy(new_self.data + idx, self->data + idx + 1, (self->count - idx - 1) * sizeof(void *));
    return new_self;
}

List ListRemoveItem(List *self, void *item) {
    List new_self = ListInitWithCapacity(self->capacity);
    for (int i = 0; i < self->count; ++i) {
        void *item_in_self = ListGet(self, i);
        if (item_in_self != item) {
            ListAdd(&new_self, item_in_self);
        }
    }

    return new_self;
}

List ListFlatten(List *self) {
    List flattened_self = ListInitWithCapacity(self->capacity);
    for (int i = 0; i < self->count; ++i) {
        List *child_list = (List *) ListGet(self, i);
        for (int j = 0; j < child_list->count; ++j) {
            void *item = ListGet(child_list, j);
            ListAdd(&flattened_self, item);
        }
    }

    return flattened_self;
}

List ListPartition(List *self, int num_partitions) {
    List partitions = ListInitWithCapacity(num_partitions);
    int *num_items_in_partition = (int *) malloc(num_partitions * sizeof(int));
    int num_items_in_each_partition = (int) (self->count / num_partitions);
    int leftover_items = self->count % num_partitions;
    for (int i = 0; i < num_partitions; ++i) {
        num_items_in_partition[i] = num_items_in_each_partition;
    }

    for (int i = 0; i < leftover_items; ++i) {
        num_items_in_partition[i] += 1;
    }

    int num_items_added = 0;
    for (int i = 0; i < num_partitions; ++i) {
        List *partition = (List *) malloc(sizeof(List));
        InitWithCapacity(partition, num_items_in_partition[i]);
        for (int j = num_items_added; j < num_items_added + num_items_in_partition[i]; ++j) {
            ListAdd(partition, ListGet(self, j));
        }

        num_items_added += num_items_in_partition[i];
        ListAdd(&partitions, partition);
    }

    return partitions;
}
