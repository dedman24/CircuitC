/*
   NOAHZK_bigint reference source code package - reference C implementations

   Copyright 2025, dedmanwalking <dedmanwalking@proton.me>.  You may use this under the
   terms of the CC0 1.0 Universal license, linked below:
   - CC0 1.0 Universal : http://creativecommons.org/publicdomain/zero/1.0
*/

#ifndef NOAHZK_bigint_sub_included
#define NOAHZK_bigint_sub_included

#include "definitions.h"    // NOAHZK variable-width type
#include "stdint.h"         // integer types

// BE WARY OF HOW C CONVERTS BETWEEN UNSIGNED TYPES!
// it's not the fault of C moreso I'm just dumb

void NOAHZK_variable_width_sub_handle_borrow(NOAHZK_variable_width_t* const dst, const NOAHZK_limb_t rs0_sign, const NOAHZK_limb_t rs1_sign, const NOAHZK_limb_t bout){
// see handle_cout
    const NOAHZK_limb_t new_sign = NOAHZK_variable_width_get_sign(dst);
    if(rs0_sign || !rs1_sign) dst->sign = new_sign;
    else dst->sign = 0;

    if(bout){
// see truth table above
        if(rs0_sign != rs1_sign){
// negative - positive -> negative 
            if(rs0_sign && !dst->sign) NOAHZK_variable_width_resize_by_one(dst, -bout);
// positive - negative -> positive (effectively addition)
            else if(!rs0_sign) NOAHZK_variable_width_resize_by_one(dst, bout);
        }        
    } 
}

// negates src according to the contents of op in constant time.
// returns two's complement of src unsigned if op is 1, otherwise src unmodified if it is 0.
uint64_t NOAHZK_variable_width_conditionally_negate(const uint64_t src, NOAHZK_op_t op){
    return (src ^ -(uint64_t)op) + op;
}

// dst = rs0 + or - (by virtue of op) rs1; constant-time regardless of op as long as add and sub take equal time in the CPU
void NOAHZK_variable_width_add_or_sub(NOAHZK_variable_width_t* const dst, const NOAHZK_variable_width_t* const rs0, const NOAHZK_variable_width_t* const rs1, NOAHZK_op_t op){
    uint64_t borrow = 0;
// in LISP (used as pseudocode here), subtraction may be defined as (+ being addition, ~ being bitwise negation)
// (define (- a b) (+ a (~ b) 1))    
// we want to invert b if op is 1, and add opposite along with it, in which case we treat borrow specially.
// to negate b we have to:
//      extend b to occupy the whole byte somehow and XOR it with b
// (define (cond-not x op) (^ x (- op)))
// (define (plus-or-minus a b op) (+ a (cond-not b op) op))

    for(uint64_t i = 0; i < dst->width; i++){
        const NOAHZK_limb_t rs1_limb = NOAHZK_variable_width_get_arr(rs1->arr, rs1->width, rs1->sign, i);
// if op is 1, then -op = -1 = UINT64_MAX (after type conversion) which when xored computes the one's complement of rs1_limb.
// the two's complement is computed when op is added back
        const uint64_t negated_rs1_limb = NOAHZK_variable_width_conditionally_negate(rs1_limb, op);

        uint64_t z = NOAHZK_variable_width_get_arr(rs0->arr, rs0->width, rs0->sign, i) + negated_rs1_limb + borrow;
        dst->arr[i] = z & NOAHZK_LIMB_MAX;
        borrow = NOAHZK_variable_width_conditionally_negate(NOAHZK_variable_width_get_out(z), op);
    }

    NOAHZK_variable_width_update_sign(dst);
}

// dst = rs0 + or - (by virtue of op) k; constant-time regardless of op
void NOAHZK_variable_width_add_or_sub_constant(NOAHZK_variable_width_t* const dst, const NOAHZK_variable_width_t* const rs0, const uint64_t k, NOAHZK_op_t op){
    uint64_t borrow = 0;

    for(uint64_t i = 0; i < dst->width; i++){
        const NOAHZK_limb_t rs1_limb = NOAHZK_get_section_from_var(k, NOAHZK_LIMB_MAX, i, NOAHZK_limb_t);
        const uint64_t negated_rs1_limb = NOAHZK_variable_width_conditionally_negate(rs1_limb, op);

        const uint64_t z = NOAHZK_variable_width_get_arr(rs0->arr, rs0->width, rs0->sign, i) + negated_rs1_limb + borrow;
        dst->arr[i] = z & NOAHZK_LIMB_MAX;
        borrow = NOAHZK_variable_width_conditionally_negate(NOAHZK_variable_width_get_out(z), op);
    }

    NOAHZK_variable_width_update_sign(dst);
}

