#ifndef VERB_bytecode_primitives_included
#define VERB_bytecode_primitives_included

#include "stdint.h"
#include "types/const.h"
#include "../tokeniser/backend/compatibility.h"
#include "../tokeniser/backend/tokeniser_ctx.h"
#include "../tokeniser/variables/_all_variables.h"
#include "../../libraries/arrays/dynamic_bitarr.h"
#include "../../frontend/interpreter/interpreter.h"

void VERB_lexer_line(char* restrict* const restrict, VERB_tokeniser_t* const restrict, const char* const restrict);

static void VERB_tokeniser_not_whitespace(VERB_tokeniser_t* const tokeniser){
    if(VERB_tokeniser_state_check(tokeniser, VERB_tokeniser_state_scope_after_newline_and_period)){
        VERB_tokeniser_state_remove(tokeniser, VERB_tokeniser_state_scope_after_newline_and_period);
        if(tokeniser->old_whitespace_cnt > tokeniser->new_whitespace_cnt){
            for(unsigned int i = 0; i < tokeniser->old_whitespace_cnt - tokeniser->new_whitespace_cnt; i++)
                VERB_tokeniser_backend_scope_del(&tokeniser->backend);
        }
        else{
            for(unsigned int i = 0; i < tokeniser->new_whitespace_cnt - tokeniser->old_whitespace_cnt; i++)
                VERB_tokeniser_backend_scope_new(&tokeniser->backend);
        }
        tokeniser->old_whitespace_cnt = tokeniser->new_whitespace_cnt;
        tokeniser->new_whitespace_cnt = 0;
    }
}

static void VERB_tokeniser_operator(VERB_tokeniser_t* const tokeniser){
    VERB_tokeniser_state_assert(tokeniser, VERB_tokeniser_state_after_operator);
}

static void VERB_tokeniser_not_operator(VERB_tokeniser_t* const tokeniser){
    VERB_tokeniser_state_remove(tokeniser, VERB_tokeniser_state_after_operator);
}

VERB_type_t* VERB_lexer_line_process_element_convert(VERB_tokeniser_t* const restrict tokeniser, const VERB_bytecode_t op, VERB_bytecode_argcnt_t srccnt, VERB_variable_token_t* restrict srcargs, VERB_variable_t** restrict src, VERB_variable__groups_t* const restrict group, VERB_variable__flags_t* const restrict flags){
// ops that handle types differently.
    switch(op){
    case VERB_BC_conversion:
        *flags = src[1]->flags;
        *group = src[1]->group;
        return src[1]->type;
    case VERB_BC_dereference:
    case VERB_BC_reference:
    case VERB_BC_index:
    case VERB_BC_pick:                          // ternary-if.
        src += 1;                               // removes first element from those to be processed.
        srccnt -= 1;                            // this is because the first element's type doesn't really matter.
        srcargs += 1;                           // only the types of the other two have to be considered.
        break;
    case VERB_BC_set:
    default:
        *flags &= ~VERB_VARIABLE_FLAG_lvalue;   // all other ops return rvalues/temporaries. 
        break;
    }
    
    VERB_bitarr_t conversion_mask_arr = VERB_bitarr_init();
    VERB_type_t* const ftype = VERB_variable_list_compatible(&tokeniser->backend, srccnt, src, flags, &conversion_mask_arr);
    if(!ftype){ 
        VERB_error_report(tokeniser->specifics, VERB_error_invalid_types, tokeniser->line, tokeniser->offset, 3, op, srccnt, src);
    // assigns it to first var; perhaps this is wrong. there should be a VERB_type_special_compatible_with_all type that skips over type compatibility shenanigans.
        return src[0]->type;
    }
    uint8_t* const restrict conversion_mask = VERB_bitarr_extract(&conversion_mask_arr);
    VERB_variable_list_convert(&tokeniser->backend, tokeniser->code, srccnt, srcargs, ftype, *group, *flags, conversion_mask);
    free(conversion_mask);

    return ftype;
}

