#ifndef VERB_library_error_type_included
#define VERB_library_error_type_included

#include "stdlib.h"                                     // dynamic memory handling.
#include "../dynamic_arrays.h"                          // for dynamic array type & ops.

typedef struct{
    VERB_array_t warnings;                              // array of all warnings, null-terminated.
    VERB_array_t errors;                                // custom error statements.
} VERB_error_specifics_t;

VERB_error_specifics_t* VERB_error_specifics_init(VERB_error_specifics_t* errors){
    if(!errors) errors = malloc(sizeof(*errors));

    VERB_array_dyn_init(&errors->warnings);
    VERB_array_dyn_init(&errors->errors);

    return errors;
}

void VERB_error_specifics_destroy(VERB_error_specifics_t* const error_specifics){
    free(error_specifics);
}

#endif
