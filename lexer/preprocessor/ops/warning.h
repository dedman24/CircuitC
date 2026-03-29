#ifndef VERB_preprocessor_ops_warning_included
#define VERB_preprocessor_ops_warning_included

#include "_includes.h"
#include "../../../libraries/errors_warnings/errors_warnings.h"

// #warning ~ reports warning.
// how do we handle warnings? I didn't really think of this.
// I perhaps might have to overhaul how error handling works. I hope not, but currently it's anything but elegant.

VERB_token_t VERB_preprocessor_op_warning(char* restrict* const restrict string, VERB_tokeniser_t* const restrict tokeniser){
// incorrect formatting
    if(!VERB_preprocessor_op__check_for_whitespace(string, "#warning", tokeniser)) return VERB_TOKEN_special_IGNORE;
    if(**string != '"'){
        VERB_error_report(tokeniser->specifics, VERB_error_invalid_statement, tokeniser->line, tokeniser->offset, 1, "NO QUOTES BEFORE WARNING STRING IN #warning");

        char* const new_string = strchr(*string, '\n');
        *string = new_string? new_string: *string + strlen(*string);
        return VERB_TOKEN_special_IGNORE;
    }
    ++**string; ++tokeniser->offset;

    char* const startofwarning = *string;
    char* const endofwarning = strchr(*string, '"');
    if(!endofwarning){
        VERB_error_report(tokeniser->specifics, VERB_error_invalid_statement, tokeniser->line, tokeniser->offset, 1, "NO QUOTES AFTER WARNING STRING IN #warning");
        *string += strlen(*string);                 // goes to end of string; no proper error message can be assembled, none should be.
        return VERB_TOKEN_special_IGNORE;
    }
    const size_t warning_len = (size_t)(endofwarning - startofwarning);
    *string += warning_len + 1;                     // + 1 so it skips over the last quote character.
// null-terminates warning string.
    startofwarning[warning_len] = 0;
    VERB_warning_report(tokeniser->specifics, startofwarning, VERB_warning_custom, tokeniser->line, tokeniser->offset);
    startofwarning[warning_len] = '"';

    return VERB_TOKEN_special_IGNORE;
}

#endif
