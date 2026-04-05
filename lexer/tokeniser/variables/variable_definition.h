#ifndef VERB_tokeniser_variable_definition_included
#define VERB_tokeniser_variable_definition_included

#include "stddef.h"
#include "stdint.h"
#include "../backend/backend.h"

typedef enum{
    VERB_VARIABLE_TYPE,
    VERB_VARIABLE_ARITH,
    VERB_VARIABLE_PTR,
    VERB_VARIABLE_FUN,
} VERB_variable__groups_t;                        // major groups of types a variable can assume. 

typedef enum{
    VERB_VARIABLE_FLAG_none                 = 0,
    VERB_VARIABLE_FLAG_const                = 1,
    VERB_VARIABLE_FLAG_lvalue               = 2,            // variable has an address/is referenceable.
} VERB_variable__flags_t;

#define VERB_VARIABLE_FLAG_ALL (VERB_VARIABLE_FLAG_const | VERB_VARIABLE_FLAG_lvalue)

#define VERB_variable_definition_flags_combine(x, y) ((x) & (y))

typedef struct{
    void* custom_data;                                      // custom data for different values.
    void (*destructor)(void*);                              // destructor for custom data.
    VERB_type_t* type;                                      // type of variable.
    VERB_variable_token_t type_name;                        // name (token) of type.
    VERB_variable_token_t name;                             // name (token) of variable.
    VERB_variable__groups_t group;
    VERB_variable__flags_t flags;
} VERB_variable_t;

#endif
