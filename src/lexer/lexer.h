#ifndef VERB_lexer_included
#define VERB_lexer_included

#include "../libraries/errors_warnings/errors_warnings.h"       // error handling.
#include "../libraries/arrays/dynamic_arrays.h"                 // dynamic array type & operations.
#include "../libraries/value_conversion.h"                      // value conversion operations.
#include "tokeniser/backend/tokeniser_type.h"
#include "tokeniser/tokeniser.h"                                // getting token from a string.
#include "string.h"

// a lexer converts human-readable VERB code into a computer-readable representation of said code (VERB bytecode).

static void VERB_lexer(char* restrict* const restrict string, VERB_tokeniser_t* const restrict tokeniser, const char* const restrict terminating_symbols){
    while(!strchr(terminating_symbols, **string)) VERB_token_get(string, tokeniser);
}

static void VERB_lexer_bracket_depth(char* restrict* const restrict string, VERB_tokeniser_t* const restrict tokeniser, const char terminating_symbol){
    const unsigned long long terminating_bracket_depth = tokeniser->bracket_depth;
    while(
        **string && 
        **string != terminating_symbol && 
        (tokeniser->bracket_depth > terminating_bracket_depth || **string != ')')
    ) VERB_token_get(string, tokeniser);
}

static void VERB_lexer_scope_depth(char* restrict* const restrict string, VERB_tokeniser_t* const restrict tokeniser, const char terminating_symbol){
    const unsigned long long terminating_scope_depth = tokeniser->backend.scope_depth;
    while(
        **string && 
        **string != terminating_symbol && 
        (tokeniser->backend.scope_depth > terminating_scope_depth || **string != '}')
    ) VERB_token_get(string, tokeniser);
}

// given string of VERB code, converts it to string of tokens that has to be freed by user
void* VERB_lex_all(char* restrict string, VERB_error_specifics_t** restrict specifics, size_t* const restrict tokenised_code_len){
// static initialization? it doesn't really matter which one I use in the end.
    VERB_tokeniser_t tokeniser; VERB_tokeniser_init(&tokeniser);

    VERB_lexer(&string, &tokeniser, "");
// DEBUG INFO: in theory this function SHOULD be called but as of 3/26/2026 nothing changes if we do not.
    if(false) VERB_bytecode_op_eof(&string, &tokeniser);
    *specifics = tokeniser.specifics; tokeniser.specifics = NULL;

    *tokenised_code_len = VERB_array_extract_length(tokeniser.code);
    void* tokenised_string = VERB_array_extract(tokeniser.code);
    VERB_tokeniser_destroy(&tokeniser, VERB_tokeniser_keep_ctx);

    return tokenised_string;
}

#endif
