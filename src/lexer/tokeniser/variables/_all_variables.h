#ifndef VERB_tokeniser_variables__all_variables_included
#define VERB_tokeniser_variables__all_variables_included

#include "null.h"
#include "arith.h"
#include "arrays.h"
#include "functions.h"
#include "complextypes.h"
#include "../backend/backend.h"
#include "variable_definition.h"

#include "stdbool.h"
#include "../../../libraries/errors_warnings/errors_warnings.h"

// checks if the provided variable is truthy or not.
static bool VERB_variable_truthy(VERB_error_specifics_t* const restrict specifics, const unsigned long long line, const unsigned long long offset, VERB_variable_t* const restrict variable){
    if(variable->type->group == VERB_TYPEGROUP_ARITH) return VERB_variable_arith_truthy(variable->custom_data);
    if(variable->type->group == VERB_TYPEGROUP_ARR) return VERB_variable_arr_truthy(variable->custom_data);
    
    VERB_error_report(specifics, VERB_error_invalid_statement, line, offset, 1, "OBJECT THAT CANNOT BE EVALUATED AS TRUE OR FALSE IS BEING EVALUATED AS SUCH");
    return false;
}

#endif
