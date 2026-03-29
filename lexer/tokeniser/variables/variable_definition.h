#ifndef VERB_tokeniser_variable_definition_included
#define VERB_tokeniser_variable_definition_included

#include "stddef.h"
#include "stdint.h"
#include "../backend/backend.h"

typedef enum{
    VERB_VARIABLE_DEFINITION_TYPE,
    VERB_VARIABLE_DEFINITION_ARITH,
    VERB_VARIABLE_DEFINITION_PTR,
    VERB_VARIABLE_DEFINITION_FUN,
} VERB_variable_definition_groups_t;                        // major groups of types a variable can assume. 

// flags handled by compiler, moreso compiler hints/statements than data about the value itself.
typedef enum{
    VERB_VARIABLE_DEFINITION_FLAG_none                  = 0,
    VERB_VARIABLE_DEFINITION_FLAG_mut                   = 1,
    VERB_VARIABLE_DEFINITION_FLAG_mut_priv              = 2,
    VERB_VARIABLE_DEFINITION_FLAG_mut_pub               = 3,
    VERB_VARIABLE_DEFINITION_FLAG_inaccessible          = 4,
    VERB_VARIABLE_DEFINITION_FLAG_priv                  = 8,
    VERB_VARIABLE_DEFINITION_FLAG_volatile              = 16,
    VERB_VARIABLE_DEFINITION_FLAG_static                = 32,
    VERB_VARIABLE_DEFINITION_FLAG_strict                = 64
} VERB_variable_definition_flags_t;

typedef struct{
    void* custom_data;                                      // custom data for different values.
    void (*destructor)(void*);                              // destructor for custom data.
    VERB_type_t* type;                                      // type of variable.
    VERB_variable_token_t type_name;                        // name (token) of type.
    VERB_variable_token_t name;                             // name (token) of variable.
    VERB_variable_definition_groups_t group;
    VERB_variable_definition_flags_t flags;                 // data about operations valid on type.
} VERB_variable_definition_t;

#endif
