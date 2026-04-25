#ifndef VERB_token_type_type_included
#define VERB_token_type_type_included

#include "stddef.h"
#include "stdint.h"
#include "stdlib.h"
#include "string.h"
#include "stdbool.h"
#include "../../libraries/arrays/dynamic_arrays.h"

// data about type.
typedef enum{
    VERB_TYPE_FLAG_none                  = 0,
    VERB_TYPE_FLAG_mut                   = 1,
    VERB_TYPE_FLAG_mut_priv              = 2,
    VERB_TYPE_FLAG_mut_pub               = 3,
    VERB_TYPE_FLAG_inaccessible          = 4,
    VERB_TYPE_FLAG_priv                  = 8,
    VERB_TYPE_FLAG_volatile              = 16,
    VERB_TYPE_FLAG_static                = 32,
    VERB_TYPE_FLAG_strict                = 64,
} VERB_type__flag_t;

// VERB types
typedef enum: uint8_t{
// types
// function types
    VERB_TYPE_fn,
    VERB_TYPE_qualifier,
// integer types
// signed
    VERB_TYPE_s8,
    VERB_TYPE_s16,
    VERB_TYPE_s32,
    VERB_TYPE_s64,
    VERB_TYPE_ssize_t,
// unsigned
    VERB_TYPE_u8,
    VERB_TYPE_u16,
    VERB_TYPE_u32,
    VERB_TYPE_u64,
    VERB_TYPE_size_t,
// variable-width
    VERB_TYPE_vwint_t,
    VERB_TYPE_ctint_t,
// floating-point types
    VERB_TYPE_f8,
    VERB_TYPE_f16,
    VERB_TYPE_f32,
    VERB_TYPE_f64,
// boolean types
    VERB_TYPE_bool,
// array types & qualifiers
    VERB_TYPE_str,                      // string type.
// type-defining types
    VERB_TYPE_type,                     // type.
// generic types
    VERB_TYPE_int,                      // all integer types.
    VERB_TYPE_float,                    // all floating point types.
    VERB_TYPE_sint,                     // all signed integer types, alias of int signed.
    VERB_TYPE_uint,                     // all unsigned integer types, alias of int unsigned.
    VERB_TYPE_i8,
    VERB_TYPE_i16,
    VERB_TYPE_i32,
    VERB_TYPE_i64,
    VERB_TYPE_raw_t,
    VERB_TYPE_gen_t,
    VERB_TYPE_union,
    VERB_TYPE_many_t,
// enumeration types
    VERB_TYPE_enum,
// type qualifiers
    VERB_TYPE_special_QUALIFIER_START,  // DO NOT CHANGE.
    VERB_TYPE_exact,
    VERB_TYPE_array,                    // []
// DO NOT REMOVE NOR ALTER!
    VERB_TYPE_special_RETURNARGS,
    VERB_TYPE_special_SRCARGS,
    VERB_TYPE_special_DSTARGS,
    VERB_TYPE_special_END
} VERB_type_tok_t;

#define VERB_type__get_bit(x) (1<<x)

// works surprisingly well.
// holds bitmap of which types are part of the signature & which aren't.
const uint64_t VERB_type_signature_mask =
      (uint64_t)1 << VERB_TYPE_fn
    | (uint64_t)1 << VERB_TYPE_qualifier
    | (uint64_t)1 << VERB_TYPE_s8
    | (uint64_t)1 << VERB_TYPE_s16
    | (uint64_t)1 << VERB_TYPE_s32
    | (uint64_t)1 << VERB_TYPE_s64
    | (uint64_t)1 << VERB_TYPE_ssize_t
    | (uint64_t)1 << VERB_TYPE_u8
    | (uint64_t)1 << VERB_TYPE_u16
    | (uint64_t)1 << VERB_TYPE_u32
    | (uint64_t)1 << VERB_TYPE_u64
    | (uint64_t)1 << VERB_TYPE_size_t
    | (uint64_t)1 << VERB_TYPE_vwint_t
    | (uint64_t)1 << VERB_TYPE_ctint_t
    | (uint64_t)1 << VERB_TYPE_f8
    | (uint64_t)1 << VERB_TYPE_f16
    | (uint64_t)1 << VERB_TYPE_f32
    | (uint64_t)1 << VERB_TYPE_f64
    | (uint64_t)1 << VERB_TYPE_bool
    | (uint64_t)1 << VERB_TYPE_str
    | (uint64_t)1 << VERB_TYPE_type
    | (uint64_t)1 << VERB_TYPE_int
    | (uint64_t)1 << VERB_TYPE_float
    | (uint64_t)1 << VERB_TYPE_sint
    | (uint64_t)1 << VERB_TYPE_uint
    | (uint64_t)1 << VERB_TYPE_i8
    | (uint64_t)1 << VERB_TYPE_i16
    | (uint64_t)1 << VERB_TYPE_i32
    | (uint64_t)1 << VERB_TYPE_i64
    | (uint64_t)1 << VERB_TYPE_raw_t
    | (uint64_t)1 << VERB_TYPE_gen_t
    | (uint64_t)1 << VERB_TYPE_union
    | (uint64_t)1 << VERB_TYPE_many_t
    | (uint64_t)1 << VERB_TYPE_enum
    | (uint64_t)1 << VERB_TYPE_array
