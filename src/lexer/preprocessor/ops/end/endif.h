#ifndef VERB_preprocessor_ops_endif_included
#define VERB_preprocessor_ops_endif_included

// skips all statements & nested #ifdef s until the proper #endif

#include "stdbool.h"
#include "../_includes.h"

// setTrueOnlyForElse ~ false if we should consider #else a 'valid string to decrement on' (most statements), false otherwise (#else & similar).
void VERB_preprocessor_op_skip_endif(char* restrict* const restrict string, VERB_tokeniser_t* const restrict tokeniser, const bool setTrueOnlyForElse){    
    VERB_preprocessor_strings_t toincrease[] = {
        VERB_preprocessor_op_skip_entry(ifdef),
        VERB_preprocessor_op_skip_entry(ifndef),
        VERB_preprocessor_op_skip_entry(if),
    };
    VERB_preprocessor_strings_t todecrease[] = {
        VERB_preprocessor_op_skip_entry(endif),
        VERB_preprocessor_op_skip_entry(else),
        VERB_preprocessor_op_skip_entry(elseif),
        VERB_preprocessor_op_skip_entry(elif)
    };
    const int number_of_elses = 3;

    VERB_preprocessor_op__skip_nested(
        toincrease, sizeof(toincrease)/sizeof(*toincrease), 
        todecrease, sizeof(todecrease)/sizeof(*todecrease) - number_of_elses*setTrueOnlyForElse, 
        string, tokeniser
    );
}

void VERB_preprocessor_op_endif(char* restrict* const restrict string, VERB_tokeniser_t* const restrict tokeniser){
    VERB_warning_report(tokeniser->specifics, "#endif", VERB_warning_stray_preprocessor_directive, tokeniser->line, tokeniser->offset);
}

#endif
