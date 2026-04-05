#ifndef VERB_tokeniser_variables_functions_included
#define VERB_tokeniser_variables_functions_included

#include "types.h"
#include "stddef.h"
#include "stdbool.h"
#include "variable_definition.h"
#include "../../../libraries/trees/vartok_tree.h"

typedef struct{
// the VERB_variable_type_t structures src, dst point to are not freed when freeing VERB_variable_function_t as they are the types of the tokens within the function.
// they are freed when THOSE tokens are freed.
    VERB_variable_t** src;                  // array of pointers to source arguments.
    VERB_variable_t** dst;                  // array of pointers to dest arguments.
    VERB_tree_vartok_t* overloads;          // tree of all valid overloads indexed by n* of args; returns token of non-overloaded function.
    char* code;                             // code.
    size_t codelen;                         // length of code.
    size_t srccnt;                          // arg_cnt holds minimum number of arguments variadic function can take, if the function is variadic.
    size_t dstcnt;                          // dst_cnt holds number of return arguments.
    bool isVariadic;                        // whether function is variadic or not.
} VERB_variable_function_t;

// functions in VERB are only overloadable by the number of arguments they take, not the types of the arguments themselves.
// that must be done with generic types.
// this is so a function has to implement the same generic behaviour for all types it takes.
// if a function named 'add' computed the square root of some numbers when given u8 arguments, but added u32 arguments, it surely wouldn't make much sense.
VERB_variable_t* VERB_variable_function_resolve_overloads(VERB_tokeniser_backend_t* const restrict backend, VERB_variable_t* const restrict defOverloaded, const size_t argcnt){
    VERB_variable_function_t* const restrict fun = defOverloaded->custom_data;
    if(!fun->overloads || fun->isVariadic) return defOverloaded;
    const VERB_variable_token_t funReal = VERB_tree_vartok_search(fun->overloads, (void*)&argcnt, sizeof(argcnt));
    return VERB_variable_search_byToken(backend, funReal);
}

#define VERB_variable_function_isVariadic(def) (((VERB_variable_function_t*)(def)->custom_data)->isVariadic)

size_t VERB_variable_function_get_srccnt(VERB_variable_t* def){
    if(def->group != VERB_VARIABLE_FUN) return 0;
    VERB_variable_function_t* const restrict fun = def->custom_data;
    return fun->srccnt;
}

size_t VERB_variable_function_get_dstcnt(VERB_variable_t* def){
    if(def->group != VERB_VARIABLE_FUN) return 1;

    VERB_variable_function_t* fun = (VERB_variable_function_t*)def->custom_data;

    return fun->dstcnt;
}

#endif