;

const char* VERB_type_tok_strings[] = {
    [VERB_TYPE_fn]                          = "fn",
    [VERB_TYPE_qualifier]                   = "qualifier",
    [VERB_TYPE_s8]                          = "s8",
    [VERB_TYPE_s16]                         = "s16",
    [VERB_TYPE_s32]                         = "s32",
    [VERB_TYPE_s64]                         = "s64",
    [VERB_TYPE_ssize_t]                     = "ssize_t",
    [VERB_TYPE_u8]                          = "u8",
    [VERB_TYPE_u16]                         = "u16",
    [VERB_TYPE_u32]                         = "u32",
    [VERB_TYPE_u64]                         = "u64",
    [VERB_TYPE_size_t]                      = "size_t",
    [VERB_TYPE_f8]                          = "f8",
    [VERB_TYPE_f16]                         = "f16",
    [VERB_TYPE_f32]                         = "f32",
    [VERB_TYPE_f64]                         = "f64",
    [VERB_TYPE_bool]                        = "bool",     
    [VERB_TYPE_str]                         = "str",
    [VERB_TYPE_type]                        = "type",
    [VERB_TYPE_int]                         = "int",
    [VERB_TYPE_float]                       = "float",
    [VERB_TYPE_sint]                        = "sint",
    [VERB_TYPE_uint]                        = "uint",
    [VERB_TYPE_i8]                          = "i8",
    [VERB_TYPE_i16]                         = "i16",
    [VERB_TYPE_i32]                         = "i32",
    [VERB_TYPE_i64]                         = "i64",
    [VERB_TYPE_raw_t]                       = "raw_t",
    [VERB_TYPE_gen_t]                       = "gen_t",
    [VERB_TYPE_union]                       = "union",
    [VERB_TYPE_many_t]                      = "many_t",
    [VERB_TYPE_enum]                        = "enum",
    [VERB_TYPE_exact]                       = "exact",
    [VERB_TYPE_array]                       = "[]",
};

typedef enum{
    VERB_TYPEGROUP_TYPE,                                // type-specifying type.
    VERB_TYPEGROUP_ARITH,                               // arithmetic type.
    VERB_TYPEGROUP_ARR,                                 // array.
    VERB_TYPEGROUP_FUN,                                 // function.
    VERB_TYPEGROUP_COMPLEX,                             // complex type.
} VERB_type__group_t;                                   // major groups of types a type can be part of. 

typedef struct{
    VERB_type_tok_t* type;                              // tokenised type.
    VERB_type_tok_t* sig;                               // 'type signature'/minimal type to check for compatibility.
    size_t typelen;                                     // length of tokenised type.
    size_t siglen;                                      // length of type that has to be checked to ensure two types are compatible.
    uint64_t ctr;                                       // counter to track ownership.
    VERB_type__flag_t flags;                            // flags regarding type.
    VERB_type__group_t group;                           // group type belongs to.
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
        free(type->sig);
        free(type);
    }
}

VERB_type_t* VERB_type_init(const VERB_type__group_t group, VERB_type_tok_t* const restrict type, const size_t typelen, VERB_type_tok_t* const restrict sig, const size_t siglen){
    VERB_type_t* const restrict final = malloc(sizeof(*final));

    final->type = type;
    final->typelen = typelen;

    final->sig = sig;
    final->siglen = siglen;
// proper initialisation of ctr; when ctr hits 0, the VERB_type_t is freed.
    final->ctr = 1;

    final->group = group;

    return final;
}

VERB_type_t* VERB_type_copy(VERB_type_tok_t* const restrict type, const size_t typelen, VERB_type_tok_t* const restrict sig, const size_t siglen){
    VERB_type_t* const restrict final = malloc(sizeof(*final));

    final->typelen = typelen;
    final->type = malloc(typelen*sizeof(*final->type));
    memcpy(final->type, type, typelen*sizeof(*final->type));

    final->siglen = siglen;
    final->sig = malloc(siglen*sizeof(*final->sig));
    memcpy(final->sig, sig, siglen*sizeof(*final->sig));
// proper initialisation of ctr; when ctr hits 0, the VERB_type_t is freed.
    final->ctr = 1;

    return final;
}

#define VERB_type_signature_build(type, typelen, sig, siglen)                               \
    VERB_array_t signature = VERB_array_init();                                             \
    for(size_t i = 0; i < typelen; i++){                                                    \
        if(VERB_type_signature_mask & (uint64_t)1 << type[i])                               \
            VERB_array_push(type[i], &signature);                                           \
    }                                                                                       \
    VERB_array_extract_all(&signature, const size_t siglen, VERB_type_tok_t* const restrict sig, false);

// returns true if the two types are equal, false otherwise.
bool VERB_type_equal(VERB_type_t* const t0, VERB_type_t* const t1){
    return t0 == t1 || (t0->siglen == t1->siglen && !memcmp(t0->type, t1->type, t0->siglen*sizeof(*t0->type))); 
}

#endif
