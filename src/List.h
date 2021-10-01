#ifndef BRIC_LIST_H
#define BRIC_LIST_H


struct List {
    int capacity;
    int count;
    void **data;
} typedef List;


List ListInit();

List ListInitWithCapacity(int capacity);

void ListDelete(List *self);

void ListDeleteListOfLists(List *self);

void ListAdd(List *self, void *item);

void *ListGet(List *self, int idx);

List ListRemoveIdx(List *self, int idx);

List ListRemoveItem(List *self, void *item);

List ListFlatten(List *self);

List ListPartition(List *self, int num_partitions);

#endif // BRIC_LIST_H
