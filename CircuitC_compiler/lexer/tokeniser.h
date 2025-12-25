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

// counts number of occurrences of the character '\n' in the portion of string 'string' of length 'length_string'
// also returns offset since newline
size_t CIRCUITC_tokeniser_count_lines_skipped(char* string, size_t length_string, size_t* offset){
    const char target = '\n';
    size_t count = 0;

    while(length_string--){
        if(*string == target){
            count++;
            *offset = 0;
        }
        else (*offset)++;
    }

    return count;
}

// skips comment
char* CIRCUITC_tokeniser_comment_skip(char* string, const CIRCUITC_token_t comment, size_t* lines_skipped, size_t* offset){
    const char* closing_comment_symbol = CIRCUITC_comments_end[comment];

    char* newstring = strstr(string, closing_comment_symbol);
    if(newstring == NULL) return string + strlen(string);       // comment is closed by \x00; next call to CIRCUITC_token_get will return CIRCUITC_TOKEN_EOF and lexing will be halted
// we want to count the number of lines skipped in said comment
    *lines_skipped = CIRCUITC_tokeniser_count_lines_skipped(string, (size_t)(string - newstring), offset);

    return newstring + strlen(closing_comment_symbol);
}

char CIRCUITC_character_toupper(const char character){
    return character &~0x20;
}

bool CIRCUITC_tokeniser_REGEX_is_alphabetic(const char character){
    const char uppercase_character = CIRCUITC_character_toupper(character);
    return 0x41 <= uppercase_character && uppercase_character <= 0x5A;
}

bool CIRCUITC_tokeniser_REGEX_is_numeric(const char character){
    return 0x30 <= character && character <= 0x39;
}

// the regex being [a-zA-Z] or [a-zA-Z0-9] if 'position' is nonzero
bool CIRCUITC_tokeniser_REGEX_alphanumeric_check(const char character, const size_t position){
    return CIRCUITC_tokeniser_REGEX_is_alphabetic(character) || (CIRCUITC_tokeniser_REGEX_is_numeric(character) && position);
}

// returns length of largest substring whose start is 'string' that is accepted the following regex: [a-zA-Z][a-zA-Z0-9]*
// if no string is accepted by said REGEX, it returns 0.
size_t CIRCUITC_tokeniser_REGEX_alphanumeric_length(char* string){
    size_t length = 0;
    while(CIRCUITC_tokeniser_REGEX_alphanumeric_check(string[length], length)) length++;
    return length;
}

// the regex being [0-9] or [a-zA-Z0-9] if 'position' is nonzero
bool CIRCUITC_tokeniser_REGEX_numeric_check(const char character, const size_t position){
    return (CIRCUITC_tokeniser_REGEX_is_alphabetic(character) && position) || CIRCUITC_tokeniser_REGEX_is_numeric(character);
}

// returns length of largest substring whose start is 'string' that is accepted the following regex:[0-9][a-zA-Z0-9]*
// if no string is accepted by said REGEX, it returns 0.
size_t CIRCUITC_tokeniser_REGEX_numeric_length(char* string){
    size_t length = 0;
    while(CIRCUITC_tokeniser_REGEX_numeric_check(string[length], length)) length++;
    return length;
}

// parses operator, turns it into token if one exists, returns the value of token if one doesn't.
CIRCUITC_token_t CIRCUITC_tokeniser_parse_operator(char** string, CIRCUITC_tokeniser_t* tokeniser, CIRCUITC_token_t token){
    size_t operator_length = 1;
    CIRCUITC_tree_error_code_t error_code;
    CIRCUITC_token_t new_token = token;
// pretty cool algorithm, tokenises arbitrarily-long operators. not that efficient but operators are usually short anyways.
// by operators we mean any symbol that does not conform to the regex [A-Za-z][A-Za-z0-9]* or [0-9][A-Za-z0-9]*. this is stuff like ++, --, != et cetera
// say we had three operators: !, != and !==. ! has the CIRCUITC_TOKEN_METADATA_EXTENDED flag set, so does !=.
// !== doesn't, as there is no operator that is longer than it & that shares the starting sequence of symbols.
// our algorithm cannot decompose the operator !== into ! and == tokens; this doesn't bother me right now.
// it would decompose that into a != and an = token. one could define != as a special form of !== and be done with it.
    do{
        token = new_token;
        new_token = CIRCUITC_tree_search(tokeniser->keywords, *string, operator_length++, &error_code);
    } while(error_code == CIRCUITC_tree_no_error && CIRCUITC_TOKEN_METADATA_GET(new_token) == CIRCUITC_TOKEN_METADATA_EXTENDED);
// because operator_length is always 2 larger than the effective operator
    *string += operator_length - 2;
    return token;
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
    token = CIRCUITC_tokeniser_parse_operator(string, tokeniser, CIRCUITC_TOKEN_EOF);
    if(token != CIRCUITC_TOKEN_EOF) return token;               // CIRCUITC_tokeniser_parse_operator does not change the token it is passed if it finds no operator
// it is therefore a number, and we check for its length
    length = CIRCUITC_tokeniser_REGEX_numeric_length(*string);

    *nameval_token_length = length;
    return CIRCUITC_TOKEN_VALUE;
}

// from string, sees if it can be converted to token, does so if possible, returns new string (advanced).
// nameval_token_length ~ ptr to size_t variable that holds length of byte string following <name> or <value> token in bytes, limited to 256.
// undefined value if token is not CIRCUITC_TOKEN_NAME or CIRCUITC_TOKEN_VALUE.
CIRCUITC_token_t CIRCUITC_token_get(char** string, CIRCUITC_tokeniser_t* tokeniser, size_t* lines_skipped, size_t* offset, size_t* nameval_token_length){
// checks whether character is whitespace (all whitespaces are 1-char long)
    CIRCUITC_tree_error_code_t error_code;
    CIRCUITC_token_t cur_token = CIRCUITC_tree_search(tokeniser->whitespaces, *string, 1, &error_code);
    if(error_code == CIRCUITC_tree_no_error){
        if(cur_token == CIRCUITC_TOKEN_NEWLINE){ *lines_skipped = 1; *offset = 0; }

        *offset += 1;
        *string += 1;                                           // all whitespaces have length 1
        return cur_token;                                       // whitespace -> return CIRCUITC_TOKEN_WHITESPACE, which is always ignored
    } 
// checks whether character is comment (all comments are 2-char long)
    cur_token = CIRCUITC_tree_search(tokeniser->comments, *string, 2, &error_code);
    if(error_code == CIRCUITC_tree_no_error){
        *string = CIRCUITC_tokeniser_comment_skip(*string, cur_token, lines_skipped, offset);
        return CIRCUITC_TOKEN_WHITESPACE;
    }
// checks for character in keyword bst
    cur_token = CIRCUITC_tokeniser_parse_statement(string, tokeniser, nameval_token_length);
    *lines_skipped = 0;                                         // no lines are skipped as '\n' is not valid regex
    *offset += *nameval_token_length;                           // offset is incremented by nameval_token_length for similar reasons
    return cur_token;
}

#endif
