#ifndef VERB_compiler_ops_included
#define VERB_compiler_ops_included

// essential includes
#include "includes.h"

// the basic idea is to have a VERB_array_t* const restrict holding the actual compiled code,
// and a char* holding the token list, which is updated as we execute.
void* (*VERB_compiler_token_to_code[])(VERB_frontend_t* const restrict compilation, VERB_array_t* const restrict, char* restrict) = {

};

#endif
