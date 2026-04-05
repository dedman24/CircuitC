#ifndef VERB_preprocessor_ops_elseif_included
#define VERB_preprocessor_ops_elseif_included

#include "../_includes.h"
#include "../end/endif.h"
#include "../replacement/primitives.h"
#include "../../../tokeniser/variables/arith.h"

// implements #elseif, #elif.

void VERB_preprocessor_op_elseif(char* restrict* const restrict string, VERB_tokeniser_t* const restrict tokeniser){
    if(tokeniser->preprocessor->if_state == VERB_preprocessor_if_true){
        VERB_preprocessor_op_skip_endif(string, tokeniser, true);
        tokeniser->preprocessor->if_state = VERB_preprocessor_if_none;
        return;
    } 

    VERB_lexer_line(string, tokeniser, ".");
    const VERB_variable_token_t result = VERB_array_pop_type(VERB_variable_token_t, &tokeniser->values);

    VERB_variable_t* const restrict def = VERB_variable_search_byToken(&tokeniser->backend, result);
    if(!def || def->group != VERB_VARIABLE_ARITH){
        VERB_error_report(tokeniser->specifics, VERB_error_invalid_statement, tokeniser->line, tokeniser->offset, 1, "INVALID SYMBOL IN #if; NOT SOMETHING THAT CAN EVALUATE TO TRUE");
    }

    VERB_variable_arith_t* const restrict arithmetic = def->custom_data;
    if(!VERB_variable_arith_truthy(arithmetic)){
        tokeniser->preprocessor->if_state = VERB_preprocessor_if_false;
        VERB_preprocessor_op_skip_endif(string, tokeniser, true);
    }
    else tokeniser->preprocessor->if_state = VERB_preprocessor_if_true;
}

// #elsif, #elif are aliases of #elseif.
#define VERB_preprocessor_op_elsif VERB_preprocessor_op_elseif
#define VERB_preprocessor_op_elif  VERB_preprocessor_op_elseif

#endif
