#ifndef CIRCUITC_tokeniser_included
#define CIRCUITC_tokeniser_included

#include "stdbool.h"            // boolean type
#include "stdlib.h"             // dynamic memory ops
#include "stdint.h"             // variable-width types
#include "string.h"             // strstr
#include "tokens.h"             // token type, tokens themselves
#include "bst.h"                // binary search tree type & ops

// all tokens
typedef struct{
    CIRCUITC_tree_t* keywords;
    CIRCUITC_tree_t* comments;
    CIRCUITC_tree_t* whitespaces;
} CIRCUITC_tokeniser_t;

typedef enum{ CIRCUITC_tokeniser_keep_ctx, CIRCUITC_tokeniser_free_ctx } CIRCUITC_tokeniser_options_t;

CIRCUITC_tokeniser_t* CIRCUITC_tokeniser_init(CIRCUITC_tokeniser_t* ctx){
    if(!ctx) ctx = malloc(sizeof(*ctx));

    ctx->keywords = CIRCUITC_token_keywords_init();
    ctx->comments = CIRCUITC_token_comments_init();
    ctx->whitespaces = CIRCUITC_token_whitespaces_init();

    return ctx;
}

void CIRCUITC_tokeniser_destroy(CIRCUITC_tokeniser_t* ctx, CIRCUITC_tokeniser_options_t freectx){
    CIRCUITC_tree_destroy(ctx->keywords, CIRCUITC_tree_free_key);
    CIRCUITC_tree_destroy(ctx->comments, CIRCUITC_tree_free_key);
    CIRCUITC_tree_destroy(ctx->whitespaces, CIRCUITC_tree_free_key);

    if(freectx == CIRCUITC_tokeniser_free_ctx) free(ctx);
}

// skips comment
char* CIRCUITC_tokeniser_comment_skip(char* string, const CIRCUITC_token_t comment){
    const CIRCUITC_token_t type = CIRCUITC_COMMENT_TYPE_GET(comment);
    const char* closing_comment_symbol = CIRCUITC_COMMENT_CLOSING_INDEX_GET(comment);

    char* newstring = strstr(string, closing_comment_symbol);
    if(newstring == NULL) return string + strlen(string);       // comment is closed by \x00; next call to CIRCUITC_token_get will return CIRCUITC_TOKEN_EOF and lexing will be halted
    else return newstring + strlen(closing_comment_symbol);
}

// the regex being [a-zA-Z] or [a-zA-Z0-9] if 'position' is nonzero
bool CIRCUITC_tokeniser_REGEX_alphanumeric_check(const char character, const size_t position){
    return (position && 0x30 <= character && character <= 0x39) || (0x41 <= character && character <= 0x5A) || (0x61 <= character && character <= 0x7A);
}

// returns length of largest substring whose start is 'string' that is accepted the following regex: [a-zA-Z][a-zA-Z0-9]*
// if no string is accepted by said REGEX, it returns 0.
size_t CIRCUITC_tokeniser_REGEX_alphanumeric_length(char* string){
    size_t length = 0;
    while(CIRCUITC_tokeniser_REGEX_alphanumeric_check(string[length], length)) length++;
    return length;
}

// the regex being [0-9] or or [a-zA-Z0-9] if 'position' is nonzero
bool CIRCUITC_tokeniser_REGEX_numeric_check(const char character, const size_t position){
    return ((0x41 <= character && character <= 0x5A) || (0x61 <= character && character <= 0x7A)) && position || (0x30 <= character && character <= 0x39);
}

// returns length of largest substring whose start is 'string' that is accepted the following regex:[0-9][a-zA-Z0-9]*
// if no string is accepted by said REGEX, it returns 0.
size_t CIRCUITC_tokeniser_REGEX_numeric_length(char* string){
    size_t length = 0;
    while(CIRCUITC_tokeniser_REGEX_numeric_check(string[length], length)) length++;
    return length;
}

// parses statement
CIRCUITC_token_t CIRCUITC_tokeniser_parse_statement(char** string, CIRCUITC_tokeniser_t* tokeniser, size_t* nameval_token_length){
// checks if it is an alphanumeric string
    size_t length = CIRCUITC_tokeniser_REGEX_alphanumeric_length(*string);

    CIRCUITC_tree_error_code_t error_code; 
    CIRCUITC_token_t token;

    if(length){
        token = CIRCUITC_tree_search(tokeniser->keywords, *string, length, &error_code);

        if(error_code == CIRCUITC_tree_no_error){
            *string += length;
            return token;                                       // all strings accepted by regex in 'keywords' return the token they're assigned by 'keywords' here
        } 

        *nameval_token_length = length;                         // handles all function and variable names
        return CIRCUITC_TOKEN_NAME;
    } 
// checks if it is an operator (+ - , et cetera)
    char symbol = (*string)[0];
    token = CIRCUITC_tree_search(tokeniser->keywords, &symbol, sizeof(symbol), &error_code);
    if(error_code == CIRCUITC_tree_no_error){
// if token allows for duplication, and said duplication is present, return duplicated token
        if(CIRCUITC_TOKEN_METADATA_GET(token) == CIRCUITC_TOKEN_METADATA_DUPLICATION && string[1] == string[0])
            return CIRCUITC_tree_search(tokeniser->keywords, *string, 2, NULL);

        return token;
    }
// it is therefore a number, and we check for its length
    length = CIRCUITC_tokeniser_REGEX_numeric_length(*string);

    *nameval_token_length = length;
    return CIRCUITC_TOKEN_VALUE;
}

void CIRCUITC_tokeniser_update_string_positition(char** string, const uint64_t length_of_symbol){
    *string += length_of_symbol;
}

// from string, sees if it can be converted to token, does so if possible, returns new string (advanced).
// nameval_token_length ~ ptr to size_t variable that holds length of byte string following <name> or <value> token in bytes, limited to 256.
// undefined value if token is not CIRCUITC_TOKEN_NAME or CIRCUITC_TOKEN_VALUE.
CIRCUITC_token_t CIRCUITC_token_get(char** string, CIRCUITC_tokeniser_t* tokeniser, size_t* nameval_token_length){
// checks whether character is whitespace (all whitespaces are 1-char long)
    CIRCUITC_tree_error_code_t error_code;
    CIRCUITC_token_t cur_token = CIRCUITC_tree_search(tokeniser->whitespaces, *string, 1, &error_code);
    if(error_code == CIRCUITC_tree_no_error){
        CIRCUITC_tokeniser_update_string_positition(string, 1);
        return cur_token;                                       // whitespace -> return CIRCUITC_TOKEN_WHITESPACE, which is always ignored
    } 
// checks whether character is comment (all comments are 2-char long)
    cur_token = CIRCUITC_tree_search(tokeniser->comments, *string, 2, &error_code);
    if(error_code == CIRCUITC_tree_no_error){
        *string = CIRCUITC_tokeniser_comment_skip(*string, cur_token);
        return CIRCUITC_TOKEN_WHITESPACE;
    }

    return CIRCUITC_tokeniser_parse_statement(string, tokeniser, nameval_token_length);
}

#endif
