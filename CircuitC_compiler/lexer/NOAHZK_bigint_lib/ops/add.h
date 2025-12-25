/*
   NOAHZK_bigint reference source code package - reference C implementations

   Copyright 2025, dedmanwalking <dedmanwalking@proton.me>.  You may use this under the
   terms of the CC0 1.0 Universal license, linked below:
   - CC0 1.0 Universal : http://creativecommons.org/publicdomain/zero/1.0
*/

#ifndef NOAHZK_bigint_add_included
#define NOAHZK_bigint_add_included

#include "definitions.h"    // NOAHZK variable-width type
#include "stdint.h"         // integer types
#include "stdlib.h"         // dynamic memory operations
#include "string.h"         // memset

// do not define any of these as restrict; it is vlaid for them to be aliased

// compiles to constant-time code on any cpu with constant-time shifts.
// used for proving, so having it be constant-time is integral
void NOAHZK_variable_width_add(NOAHZK_variable_width_t* dst, NOAHZK_variable_width_t* rs0, NOAHZK_variable_width_t* rs1){
    NOAHZK_limb_t carry = 0;

    for(uint64_t i = 0; i < dst->width; i++){
// carry-out goes into the 33rd bit, which can be extracted in constant-time assuming constant-time shifts
        uint64_t z = (uint64_t)(i < rs0->width? rs0->arr[i]: 0) + (uint64_t)(i < rs1->width? rs1->arr[i]: 0) + carry;
        dst->arr[i] = z & NOAHZK_LIMB_MAX;
        carry = z >> BITS_IN_NOAHZK_LIMB;
    }
}

void NOAHZK_variable_width_add_constant(NOAHZK_variable_width_t* dst, NOAHZK_variable_width_t* rs0, const uint64_t k){
    NOAHZK_limb_t carry = 0;

    for(uint64_t i = 0; i < dst->width; i++){
// carry-out goes into the 33rd bit, which can be extracted in constant-time assuming constant-time shifts
        uint64_t z = (uint64_t)(i < rs0->width? rs0->arr[i]: 0) + NOAHZK_get_section_from_var(k, NOAHZK_LIMB_MAX, i, NOAHZK_limb_t) + carry;
        dst->arr[i] = z & NOAHZK_LIMB_MAX;
        carry = z >> BITS_IN_NOAHZK_LIMB;
    }
}

// NOT CONSTANT-TIME!!! as resizing happens based on what rs0 and rs1 hold.
void NOAHZK_variable_width_add_and_resize(NOAHZK_variable_width_t* dst, NOAHZK_variable_width_t* rs0, NOAHZK_variable_width_t* rs1){
    NOAHZK_limb_t carry = 0;

    const uint64_t largest_width = NOAHZK_MAX(rs0->width, rs1->width); 
// expands dst to size of largest operand, initializing new space to 0 
    if(dst->width < largest_width){
        dst->arr = realloc(dst->arr, NOAHZK_GET_WIDTH_FROM_VAR_WIDTH_TYPE_INT(largest_width));
        memset(dst->arr + dst->width, 0, NOAHZK_GET_WIDTH_FROM_VAR_WIDTH_TYPE_INT(largest_width - dst->width));
        dst->width = largest_width;
    }

    for(uint64_t i = 0; i < dst->width; i++){
        uint64_t z = (uint64_t)(i < rs0->width? rs0->arr[i]: 0) + (uint64_t)(i < rs1->width? rs1->arr[i]: 0) + carry;
        dst->arr[i] = z & NOAHZK_LIMB_MAX;
        carry = z >> BITS_IN_NOAHZK_LIMB;
    }
// expands dst by one byte in case of carry
    if(carry){
        dst->arr = realloc(dst->arr, (dst->width + 1)*sizeof(NOAHZK_limb_t));
        dst->arr[dst->width] = carry;
        dst->width++;
    }
}

// NOT CONSTANT-TIME!!! as resizing happens based on what rs0 and rs1 hold.
void NOAHZK_variable_width_add_and_resize_constant(NOAHZK_variable_width_t* dst, NOAHZK_variable_width_t* rs0, const uint64_t k){
    NOAHZK_limb_t carry = 0;

    const uint64_t largest_width = NOAHZK_MAX(rs0->width, sizeof(k)/sizeof(*rs0->arr)); 
// expands dst to size of largest operand, initializing new space to 0 
    if(dst->width < largest_width){
        dst->arr = realloc(dst->arr, NOAHZK_GET_WIDTH_FROM_VAR_WIDTH_TYPE_INT(largest_width));
        memset(dst->arr + dst->width, 0, NOAHZK_GET_WIDTH_FROM_VAR_WIDTH_TYPE_INT(largest_width - dst->width));
        dst->width = largest_width;
    }

    for(uint64_t i = 0; i < dst->width; i++){
        uint64_t z = (uint64_t)(i < rs0->width? rs0->arr[i]: 0) + NOAHZK_get_section_from_var(k, NOAHZK_LIMB_MAX, i, NOAHZK_limb_t) + carry;
        dst->arr[i] = z & NOAHZK_LIMB_MAX;
        carry = z >> BITS_IN_NOAHZK_LIMB;
    }
// expands dst by one byte in case of carry
    if(carry){
        dst->arr = realloc(dst->arr, (dst->width + 1)*sizeof(NOAHZK_limb_t));
        dst->arr[dst->width] = carry;
        dst->width++;
    }
}

