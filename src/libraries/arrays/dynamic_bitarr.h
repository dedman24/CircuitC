#ifndef VERB_library_dynamic_bitarr_included
#define VERB_library_dynamic_bitarr_included

#include "stddef.h"             // size_t.
#include "stdlib.h"             // dynamic memory operations.
#include "stdint.h"             // types.
#include "string.h"             // memcpy.
#include "stdbool.h"            // boolean types.

// dynamic array library so that we don't have to handle that stuff ourselves

typedef struct{
    uint8_t* arr;
    size_t size;                // bit position
    size_t capacity;            // bytes allocated
} VERB_bitarr_t;

// clears array passed to it if it already exists & it holds something. otherwise creates new one.
VERB_bitarr_t VERB_bitarr_init(void){
    const size_t initial_capacity = 16;

    return (VERB_bitarr_t){calloc(1, initial_capacity), 0, initial_capacity};
}

void VERB_bitarr_destroy(VERB_bitarr_t* array){
    if(!array) return;
    if(array->arr) free(array->arr);
}

size_t VERB_bitarr_extract_length(VERB_bitarr_t* array){
    return array->size;
}

// array is not guaranteed to still be usable after. assume it isn't.
uint8_t* VERB_bitarr_extract(VERB_bitarr_t* array){
    uint8_t* arr = array->arr;
    array->arr = NULL;
    array->capacity = 0;
    array->size = 0;
    return arr;
}

#define VERB_bitarr_extract_all(array, size, data)                                   \
        size = VERB_bitarr_extract_length(array);                                    \
        data = VERB_bitarr_extract(array);                                           \
        VERB_bitarr_destroy(array, VERB_bitarr_free_ctx)

// growth factor of 1.5
void VERB_bitarr_expand(VERB_bitarr_t* array, size_t new_size){
    const size_t new_capacity = array->capacity*2;
    array->arr = realloc(array->arr, array->capacity);
// zeroes out new data.
// MICROPTIMISATION:
// since we know the uninitialised data measures as much as the old capacity does, we can just use that.
// MUST be initialised to 0 or bad stuff happens.
    memset(array->arr + array->capacity, 0, array->capacity);
    array->capacity = new_capacity;
}

// pushes element to top of array
void VERB_bitarr_push_1(VERB_bitarr_t* array){
    const size_t new_size = array->size + 1;
    if(new_size/8 >= array->capacity) VERB_bitarr_expand(array, new_size);

    array->arr[array->size/8] |= 1 << array->size%8;
    array->size = new_size;
}

void VERB_bitarr_push_0(VERB_bitarr_t* array){
    const size_t new_size = array->size + 1;
    if(new_size/8 >= array->capacity) VERB_bitarr_expand(array, new_size);
    array->size = new_size;
}

#endif
