#ifndef VERB_preprocessor_ops_module_included
#define VERB_preprocessor_ops_module_included

#include "_includes.h"
#include "endmodule.h"

// modules!!! or at least I think these are modules.
// checks if specific string is in module list, otherwise skips to #endmodule.

VERB_token_t VERB_preprocessor_op_module(char* restrict* const restrict string, VERB_tokeniser_t* const restrict tokeniser){
    if(!VERB_preprocessor_op__check_for_whitespace(string, "#module", tokeniser)) return VERB_TOKEN_special_IGNORE;
// gets length of module name
    const size_t name_len = VERB_REGEX_statement_length(*string);
    if(!name_len){ 
        VERB_error_report(tokeniser->specifics, VERB_error_preprocessor_no_name_after, tokeniser->line, tokeniser->offset, 1, "#module");
        return VERB_TOKEN_special_IGNORE;
    }

    if(!VERB_tree_boolean_del(&tokeniser->preprocessor->modules, *string, name_len, VERB_tree_keep_key))
        VERB_preprocessor_skip_endmodule(string, tokeniser);
        
    return VERB_TOKEN_special_IGNORE;
}


#endif
