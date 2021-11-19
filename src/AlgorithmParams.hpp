#ifndef BRIC_ALGORITHM_PARAMS_HPP
#define BRIC_ALGORITHM_PARAMS_HPP
#include "AstNodes.hpp"
#include "Timer.hpp"


struct AlgorithmParams {
    Ast *root_node;
    const char *file_name;
    const char *run_predicate_command;
};


bool IsPredicateSuccessful(AlgorithmParams params);

bool IsPredicateSuccessful(AlgorithmParams params, Timer &timer);

#endif // BRIC_ALGORITHM_PARAMS_HPP
