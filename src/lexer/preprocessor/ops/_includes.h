#ifndef VERB_preprocessor_ops__includes_included
#define VERB_preprocessor_ops__includes_included

// includes

#include "../../../libraries/errors_warnings/errors_warnings.h"     // error handling.
#include "../../../libraries/arrays/dynamic_arrays.h"               // dynamic arrays.
#include "../../tokeniser/backend/tokeniser_ctx.h"                  // VERB_tokeniser_t type. 
#include "../../../libraries/regex_stuff.h"                         // regex operations to check if. 
#include "../../../libraries/rhts/rht.h"                            // recursive hash table & ops.
#include "../../../libraries/scope.h"                               // scope handling.
#include "../../../tokens/tokens.h"                                 // token type.
#include "stdbool.h"                                                // bool type.
#include "stddef.h"                                                 // size_t.
#include "stdint.h"                                                 // fixed-width types.
#include "string.h"                                                 // string operations.


// function primitives
void VERB_lexer(char* restrict* const restrict, VERB_tokeniser_t* const restrict, const char* const restrict);

// useful macros.

#define VERB_preprocessor_op_skip_entry(x) { #x, strlen(#x) }

// useful functions

// returns false on errors
bool VERB_preprocessor_op__check_for_whitespace(char* restrict* const restrict string, char* const statement, VERB_tokeniser_t* restrict tokeniser){
    const size_t whitespace_len = VERB_REGEX_whitespace_length(*string);
    if(!whitespace_len) {
        VERB_error_report(tokeniser->specifics, VERB_error_preprocessor_no_whitespace, tokeniser->line, tokeniser->offset, 1, statement);
// skips over the #define statement.
        char* const end_of_define = strchr(*string, '\n');
        const size_t area_to_skip_over = end_of_define? (size_t)(end_of_define - *string): sizeof(*string);
        *string += area_to_skip_over; tokeniser->offset += area_to_skip_over;       // doesn't skip over newline as newlines have special handling.
        return false;
    }
    *string += whitespace_len; tokeniser->offset += whitespace_len;
    return true;
}

typedef struct{
    char* const restrict string;
    const size_t len;
} VERB_preprocessor_strings_t;

bool VERB_preprocessor_op__increase_nesting_depth(char* restrict* const restrict string, const size_t len, VERB_preprocessor_strings_t* toincrease, const size_t el){
    for(size_t i = 0; i < el; i++){
        if(len != toincrease[i].len) continue;
        if(memcmp(*string, toincrease[i].string, len)) return true;
    }
    return false;
}

bool VERB_preprocessor_op__decrease_nesting_depth(char* restrict* const restrict string, const size_t len, VERB_preprocessor_strings_t* todecrease, const size_t el){
    for(size_t i = 0; i < el; i++){
        if(len != todecrease[i].len) continue;
        if(memcmp(*string, todecrease[i].string, len)) return true; 
    }
    return false;
}

// skips over relevant todecrease string, returns ptr to end of it.
char* VERB_preprocessor_op__skip_nested(VERB_preprocessor_strings_t* const restrict toincrease, const size_t icnt, VERB_preprocessor_strings_t* const restrict todecrease, const size_t dcnt, char* restrict* const restrict string, VERB_tokeniser_t* const restrict tokeniser){
// initialised to 1 because we're 1 nested statement deep
    unsigned long long nesting_depth = 1;
// not the most efficient way to search for given character sequences
    char* exclusive = NULL;
    do{
        size_t len;
        switch(**string){
        case '#':
            exclusive = *string;
            *string += 1; tokeniser->offset += 1;
            VERB_tokeniser_skip_whitespace(string, tokeniser);
            len = VERB_REGEX_statement_length(*string);
            if(VERB_preprocessor_op__increase_nesting_depth(string, len, toincrease, icnt)) nesting_depth++;
            else if(VERB_preprocessor_op__decrease_nesting_depth(string, len, todecrease, dcnt)) nesting_depth--; 
            break;
        case '\n':
            *string += 1; VERB_bytecode_op_newline_proper_behaviour(tokeniser);
        default:
            len = strcspn(*string, "#\n");      // skips to next line or preprocessor directive.
            break;
        }
        *string += len; tokeniser->offset += len;
    } while(nesting_depth && **string);

    return exclusive;
}

#endif
