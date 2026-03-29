#ifndef VERB_library_warning_handling_included
#define VERB_library_warning_handling_included

#include "type.h"
#include "stdio.h"                                      // sprintf.
#include "stdarg.h"                                     // support for variadic functions
#include "string.h"                                     // string operations.
#include "stdlib.h"                                     // dynamic memory handling.
#include "error_handling.h"                             // to report any error
#include "../dynamic_arrays.h"                          // for dynamic array type & ops.

typedef enum{
    VERB_warning_custom,
} VERB_warning_code_t;

// 'warning' holds warning to add to array of all warnings.
void VERB_warning_report_many_primitive(VERB_error_specifics_t* specifics, char* restrict* const restrict warning, size_t* const restrict len, const size_t count, const unsigned long long line, const unsigned long long offset){
    const char* const msg_line = "ERROR at line ";
    VERB_array_push_string(msg_line, strlen(msg_line), &specifics->warnings);
// 21 because llu has maximum 20 characters, +1 for the \0 ending.
    char line_str[21];
    sprintf(line_str, "%llu", line);
    VERB_array_push_string(line_str, strlen(line_str), &specifics->warnings);
// 
    const char* const msg_offset = " and offset ";
    VERB_array_push_string(msg_offset, strlen(msg_offset), &specifics->warnings);
//
    char offset_str[24];
    sprintf(offset_str, "%llu : ", offset);
    VERB_array_push_string(offset_str, strlen(offset_str), &specifics->warnings);

    for(size_t i = 0; i < count; i++)                                   // pushes all warnings
        VERB_array_push_string(warning[i], len[i], &specifics->warnings);
    VERB_array_push_string("\n", 2, &specifics->warnings);              // must be terminated by newline and 0.
}

void VERB_warning_report_many(VERB_error_specifics_t* specifics, const unsigned long long line, const unsigned long long offset, const size_t count, ...){
// C variadic functions are such a mess... I want VERB to be much cleaner.
    va_list args;
    va_start(args, count);

    char* (*warning)[count] = malloc(sizeof(*warning));
    size_t (*len)[count] = malloc(sizeof(*len));

    for(size_t i = 0; i < count; i++){
        (*warning)[i] = va_arg(args, char*);
        (*len)[i] = strlen((*warning)[i]);
    }

    VERB_warning_report_many_primitive(specifics, *warning, *len, count, line, offset);

    va_end(args);
    free(warning);
    free(len);
}

// msg ~ any custom null-terminated message to add to warning.
// warning_code ~ specific warning code.
void VERB_warning_report(VERB_error_specifics_t* specifics, char* const msg, const VERB_warning_code_t warning_code, const unsigned long long line, const unsigned long long offset){
    switch(warning_code){
        case VERB_warning_custom:{
            VERB_warning_report_many(specifics, line, offset, 1, msg);
            break;
        }
        default: {
            char error_code_str[21];
            sprintf(error_code_str, "%u", warning_code);
            VERB_error_report(specifics, VERB_error_custom, line, offset, 3, "WARNING HANDLING IS BROKEN; unhandled warning code IS ", error_code_str, "; CONTACT THE DEVELOPERS & SEND THEM THIS ERROR MESSAGE!");
            break;
        }
    }
}

#endif
