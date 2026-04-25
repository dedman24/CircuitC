#ifndef VERB_library_error_handling_included
#define VERB_library_error_handling_included

#include "type.h"
#include "stdio.h"                                                  // sprintf.
#include "stdarg.h"                                                 // support for variadic functions.
#include "string.h"                                                 // string operations.
#include "stdlib.h"                                                 // dynamic memory handling.
#include "../../tokens/token_type.h"                                // for VERB_bytecode_t, VERB_type_t.
#include "../arrays/dynamic_arrays.h"                               // for dynamic array type & ops.
#include "../../lexer/tokeniser/backend/backend.h"                  // for VERB_variable_name_t, VERB_variable_definition_t.
#include "../../lexer/tokeniser/variables/variable_definition.h"    // for VERB_variable_definition_t.
VERB_variable_t* VERB_variable_search_byToken(VERB_tokeniser_backend_t*, VERB_variable_token_t);

typedef enum{
    VERB_error_preprocessor_no_whitespace,
    VERB_error_preprocessor_no_name_after,
    VERB_error_preprocessor_include_no_file,
    VERB_error_invalid_statement,
    VERB_error_custom,
    VERB_error_value,
    VERB_error_debug,                                   // for debug statements.
    VERB_error_invalid_types_op,                        // invalid types in a normal operation.
    VERB_error_invalid_types_call,                      // invalid types in a function call.
    VERB_error_macro_redefinition,                      // for when a macro is redefined.
    VERB_error_macro_argument_redefinition,             // for when a macro's argument is redefined.
    VERB_error_const_violation_call,                    // argument supposed to be const is passed a non-const argument.
} VERB_error_code_t;

#define VERB_error_report_primitive_start(specifics, line, offset)                          \
    do{                                                                                     \
        const char* const msg_line = "ERROR at line ";                                      \
        VERB_array_push_string(msg_line, strlen(msg_line), &(specifics)->errors);           \
        \
        char line_str[21];                                                                  \
        sprintf(line_str, "%llu", (line));                                                  \
        VERB_array_push_string(line_str, strlen(line_str), &(specifics)->errors);           \
        \
        const char* const msg_offset = " and offset ";                                      \
        VERB_array_push_string(msg_offset, strlen(msg_offset), &(specifics)->errors);       \
        \
        char offset_str[24];                                                                \
        sprintf(offset_str, "%llu : ", (offset));                                           \
        VERB_array_push_string(offset_str, strlen(offset_str), &(specifics)->errors);       \
    } while(0)

// 'error' holds error to add to array of all errors.
void VERB_error_report_primitive_arr(VERB_error_specifics_t* specifics, char* restrict* const restrict error, size_t* const restrict len, const size_t count, const unsigned long long line, const unsigned long long offset){
    VERB_error_report_primitive_start(specifics, line, offset);

    for(size_t i = 0; i < count; i++)                                       // pushes all errors
        VERB_array_push_string(error[i], len[i], &specifics->errors);
    VERB_array_push_string("\n", strlen("\n") + 1, &specifics->errors);     // must be terminated by newline and 0.
}

void VERB_error_report__varg(VERB_error_specifics_t* specifics, const unsigned long long line, const unsigned long long offset, const size_t count, ...){
// C variadic functions are such a mess... I want VERB to be much cleaner.
    va_list args;
    va_start(args, count);

    char* (*error)[count] = malloc(sizeof(*error));
    size_t (*len)[count] = malloc(sizeof(*len));

    for(size_t i = 0; i < count; i++){
        (*error)[i] = va_arg(args, char*);
        (*len)[i] = strlen((*error)[i]);
    }

    VERB_error_report_primitive_arr(specifics, *error, *len, count, line, offset);

    va_end(args);
    free(error);
    free(len);
}

void VERB_error__type_push(VERB_type_t* const restrict type, VERB_array_t* const restrict arr){
    for(size_t i = 0; i < type->siglen; i++){
        VERB_array_push_string(VERB_type_tok_strings[type->sig[i]], strlen(VERB_type_tok_strings[type->sig[i]]), arr);
        if(i != type->siglen - 1) VERB_array_push_string(" ", strlen(" "), arr);
    }
}

// name ~ names of variables with conflicting types.
void VERB_error_report__type_call(VERB_error_specifics_t* specifics, const unsigned long long line, const unsigned long long offset, const size_t index, VERB_tokeniser_backend_t* const restrict backend, VERB_type_t* const restrict expected, VERB_type_t* const restrict actual){
    VERB_error_report_primitive_start(specifics, line, offset);

    VERB_array_push_string("CONFLICTING TYPES IN FUNCTION CALL FOR ARGUMENT No. ", strlen("CONFLICTING TYPES IN FUNCTION CALL FOR ARGUMENT No. "), &specifics->errors);
// pushes index of where error happened.
    char index_str[21];
    sprintf(index_str, "%ld", index);
    VERB_array_push_string(index_str, strlen(index_str), &specifics->errors);

    VERB_array_push_string(" - EXPECTED '", strlen(" - EXPECTED"), &specifics->errors);
    VERB_error__type_push(expected, &specifics->errors);
    VERB_array_push_string("' OR COMPATIBLE TYPE, PASSED '", strlen("' OR COMPATIBLE TYPE, PASSED '"), &specifics->errors);
    VERB_error__type_push(actual, &specifics->errors);
    VERB_array_push_string("'\n", strlen("'\n") + 1, &specifics->errors);   // so it may push the \0 too.
}

