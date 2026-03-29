#ifndef VERB_lexer_included
#define VERB_lexer_included

#include "../libraries/errors_warnings/errors_warnings.h"       // error handling
#include "../libraries/value_conversion.h"                      // value conversion operations
#include "../libraries/dynamic_arrays.h"                        // dynamic array type & operations
#include "tokeniser/backend/backend.h"
#include "tokeniser/backend/compatibility.h"
#include "tokeniser/variables/variable_definition.h"
#include "tokeniser/backend/variable_ops.h"
#include "tokeniser/variables/functions.h"
#include "tokeniser/tokeniser.h"                                // getting token from a string
#include "tokens/primitives/bytecode.h"
#include "tokens/token_processing.h"
#include "tokens/token_type.h"

// a lexer converts human-readable VERB code into a computer-readable representation of said code, encoded in TOKENS.
// example VERB code:
// fn[u8 foo, u8 bar ~ u8] foo
//      ret bar & baz
// the lexer would conver this into (where <x> represents the token x)
// <fn> <[> <u8> <name> 3 foo <,> <u8> <name> 3 bar <~> <u8> <]> <name> 3 foo
//      <ret> <name> 3 bar <&> <name> 3 baz
//
// tokens and the piece of syntax they define are held in tokens.h
// "preprocessor directives" are handled by the lexer.
// 
// the lexer returns a specialised error struct when it errors out;
// the lexer was also designed to be as general-purpose as possible. this means that it can be retargeted from one language onto another very easily.

int VERB_lexer_is_terminating_token(const VERB_token_t token, const VERB_token_t* const terminating_tokens, const size_t terminating_tokens_count){
    for(size_t i = 0; i < terminating_tokens_count; i++){
        if(token == terminating_tokens[i]) return 1;
    }

    return 0;
}

VERB_variable_definition_t* VERB_lexer_line_process_element_convert(VERB_tokeniser_t* const restrict tokeniser, const VERB_bytecode_argcnt_t srccnt, VERB_variable_token_t* const restrict srcargs, const VERB_bytecode_t op){
    VERB_variable_definition_t** restrict src = malloc(srccnt*sizeof(*src));
    for(VERB_bytecode_argcnt_t i = 0; i < srccnt; i++) src[i] = VERB_variable_search_byToken(&tokeniser->backend, srcargs[i]);

    VERB_variable_definition_t* var;
    if(op == VERB_BC_conversion){ 
        var = src[1]; 
        goto end;                           // YES I KNOW an else statement would do the EXACT SAME THING but I disliked how it looked with indentation. 
    }
    uint64_t conversion_mask = 0;           // only allows up to 64 arguments to a single op... not that it's not enough, but there HAS to be a better way.
    var = VERB_variable_list_compatible(&tokeniser->backend, srccnt, src, &conversion_mask);
    if(!var){ 
        VERB_error_report(tokeniser->specifics, VERB_error_invalid_types, tokeniser->line, tokeniser->offset, 3, op, srccnt, src);
    // assigns it to first var; perhaps this is wrong. there should be a VERB_type_special_compatible_with_all type that skips over type compatibility shenanigans.
        var = VERB_variable_search_byToken(&tokeniser->backend, srcargs[0]);
    }
    VERB_variable_list_convert(&tokeniser->backend, tokeniser->code, srccnt, srcargs, var, conversion_mask);
end:
    free(src);
    return var;
}

void VERB_lexer_line_process_element(VERB_tokeniser_t* const restrict tokeniser, const VERB_bytecode_t op, const uint32_t current_offset, VERB_array_t* restrict const ops, VERB_array_t* restrict const values){
    if(op >= VERB_BC_special_START) return;

    const VERB_bytecode_argcnt_t srccnt = VERB_bytecode_argcnt_src(VERB_bytecode_argcnt[op]);
    const VERB_bytecode_argcnt_t dstcnt = VERB_bytecode_argcnt_dst(VERB_bytecode_argcnt[op]);
    VERB_variable_token_t* const restrict srcargs = VERB_array_top_offset_typed(srccnt, VERB_variable_token_t, values);
// checks for compatibility between types & operations.

// checks if variables are compatible under given op, O(nlogn) where n == number of source variables. returns ptr to type to be used.
    VERB_variable_definition_t* const restrict var = VERB_lexer_line_process_element_convert(tokeniser, srccnt, srcargs, op);

// pushes op, src arguments & dst arguments respectively, in that order.
    VERB_bytecode_push(op, tokeniser->code);
    VERB_bytecode_push_str(srcargs, srccnt*sizeof(VERB_variable_token_t), tokeniser->code);
    VERB_array_remove_type(srccnt, VERB_variable_token_t, values);
// pushes dst arguments HERE; pushes them both to values array/stack & to backend array/stack.
// TODO: give it a type based on the types of the source args! make it a proper token! give it a type & put it in the rht & everything!
    for(VERB_bytecode_argcnt_t i = 0; i < dstcnt; i++){
        VERB_variable_token_t dst = VERB_variable_definition_internal_init(&tokeniser->backend, var->type, var->custom_data);

        VERB_bytecode_push_str(&dst, sizeof(dst), tokeniser->code);
        VERB_array_push_string(&dst, sizeof(dst), values);
    }
}

