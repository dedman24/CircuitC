#ifndef VERB_bytecode_op_if_included
#define VERB_bytecode_op_if_included

#include "../_includes.h"
#include "../_primitives/primitives.h"

static void VERB_bytecode_op_if(char* restrict* const restrict string, VERB_tokeniser_t* const restrict tokeniser){
    VERB_op__opcode(tokeniser, VERB_BC_if);

    VERB_tokeniser_skip_whitespace(string, tokeniser);
    if(**string != '('){                                // condition handles itself.
        VERB_error_report(tokeniser->specifics, VERB_error_invalid_statement, tokeniser->line, tokeniser->offset, 1, "NO OPEN ROUND BRACKET '(' AFTER 'if' STATEMENT");
        return;
    }
}

static void VERB_bytecode_op_else(char* restrict* const restrict string, VERB_tokeniser_t* const restrict tokeniser){
    VERB_op__opcode(tokeniser, VERB_BC_else);
}

#endif