static void VERB_lexer_line_process_element(VERB_tokeniser_t* const restrict tokeniser, const VERB_bytecode_t op){
    const VERB_bytecode_argcnt_t srccnt = VERB_bytecode_argcnt_src(VERB_bytecode_argcnt[op]);
    const VERB_bytecode_argcnt_t dstcnt = VERB_bytecode_argcnt_dst(VERB_bytecode_argcnt[op]);
    VERB_variable_token_t* const restrict srcargs = VERB_array_pop_offset_type(srccnt, VERB_variable_token_t, &tokeniser->values);
// special handling of comma operator; could be handled more 'traditionally' but this allows for more optimisations later on.
    if(op == VERB_BC_comma){
        VERB_array_push_string(srcargs + 1, sizeof(*srcargs), &tokeniser->values);
        return;
    }
// checks for compatibility between types & operations.
    VERB_variable_t** const restrict src = malloc(sizeof(*src)*srccnt);
    for(VERB_bytecode_argcnt_t i = 0; i < srccnt; i++)
        src[i] = VERB_variable_search_byToken(&tokeniser->backend, srcargs[i]);
// checks if variables are compatible under given op, O(nlogn) where n == number of source variables. returns ptr to type to be used.
    VERB_variable__flags_t dstflags = VERB_VARIABLE_FLAG_ALL;
    VERB_variable__groups_t dstgroup = VERB_VARIABLE_FUN;
    VERB_type_t* const restrict dsttype = VERB_lexer_line_process_element_convert(tokeniser, op, srccnt, srcargs, src, &dstgroup, &dstflags);

// pushes dst arguments HERE SOLELY to values array/stack & not to code (done later).
    for(VERB_bytecode_argcnt_t i = 0; i < dstcnt; i++){
        VERB_variable_token_t dst = VERB_variable_definition_internal_init(&tokeniser->backend, dsttype, dstgroup, dstflags, NULL)->name;
        VERB_array_push_string(&dst, sizeof(dst), &tokeniser->values);
    }
// done so code is not emitted for const-typed ops.
    if(dstflags & VERB_VARIABLE_FLAG_const)
        VERB_constant_propagate(tokeniser, op, srccnt, src, dstcnt, VERB_array_top_offset_type(dstcnt, VERB_variable_token_t, &tokeniser->values));
    else{
// pushes op, src arguments & dst arguments respectively, in that order.
        VERB_bytecode_push(op, tokeniser->code);
        VERB_bytecode_push_str(srcargs, srccnt*sizeof(VERB_variable_token_t), tokeniser->code);
        VERB_bytecode_push_str(VERB_array_top_offset(dstcnt, VERB_variable_token_t, &tokeniser->values), dstcnt*sizeof(VERB_variable_token_t), tokeniser->code);
    }

    free(src);
}

void VERB_lexer_push_opcode(VERB_tokeniser_t* const restrict tokeniser, const VERB_bytecode_t bc){
    if(bc == VERB_BC_special_CLOSED_ROUND_BRACKET){
        VERB_bytecode_t top = VERB_array_pop_type(VERB_bytecode_t, &tokeniser->values);
        if(top == VERB_BC_special_OPENED_ROUND_BRACKET)
            VERB_error_report(tokeniser->specifics, VERB_error_invalid_statement, tokeniser->line, tokeniser->offset, 1, "NO EXPRESSION INSIDE ROUND BRACKETS");
        else do{
            VERB_lexer_line_process_element(tokeniser, top);
        } while(
            (top = VERB_array_pop_type(VERB_bytecode_t, &tokeniser->values)) != VERB_BC_special_OPENED_ROUND_BRACKET
        );
        return;
    }

    VERB_bytecode_t top = VERB_array_pop_type(VERB_bytecode_t, &tokeniser->ops);
    
    if(VERB_priority[top] < VERB_priority[bc]) VERB_array_push_string(&bc, sizeof(bc), &tokeniser->ops);
    else do{
        VERB_lexer_line_process_element(tokeniser, top);
    } while(
        VERB_priority[(top = VERB_array_pop_type(VERB_bytecode_t, &tokeniser->ops))] < 
        VERB_priority[bc] + (tokeniser->bracket_depth? VERB_PRIORITY_special_highest: 0)
    );
}

bool VERB_lexer_call_convert(VERB_tokeniser_t* const restrict tokeniser, VERB_variable_function_t* const restrict fun, const size_t srccnt, VERB_variable_token_t* const restrict srcargs, VERB_variable_t** const restrict defs){
    for(size_t i = 0; i < srccnt; i++){
        VERB_variable_t* src = VERB_variable_search_byToken(&tokeniser->backend, srcargs[i]);
        VERB_variable_t* dst = fun->src[i];
// if the two types are not compatible, converts src to dst's type.
        if(!VERB_variable_type_implicit_conversion_allowed(&tokeniser->backend, src, dst)){
            VERB_error_report(tokeniser->specifics, VERB_error_invalid_types, tokeniser->line, tokeniser->offset, 5, VERB_BC_call, i, &tokeniser->backend, dst->type, src->type);
            return false;
        }
        if(!VERB_variable_compatible(src->type, dst->type)) {
// TODO: add checking that srcargs, dstargs have both or neither const qualifier.
            src = VERB_variable_convert(&tokeniser->backend, tokeniser->code, srcargs[i], dst->type, dst->group, VERB_variable_definition_flags_combine(src->flags, dst->flags));
            srcargs[i] = src->name;
        }
// only passed if function is const.
        if(defs){
            defs[i] = src;
            if((defs[i]->flags & VERB_VARIABLE_FLAG_const) == 0){
                VERB_error_report(tokeniser->specifics, VERB_error_invalid_statement, tokeniser->line, tokeniser->offset, 1, "FUNCTION CALL TYPED 'const' GIVEN ARGUMENTS NOT DETERMINABLE AT COMPILE-TIME");
                return false;
            } 
        }
    }
    return true;
}

