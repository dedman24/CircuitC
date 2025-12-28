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

// dst = rs0 + or - (by virtue of op) rs1; constant-time regardless of op as long as add and sub take equal time in the CPU
void NOAHZK_variable_width_add_or_sub(NOAHZK_variable_width_t* const dst, const NOAHZK_variable_width_t* const rs0, const NOAHZK_variable_width_t* const rs1, const NOAHZK_op_t op){
    NOAHZK_limb_t borrow = 0;

// in LISP (used as pseudocode here), subtraction may be defined as (+ being addition, ~ being bitwise negation)
// (define (- a b) (+ a (~ b) 1))    
// we want to invert b if op is 1, and add opposite along with it, in which case we treat borrow specially.
// to negate b we have to:
//      extend b to occupy the whole byte somehow and XOR it with b
// (define (cond-not x op) (^ x (- op)))
// (define (plus-or-minus a b op) (+ a (cond-not b op) op))

    for(uint64_t i = 0; i < dst->width; i++){
        NOAHZK_limb_t rs1_limb = i < rs1->width? rs1->arr[i]: 0;
        rs1_limb ^= -((NOAHZK_limb_t)op);               // if op is 1, then -op = -1 = NOAHZK_LIMB_MAX (after type conversion) which when xored negates rs1_limb. 

        uint64_t z = (uint64_t)(i < rs0->width? rs0->arr[i]: 0) + (uint64_t)rs1_limb + op + borrow;
        dst->arr[i] = z & NOAHZK_LIMB_MAX;
        borrow = z >> BITS_IN_NOAHZK_LIMB;
    }
}

// dst = rs0 + or - (by virtue of op) k; constant-time regardless of op as long as add and sub take equal time in the CPU
void NOAHZK_variable_width_add_or_sub_constant(NOAHZK_variable_width_t* const dst, const NOAHZK_variable_width_t* const rs0, const uint64_t k, const NOAHZK_op_t op){
    NOAHZK_limb_t borrow = 0;

    for(uint64_t i = 0; i < dst->width; i++){
        NOAHZK_limb_t rs1_limb = NOAHZK_get_section_from_var(k, NOAHZK_LIMB_MAX, i, NOAHZK_limb_t);
        rs1_limb ^= -((NOAHZK_limb_t)op); 

        uint64_t z = (uint64_t)(i < rs0->width? rs0->arr[i]: 0) + (uint64_t)rs1_limb + op + borrow;
        dst->arr[i] = z & NOAHZK_LIMB_MAX;
        borrow = z >> BITS_IN_NOAHZK_LIMB;
    }
}

void NOAHZK_variable_width_sub(NOAHZK_variable_width_t* const dst, const NOAHZK_variable_width_t* const rs0, const NOAHZK_variable_width_t* const rs1){
    NOAHZK_limb_t borrow = 0;

    for(uint64_t i = 0; i < dst->width; i++){
// borrow goes into the 33rd bit, which can be extracted in constant-time assuming constant-time shifts
        uint64_t z = (uint64_t)(i < rs0->width? rs0->arr[i]: 0) - (uint64_t)(i < rs1->width? rs1->arr[i]: 0) + borrow;
        dst->arr[i] = z & NOAHZK_LIMB_MAX;
        borrow = z >> BITS_IN_NOAHZK_LIMB;
    }
}

void NOAHZK_variable_width_sub_constant(NOAHZK_variable_width_t* const dst, const NOAHZK_variable_width_t* const rs0, const uint64_t k){
    NOAHZK_limb_t borrow = 0;

    for(uint64_t i = 0; i < dst->width; i++){
// borrow goes into the 33rd bit, which can be extracted in constant-time assuming constant-time shifts
        uint64_t z = (uint64_t)(i < rs0->width? rs0->arr[i]: 0) - NOAHZK_get_section_from_var(k, NOAHZK_LIMB_MAX, i, NOAHZK_limb_t) - borrow;
        dst->arr[i] = z & NOAHZK_LIMB_MAX;
        borrow = z >> BITS_IN_NOAHZK_LIMB & 1;
    }
}

