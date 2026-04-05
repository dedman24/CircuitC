#ifndef VERB_compiler_ops_type_handler_included
#define VERB_compiler_ops_type_handler_included

#include "../includes.h"
#include "variable_or_func.h"       // needed to distinguish whether a statement is a variable or a function

// handles types

VERB_VM_ADDR_t VERB_compiler_op_type_handler_allocate_variable(VERB_compilation_t* const restrict compilation, VERB_array_t* const restrict bytecode, char* restrict tokens, char* restrict type, const size_t type_len){
// formatting checked earlier; this is a name token & we know it.
    VERB_token_read(&tokens);
    const size_t name_length = VERB_token_get_nameval_size(&tokens);
// adds variable to scope
    const VERB_VM_ADDR_t addr = VERB_compilation_add_to_scope_var(compilation, tokens, name_length, type, type_len);
// EALLOC 1
    VERB_compiler_insn_EALLOC(bytecode, VERB_BYTECODE_ARG_IMM32, VERB_BYTECODE_ARG_NONE, 1, NULL, VERB_BYTECODE_ARG_NONE);
// skips over name token
    VERB_token_skip_over_area(&tokens, name_length);
    return addr;
}

void* VERB_compiler_op_type_handler_variable(VERB_compilation_t* const restrict compilation, VERB_array_t* const restrict bytecode, char* restrict tokens){
// symbol under which we exit from this function altogether
    const VERB_token_t exiting_symbol = VERB_TOKEN_SEMICOLON;
// symbols that comport valid variable delimeters, of which the exiting symbol is always part.
    const VERB_token_t closing_symbols[] = {
        VERB_TOKEN_COMMA,
        exiting_symbol
    };
    const size_t closing_symbols_size = sizeof(closing_symbols)/sizeof(*closing_symbols);

    VERB_token_t closing_token;

    size_t type_len;
    char* restrict type = VERB_compilation_extract_type_qualifier(compilation, &type_len);

    do{
        const VERB_VM_ADDR_t absaddr = VERB_compiler_op_type_handler_allocate_variable(compilation, bytecode, tokens, type, type_len);
        
        tokens = VERB_compile(compilation, bytecode, tokens, closing_symbols, closing_symbols_size, &closing_token);
        if(!tokens) return NULL;

        const VERB_VM_ADDR_t reladdr = VERB_compilation_abs_addr_to_estack_off(compilation, absaddr);
        VERB_compiler_insn_STORE(bytecode, VERB_BYTECODE_ARG_IMM32, reladdr, VERB_BYTECODE_ARG_NONE, NULL, VERB_BYTECODE_ARG_NONE);
    } while(closing_token != exiting_symbol);
// can be freed because type is reallocated & copied over when adding to scope
    free(type);

    return tokens;
}

void* VERB_compiler_op_type_allocate_function(VERB_compilation_t* const restrict compilation, char* restrict tokens){
    VERB_token_read(&tokens);
    const size_t name_length = VERB_token_get_nameval_size(&tokens);

    size_t type_len;
    char* restrict type = VERB_compilation_extract_type_qualifier(compilation, &type_len);
    VERB_compilation_add_to_scope_fun(compilation, tokens, name_length, type, type_len);
    free(type);
// skips over name token
    VERB_token_skip_over_area(&tokens, name_length);
    return tokens;
}

void* VERB_compiler_op_type_handler_function(VERB_compilation_t* const restrict compilation, VERB_array_t* const restrict bytecode, char* restrict tokens){
    tokens = VERB_compiler_op_type_allocate_function(compilation, tokens);
// adds new scope under which to define all variables, formal or not
    VERB_compilation_new_scope(compilation);

    const size_t initiator_len = sizeof(VERB_token_t);                  // initial ( in function declaration
    VERB_token_skip_over_area(&tokens, initiator_len);

    const VERB_token_t terminator = VERB_TOKEN_CLOSED_BRACKET, delim = VERB_TOKEN_COMMA;

    VERB_token_t tok;
    while((tok = VERB_token_read(&tokens)) != terminator){
        if(tok == VERB_TOKEN_EOF) return NULL;                          // huge error
// read all variable tokens until name & allocates new variable with that name
        if(tok == VERB_TOKEN_NAME){
// length of name
            const size_t name_len = VERB_token_get_nameval_size(&tokens);
            size_t type_len;
            char* type = VERB_compilation_extract_type_qualifier(compilation, &type_len);

            VERB_compilation_add_to_scope_var(compilation, tokens, name_len, type, type_len);
            VERB_token_skip_over_area(&tokens, name_len);
        }
        else if(tok == delim) continue;
        else VERB_compilation_add_type_qualifier(compilation, tok);

    }

    return tokens;
}

void* VERB_compiler_op_type_handler_suffix_qualifiers(VERB_compilation_t* const restrict compilation, char* restrict tokens, const VERB_token_t type){
    VERB_token_t tok;

    VERB_compilation_add_type_qualifier(compilation, type);
// type qualifiers are (potentially) everything from before type that is allowed to be a qualifier to VERB_TOKEN_NAME.
// how these are handled is then up to the interpreter.
    while((tok = VERB_token_read(&tokens)) != VERB_TOKEN_NAME)
        VERB_compilation_add_type_qualifier(compilation, tok);
// we skip back because we read VERB_TOKEN_NAME, which we weren't supposed to do.
    VERB_token_skip_back_area(&tokens, sizeof(VERB_token_t));

    return tokens;
}

void* VERB_compiler_op_type_handler(VERB_compilation_t* const restrict compilation, VERB_array_t* const restrict bytecode, char* restrict tokens, const VERB_token_t type){
    int statement_type = VERB_compiler_determine_statement(tokens);

    tokens = VERB_compiler_op_type_handler_suffix_qualifiers(compilation, tokens, type);

    if(statement_type == VERB_COMPILER_IS_VARIABLE)
        tokens = VERB_compiler_op_type_handler_variable(compilation, bytecode, tokens);
    else if(statement_type == VERB_COMPILER_IS_FUNCTION)
        tokens = VERB_compiler_op_type_handler_function(compilation, bytecode, tokens);
    else tokens = NULL;

    VERB_compilation_clear_type_qualifiers(compilation);
    return tokens;
}

#endif
