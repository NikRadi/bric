#ifndef BRIC_ALGORITHM_GENERAL_HPP
#define BRIC_ALGORITHM_GENERAL_HPP
#include <string>
#include <vector>


struct Ast;
struct Tree;

struct AlgorithmParams {
    std::string c_file_name;
    std::string run_predicate;
    Tree *tree;
};

struct Unit {
    Ast *ast_value;
    Ast **ast_in_tree;
};


Unit ToUnit(Ast **ast);

std::vector<Unit> ToUnits(std::vector<Ast **> asts);

std::vector<std::vector<Unit>> Partition(std::vector<Unit> units, size_t num_partitions);

void Enable(Unit unit);

void Enable(std::vector<Unit> partition);

void Disable(Unit unit);

void Disable(std::vector<Unit> partition);

bool IsDisabled(Unit unit);

#endif // BRIC_ALGORITHM_GENERAL_HPP
