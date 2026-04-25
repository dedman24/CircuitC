/*
   NOAHZK_bigint reference source code package - reference C implementations

   Copyright 2025, dedmanwalking <dedmanwalking@proton.me>.  You may use this under the
   terms of the CC0 1.0 Universal license, linked below:
   - CC0 1.0 Universal : http://creativecommons.org/publicdomain/zero/1.0
*/

#ifndef NOAHZK_bigint_mul_included
#define NOAHZK_bigint_mul_included

#include "definitions.h"    // NOAHZK variable-width type
#include "logarithms.h"     // bit and byte width of constants and arrays
#include "stdint.h"         // integer types
#include "stdlib.h"         // dynamic memory operations
#include "string.h"         // memset
#include "logic.h"          // NOAHZK_variable_width_negate_conditionally
#include "type.h"           // ops to allocate, destroy variable width types 
#include "add.h"            // variable-width addition 
#include "sub.h"            // variable-width subtraction

// only constant-time if shamt is.
void NOAHZK_variable_width_shift_right_constant(NOAHZK_variable_width_t* const dst, const NOAHZK_variable_width_t* const src, const uint64_t shamt){
    NOAHZK_limb_t dst_arr[dst->width];
    memset(dst_arr, 0, NOAHZK_GET_WIDTH_FROM_VAR_WIDTH_TYPE_PTR(dst));
    if(shamt == 0) memcpy(dst_arr, dst->arr, NOAHZK_GET_WIDTH_FROM_VAR_WIDTH_TYPE_PTR(dst));
    else if(shamt != NOAHZK_GET_WIDTH_FROM_VAR_WIDTH_TYPE_PTR_BITS(dst)){
        for(uint64_t i = NOAHZK_GET_WIDTH_FROM_VAR_WIDTH_TYPE_PTR_BITS(dst) - 1; i >= shamt; i--)
            dst_arr[(i - shamt)/BITS_IN_NOAHZK_LIMB] |= (src->arr[i/BITS_IN_NOAHZK_LIMB] >> (i % BITS_IN_NOAHZK_LIMB) & 1) << ((i - shamt)%BITS_IN_NOAHZK_LIMB);
    }

    memcpy(dst->arr, dst_arr, NOAHZK_GET_WIDTH_FROM_VAR_WIDTH_TYPE_PTR(dst));
}

// multiplies array rs0 with byte rs1
void NOAHZK_variable_width_mul_arr_with_byte(uint8_t* const dst, const uint8_t* const rs0, const uint8_t rs1, const uint64_t width){
    uint16_t product = 0;
    for(uint64_t i = 0; i < width; i++){
        product += (uint16_t)rs0[i] * (uint16_t)rs1;
        dst[i] = product;
        product >>= BITS_IN_UINT8_T;        // this way if dst == rs1 dst[i] isn't overwritten until after we're done with rs1[i]
    }
    dst[width] = product;
}