NOAHZK_limb_t NOAHZK_variable_width_sub_primitive(NOAHZK_limb_t* const dst, const NOAHZK_limb_t* const rs0, const NOAHZK_limb_t* const rs1, const uint64_t width_dst, const uint64_t width0, const uint64_t width1, const NOAHZK_limb_t sign0, const NOAHZK_limb_t sign1){
    NOAHZK_limb_t borrow = 0;

    for(uint64_t i = 0; i < width_dst; i++){
        uint64_t z = NOAHZK_variable_width_get_arr(rs0, width0, sign0, i) - NOAHZK_variable_width_get_arr(rs1, width1, sign1, i) - borrow;
        dst[i] = z & NOAHZK_LIMB_MAX;
        borrow = NOAHZK_variable_width_get_out(z);
    }
    return borrow;
}

NOAHZK_limb_t NOAHZK_variable_width_sub_constant_primitive(NOAHZK_limb_t* const dst, const NOAHZK_limb_t* const rs0, const uint64_t k, const uint64_t width_dst, const uint64_t width0, const NOAHZK_limb_t sign0){
    NOAHZK_limb_t borrow = 0;

    for(uint64_t i = 0; i < width_dst; i++){
        uint64_t z = NOAHZK_variable_width_get_arr(rs0, width0, sign0, i) - NOAHZK_get_section_from_var(k, NOAHZK_LIMB_MAX, i, NOAHZK_limb_t) - borrow;
        dst[i] = z & NOAHZK_LIMB_MAX;
        borrow = NOAHZK_variable_width_get_out(z);
    }
    return borrow;
}

void NOAHZK_variable_width_sub(NOAHZK_variable_width_t* const dst, const NOAHZK_variable_width_t* const rs0, const NOAHZK_variable_width_t* const rs1){
    NOAHZK_variable_width_sub_primitive(dst->arr, rs0->arr, rs1->arr, dst->width, rs0->width, rs1->width, rs0->sign, rs1->sign);
    NOAHZK_variable_width_update_sign(dst);
}

void NOAHZK_variable_width_sub_constant(NOAHZK_variable_width_t* const dst, const NOAHZK_variable_width_t* const rs0, const uint64_t k){
    NOAHZK_variable_width_sub_constant_primitive(dst->arr, rs0->arr, k, dst->width, rs0->width, rs0->sign);
    NOAHZK_variable_width_update_sign(dst);
}

// for sub ops where dst may have a size of 0, initialises dst's width to the width of the smallest src operand.
void NOAHZK_variable_width_sub_and_resize(NOAHZK_variable_width_t* const dst, const NOAHZK_variable_width_t* const rs0, const NOAHZK_variable_width_t* const rs1){
    NOAHZK_variable_width_resize_to_largest(dst, rs0->width, rs1->width);
    const NOAHZK_limb_t bout = NOAHZK_variable_width_sub_primitive(dst->arr, rs0->arr, rs1->arr, dst->width, rs0->width, rs1->width, rs0->sign, rs1->sign);
    NOAHZK_variable_width_sub_handle_borrow(dst, rs0->sign, rs1->sign, bout);
}

void NOAHZK_variable_width_sub_and_resize_constant(NOAHZK_variable_width_t* const dst, const NOAHZK_variable_width_t* const rs0, const uint64_t k){
    NOAHZK_variable_width_resize_to_largest(dst, rs0->width, sizeof(k)/sizeof(NOAHZK_limb_t));
    const NOAHZK_limb_t bout = NOAHZK_variable_width_sub_constant_primitive(dst->arr, rs0->arr, k, dst->width, rs0->width, rs0->sign);
    NOAHZK_variable_width_sub_handle_borrow(dst, rs0->sign, 1, bout);     // constants that are subtracted are ALWAYS negative
}

#endif
