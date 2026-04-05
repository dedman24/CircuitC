#ifndef VERB_tokeniser_included
#define VERB_tokeniser_included
 
#include "stdlib.h"                                         // dynamic memory ops
#include "stdint.h"                                         // variable-width types
#include "string.h"                                         // strstr
#include "stdbool.h"                                        // boolean type
#include "../ops/_primitives.h"                             // VERB_tokeniser_not_whitespace.
#include "../../tokens/tokens.h"                            // token type, tokens themselves
#include "backend/variable_ops.h"                           // just so it's included down the line.
#include "backend/compatibility.h"                          // just so it's included down the line.
#include "../../libraries/scope.h"                          // scoping stuff
#include "../../libraries/regex_stuff.h"                    // regex stuff
#include "../preprocessor/preprocessor.h"                   // preprocessor functions
#include "../../libraries/arrays/dynamic_arrays.h"          // dynamic array type & ops
#include "../../libraries/trees/bytecode_tree.h"            // binary search tree type & ops

#include "backend/backend.h"
#include "backend/tokeniser_ctx.h"
#include "backend/variable_ops.h"

typedef enum{ VERB_tokeniser_keep_ctx, VERB_tokeniser_free_ctx } VERB_tokeniser_options_t;

VERB_tokeniser_t* VERB_tokeniser_init(VERB_tokeniser_t* tokeniser){
    if(!tokeniser) tokeniser = malloc(sizeof(*tokeniser));

    tokeniser->keywords = VERB_token_keywords_init();
    tokeniser->comments = VERB_token_comments_init();
    tokeniser->whitespaces = VERB_token_whitespaces_init();
    tokeniser->code = VERB_array_dyn_init(NULL);
    tokeniser->preprocessor = VERB_preprocessor_init(NULL);
    tokeniser->backend = VERB_tokeniser_backend_init();
    tokeniser->ops = VERB_array_init();
    tokeniser->values = VERB_array_init();

    tokeniser->line   = 0;
    tokeniser->offset = 0;
    tokeniser->bracket_depth = 0;
// can be left uninitialised
//    ctx->nameval_length = 0;
    tokeniser->old_whitespace_cnt = 0;
    tokeniser->new_whitespace_cnt = 0;
    tokeniser->state = VERB_tokeniser_state_none;

    return tokeniser;
}

void VERB_tokeniser_destroy(VERB_tokeniser_t* const tokeniser, const VERB_tokeniser_options_t freectx){
    VERB_tree_bytecode_destroy(tokeniser->keywords, VERB_tree_free_key);
    VERB_tree_bytecode_destroy(tokeniser->comments, VERB_tree_free_key);
    VERB_tree_bytecode_destroy(tokeniser->whitespaces, VERB_tree_free_key);
    VERB_array_destroy(tokeniser->code, VERB_array_free_ctx);
    VERB_preprocessor_destroy(tokeniser->preprocessor);
    VERB_tokeniser_backend_destroy(&tokeniser->backend);

    if(freectx == VERB_tokeniser_free_ctx) free(tokeniser);
}

// parses operator, turns it into token if one exists, returns the value of token if one doesn't.
VERB_token_fun_t VERB_tokeniser_parse_operator(char* restrict* const restrict string, VERB_tokeniser_t* const restrict tokeniser, size_t length){
    VERB_token_fun_t fun = NULL;
// length is set to the length of the longest operator possible that could fit here theoretically. 
// it progressively checks if smaller & smaller operators exist.
// this is O(n), which is unoptimal, but given the lexer's such a small part of the overall compiler runtime it doesn't matter that much.
// how else would you check for operators of different lengths, where one may be a substring of another?
    do{
        fun = VERB_tree_bytecode_search(tokeniser->keywords, *string, length--);
    } while(fun);
    *string += length;
    return fun;
}

// parses statement
VERB_token_fun_t VERB_tokeniser_parse_statement(char* restrict* const restrict string, VERB_tokeniser_t* const restrict tokeniser){
    VERB_token_fun_t fun;
// checks if it is an alphanumeric string
    {
        const size_t length = VERB_REGEX_statement_length(*string);
        if(length){
            tokeniser->offset += length;
            fun = VERB_tree_bytecode_search(tokeniser->keywords, *string, length);
            
            *string += length;
            if(fun) return fun;
            tokeniser->nameval_length = length;
            return VERB_bytecode_op_name;
        } 
    }
    {
// checks if it is an operator (+ - , et cetera)
        const size_t length = VERB_REGEX_operator_length(*string);
        tokeniser->offset += length;
        fun = VERB_tokeniser_parse_operator(string, tokeniser, length);
        if(fun) return fun;
    }
    {
// it is therefore a number, and we check for its length
        const size_t length = VERB_REGEX_numeric_length(*string);
        tokeniser->offset += length;
        tokeniser->nameval_length = length;
        if(length) return VERB_bytecode_op_value;
    }
// best to check all edge cases
    return NULL;
}

// from string, sees if it can be converted to token, does so if possible, returns new string (advanced).
// undefined value if token is not VERB_TOKEN_NAME or VERB_TOKEN_VALUE.
void VERB_token_get(char* restrict* const restrict string, VERB_tokeniser_t* const restrict tokeniser){
// do these HAVE to be called in this order? not really.
    VERB_token_fun_t fun;
// checks if it's a preprocessor directive or not.
    if(**string == '#'){ 
        *string += 1;
        VERB_preprocessor_handle(string, VERB_REGEX_preprocessor_length(*string), tokeniser); 
        return;
    }
    if(**string == '\n'){
        *string += 1;
        VERB_bytecode_op_newline(string, tokeniser);
        return;
    }
// checks whether character is whitespace (all whitespaces are 1-char long ALWAYS)
    fun = VERB_tree_bytecode_search(tokeniser->whitespaces, *string, 1);
    if(fun){
        *string += 1; tokeniser->offset++;                              // properly handling offset is such a pain ;-;
        fun(string, tokeniser);
        return;
    } 
// checks whether character is comment (all comments are 2-char long)
    fun = VERB_tree_bytecode_search(tokeniser->comments, *string, 2);
    if(fun){
        *string += 1; tokeniser->offset++;
        fun(string, tokeniser);
        return;
    } 
// checks whether character is something else
    fun = VERB_tokeniser_parse_statement(string, tokeniser);
    if(fun){
        VERB_tokeniser_not_whitespace(tokeniser);
        fun(string, tokeniser); 
        return;
    }
// no token corresponds to what is written (file is formatted erroneously or corrupted); an error is returned.
    VERB_error_report(tokeniser->specifics, VERB_error_invalid_statement, tokeniser->line, tokeniser->offset, 1, "STRING IS UNPARSEABLE");
}

#endif