void NOAHZK_variable_width_add_with_bit_offset_byte(void* real_dst, const void* real_rs0, const void* real_rs1, const uint64_t width0, const uint64_t width1, const uint64_t width_result, uint64_t bit_offset){
    uint8_t carry = 0;
    uint8_t *dst = real_dst;
    const uint8_t *rs0 = real_rs0, *rs1 = real_rs1;

    const uint64_t byte_offset = bit_offset/BITS_IN_UINT8_T;

    for(uint64_t i = 0; i < width_result; i++){
// runs in constant time regardless of conditional statements because those statements are NOT DATA-DEPENDENT; widths of operands in NOAHZK are public.
// correctly applies offset because shifts rs1 by bit_offset modulo BITS_IN_UINT8_T
// the top 8 bits of z are set to the "overflow/shifted data" that doesn't fit inside the bottom 8 bits, to which the carry out from the previous addition is also added
// the top 8 bits are set as the carry for the next repetition and this is repeated
// we have the top bits that didn't fit in 8 bits of the previous addtition stored inside carry and added to the bottom part of the next addition
// idk how good this explanation is.
        uint16_t z = (uint16_t)(i < width0? rs0[i]: 0) + (i < byte_offset? 0: (i-byte_offset < width1? (uint16_t)rs1[i-byte_offset] << (bit_offset%BITS_IN_UINT8_T): 0)) + carry;

        dst[i] = z & UINT8_MAX;
        carry = z >> BITS_IN_UINT8_T;
    }
}

void NOAHZK_variable_width_add_with_byte_offset_byte(void* real_dst, const void* real_rs0, const void* real_rs1, const uint64_t width0, const uint64_t width1, const uint64_t width_result, uint64_t byte_offset){
    uint8_t carry = 0;
    uint8_t *dst = real_dst;
    const uint8_t *rs0 = real_rs0, *rs1 = real_rs1;

    for(uint64_t i = 0; i < width_result; i++){
// runs in constant time regardless of conditional statements because those statements are NOT DATA-DEPENDENT; widths of operands in NOAHZK are public.
        uint16_t z = (uint16_t)(i < width0? rs0[i]: 0) + (i < byte_offset? 0: (i-byte_offset < width1? (uint16_t)rs1[i-byte_offset]: 0)) + carry;

        dst[i] = z & UINT8_MAX;
        carry = z >> BITS_IN_UINT8_T;
    }
}

// compiles to constant-time code on any cpu with constant-time shifts.
// used for proving, so having it be constant-time is integral
void NOAHZK_variable_width_add_constant_byte(void* real_dst, const void* real_rs0, const uint64_t k, const uint64_t width0, const uint64_t width_result){
    uint8_t carry = 0;
    uint8_t *dst = real_dst;
    const uint8_t *rs0 = real_rs0;

    for(uint64_t i = 0; i < width_result; i++){
// carry-out goes into the ninth bit, which can be extracted in constant-time assuming constant-time shifts
// runs in constant time regardless of conditional statements because those statements are NOT DATA-DEPENDENT; widths of operands in NOAHZK are public.
        uint16_t z = (uint16_t)(i < width0? rs0[i]: 0) + NOAHZK_get_section_from_var(k, UINT8_MAX, i, uint8_t) + carry;
        dst[i] = z & UINT8_MAX;
        carry = z >> BITS_IN_UINT8_T;
    }
}

// compiles to constant-time code on any cpu with constant-time shifts.
// used for proving, so having it be constant-time is integral
void NOAHZK_variable_width_add_byte(void* real_dst, const void* real_rs0, const void* real_rs1, const uint64_t width0, const uint64_t width1, const uint64_t width_result){
    uint8_t carry = 0;
    uint8_t *dst = real_dst;
    const uint8_t *rs0 = real_rs0, *rs1 = real_rs1;

    for(uint64_t i = 0; i < width_result; i++){
// carry-out goes into the ninth bit, which can be extracted in constant-time assuming constant-time shifts
// runs in constant time regardless of conditional statements because those statements are NOT DATA-DEPENDENT; widths of operands in NOAHZK are public.
        uint16_t z = (uint16_t)(i < width0? rs0[i]: 0) + (uint16_t)(i < width1? rs1[i]: 0) + carry;
        dst[i] = z & UINT8_MAX;
        carry = z >> BITS_IN_UINT8_T;
    }
}

// dst += src, where dst is a byte array and src a variable-width variable
void NOAHZK_variable_width_add_vwv_to_byte(void* dst, NOAHZK_variable_width_t* src, const uint64_t width){
    NOAHZK_variable_width_add_byte(dst, dst, src->arr, width, NOAHZK_GET_WIDTH_FROM_VAR_WIDTH_TYPE_PTR(src), width);
}

#endif
