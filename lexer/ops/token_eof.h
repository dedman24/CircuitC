#ifndef VERB_tokens_op_token_eof_included
#define VERB_tokens_op_token_eof_included

// cleanup for \00 (AKA EOF token)
// when file ends:
//      all implicit scopes have to be closed

#include "_includes.h"

// to be called when a file is finished lexing; closes all implcit scopes opened by it.

VERB_bytecode_t VERB_bytecode_op_eof(char* restrict* const restrict string, VERB_tokeniser_t* const restrict tokeniser){
    while(tokeniser->old_whitespace_cnt--) VERB_tokeniser_backend_scope_del(&tokeniser->backend);
    return VERB_BC_special_IGNORE;
}

#endif
