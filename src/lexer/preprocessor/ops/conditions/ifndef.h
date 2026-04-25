#ifndef VERB_preprocessor_ops_ifndef_included
#define VERB_preprocessor_ops_ifndef_included

#include "stdbool.h"        // boolean types to make code clearer
#include "../_includes.h"
#include "../end/endif.h"

/*
// the function in this file looks like this in VERB.
// it is really ugly.
// I might change it.
fn ifndef(str string, tokeniser_t !acq tokeniser) ~ (str, tokeniser_t){
    bool success, tokeniser_t updated_tok, newstr = _check_for_whitespace(string, "#ifndef", tokeniser).
    if(!success) return updated_tok, newstr.

    size_t namelen = REGEX~statement_length(newstr).
    if !namelen 
        return newstr, errors~report(tokeniser).
    tokeniser_t updated_if_state = 
        updated_tok~preprocessor~if_state :- updated_tok~preprocessor~defined_things~search(newstr, name_len)? true: false.
    
    return 
        if updated_if_state~preprocessor~if_state do newstr + namelen
        else VERB_preprocessor_op_skip_endif(newstr, updated_if_state, false),
        updated_if_state.
}
*/

void VERB_preprocessor_op_ifndef(char* restrict* const restrict string, VERB_tokeniser_t* const restrict tokeniser){
    if(!VERB_preprocessor_op__check_for_whitespace(string, "#ifndef", tokeniser)) return;
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
