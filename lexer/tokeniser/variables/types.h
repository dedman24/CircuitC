#ifndef VERB_tokeniser_variables_types_included
#define VERB_tokeniser_variables_types_included

#include "stdarg.h"                                 // variadic functions.
#include "stddef.h"                                 // size_t.
#include "stdint.h"                                 // various integer types.
#include "stdbool.h"                                // bool, true, false.
#include "../backend/backend.h"                     // backend type & ops.
#include "../backend/variable_ops.h"                // VERB_variable_search_byToken.
#include "variable_definition.h"                    // VERB_variable_definition_t type.

// // // // // // // //
// type conversion.  //
// // // // // // // //

typedef enum{
// DO NOT CHANGE THE ORDER OF THESE!
    VERB_VARIABLE_TYPE_CONVERSION_RANK_LOWER,
    VERB_VARIABLE_TYPE_CONVERSION_RANK_SAME,
    VERB_VARIABLE_TYPE_CONVERSION_RANK_HIGHER
} VERB_variable_type_conversion_rank_t;

typedef enum{
    VERB_VARIABLE_TYPE_CONVERSION_FLAG_NONE            = 0,
    VERB_VARIABLE_TYPE_CONVERSION_FLAG_FULLY_QUALIFIED = 1,     // that conversion has to also check fully-qualified type name.
} VERB_variable_type_conversion_flag_t;

typedef struct{
    VERB_variable_token_t dst;                          // destination type.
    VERB_variable_token_t fun;                          // function name.
    VERB_variable_type_conversion_rank_t rank;          // rank.
    VERB_variable_type_conversion_flag_t flag;          // flags/info about conversion.
} VERB_variable_type_conversion_t;

VERB_variable_type_conversion_t VERB_variable_type_conversion_init(const VERB_variable_token_t dst, const VERB_variable_token_t fun, const VERB_variable_type_conversion_rank_t rank){
    return (VERB_variable_type_conversion_t){dst, fun, rank};
}

void VERB_variable_type_conversion_destroy(void* conversion_formal){
    free(conversion_formal);
}

// // // // // // // //
// types themselves. //
// // // // // // // //

typedef struct{
    VERB_rht_t* fields;                             // rht hodling variables/fields of complex type (TODO: consider making this not an RHT?).
    VERB_rht_t implicit_conversion;                 // names of types it can implicitly be converted to.
    VERB_rht_t explicit_conversion;                 // names of types it can explicitly be converted to.
} VERB_variable_type_t;

VERB_variable_type_t* VERB_variable_type_init(const size_t fieldcnt, ...){
    VERB_variable_type_t* const restrict type = malloc(sizeof(*type));

    VERB_rht_init(&type->explicit_conversion);
    VERB_rht_init(&type->implicit_conversion);

    type->fields = VERB_rht_init(NULL);

    va_list fields;
    va_start(fields, fieldcnt);

    for(size_t i = 0; i < fieldcnt; i++){
    // can NEVER fail.
        VERB_variable_definition_t* const restrict field = va_arg(fields, VERB_variable_definition_t*);
        VERB_rht_put(type->fields, &field->name, NULL, NULL, sizeof(field->name), VERB_rht_destroy_none);
    }

    va_end(fields);

    return type;
}

VERB_variable_type_t* VERB_variable_type_direct_init(VERB_rht_t* const restrict fields){
    VERB_variable_type_t* const restrict type = malloc(sizeof(*type));

    VERB_rht_init(&type->explicit_conversion);
    VERB_rht_init(&type->implicit_conversion);

    type->fields = fields;

    return type;
}

void VERB_variable_type_destroy(void* type_formal){
    VERB_variable_type_t* type = type_formal;

    VERB_rht_destroy(&type->implicit_conversion, VERB_rht_keep);
    VERB_rht_destroy(&type->implicit_conversion, VERB_rht_keep);

    if(type->fields) free(type->fields);
    free(type);
}

// returns VERB_variable_definition_t* of the variable whose type has the higher priority, NULL if neither.
// priority specifies that the other must be converted to ours before ours is converted to the other.
VERB_variable_definition_t* VERB_variable_type_conversion_check_flags(VERB_variable_type_conversion_t* const restrict conv0, VERB_variable_definition_t* const restrict v0, VERB_variable_type_conversion_t* const restrict conv1, VERB_variable_definition_t* const restrict v1){
// this way, if the ranks are equal, conv0's type is chosen.
    VERB_variable_definition_t* const restrict v_highest = conv1->rank > conv0->rank? v1: v0;
    VERB_variable_type_conversion_t* const restrict conv_highest = conv1->rank > conv0->rank? conv1: conv0;

    if(conv_highest->flag != VERB_VARIABLE_TYPE_CONVERSION_FLAG_NONE){
        if(conv_highest->flag & VERB_VARIABLE_TYPE_CONVERSION_FLAG_FULLY_QUALIFIED){
            VERB_type_t *const restrict type0 = v0->type, *const restrict type1 = v1->type;
            if(memcmp(type0->type, type1->type, type0->len_fully_qualified*sizeof(*type0->type))) return NULL;
        }
    }
    return v_highest;
}

