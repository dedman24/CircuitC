#ifndef CIRUCITC_dynamic_arrays_included
#define CIRCUITC_dynamic_arrays_included

#include "stdlib.h"             // dynamic memory operations
#include "stdint.h"             // types
#include "string.h"             // memcpy

// dynamic array library so that we don't have to handle that stuff ourselves

typedef struct{
    char* arr;
    size_t size;                // bytes used
    size_t capacity;            // bytes allocated
} CIRCUITC_array_t;

typedef enum{ CIRCUITC_array_keep_ctx, CIRCUITC_array_free_ctx } CIRCUITC_array_options_t;

CIRCUITC_array_t* CIRCUITC_array_init(CIRCUITC_array_t* array){
    const size_t initial_capacity = 256;
    
    if(!array) array = malloc(sizeof(*array));

    array->size = 0;
    array->capacity = initial_capacity;
    array->arr = calloc(1, array->capacity);

    return array;
}

void CIRCUITC_array_destroy(CIRCUITC_array_t* array, CIRCUITC_array_options_t freectx){
    if(array->arr) free(array->arr);
    if(freectx == CIRCUITC_array_free_ctx) free(array);
}


void* CIRCUITC_array_extract_array(CIRCUITC_array_t* array){
    char* arr = array->arr;
    array->arr = NULL;
    return arr;
}

// growth factor of 1.5
void CIRCUITC_array_expand(CIRCUITC_array_t* array, size_t new_size){
    size_t new_capacity = array->capacity*3/2;                          // if one tries pushing something larger than what the new capacity would be on the array,
    if(new_size > new_capacity) new_capacity = new_size*3/2;            // we can either iteratively increase capacity until it is larger than what the new array size would be
    array->capacity = new_capacity;                                     // or we can set the new capacity to the new size times some constant. I chose the latter approach.
    
    array->arr = realloc(array->arr, array->capacity);
}

// pushes element to top of array
void CIRCUITC_array_push(CIRCUITC_array_t* array, const char character){
    const size_t new_size = array->size + sizeof(character);
    if(new_size >= array->capacity) CIRCUITC_array_expand(array, new_size);

    array->arr[array->size] = character;
    array->size = new_size;
}

// pushes string to top of array
void CIRCUITC_array_push_string(CIRCUITC_array_t* array, void* string, const size_t length_string){
    const size_t new_size = array->size + length_string;
    if(array->size + length_string >= array->capacity) CIRCUITC_array_expand(array, array->size + length_string);
    
    memcpy(array->arr + array->size, string, length_string);
    array->size = new_size;
}

#endif
