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
#include "alloca.h"         // static allocation
#include "stdint.h"         // integer types
#include "stdlib.h"         // dynamic memory operations
#include "string.h"         // memset
#include "type.h"           // ops to allocate, destroy variable width types 
#include "add.h"            // variable-width addition 

// not constant time, no clue on how one would implement this in constant time
void NOAHZK_variable_width_shift_right(NOAHZK_variable_width_t* const dst, const NOAHZK_variable_width_t* const src, const uint64_t shamt){
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
void NOAHZK_variable_width_mul_byte(void* const dst, const void* const rs0, const void* const rs1, const uint64_t width0, const uint64_t width1){
// handles common cases quickly
    if(width0 == 0 && width1 == 0) return;
    if(width0 == 0 || width1 == 0) return;
    if(width0 == sizeof(uint8_t) && width1 == sizeof(uint8_t)){
        *(uint16_t*)dst = (uint16_t)( *(uint8_t*)rs0 ) * (uint16_t)( *(uint8_t*)rs1 ); 
        return;
    }
    if(width0 == sizeof(uint8_t)){ NOAHZK_variable_width_mul_arr_with_byte(dst, rs1, *(uint8_t*)rs0, width1); return; }
    if(width1 == sizeof(uint8_t)){ NOAHZK_variable_width_mul_arr_with_byte(dst, rs0, *(uint8_t*)rs1, width0); return; }

    const uint64_t n = width0/2, m = width1/2;

    const uint64_t width_X1Y1 = width0-n + width1-m;
    uint8_t X1Y1[width_X1Y1];
    NOAHZK_variable_width_mul_byte(X1Y1, (uint8_t*)rs0 + n, (uint8_t*)rs1 + m, width0-n, width1-m);

    const uint64_t width_X1Y0 = width0-n + m;
    uint8_t X1Y0[width_X1Y0];
    NOAHZK_variable_width_mul_byte(X1Y0, (uint8_t*)rs0 + n, rs1, width0-n, m);
    
    const uint64_t width_X0Y1 = n + width1-m;
    uint8_t X0Y1[width_X0Y1];
    NOAHZK_variable_width_mul_byte(X0Y1, rs0, (uint8_t*)rs1 + m, n, width1-m);
    
    const uint64_t width_X0Y0 = n + m;
    uint8_t X0Y0[width_X0Y0];
    NOAHZK_variable_width_mul_byte(X0Y0, rs0, rs1, n, m);
// HAS to be done at the end because, if we do this at the start and dst == rs0, then we'd be setting rs0 to 0, which is bad.
    memset(dst, 0, width0+width1);
    const uint64_t width_dst = width0 + width1;
    NOAHZK_variable_width_add_with_byte_offset_byte(dst, X0Y0, X0Y1, width_X0Y0, width_X0Y1, width_dst, m);
    NOAHZK_variable_width_add_with_byte_offset_byte(dst, dst,  X1Y0, width_dst,  width_X1Y0, width_dst, n);
    NOAHZK_variable_width_add_with_byte_offset_byte(dst, dst,  X1Y1, width_dst,  width_X1Y1, width_dst, n+m);
}

void NOAHZK_variable_width_mul_constant_byte(void* const dst, const void* const rs0, const uint64_t k, const uint64_t width0){
    uint64_t bytes_k = NOAHZK_min_bytecnt_var(k);
    NOAHZK_variable_width_mul_byte(dst, rs0, &k, width0, bytes_k);
}

// multiplies two variable width variables together, returns the result in dst
// constant time
void NOAHZK_variable_width_mul(NOAHZK_variable_width_t* const dst, const NOAHZK_variable_width_t* const rs0, const NOAHZK_variable_width_t* const rs1){
    const uint64_t new_width = rs0->width + rs1->width;
    dst->arr = realloc(dst->arr, NOAHZK_GET_WIDTH_FROM_VAR_WIDTH_TYPE_INT(new_width));
// does not need to set dst's extra space to 0 because NOAHZK_variable_width_mul_byte already sets everything to 0
    NOAHZK_variable_width_mul_byte(dst->arr, rs0->arr, rs1->arr, NOAHZK_GET_WIDTH_FROM_VAR_WIDTH_TYPE_PTR(rs0), NOAHZK_GET_WIDTH_FROM_VAR_WIDTH_TYPE_PTR(rs1));
// needs to be done after everything because rs0, rs1, dst may all alias
    dst->width = new_width;
}

void NOAHZK_variable_width_mul_constant(NOAHZK_variable_width_t* const dst, const NOAHZK_variable_width_t* const rs0, const uint64_t k){
    const uint64_t limbs_k = NOAHZK_GET_LIMB_WIDTH_FROM_INT(NOAHZK_min_bytecnt_var(k));
    const uint64_t new_width = rs0->width + limbs_k;
    dst->arr = realloc(dst->arr, NOAHZK_GET_WIDTH_FROM_VAR_WIDTH_TYPE_INT(new_width));

    NOAHZK_variable_width_mul_byte(dst->arr, rs0->arr, &k, NOAHZK_GET_WIDTH_FROM_VAR_WIDTH_TYPE_PTR(rs0), NOAHZK_GET_WIDTH_FROM_VAR_WIDTH_TYPE_INT(limbs_k));
    dst->width = new_width;
}

void NOAHZK_variable_width_mul_both_constants(NOAHZK_variable_width_t* const dst, const uint64_t k0, const uint64_t k1){
    const uint64_t bytes_k0 = NOAHZK_min_bytecnt_var(k0);
    const uint64_t bytes_k1 = NOAHZK_min_bytecnt_var(k1);
// no need to clear dst->arr
    dst->width = NOAHZK_GET_LIMB_WIDTH_FROM_INT(bytes_k0 + bytes_k1);
    dst->arr = realloc(dst->arr, NOAHZK_GET_WIDTH_FROM_VAR_WIDTH_TYPE_PTR(dst));

    NOAHZK_variable_width_mul_byte(dst->arr, &k0, &k1, bytes_k0, bytes_k1);
}

// dst = rs0*rs1*rs1
void NOAHZK_variable_width_mul_by_square(NOAHZK_variable_width_t* const dst, const NOAHZK_variable_width_t* const rs0, const NOAHZK_variable_width_t* const rs1){
    NOAHZK_variable_width_t product = NOAHZK_variable_width_INITIALIZER;
    NOAHZK_variable_width_mul(&product, rs1, rs1);

    NOAHZK_variable_width_add_and_resize(dst, rs0, &product);
    NOAHZK_variable_width_destroy(&product, NOAHZK_variable_width_keep_ptr);
}

void NOAHZK_variable_width_square(NOAHZK_variable_width_t* const dst, NOAHZK_variable_width_t* const src){
    NOAHZK_variable_width_mul(dst, src, src);
}

void NOAHZK_variable_width_square_constant(NOAHZK_variable_width_t* const dst, const uint64_t k){
    NOAHZK_variable_width_mul_both_constants(dst, k, k);
}

void NOAHZK_variable_width_mul_by_square_constant(NOAHZK_variable_width_t* const dst, const NOAHZK_variable_width_t* const rs0, const uint64_t k){
    NOAHZK_variable_width_t product = NOAHZK_variable_width_INITIALIZER;
    NOAHZK_variable_width_square_constant(&product, k);

    NOAHZK_variable_width_add_and_resize(dst, rs0, &product);
    NOAHZK_variable_width_destroy(&product, NOAHZK_variable_width_keep_ptr);
}

// dst = rs0 * rs1**power, where power >= 0 (because it's an unsigned 64-bit integer)
void NOAHZK_variable_width_mul_to_power_constant(NOAHZK_variable_width_t* const dst, const NOAHZK_variable_width_t* const rs0, const NOAHZK_variable_width_t* const rs1, const uint64_t power){
    NOAHZK_variable_width_t* product = NOAHZK_variable_width_init_constant(alloca(sizeof(NOAHZK_variable_width_t)), 1);
    for(uint64_t i = 0; i < power; i++) NOAHZK_variable_width_mul(product, product, rs1);

    NOAHZK_variable_width_add_and_resize(dst, rs0, product);
    NOAHZK_variable_width_destroy(product, NOAHZK_variable_width_keep_ptr);
}

// dst = rs0 * k**power, where power >= 0 (because it's an unsigned 64-bit integer)
void NOAHZK_variable_width_mul_by_constant_to_power_constant(NOAHZK_variable_width_t* const dst, const NOAHZK_variable_width_t* const rs0, const uint64_t k, const uint64_t power){
    NOAHZK_variable_width_t* product = NOAHZK_variable_width_init_constant(alloca(sizeof(NOAHZK_variable_width_t)), 1);
    for(uint64_t i = 0; i < power; i++) NOAHZK_variable_width_mul_constant(product, product, k);

    NOAHZK_variable_width_add_and_resize(dst, rs0, product);
    NOAHZK_variable_width_destroy(product, NOAHZK_variable_width_keep_ptr);
}

// dst += src*k
void NOAHZK_variable_width_madd_constant(NOAHZK_variable_width_t* const dst, const NOAHZK_variable_width_t* const src, const uint64_t k){
    if(!src->width) return;

    NOAHZK_variable_width_t product = NOAHZK_variable_width_INITIALIZER;
    NOAHZK_variable_width_mul_constant(&product, src, k);

    NOAHZK_variable_width_add_and_resize(dst, dst, &product);
    NOAHZK_variable_width_destroy(&product, NOAHZK_variable_width_keep_ptr);
}

// dst = (dst + rs1)*rs2, where all are variable-width vars
void NOAHZK_variable_width_add_and_mul_and_resize(void* const dst, const NOAHZK_variable_width_t* const rs1, const NOAHZK_variable_width_t* const rs2){
    NOAHZK_variable_width_add_and_resize(dst, dst, rs1);
    NOAHZK_variable_width_mul(dst, dst, rs2);
}


// dst = (dst + rs1)*rs2, where dst is a byte array & rs1 and rs2 are variable-width vars
// constant-time for safety reasons  
void NOAHZK_variable_width_add_and_mul_into_byte(void* dst, const NOAHZK_variable_width_t* const rs1, const NOAHZK_variable_width_t* const rs2, const uint64_t width0){
    NOAHZK_variable_width_t* rs0 =  NOAHZK_variable_width_init_arr(alloca(sizeof(NOAHZK_variable_width_t)), dst, width0);

    NOAHZK_variable_width_add(rs0, rs0, rs1);
    NOAHZK_variable_width_mul(rs0, rs0, rs2);

    NOAHZK_variable_width_copy_to_arr(dst, width0, rs0);
    NOAHZK_variable_width_destroy(rs0, NOAHZK_variable_width_keep_ptr);
}

// not constant-time
void* NOAHZK_variable_width_nth_triangle_number(NOAHZK_variable_width_t* dst, const uint64_t n){
    if(!dst) dst = NOAHZK_variable_width_init(NULL, 0);
    NOAHZK_variable_width_square_constant(dst, n);

    NOAHZK_variable_width_add_and_resize_constant(dst, dst, n);
    NOAHZK_variable_width_shift_right(dst, dst, 1);

    return dst;
}

#endif
