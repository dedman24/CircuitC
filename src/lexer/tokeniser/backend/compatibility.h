#ifndef VERB_tokeniser_compatibility_included
#define VERB_tokeniser_compatibility_included

#include "stddef.h"                                     // size_t
#include "backend.h"                                    // VERB_tokeniser_backend_t.
#include "stdbool.h"                                    // boolean type
#include "variable_ops.h"
#include "../variables/types.h"
#include "../../../tokens/token_type.h"                 // VERB_bytecode_push.
#include "../../../tokens/token_processing.h"
#include "../variables/variable_definition.h"
#include "../../../libraries/arrays/dynamic_arrays.h"   // VERB_array_t & ops.
#include "../../../libraries/arrays/dynamic_bitarr.h"   // VERB_bitarr_t & ops.

// converts src to dst's type, returns new converted var.
static VERB_variable_t* VERB_variable_convert(VERB_tokeniser_backend_t* const restrict backend, VERB_array_t* const restrict code, const VERB_variable_token_t srcname, VERB_type_t* const restrict dsttype, const VERB_variable__groups_t dstgroup, const VERB_variable__flags_t dstflags){
    VERB_variable_t* var = VERB_variable_definition_internal_init(backend, dsttype, dstgroup, dstflags, NULL);
    VERB_bytecode_push(VERB_BC_conversion, code);
    VERB_bytecode_push_str((void*)&srcname,  sizeof(srcname),  code);
    VERB_bytecode_push_str(&var->name, sizeof(var->name), code);
    VERB_bytecode_push_str(&dsttype->len_fully_qualified, sizeof(dsttype->len_fully_qualified), code);
    VERB_bytecode_push_str(dsttype->type, dsttype->len_fully_qualified, code);
    return var;
}

// converts all operators it has to convert to dst type (marked by mask).
void VERB_variable_list_convert(VERB_tokeniser_backend_t* const restrict backend, VERB_array_t* const restrict code, const VERB_bytecode_argcnt_t srccnt, VERB_variable_token_t* const names, VERB_type_t* const dsttype, const VERB_variable__groups_t dstgroup, const VERB_variable__flags_t dstflags, uint8_t* const restrict mask){
// converts all variable from a type to another, explicitly; converts implicit conversions to explicit ones.
    for(VERB_bytecode_argcnt_t i = 0; i < srccnt; i++){
    // checks that respective bit is set.
        if(mask[i/8] & 1 << i%8)
            names[i] = VERB_variable_convert(backend, code, names[i], dsttype, dstgroup, dstflags)->name;
    }
}

bool VERB_variable_compatible(VERB_type_t* const restrict t0, VERB_type_t* const restrict t1){
    if(t0 != t1 || t0->len_fully_qualified != t1->len_fully_qualified || memcmp(t0->type, t1->type, t0->len_fully_qualified))
        return false;
    return true;
}

// in O(nlogn), checks if srcargs are compatible by checking all of one with all of the others, then repeat with the next & with the next & so on.
// returns ptr to variable whose type has to be used, NULL in case of errors.
VERB_type_t* VERB_variable_list_compatible(VERB_tokeniser_backend_t* const restrict backend, const VERB_bytecode_argcnt_t srccnt, VERB_variable_t** const restrict src, VERB_variable__flags_t* const restrict flags, VERB_bitarr_t* const restrict mask){
    VERB_variable_t* restrict finalvar = src[0];

    for(VERB_bytecode_argcnt_t i = 1; i < srccnt; i++){
        finalvar = VERB_variable_type_implicit_conversion_allowed(backend, finalvar, src[i]);
        if(!finalvar) return NULL;
    }

    VERB_type_t* const restrict finaltype = finalvar->type;
    for(VERB_bytecode_argcnt_t i = 0; i < srccnt; i++){
        *flags &= src[i]->flags;
    // marks that variable requires conversion.
        if(VERB_variable_compatible(src[i]->type, finaltype))
            VERB_bitarr_push_1(mask);
        else 
            VERB_bitarr_push_0(mask);
    }

    return finaltype;
}

#endif
