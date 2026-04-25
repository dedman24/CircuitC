/*
   NOAHZK_bigint reference source code package - reference C implementations

   Copyright 2025, dedmanwalking <dedmanwalking@proton.me>.  You may use this under the
   terms of the CC0 1.0 Universal license, linked below:
   - CC0 1.0 Universal : http://creativecommons.org/publicdomain/zero/1.0
*/

#ifndef NOAHZK_bigint_logic_included
#define NOAHZK_bigint_logic_included

#include "definitions.h"        // variable-width type definition
#include "type.h"               // NOAHZK_variable_width_copy_properties
#include "add.h"                // NOAHZK_variable_width_add_constant

void NOAHZK_variable_width_invert_primitive(NOAHZK_limb_t* const dst, const NOAHZK_limb_t* const src, const uint64_t width_dst, const uint64_t width_src){
    for(uint64_t i = 0; i < width_dst; i++) dst[i] = i < width_src? ~src[i]: NOAHZK_LIMB_MAX;
}

// dst = ~src
void NOAHZK_variable_width_invert(NOAHZK_variable_width_t* const dst, const NOAHZK_variable_width_t* const src){
    NOAHZK_variable_width_invert_primitive(dst->arr, src->arr, dst->width, src->width);
    NOAHZK_variable_width_update_sign(dst);
}

// dst = ~src + 1; effectively the two's complement of src
void NOAHZK_variable_width_negate(NOAHZK_variable_width_t* const dst, const NOAHZK_variable_width_t* const src){
    NOAHZK_variable_width_invert(dst, src);
    NOAHZK_variable_width_add_constant(dst, dst, 1);
}

void NOAHZK_variable_width_invert_conditionally_primitive(NOAHZK_limb_t* const dst, const NOAHZK_limb_t* const src, const uint64_t width_dst, const uint64_t width_src, NOAHZK_op_t op){
    const NOAHZK_limb_t mask = -op;
    
    for(uint64_t i = 0; i < width_dst; i++) dst[i] = (i < width_src? src[i]: NOAHZK_LIMB_MAX) ^ mask;
}

// if op == 1, negates src and stores it in dst
// if op == 0, copies src to dst
// the value of op may be either 1 or 0. undefined what happens otherwise
// all in constant time, regardless of the value of op B))
void NOAHZK_variable_width_negate_conditionally(NOAHZK_variable_width_t* const dst, const NOAHZK_variable_width_t* const src, NOAHZK_op_t op){
    NOAHZK_variable_width_invert_conditionally_primitive(dst->arr, src->arr, dst->width, src->width, op);
// we don't have to update the sign because add_constant updates it later and the sign being updated or not doesn't change anything in add_constant (in this case).
    NOAHZK_variable_width_add_constant(dst, dst, op);
}

// dst = abs(src)
void NOAHZK_variable_width_abs(NOAHZK_variable_width_t* const dst, const NOAHZK_variable_width_t* const src){
    NOAHZK_variable_width_negate_conditionally(dst, src, src->sign);
}

#endif