// for sub ops where dst may have a size of 0, initialises dst's width to the width of the smallest src operand.
void NOAHZK_variable_width_sub_and_resize(NOAHZK_variable_width_t* const dst, const NOAHZK_variable_width_t* const rs0, const NOAHZK_variable_width_t* const rs1){
    NOAHZK_limb_t borrow = 0;

    const uint64_t largest_width = NOAHZK_MAX(rs0->width, rs1->width); 
// expands dst to size of largest operand, initializing new space to 0 
    if(dst->width < largest_width){
        dst->arr = realloc(dst->arr, NOAHZK_GET_WIDTH_FROM_VAR_WIDTH_TYPE_INT(largest_width));
        memset(dst->arr + dst->width, 0, NOAHZK_GET_WIDTH_FROM_VAR_WIDTH_TYPE_INT(largest_width - dst->width));
        dst->width = largest_width;
    }

    for(uint64_t i = 0; i < dst->width; i++){
// borrow goes into the 33rd bit, which can be extracted in constant-time assuming constant-time shifts
        uint64_t z = (uint64_t)(i < rs0->width? rs0->arr[i]: 0) - (uint64_t)(i < rs1->width? rs1->arr[i]: 0) + borrow;
        dst->arr[i] = z & NOAHZK_LIMB_MAX;
        borrow = z >> BITS_IN_NOAHZK_LIMB;
    }
}

void NOAHZK_variable_width_sub_and_resize_constant(NOAHZK_variable_width_t* const dst, const NOAHZK_variable_width_t* const rs0, const uint64_t k){
    NOAHZK_limb_t borrow = 0;

    const uint64_t largest_width = NOAHZK_MAX(rs0->width, sizeof(k)/sizeof(*rs0->arr)); 
// expands dst to size of largest operand, initializing new space to 0 
    if(dst->width < largest_width){
        dst->arr = realloc(dst->arr, NOAHZK_GET_WIDTH_FROM_VAR_WIDTH_TYPE_INT(largest_width));
        memset(dst->arr + dst->width, 0, NOAHZK_GET_WIDTH_FROM_VAR_WIDTH_TYPE_INT(largest_width - dst->width));
        dst->width = largest_width;
    }

    for(uint64_t i = 0; i < dst->width; i++){
// borrow goes into the 33rd bit, which can be extracted in constant-time assuming constant-time shifts
        uint64_t z = (uint64_t)(i < rs0->width? rs0->arr[i]: 0) - NOAHZK_get_section_from_var(k, NOAHZK_LIMB_MAX, i, NOAHZK_limb_t) - borrow;
        dst->arr[i] = z & NOAHZK_LIMB_MAX;
        borrow = z >> BITS_IN_NOAHZK_LIMB & 1;
    }
}

void NOAHZK_variable_width_neg_byte(void* const real_dst, const void* const real_src, const uint64_t width){
    uint8_t carry = 1;
    uint8_t *dst = real_dst;
    const uint8_t *src = real_src;

    for(uint64_t i = 0; i < width; i++){
// if subtraction overflows and requires a borrow, z (when interpreted as a signed type) is negative and thus bit 15 is set. this also means that bit 8 is set, so we can check that for borrow. 
        uint16_t z = (uint16_t)(~src[i]) + carry;
        dst[i] = z & UINT8_MAX;
        carry = z >> BITS_IN_UINT8_T;
    }
}

