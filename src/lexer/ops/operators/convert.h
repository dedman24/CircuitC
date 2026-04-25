#ifndef VERB_bytecode_op_convert_included
#define VERB_bytecode_op_convert_included

#include "../_includes.h"
#include "../_primitives/primitives.h"

// '->' conversion operator.
// wouldn't something like the () prefix work better?
// how would foo->bar !acq be handled? we need some special rules when processing types in type-processing stuff, but other than that it's fine.
static void VERB_bytecode_op_convert(char* restrict* const restrict string, VERB_tokeniser_t* const restrict tokeniser){
    VERB_op__opcode(tokeniser, VERB_BC_convert);
}

#endif
