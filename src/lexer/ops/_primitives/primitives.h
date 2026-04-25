#ifndef VERB_bytecode_op__primitives_included
#define VERB_bytecode_op__primitives_included

#include "stdint.h"
#include "functions.h"
#include "../types/const.h"
#include "operator_overloading.h"
#include "../../tokeniser/backend/compatibility.h"
#include "../../tokeniser/backend/tokeniser_ctx.h"
#include "../../tokeniser/variables/_all_variables.h"
#include "../../../libraries/arrays/dynamic_bitarr.h"

static VERB_type_t* VERB_op__process_compatible_largest_precision(const VERB_bytecode_argcnt_t srccnt, VERB_variable_t** const restrict src){
    if(src[0]->type->group != VERB_TYPEGROUP_ARITH) return src[0]->type;
    VERB_variable_arith_t* a0 = src[0]->custom_data;
    if(a0->type != VERB_VARIABLE_ARITH_SINT || a0->type != VERB_VARIABLE_ARITH_UINT) return src[0]->type;

    VERB_variable_t* maxPrecision = src[0];
    for(VERB_bytecode_argcnt_t i = 1; i < srccnt; i++)
        maxPrecision = VERB_variable_arith_precision_larger(maxPrecision->custom_data, src[i]->custom_data)? 
            src[i]: maxPrecision;

    return maxPrecision->type;
}

static VERB_type_t* VERB_op__process_compatible(VERB_tokeniser_t* const restrict tokeniser, const VERB_bytecode_t op, VERB_bytecode_argcnt_t srccnt, VERB_variable_t** restrict src, VERB_variable__flags_t* const restrict flags){
    switch(op){
        case VERB_BC_index:                 // \ these two ops depend on how we actually implement them.
        case VERB_BC_complex_type_field:    // / TODO: add these soon.
        case VERB_BC_if:                    // TODO: add this returning an option type!
            srccnt -= 1;
            src += 1;
            break;
        case VERB_BC_else:
            srccnt -= 1;
            src += 1;
            if(VERB_array_top(VERB_bytecode_t, &tokeniser->ops) != VERB_BC_if){
                VERB_error_report(tokeniser->specifics, VERB_error_invalid_statement, tokeniser->line, tokeniser->offset, 1, "NO 'if' STATEMENT TO 'else' STATEMENT");
                return NULL;
            }
            VERB_array_pop_type(VERB_bytecode_t, &tokeniser->ops);
            break;
        default:
            break;
    }

    *flags = src[0]->flags;
    for(VERB_bytecode_argcnt_t i = 1; i < srccnt; i++){
    // has to be written in this way; if operation is sub, then src[i] must be smaller than src[0].
    // otherwise the two can have any width; but the destination must have the largest type between the two.
        if(!VERB_type_compatible(src[i], src[0], op != VERB_BC_sub)){
            VERB_error_report(tokeniser->specifics, VERB_error_invalid_types_op, tokeniser->line, tokeniser->offset, 3, op, srccnt, src);
            return NULL;
        }
        *flags &= src[i]->flags;
    }
    return VERB_op__process_compatible_largest_precision(srccnt, src);
}