void NOAHZK_variable_width_sub_with_bit_offset_byte(void* const real_dst, const void* const real_rs0, const void* const real_rs1, const uint64_t width0, const uint64_t width1, const uint64_t width_result, const uint64_t bit_offset){
    int8_t borrow = 0;
    uint8_t *dst = real_dst;
    const uint8_t *rs0 = real_rs0, *rs1 = real_rs1;

    const uint64_t byte_offset = bit_offset/BITS_IN_UINT8_T;

    for(uint64_t i = byte_offset; i < width_result; i++){
// runs in constant time regardless of conditional statements because those statements are NOT DATA-DEPENDENT; widths of operands in NOAHZK are public.
// correctly applies offset because shifts rs1 by bit_offset modulo BITS_IN_UINT8_T
// the top 8 bits of z are set to the "overflow/shifted data" that doesn't fit inside the bottom 8 bits, to which the carry out from the previous addition is also added
// the top 8 bits are set as the carry for the next repetition and this is repeated
// we have the top bits that didn't fit in 8 bits of the previous addtition stored inside carry and added to the bottom part of the next addition
// idk how good this explanation is.
        uint16_t z = (uint16_t)(i < width0? rs0[i]: 0) - (i-byte_offset < width1? (uint16_t)rs1[i-byte_offset] << (bit_offset % BITS_IN_UINT8_T): 0) + (int16_t)borrow;

        dst[i] = z & UINT8_MAX;
        borrow = z >> BITS_IN_UINT8_T;
    }
}

// compiles to constant-time code on any cpu with constant-time shifts.
void NOAHZK_variable_width_sub_byte(void* const real_dst, const void* const real_rs0, const void* const real_rs1, const uint64_t width){
    uint8_t borrow = 0;
    uint8_t *dst = real_dst;
    const uint8_t *rs0 = real_rs0, *rs1 = real_rs1;

    for(uint64_t i = 0; i < width; i++){
// if subtraction overflows and requires a borrow, z (when interpreted as a signed type) is negative and thus bit 15 is set. this also means that bit 8 is set, so we can check that for borrow. 
        uint16_t z = (uint16_t)rs0[i] - (uint16_t)rs1[i] - borrow;
        dst[i] = z & UINT8_MAX;
        borrow = z >> BITS_IN_UINT8_T & 1;
    }
}

// compiles to constant-time code on any cpu with constant-time shifts.
void NOAHZK_variable_width_sub_constant_byte(void* const real_dst, const void* const real_rs0, const uint64_t k, const uint64_t width){
    uint8_t borrow = 0;
    uint8_t *dst = real_dst;
    const uint8_t *rs0 = real_rs0;

    for(uint64_t i = 0; i < width; i++){
// if subtraction overflows and requires a borrow, z (when interpreted as a signed type) is negative and thus bit 15 is set. this also means that bit 8 is set, so we can check that for borrow. 
        uint16_t z = (uint16_t)rs0[i] - NOAHZK_get_section_from_var(k, UINT8_MAX, i, uint8_t) - borrow;
        dst[i] = z & UINT8_MAX;
        borrow = z >> BITS_IN_UINT8_T & 1;
    }
}

// compiles to constant-time code on any cpu with constant-time shifts.
void NOAHZK_variable_width_both_sub_byte(void* const real_dst, const void* const real_rs0, const void* const real_rs1, const uint64_t width0, const uint64_t width1, const uint64_t width_result){
    uint8_t borrow = 0;
    uint8_t *dst = real_dst;
    const uint8_t *rs0 = real_rs0, *rs1 = real_rs1;

    for(uint64_t i = 0; i < width_result; i++){
// if subtraction overflows and requires a borrow, z (when interpreted as a signed type) is negative and thus bit 15 is set. this also means that bit 8 is set, so we can check that for borrow. 
// runs in constant time regardless of conditional statements because those statements are NOT DATA-DEPENDENT; widths of operands in NOAHZK are public.
        uint16_t z = (uint16_t)(i < width0? rs0[i]: 0) - (uint16_t)(i < width1? rs1[i]: 0) - borrow;
        dst[i] = z & UINT8_MAX;
        borrow = z >> BITS_IN_UINT8_T & 1;
    }
}

#endif
