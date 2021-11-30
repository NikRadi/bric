#include "HierarchicalDeltaDebugging.hpp"
#include "DeltaDebugging.hpp"


void HierarchicalDeltaDebugging(AlgorithmParams params) {
    bool removed_nodes;
    do {
        removed_nodes = false;
        int level = 0;
        std::vector<Ast *> nodes;
        AstFindNodes(params.root_node, AST_IS_ACTIVE, level, nodes);
        while (nodes.size() > 0) {
            DeltaDebugging(params, nodes);
            if (!removed_nodes) {
                for (size_t i = 0; i < nodes.size(); ++i) {
                    if (!(nodes[i]->flags & AST_IS_ACTIVE)) {
                        removed_nodes = true;
                    }
                }
            }

            level += 1;
            nodes.clear();
            AstFindNodes(params.root_node, AST_IS_ACTIVE, level, nodes);
        }
    } while (removed_nodes);
}
