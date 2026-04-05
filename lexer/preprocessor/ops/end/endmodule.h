#ifndef VERB_preprocessor_ops_endmodule_included
#define VERB_preprocessor_ops_endmodule_included

#include "../_includes.h"

// #endmodule
// used to terminate #module statements, ignored otherwise.

void VERB_preprocessor_skip_endmodule(char* restrict* const restrict string, VERB_tokeniser_t* const restrict tokeniser){
    VERB_preprocessor_strings_t toincrease[] = {
        VERB_preprocessor_op_skip_entry(module)
    };

    VERB_preprocessor_strings_t todecrease[] = {
        VERB_preprocessor_op_skip_entry(endmodule)
    };

    VERB_preprocessor_op__skip_nested(toincrease, sizeof(toincrease)/sizeof(*toincrease), todecrease, sizeof(todecrease)/sizeof(*todecrease), string, tokeniser);
}

void VERB_preprocessor_op_endmodule(char* restrict* const restrict string, VERB_tokeniser_t* const restrict tokeniser){
    VERB_warning_report(tokeniser->specifics, "#endmodule", VERB_warning_stray_preprocessor_directive, tokeniser->line, tokeniser->offset);
}

#endif