// basically 4-mul version of Karatsuba algorithm
// had problems trying to implement 3-mul version
// constant-time 
// uses VLAs; I don't want to use dynamic memory allocation as it'd just be slow.
void NOAHZK_variable_width_mul_byte(void* const dst, const void* const rs0, const void* const rs1, const uint64_t width0, const uint64_t width1, const uint64_t width_dst){
// handles common cases quickly
    if(width0 == 0 && width1 == 0) return;
    if(width0 == 0 || width1 == 0){ memset(dst, 0, width0+width1); return; }
    if(width0 == sizeof(uint8_t) && width1 == sizeof(uint8_t)){
        *(uint16_t*)dst = (uint16_t)( *(uint8_t*)rs0 ) * (uint16_t)( *(uint8_t*)rs1 ); 
        return;
    }
    if(width0 == sizeof(uint8_t)){ NOAHZK_variable_width_mul_arr_with_byte(dst, rs1, *(uint8_t*)rs0, width1); return; }
    if(width1 == sizeof(uint8_t)){ NOAHZK_variable_width_mul_arr_with_byte(dst, rs0, *(uint8_t*)rs1, width0); return; }
/*
// consider adding these in the future
    if(width0 == sizeof(uint16_t) && width1 == sizeof(uint16_t)){ *(uint32_t*)dst = (uint32_t)( *(uint16_t*)rs0 ) * (uint32_t)( *(uint16_t*)rs1 ); return; }
    if(width0 == sizeof(uint32_t) && width1 == sizeof(uint32_t)){ *(uint64_t*)dst = (uint64_t)( *(uint32_t*)rs0 ) * (uint64_t)( *(uint32_t*)rs1 ); return; }
*/

    const uint64_t n = width0/2, m = width1/2;

    const uint64_t width_X1Y1 = width0-n + width1-m;
    uint8_t X1Y1[width_X1Y1];
    NOAHZK_variable_width_mul_byte(X1Y1, (uint8_t*)rs0 + n, (uint8_t*)rs1 + m, width0-n, width1-m, width_X1Y1);

    const uint64_t width_X1Y0 = width0-n + m;
    uint8_t X1Y0[width_X1Y0];
    NOAHZK_variable_width_mul_byte(X1Y0, (uint8_t*)rs0 + n, rs1, width0-n, m, width_X1Y0);
    
    const uint64_t width_X0Y1 = n + width1-m;
    uint8_t X0Y1[width_X0Y1];
    NOAHZK_variable_width_mul_byte(X0Y1, rs0, (uint8_t*)rs1 + m, n, width1-m, width_X0Y1);
    
    const uint64_t width_X0Y0 = n + m;
    uint8_t X0Y0[width_X0Y0];
    NOAHZK_variable_width_mul_byte(X0Y0, rs0, rs1, n, m, width_X0Y0);
// HAS to be done at the end because, if we do this at the start and dst == rs0, then we'd be setting rs0 to 0, which is bad.
    memset(dst, 0, width_dst);
    NOAHZK_variable_width_add_with_byte_offset_byte(dst, X0Y0, X0Y1, width_X0Y0, width_X0Y1, width_dst, m);
    NOAHZK_variable_width_add_with_byte_offset_byte(dst, dst,  X1Y0, width_dst,  width_X1Y0, width_dst, n);
    NOAHZK_variable_width_add_with_byte_offset_byte(dst, dst,  X1Y1, width_dst,  width_X1Y1, width_dst, n+m);
}

// variable-width mul primitives

void NOAHZK_variable_width_mul_primitive(NOAHZK_variable_width_t* const dst, const NOAHZK_variable_width_t* const rs0, const NOAHZK_variable_width_t* const rs1, const uint64_t new_dst_width){
    NOAHZK_variable_width_t abs_rs0, abs_rs1;
    NOAHZK_variable_width_copy_properties(&abs_rs0, rs0);
    NOAHZK_variable_width_copy_properties(&abs_rs1, rs1);
    NOAHZK_variable_width_abs(&abs_rs0, rs0);
    NOAHZK_variable_width_abs(&abs_rs1, rs1);

    NOAHZK_variable_width_mul_byte(dst->arr, abs_rs0.arr, abs_rs1.arr, NOAHZK_GET_WIDTH_FROM_VAR_WIDTH_TYPE(abs_rs0), NOAHZK_GET_WIDTH_FROM_VAR_WIDTH_TYPE(abs_rs1), NOAHZK_GET_WIDTH_FROM_VAR_WIDTH_TYPE_INT(new_dst_width));

    NOAHZK_variable_width_destroy(&abs_rs0, NOAHZK_variable_width_keep_ptr);
    NOAHZK_variable_width_destroy(&abs_rs1, NOAHZK_variable_width_keep_ptr);
// updates width after multiplication so negate_conditionally operates over the correct area
// done after because dst, rs0, rs1 may ALL alias
    dst->width = new_dst_width;
    const NOAHZK_limb_t dst_sign = rs0->sign ^ rs1->sign;
    NOAHZK_variable_width_negate_conditionally(dst, dst, dst_sign);
}

