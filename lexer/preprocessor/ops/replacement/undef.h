#ifndef VERB_preprocessor_ops_undef_included
#define VERB_preprocessor_ops_undef_included

#include "stdbool.h"
#include "../_includes.h"

void VERB_preprocessor_op_undef(char* restrict* const restrict string, VERB_tokeniser_t* const restrict tokeniser){
    if(!VERB_preprocessor_op__check_for_whitespace(string, "#undef", tokeniser)) return;
// gets length of name
    const size_t name_len = VERB_REGEX_statement_length(*string);
    if(!name_len){ 
        VERB_error_report(tokeniser->specifics, VERB_error_preprocessor_no_name_after, tokeniser->line, tokeniser->offset, 1, "#undef");
        return;
    }
// deletes name from list of defined things
    VERB_rht_del(tokeniser->preprocessor->defined_things, *string, name_len);
    *string += name_len;    
}

#endif
