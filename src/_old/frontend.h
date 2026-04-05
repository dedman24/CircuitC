#ifndef VERB_frontend_included
#define VERB_frontend_included

#include "stddef.h"                                             // for size_t.
#include "frontend_ctx/ctx.h"                                   // for ctx.
#include "../libraries/dynamic_arrays.h"                        // for dynamic array type.
#include "../libraries/errors_warnings/errors_warnings.h"       // for errors and warnings.

// the frontend's job is to take VERB tokens and to translate them into a standard list of dataflow instructions the backend can work with.
// the backend, in turn

void* VERB_frontend(char* const restrict tokenised, const size_t len, size_t* const bytecode_len, VERB_error_specifics_t* const restrict specifics){
    VERB_frontend_t frontend; VERB_frontend_init(&frontend, specifics);

    *bytecode_len = VERB_array_extract_length(&frontend.bytecode);
    char* const bytecode = VERB_array_extract(&frontend.bytecode);
    VERB_frontend_destroy(&frontend, VERB_frontend_keep_ctx);
    return bytecode;
}

#endif
