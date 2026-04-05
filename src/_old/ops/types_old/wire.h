#ifndef VERB_compiler_ops_wire_included
#define VERB_compiler_ops_wire_included

#include "../includes.h"
#include "type_handler.h"

void* VERB_compiler_op_wire(VERB_compilation_t* const restrict compilation, VERB_array_t* restrict bytecode, char* restrict tokens){
    return VERB_compiler_op_type_handler(compilation, bytecode, tokens, VERB_TOKEN_W);
}

#endif
