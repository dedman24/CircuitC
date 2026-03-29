/*
   NOAHZK_bigint reference source code package - reference C implementations

   Copyright 2025, dedmanwalking <dedmanwalking@proton.me>.  You may use this under the
   terms of the CC0 1.0 Universal license, linked below:
   - CC0 1.0 Universal : http://creativecommons.org/publicdomain/zero/1.0
*/

#ifndef NOAHZK_bigint_definitions_included
#define NOAHZK_bigint_definitions_included

#include "stdint.h"     // integer types
#include "stdlib.h"     // dynamic memory handling
#include "string.h"     // memset, memcpy & so on
#include "stdio.h"      // DEBUG

#define NOAHZK_BIGINT_OP_ADD 0
#define NOAHZK_BIGINT_OP_SUB 1

typedef uint32_t NOAHZK_limb_t;
typedef const NOAHZK_limb_t NOAHZK_op_t;        // NOAHZK_bigint relies on NOAHZK_op_t secretely being NOAHZK_limb_t, so do not change this!
#define NOAHZK_LIMB_MAX UINT32_MAX

#define BITS_IN_UINT64_T    64
#define BITS_IN_UINT32_T    32
#define BITS_IN_UINT8_T     8
#define BITS_IN_NOAHZK_LIMB (sizeof(NOAHZK_limb_t)*BITS_IN_UINT8_T)

#define NOAHZK_convert_from_bits_to_bytes(x) (((x)/BITS_IN_UINT8_T) + ((x)%BITS_IN_UINT8_T != 0))
// gets section from variable; say variable is 0x01234567; NOAHZK_get_section_from_var(variable, UINT8_MAX, 0, uint8_t) will return a value of the same type as var holding 0x67
#define NOAHZK_get_section_from_var(var, section_mask, section, section_type) ((section) < sizeof(var)/sizeof(section_type)? var >> (section)*sizeof(section_type)*BITS_IN_UINT8_T & section_mask: 0)

#define NOAHZK_MAX(x, y) ((x) > (y)? (x): (y))
#define NOAHZK_MIN(x, y) ((x) < (y)? (x): (y))
#define NOAHZK_SWP(x, y) (x) = (x) ^ (y); (y) = (x) ^ (y); (x) = (x) ^ (y)
// I think this is portable, compilers will optimize this anyways.
#define NOAHZK_SWP_PTR(x, y)    {                       \
                                    void* temp = x;     \
                                    x = (void*)y;       \
                                    y = temp;           \
                                }
#define NOAHZK_SIZE_AS_ARR_OF_TYPE(size, size_type) (((size)/size_type) + ((size)%size_type != 0))
// gets width in limbs of a variable of width x
#define NOAHZK_GET_LIMB_WIDTH_FROM_INT(x)           ((x)/sizeof(NOAHZK_limb_t) + ((x)%sizeof(NOAHZK_limb_t) != 0))
#define NOAHZK_GET_WIDTH_FROM_VAR_WIDTH_TYPE(x)     ((x). width*sizeof(NOAHZK_limb_t))
#define NOAHZK_GET_WIDTH_FROM_VAR_WIDTH_TYPE_PTR(x) ((x)->width*sizeof(NOAHZK_limb_t))
// width of said var-width type in number of limbs is passed directly
#define NOAHZK_GET_WIDTH_FROM_VAR_WIDTH_TYPE_INT(x) ((x)       *sizeof(NOAHZK_limb_t))
#define NOAHZK_GET_WIDTH_FROM_VAR_WIDTH_TYPE_PTR_BITS(x) ((x)->width*BITS_IN_NOAHZK_LIMB)

#define NOAHZK_variable_width_INITIALISER {0, NULL, 0}

typedef struct{
    uint64_t width;
    NOAHZK_limb_t* arr;
    NOAHZK_limb_t sign; 
} NOAHZK_variable_width_t;

// SIGN HANDLING RULES:
//      when an add_and_resize overflows, it sets the extra space to whatever's appropriate:
//          a+b ~ a is positive, b is positive -> dst is positive, extend by one and set to cout
//                a is positive, b is negative -> dst is positive, do nothing
//                a is negative, b is positive -> dst is positive, do nothing
//                a is negative, b is negative -> dst is negative, extend by one and set to -cout only if the dst's sign is positive
//      when a sub_and_resize overflows, it sets the extra space to whatever's appropriate: 
//          a-b ~ a is positive, b is positive -> dst is positive, do nothing
//                a is positive, b is negative -> dst is positive, extend by one and set to bout
//                a is negative, b is positive -> dst is negative, extend by one and set to -bout only if the dst's sign is negative
//                a is negative, b is negative -> dst is positive, do nothing
//      TODO: sign in mul? also sign handling has greater issues when _byte operations are involved.

// occasionally useful 

// guaranteed to be constant-time

// gets carry/borrow
NOAHZK_limb_t NOAHZK_variable_width_get_out(const uint64_t z){ 
    return z >> BITS_IN_NOAHZK_LIMB & 1;
}

// guaranteed to be constant-time as long as src->width is constant

// this produces either 0||arr[index], 0 or 0||UINT32_MAX. this is by design, this SHOULD be correct.
uint64_t NOAHZK_variable_width_get_arr(const NOAHZK_limb_t* const arr, const uint64_t width, const NOAHZK_limb_t sign, const uint64_t index){
    return index < width? arr[index]: -sign;
}

NOAHZK_limb_t NOAHZK_variable_width_get_sign(const NOAHZK_variable_width_t* const src){
    return src->arr[src->width - 1] >> (BITS_IN_NOAHZK_LIMB - 1) & 1;
}

void NOAHZK_variable_width_update_sign(NOAHZK_variable_width_t* const src){
    src->sign = NOAHZK_variable_width_get_sign(src);
}

// not guaranteed to be constant-time even under assumptions aboce 

void NOAHZK_variable_width_resize_to_largest(NOAHZK_variable_width_t* const dst, const uint64_t width0, const uint64_t width1){
    const uint64_t largest_width = NOAHZK_MAX(width0, width1); 
// expands dst to size of largest operand, initializing new space to 0 
    if(dst->width < largest_width){
        dst->arr = realloc(dst->arr, NOAHZK_GET_WIDTH_FROM_VAR_WIDTH_TYPE_INT(largest_width));
        memset(dst->arr + dst->width, -dst->sign, NOAHZK_GET_WIDTH_FROM_VAR_WIDTH_TYPE_INT(largest_width - dst->width));
        dst->width = largest_width;
    }
}

// specifically for mul ops
// where width0, width1 are widths in number of limbs
uint64_t NOAHZK_variable_width_resize_to_sum(NOAHZK_variable_width_t* const dst, const uint64_t width0, const uint64_t width1){
    const uint64_t new_width = width0 + width1;
    dst->arr = realloc(dst->arr, NOAHZK_GET_WIDTH_FROM_VAR_WIDTH_TYPE_INT(new_width));
    return new_width;
}

void NOAHZK_variable_width_resize_by_one(NOAHZK_variable_width_t* const toresize, const NOAHZK_limb_t toput){
    toresize->arr = realloc(toresize->arr, NOAHZK_GET_WIDTH_FROM_VAR_WIDTH_TYPE_INT(toresize->width + 1));
    toresize->arr[toresize->width] = toput;
    toresize->width++;
}

#endif
