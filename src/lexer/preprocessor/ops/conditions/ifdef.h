#ifndef VERB_preprocessor_ops_ifdef_included
#define VERB_preprocessor_ops_ifdef_included

#include "stdbool.h"
#include "../_includes.h"
#include "../end/endif.h"

void VERB_preprocessor_op_ifdef(char* restrict* const restrict string, VERB_tokeniser_t* const restrict tokeniser){
    if(!VERB_preprocessor_op__check_for_whitespace(string, "#ifdef", tokeniser)) return;
// gets length of name
    const size_t name_len = VERB_REGEX_statement_length(*string);
    if(!name_len){ 
        VERB_error_report(tokeniser->specifics, VERB_error_preprocessor_no_name_after, tokeniser->line, tokeniser->offset, 1, "#ifdef");
        return;
    }
    
    tokeniser->preprocessor->if_state = VERB_rht_search_bool(tokeniser->preprocessor->defined_things, *string, name_len)?
        VERB_preprocessor_if_true: VERB_preprocessor_if_false;
    *string += name_len;

    if(tokeniser->preprocessor->if_state == VERB_preprocessor_if_false)
        VERB_preprocessor_op_skip_endif(string, tokeniser, false);
        
    return;
}

#endif
