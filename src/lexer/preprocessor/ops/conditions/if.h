#ifndef VERB_preprocessor_ops_if_included
#define VERB_preprocessor_ops_if_included

#include "../_includes.h"
#include "../end/endif.h"
#include "../replacement/primitives.h"
#include "../../../tokeniser/variables/arith.h"

// implements #if.

void VERB_preprocessor_op_if(char* restrict* const restrict string, VERB_tokeniser_t* const restrict tokeniser){
    VERB_lexer_line(string, tokeniser, ".");
    const VERB_variable_token_t result = VERB_array_pop_type(VERB_variable_token_t, &tokeniser->values);

    VERB_variable_t* const restrict def = VERB_variable_search_byToken(&tokeniser->backend, result);
    if(!def || !(def->flags & VERB_VARIABLE_FLAG_const) || def->group != VERB_VARIABLE_ARITH){
        VERB_error_report(tokeniser->specifics, VERB_error_invalid_statement, tokeniser->line, tokeniser->offset, 1, "INVALID SYMBOL IN #if; NOT SOMETHING THAT CAN BE EVALUATED TO TRUE");
        VERB_preprocessor_op_skip_endif(string, tokeniser, false);
        return;
    }

    VERB_variable_arith_t* const restrict arithmetic = def->custom_data;
    if(!VERB_variable_arith_truthy(arithmetic)){
        tokeniser->preprocessor->if_state = VERB_preprocessor_if_false;
        VERB_preprocessor_op_skip_endif(string, tokeniser, false);
    }
    else tokeniser->preprocessor->if_state = VERB_preprocessor_if_true;

    return;
}



#endif