// faster than signed counterpart; assumes both rs0 and rs1 have sign == 0. do not use if this cannot be guaranteed.
void NOAHZK_variable_width_mul_unsigned_primitive(NOAHZK_variable_width_t* const dst, const NOAHZK_variable_width_t* const rs0, const NOAHZK_variable_width_t* const rs1, const uint64_t new_dst_width){
    NOAHZK_variable_width_mul_byte(dst->arr, rs0->arr, rs1->arr, NOAHZK_GET_WIDTH_FROM_VAR_WIDTH_TYPE_PTR(rs0), NOAHZK_GET_WIDTH_FROM_VAR_WIDTH_TYPE_PTR(rs1), NOAHZK_GET_WIDTH_FROM_VAR_WIDTH_TYPE_INT(new_dst_width));
    dst->width = new_dst_width;
    dst->sign = 0;
}

void NOAHZK_variable_width_mul_constant_primitive(NOAHZK_variable_width_t* const dst, const NOAHZK_variable_width_t* const rs0, const uint64_t k, const uint64_t width_k, const uint64_t new_dst_width){
    NOAHZK_variable_width_t abs_rs0;
    NOAHZK_variable_width_copy_properties(&abs_rs0, rs0);
    NOAHZK_variable_width_abs(&abs_rs0, rs0);

    NOAHZK_variable_width_mul_byte(dst->arr, abs_rs0.arr, &k, NOAHZK_GET_WIDTH_FROM_VAR_WIDTH_TYPE(abs_rs0), width_k, NOAHZK_GET_WIDTH_FROM_VAR_WIDTH_TYPE_INT(new_dst_width));

    NOAHZK_variable_width_destroy(&abs_rs0, NOAHZK_variable_width_keep_ptr);
    dst->width = new_dst_width;
    const NOAHZK_limb_t dst_sign = rs0->sign;
    NOAHZK_variable_width_negate_conditionally(dst, dst, dst_sign);
}

// functions that use said primitives

// multiplies two variable width variables together, returns the result in dst
// does so in constant time.
void NOAHZK_variable_width_mul(NOAHZK_variable_width_t* const dst, const NOAHZK_variable_width_t* const rs0, const NOAHZK_variable_width_t* const rs1){
    NOAHZK_variable_width_mul_primitive(dst, rs0, rs1, dst->width);
}

// k always treated as positive
void NOAHZK_variable_width_mul_constant(NOAHZK_variable_width_t* const dst, const NOAHZK_variable_width_t* const rs0, const uint64_t k, const uint64_t width_k){
    NOAHZK_variable_width_mul_constant_primitive(dst, rs0, k, width_k, dst->width);
}

// multiplies two variable width variables together, returns the result in dst
void NOAHZK_variable_width_mul_and_resize(NOAHZK_variable_width_t* const dst, const NOAHZK_variable_width_t* const rs0, const NOAHZK_variable_width_t* const rs1){
    const uint64_t new_width = NOAHZK_variable_width_resize_to_sum(dst, rs0->width, rs1->width);
    NOAHZK_variable_width_mul_primitive(dst, rs0, rs1, new_width);
}

// multiplies two variable width variables together, returns the result in dst
void NOAHZK_variable_width_mul_and_resize_unsigned(NOAHZK_variable_width_t* const dst, const NOAHZK_variable_width_t* const rs0, const NOAHZK_variable_width_t* const rs1){
    const uint64_t new_width = NOAHZK_variable_width_resize_to_sum(dst, rs0->width, rs1->width);
    NOAHZK_variable_width_mul_unsigned_primitive(dst, rs0, rs1, new_width);
}

