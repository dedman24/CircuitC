#ifndef VERB_tokeniser_variable_definition_included
#define VERB_tokeniser_variable_definition_included

#include "stddef.h"
#include "stdint.h"
#include "../backend/backend.h"

typedef enum{
    VERB_VARIABLE_FLAG_none                 = 0,
    VERB_VARIABLE_FLAG_const                = 1,            // value must be computed at compile-time.
    VERB_VARIABLE_FLAG_mut                  = 2,            // value cannot be computed at compile-time (but program stays pure).
    VERB_VARIABLE_FLAG_priv                 = 4,            // variable is priv.
} VERB_variable__flags_t;

#define VERB_variable_flags_is_const_propagation_allowed(x) ((x) & VERB_VARIABLE_FLAG_const)

#define VERB_VARIABLE_FLAG_ALL (VERB_VARIABLE_FLAG_const | VERB_VARIABLE_FLAG_mut)

#define VERB_variable_flags_combine(x, y) ((x) & (y))

typedef struct VERB_variable{
    void* custom_data;                                      // custom data for different values.
    VERB_type_t* type;                                      // type of variable.
    struct VERB_variable* type_ctx;                         // context of type.
    struct VERB_variable* father;                           // complex type it is part of (NULL if none).
    char* namestr;                                          // name in string format.
    size_t namestrlen;                                      // length of namestr
    VERB_variable_token_t name;                             // name (token) of this variable.
    VERB_variable__flags_t flags;
} VERB_variable_t;

#endif
