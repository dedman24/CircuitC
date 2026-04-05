#ifndef VERB_compiler_ops_mut_included
#define VERB_compiler_ops_mut_included

#include "../includes.h"

void* VERB_compiler_op_mut(VERB_compilation_t* const restrict compilation, VERB_array_t* restrict bytecode, char* restrict tokens){
    VERB_compilation_add_type_qualifier(compilation, VERB_TOKEN_MUT);
// returns type straight up because it doesn't really do anything with it
    return tokens;
}

#endif
