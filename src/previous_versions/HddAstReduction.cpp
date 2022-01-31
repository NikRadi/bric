#include "HierarchicalDeltaDebugging.hpp"
#include "DeltaDebugging.hpp"
#include "Timer.hpp"


#define REDUCE_1_CHILD
#define REDUCE_UNNAMED


static int CountActiveNodes(Ast *node) {
    int count = 0;
    for (auto child : node->children) {
        if (child->is_active) {
            count += Count(child);
        }
    }

    return count + 1;
}

#ifdef REDUCE_1_CHILD
static void Reduce1Child(Ast *node) {
    for (auto it = node->children.begin(); it != node->children.end();) {
        if ((*it)->type != AST_TYPE_LEAF) {
            if ((*it)->children.empty()) {
                node->children.erase(it);
            }
            else {
                if ((*it)->children.size() == 1) {
                    (*it) = (*it)->children[0];
                }

                Reduce1Child((*it));
                it += 1;
            }
        }
        else {
            it += 1;
        }
    }
}
#endif // REDUCE_1_CHILD
#ifdef REDUCE_UNNAMED
static void RemoveInactiveNodes(Ast *node) {
    for (auto it = node->children.begin(); it != node->children.end();) {
        if (!(*it)->is_active) {
            it = node->children.erase(it);
        }
        else {
            if ((*it)->type != AST_TYPE_LEAF) {
                RemoveInactiveNodes((*it));
            }

            it += 1;
        }
    }
}

static void _ReduceUnnamed(Ast *node, Ast **prev_leaf) {
    if (node->type == AST_TYPE_LEAF) {
        if (node->ts_type != ";") {
            (*prev_leaf) = node;
        }
        else if (prev_leaf != NULL) {
            (*prev_leaf)->value += ";";
            node->is_active = false;
        }
    }
    else {
        for (auto child : node->children) {
            _ReduceUnnamed(child, prev_leaf);
        }
    }
}

static void ReduceUnnamed(Ast *node) {
    Ast *prev_leaf = NULL;
    _ReduceUnnamed(node, &prev_leaf);
    RemoveInactiveNodes(node);
}
#endif // REDUCE_UNNAMED

void HierarchicalDeltaDebugging(Ast *root, std::string f, std::string p) {
    Timer t1;
    t1.Start();
    printf("before: %d\n", CountActiveNodes(root));
#ifdef REDUCE_UNNAMED
    ReduceUnnamed(root);
    printf("after ReduceUnnamed: %d\n", CountActiveNodes(root));
#endif
#ifdef REDUCE_1_CHILD
    Reduce1Child(root);
    printf("after Reduce1Child: %d\n", CountActiveNodes(root));
#endif // REDUCE_1_CHILD
    temp x;
    x.c_hits = 0;
    x.tests = 0;
    int ddmin_calls = 0;
    int num_hdd = 0;
    size_t nodes_tested = 0;
    bool is_finished;
    t1.Stop();
    printf("before hdd (%llums)\n", t1.ElapsedMilliseconds());
    Timer t2;
    t2.Start();
    do {
        int level = 0;
        is_finished = true;
        num_hdd += 1;
        while (true) {
            std::vector<Ast *> nodes;
            FindActiveNodes(root, level, nodes);
            if (nodes.empty()) {
                break;
            }

            ddmin_calls += 1;
            nodes_tested += nodes.size();
            temp y = DeltaDebugging(nodes, root, f, p);
            x.tests += y.tests;
            x.c_hits += y.c_hits;
            if (is_finished) {
                for (auto node : nodes) {
                    if (!node->is_active) {
                        is_finished = false;
                        break;
                    }
                }
            }

            level += 1;
        }
    } while (!is_finished);
    t2.Stop();
    printf("after hdd (%llums)\n", t2.ElapsedMilliseconds());
    printf("after: %d\n", CountActiveNodes(root));
    printf(
        "tests: %d, c_hits: %d, diff: %d, nodes_tested: %llu, ddmin_calls: %d, num_hdd: %d\n",
        x.tests, x.c_hits, x.tests - x.c_hits, nodes_tested, ddmin_calls, num_hdd
    );
}
