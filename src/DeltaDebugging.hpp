#ifndef BRIC_DELTA_DEBUGGING_HPP
#define BRIC_DELTA_DEBUGGING_HPP
#include "AstNodes.hpp"
#include <vector>


void DeltaDebugging(Ast *root_node, const char *file_name, const char *run_predicate_command, std::vector<Ast *> nodes);

#endif // BRIC_DELTA_DEBUGGING_HPP
