#include "HierarchicalDeltaDebugging.hpp"
#include "DeltaDebugging.hpp"


void HierarchicalDeltaDebugging(Ast *root_node, const char *file_name, const char *run_predicate_command) {
    bool removed_nodes;
    do {
        removed_nodes = false;
        int level = 0;
        std::vector<Ast *> nodes;
        do {
            nodes.clear();
            AstFindNodes(root_node, AST_IS_ACTIVE, level, nodes);
            printf("Level %d (%zu nodes) \n", level, nodes.size());
            DeltaDebugging(root_node, file_name, run_predicate_command, nodes);
            if (!removed_nodes) {
                for (size_t i = 0; i < nodes.size(); ++i) {
                    if (!(nodes[i]->flags & AST_IS_ACTIVE)) {
                        removed_nodes = true;
                    }
                }
            }

            level += 1;
        } while (nodes.size() > 0);
    } while (removed_nodes);
}
