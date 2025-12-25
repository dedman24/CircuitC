/*
   NOAHZK_bigint reference source code package - reference C implementations

   Copyright 2025, dedmanwalking <dedmanwalking@proton.me>.  You may use this under the
   terms of the CC0 1.0 Universal license, linked below:
   - CC0 1.0 Universal : http://creativecommons.org/publicdomain/zero/1.0
*/

#ifndef NOAHZK_bigint_definitions_included
#define NOAHZK_bigint_definitions_included

#include "stdint.h"     // integer types

#define NOAHZK_BIGINT_OP_ADD 0
#define NOAHZK_BIGINT_OP_SUB 1

typedef uint32_t NOAHZK_limb_t;
typedef const uint32_t NOAHZK_op_t;
#define NOAHZK_LIMB_MAX UINT32_MAX

#define BITS_IN_UINT64_T    64
#define BITS_IN_UINT32_T    32
#define BITS_IN_UINT8_T     8
#define BITS_IN_NOAHZK_LIMB (sizeof(NOAHZK_limb_t)*BITS_IN_UINT8_T)

#define NOAHZK_convert_from_bits_to_bytes(x) (((x)/BITS_IN_UINT8_T) + ((x)%BITS_IN_UINT8_T != 0))
// gets section from variable; say variable is 0x01234567; NOAHZK_get_section_from_var(variable, UINT8_MAX, 0, uint8_t) will return a value of the same type as var holding 0x67
#define NOAHZK_get_section_from_var(var, section_mask, section, section_type) ((section) < sizeof(var)/sizeof(section_type)? var >> (section)*(sizeof(var)*BITS_IN_UINT8_T) & section_mask: 0)

#define NOAHZK_MAX(x, y) ((x) > (y)? (x): (y))
#define NOAHZK_MIN(x, y) ((x) < (y)? (x): (y))
#define NOAHZK_SWP(x, y) (x) = (x) ^ (y); (y) = (x) ^ (y); (x) = (x) ^ (y)
#define NOAHZK_SWP_PTR(x, y) (x) = (void*)((intptr_t)(x) ^ (intptr_t)(y)); (y) = (void*)((intptr_t)(x) ^ (intptr_t)(y)); (x) = (void*)((intptr_t)(x) ^ (intptr_t)(y))
#define NOAHZK_SIZE_AS_ARR_OF_TYPE(size, size_type) (((size)/size_type) + ((size)%size_type != 0))
// gets width in limbs of a variable of width x
#define NOAHZK_GET_LIMB_WIDTH_FROM_INT(x)           ((x)/sizeof(NOAHZK_limb_t) + ((x)%sizeof(NOAHZK_limb_t) != 0))
#define NOAHZK_GET_WIDTH_FROM_VAR_WIDTH_TYPE(x)     ((x). width*sizeof(NOAHZK_limb_t))
#define NOAHZK_GET_WIDTH_FROM_VAR_WIDTH_TYPE_PTR(x) ((x)->width*sizeof(NOAHZK_limb_t))
// width of said var-width type in number of limbs is passed directly
#define NOAHZK_GET_WIDTH_FROM_VAR_WIDTH_TYPE_INT(x) ((x)       *sizeof(NOAHZK_limb_t))
#define NOAHZK_GET_WIDTH_FROM_VAR_WIDTH_TYPE_PTR_BITS(x) ((x)->width*BITS_IN_NOAHZK_LIMB)

#define NOAHZK_variable_width_var_INITIALIZER {0, NULL}

typedef struct{
    uint64_t width;
    NOAHZK_limb_t* arr;
} NOAHZK_variable_width_t;

#endif