// name ~ names of variables with conflicting types.
void VERB_error_report__type_op(VERB_error_specifics_t* specifics, const unsigned long long line, const unsigned long long offset, const VERB_bytecode_t op_tok, const size_t argcnt, VERB_variable_t* restrict* const restrict args){
    VERB_error_report_primitive_start(specifics, line, offset);

    VERB_array_push_string("CONFLICTING TYPES IN OPERATION '", strlen("CONFLICTING TYPES IN OPERATION '"), &specifics->errors);
    const char* const restrict op_str = VERB_bytecode_str[op_tok];
    VERB_array_push_string(op_str,  strlen(op_str),  &specifics->errors);
    VERB_array_push_string("' - '", strlen("' - '"), &specifics->errors);

    for(size_t i = 0; i < argcnt; i++){
        VERB_error__type_push(args[i]->type, &specifics->errors);
        if(i != argcnt - 1)
            VERB_array_push_string("' AND '", strlen("' AND '"), &specifics->errors);
        else
            VERB_array_push_string("'\n", strlen("'\n") + 1, &specifics->errors);
    }
}

void VERB_error_report_types(VERB_error_specifics_t* specifics, const unsigned long long line, const unsigned long long offset, const VERB_bytecode_t op_tok, const size_t argcnt, ...){
    va_list variables; va_start(variables, argcnt);

    VERB_variable_t** const restrict arguments = malloc(sizeof(*arguments)*argcnt);
    for(size_t i = 0; i < argcnt; i++) arguments[i] = va_arg(variables, VERB_variable_t*);
    VERB_error_report__type_op(specifics, line, offset, op_tok, argcnt, arguments);
    free(arguments);

    va_end(variables);
}


// msg ~ any custom null-terminated message to add to error.
// error_code ~ specific error code.
void VERB_error_report(VERB_error_specifics_t* specifics, const VERB_error_code_t error_code, const unsigned long long line, const unsigned long long offset, const size_t count, ...){
    va_list msg; va_start(msg, count);

    switch(error_code){
        case VERB_error_preprocessor_no_whitespace:{
            VERB_error_report__varg(specifics, line, offset, 2, "INVALID STATEMENT: NO WHITESPACES AFTER ", va_arg(msg, char*));
            break;
        }
        case VERB_error_preprocessor_no_name_after:{
            VERB_error_report__varg(specifics, line, offset, 2, "INVALID STATEMENT: NO NAME AFTER ", va_arg(msg, char*));
            break;
        }
        case VERB_error_preprocessor_include_no_file:{
            VERB_error_report__varg(specifics, line, offset, 2, "NO FILE NAMED ", va_arg(msg, char*));
            break;
        }
        case VERB_error_invalid_statement:{
            VERB_error_report__varg(specifics, line, offset, 2, "INVALID STATEMENT: ", va_arg(msg, char*));
            break;
        }
        case VERB_error_custom:{
            VERB_error_report__varg(specifics, line, offset, 1, va_arg(msg, char*));
            break;
        }
        case VERB_error_value:{
            VERB_error_report__varg(specifics, line, offset, 1, "INVALID FORMATTING OF VALUE");
            break;
        }
        case VERB_error_invalid_types_op:{
// op - arg cnt - array of variables in op
            VERB_error_report__type_op(specifics, line, offset, va_arg(msg, int), va_arg(msg, size_t), va_arg(msg, VERB_variable_t* restrict*));
            break;
        }
        case VERB_error_invalid_types_call:{
// arg num - tokeniser backend - dst type - src type
            VERB_error_report__type_call(specifics, line, offset, va_arg(msg, size_t), va_arg(msg, VERB_tokeniser_backend_t*), va_arg(msg, VERB_type_t*), va_arg(msg, VERB_type_t*));
            break;
        }
        case VERB_error_debug:{
            VERB_error_report__varg(specifics, line, offset, 2, "DEBUG ERROR: ", va_arg(msg, char*));
            break;
        }
        case VERB_error_macro_redefinition:{                // expects macro name, macro name length.
            char* const restrict name = va_arg(msg, char*);
            const size_t len = va_arg(msg, size_t); 
            const char old = name[len]; name[len] = '\0';   // null terminates it; macro names are usually not null terminated. 

            VERB_error_report__varg(specifics, line, offset, 3, "INVALID REDEFINITION OF MACRO '", name, "'");
            name[len] = old;
            break;
        }
        case VERB_error_macro_argument_redefinition:{       // expects macro name, macro name length.
            char* const restrict name = va_arg(msg, char*);
            const size_t len = va_arg(msg, size_t); 
            const char old = name[len]; name[len] = '\0';   // null terminates it; macro names are usually not null terminated. 

            VERB_error_report__varg(specifics, line, offset, 3, "INVALID REDEFINITION OF MACRO ARGUMENT '", name, "'");
            name[len] = old;
            break;
        }
        default: {
            char error_code_str[21];
            sprintf(error_code_str, "%u", error_code);
            VERB_error_report__varg(specifics, line, offset, 3, "ERROR HANDLING IS BROKEN; unhandled error code IS ", error_code_str, "; CONTACT THE DEVELOPERS & SEND THEM THIS ERROR MESSAGE!");
            break;
        }
    }

    va_end(msg);
}

#endif
