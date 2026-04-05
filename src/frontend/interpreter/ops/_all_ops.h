#ifndef VERB_interpreter__all_ops_included
#define VERB_interpreter__all_ops_included

#include "arith.h"
#include "../_includes.h"
#include "../../../tokens/primitives/bytecode.h"

// op table
VERB_interpreter_implementation_t VERB_interpreter_implementation_table[] = {
    [VERB_BC_add] = VERB_interpreter_op_add_impl
};

#endif
