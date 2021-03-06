#include "HierarchicalDeltaDebugging.hpp"
#include "DeltaDebugging.hpp"


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

void HierarchicalDeltaDebugging(Ast *root, std::string f, std::string p) {
    ReduceUnnamed(root);
    Reduce1Child(root);
    bool is_finished;
    do {
        int level = 0;
        is_finished = true;
        while (true) {
            std::vector<Ast *> nodes;
            FindActiveNodes(root, level, nodes);
            if (nodes.empty()) {
                break;
            }

            DeltaDebugging(nodes, root, f, p);
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
}
