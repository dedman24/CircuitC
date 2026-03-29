#ifndef VERB_token_type_type_included
#define VERB_token_type_type_included

#include "stddef.h"
#include "stdint.h"
#include "stdlib.h"
#include "stdbool.h"

typedef uint8_t VERB_type_tok_t;

typedef struct{
    VERB_type_tok_t* type;                              // tokenised type of variable.
    char* type_str;                                     // string that holds the type of the variable.
    size_t len;                                         // length of tokenised type of variable.
    size_t len_fully_qualified;                         // length of fully qualified tokenised type of variable; basically length of type minus the last qualifier.
    size_t len_str;                                     // length of string of type of variable.
    uint64_t ctr;                                       // counter to track ownership.
} VERB_type_t;

// done this way to abstract interface & discourage accesses to ctr field directly.
// VERB_type_own HAS to be 'called' each time one wishes to keep a reference to a VERB_type_t-typed variable.
// VERB_type_disown HAS to be 'called' to clean up said reference when one wishes to free it. it is done automatically with VERB_type_destroy,

#define VERB_type_own(t)        do{ ((t)->ctr++); } while(0)
#define VERB_type_disown(t)     do{ ((t)->ctr--); } while(0)

// fat ptr tracking ownership B))
void VERB_type_destroy(VERB_type_t* const restrict type){
    VERB_type_disown(type);
    if(!type->ctr){
        free(type->type);
        free(type);
    }
}

VERB_type_t* VERB_type_init(VERB_type_tok_t* const restrict tok, const size_t len, char* const restrict type_str, const size_t len_str){
    VERB_type_t* const restrict type = malloc(sizeof(*type));

    type->type = tok;
    type->len = len;

    type->type_str = type_str;
    type->len_str = len_str;
// proper initialisation of ctr; when ctr hits 0, the VERB_type_t is freed.
    type->ctr = 1;

    return type;
}

// VERB types
enum{
// types
// function types
    VERB_TYPE_fn,
    VERB_TYPE_op_t,
    VERB_TYPE_qualifier,
    VERB_TYPE_conversion,
    VERB_TYPE_implicit,
    VERB_TYPE_explicit,
// integer types
    // signed
        VERB_TYPE_i8,
        VERB_TYPE_i16,
        VERB_TYPE_i32,
        VERB_TYPE_i64,
        VERB_TYPE_intptr,
    // unsigned
        VERB_TYPE_u8,
        VERB_TYPE_u16,
        VERB_TYPE_u32,
        VERB_TYPE_u64,
        VERB_TYPE_size_t,
        VERB_TYPE_uintptr_t,
    // variable-width
        VERB_TYPE_vwint_t,
        VERB_TYPE_ctint_t,
// floating-point types
    VERB_TYPE_f8,
    VERB_TYPE_f16,
    VERB_TYPE_f32,
    VERB_TYPE_f64,
// boolean types
    VERB_TYPE_b,
    VERB_TYPE_bgen,
    // VERB_TYPE_bool/boolgen are aliases of b/bgen respectively
// pointer types & qualifiers
    VERB_TYPE_str,                      // string type
    VERB_TYPE_array,                    // :
    VERB_TYPE_weak_nullable_array,      // ?:
    VERB_TYPE_strong_nullable_array,    // !:
    VERB_TYPE_weak_nullable_p,          // ?p
    VERB_TYPE_strong_nullable_p,        // !p
    // DO NOT CHANGE THE ORDER THE TOKENS VERB_TYPE_p through VERB_TYPE_boolean ARE IN!
    // these are used to quickly check for type compatibility.
    // they are used to mark the generic 'type group' a type belongs to, to then figure out if they can be used the same op.
    VERB_TYPE_p,                        // p
// type-defining types
    VERB_TYPE_type,                     // type
// generic types
    VERB_TYPE_int,                      // all integer types.
    VERB_TYPE_float,                    // all floating point types.
    VERB_TYPE_boolean,                  // all boolean types.
    VERB_TYPE_sint,                     // all signed integer types, alias of int signed.
    VERB_TYPE_uint,                     // all unsigned integer types, alias of int unsigned.
    VERB_TYPE_raw_t,
    VERB_TYPE_gen_t,
    VERB_TYPE_union,
    VERB_TYPE_many_t,
    VERB_TYPE_sym_t,
// enumeration types
    VERB_TYPE_enum,
// type qualifiers
    // integer types
        VERB_TYPE_exact,
        VERB_TYPE_signed,
        VERB_TYPE_unsigned,
// DO NOT REMOVE NOR ALTER!
    VERB_TYPE_special_NAME              // VERB type name.
};

#endif
