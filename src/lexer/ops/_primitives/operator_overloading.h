#ifndef VERB_bytecode_op__operator_overloading_included
#define VERB_bytecode_op__operator_overloading_included

#include "stdint.h"
#include "../../../tokens/primitives/type.h"
#include "../../../tokens/token_processing.h"
#include "../../../tokens/primitives/bytecode.h"
#include "../../tokeniser/backend/tokeniser_ctx.h"
#include "../../tokeniser/variables/_all_variables.h"

static void VERB_op__call_process(VERB_tokeniser_t*, VERB_variable_t*, VERB_variable_function_t*, VERB_bytecode_argcnt_t, VERB_variable_token_t*, VERB_variable_t**);

static size_t VERB_op__operator_overload_query_len(VERB_tokeniser_t* const restrict tokeniser, const VERB_bytecode_t op, const VERB_bytecode_argcnt_t srccnt, VERB_variable_t** const restrict src){
    size_t sumoflengths = sizeof(VERB_bytecode_t);
// VERB_BC_convert has special-case handling.
    if(op == VERB_BC_convert){
        sumoflengths += src[0]->type->siglen*sizeof(VERB_type_tok_t) + sizeof(VERB_type_tok_t);
        if(src[1]->type->group != VERB_TYPEGROUP_TYPE){
            VERB_error_report(tokeniser->specifics, VERB_error_invalid_statement, tokeniser->line, tokeniser->offset, 1, "INVALID CONVERSION TO A NON-TYPE");
            return 0;
        }
        if(!(src[1]->flags & VERB_VARIABLE_FLAG_const)){
            VERB_error_report(tokeniser->specifics, VERB_error_invalid_statement, tokeniser->line, tokeniser->offset, 1, "CONVERSION TO TYPE THAT CANNOT BE FOUND AT COMPILETIME (the type, not the conversion)");
            return 0;
        }
    }
    else for(VERB_bytecode_argcnt_t i = 0; i < srccnt; i++)
        sumoflengths += src[i]->type->siglen*sizeof(VERB_type_tok_t);

    return sumoflengths;
}

static char* const restrict VERB_op__operator_overload_query(VERB_tokeniser_t* const restrict tokeniser, const VERB_bytecode_t op, const VERB_bytecode_argcnt_t srccnt, VERB_variable_t** const restrict src, const size_t querylen){
    // string is composed of opcode + all types concatenated.
// opcode put at start so rht is smaller; similar operators with different types make the rht 1-2 entries deep, while different operators with similar types would otherwise make it unreasonably deep.
// we reuse the fn type as a query as we don't know the return argument.
    char* const restrict query = malloc(querylen);

    *(VERB_bytecode_t*)(query) = op;
    {
        size_t offset = sizeof(VERB_bytecode_t);
        if(op == VERB_BC_convert){      // VERB_BC_convert has the format '->' 'src variable type' 'dst type'
            memcpy(query + offset, src[0]->type->sig, src[0]->type->siglen*sizeof(VERB_type_tok_t));
            offset += src[0]->type->siglen*sizeof(VERB_type_tok_t);
            memcpy(query + offset, ((VERB_type_t*)src[1]->custom_data)->sig, sizeof(VERB_type_tok_t));
        }
        else for(VERB_bytecode_argcnt_t i = 0; i < srccnt; i++){
            memcpy(query + offset, src[i]->type->sig, src[i]->type->siglen*sizeof(VERB_type_tok_t));
            offset += src[i]->type->siglen*sizeof(VERB_type_tok_t);
        }
    }

    return query;
}

// operator overloads in VERB have to match EXACTLY with the type.
// for example, an overload valid for u8 exact isn't valid for u8 or u16 because the two are INCOMPATIBLE.
// I fear this leads to a lot of boilerplate, where something like:
// fn +(mytype, u64)
// has to be created for u32, u16 and u8 etc. use plain uint in that case!
static bool VERB_op__resolve_operator_overload(VERB_tokeniser_t* const restrict tokeniser, const VERB_bytecode_t op, const VERB_bytecode_argcnt_t srccnt, VERB_variable_t** const restrict src){
/*
 * BASIC OPERATOR OVERLOADING RESOLUTION ALGORITHM: 
 *      build string of all types.
 *      search for token within rht.
 *      if no such token is found, return false.
 *      else push correct CALL thing & return proper arguments.
*/
    const size_t querylen = VERB_op__operator_overload_query_len(tokeniser, op, srccnt, src);
    if(!querylen)                                   // true so it doesn't mess anything up. error value of 0 should be fine.
        return true;
    char* const restrict query = VERB_op__operator_overload_query(tokeniser, op, srccnt, src, querylen);
// searches for said string.
    VERB_variable_t* const restrict overload = VERB_tree_ptr_search(tokeniser->overloaded_operators, query, querylen);
    if(!overload) return false;
// operators in VERB may resolve to values.
    if(overload->type->group != VERB_TYPEGROUP_FUN){
        VERB_bytecode_push_type(overload->name, VERB_variable_token_t, &tokeniser->values);
        return true;
    }
    
    VERB_op__call_process(tokeniser, overload, overload->custom_data, srccnt, NULL, src);
    return true;
}

#endif
