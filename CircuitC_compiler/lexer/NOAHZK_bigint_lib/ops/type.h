/*
   NOAHZK_bigint reference source code package - reference C implementations

   Copyright 2025, dedmanwalking <dedmanwalking@proton.me>.  You may use this under the
   terms of the CC0 1.0 Universal license, linked below:
   - CC0 1.0 Universal : http://creativecommons.org/publicdomain/zero/1.0
*/

#ifndef NOAHZK_bigint_type_included
#define NOAHZK_bigint_type_included

#include "definitions.h"    // NOAHZK variable-width type
#include "logarithms.h"     // bit and byte width of integer values and arrays
#include "stdint.h"         // integer types
#include "string.h"         // memset, memcpy
#include "stdlib.h"         // dynamic memory operations

void* NOAHZK_variable_width_init(NOAHZK_variable_width_t* toinit, const uint64_t width_in_bytes){
    if(!toinit) toinit = malloc(sizeof(NOAHZK_variable_width_t));

    const uint64_t width = NOAHZK_SIZE_AS_ARR_OF_TYPE(width_in_bytes, sizeof(NOAHZK_limb_t));
    if(width){
        toinit->arr = calloc(1, NOAHZK_GET_WIDTH_FROM_VAR_WIDTH_TYPE_INT(width));
        toinit->width = width;
    }
    else{
        toinit->arr = NULL;
        toinit->width = 0;
    }
    return toinit;
}

// initializes toinit to array "arr" of width (in bytes) "width_in_bytes", or returns ptr to NOAHZK_variable_width_t that holds array holding the same contents of arr
void* NOAHZK_variable_width_init_arr(NOAHZK_variable_width_t* toinit, const void* const arr, const uint64_t width_in_bytes){
    if(!toinit) toinit = malloc(sizeof(NOAHZK_variable_width_t));

    const uint64_t width = NOAHZK_SIZE_AS_ARR_OF_TYPE(width_in_bytes, sizeof(NOAHZK_limb_t));
    if(width){
        toinit->arr = calloc(1, NOAHZK_GET_WIDTH_FROM_VAR_WIDTH_TYPE_INT(width));
        toinit->width = width;
        memcpy(toinit->arr, arr, width_in_bytes);
    }
    else{
        toinit->arr = NULL;
        toinit->width = 0;
    }
    return toinit;
}

// initializes toinit to k, or returns ptr to NOAHZK_variable_width_t that holds k
void* NOAHZK_variable_width_init_constant(NOAHZK_variable_width_t* toinit, const uint64_t k){
    if(!toinit) toinit = malloc(sizeof(NOAHZK_variable_width_t));

    const uint64_t width_in_bits = NOAHZK_min_bitcnt_var(k);
    const uint64_t width = NOAHZK_SIZE_AS_ARR_OF_TYPE(width_in_bits, BITS_IN_NOAHZK_LIMB);
    if(width){
        toinit->arr = malloc(NOAHZK_GET_WIDTH_FROM_VAR_WIDTH_TYPE_INT(width));
        memcpy(toinit->arr, &k, NOAHZK_GET_WIDTH_FROM_VAR_WIDTH_TYPE_INT(width));

        toinit->width = width;
    }
    else{
        toinit->arr = NULL;
        toinit->width = 0;
    }
    return toinit;
}

// initializes toinit to k, or returns ptr to NOAHZK_variable_width_t that holds k
// does so in constant time in regards to k by allocating a constant number of bytes for it
void* NOAHZK_variable_width_init_constant_in_constant_time(NOAHZK_variable_width_t* toinit, const uint64_t k){
    if(!toinit) toinit = malloc(sizeof(NOAHZK_variable_width_t));

    const uint64_t width = NOAHZK_SIZE_AS_ARR_OF_TYPE(sizeof(k), sizeof(NOAHZK_limb_t));
    if(width){
        toinit->arr = malloc(NOAHZK_GET_WIDTH_FROM_VAR_WIDTH_TYPE_INT(width));
        memcpy(toinit->arr, &k, NOAHZK_GET_WIDTH_FROM_VAR_WIDTH_TYPE_INT(width));

        toinit->width = width;
    }
    else{
        toinit->arr = NULL;
        toinit->width = 0;
    }
    return toinit;
}

void* NOAHZK_variable_width_copy(NOAHZK_variable_width_t* restrict dst, const NOAHZK_variable_width_t* const restrict src){
    if(!dst) dst = malloc(sizeof(NOAHZK_variable_width_t));

    dst->width = src->width;
    dst->arr = malloc(NOAHZK_GET_WIDTH_FROM_VAR_WIDTH_TYPE_PTR(src));

    memcpy(dst->arr, src->arr, NOAHZK_GET_WIDTH_FROM_VAR_WIDTH_TYPE_PTR(src));

    return dst;
}

void* NOAHZK_variable_width_copy_to_arr(void* dst, uint64_t width_dst, const NOAHZK_variable_width_t* const src){
    if(!width_dst) width_dst = NOAHZK_GET_WIDTH_FROM_VAR_WIDTH_TYPE_PTR(src);
    if(!dst) dst = malloc(width_dst);

    memcpy(dst, src->arr, width_dst);

    return dst;
}

// moves fields from src to dst, clears src. doesn't free it.
// the two may not point to the same object
// allocates dst if NULL is passed
void* NOAHZK_variable_width_move(NOAHZK_variable_width_t* restrict dst, NOAHZK_variable_width_t* const restrict src){
    if(!dst) dst = malloc(sizeof(NOAHZK_variable_width_t));

    dst->width = src->width; src->width = 0;
    dst->arr = src->arr; src->arr = NULL;

    return dst;
}

typedef enum{ NOAHZK_variable_width_keep_ptr, NOAHZK_variable_width_free_ptr } NOAHZK_variable_width_option_t;

void NOAHZK_variable_width_destroy(NOAHZK_variable_width_t* const todestroy, const NOAHZK_variable_width_option_t freeptr){
    if(todestroy->arr){
        memset(todestroy->arr, 0, NOAHZK_GET_WIDTH_FROM_VAR_WIDTH_TYPE_PTR(todestroy));
        memset(&todestroy->width, 0, sizeof(todestroy->width));
        free(todestroy->arr);
    } 

    if(freeptr == NOAHZK_variable_width_free_ptr) free(todestroy);
}

#endif