static void VERB_op__process(VERB_tokeniser_t* const restrict tokeniser, const VERB_bytecode_t op){
    const VERB_bytecode_argcnt_t srccnt = VERB_bytecode_argcnt_src(VERB_bytecode_argcnt[op]);
    const VERB_bytecode_argcnt_t dstcnt = VERB_bytecode_argcnt_dst(VERB_bytecode_argcnt[op]);
// MUST NOT BE FREED!
    VERB_variable_token_t* const restrict srcargs = VERB_array_pop_offset_type(srccnt, VERB_variable_token_t, &tokeniser->values);
// searches for all source values.
    VERB_variable_t** const restrict src = malloc(sizeof(*src)*srccnt);
    for(VERB_bytecode_argcnt_t i = 0; i < srccnt; i++)
        src[i] = VERB_variable_search_byToken(&tokeniser->backend, srcargs[i]);
// checks for operator overloading.
    if(VERB_op__resolve_operator_overload(tokeniser, op, srccnt, src)){
        free(src);
        return;
    }
// checks if variables are compatible under given op. returns ptr to type to be used.
    VERB_variable__flags_t dstflags;
    VERB_type_t* const restrict dsttype = VERB_op__process_compatible(tokeniser, op, srccnt, src, &dstflags);
    if(!dsttype) return;
// pushes dst arguments HERE SOLELY to values array/stack & not to code (done later).
    for(VERB_bytecode_argcnt_t i = 0; i < dstcnt; i++){
        VERB_variable_token_t dst = VERB_variable_definition_internal_init(&tokeniser->backend, dsttype, dstflags)->name;
        VERB_array_push_string(&dst, sizeof(dst), &tokeniser->values);
    }
// done so code is not emitted for const-typed ops.
    if(VERB_variable_flags_is_const_propagation_allowed(dstflags))
        VERB_constant_propagate(tokeniser, op, srccnt, src, dstcnt, VERB_array_top_offset_type(dstcnt, VERB_variable_token_t, &tokeniser->values));
    else{
// pushes op, src arguments & dst arguments respectively, in that order.
        VERB_bytecode_push(op, tokeniser->code);
        VERB_bytecode_push_str(srcargs, srccnt*sizeof(VERB_variable_token_t), tokeniser->code);
        VERB_bytecode_push_str(VERB_array_top_offset(dstcnt, VERB_variable_token_t, &tokeniser->values), dstcnt*sizeof(VERB_variable_token_t), tokeniser->code);
    }

    free(src);
}

static bool VERB_op__push_opcode_condition(VERB_tokeniser_t* const restrict tokeniser, const VERB_bytecode_t bc){
    VERB_bytecode_t top = VERB_array_pop_type(VERB_bytecode_t, &tokeniser->ops);
    return VERB_priority[top] < VERB_priority[bc] + (top == VERB_BC_special_OPENED_ROUND_BRACKET? VERB_PRIORITY_special_highest: 0);
}

// CALL THIS TO PUSH OPCODES!
static void VERB_op__opcode(VERB_tokeniser_t* const restrict tokeniser, const VERB_bytecode_t bc){
// most code assumes this works THE WAY IT'S IMPLEMENTED (in that it pushes an opcode while processing all previous ones, without processing the new opcode).
    if(bc == VERB_BC_special_CLOSED_ROUND_BRACKET){
    // implements correct handling of stray brackets.
        VERB_bytecode_t top = VERB_BC_special_NONE;
        while(
            (top = VERB_array_pop_type(VERB_bytecode_t, &tokeniser->values)) != VERB_BC_special_OPENED_ROUND_BRACKET
        ){
            VERB_op__process(tokeniser, top);
        }
        return;
    }

// should priority be > or >=? if it was >, then something like:
// 3 + 4 + 5 + 6.
// is processed as
// + + +
// 3 4 5 6
// once '.' comes in, it forces everything to be processed.
// both work. it works with non-commutative operators too ('-' used, really any operator is fine):
// '3 - 4 - 5 - 6' becomes '- - -', '3 4 5 6' which becomes (3 - (4 - (5 - 6))).
// I have decided to go ahead with > because the symbol looks cooler.
    VERB_bytecode_t top;
    while(
        VERB_priority[(top = VERB_array_pop_type(VERB_bytecode_t, &tokeniser->ops))] > VERB_priority[bc]
    ){
        VERB_op__process(tokeniser, top);
    }
// VERB_BC_period NEVER pushed as an actual op.
   if(bc != VERB_BC_period) VERB_array_push(bc, &tokeniser->ops);
}

static void VERB_op__token(char* restrict* const restrict string, VERB_tokeniser_t* const restrict tokeniser, const VERB_variable_token_t token){
    VERB_tokeniser_not_operator(tokeniser);
    if(VERB_variable_name_isFunction(token))
        VERB_op__call(string, tokeniser, token);
    else VERB_array_push_string(&token, sizeof(token), &tokeniser->values); 
}

#endif