void VERB_lexer_line_isoperand(const VERB_bytecode_t bc, VERB_tokeniser_t* const restrict tokeniser, const uint32_t current_offset, VERB_array_t* restrict const ops, VERB_array_t* restrict const values){
    VERB_bytecode_t top = VERB_array_top(VERB_bytecode_t, ops);
    
    if(VERB_priority[top] < VERB_priority[bc]) VERB_array_push_string(&bc, sizeof(bc), ops);
    else while(
        VERB_priority[(top = VERB_array_pop_type(VERB_bytecode_t, ops))] < VERB_priority[bc] + (current_offset? VERB_PRIORITY_special_highest: 0)
    ) VERB_lexer_line_process_element(tokeniser, top, current_offset, ops, values);
}

VERB_array_t* VERB_lexer_line(char* restrict*, VERB_tokeniser_t*, const bool);

void VERB_lexer_line_handle_calls(char* restrict* const restrict string, VERB_tokeniser_t* const restrict tokeniser, const VERB_variable_token_t function_name, VERB_array_t* const restrict values){    
    VERB_variable_definition_t* const restrict defOverloaded = VERB_variable_search_byToken(&tokeniser->backend, function_name);
// function does not exist.
    if(!defOverloaded || defOverloaded->group != VERB_VARIABLE_DEFINITION_FUN){
        VERB_error_report(tokeniser->specifics, VERB_error_invalid_statement, tokeniser->line, tokeniser->offset, 1, "UNKNOWN FUNCTION CALLED");
        return;
    }
// parses function arguments.
    VERB_array_t* const restrict functionArgs = VERB_lexer_line(string, tokeniser, true);
    VERB_array_extract_all(functionArgs, const size_t srccnt, VERB_variable_token_t* const restrict srcargs);
// resolves function overloads.
    VERB_variable_definition_t* const restrict def = VERB_variable_function_resolve_overloads(&tokeniser->backend, defOverloaded, srccnt);
    if(!def){
        VERB_error_report(tokeniser->specifics, VERB_error_invalid_statement, tokeniser->line, tokeniser->offset, 1, "FUNCTION CALL HAS AN INVALID NUMBER OF ARGUMENTS");
        free(srcargs);
        return;
    }
    VERB_bytecode_push_type(def->name, VERB_variable_token_t, tokeniser->code);
// checks the function arguments' types.
    VERB_variable_function_t* const restrict fun = def->custom_data;

    for(size_t i = 0; i < srccnt; i++){
        VERB_variable_definition_t* const restrict src = VERB_variable_search_byToken(&tokeniser->backend, srcargs[i]);
        VERB_variable_definition_t* const restrict dst = fun->src[i];
// if the two types are not compatible, converts src to dst's type.
        if(!VERB_variable_type_implicit_conversion_allowed_bool(&tokeniser->backend, src, dst)){
            VERB_error_report(tokeniser->specifics, VERB_error_invalid_types, tokeniser->line, tokeniser->offset, 5, VERB_BC_call, i, &tokeniser->backend, dst->type, src->type);
            free(srcargs);
            return;
        }
        if(!VERB_variable_compatible(src->type, dst->type)) 
            srcargs[i] = VERB_variable_convert(&tokeniser->backend, tokeniser->code, srcargs[i], dst->custom_data, dst->name, dst->type);
    }

// -- ASSEMBLING OPCODE -- .
// puts basic opcode.
    VERB_bytecode_push(VERB_BC_call, tokeniser->code);
    VERB_bytecode_push_type(srccnt, size_t, tokeniser->code);
    VERB_bytecode_push_str(srcargs, srccnt*sizeof(*srcargs), tokeniser->code);
// cleanup.
    free(srcargs);
// pushes dst function arguments to bytecode array.
    const size_t dstcnt = VERB_variable_function_get_dstcnt(def);
    VERB_bytecode_push_type(dstcnt, size_t, tokeniser->code);

    for(size_t i = 0; i < dstcnt; i++){
// TODO: add proper custom data.
        VERB_variable_token_t dst = VERB_variable_definition_internal_init(&tokeniser->backend, fun->dst[i]->type, NULL);
        VERB_bytecode_push_str(&dst, sizeof(dst), tokeniser->code);
        VERB_array_push_string(&dst, sizeof(dst), values);
    }
}

