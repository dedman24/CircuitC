#ifndef VERB_preprocessor_ops_endmacro_included
#define VERB_preprocessor_ops_endmacro_included

// skips all statements & nested #ifdef s until the proper #endif

#include "stdbool.h"
#include "../_includes.h"

char* VERB_preprocessor_op_skip_endmacro(char* const restrict string, char** const restrict exclusive, VERB_tokeniser_t* const restrict tokeniser){
    char* newstring = string;

    VERB_preprocessor_strings_t toincrease[] = {
        VERB_preprocessor_op_skip_entry(macro)
    };
    VERB_preprocessor_strings_t todecrease[] = {
        VERB_preprocessor_op_skip_entry(endmacro)
    };

    char* const r = VERB_preprocessor_op__skip_nested(toincrease, sizeof(toincrease)/sizeof(*toincrease), todecrease, sizeof(todecrease)/sizeof(*todecrease), &newstring, tokeniser);
    if(exclusive) *exclusive = r;
    return newstring;
}

void VERB_preprocessor_op_endmacro(char* restrict* const restrict string, VERB_tokeniser_t* const restrict tokeniser){
    VERB_warning_report(tokeniser->specifics, "#endmacro", VERB_warning_stray_preprocessor_directive, tokeniser->line, tokeniser->offset);
}

#endif