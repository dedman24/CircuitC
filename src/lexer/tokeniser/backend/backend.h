#ifndef VERB_tokeniser_backend_included
#define VERB_tokeniser_backend_included

#include "stddef.h"                                         // size_t
#include "../../../libraries/scope.h"                       // scoping stuff
#include "../../../tokens/token_type.h"                     // tokens
#include "../../../libraries/arrays/dynamic_arrays.h"       // VERB_array_t type.

// WARNING: this value must be large enough to represent ALL bytecodes defined in "lexer/tokens/token_type.h" along with all of the values it CAN represent.
// it is recommended to not make this smaller than a uint32_t. DO NOT TOUCH IT!
// see "lexer/lexer.h" VERB_lexer_line for why (not a textual explanation, but the code that relies on this).
typedef uint64_t VERB_variable_token_t;

#define VERB_variable_name_mask_function ((uint64_t)1<<63)
#define VERB_variable_name_isFunction(name) ((name)&VERB_variable_name_mask_function)

typedef struct{
    VERB_variable_token_t var_start;
} VERB_variable_name_alloc_ctx;

// !!! recently_defined_tokens HAS to be destroyed before all_token_defs !!!
typedef struct{
    VERB_variable_name_alloc_ctx alloc_ctx;
    VERB_scope_t* variables;                                // current variable scope.
    VERB_scope_t* alias_objects;                            // current alias object scope; alias objects & other vars must inhabit different scope spaces.
    VERB_rht_t* all_token_defs;                             // all token definitions.
    VERB_array_t* recently_defined_tokens;                  // array pointers to variables are added to when not null, to support namespaces & complex types.
    unsigned long long scope_depth;                         // depth of scopes.
} VERB_tokeniser_backend_t;

//
// VERB_variable_name_alloc_ctx stuff
//

#define VERB_variable_name_alloc_ctx_start 1

VERB_variable_name_alloc_ctx VERB_variable_name_alloc_ctx_init(){
    return (VERB_variable_name_alloc_ctx){VERB_variable_name_alloc_ctx_start};
}

void VERB_variable_name_alloc_ctx_destroy(VERB_variable_name_alloc_ctx* ctx){
    return;             // NOOP, exists for API needs (abstracts interface).
}

#define VERB_variable_alloc(backend) ((backend)->alloc_ctx.var_start++)

// 
// VERB_tokeniser_backend_t stuff
//

VERB_tokeniser_backend_t VERB_tokeniser_backend_init(){
    return (VERB_tokeniser_backend_t){
        VERB_variable_name_alloc_ctx_init(),            // alloc_ctx.
        VERB_scope_init(NULL, NULL),                    // variables.
        VERB_scope_init(NULL, NULL),                    // alias_objects.
        VERB_rht_init(NULL),                            // all_token_defs.
        NULL                                            // recently_defined_tokens.
    };
}

void VERB_tokeniser_backend_destroy(VERB_tokeniser_backend_t* const backend){
    VERB_variable_name_alloc_ctx_destroy(&backend->alloc_ctx);
    VERB_scope_destroy(backend->variables);
    VERB_scope_destroy(backend->alias_objects);
}

#define VERB_tokeniser_backend_scope_new(backend)                                                   \
    do{                                                                                             \
        (backend)->variables = VERB_scope_init(NULL, (backend)->variables);                         \
        (backend)->alias_objects = VERB_scope_init(NULL, (backend)->alias_objects);                 \
        (backend)->scope_depth++;                                                                   \
    } while(0)

#define VERB_tokeniser_backend_scope_del(backend)                                                   \
    do{                                                                                             \
        (backend)->variables = VERB_scope_destroy((backend)->variables);                            \
        (backend)->alias_objects = VERB_scope_destroy((backend)->alias_objects);                    \
        (backend)->scope_depth--;                                                                   \
    } while(0)

void VERB_tokeniser_backend_set_recently_added(VERB_tokeniser_backend_t* const restrict backend, void* const restrict old){
    backend->recently_defined_tokens = old;
}

VERB_array_t* VERB_tokeniser_backend_add_recently_added(VERB_tokeniser_backend_t* const backend){
    void* old = backend->recently_defined_tokens;
    backend->recently_defined_tokens = VERB_array_dyn_init(NULL);
    return old;
}

void* VERB_tokeniser_backend_extract_recently_added(VERB_tokeniser_backend_t* const backend){
    VERB_array_t* const restrict r = backend->recently_defined_tokens;
    backend->recently_defined_tokens = NULL;
    return r;
}

#endif
