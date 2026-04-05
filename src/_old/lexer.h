#ifndef VERB_lexer_included
#define VERB_lexer_included

#include "../libraries/errors_warnings/errors_warnings.h"       // error handling.
#include "../libraries/value_conversion.h"                      // value conversion operations.
#include "../libraries/dynamic_arrays.h"                        // dynamic array type & operations.
#include "tokeniser/backend/tokeniser_type.h"
#include "tokeniser/tokeniser.h"                                // getting token from a string.

// a lexer converts human-readable VERB code into a computer-readable representation of said code (VERB bytecode).

VERB_variable_token_t VERB_lexer_line_tungutnguntg(char* restrict* const restrict string, VERB_tokeniser_t* const restrict tokeniser, VERB_array_t* const restrict functionArgs){
// gets longest string terminated by the valid terminator symbols.
    const char* const restrict line_terminating_symbols = ".";
    char* const end = *string + strcspn(*string, line_terminating_symbols);
// incremented each time '(' is met, decremented each time ')' is met.
    uint32_t current_offset = 0;
// >= so the final period is processed too.
    while(**string){
        VERB_token_get(string, tokeniser);
        if(tok == VERB_BC_special_IGNORE) continue;

        if(tok >= VERB_BC_special_NAME_START){
            
            continue;
        }

        const VERB_bytecode_t opcode = tok;
        VERB_tokeniser_operator(tokeniser);
        if(opcode == VERB_BC_special_OPENED_ROUND_BRACKET) current_offset++;
        else if(opcode == VERB_BC_special_CLOSED_ROUND_BRACKET){
            if(!current_offset) break;
            else current_offset--;
            
            VERB_bytecode_t op = VERB_array_pop_type(VERB_bytecode_t, &ops);
            if(op == VERB_BC_special_OPENED_ROUND_BRACKET)
                VERB_error_report(tokeniser->specifics, VERB_error_invalid_statement, tokeniser->line, tokeniser->offset, 1, "OPENED PARENTHESIS IMMEDIATELY TERMINATED BY CLOSED ONE WITH NO EXPRESSION NOR VALUE");
            
            while(op != VERB_BC_special_OPENED_ROUND_BRACKET){
                VERB_lexer_line_process_element(tokeniser, op, current_offset, &ops, &values);
                op = VERB_array_pop_type(VERB_bytecode_t, &ops);
            } 
            
            continue;
        }
        if(functionArgs && !current_offset && opcode == VERB_BC_comma){
            VERB_lexer_line_isOpcode(tokeniser, VERB_BC_farg, current_offset, &ops, &values);
            const VERB_variable_token_t arg = VERB_array_pop_type(VERB_variable_token_t, &values);
            VERB_array_push(arg, functionArgs);

            continue;
        } 

        VERB_lexer_line_isOpcode(tokeniser, opcode, current_offset, &ops, &values);             // implicit conversion of first arg is intended!
    }

    while(!VERB_array_is_empty(&ops)){                                                          // consumes all ops.
        const VERB_bytecode_t op = VERB_array_pop_type(VERB_bytecode_t, &ops);
        VERB_lexer_line_process_element(tokeniser, op, current_offset, &ops, &values);
    }
// useful for const-typed functions & similar.
    const VERB_variable_token_t result = VERB_array_top_safe(VERB_variable_token_t, &values);

    VERB_array_destroy(&ops, VERB_array_keep_ctx);
    VERB_array_destroy(&values, VERB_array_keep_ctx);

    return result;
}


// given string of VERB code, converts it to string of tokens that has to be freed by user
void* VERB_lexer(char* restrict string, VERB_error_specifics_t** restrict specifics, size_t* const restrict tokenised_code_len){
// static initialization? it doesn't really matter which one I use in the end.
    VERB_tokeniser_t tokeniser; VERB_tokeniser_init(&tokeniser);

    while(*string) VERB_lexer_line(&string, &tokeniser, false);
// DEBUG INFO: in theory this function SHOULD be called but as of 3/26/2026 nothing changes if we do not.
    if(false) VERB_bytecode_op_eof(&string, &tokeniser);
    *specifics = tokeniser.specifics; tokeniser.specifics = NULL;

    *tokenised_code_len = VERB_array_extract_length(tokeniser.code);
    void* tokenised_string = VERB_array_extract(tokeniser.code);
    VERB_tokeniser_destroy(&tokeniser, VERB_tokeniser_keep_ctx);

    return tokenised_string;
}

#endif
