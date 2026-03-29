#ifndef VERB_token_processing_included
#define VERB_token_processing_included

#include "stddef.h"                                 // size_t & similar
#include "stdint.h"                                 // integer types

#include "token_type.h"                             // VERB_bytecode_t type
#include "../../libraries/dynamic_arrays.h"         // VERB_array_push_string

void VERB_bytecode_push(VERB_bytecode_t bc, VERB_array_t* const restrict array){
    VERB_array_push_string(&bc, sizeof(bc), array);
}

void VERB_bytecode_push_str(void* restrict str, const size_t len, VERB_array_t* const restrict array){
    VERB_array_push_string(str, len, array);
}

// I LOVE MACROS!
#define VERB_bytecode_push_type(var, type, array)                               \
    do{                                                                         \
        type temp = (var);                                                      \
        VERB_array_push_string(&temp, sizeof(temp), (array));                   \
    } while(0)

#endif
