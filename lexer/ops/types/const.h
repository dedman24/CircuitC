#ifndef VERB_lexer_const_included
#define VERB_lexer_const_included

#include "stdint.h"
#include "../../tokeniser/backend/tokeniser_ctx.h"
#include "../../../frontend/interpreter/ops/_all_ops.h"

void VERB_constant_propagate(VERB_tokeniser_t* const restrict tokeniser, const VERB_bytecode_t op, const VERB_bytecode_argcnt_t srccnt, VERB_variable_t** const restrict src, const VERB_bytecode_argcnt_t dstcnt, VERB_variable_token_t* const restrict dstargs){    
    VERB_variable_t** const restrict dst = malloc(sizeof(*dst)*dstcnt);
    for(VERB_bytecode_argcnt_t i = 0; i < dstcnt; i++) 
        dst[i] = VERB_variable_search_byToken(&tokeniser->backend, dstargs[i]);
    VERB_interpreter_implementation_table[op](src, dst);

    free(dst);
}

#endif
