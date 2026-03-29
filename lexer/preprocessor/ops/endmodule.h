#ifndef VERB_preprocessor_ops_endmodule_included
#define VERB_preprocessor_ops_endmodule_included

#include "_includes.h"

// #endmodule
// used to terminate #module statements, ignored otherwise.

void VERB_preprocessor_skip_endmodule(char* restrict* const restrict string, VERB_tokeniser_t* const restrict tokeniser){
    VERB_preprocessor_strings_t toincrease[] = {
        { "#module", strlen("#module") },
    };

    VERB_preprocessor_strings_t todecrease[] = {
        { "#endmodule", strlen("#endmodule") }
    };

    VERB_preprocessor_op__skip_nested(toincrease, sizeof(toincrease)/sizeof(*toincrease), todecrease, sizeof(todecrease)/sizeof(*todecrease), string, tokeniser);
}

VERB_token_t VERB_preprocessor_op_endmodule(char* restrict* const restrict string, VERB_tokeniser_t* const restrict tokeniser){
    return VERB_TOKEN_special_IGNORE;                           // random #endonce statements in the middle of the code are just ignored
}

#endif