void NOAHZK_variable_width_mul_and_resize_constant(NOAHZK_variable_width_t* const dst, const NOAHZK_variable_width_t* const rs0, const NOAHZK_limb_t k){
    const uint64_t limbs_k = 1;
    const uint64_t new_width = NOAHZK_variable_width_resize_to_sum(dst, rs0->width, limbs_k);

    NOAHZK_variable_width_mul_constant_primitive(dst, rs0, k, NOAHZK_GET_WIDTH_FROM_VAR_WIDTH_TYPE_INT(limbs_k), new_width);
}

void NOAHZK_variable_width_square_and_resize_unsigned(NOAHZK_variable_width_t* const dst, const NOAHZK_variable_width_t* const src){
    NOAHZK_variable_width_mul_and_resize_unsigned(dst, src, src);
}

// dst = rs0 * rs1**power, where power is an unsigned 64-bit integer
void NOAHZK_variable_width_mul_to_power_and_resize_constant(NOAHZK_variable_width_t* const dst, const NOAHZK_variable_width_t* const rs0, const NOAHZK_variable_width_t* const rs1, const uint64_t power){
    NOAHZK_variable_width_t product; 
    NOAHZK_variable_width_init_and_resize_unsigned_constant(&product, 1);
    for(uint64_t i = 0; i < power; i++) NOAHZK_variable_width_mul_and_resize(&product, &product, rs1);

    NOAHZK_variable_width_mul_and_resize(dst, rs0, &product);
    NOAHZK_variable_width_destroy(&product, NOAHZK_variable_width_keep_ptr);
}

// dst += rs0*rs1
void NOAHZK_variable_width_madd_and_resize(NOAHZK_variable_width_t* const dst, const NOAHZK_variable_width_t* const rs0, const NOAHZK_variable_width_t* const rs1){
    NOAHZK_variable_width_t product = NOAHZK_variable_width_INITIALISER;
    NOAHZK_variable_width_mul_and_resize(&product, rs0, rs1);

    NOAHZK_variable_width_add_and_resize(dst, dst, &product);
    NOAHZK_variable_width_destroy(&product, NOAHZK_variable_width_keep_ptr);
}

// dst = (dst + rs1)*rs2, where all are variable-width vars.
void NOAHZK_variable_width_add_and_mul(NOAHZK_variable_width_t* const dst, const NOAHZK_variable_width_t* const rs1, const NOAHZK_variable_width_t* const rs2){
    NOAHZK_variable_width_add(dst, dst, rs1);
    NOAHZK_variable_width_mul(dst, dst, rs2);
}

// dst = (dst + rs1)*rs2, where all are variable-width vars; resizes to appropriate width.
void NOAHZK_variable_width_add_and_mul_and_resize(NOAHZK_variable_width_t* const dst, const NOAHZK_variable_width_t* const rs1, const NOAHZK_variable_width_t* const rs2){
    NOAHZK_variable_width_add_and_resize(dst, dst, rs1);
    NOAHZK_variable_width_mul_and_resize(dst, dst, rs2);
}

// not constant-time
// computes the (n-1)th triangle number
void* NOAHZK_variable_width_n_minus_1_th_triangle_number(NOAHZK_variable_width_t* dst, const NOAHZK_variable_width_t* const restrict n){
    if(!dst) dst = NOAHZK_variable_width_init(NULL, 0);

    NOAHZK_variable_width_t n1 = NOAHZK_variable_width_INITIALISER;
    NOAHZK_variable_width_sub_and_resize_constant(&n1, n, 1);                   // n1 = n-1
    NOAHZK_variable_width_square_and_resize_unsigned(dst, &n1);                 // dst = n1**2
    NOAHZK_variable_width_add_and_resize(dst, dst, &n1);                        // dst = n1**2 + n1
    NOAHZK_variable_width_destroy(&n1, NOAHZK_variable_width_keep_ptr);         // // no longer needed
    NOAHZK_variable_width_shift_right_constant(dst, dst, 1);                    // dst = (n1**2 + n1)/2

    return dst;
}

#endif
