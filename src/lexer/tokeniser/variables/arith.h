#ifndef VERB_tokeniser_variables_arith_included
#define VERB_tokeniser_variables_arith_included

#include "stdint.h"
#include "stdlib.h"
#include "stdbool.h"
#include "../../../libraries/NOAHZK_bigint_lib/noahzk_bigint.h"
#include "variable_definition.h"

// can assume the form of an infinite precision variable-width variable, of a float32 or of a float64.

typedef enum{
    VERB_VARIABLE_ARITH_SINT,           // unsigned integers.
    VERB_VARIABLE_ARITH_UINT,           // signed integers.
    VERB_VARIABLE_ARITH_BOOL,           // booleans.
    VERB_VARIABLE_ARITH_FLOAT32,        // IDK how to make these 100% consistent at compile-time.
    VERB_VARIABLE_ARITH_FLOAT64
} VERB_variable_arith_type_t;

typedef struct{
    VERB_variable_arith_type_t type;
// all the values it can assume.
    union{
        NOAHZK_variable_width_t i;
        bool b;
        float f32;
        double f64;
    };
} VERB_variable_arith_t;

// precision (in bytes) represents how many bytes the variable occupies.
void* VERB_variable_arith_init(const VERB_variable_arith_type_t type, const size_t precision){
    VERB_variable_arith_t* const var = malloc(sizeof(*var));
    var->type = type;
    switch(type){
    case VERB_VARIABLE_ARITH_SINT:
    case VERB_VARIABLE_ARITH_UINT:
        NOAHZK_variable_width_init(&var->i, precision);
        break;
    case VERB_VARIABLE_ARITH_BOOL:
    case VERB_VARIABLE_ARITH_FLOAT32:
    case VERB_VARIABLE_ARITH_FLOAT64:
        break;
    }

    return var;
}

size_t VERB_variable_arith_integer_get_precision(VERB_variable_arith_t* const var){
    return var->i.width;
}

bool VERB_variable_arith_precision_larger(VERB_variable_arith_t* const a0, VERB_variable_arith_t* const a1){
    return a1->i.width > a0->i.width;
}

size_t VERB_variable_arith_precision_max(VERB_variable_arith_t* const a0, VERB_variable_arith_t* const a1){
    return a0->i.width > a1->i.width? a0->i.width: a1->i.width;
}

bool VERB_variable_arith_truthy(VERB_variable_arith_t* const var){
    switch(var->type){
    case VERB_VARIABLE_ARITH_SINT:
    case VERB_VARIABLE_ARITH_UINT:
        if(NOAHZK_variable_width_is0(&var->i)) return false;
        return true;
    case VERB_VARIABLE_ARITH_BOOL:
        return var->b;    
    case VERB_VARIABLE_ARITH_FLOAT32:
        return var->f32 != 0.0f;
    case VERB_VARIABLE_ARITH_FLOAT64:
        return var->f64 != 0.0;
    }
}

#endif
