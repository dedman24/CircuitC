#ifndef VERB_bytecode_op_assignment_included
#define VERB_bytecode_op_assignment_included

#include "../_includes.h"

void VERB_bytecode_op_definition(VERB_tokeniser_t* const tokeniser);

void VERB_bytecode_op_assignment__definition(VERB_tokeniser_t* const restrict tokeniser, const VERB_variable_token_t assigned, const VERB_variable_token_t assigner, const VERB_variable_token_t type){

}

// '=' operator.
void VERB_bytecode_op_assignment(char* restrict* const restrict string, VERB_tokeniser_t* const restrict tokeniser){
    const VERB_variable_token_t assigner = VERB_array_pop_type(VERB_variable_token_t, &tokeniser->values);
    const VERB_variable_token_t assigned = VERB_array_pop_type(VERB_variable_token_t, &tokeniser->values);

    const VERB_variable_token_t type = VERB_array_pop_type_memsafe(VERB_variable_token_t, &tokeniser->values);
    if(type != VERB_variable_null_token) VERB_bytecode_op_assignment__definition(tokeniser, assigned, assigner, type);
}

#endif
