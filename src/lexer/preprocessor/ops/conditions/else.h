#ifndef VERB_preprocessor_ops_else_included
#define VERB_preprocessor_ops_else_included

#include "../end/endif.h"
#include "../_includes.h"
#include "stdbool.h"

void VERB_preprocessor_op_else(char* restrict* const restrict string, VERB_tokeniser_t* const restrict tokeniser){
// #else skip to #endif when VERB_preprocessor_if_true is set (last if statement evaluated truthy).
    if(tokeniser->preprocessor->if_state == VERB_preprocessor_if_true){
        VERB_preprocessor_op_skip_endif(string, tokeniser, true);
        tokeniser->preprocessor->if_state = VERB_preprocessor_if_none;
    }
}

#endif