// checks whether implicit type conversion from the type of the variable v0 to the type of the variable v1 is allowed.
// returns VERB_variable_definition_t* of the variable whose type has the highest rank (the one that does not get converted).
VERB_variable_definition_t* VERB_variable_type_implicit_conversion_allowed(VERB_tokeniser_backend_t* const restrict backend, VERB_variable_definition_t* const restrict v0, VERB_variable_definition_t* const restrict v1){
// types are equal :PP
// if this fails, we KNOW v0 != v1 and thus everything is qualified with restrict.
    if(v0->type == v1->type) return v0;
// by definition, VERB_VARIABLE_DEFINITION_FLAG_strict does not allow any implicit conversion.
    if(v0->flags & VERB_VARIABLE_DEFINITION_FLAG_strict || v1->flags & VERB_VARIABLE_DEFINITION_FLAG_strict) return NULL;
    
    VERB_variable_definition_t* const restrict t0 = VERB_variable_search_byToken(backend, v0->type_name);
    VERB_variable_definition_t* const restrict t1 = VERB_variable_search_byToken(backend, v1->type_name);
// should be marked as compiler/debug error.
    if(t0->group != VERB_VARIABLE_DEFINITION_TYPE || t1->group != VERB_VARIABLE_DEFINITION_TYPE) return NULL;

    VERB_variable_type_t *const restrict type0 = t0->custom_data, *const restrict type1 = t1->custom_data;

    VERB_variable_type_conversion_t* const restrict conv0 = VERB_rht_search(&type0->implicit_conversion, &t1->name, sizeof(t1->name));  // implicit conversion from t0 to t1.
    VERB_variable_type_conversion_t* const restrict conv1 = VERB_rht_search(&type1->implicit_conversion, &t0->name, sizeof(t0->name));  // implicit conversion from t1 to t0.
    if(conv0 && conv1)
        return VERB_variable_type_conversion_check_flags(conv0, v0, conv1, v1);
// if conversion is not defined for t0->t1 or t1->t0, returns the var for which conversion IS defined.
// if conv0 exists, then implicit conversion from t0 to t1 exists; v1 is the final type.
// if conv0 doesn't exist, then conv1 exists and implicit conversion from t1 to t0 exists and v0 is the final type.
    if(conv0 || conv1) return conv0? v1: v0;
// neither exists; return NULL.
    return NULL;
}

// returns VERB_variable_definition_t* of the variable whose type has the higher priority, NULL if neither.
// priority specifies that the other must be converted to ours before ours is converted to the other.
bool VERB_variable_type_conversion_check_flags_bool(VERB_variable_type_conversion_t* const restrict conv0, VERB_variable_definition_t* const restrict v0, VERB_variable_type_conversion_t* const restrict conv1, VERB_variable_definition_t* const restrict v1){
    VERB_variable_type_conversion_t* const restrict conv_highest = conv1->rank > conv0->rank? conv1: conv0;

    if(conv_highest->flag != VERB_VARIABLE_TYPE_CONVERSION_FLAG_NONE){
        if(conv_highest->flag & VERB_VARIABLE_TYPE_CONVERSION_FLAG_FULLY_QUALIFIED){
            VERB_type_t *const restrict type0 = v0->type, *const restrict type1 = v1->type;
            if(memcmp(type0->type, type1->type, type0->len_fully_qualified*sizeof(*type0->type))) return false;
        }
    }
    return true;
}

bool VERB_variable_type_implicit_conversion_allowed_bool(VERB_tokeniser_backend_t* const restrict backend, VERB_variable_definition_t* const restrict v0, VERB_variable_definition_t* const restrict v1){
    // types are equal :PP
// if this fails, we KNOW v0 != v1 and thus everything is qualified with restrict.
    if(v0->type == v1->type) return true;
// by definition, VERB_VARIABLE_DEFINITION_FLAG_strict does not allow any implicit conversion.
    if(v0->flags & VERB_VARIABLE_DEFINITION_FLAG_strict || v1->flags & VERB_VARIABLE_DEFINITION_FLAG_strict) return false;
    
    VERB_variable_definition_t* const restrict t0 = VERB_variable_search_byToken(backend, v0->type_name);
    VERB_variable_definition_t* const restrict t1 = VERB_variable_search_byToken(backend, v1->type_name);
// should be marked as compiler/debug error.
    if(t0->group != VERB_VARIABLE_DEFINITION_TYPE || t1->group != VERB_VARIABLE_DEFINITION_TYPE) return false;

    VERB_variable_type_t *const restrict type0 = t0->custom_data, *const restrict type1 = t1->custom_data;

    VERB_variable_type_conversion_t* const restrict conv0 = VERB_rht_search(&type0->implicit_conversion, &t1->name, sizeof(t1->name));  // implicit conversion from t0 to t1.
    VERB_variable_type_conversion_t* const restrict conv1 = VERB_rht_search(&type1->implicit_conversion, &t0->name, sizeof(t0->name));  // implicit conversion from t1 to t0.
// returns true if any implicit conversion is defined.
    if(conv0 || conv1) 
        return VERB_variable_type_conversion_check_flags_bool(conv0, v0, conv1, v1);
    return false;
}

// checks whether explicit type conversion from t0 to t1 is allowed.
// returns VERB_variable_definition_t* of the type being converted to.
VERB_variable_definition_t* VERB_variable_type_explicit_conversion_allowed(VERB_variable_definition_t* const restrict t0, VERB_variable_definition_t* const restrict t1){
    if(t0->group != VERB_VARIABLE_DEFINITION_TYPE || t1->group != VERB_VARIABLE_DEFINITION_TYPE) return false;
    
    VERB_variable_type_t* const restrict type0 = t0->custom_data;

    if(VERB_rht_search_bool(&type0->implicit_conversion, &t1->name, sizeof(t1->name))) return t1;
    if(VERB_rht_search_bool(&type0->explicit_conversion, &t1->name, sizeof(t1->name))) return t1;
    return NULL;
}


#endif