void VERB_lexer_call_handle(char* restrict* const restrict string, VERB_tokeniser_t* const restrict tokeniser, const VERB_variable_token_t function_name, VERB_array_t* const restrict values){    
    VERB_variable_t* const restrict defOverloaded = VERB_variable_search_byToken(&tokeniser->backend, function_name);
// function does not exist/token is not a function.
    if(!defOverloaded || defOverloaded->group != VERB_VARIABLE_FUN){
        VERB_error_report(tokeniser->specifics, VERB_error_invalid_statement, tokeniser->line, tokeniser->offset, 1, "UNDEFINED FUNCTION CALLED");
        return;
    }
// parses function arguments.
    VERB_array_t functionArgs = VERB_array_init();
    {
        const unsigned int starting_depth = tokeniser->bracket_depth;
        while(tokeniser->bracket_depth >= starting_depth){
            VERB_lexer_line(string, tokeniser, ",.");
            VERB_lexer_push_opcode(tokeniser, VERB_BC_special_ARGLESS_LOW);

            const VERB_variable_token_t argtok = VERB_array_pop_type(VERB_variable_token_t, &tokeniser->values);
            VERB_array_push(argtok, &functionArgs);
            VERB_array_pop_type(VERB_bytecode_t, &tokeniser->ops);
        } 
    }
    VERB_array_extract_all(&functionArgs, const size_t srccnt, VERB_variable_token_t* const restrict srcargs);
// resolves function overloads.
    VERB_variable_t* const restrict def = VERB_variable_function_resolve_overloads(&tokeniser->backend, defOverloaded, srccnt);
    if(!def){
        VERB_error_report(tokeniser->specifics, VERB_error_invalid_statement, tokeniser->line, tokeniser->offset, 1, "FUNCTION HAS AN INVALID NUMBER OF ARGUMENTS");
        free(srcargs);
        return;
    }
    if(def->group != VERB_VARIABLE_FUN){
        VERB_error_report(tokeniser->specifics, VERB_error_invalid_statement, tokeniser->line, tokeniser->offset, 1, "FUNCTION HAS AN INVALID NUMBER OF ARGUMENTS");
        free(srcargs);
        return;
    }

    VERB_variable_t** const restrict src = def->flags & VERB_VARIABLE_FLAG_const? malloc(sizeof(*src)*srccnt): NULL;
// checks the function arguments' types.
    VERB_variable_function_t* const restrict fun = def->custom_data;
    if(!VERB_lexer_call_convert(tokeniser, fun, srccnt, srcargs, src)){
        free(srcargs);
        return;
    }
    // handles const-typed functions.
    if(def->flags & VERB_VARIABLE_FLAG_const){
        VERB_array_preallocate(&tokeniser->values, fun->dstcnt*sizeof(VERB_variable_token_t));
        VERB_interpreter_execute_const_function(fun, (VERB_variable_token_t*)tokeniser->values.arr, fun->dstcnt, src, srccnt, tokeniser->backend.all_token_defs);
        tokeniser->values.size += fun->dstcnt*sizeof(VERB_variable_token_t);
        return;
    }
// -- ASSEMBLING OPCODE -- .
// puts basic opcode.
    VERB_bytecode_push(VERB_BC_call, tokeniser->code);
    VERB_bytecode_push_type(def->name, VERB_variable_token_t, tokeniser->code);
    VERB_bytecode_push_type(srccnt, size_t, tokeniser->code);
    VERB_bytecode_push_str(srcargs, srccnt*sizeof(*srcargs), tokeniser->code);
// cleanup.
    free(srcargs);
// pushes dst function arguments to bytecode array.
    const size_t dstcnt = VERB_variable_function_get_dstcnt(def);
    VERB_bytecode_push_type(dstcnt, size_t, tokeniser->code);

    for(size_t i = 0; i < dstcnt; i++){
// TODO: add proper custom data.
        VERB_variable_t* const restrict dst = fun->dst[i];
        VERB_variable_token_t dstname = VERB_variable_definition_internal_init(&tokeniser->backend, dst->type, dst->group, dst->flags, NULL)->name;
        VERB_bytecode_push_str(&dstname, sizeof(dstname), tokeniser->code);
        VERB_array_push_string(&dstname, sizeof(dstname), values);
    }
}

void VERB_lexer_push_variable(char* restrict* const restrict string, VERB_tokeniser_t* const restrict tokeniser, const VERB_variable_token_t variable){
    VERB_tokeniser_not_operator(tokeniser);
    if(VERB_variable_name_isFunction(variable))
        VERB_lexer_call_handle(string, tokeniser, VERB_array_pop_type(VERB_variable_token_t, &tokeniser->values), &tokeniser->values);
    else VERB_array_push_string(&variable, sizeof(variable), &tokeniser->values); 
}

#endif
