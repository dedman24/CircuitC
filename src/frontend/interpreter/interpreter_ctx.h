#ifndef VERB_frontend_interpreter_ctx_included
#define VERB_frontend_interpreter_ctx_included

#include "stddef.h"
#include "stdint.h"
#include "../../libraries/rhts/rht.h"                                   // VERB_rht_t & ops.
#include "../../lexer/tokeniser/variables/variable_definition.h"        // all variable types.

typedef struct{
    char* code;
    size_t pos;
} VERB_code_t;

typedef void (*VERB_interpreter_implementation_t)(VERB_variable_t**, VERB_variable_t**);

#endif
