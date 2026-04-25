#ifndef VERB_tokeniser_compatibility_included
#define VERB_tokeniser_compatibility_included

#include "stddef.h"                                     // size_t
#include "backend.h"                                    // VERB_tokeniser_backend_t.
#include "stdbool.h"                                    // boolean type
#include "variable_ops.h"
#include "../../../tokens/token_type.h"                 // VERB_bytecode_push.
#include "../variables/_all_variables.h"
#include "../../../tokens/token_processing.h"
#include "../../../libraries/arrays/dynamic_arrays.h"   // VERB_array_t & ops.
#include "../../../libraries/arrays/dynamic_bitarr.h"   // VERB_bitarr_t & ops.

bool VERB_type_compatible_ints(VERB_variable_t* const v0, VERB_variable_t* const v1, const bool intsCanBeAnySize){
    if(v0->type->group != VERB_TYPEGROUP_ARITH || v1->type->group != VERB_TYPEGROUP_ARITH) return false;

    VERB_variable_arith_t *const a0 = v0->custom_data, *const a1 = v1->custom_data;
    if(a0->type != VERB_VARIABLE_ARITH_SINT && a0->type != VERB_VARIABLE_ARITH_UINT) return false;
    if(a0->type != a1->type) return false;
    if(!intsCanBeAnySize && VERB_variable_arith_integer_get_precision(a0) > VERB_variable_arith_integer_get_precision(a1)) return false;
    return true;
}

// intsCanBeAnySize ~ if true, then v0, v1 as ints can be any size; otherwise, v1 must be larger than or equal to v0 in precision.
bool VERB_type_compatible(VERB_variable_t* const v0, VERB_variable_t* const v1, const bool intsCanBeAnySize){
    if(v0 == v1) return true;
    VERB_type_t *const t0 = v0->type, *const t1 = v1->type;

    if(VERB_type_compatible_ints(v0, v1, intsCanBeAnySize)) return true;
    if(t0 != t1 || t0->siglen != t1->siglen || memcmp(t0->sig, t1->sig, t0->siglen*sizeof(*t0->sig)))
        return false;
    return true;
}

#endif
