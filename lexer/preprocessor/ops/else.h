#ifndef VERB_preprocessor_ops_else_included
#define VERB_preprocessor_ops_else_included

#include "endif.h"
#include "stdbool.h"
#include "_includes.h"

VERB_token_t VERB_preprocessor_op_else(char* restrict* const restrict string, VERB_tokeniser_t* const restrict tokeniser){
// #else skip over all other statements when VERB_preprocessor_if_true is set  
    if(tokeniser->preprocessor->if_state == VERB_preprocessor_if_true){
        VERB_preprocessor_op_skip_endif(string, tokeniser);
        tokeniser->preprocessor->if_state = VERB_preprocessor_if_none;
    }
        
    return VERB_TOKEN_special_IGNORE;
}

#endif
