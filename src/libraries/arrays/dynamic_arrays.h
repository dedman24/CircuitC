#ifndef VERB_library_dynamic_arrays_included
#define VERB_library_dynamic_arrays_included

#include "stdlib.h"             // dynamic memory operations
#include "stdint.h"             // types
#include "string.h"             // memcpy

// dynamic array library so that we don't have to handle that stuff ourselves

typedef struct{
    char* arr;                  // char* because it can alias with any type
    size_t size;                // bytes used
    size_t capacity;            // bytes allocated
} VERB_array_t;

#define VERB_array_empty ((VERB_array_t){NULL, 0, 0})

typedef enum{ VERB_array_keep_ctx, VERB_array_free_ctx } VERB_array_options_t;

// clears array passed to it if it already exists & it holds something. otherwise creates new one.
VERB_array_t VERB_array_init(void){
    const size_t initial_capacity = 256;

    return (VERB_array_t){malloc(initial_capacity), 0, initial_capacity};
}

VERB_array_t* VERB_array_dyn_init(VERB_array_t* array){
    const size_t initial_capacity = 256;
    
    if(!array) array = malloc(sizeof(*array));

    array->size = 0;
    array->capacity = initial_capacity;
    if(!array->arr) array->arr = calloc(1, array->capacity);
    else{
        array->arr = realloc(array->arr, initial_capacity);
        memset(array->arr, 0, initial_capacity);
    }

    return array;
}

void VERB_array_destroy(VERB_array_t* array, VERB_array_options_t freectx){
    if(!array) return;
    if(array->arr) free(array->arr);
    if(freectx == VERB_array_free_ctx) free(array);
}

size_t VERB_array_extract_length(VERB_array_t* array){
    return array->size;
}

// array is not guaranteed to still be usable after. assume it isn't.
void* VERB_array_extract(VERB_array_t* array){
    char* arr = array->arr;
    array->arr = NULL;
    array->capacity = 0;
    array->size = 0;
    return arr;
}

#define VERB_array_extract_all(array, size, data)                                   \
        size = VERB_array_extract_length(array);                                    \
        data = VERB_array_extract(array);                                           \
        VERB_array_destroy(array, VERB_array_free_ctx)

// growth factor of 1.5
void VERB_array_expand(VERB_array_t* array, size_t new_size){
    size_t new_capacity = array->capacity*3/2;                          // if one tries pushing something larger than what the new capacity would be on the array,
    if(new_size > new_capacity) new_capacity = new_size*3/2;            // we can either iteratively increase capacity until it is larger than what the new array size would be
    array->capacity = new_capacity;                                     // or we can set the new capacity to the new size times some constant. I chose the latter approach.
    
    array->arr = realloc(array->arr, array->capacity);
}

// preallocates n bytes onto array without actually pushing anything.
void VERB_array_preallocate(VERB_array_t* array, size_t toallocate){
    const size_t new_size = array->size + toallocate;
    if(new_size >= array->capacity) VERB_array_expand(array, new_size);
}

// pushes element to top of array
void VERB_array_push_char(const char character, VERB_array_t* array){
    const size_t new_size = array->size + sizeof(character);
    if(new_size >= array->capacity) VERB_array_expand(array, new_size);

    array->arr[array->size] = character;
    array->size = new_size;
}

// pushes string to top of array
void VERB_array_push_string(const void* const restrict string, const size_t length_string, VERB_array_t* const restrict array){
    const size_t new_size = array->size + length_string;
    if(new_size + length_string >= array->capacity) VERB_array_expand(array, new_size);
    
    memcpy(array->arr + array->size, string, length_string);
    array->size = new_size;
}

// macros

// gets top of stack, given type of top of stack.
#define VERB_array_top(type, a) (*(type*)((a)->arr + (a)->size - sizeof(type)))

#define VERB_array_top_safe(type, a) ((a)->size > sizeof(type)? VERB_array_top(type, a): (type)0)

#define VERB_array_remove_type(cnt, type, a)                \
    do{                                                     \
        (a)->size -= (cnt)*sizeof(type);                    \
    } while(0)                                              \

#define VERB_array_push(el, a)                              \
    do{                                                     \
        VERB_array_push_string(&(el), sizeof(el), (a));     \
    } while(0)

// W COMMA OPERATOR! 
#define VERB_array_pop_type(type, a) ((a)->size -= sizeof(type), *(type*)((a)->arr + (a)->size))

#define VERB_array_pop_offset_type(cnt, type, a) ((a)->size -= sizeof(type)*(cnt), (type*)((a)->arr + (a)->size))

#define VERB_array_top_offset(cnt, type, a) ((a)->arr + (a)->size - (cnt)*sizeof(type))

#define VERB_array_top_offset_type(cnt, type, a) ((type*)VERB_array_top_offset(cnt, type, a))

#define VERB_array_is_empty(a) ((a)->size == 0)

#endif
