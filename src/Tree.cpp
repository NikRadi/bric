#include "Tree.hpp"
#include <fstream>
#include <cstring>


static Ast *AstInit(TSNode ts_node, std::string source_code, Leaf **prev_leaf) {
    uint num_children = ts_node_child_count(ts_node);
    if (num_children == 0) {
        uint start_byte = ts_node_start_byte(ts_node);
        uint end_byte = ts_node_end_byte(ts_node);

        Leaf *leaf = new Leaf(ts_node);
        leaf->value = std::string(source_code.c_str() + start_byte, end_byte - start_byte);
        leaf->has_prev_leaf = (*prev_leaf) != NULL;
        if (leaf->has_prev_leaf) {
            leaf->prev_leaf_end_byte = ts_node_end_byte((*prev_leaf)->ts_node);
        }

        return static_cast<Ast *>(leaf);
    }

    Branch *branch = new Branch(ts_node);
    for (size_t i = 0; i < num_children; ++i) {
        TSNode ts_child = ts_node_child(ts_node, i);
        Ast *child = AstInit(ts_child, source_code, prev_leaf);
        child->parent = branch;
        child->child_idx = i;
        branch->children.push_back(child);
        if (child->type == AST_LEAF) {
            *prev_leaf = static_cast<Leaf *>(child);
        }
    }

    return static_cast<Ast *>(branch);
}

static void AstDelete(Ast *self) {
    if (self->type == AST_BRANCH) {
        Branch *b = static_cast<Branch *>(self);
        for (size_t i = 0; i < b->children.size(); ++i) {
            AstDelete(b->children[i]);
        }
    }

    delete self;
}

static void AstFindUnits(Ast *self, std::vector<Ast **> &units, std::string ts_type) {
    if (self->type == AST_BRANCH) {
        Branch *branch = (Branch *) self;
        for (size_t i = 0; i < branch->children.size(); ++i) {
            if (branch->children[i] == NULL) {
                continue;
            }

            if (ts_node_type(branch->children[i]->ts_node) == ts_type) {
                units.push_back(&branch->children[i]);
            }

            AstFindUnits(branch->children[i], units, ts_type);
        }
    }
}

static void AstWriteToFile(Ast *self, std::string source_code, std::ofstream &ofstream) {
    if (self->type == AST_LEAF) {
        Leaf *leaf = static_cast<Leaf *>(self);
        if (leaf->has_prev_leaf) {
            uint bytes_to_copy = ts_node_start_byte(leaf->ts_node) - leaf->prev_leaf_end_byte;
            if (bytes_to_copy > 0) {
                const char *str_start = source_code.c_str() + leaf->prev_leaf_end_byte;
                ofstream << strndup(str_start, bytes_to_copy);
            }
        }

        ofstream << leaf->value;
    }
    else {
        Branch *branch = static_cast<Branch *>(self);
        for (size_t i = 0; i < branch->children.size(); ++i) {
            if (branch->children[i] == NULL) {
                continue;
            }

            AstWriteToFile(branch->children[i], source_code, ofstream);
        }
    }
}

static void AstFindUnitsInEachLevel(Ast **self, std::vector<std::vector<Unit>> &units_in_each_level, int current_level) {
    Unit unit = ToUnit(self);
    units_in_each_level[current_level].push_back(unit);
    if ((*self)->type == AST_BRANCH) {
        std::vector<Unit> units_in_next_level;
        units_in_each_level.push_back(units_in_next_level);
        Branch *branch = static_cast<Branch *>(*self);
        for (size_t i = 0; i < branch->children.size(); ++i) {
            if (branch->children[i] != NULL) {
                int next_level = current_level + 1;
                AstFindUnitsInEachLevel(&branch->children[i], units_in_each_level, next_level);
            }
        }
    }
}

static void AstFindUnitsInLevel(Ast **self, int level, std::vector<Unit> &units) {
    if (level == 0) {
        Unit unit = ToUnit(self);
        units.push_back(unit);
    }
    else if ((*self) -> type == AST_BRANCH) {
        Branch *branch = static_cast<Branch *>(*self);
        for (size_t i = 0; i < branch->children.size(); ++i) {
            if (branch->children[i] != NULL) {
                AstFindUnitsInLevel(&branch->children[i], level - 1, units);
            }
        }
    }
}

Tree *TreeInit(TSNode ts_node, std::string source_code) {
    Tree *self = new Tree();
    self->source_code = source_code;
    Leaf *prev_leaf = NULL;
    self->root_node = AstInit(ts_node, source_code, &prev_leaf);
    return self;
}

void TreeDelete(Tree *self) {
    AstDelete(self->root_node);
    delete self;
}

void TreeFindUnits(Tree *self, std::vector<Ast **> &units, std::string ts_type) {
    AstFindUnits(self->root_node, units, ts_type);
}

void TreeWriteToFile(Tree *self, std::string file_name) {
    std::ofstream ofstream(file_name);
    if (self->root_node != NULL) {
        AstWriteToFile(self->root_node, self->source_code, ofstream);
    }

    ofstream << "\n";
    ofstream.close();
}

void AstPrintXml(Ast *self) {
    static int indent = 0;
    if (self->type == AST_LEAF) {
        for (int i = 0; i < indent; ++i) printf(" ");
        printf("<Leaf value=\"%s/>\n", ((Leaf *) self)->value.c_str());
    }
    else {
        for (int i = 0; i < indent; ++i) printf(" ");
        printf("<Branch>\n");
        indent += 4;
        Branch *branch = static_cast<Branch *>(self);
        for (size_t i = 0; i < branch->children.size(); ++i) {
            if (branch->children[i] == NULL) {
                continue;
            }

            AstPrintXml(branch->children[i]);
        }

        indent -= 4;
        for (int i = 0; i < indent; ++i) printf(" ");
        printf("<Branch/>\n");
    }
}

void TreePrintXml(Tree *self) {
    AstPrintXml(self->root_node);
}

std::vector<std::vector<Unit>> TreeFindUnitsInEachLevel(Tree *self) {
    std::vector<std::vector<Unit>> units_in_each_level;
    std::vector<Unit> units_in_first_level;
    units_in_each_level.push_back(units_in_first_level);
    AstFindUnitsInEachLevel(&self->root_node, units_in_each_level, 0);
    return units_in_each_level;
}

std::vector<Unit> TreeFindUnitsInLevel(Tree *self, int level) {
    std::vector<Unit> units_in_level;
    AstFindUnitsInLevel(&self->root_node, level, units_in_level);
    return units_in_level;
}
