#ifndef VERB_tokeniser_compatibility_included
#define VERB_tokeniser_compatibility_included

#include "stddef.h"                                     // size_t
#include "backend.h"                                    // VERB_tokeniser_backend_t.
#include "stdbool.h"                                    // boolean type
#include "variable_ops.h"
#include "../variables/types.h"
#include "../../tokens/token_type.h"                    // VERB_bytecode_push.
#include "../../tokens/token_processing.h"
#include "../variables/variable_definition.h"
#include "../../../libraries/dynamic_arrays.h"

// converts src to dst's type, returns new name for src.
VERB_variable_token_t VERB_variable_convert(VERB_tokeniser_backend_t* const restrict backend, VERB_array_t* const restrict code, const VERB_variable_token_t src, void* newcustom, const VERB_variable_token_t dst, VERB_type_t* const restrict dsttype){
    VERB_variable_token_t tok = VERB_variable_definition_internal_init(backend, dsttype, newcustom);
// 
    VERB_bytecode_push(VERB_BC_conversion, code);
    VERB_bytecode_push_str((void*)&src, sizeof(src), code);
    VERB_bytecode_push_str((void*)&dst, sizeof(dst), code);
    VERB_bytecode_push_str(&tok, sizeof(tok), code);

    
    return tok;
}

void VERB_variable_list_convert(VERB_tokeniser_backend_t* const restrict backend, VERB_array_t* const restrict code, const VERB_bytecode_argcnt_t srccnt, VERB_variable_token_t* const names, VERB_variable_definition_t* const dst, const uint64_t mask){
// converts all variable from a type to another, explicitly; converts implicit conversions to explicit ones.
    for(VERB_bytecode_argcnt_t i = 0; i < srccnt; i++){
        if(!(mask & 1<<i)) continue;
        names[i] = VERB_variable_convert(backend, code, names[i], dst->custom_data, dst->name, dst->type);
    }
}

bool VERB_variable_compatible(VERB_type_t* const restrict t0, VERB_type_t* const restrict t1){
    if(t0 != t1 || t0->len_fully_qualified != t1->len_fully_qualified || memcmp(t0->type, t1->type, t0->len_fully_qualified))
        return false;
    return true;
}

// in O(nlogn), checks if srcargs are compatible by checking all of one with all of the others, then repeat with the next & with the next & so on.
// returns ptr to type to be used, NULL in case of errors.
VERB_variable_definition_t* VERB_variable_list_compatible(VERB_tokeniser_backend_t* const restrict backend, const VERB_bytecode_argcnt_t srccnt, VERB_variable_definition_t** const restrict src, uint64_t* const restrict mask){
    VERB_variable_definition_t* restrict finalvar = src[0];

    for(VERB_bytecode_argcnt_t i = 1; i < srccnt; i++){
        finalvar = VERB_variable_type_implicit_conversion_allowed(backend, finalvar, src[i]);
        if(!finalvar) return NULL;
    }
        

    VERB_type_t* const restrict finaltype = finalvar->type;
    for(VERB_bytecode_argcnt_t i = 0; i < srccnt; i++){
    // marks that variable requires conversion.
        if(VERB_variable_compatible(src[i]->type, finaltype))
            *mask |= 1<<i;
    }

    return finalvar;
}

#endif
