/*
   NOAHZK_bigint reference source code package - reference C implementations

   Copyright 2025, dedmanwalking <dedmanwalking@proton.me>.  You may use this under the
   terms of the CC0 1.0 Universal license, linked below:
   - CC0 1.0 Universal : http://creativecommons.org/publicdomain/zero/1.0
*/

#ifndef NOAHZK_bigint_logarithm_included
#define NOAHZK_bigint_logarithm_included

#include "definitions.h"    // NOAHZK variable-width type
#include "stdint.h"         // integer types

// runs in constant time 
uint64_t NOAHZK_min_bitcnt_var(const uint64_t value){
// done to avoid clzll of value when value == 0, because that's undefined. basically forces (in constant time) value to 1 whenever it'd actually be 0
    uint8_t number_of_0_bits = __builtin_popcountll(~value);
    uint8_t is_value_0 = (number_of_0_bits - BITS_IN_UINT64_T + 1) & 1 & ~((number_of_0_bits - BITS_IN_UINT64_T) >> (BITS_IN_UINT8_T - 1));
// forces input to __builtin_clzll to 1 if value is 0 (because __builtin_clzll on a 0 is invalid) and subtracts it at the end so log2(0) == 0
    uint64_t result = BITS_IN_UINT64_T - __builtin_clzll(value | is_value_0);
    return result - is_value_0;
}

// runs in constant time 
uint64_t NOAHZK_ceil_log2_value(const uint64_t value){
// done to avoid clzll of value when value == 0, because that's undefined. basically forces (in constant time) value to 1 whenever it'd actually be 0
    uint8_t number_of_0_bits = __builtin_popcountll(~value);
    uint8_t is_value_0 = (number_of_0_bits - BITS_IN_UINT64_T + 1) & 1 & ~((number_of_0_bits - BITS_IN_UINT64_T) >> (BITS_IN_UINT8_T - 1));
    uint64_t is_value_0_flag = -(uint64_t)is_value_0;

    return NOAHZK_min_bitcnt_var(value-1) & ~is_value_0_flag;
}

// runs in constant time 
uint64_t NOAHZK_min_bytecnt_var(const uint64_t value){
    uint64_t min_bitcnt = NOAHZK_min_bitcnt_var(value);
    return min_bitcnt/BITS_IN_UINT8_T + (min_bitcnt%BITS_IN_UINT8_T != 0);
}

uint64_t NOAHZK_variable_width_min_bitcnt_byte(const void* const real_value, uint64_t size){
    uint64_t number_of_leading_zeroes = 0;
    uint8_t flag = 0;

    const uint8_t* const value = real_value;

// exploits unsigned integer overflow so it iterates from top bit to last one
    for(uint64_t i = size - 1; i < size; i--){
        uint8_t number_of_0_bits = __builtin_popcountll(~value[i]);
        uint8_t is_value_0 = (number_of_0_bits - BITS_IN_UINT64_T + 1) & 1 & ~((number_of_0_bits - BITS_IN_UINT64_T) >> (BITS_IN_UINT8_T - 1));

        uint8_t this_byte_number_of_leading_zeroes = __builtin_clzll(value[i] | flag | is_value_0) - (BITS_IN_UINT64_T - BITS_IN_UINT8_T);
        uint8_t is_value_all_zeroes = BITS_IN_UINT8_T - this_byte_number_of_leading_zeroes;
        flag |= (uint16_t)(is_value_all_zeroes - is_value_0) << (__builtin_clzll(is_value_all_zeroes) - (BITS_IN_UINT64_T - BITS_IN_UINT8_T));

        number_of_leading_zeroes += this_byte_number_of_leading_zeroes + is_value_0;
    }

    return size*BITS_IN_UINT8_T - number_of_leading_zeroes;
}

uint64_t NOAHZK_variable_width_min_bitcnt(const NOAHZK_variable_width_t* const value){
    return NOAHZK_variable_width_min_bitcnt_byte(value->arr, NOAHZK_GET_WIDTH_FROM_VAR_WIDTH_TYPE_PTR(value));
}

uint64_t NOAHZK_variable_width_min_bytecnt_byte(const void* const real_value, const uint64_t size){
    uint64_t min_bitcnt = NOAHZK_variable_width_min_bitcnt_byte(real_value, size);
    return min_bitcnt/BITS_IN_UINT8_T + (min_bitcnt%BITS_IN_UINT8_T != 0);
}

uint64_t NOAHZK_variarble_width_min_bytecnt(const NOAHZK_variable_width_t* const value){
    return NOAHZK_variable_width_min_bytecnt_byte(value->arr, NOAHZK_GET_WIDTH_FROM_VAR_WIDTH_TYPE_PTR(value));
}

#endif
