#ifndef VERB_preprocessor_ops_endif_included
#define VERB_preprocessor_ops_endif_included

// skips all statements & nested #ifdef s until the proper #endif

#include "stdbool.h"
#include "_includes.h"

void VERB_preprocessor_op_skip_endif(char* restrict* const restrict string, VERB_tokeniser_t* const restrict tokeniser){
    VERB_preprocessor_strings_t toincrease[] = {
        { "#ifdef",  strlen("#ifdef")  },
        { "#ifndef", strlen("#ifndef") },
        { "#else",   strlen("#else")   },
    };

    VERB_preprocessor_strings_t todecrease[] = {
        { "#endif", strlen("#endif") }
    };

    VERB_preprocessor_op__skip_nested(toincrease, sizeof(toincrease)/sizeof(*toincrease), todecrease, sizeof(todecrease)/sizeof(*todecrease), string, tokeniser);
}

VERB_token_t VERB_preprocessor_op_endif(char* restrict* const restrict string, VERB_tokeniser_t* const restrict tokeniser){
    return VERB_TOKEN_special_IGNORE;
}

#endif
