#ifndef VERB_preprocessor_ops_endonce_included
#define VERB_preprocessor_ops_endonce_included

#include "_includes.h"

// #endonce
// used to terminate #once statements, ignored otherwise.

// very ugly but ICBA.
char* VERB_preprocessor_skip_endonce(char* restrict* const restrict string, VERB_tokeniser_t* const restrict tokeniser){
    char* restrict* new_string = string;

    VERB_preprocessor_strings_t toincrease[] = {
        { "#once", strlen("#once") },
    };

    VERB_preprocessor_strings_t todecrease[] = {
        { "#endonce", strlen("#endonce") }
    };

    VERB_preprocessor_op__skip_nested(toincrease, sizeof(toincrease)/sizeof(*toincrease), todecrease, sizeof(todecrease)/sizeof(*todecrease), new_string, tokeniser);
    return *new_string;
}

VERB_token_t VERB_preprocessor_op_endonce(char* restrict* const restrict string, VERB_tokeniser_t* const restrict tokeniser){
    return VERB_TOKEN_special_IGNORE;                           // random #endonce statements in the middle of the code are just ignored
}

#endif
