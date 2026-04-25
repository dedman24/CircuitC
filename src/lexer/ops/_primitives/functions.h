#ifndef VERB_bytecode_op__functions_included
#define VERB_bytecode_op__functions_included

#include "../_includes.h"
#include "../../../frontend/interpreter/interpreter.h"

static void VERB_op__opcode(VERB_tokeniser_t*, const VERB_bytecode_t);

static void VERB_op__call_process(VERB_tokeniser_t* const restrict tokeniser, VERB_variable_t* const restrict def, VERB_variable_function_t* const restrict fun, const VERB_bytecode_argcnt_t srccnt, VERB_variable_token_t* const restrict srcargs, VERB_variable_t** const restrict src){
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
    if(srcargs) VERB_bytecode_push_str(srcargs, srccnt*sizeof(*srcargs), tokeniser->code);
    else{
        for(VERB_bytecode_argcnt_t i = 0; i < srccnt; i++)
            VERB_bytecode_push_type(src[i]->name, VERB_variable_token_t, tokeniser->code);
    }
// pushes dst function argument to bytecode array.
// multiple return arguments are implicitly converted to a complex type.
    const size_t dstcnt = VERB_variable_function_get_dstcnt(def);
    VERB_array_t finaltype = VERB_array_init();
    VERB_array_push_type(VERB_type_tok_t, VERB_TYPE_type, &finaltype);
    for(size_t i = 0; i < dstcnt; i++){
        VERB_variable_t* const restrict dst = fun->dst[i];
        VERB_array_push_string(dst->type->type, dst->type->typelen*sizeof(*dst->type->type), &finaltype);
    }
    VERB_array_extract_all(&finaltype, const size_t typelen, VERB_type_tok_t* const restrict type, false);
    VERB_type_signature_build(type, typelen, sig, siglen);                                          // TODO: sig, siglen LEFT UNUSED!
    VERB_variable_definition_internal_init(&tokeniser->backend, type, typelen);
}

static bool VERB_op__call_compatible(VERB_tokeniser_t* const restrict tokeniser, VERB_variable_function_t* const restrict fun, const size_t srccnt, VERB_variable_token_t* const restrict srcargs, VERB_variable_t** const restrict defs){
    for(size_t i = 0; i < srccnt; i++){
        VERB_variable_t* src = VERB_variable_search_byToken(&tokeniser->backend, srcargs[i]);
        VERB_variable_t* dst = fun->src[i];
// if dst is const and src isn't, errors out.
        if(dst->flags & VERB_VARIABLE_FLAG_const && !(src->flags & VERB_VARIABLE_FLAG_const)){
            VERB_error_report(tokeniser->specifics, VERB_error_invalid_statement, tokeniser->line, tokeniser->offset, 1, "'const' FUNCTION IS GIVEN ARGUMENTS WHOSE VALUES ARE NOT DETERMINABLE AT COMPILE-TIME");
            return false;
        }
        if(!VERB_type_compatible(src, dst, false)){
            VERB_error_report(tokeniser->specifics, VERB_error_invalid_types_call, tokeniser->line, tokeniser->offset, 4, i, tokeniser->backend, dst->type, src->type);
            return false;
        }
// only passed if function is const.
// if function is const, and not all of its arguments are, error out.
        if(defs) defs[i] = src;
    }
    return true;
}

static void VERB_op__call(char* restrict* const restrict string, VERB_tokeniser_t* const restrict tokeniser, const VERB_variable_token_t function_name){    
    VERB_variable_t* const restrict defOverloaded = VERB_variable_search_byToken(&tokeniser->backend, function_name);
// function does not exist/token is not a function.
    if(!defOverloaded || defOverloaded->type->group != VERB_TYPEGROUP_FUN){
        VERB_error_report(tokeniser->specifics, VERB_error_invalid_statement, tokeniser->line, tokeniser->offset, 1, "UNDEFINED FUNCTION CALLED");
        return;
    }
// parses function arguments.
    VERB_array_t functionArgs = VERB_array_init();
    {
        const unsigned int starting_depth = tokeniser->bracket_depth;
        while(tokeniser->bracket_depth >= starting_depth){
            VERB_lexer(string, tokeniser, ",.");
            VERB_op__opcode(tokeniser, VERB_BC_special_ARGLESS_LOW);

            const VERB_variable_token_t argtok = VERB_array_pop_type(VERB_variable_token_t, &tokeniser->values);
            VERB_array_push(argtok, &functionArgs);
            VERB_array_pop_type(VERB_bytecode_t, &tokeniser->ops);
        } 
    }
    VERB_array_extract_all(&functionArgs, const size_t srccnt, VERB_variable_token_t* const restrict srcargs, false);
// resolves function overloads.
    VERB_variable_t* const restrict def = VERB_variable_function_resolve_overloads(&tokeniser->backend, defOverloaded, srccnt);
    if(!def){
        VERB_error_report(tokeniser->specifics, VERB_error_invalid_statement, tokeniser->line, tokeniser->offset, 1, "FUNCTION HAS AN INVALID NUMBER OF ARGUMENTS");
        free(srcargs);
        return;
    }
    if(def->type->group != VERB_TYPEGROUP_FUN){
        VERB_error_report(tokeniser->specifics, VERB_error_invalid_statement, tokeniser->line, tokeniser->offset, 1, "FUNCTION HAS AN INVALID NUMBER OF ARGUMENTS");
        free(srcargs);
        return;
    }

    VERB_variable_t** const restrict src = def->flags & VERB_VARIABLE_FLAG_const? malloc(sizeof(*src)*srccnt): NULL;
// checks the function arguments' types.
    VERB_variable_function_t* const restrict fun = def->custom_data;
    if(!VERB_op__call_compatible(tokeniser, fun, srccnt, srcargs, src)){
        free(srcargs);
        if(src) free(src);
        return;
    }
    VERB_op__call_process(tokeniser, def, fun, srccnt, srcargs, src);
    free(srcargs);
    free(src);
}

#endif
