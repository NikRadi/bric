#ifndef BRIC_BINARY_REDUCTION_HPP
#define BRIC_BINARY_REDUCTION_HPP
#include "AstNodes.hpp"
#include <vector>


void BinaryReduction(Ast *root_node, const char *file_name, const char *run_predicate_command, std::vector<Ast *> function_defs);

#endif // BRIC_BINARY_REDUCTION_HPP
