#ifndef VERB_tokeniser_variables_functions_included
#define VERB_tokeniser_variables_functions_included

#include "stddef.h"
#include "stdbool.h"
#include "variable_definition.h"
#include "../backend/variable_ops.h"
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

VERB_variable_function_t* VERB_variable_function_init(VERB_variable_t** const restrict src, VERB_variable_t** const restrict dst, VERB_tree_vartok_t* const restrict overloads, char* const restrict code, const size_t codelen, const size_t srccnt, const size_t dstcnt, const bool isVariadic){
    VERB_variable_function_t* const restrict fun = malloc(sizeof(*fun));

    fun->src = src;
    fun->dst = dst;
    fun->overloads = overloads;
    fun->code = code;
    fun->codelen = codelen;
    fun->srccnt = srccnt;
    fun->dstcnt = dstcnt;
    fun->isVariadic = isVariadic;

    return fun;
}

// format of a function's type is:
// fn(src args)(dst args)
VERB_type_t* VERB_variable_function_type_init(VERB_variable_t** const src, const size_t srccnt, VERB_variable_t** const dst, const size_t dstcnt){
    VERB_array_t typearr = VERB_array_init();
    VERB_array_t sigarr = VERB_array_init();            // tienes sigarr amego?
    
    VERB_array_push_type(VERB_type_tok_t, VERB_TYPE_fn, &typearr); 
    VERB_array_push_type(VERB_type_tok_t, VERB_TYPE_fn, &sigarr);

    VERB_array_push_type(VERB_type_tok_t, VERB_TYPE_special_SRCARGS, &typearr); 
    VERB_array_push_type(VERB_type_tok_t, VERB_TYPE_special_SRCARGS, &sigarr);
    
    for(size_t i = 0; i < srccnt; i++){
        VERB_array_push_string(src[i]->type->type, src[i]->type->typelen*sizeof(*src[i]->type->type), &typearr);
        VERB_array_push_string(src[i]->type->sig,  src[i]->type->siglen *sizeof(*src[i]->type->sig),  &sigarr);
    }

    VERB_array_push_type(VERB_type_tok_t, VERB_TYPE_special_DSTARGS, &typearr);
    VERB_array_push_type(VERB_type_tok_t, VERB_TYPE_special_DSTARGS, &sigarr);

    for(size_t i = 0; i < dstcnt; i++){
        VERB_array_push_string(dst[i]->type->type, dst[i]->type->typelen*sizeof(*dst[i]->type->type), &typearr);
        VERB_array_push_string(dst[i]->type->sig,  dst[i]->type->siglen *sizeof(*dst[i]->type->sig),  &sigarr);
    }

    VERB_array_extract_all(&typearr, const size_t typelen, VERB_type_tok_t* const restrict type, false);
    VERB_array_extract_all(&sigarr,  const size_t siglen,  VERB_type_tok_t* const restrict sig, false);

    return VERB_type_init(VERB_TYPEGROUP_FUN, type, typelen, sig, siglen);
}

size_t VERB_variable_function_get_srccnt(VERB_variable_t* def){
    if(def->type->group != VERB_TYPEGROUP_FUN) return 0;
    VERB_variable_function_t* const restrict fun = def->custom_data;
    return fun->srccnt;
}

size_t VERB_variable_function_get_dstcnt(VERB_variable_t* def){
    if(def->type->group != VERB_TYPEGROUP_FUN) return 1;

    VERB_variable_function_t* fun = (VERB_variable_function_t*)def->custom_data;

    return fun->dstcnt;
}

#endif
