#ifndef VERB_preprocessor_ops_endonce_included
#define VERB_preprocessor_ops_endonce_included

#include "../_includes.h"

// #endonce
// used to terminate #once statements, ignored otherwise.

// very ugly but ICBA.
char* VERB_preprocessor_skip_endonce(char* restrict* const restrict string, VERB_tokeniser_t* const restrict tokeniser){
    char* restrict* new_string = string;

    VERB_preprocessor_strings_t toincrease[] = {
        VERB_preprocessor_op_skip_entry(once)
    };
    VERB_preprocessor_strings_t todecrease[] = {
        VERB_preprocessor_op_skip_entry(endonce)
    };

    VERB_preprocessor_op__skip_nested(toincrease, sizeof(toincrease)/sizeof(*toincrease), todecrease, sizeof(todecrease)/sizeof(*todecrease), new_string, tokeniser);
    return *new_string;
}

void VERB_preprocessor_op_endonce(char* restrict* const restrict string, VERB_tokeniser_t* const restrict tokeniser){
    VERB_warning_report(tokeniser->specifics, "#endonce", VERB_warning_stray_preprocessor_directive, tokeniser->line, tokeniser->offset);
}

#endif
