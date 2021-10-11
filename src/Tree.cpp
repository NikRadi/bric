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
        printf("<Leaf is_active=\"%d\" value=\"%s\"/>\n", l->is_active, l->value.c_str());
    }
    else if (node->type == AST_BRANCH) {
        Branch *b = (Branch *) node;
        for (int i = 0; i < indent; ++i) printf(" ");
        printf("<Branch type=\"%s\">\n", ts_node_type(b->ts_node));
        indent += 4;
        for (size_t i = 0; i < b->children.size(); ++i) {
            PrintAst(b->children[i]);
        }

        indent -= 4;
        for (int i = 0; i < indent; ++i) printf(" ");
        printf("<Branch/>\n");
    }
    else {
        printf("PrintAst error\n");
    }
}

static void DeleteAst(Ast *node) {
    if (node == NULL) {
        return;
    }

    if (node->type == AST_BRANCH) {
        Branch *branch = (Branch *) node;
        for (size_t i = 0; i < branch->children.size(); ++i) {
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
        for (size_t i = 0; i < branch->children.size(); ++i) {
            FindNodes(branch->children[i], nodes, type);
        }
    }
}

static Ast *MakeAst(Tree &tree, TSNode ts_node, Leaf **prev_leaf) {
    uint child_count = ts_node_child_count(ts_node);
    if (child_count == 0) {
        uint start_byte = ts_node_start_byte(ts_node);
        uint end_byte = ts_node_end_byte(ts_node);

        Leaf *leaf = new Leaf(ts_node);
        leaf->prev_leaf = *prev_leaf;
        leaf->value = std::string(tree.source_code.c_str() + start_byte, end_byte - start_byte);
        return (Ast *) leaf;
    }

    Branch *branch = new Branch(ts_node);
    for (size_t i = 0; i < child_count; ++i) {
        TSNode ts_child = ts_node_child(ts_node, i);
        Ast *child = MakeAst(tree, ts_child, prev_leaf);
        branch->children.push_back(child);
        if (child->type == AST_LEAF) {
            Leaf *child_leaf = (Leaf *) child;
            if (*prev_leaf != NULL) {
                child_leaf->prev_leaf = *prev_leaf;
                (*prev_leaf)->next_leaf = child_leaf;
            }

            *prev_leaf = child_leaf;
        }
    }

    Ast *c = branch->children[0];
    branch->leftmost_leaf = (c->type == AST_LEAF) ? (Leaf *) c : ((Branch *) c)->leftmost_leaf;

    c = branch->children[branch->children.size() - 1];
    branch->rightmost_leaf = (c->type == AST_LEAF) ? (Leaf *) c : ((Branch *) c)->rightmost_leaf;
    return (Ast *) branch;
}

void SetIsActive(std::vector<Ast *> &units, bool is_active) {
    for (size_t i = 0; i < units.size(); ++i) {
        Ast *node = units[i];
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

void SetIsActive(std::vector<std::vector<Ast *>> &units, bool is_active) {
    for (size_t i = 0; i < units.size(); ++i) {
        SetIsActive(units[i], is_active);
    }
}

Tree TreeInit(TSNode ts_node, std::string source_code) {
    Tree self;
    self.source_code = source_code;

    Leaf *prev_leaf = NULL;
    self.root_node = (Branch *) MakeAst(self, ts_node, &prev_leaf);
    assert(self.root_node->type == AST_BRANCH);
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

    leaf = leaf->next_leaf;
    while (leaf != NULL) {
        if (leaf->is_active) {
            uint start_byte = ts_node_end_byte(leaf->prev_leaf->ts_node);
            uint end_byte = ts_node_start_byte(leaf->ts_node);
            uint bytes_to_copy = end_byte - start_byte;
            if (bytes_to_copy > 0) {
                const char *str_start = self.source_code.c_str() + start_byte;
                const char *str_to_copy = strndup(str_start, bytes_to_copy);
                fputs(str_to_copy, file.stream);
            }

            fputs(leaf->value.c_str(), file.stream);
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
