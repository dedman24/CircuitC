#ifndef VERB_preprocessor_ops_error_included
#define VERB_preprocessor_ops_error_included

#include "_includes.h"
#include "../../../libraries/errors_warnings/errors_warnings.h"

// #error ~ throws error to compiler, at which point compilation stops.
// string stuff in C is so miserable.

VERB_token_t VERB_preprocessor_op_error(char* restrict* const restrict string, VERB_tokeniser_t* const restrict tokeniser){
// incorrect formatting
    if(!VERB_preprocessor_op__check_for_whitespace(string, "#error", tokeniser)) return VERB_TOKEN_special_IGNORE;
    if(**string != '"'){
        VERB_error_report(tokeniser->specifics, VERB_error_invalid_statement, tokeniser->line, tokeniser->offset, 1, "NO QUOTES BEFORE WARNING STRING IN #error");

        char* const new_string = strchr(*string, '\n');
        *string = new_string? new_string: *string + strlen(*string);
        return VERB_TOKEN_special_IGNORE;
    }
    ++**string; ++tokeniser->offset;

    char* const startoferror = *string;
    char* const endoferror = strchr(*string, '"');
    if(!endoferror){
        VERB_error_report(tokeniser->specifics, VERB_error_invalid_statement, tokeniser->line, tokeniser->offset, 1, "NO QUOTES AFTER WARNING STRING IN #error");
        *string += strlen(*string);                 // goes to end of string; no proper error message can be assembled, none should be.
        return VERB_TOKEN_special_IGNORE;
    }
    const size_t error_len = (size_t)(endoferror - startoferror);
    *string += error_len + 1;                       // + 1 so it skips over the last quote character.
// null-terminates error string.
    startoferror[error_len] = 0;
    VERB_error_report(tokeniser->specifics, VERB_error_custom, tokeniser->line, tokeniser->offset, 1, startoferror);
    startoferror[error_len] = '"';

    return VERB_TOKEN_special_IGNORE;
}

#endif
