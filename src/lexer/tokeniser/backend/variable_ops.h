#ifndef VERB_tokeniser_variable_included
#define VERB_tokeniser_variable_included

#include "stdint.h"
#include "stddef.h"
#include "backend.h"
#include "../variables/variable_definition.h"
#include "../../../libraries/errors_warnings/error_handling.h"

// ops on variables

VERB_variable_t* VERB_variable_search_byName(VERB_tokeniser_backend_t* const restrict backend, void* const restrict key, const size_t keylen){
    return VERB_scope_search(backend->variables, key, keylen);
}

VERB_variable_t* VERB_variable_search_byToken_rht(VERB_rht_t* const restrict all_token_defs, VERB_variable_token_t tok){
    return VERB_rht_search(all_token_defs, &tok, sizeof(tok));
}

VERB_variable_t* VERB_variable_search_byToken(VERB_tokeniser_backend_t* const restrict backend, VERB_variable_token_t tok){
    return VERB_rht_search(backend->all_token_defs, &tok, sizeof(tok));
}

void VERB_variable_definition_destroy(void* def_formal){
    VERB_variable_t* def = def_formal;
// TODO: implement proper destructor behaviour.
    switch(def->type->group){
        case VERB_TYPEGROUP_TYPE:
            break;
        case VERB_TYPEGROUP_ARITH:
            break;
        case VERB_TYPEGROUP_ARR:
            break;
        case VERB_TYPEGROUP_FUN:
            break;
        case VERB_TYPEGROUP_COMPLEX:
            break;
    }
    VERB_type_destroy(def->type);
    free(def);
}

VERB_variable_t* VERB_variable_definition_init(VERB_tokeniser_backend_t* const restrict backend, char* const restrict name, const size_t namelen, VERB_type_t* const restrict type, const VERB_variable__flags_t flags){
    VERB_variable_t* def = malloc(sizeof(*def));

    def->name = VERB_variable_alloc(backend);
    VERB_type_own(type);
    def->type = type;
    def->flags = flags;

    VERB_scope_put(backend->variables, name, def, NULL, namelen, VERB_scope_destroy_none);
// can NEVER fail.
    VERB_rht_put(backend->all_token_defs, &def->name, def, VERB_variable_definition_destroy, sizeof(def->name), VERB_rht_destroy_obj);
    if(backend->recently_defined_tokens) 
        VERB_array_push(def, backend->recently_defined_tokens);

    return def;
}

// initialises internal variable.
// an internal variable is the result of any operation; it cannot be accessed & they're used by the bytecode to represent intermediate results.
// they're dataflow tokens. not 'basically' or 'almost', they ARE dataflow tokens & they must not be added to the tokeniser's scope.
VERB_variable_t* VERB_variable_definition_internal_init(VERB_tokeniser_backend_t* const restrict backend, void* const restrict type, const VERB_variable__flags_t flags){
    VERB_variable_t* def = malloc(sizeof(*def));

    def->name = VERB_variable_alloc(backend);
    VERB_type_own((VERB_type_t*)type);
    def->type = type;
    def->flags = flags;
// can NEVER fail.
    VERB_rht_put(backend->all_token_defs, &def->name, def, VERB_variable_definition_destroy, sizeof(def->name), VERB_rht_destroy_obj);
    if(backend->recently_defined_tokens) 
        VERB_array_push(def, backend->recently_defined_tokens);

    return def;
}

#endif