VERB_array_t* VERB_lexer_line(char* restrict* const restrict string, VERB_tokeniser_t* const restrict tokeniser, const bool isFunctionCall){
    VERB_array_t ops = VERB_array_init(), values = VERB_array_init();
// it should be faster returning a dynamically-allocated ptr to VERB_array_t over a statically-allocated one, despite dynamic allocation being slowww ;-;.
    VERB_array_t* functionArgs;
    if(isFunctionCall){ functionArgs = malloc(sizeof(*functionArgs)); *functionArgs = VERB_array_init(); }
// gets longest string terminated by the valid terminator symbols.
    const char* const restrict line_terminating_symbols = ".";
    char* const end = *string + strcspn(*string, line_terminating_symbols);
// incremented each time '(' is met, decremented each time ')' is met.
    uint32_t current_offset = 0;
// >= so the final period is processed too.
    while(end >= *string){
        const VERB_variable_token_t tok = VERB_token_get(string, tokeniser);
        if(tok == VERB_BC_special_IGNORE) continue;

        if(tok >= VERB_BC_special_NAME_START){
            if(VERB_variable_name_isFunction(tok))
               VERB_lexer_line_handle_calls(string, tokeniser, VERB_array_pop_type(VERB_variable_token_t, &values), &values);
            else VERB_array_push_string(&tok, sizeof(tok), &values); 

            continue;
        }

        const VERB_bytecode_t opcode = tok;
        if(opcode == VERB_BC_special_OPENED_ROUND_BRACKET) current_offset++;
        else if(opcode == VERB_BC_special_CLOSED_ROUND_BRACKET){
            if(!current_offset) break;
            else current_offset--;
            
            VERB_bytecode_t op = VERB_array_pop_type(VERB_bytecode_t, &ops);
            if(op == VERB_BC_special_OPENED_ROUND_BRACKET)
                VERB_error_report(tokeniser->specifics, VERB_error_invalid_statement, tokeniser->line, tokeniser->offset, 1, "OPENED PARENTHESIS IMMEDIATELY TERMINATED BY CLOSED ONE WITH NO EXPRESSION NOR VALUE");
            
            while(op != VERB_BC_special_OPENED_ROUND_BRACKET){
                VERB_lexer_line_process_element(tokeniser, op, current_offset, &ops, &values);
                op = VERB_array_pop_type(VERB_bytecode_t, &ops);
            } 
            
            continue;
        }
        if(isFunctionCall && !current_offset && opcode == VERB_BC_comma){
            VERB_lexer_line_isoperand(VERB_BC_farg, tokeniser, current_offset, &ops, &values);
            const VERB_variable_token_t arg = VERB_array_pop_type(VERB_variable_token_t, &values);
            VERB_array_push(arg, functionArgs);

            continue;
        } 
        VERB_lexer_line_isoperand(opcode, tokeniser, current_offset, &ops, &values);            // implicit conversion in first arg is intended!
    }

    while(!VERB_array_is_empty(&ops)){                                                          // consumes all ops.
        const VERB_bytecode_t op = VERB_array_pop_type(VERB_bytecode_t, &ops);
        VERB_lexer_line_process_element(tokeniser, op, current_offset, &ops, &values);
    }

    VERB_array_destroy(&ops, VERB_array_keep_ctx);
    VERB_array_destroy(&values, VERB_array_keep_ctx);

    return functionArgs;
}

// given string of VERB code, converts it to string of tokens that has to be freed by user
void* VERB_lexer(char* restrict string, VERB_error_specifics_t** restrict specifics, size_t* const restrict tokenised_code_len){
// static initialization? it doesn't really matter which one I use in the end.
    VERB_tokeniser_t tokeniser; VERB_tokeniser_init(&tokeniser);

    while(*string) VERB_lexer_line(&string, &tokeniser, false);
// DEBUG INFO: in theory this function SHOULD be called but as of 3/26/2026 nothing changes if we do not.
    if(false) VERB_bytecode_op_eof(&string, &tokeniser);
    *specifics = tokeniser.specifics; tokeniser.specifics = NULL;

    *tokenised_code_len = VERB_array_extract_length(tokeniser.code);
    void* tokenised_string = VERB_array_extract(tokeniser.code);
    VERB_tokeniser_destroy(&tokeniser, VERB_tokeniser_keep_ctx);

    return tokenised_string;
}

#endif
