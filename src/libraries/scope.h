#ifndef VERB_library_scope_included
#define VERB_library_scope_included

#include "stdlib.h"             // dynamic memory operations.
#include "rhts/rht.h"           // generic recursive hash tables.

// VERB scope library.
// scopes are implemented as a linked list of rhts, each scope pointing to one above it.
// "global scope" may be considered the highest one in the chain, according to one's needs.
// searching for an element in a given scope starts at the scope provided & it goes up the linked list, checking all subsequent levels, until it cannot anymore.
// say one has a function like:
//
// fun foo(bar, baz, qux){
//      ...
//      ...
// }
//
// a programming language could think of foo as belonging to some scope (for this example, global scope), and its arguments (bar, baz, qux) to some other scope below the one foo is in.
// thus it would define global scope as the highest element in the linked list & define foo's scope as below it (thus pointing to global scope).
// a search within foo for some symbol defined within global scope would first search said symbol in foo's scope, fail & search it in global scope.
// this library does not support scopes that have two different scopes above them, neither does VERB..

// MUST HAVE THESE VALUES! DO NOT ALTER THESE!
// should be ALWAYS compatible with rht_destroy
typedef enum{ 
    VERB_scope_destroy_none = VERB_rht_destroy_none, 
    VERB_scope_destroy_key  = VERB_rht_destroy_key, 
    VERB_scope_destroy_obj  = VERB_rht_destroy_obj 
} VERB_scope_destroy_t;

typedef struct VERB_scope_struct{
    struct VERB_scope_struct* above;
    VERB_rht_t* symbols;
} VERB_scope_t;

void* VERB_scope_init(VERB_scope_t* restrict scope, VERB_scope_t* const restrict above){
    if(!scope) scope = malloc(sizeof(*scope));

    scope->above = above;
    scope->symbols = VERB_rht_init(NULL);

    return scope;
}

// destroys scope, returns scope above this one.
void* VERB_scope_destroy(VERB_scope_t* scope){
    void* above = scope->above;
// never NULL.
    VERB_rht_destroy(scope->symbols, VERB_rht_free);
    free(scope);

    return above;
}

// builds & returns new scope below given scope
void* VERB_scope_new(VERB_scope_t* const scope){
    return VERB_scope_init(NULL, scope);
}

// puts new symbol
void VERB_scope_put(VERB_scope_t* scope, void* const restrict key, void* const restrict obj, void (*destructor)(void*), const size_t keylen, const VERB_scope_destroy_t destroy_list){
// yes, enum conversion is used here; it is intended, as VERB_scope_destroy_t and VERB_rht_destroy_t are compatible
    VERB_rht_put(scope->symbols, key, obj, destructor, keylen, (VERB_rht_destroy_t)destroy_list);
}

void* VERB_scope_search(VERB_scope_t* scope, void* const restrict key, const size_t keylen){
    void* obj;
// checks successive scopes until it cannot anymore.
    while((obj = VERB_rht_search(scope->symbols, key, keylen)) == NULL && scope->above)
        scope = scope->above;

    return obj;
}

// tries to delete element from scope & all successive scopes if it doesn't find the symbol in lower levels.
int VERB_scope_del(VERB_scope_t* scope, void* const restrict key, const size_t keylen){
    int success;
// checks successive scopes until it cannot anymore.
    while((success = VERB_rht_del(scope->symbols, key, keylen)) == 0 && scope->above)
        scope = scope->above;

    return success;
}

#endif
