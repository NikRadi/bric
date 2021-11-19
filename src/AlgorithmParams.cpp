#include "AlgorithmParams.hpp"


bool IsPredicateSuccessful(AlgorithmParams params) {
    AstWriteToFile(params.root_node, params.file_name);
    return system(params.run_predicate_command) == 0;
}

bool IsPredicateSuccessful(AlgorithmParams params, Timer &timer) {
    AstWriteToFile(params.root_node, params.file_name);
    timer.Start();
    bool result = system(params.run_predicate_command) == 0;
    timer.Stop();
    return result;
}
