#include "Tree.hpp"
#include "FileHandling.hpp"
#include <assert.h>
#include <stdarg.h>
#include <string.h>


static void PrintAst(Ast *node) {
    static int indent = 0;
    if (node->type == AST_LEAF)  {
        for (int i = 0; i < indent; ++i) printf(" ");
        Leaf *l = (Leaf *) node;
        printf("<Leaf is_active=\"%d\" is_mergeable=\"%d\" value=\"%s\"/>\n", l->is_active, l->is_mergeable, l->value.c_str());
    }
    else if (node->type == AST_BRANCH) {
        Branch *branch = (Branch *) node;
        for (int i = 0; i < indent; ++i) printf(" ");
        printf("<Branch type=\"%s\">\n", ts_node_type(branch->ts_node));
        indent += 4;
        for (int i = 0; i < branch->children.size(); ++i) {
            PrintAst(branch->children[i]);
        }

        indent -= 4;
        for (int i = 0; i < indent; ++i) printf(" ");
        printf("<Branch/>\n");
    }
    else {
        printf("PrintAst error\n");
    }
}

static bool IsSpace(std::string str) {
    for (int i = 0; i < str.size(); ++i) {
        if (!isspace(str[i])) {
            return false;
        }
    }

    return true;
}

static void DeleteAst(Ast *node) {
    if (node == NULL) {
        return;
    }

    if (node->type == AST_BRANCH) {
        Branch *branch = (Branch *) node;
        for (int i = 0; i < branch->children.size(); ++i) {
            DeleteAst(branch->children[i]);
        }
    }

    delete node;
    node = NULL;
}

static void FindNodes(Ast *node, std::vector<Ast *> &nodes, std::string type) {
    std::string ts_type = ts_node_type(node->ts_node);
    if (ts_type == type) {
        nodes.push_back(node);
    }

    if (node->type == AST_BRANCH) {
        Branch *branch = (Branch *) node;
        for (int i = 0; i < branch->children.size(); ++i) {
            FindNodes(branch->children[i], nodes, type);
        }
    }
}

static Leaf *LeafInit(TSNode ts_node, std::string value, Leaf *prev_leaf) {
    if (IsSpace(value)) {
        return NULL;
    }

    Leaf *leaf = new Leaf(ts_node);
    leaf->prev_leaf = prev_leaf;
    leaf->value = value;
    leaf->start_byte = ts_node_start_byte(ts_node);
    leaf->end_byte = ts_node_end_byte(ts_node);
    return leaf;
}


static Ast *MakeAst(Tree &tree, TSNode ts_node, std::string source_code, Leaf **prev_leaf) {
    uint child_count = ts_node_child_count(ts_node);
    if (child_count == 0) {
        uint start_byte = ts_node_start_byte(ts_node);
        uint end_byte = ts_node_end_byte(ts_node);
        std::string value = strndup(source_code.c_str() + start_byte, end_byte - start_byte);
        Leaf *leaf = LeafInit(ts_node, value, *prev_leaf);
        if (leaf != NULL) {
            leaf->is_mergeable = !ts_node_is_named(ts_node);
        }

        return (Ast *) leaf;
    }

    if (child_count == 1) {
        // Assuming that the child is a leaf and not a branch
        // with multiple children. Although I am not sure if
        // anything would go wrong if that is the case.
        return MakeAst(tree, ts_node_child(ts_node, 0), source_code, prev_leaf);
    }

    std::vector<Ast *> children;
    bool are_all_children_leaves = true;
    int num_non_mergeable_leaves = 0;
    for (int i = 0; i < child_count; ++i) {
        TSNode ts_child = ts_node_child(ts_node, i);
        Ast *child = MakeAst(tree, ts_child, source_code, prev_leaf);
        if (child == NULL) {
            continue;
        }

        children.push_back(child);
        if (child->type == AST_LEAF) {
            Leaf *leaf = (Leaf *) child;
            *prev_leaf = leaf;
            if (!leaf->is_mergeable) num_non_mergeable_leaves += 1;
        }
        else are_all_children_leaves = false;
    }

    if (are_all_children_leaves && num_non_mergeable_leaves <= 2) {
        Leaf *first_leaf = (Leaf *) children[0];
        Leaf *last_leaf = (Leaf *) children[children.size() - 1];
        uint start_byte = first_leaf->start_byte;
        uint end_byte = last_leaf->end_byte;
        std::string value = strndup(source_code.c_str() + start_byte, end_byte - start_byte);
        Leaf *leaf = LeafInit(ts_node, value, first_leaf->prev_leaf);
        leaf->start_byte = start_byte;
        leaf->end_byte = end_byte;
        leaf->is_mergeable = num_non_mergeable_leaves == 0;
        return (Ast *) leaf;
    }

    Branch *branch = new Branch(ts_node);
    branch->children = children;
    for (int i = 0; i < children.size(); ++i) {
        if (children[i]->type == AST_LEAF) {
            tree.leaves.push_back(children[i]);
        }
    }

    Ast *c = branch->children[0];
    branch->leftmost_leaf = (c->type == AST_LEAF) ? (Leaf *) c : ((Branch *) c)->leftmost_leaf;

    c = branch->children[branch->children.size() - 1];
    branch->rightmost_leaf = (c->type == AST_LEAF) ? (Leaf *) c : ((Branch *) c)->rightmost_leaf;

    return (Ast *) branch;
}

Tree TreeInit(TSNode ts_node, std::string source_code) {
    Tree self;
    self.source_code = source_code;

    Leaf *prev_leaf = NULL;
    self.root_node = (Branch *) MakeAst(self, ts_node, source_code, &prev_leaf);
    assert(self.root_node->type == AST_BRANCH);

    Leaf *leaf = self.root_node->rightmost_leaf;
    while (leaf->prev_leaf != NULL) {
        Leaf *prev_leaf = leaf->prev_leaf;
        prev_leaf->next_leaf = leaf;
        leaf = prev_leaf;
    }

    return self;
}

void TreeDelete(Tree &self) {
    DeleteAst(self.root_node);
}

void TreeFindNodes(Tree &self, std::vector<Ast *> &nodes, std::string type) {
    FindNodes(self.root_node, nodes, type);
}

void TreeWriteToFile(Tree &self, std::string file_name, std::string mode) {
    File file = OpenFile(file_name, mode);
    Leaf *leaf = self.root_node->leftmost_leaf;
    // Write the first active leaf so that leaf->prev leaf is not NULL
    while (true) {
        if (leaf->is_active) {
            fputs(leaf->value.c_str(), file.stream);
            break;
        }

        leaf = leaf->next_leaf;
    }

    Leaf *last_active_leaf = leaf;
    leaf = leaf->next_leaf;
    while (leaf != NULL) {
        if (leaf->is_active) {
            //uint start_byte = leaf->prev_leaf->end_byte;
            //uint end_byte = leaf->start_byte;
            uint start_byte = last_active_leaf->end_byte;
            uint end_byte = last_active_leaf->next_leaf->start_byte;
            uint bytes_to_copy = end_byte - start_byte;
            if (bytes_to_copy > 0) {
                const char *str_start = self.source_code.c_str() + start_byte;
                const char *str_to_copy = strndup(str_start, bytes_to_copy);
                fputs(str_to_copy, file.stream);
            }

            fputs(leaf->value.c_str(), file.stream);
            last_active_leaf = leaf;
        }

        assert(leaf->prev_leaf->next_leaf == leaf);
        leaf = leaf->next_leaf;
    }

    fputs("\n", file.stream);
    CloseFile(file);
}

void TreePrint(Tree &self) {
    PrintAst(self.root_node);
}
