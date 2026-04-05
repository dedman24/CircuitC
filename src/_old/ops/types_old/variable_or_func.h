#ifndef VERB_compiler_ops_types_variable_or_func_included
#define VERB_compiler_ops_types_variable_or_func_included

#include "../includes.h"

enum{
    VERB_COMPILER_IS_VARIABLE,
    VERB_COMPILER_IS_FUNCTION,

// KEEP THIS LAST!
    VERB_COMPILER_NOT_A_SUPPORTED_STATEMENT
};

// determining symbols to distinguish statements
const VERB_token_t VERB_compiler_determining_token[] = {
    [VERB_COMPILER_IS_VARIABLE] = VERB_TOKEN_SINGLE_EQUALS_SIGN,
    [VERB_COMPILER_IS_FUNCTION] = VERB_TOKEN_OPEN_BRACKET
};
const size_t VERB_compiler_determining_token_size = sizeof(VERB_compiler_determining_token)/sizeof(*VERB_compiler_determining_token);

// all statments distinguished by this function must have their tokens arranged as:
// <name> token
// determining token
// we compare said determining token with the ones listed above in VERB_compiler_determining_token & return the appropriate statments, or VERB_COMPILER_NOT_A_SUPPORTED_STATEMENT.
// take for example:
// int foo(){
//      ...
// }
// &
// int bar = ... ;
// clearly these are two different statements, and the function below distinguishes the two.

// returns what statement the provided token array follows; whether it is a function or a variable or any other statements listed above.
int VERB_compiler_determine_statement(char* tokens){
// skips everything until first name token (everything in the middle assumed to be a type qualifier, even if it includes the whole program with it).
// very clearly if the whole program is between a variable type & its declaration, this isn't valid code!
// this error is handled at the interpreter stage, which is also when types are considered.
    VERB_token_t tok;
    while((tok = VERB_token_read(&tokens)) != VERB_TOKEN_NAME){
        if(tok == VERB_TOKEN_EOF) return VERB_COMPILER_NOT_A_SUPPORTED_STATEMENT; 
    }
    VERB_token_nameval_skip(&tokens);
    VERB_token_t current_token = VERB_token_read(&tokens);

    for(size_t i = 0; i < VERB_compiler_determining_token_size; i++){
        if(VERB_compiler_determining_token[i] == current_token) return i;
    }

    return VERB_COMPILER_NOT_A_SUPPORTED_STATEMENT;
}


#endif
